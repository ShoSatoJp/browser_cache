#pragma once
#include "stdafx.h"

string format_size(uint64_t size) {
	vector<string> units = { "B", "KB","MB", "GB" };
	for (string unit : units) {
		if (size < 1024)return to_string(size) + unit;
		size /= 1024;
	}
	return to_string(size * 1024) + units.back();
}

class HttpHeader {
public:
	HttpHeader() {}
	int status_code;
	string status_source;
	string protocol;
	map<string, string> headers;
	string to_string() {
		stringstream ss;
		ss << status_source << '\n';
		for (pair<string, string> p : headers) {
			ss << setw(20) << right << p.first << " : " << left << p.second << '\n';
		}
		return ss.str();
	}
	bool is_gzipped() {
		return headers.count("content-encoding") != 0 && headers["content-encoding"] == "gzip";
	}
private:
	vector<string> text_mime_type = { "text/plain","text/css","text/csv","text/html","application/javascript","application/json","application/xml" };
};

HttpHeader *load_http_header(char* data, int size) {
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
	HttpHeader *header = new HttpHeader();
	string status_source = src.front();
	header->status_source = status_source;
	vector<string> status_re;
	boost::algorithm::split(status_re, status_source, boost::is_any_of(" "));
	if (status_re.size() == 2) {
		header->protocol = status_re[0];
		header->status_code = stoi(status_re[1]);
	}
	src.erase(src.begin());
	for (string str : src) {
		transform(str.begin(), str.end(), str.begin(), ::tolower);
		vector<string> re;
		boost::algorithm::split(re, str, boost::is_any_of(":"));
		if (re.size() == 2) {
			header->headers[re[0]] = re[1];
		}
	}
	return header;
}

class ChromeCacheAddress {
public:
	ChromeCacheAddress(uint32_t addr) {
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
	string tostring() {
		stringstream ss;
		ss << "initialized:" << initialized << '\n'
			<< "filetype:" << filetype << '\n'
			<< "num:" << num << '\n'
			<< "block_length:" << block_length << '\n'
			<< "file_selector:" << file_selector << '\n';
		return ss.str();
	}
	bool initialized;
	FileType filetype;
	uint32_t num;
	int block_length = 0;
	int file_selector = 0;
private:
	static const uint32_t kInitializedMask = 0x80000000;
	static const uint32_t kFileTypeMask = 0x70000000;
	static const uint32_t kFileTypeOffset = 28;
	static const uint32_t kReservedBitsMask = 0x0c000000;
	static const uint32_t kNumBlocksMask = 0x03000000;
	static const uint32_t kNumBlocksOffset = 24;
	static const uint32_t kFileSelectorMask = 0x00ff0000;
	static const uint32_t kFileSelectorOffset = 16;
	static const uint32_t kStartBlockMask = 0x0000FFFF;
	static const uint32_t kFileNameMask = 0x0FFFFFFF;
};
class ChromeCacheEntry {
public:
	ChromeCacheEntry(EntryStore* es, string key) :es(es), key(key) {
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
	EntryStore *es;
	string key;
	vector<uint32_t> data_lengths;
	vector<ChromeCacheAddress> data_addrs;
	int data_count;

};
class ChromeCacheBlockFile {
public:
	ChromeCacheBlockFile() {}
	ChromeCacheBlockFile(FileType filetype, filesystem::path cache_dir) {
		this->filetype = filetype;
		int filenum = filetype - 1;
		ifs = ifstream(cache_dir / ("data_" + to_string(filenum)), ios::binary | ios::in | ios::out);
		if (!ifs)throw exception(string("cannot open data_" + to_string(filenum) + " file").c_str());
		//cout << !!ifs << endl;
		ifs.read((char*)&header, sizeof(BlockFileHeader));
		data_start = ifs.tellg();
	}

	char* get_data(ChromeCacheAddress addr, int size) {
		ifs.clear();
		ifs.seekg(data_start + BlockSizeForFileType(addr.filetype)*addr.num, ios::beg);
		char* data_ = new char[size];
		//cout << ifs.tellg() << endl;
		ifs.read(data_, size);
		return data_;
	}

