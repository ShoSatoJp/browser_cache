#include "stdafx.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <filesystem>

template <class T> void endswap(T *objp) {
	unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
	std::reverse(memp, memp + sizeof(T));
}
using namespace std;
struct CacheIndexHeader {
	uint32_t mVersion;
	uint32_t mTimeStamp;
	uint32_t mIsDirty;
	static void read(ifstream* ifs, CacheIndexHeader* header) {
		(*ifs).read((char*)header, sizeof(CacheIndexHeader));
		endswap(&(*header).mVersion);
		endswap(&(*header).mTimeStamp);
		endswap(&(*header).mIsDirty);
	}
};

static const size_t kHashSize = 20;
typedef uint8_t Hash[kHashSize];
typedef uint64_t OriginAttrsHash;

#pragma pack(push, 4)
struct CacheIndexRecord {
	Hash mHash;
	uint32_t mFrecency;
	OriginAttrsHash mOriginAttrsHash;
	uint32_t mExpirationTime;
	uint16_t mOnStartTime;
	uint16_t mOnStopTime;
	uint32_t mFlags;
	string hash_tostring() {
		stringstream str;
		str << hex << uppercase;
		for (int i = 0; i < kHashSize; i++)
			str << setfill('0') << right << setw(2) << (int)mHash[i];
		return string(str.str());
	}
	static void read(ifstream* ifs, CacheIndexRecord* record) {
		(*ifs).read((char*)record, sizeof(CacheIndexRecord));
		endswap(&(*record).mFrecency);
		endswap(&(*record).mOriginAttrsHash);
		endswap(&(*record).mExpirationTime);
		endswap(&(*record).mOnStartTime);
		endswap(&(*record).mOnStopTime);
		endswap(&(*record).mFlags);
	}
};
#pragma pack(pop)
static const int CHUNK_SIZE = 256 * 1024;

struct FirefoxMetaData {
	uint32_t mVersion;
	uint32_t mFetchCount;
	uint32_t mLastFetched;
	uint32_t mLastModified;
	uint32_t mFrecency;
	uint32_t mExpirationTime;
	uint32_t mKeySize;
	uint32_t mFlags;
	static void read(ifstream* ifs, FirefoxMetaData* fmd) {
		(*ifs).read((char*)fmd, sizeof(FirefoxMetaData));
		endswap(&(*fmd).mVersion);
		endswap(&(*fmd).mExpirationTime);
		endswap(&(*fmd).mFetchCount);
		endswap(&(*fmd).mFlags);
		endswap(&(*fmd).mFrecency);
		endswap(&(*fmd).mKeySize);
		endswap(&(*fmd).mLastFetched);
		endswap(&(*fmd).mLastModified);
	}
};

class FirefoxCacheFile {
public:
	FirefoxCacheFile(string path) {
		this->file_path = path;
		ifstream ifs(path, ios::binary);
		//meta start
		ifs.seekg(-4, ios::end);
		this->meta_end = ifs.tellg();
		ifs.read((char*)&meta_start, 4);
		endswap(&meta_start);
		//meta data
		int numHashChunks = (int)ceil((double)meta_start / CHUNK_SIZE);
		ifs.seekg(meta_start + 4 + numHashChunks * 2, ios::beg);
		FirefoxMetaData::read(&ifs, &metadata);
		//key
		char* key = new char[metadata.mKeySize + 1];
		ifs.read(key, metadata.mKeySize + 1);
		string keysrc(key, metadata.mKeySize);
		this->key = keysrc.substr(keysrc.find(':') + 1);
		delete[] key;
		ifs.seekg(1, ios::cur);
		this->map_start = ifs.tellg();
		ifs.close();
	}

	map<string, string> load_map() {
		map<string, string> result;
		ifstream ifs(this->file_path, ios::binary);
		ifs.seekg(map_start, ios::beg);
		vector<string> vec;
		stringstream ss;
		char c;
		while (ifs.read(&c, 1), ifs.tellg() < this->meta_end) {
			if (c != '\0') ss << c;
			else {
				vec.push_back(ss.str());
				ss.str(""); ss.clear();
			}
		}
		ifs.close();
		for (int i = 0; i + 1 < vec.size(); i += 2) {
			result[vec[i]] = vec[i + 1];
		}
		return result;
	}

