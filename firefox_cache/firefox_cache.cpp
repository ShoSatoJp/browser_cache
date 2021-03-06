#include "stdafx.h"
#include "firefox_cache.hpp"


//CacheIndexHeader
void CacheIndexHeader::read(ifstream* ifs, CacheIndexHeader* header) {
	(*ifs).read((char*)header, sizeof(CacheIndexHeader));
	endswap(&(*header).mVersion);
	endswap(&(*header).mTimeStamp);
	endswap(&(*header).mIsDirty);
}

//CacheIndexRecord
string CacheIndexRecord::hash_tostring() {
	stringstream str;
	str << hex << uppercase;
	for (int i = 0; i < kHashSize; i++)
		str << setfill('0') << right << setw(2) << (int)mHash[i];
	return string(str.str());
}
void CacheIndexRecord::read(ifstream* ifs, CacheIndexRecord* record) {
	(*ifs).read((char*)record, sizeof(CacheIndexRecord));
	endswap(&(*record).mFrecency);
	endswap(&(*record).mOriginAttrsHash);
	endswap(&(*record).mExpirationTime);
	endswap(&(*record).mOnStartTime);
	endswap(&(*record).mOnStopTime);
	endswap(&(*record).mFlags);
}

//FirefoxMetaData
void FirefoxMetaData::read(ifstream* ifs, FirefoxMetaData* fmd) {
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

//FirefoxCacheEntry
FirefoxCacheEntry::FirefoxCacheEntry(string path) {
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
map<string, string> FirefoxCacheEntry::load_map() {
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
void FirefoxCacheEntry::get_data(char** data, int* size) {
	ifstream ifs(this->file_path, ios::binary);
	char* data_ = new char[this->meta_start];
	ifs.read(data_, this->meta_start);
	ifs.close();
	*size = this->meta_start;
	*data = data_;
}
void FirefoxCacheEntry::save(string path) {
	int size; char* data = nullptr;
	this->get_data(&data, &size);
	ofstream ofs(path, ios::binary);
	ofs.write(data, size);
	ofs.close();
	delete[] data;
}
HttpHeader FirefoxCacheEntry::get_header() {
	return HttpHeader(load_map()["response-head"]);
}
//FirefoxCacheIndex
FirefoxCacheIndex::FirefoxCacheIndex(string path) {
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

//HttpHeader
HttpHeader::HttpHeader(string src) {
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
}

//FirefoxCache
FirefoxCache::FirefoxCache(filesystem::path cache2_dir, bool use_index) {
	filesystem::path index_file_path = cache2_dir / "index";
	filesystem::path cache_entry_dir = cache2_dir / "entries";
	if (use_index) {
		index = FirefoxCacheIndex(index_file_path.string());
		for (int i = 0, length = index.records.size() - 1; i < length; i++) {
			FirefoxCacheEntry ff((cache_entry_dir / index.records[i].hash_tostring()).string());
			records.push_back(ff);
		}
	} else {
		for (const auto & entry : filesystem::directory_iterator(cache_entry_dir)) {
			records.push_back(FirefoxCacheEntry(entry.path().string()));
		}
	}
}
FirefoxCacheEntry FirefoxCache::find(string key) {
	for (FirefoxCacheEntry e : records) {
		if (e.key == key)return e;
	}
	throw exception();
}
void FirefoxCache::find_save(string key, string path) {
	FirefoxCacheEntry ff = this->find(key);
	ff.save(path);
}
vector<string> FirefoxCache::keys() {
	vector<string> keys;
	for (FirefoxCacheEntry ff : this->records) {
		keys.push_back(ff.key);
	}
	return keys;
}