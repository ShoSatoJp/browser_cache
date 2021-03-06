#include "stdafx.hpp"
#include "addr.hpp"
#include "structs.hpp"
#include "chrome_cache.hpp"

//#include <boost/iostreams/filtering_stream.hpp>
//#include <boost/iostreams/filter/gzip.hpp>
//#include <boost/iostreams/copy.hpp>


#include <gzip/decompress.hpp>

namespace boost {
	void throw_exception(std::exception const & e) {}
}

using namespace std;
//Utils
const vector<string> Utils::files = { "data_0", "data_1", "data_2", "data_3", "index" };

//HttpHeader
const string HttpHeader::to_string() {
	stringstream ss;
	ss << status_source << '\n';
	for (auto p = headers.begin(); p != headers.end(); ++p) {
		ss << setw(20) << right << p->first << " : " << left << p->second << '\n';
	}
	return ss.str();
}
bool HttpHeader::is_gzipped() {
	return headers.count("content-encoding") != 0 && headers["content-encoding"] == "gzip";
}
unique_ptr<HttpHeader> HttpHeader::load_http_header(char* data, int size) {
	return unique_ptr<HttpHeader>(load_http_header_ptr(data, size));
}
HttpHeader* HttpHeader::load_http_header_ptr(char* data, int size) {
	stringstream ss;
	vector<string> src;
	bool isheader = false;
	for (int i = 0; i < size; i++) {
		if (!isheader && data[i] == 'H' && i + 4 < size && string(&data[i], 4) == "HTTP") {
			isheader = true;
		}
		if (isheader) {
			if (data[i] == '\0') {
				src.push_back(ss.str());
				ss.str(""); ss.clear();
				if (i + 1 < size && data[i + 1] == '\0') {
					break;
				}
			} else {
				ss << data[i];
			}
		}
	}
	HttpHeader* header = new HttpHeader();
	string status_source = src.front();
	header->status_source = status_source;
	vector<string> status_re = Utils::split(status_source, ' ');
	if (status_re.size() > 1) {
		header->protocol = status_re[0];
		header->status_code = stoi(status_re[1]);
	}
	src.erase(src.begin());
	for (string str : src) {
		transform(str.begin(), str.end(), str.begin(), ::tolower);
		vector<string> re = Utils::split(str, ':');
		if (re.size() == 2) {
			header->headers[Utils::trim(re[0])] = Utils::trim(re[1]);
		}
	}
	return header;
}
//ChromeCacheAddress
ChromeCacheAddress::ChromeCacheAddress(uint32_t addr) {
	initialized = (addr & kInitializedMask) != 0;
	filetype = static_cast<FileType>((addr & kFileTypeMask) >> kFileTypeOffset);
	if (filetype == FileType::EXTERNAL) {
		num = addr & kFileNameMask;
	} else {
		num = (addr & kStartBlockMask);
		block_length = (addr & kNumBlocksMask) >> kNumBlocksOffset;
		file_selector = (addr & kFileSelectorMask) >> kFileSelectorOffset;
	}
}
const string & ChromeCacheAddress::tostring() {
	stringstream ss;
	ss << "initialized:" << initialized << '\n'
		<< "filetype:" << filetype << '\n'
		<< "num:" << num << '\n'
		<< "block_length:" << block_length << '\n'
		<< "file_selector:" << file_selector << '\n';
	return ss.str();
}
//ChromeCacheEntry
ChromeCacheEntry::ChromeCacheEntry(EntryStore* es, const string &key, ChromeCache* cc) :es(es), key(key), cc(cc) {
	for (uint32_t length : es->data_size) {
		if (length)data_lengths.push_back(length);
	}
	for (CacheAddr addr : es->data_addr) {
		if (addr)data_addrs.push_back(ChromeCacheAddress(addr));
	}
	data_count = data_addrs.size();
}
unique_ptr<HttpHeader> ChromeCacheEntry::get_header() const {
	if (this->data_addrs.size() >= 1 && this->data_lengths.size() >= 1) {
		char * header_data = this->cc->get_data(this->data_addrs[0], this->data_lengths[0]);
		unique_ptr<HttpHeader> header = HttpHeader::load_http_header(header_data, this->data_lengths[0]);
		delete[] header_data;
		return header;
	} else {
		throw runtime_error("no header.");
	}
}
HttpHeader * ChromeCacheEntry::get_header_ptr() const {
	if (this->data_addrs.size() >= 1 && this->data_lengths.size() >= 1) {
		char * header_data = this->cc->get_data(this->data_addrs[0], this->data_lengths[0]);
		HttpHeader* header = HttpHeader::load_http_header_ptr(header_data, this->data_lengths[0]);
		delete[] header_data;
		return header;
	} else {
		throw runtime_error("no header.");
	}
}
void ChromeCacheEntry::save(const string &path) {
	this->cc->save(path, *this);
}
//ChromeCacheBlockFile
ChromeCacheBlockFile::ChromeCacheBlockFile(FileType filetype, const experimental::filesystem::path &cache_dir, ChromeCache * cc) :cc(cc) {
	this->filetype = filetype;
	int filenum = filetype - 1;
	ifs = ifstream(cache_dir / ("data_" + to_string(filenum)), ios::binary | ios::in | ios::out);
	if (!ifs)throw exception(string("cannot open data_" + to_string(filenum) + " file").c_str());
	//cout << !!ifs << endl;
	ifs.read((char*)&header, sizeof(BlockFileHeader));
	data_start = ifs.tellg();
}
char* ChromeCacheBlockFile::get_data(const ChromeCacheAddress &addr, int size) {
	ifs.clear();
	ifs.seekg(data_start + BlockSizeForFileType(addr.filetype)*addr.num, ios::beg);
	char* data_ = new char[size];
	//cout << ifs.tellg() << endl;
	ifs.read(data_, size);
	return data_;
}
void ChromeCacheBlockFile::write_as_file(const string &path, const ChromeCacheAddress &addr, int size) {
	ofstream ofs(path, ios::binary);
	char* data = get_data(addr, size);
	ofs.write(data, size);
	ofs.close();
	delete[] data;
}
ChromeCacheEntry* ChromeCacheBlockFile::get_entry(const ChromeCacheAddress &addr) {
	ifs.clear();
	ifs.seekg(data_start + BlockSizeForFileType(addr.filetype)*addr.num, ios::beg);
	EntryStore *es = new EntryStore();
	ifs.read((char*)es, sizeof(EntryStore));
	int key_len = es->long_key ? 160 : es->key_len;
	//cout << "long_key: " << es->long_key << "  ley_len: " << es->key_len << endl;
	//int key_len = es->key_len > 160 ? 160 : es->key_len;
	char* key_source = new char[key_len + 1];
	ifs.read(key_source, key_len);
	string key = string(key_source, key_len);
	delete[] key_source;
	int64_t i = ifs.tellg();
	if (ifs.fail()) {
		return nullptr;
	} else {
		return new ChromeCacheEntry(es, key, this->cc);
	}
}
void ChromeCacheBlockFile::close() {
	ifs.close();
}
int ChromeCacheBlockFile::BlockSizeForFileType(FileType file_type) {
	switch (file_type) {
		case RANKINGS:
			return 36;
		case BLOCK_256:
			return 256;
		case BLOCK_1K:
			return 1024;
		case BLOCK_4K:
			return 4096;
		case BLOCK_FILES:
			return 8;
		case BLOCK_ENTRIES:
			return 104;
		case BLOCK_EVICTED:
			return 48;
		case EXTERNAL:
			return 0;
	}
	return 0;
}
//ChromeCacheBlockFiles
ChromeCacheBlockFiles::ChromeCacheBlockFiles(const experimental::filesystem::path &cache_dir, ChromeCache *cc) :cc(cc) {
	for (int i = 0; i < 4; i++) {
		blockfiles.push_back(ChromeCacheBlockFile((FileType)(i + 1), cache_dir, cc));
	}
}
ChromeCacheEntry * ChromeCacheBlockFiles::get_entry(const ChromeCacheAddress &addr) {
	return blockfiles[addr.filetype - 1].get_entry(addr);
}
char* ChromeCacheBlockFiles::get_data(const ChromeCacheAddress &addr, int size) {
	return blockfiles[addr.filetype - 1].get_data(addr, size);
}
void ChromeCacheBlockFiles::close() {
	for (int i = 0; i < 4; i++) {
		blockfiles[i].close();
	}
}
//ChromeCache
ChromeCache::ChromeCache(const experimental::filesystem::path& cache_dir, const experimental::filesystem::path &temp_cache_dir, bool update_index = true)
	:cache_dir(cache_dir), temp_cache_dir(temp_cache_dir) {
	if (update_index) Utils::copy_files(cache_dir.string(), temp_cache_dir.string());
	blockfiles = new ChromeCacheBlockFiles(temp_cache_dir, this);
	//header
	ifstream ifs(temp_cache_dir / "index", ios::binary | ios::in);
	if (!ifs)throw runtime_error("cannot open index file.");
	ifs.read((char*)&header, sizeof(IndexHeader));
	//addrs
	vector<CacheAddr> vec(header.table_len);
	ifs.read((char*)&vec[0], sizeof(CacheAddr) * header.table_len);
	ifs.close();
	for (CacheAddr ca : vec) {
		if (ca != 0) {
			addrs.push_back(ChromeCacheAddress(ca));
		}
	}
	//entries
	for (ChromeCacheAddress addr : addrs) {
		ChromeCacheEntry *entry = blockfiles->get_entry(addr);
		if (entry != nullptr && entry->key.find("http") == 0) {
			if (entry->es->long_key) {
				entry->key = string(get_data(entry->es->long_key, entry->es->key_len), entry->es->key_len);
			}
			entries.push_back(entry);
			entries_map[entry->key] = entry;
		} else {
			delete entry;
		}
	}
}
void ChromeCache::show_keys() {
	for (string key : keys()) {
		cout << key << endl;
	}
}
vector<string> ChromeCache::keys() {
	vector<string> keys;
	for (ChromeCacheEntry * entry : entries) {
		keys.push_back(entry->key);
	}
	sort(keys.begin(), keys.end());
	return keys;
}
unique_ptr<ChromeCacheEntry> ChromeCache::get_entry(int i) {
	return unique_ptr<ChromeCacheEntry>(entries[i]);
}
ChromeCacheEntry ChromeCache::find(const string &key) {
	return *find_ptr(key);
}
ChromeCacheEntry ChromeCache::find_map(const string &key) {
	return *find_map_ptr(key);
}
ChromeCacheEntry* ChromeCache::find_ptr(const string &key) {
	for (ChromeCacheEntry*entry : entries) {
		if (entry->key == key)return entry;
	}
	throw runtime_error("entry not found.");
}
ChromeCacheEntry *ChromeCache::find_map_ptr(const string &key) {
	if (entries_map.find(key) != entries_map.end()) {
		return entries_map[key];
	} else {
		throw runtime_error("key not found in map.");
	}
}
void ChromeCache::save(const string& path, const ChromeCacheEntry &entry) {
	const int data_size = entry.data_addrs.size();
	if (data_size == 0) throw runtime_error("no header and data.");
	if (data_size >= 1) {
		//http header
		unique_ptr<HttpHeader> header = entry.get_header();
		//cout << header->to_string() << endl;
		if (data_size >= 2) {
			//data
			ChromeCacheAddress addr = entry.data_addrs[1];
			if (addr.filetype == EXTERNAL) {
				save_separated_file(path, addr);
			} else {
				uint32_t size = entry.data_lengths[1];
				save_as_file(path, addr, size, header->is_gzipped());
			}
		} else {
			throw runtime_error("no data, header only exists.");
		}
	}
}
void ChromeCache::find_save(const string& key, const string &path) {
	//save(path, *find_map_ptr(key));
	save(path, find(key));
}
void ChromeCache::close() {
	blockfiles->close();
}
int ChromeCache::count() {
	return entries.size();
}
//const string & ChromeCache::decompress_gzip(char* data, int size) {
//	return 
//	//stringstream ss;
//	//ss.write(data, size);
//	//boost::iostreams::filtering_istream filter;
//	//filter.push(boost::iostreams::gzip_decompressor());
//	//filter.push(ss);
//	//stringstream out;
//	//boost::iostreams::copy(filter, out);
//	//return out;
//}
char* ChromeCache::get_data(const ChromeCacheAddress &addr, int size) {
	return blockfiles->get_data(addr, size);
}
void ChromeCache::save_as_file(const string &path, const ChromeCacheAddress &addr, int size, bool gzipped = false) {
	ofstream ofs(path, ios::binary);
	char* data = get_data(addr, size);
	if (gzipped) {
		ofs << gzip::decompress(data, size);
	} else {
		ofs.write(data, size);
	}
	ofs.close();
	delete[] data;
}
void ChromeCache::save_separated_file(const experimental::filesystem::path &path, const ChromeCacheAddress &addr) {
	stringstream ss;
	ss << "f_" << setfill('0') << right << setw(6) << hex << addr.num << flush;
	experimental::filesystem::path target = cache_dir / ss.str();
	if (!experimental::filesystem::exists(target))
		throw runtime_error("taget file does not exists.");
	string parent = path.parent_path().string();
	if (!experimental::filesystem::exists(parent.size() == 0 ? "." : parent))
		throw runtime_error("destination directory does not exists.");
	try {
		experimental::filesystem::copy_file(target, path, experimental::filesystem::copy_options::overwrite_existing);
	} catch (experimental::filesystem::filesystem_error&) {
		cout << "copy failed." << endl;
		throw runtime_error("copy failed.");
	}
}
ChromeCache::~ChromeCache() {
	//cout <<"size:"<< this->entries.size() << endl;
	for (int i = 0, len = this->entries.size(); i < len; i++) {
		delete entries[i];
	}
	this->close();
	//for (ChromeCacheEntry *entry : this->entries) {
	//	delete entry;
	//}
	delete blockfiles;
}