	void get_data(char** data, int* size) {
		ifstream ifs(this->file_path, ios::binary);
		char* data_ = new char[this->meta_start];
		ifs.read(data_, this->meta_start);
		ifs.close();
		*size = this->meta_start;
		*data = data_;
	}

	void write_data_as_file(string path) {
		int size; char* data = nullptr;
		this->get_data(&data, &size);
		//cout << string(data, size) << endl;
		ofstream ofs(path, ios::binary);
		ofs.write(data, size);
		ofs.close();
		delete[] data;
	}
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
	FirefoxCacheIndex(string path) {
		ifstream ifs(path, ios::binary);
		CacheIndexHeader::read(&ifs, &header);
		//cout << "after header: " << ifs.tellg() << endl;
		while (!ifs.eof()) {
			CacheIndexRecord cir;
			CacheIndexRecord::read(&ifs, &cir);
			records.push_back(cir);
			//cout << "after records: " << ifs.tellg() << endl;
		}
	}
	CacheIndexHeader header;
	vector<CacheIndexRecord> records;
};
class HttpHeader {
public:
	HttpHeader(string src) {
		vector<string> result;
		boost::algorithm::split(result, src, boost::is_any_of("\n"));
		//status
		status_source = boost::algorithm::trim_copy<string>(result.front());
		vector<string> status_temp;
		boost::algorithm::split(status_temp, status_source, boost::is_any_of(" "));
		if (status_temp.size() == 3) {
			this->protocol = status_temp[0];
			this->status_code = stoi(status_temp[1]);
			this->status_message = status_temp[2];
		}
		result.erase(result.begin());
		//headers
		for (string s : result) {
			vector<string> header;
			boost::algorithm::split(header, s, boost::is_any_of(":"));
			if (header.size() == 2)
				headers[boost::algorithm::trim_copy(header[0])] = boost::algorithm::trim_copy(header[1]);
		}
		;
	}
	int status_code;
	string status_source;
	string status_message;
	string protocol;
	map<string, string> headers;
};

class FirefoxCache {
public:
	FirefoxCache(filesystem::path cache2_dir, bool use_index = true) {
		filesystem::path index_file_path = cache2_dir / "index";
		filesystem::path cache_entry_dir = cache2_dir / "entries";
		if (use_index) {
			index = FirefoxCacheIndex(index_file_path.string());
			for (int i = 0, length = index.records.size() - 1; i < length; i++) {
				FirefoxCacheFile ff((cache_entry_dir / index.records[i].hash_tostring()).string());
				records.push_back(ff);
			}
		} else {
			for (const auto & entry : filesystem::directory_iterator(cache_entry_dir)) {
				records.push_back(FirefoxCacheFile(entry.path().string()));
			}
		}
	}
	FirefoxCacheFile find_by_key(string key) {
		for (FirefoxCacheFile e : records) {
			if (e.key == key)return e;
		}
		throw exception();
	}
	FirefoxCacheIndex index;
	vector<FirefoxCacheFile> records;
};

int main() {

	filesystem::path dir(R"(C:\Users\User\AppData\Local\Mozilla\Firefox\Profiles\qht8q8ei.default\cache2\)");
	while (true) {
		string key, path;
		cin >> key >> path;
		FirefoxCache index(dir, false);
		if (key == "list") {
			for (FirefoxCacheFile f : index.records) {
				cout << f.key << endl;
			}
			cout << index.records.size() << endl;
			continue;
		}
		try {
			FirefoxCacheFile ff = index.find_by_key(key);
			ff.write_data_as_file(path);
			cout << ff.file_path << endl;
		} catch (const std::exception&) {
			cout << "not found" << endl;
		}
	}
	return 0;
}

