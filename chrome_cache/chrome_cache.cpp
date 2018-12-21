#include "stdafx.hpp"
#include "addr.hpp"
#include "structs.hpp"
#include "chrome_cache.hpp"

using namespace std;
//Utils
const vector<string> Utils::files = { "data_0", "data_1", "data_2", "data_3", "index" };

//HttpHeader
string HttpHeader::to_string() {
	stringstream ss;
	ss << status_source << '\n';
	for (pair<string, string> p : headers) {
		ss << setw(20) << right << p.first << " : " << left << p.second << '\n';
	}
	return ss.str();
}
bool HttpHeader::is_gzipped() {
	return headers.count("content-encoding") != 0 && headers["content-encoding"] == "gzip";
}
HttpHeader HttpHeader::load_http_header(char* data, int size) {
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
	HttpHeader header;
	string status_source = src.front();
	header.status_source = status_source;
	vector<string> status_re;
	boost::algorithm::split(status_re, status_source, boost::is_any_of(" "));
	if (status_re.size() == 2) {
		header.protocol = status_re[0];
		header.status_code = stoi(status_re[1]);
	}
	src.erase(src.begin());
	for (string str : src) {
		transform(str.begin(), str.end(), str.begin(), ::tolower);
		vector<string> re;
		boost::algorithm::split(re, str, boost::is_any_of(":"));
		if (re.size() == 2) {
			header.headers[re[0]] = re[1];
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
string ChromeCacheAddress::tostring() {
	stringstream ss;
	ss << "initialized:" << initialized << '\n'
		<< "filetype:" << filetype << '\n'
		<< "num:" << num << '\n'
		<< "block_length:" << block_length << '\n'
		<< "file_selector:" << file_selector << '\n';
	return ss.str();
}
//ChromeCacheEntry
ChromeCacheEntry::ChromeCacheEntry(EntryStore* es, string key, ChromeCache* cc) :es(es), key(key), cc(cc) {
	for (uint32_t length : es->data_size) {
		if (length)data_lengths.push_back(length);
	}
	for (CacheAddr addr : es->data_addr) {
		if (addr)data_addrs.push_back(ChromeCacheAddress(addr));
	}
	//if (data_lengths.size() == data_addrs.size()) {
	//	data_count = data_lengths.size();
	//} else {
	//	throw exception();
	//}
	data_count = data_addrs.size();
}
HttpHeader ChromeCacheEntry::get_header() {
	char * header_data = this->cc->get_data(this->data_addrs[0], this->data_lengths[0]);
	HttpHeader header = HttpHeader::load_http_header(header_data, this->data_lengths[0]);
	delete[] header_data;
	return header;
}
void ChromeCacheEntry::save(string path) {
	this->cc->save(path, *this);
}
//ChromeCacheBlockFile
ChromeCacheBlockFile::ChromeCacheBlockFile(FileType filetype, filesystem::path cache_dir, ChromeCache * cc) :cc(cc) {
	this->filetype = filetype;
	int filenum = filetype - 1;
	ifs = ifstream(cache_dir / ("data_" + to_string(filenum)), ios::binary | ios::in | ios::out);
	if (!ifs)throw exception(string("cannot open data_" + to_string(filenum) + " file").c_str());
	//cout << !!ifs << endl;
	ifs.read((char*)&header, sizeof(BlockFileHeader));
	data_start = ifs.tellg();
}
char* ChromeCacheBlockFile::get_data(ChromeCacheAddress addr, int size) {
	ifs.clear();
	ifs.seekg(data_start + BlockSizeForFileType(addr.filetype)*addr.num, ios::beg);
	char* data_ = new char[size];
	//cout << ifs.tellg() << endl;
	ifs.read(data_, size);
	return data_;
}
void ChromeCacheBlockFile::write_as_file(string path, ChromeCacheAddress addr, int size) {
	ofstream ofs(path, ios::binary);
	char* data = get_data(addr, size);
	ofs.write(data, size);
	ofs.close();
	delete[] data;
}
ChromeCacheEntry* ChromeCacheBlockFile::get_entry(ChromeCacheAddress addr) {
	ifs.clear();
	ifs.seekg(data_start + BlockSizeForFileType(addr.filetype)*addr.num, ios::beg);
	//cout << ifs.tellg() << endl;
	EntryStore *es = new EntryStore();
	ifs.read((char*)es, sizeof(EntryStore));
	//cout << ifs.tellg() << endl;
	int key_len = es->key_len > 160 ? 160 : es->key_len;
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
ChromeCacheBlockFiles::ChromeCacheBlockFiles(filesystem::path cache_dir, ChromeCache *cc) :cc(cc) {
	for (int i = 0; i < 4; i++) {
		blockfiles.push_back(ChromeCacheBlockFile((FileType)(i + 1), cache_dir, cc));
	}
}
ChromeCacheEntry * ChromeCacheBlockFiles::get_entry(ChromeCacheAddress addr) {
	return blockfiles[addr.filetype - 1].get_entry(addr);
}
char* ChromeCacheBlockFiles::get_data(ChromeCacheAddress addr, int size) {
	return blockfiles[addr.filetype - 1].get_data(addr, size);
}
void ChromeCacheBlockFiles::close() {
	for (int i = 0; i < 4; i++) {
		blockfiles[i].close();
	}
}
//ChromeCache
ChromeCache::ChromeCache(filesystem::path cache_dir, filesystem::path temp_cache_dir, bool update_index = true)
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
		if (entry != nullptr) {
			if (entry->es->long_key) {
				entry->key = string(get_data(entry->es->long_key, entry->es->key_len), entry->es->key_len);
			}
			entries.push_back(entry);
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
ChromeCacheEntry ChromeCache::get_entry(int i) {
	return *entries[i];
}
ChromeCacheEntry ChromeCache::find(string key) {
	return *get_entry_ptr(key);
}
ChromeCacheEntry* ChromeCache::get_entry_ptr(string key) {
	for (ChromeCacheEntry*entry : entries) {
		if (entry->key == key)return entry;
	}
	throw runtime_error("entry not found.");
}
void ChromeCache::save(string path, ChromeCacheEntry entry) {
	const int data_size = entry.data_addrs.size();
	if (data_size == 0) throw runtime_error("no header and data.");
	if (data_size >= 1) {
		//http header
		HttpHeader header = entry.get_header();
		//char * header_data = get_data(entry.data_addrs[0], entry.data_lengths[0]);
		//HttpHeader header = HttpHeader::load_http_header(header_data, entry.data_lengths[0]);
		//delete[] header_data;
		cout << header.to_string() << flush;
		if (data_size >= 2) {
			//data
			ChromeCacheAddress addr = entry.data_addrs[1];
			if (addr.filetype == EXTERNAL) {
				save_separated_file(path, addr);
			} else {
				uint32_t size = entry.data_lengths[1];
				save_as_file(path, addr, size, header.is_gzipped());
				cout << "size=" << Utils::format_size(size) << endl;
			}
		} else {
			throw runtime_error("no data, header only exists.");
		}
	}
}

void ChromeCache::find_save(string key, string path) {
	save(path, find(key));
}

void ChromeCache::close() {
	blockfiles->close();
}
int ChromeCache::count() {
	return entries.size();
}
stringstream ChromeCache::decompress_gzip(char* data, int size) {
	stringstream ss;
	ss.write(data, size);
	boost::iostreams::filtering_istream filter;
	filter.push(boost::iostreams::gzip_decompressor());
	filter.push(ss);
	stringstream out;
	boost::iostreams::copy(filter, out);
	return out;
}
char* ChromeCache::get_data(ChromeCacheAddress addr, int size) {
	return blockfiles->get_data(addr, size);
}
void ChromeCache::save_as_file(string path, ChromeCacheAddress addr, int size, bool gzipped = false) {
	ofstream ofs(path, ios::binary);
	char* data = get_data(addr, size);
	if (gzipped) {
		stringstream ss = decompress_gzip(data, size);
		ofs << ss.rdbuf();
		cout << "decompressed gzip." << endl;
	} else {
		ofs.write(data, size);
	}
	ofs.close();
	delete[] data;
}
void ChromeCache::save_separated_file(filesystem::path path, ChromeCacheAddress addr) {
	stringstream ss;
	ss << "f_" << setfill('0') << right << setw(6) << hex << addr.num << flush;
	filesystem::path target = cache_dir / ss.str();
	cout << target << " -> " << path << endl;
	if (!filesystem::exists(target) || !filesystem::exists(path.parent_path()))
		throw runtime_error("taget file or destination directory does not exists.");
	try {
		filesystem::copy_file(target, path, filesystem::copy_options::overwrite_existing);
	} catch (filesystem::filesystem_error& e) {
		cout << "copy failed." << endl;
		throw runtime_error("copy failed.");
	}
}