	void write_as_file(string path, ChromeCacheAddress addr, int size) {
		ofstream ofs(path, ios::binary);
		char* data = get_data(addr, size);
		ofs.write(data, size);
		ofs.close();
		delete[] data;
	}

	ChromeCacheEntry* get_entry(ChromeCacheAddress addr) {
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
			return new ChromeCacheEntry(es, key);
		}
	}
	void close() {
		ifs.close();
	}

	BlockFileHeader header;
private:
	FileType filetype;
	ifstream ifs;
	uint32_t data_start;
	static int BlockSizeForFileType(FileType file_type) {
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
};

class ChromeCacheBlockFiles {
public:
	ChromeCacheBlockFiles(filesystem::path cache_dir) {
		for (int i = 0; i < 4; i++) {
			blockfiles.push_back(ChromeCacheBlockFile((FileType)(i + 1), cache_dir));
		}
	}

	ChromeCacheEntry * get_entry(ChromeCacheAddress addr) {
		return blockfiles[addr.filetype - 1].get_entry(addr);
	}

	char* get_data(ChromeCacheAddress addr, int size) {
		return blockfiles[addr.filetype - 1].get_data(addr, size);
	}

	void close() {
		for (int i = 0; i < 4; i++) {
			blockfiles[i].close();
		}
	}
private:
	vector<ChromeCacheBlockFile> blockfiles;
};

class ChromeCache {
public:
	ChromeCache() {}
	//ChromeCache(string cache_dir) :ChromeCache(filesystem::path(cache_dir)) {}
	ChromeCache(filesystem::path cache_dir) {
		this->cache_dir = cache_dir;
		blockfiles = new ChromeCacheBlockFiles(cache_dir);
		//header
		ifstream ifs(cache_dir / "index", ios::binary | ios::in);
		if (!ifs)throw exception("cannot open index file.");
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
	void show_keys() {
		vector<string> keys;
		for (ChromeCacheEntry * entry : entries) {
			keys.push_back(entry->key);
		}
		sort(keys.begin(), keys.end());
		for (string key : keys) {
			cout << key << endl;
		}
	}
	ChromeCacheEntry * get_entry(int i) {
		return entries[i];
	}
	ChromeCacheEntry * get_entry(string key) {
		for (ChromeCacheEntry*entry : entries) {
			if (entry->key == key)return entry;
		}
		return nullptr;
	}
	void save(string path, ChromeCacheEntry* entry) {
		if (entry->data_addrs.size() >= 2) {
			//http header
			char * header_data = get_data(entry->data_addrs[0], entry->data_lengths[0]);
			HttpHeader * header = load_http_header(header_data, entry->data_lengths[0]);
			delete[] header_data;
			cout << header->to_string() << flush;
			//data
			ChromeCacheAddress addr = entry->data_addrs[1];
			if (addr.filetype == EXTERNAL) {
				save_separated_file(path, addr);
			} else {
				uint32_t size = entry->data_lengths[1];
				save_as_file(path, addr, size, header->is_gzipped());
				cout << "size=" << format_size(size) << endl;
			}
		}
	}
	void close() {
		blockfiles->close();
	}
	int count() {
		return entries.size();
	}
private:
	stringstream decompress_gzip(char* data, int size) {
		stringstream ss;
		ss.write(data, size);
		boost::iostreams::filtering_istream filter;
		filter.push(boost::iostreams::gzip_decompressor());
		filter.push(ss);
		stringstream out;
		boost::iostreams::copy(filter, out);
		return out;
	}

	char* get_data(ChromeCacheAddress addr, int size) {
		return blockfiles->get_data(addr, size);
	}
	void save_as_file(string path, ChromeCacheAddress addr, int size, bool gzipped = false) {
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
	void save_separated_file(string path, ChromeCacheAddress addr) {
		stringstream ss;
		ss << "f_" << setfill('0') << right << setw(6) << hex << addr.num << flush;
		filesystem::copy_file(cache_dir / ss.str(), path);
	}
	filesystem::path cache_dir;
	IndexHeader header;
	vector<ChromeCacheAddress> addrs;
	ChromeCacheBlockFiles *blockfiles = nullptr;
	vector<ChromeCacheEntry*> entries;
};
