#pragma once
#include "stdafx.h"

using namespace std;

static const int CHUNK_SIZE = 256 * 1024;
static const size_t kHashSize = 20;
typedef uint8_t Hash[kHashSize];
typedef uint64_t OriginAttrsHash;

template <class T> void endswap(T *objp) {
	unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
	std::reverse(memp, memp + sizeof(T));
}
class HttpHeader {
public:
	HttpHeader() {}
	HttpHeader(string src);
	int status_code;
	string status_source;
	string status_message;
	string protocol;
	map<string, string> headers;
};
struct CacheIndexHeader {
	uint32_t mVersion;
	uint32_t mTimeStamp;
	uint32_t mIsDirty;
	static void read(ifstream* ifs, CacheIndexHeader* header);
};
#pragma pack(push, 4)
struct CacheIndexRecord {
	Hash mHash;
	uint32_t mFrecency;
	OriginAttrsHash mOriginAttrsHash;
	uint32_t mExpirationTime;
	uint16_t mOnStartTime;
	uint16_t mOnStopTime;
	uint32_t mFlags;
	string hash_tostring();
	static void read(ifstream* ifs, CacheIndexRecord* record);
};
#pragma pack(pop)
struct FirefoxMetaData {
	uint32_t mVersion;
	uint32_t mFetchCount;
	uint32_t mLastFetched;
	uint32_t mLastModified;
	uint32_t mFrecency;
	uint32_t mExpirationTime;
	uint32_t mKeySize;
	uint32_t mFlags;
	static void read(ifstream* ifs, FirefoxMetaData* fmd);
};
class FirefoxCacheEntry {
public:
	FirefoxCacheEntry(string path);
	map<string, string> load_map();
	void get_data(char** data, int* size);
	void save(string path);
	HttpHeader get_header();
	//private:
	string file_path;
	int meta_start;
	int meta_end;
	int map_start;
	FirefoxMetaData metadata;
	string key;
};
class FirefoxCacheIndex {
public:
	FirefoxCacheIndex() {};
	FirefoxCacheIndex(string path);
	CacheIndexHeader header;
	vector<CacheIndexRecord> records;
};
class FirefoxCache {
public:
	FirefoxCache(string cache2_dir, bool use_index = false)
		:FirefoxCache(filesystem::path(cache2_dir), use_index) {}
	FirefoxCache(filesystem::path cache2_dir, bool use_index = false);
	FirefoxCacheEntry find(string key);
	FirefoxCacheIndex index;
	vector<string> keys();
	void find_save(string key, string path);
	vector<FirefoxCacheEntry> records;
};
