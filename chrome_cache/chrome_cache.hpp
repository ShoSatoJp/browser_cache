#pragma once
#include "stdafx.hpp"
#include "structs.hpp"
#include "addr.hpp"

using namespace std;

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
	static void copy_files(string dir, string dest) {
		if (!filesystem::exists(dest) && !filesystem::create_directories(dest)) {
			throw runtime_error("cannot create directory:" + dest);
		}
		for (string file : files) {
			filesystem::copy_file(filesystem::path(dir) / file, filesystem::path(dest) / file, filesystem::copy_options::overwrite_existing);
		}
	}
	//static void delete_files(string dest) {
	//	for (string file : files) filesystem::remove(filesystem::path(dest) / file);

	//}
	static const vector<string> files;
};

class HttpHeader {
public:
	HttpHeader() {}
	int status_code;
	string status_source;
	string protocol;
	map<string, string> headers;
	string to_string();
	bool is_gzipped();
	static HttpHeader *load_http_header(char*, int);
};

class ChromeCacheAddress {
public:
	ChromeCacheAddress(uint32_t);
	string tostring();
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
	ChromeCacheEntry(EntryStore*, string);
	EntryStore *es;
	string key;
	vector<uint32_t> data_lengths;
	vector<ChromeCacheAddress> data_addrs;
	int data_count;
};
class ChromeCacheBlockFile {
public:
	ChromeCacheBlockFile() {}
	ChromeCacheBlockFile(FileType, filesystem::path);
	char* get_data(ChromeCacheAddress, int);
	void write_as_file(string, ChromeCacheAddress, int);
	ChromeCacheEntry* get_entry(ChromeCacheAddress);
	void close();
	BlockFileHeader header;
private:
	FileType filetype;
	ifstream ifs;
	uint32_t data_start;
	static int BlockSizeForFileType(FileType);
};

class ChromeCacheBlockFiles {
public:
	ChromeCacheBlockFiles(filesystem::path);
	ChromeCacheEntry * get_entry(ChromeCacheAddress);
	char* get_data(ChromeCacheAddress, int);
	void close();
private:
	vector<ChromeCacheBlockFile> blockfiles;
};

class ChromeCache {
public:
	ChromeCache() {}
	ChromeCache(filesystem::path, filesystem::path);
	void show_keys();
	ChromeCacheEntry * get_entry(int);
	ChromeCacheEntry * get_entry(string);
	void save(string, ChromeCacheEntry*);
	void find_save(string key, string path);
	void close();
	int count();
private:
	stringstream decompress_gzip(char*, int);
	char* get_data(ChromeCacheAddress, int);
	void save_as_file(string, ChromeCacheAddress, int, bool);
	void save_separated_file(filesystem::path, ChromeCacheAddress);
	filesystem::path cache_dir;
	filesystem::path temp_cache_dir;
	IndexHeader header;
	vector<ChromeCacheAddress> addrs;
	ChromeCacheBlockFiles *blockfiles = nullptr;
	vector<ChromeCacheEntry*> entries;
};
