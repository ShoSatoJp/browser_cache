#pragma once
#include "stdafx.hpp"
#include "structs.hpp"
#include "addr.hpp"
#include <experimental/filesystem>

using namespace std;
using namespace std::experimental;

class Utils {
public:
	static string format_size(uint64_t size) {
		vector<string> units = { "B", "KB","MB", "GB" };
		for (string unit : units) {
			if (size < 1024)return to_string(size) + unit;
			size /= 1024;
		}
		return to_string(size * 1024) + units.back();
	}
	static void copy_files(const string &dir, const string &dest) {
		if (!experimental::filesystem::exists(dest) && !experimental::filesystem::create_directories(dest)) {
			throw runtime_error("cannot create directory:" + dest);
		}
		for (string file : files) {
			experimental::filesystem::copy_file(experimental::filesystem::path(dir) / file, experimental::filesystem::path(dest) / file, experimental::filesystem::copy_options::overwrite_existing);
		}
	}
	static const vector<string> files;
	static vector<string> split(const string& str, const char &delim) {
		vector<string> res;
		int p = 0;
		for (int i = 0, len = str.size(); i < len; i++) {
			if (str[i] == delim) {
				res.push_back(str.substr(p, i - p));
				p = i + 1;
			}
		}
		res.push_back(str.substr(p));
		return res;
	}
	static string trim(const string& str, const string& list = "\n\r\t ") {
		int start = str.find_first_not_of(list);
		if (start == -1)return "";
		int end = str.find_last_not_of(list);
		return str.substr(start, end - start + 1);
	}
};

class HttpHeader {
public:
	HttpHeader() {}
	int status_code = 0;
	string status_source;
	string protocol;
	map<string, string> headers;
	const string& to_string();
	bool is_gzipped();
	static unique_ptr<HttpHeader> load_http_header(char*, int);
};

class ChromeCacheAddress {
public:
	ChromeCacheAddress(uint32_t);
	const string & tostring();
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


class ChromeCache;

class ChromeCacheEntry {
public:
	ChromeCacheEntry() {}
	ChromeCacheEntry(EntryStore*, const string&, ChromeCache *cc);
	EntryStore *es;
	string key;
	vector<uint32_t> data_lengths;
	vector<ChromeCacheAddress> data_addrs;
	int data_count;
	unique_ptr<HttpHeader> get_header()const;
	ChromeCache * cc = nullptr;
	void save(const string &path);
private:
	//static void operator delete(void *ptr);
};
class ChromeCacheBlockFile {
public:
	ChromeCacheBlockFile() {}
	ChromeCacheBlockFile(FileType, const experimental::filesystem::path&, ChromeCache * cc);
	char* get_data(const ChromeCacheAddress&, int);
	void write_as_file(const string&, const ChromeCacheAddress&, int);
	ChromeCacheEntry* get_entry(const ChromeCacheAddress&);
	void close();
	BlockFileHeader header;
private:
	ChromeCache * cc = nullptr;
	FileType filetype;
	ifstream ifs;
	uint32_t data_start;
	static int BlockSizeForFileType(FileType);
};

class ChromeCacheBlockFiles {
public:
	ChromeCacheBlockFiles(const experimental::filesystem::path&, ChromeCache * cc);
	ChromeCacheEntry * get_entry(const ChromeCacheAddress&);
	char* get_data(const ChromeCacheAddress&, int);
	void close();
private:
	ChromeCache * cc = nullptr;
	vector<ChromeCacheBlockFile> blockfiles;
};

class ChromeCache {
public:
	ChromeCache() {}
	ChromeCache(const string &cache_dir, const string& temp_cache_dir, bool update_index = true)
		:ChromeCache(experimental::filesystem::path(cache_dir), experimental::filesystem::path(temp_cache_dir), update_index) {}
	ChromeCache(const experimental::filesystem::path& cache_dir, const experimental::filesystem::path &temp_cache_dir, bool update_index);
	void show_keys();
	vector<string> keys();
	unique_ptr<ChromeCacheEntry> get_entry(int i);
	ChromeCacheEntry find(const string &key);
	ChromeCacheEntry find_map(const string &key);
	ChromeCacheEntry* find_ptr(const string &key);
	ChromeCacheEntry* find_map_ptr(const string &key);
	void save(const string&, const ChromeCacheEntry&);
	void find_save(const string& key, const string &path);
	void close();
	int count();
	char* get_data(const ChromeCacheAddress&, int);
private:
	//const string & decompress_gzip(char*, int);
	void save_as_file(const string&, const ChromeCacheAddress&, int, bool);
	void save_separated_file(const experimental::filesystem::path&, const ChromeCacheAddress&);
	experimental::filesystem::path cache_dir;
	experimental::filesystem::path temp_cache_dir;
	IndexHeader header;
	vector<ChromeCacheAddress> addrs;
	ChromeCacheBlockFiles *blockfiles = nullptr;
	vector<ChromeCacheEntry*> entries;
	map<string, ChromeCacheEntry*> entries_map;
};
