#pragma once
#include <stdio.h>
#include <tchar.h>

const uint32_t kBlockVersion2 = 0x20000;
const uint32_t kBlockCurrentVersion = 0x30000;
const uint32_t kBlockMagic = 0xC104CAC3;
const int kBlockHeaderSize = 8192;
const int kMaxBlocks = (kBlockHeaderSize - 80) * 8;
const int kNumExtraBlocks = 1024;
typedef uint32_t AllocBitmap[kMaxBlocks / 32];
typedef uint32_t CacheAddr;
const int kIndexTablesize = 0x10000;
const uint32_t kIndexMagic = 0xC103CAC3;
const uint32_t kCurrentVersion = 0x20000;
const int kNumSparseBits = 1024;
struct LruData {
	int32_t pad1[2];
	int32_t filled;
	int32_t sizes[5];
	CacheAddr heads[5];
	CacheAddr tails[5];
	CacheAddr transaction;
	int32_t operation;
	int32_t operation_list;
	int32_t pad2[7];
};
struct IndexHeader {
	uint32_t magic;
	uint32_t version;
	int32_t num_entries;
	int32_t num_bytes;
	int32_t last_file;
	int32_t this_id;
	CacheAddr   stats;
	int32_t table_len;
	int32_t crash;
	int32_t experiment;
	uint64_t create_time;
	int32_t pad[52];
	LruData     lru;
};
struct Index {
	IndexHeader header;
	CacheAddr   table[kIndexTablesize];

};
struct EntryStore {
	uint32_t hash;
	CacheAddr   next;
	CacheAddr   rankings_node;
	int32_t reuse_count;
	int32_t refetch_count;
	int32_t state;
	uint64_t creation_time;
	int32_t key_len;
	CacheAddr   long_key;
	int32_t data_size[4];
	CacheAddr   data_addr[4];
	uint32_t flags;
	int32_t pad[4];
	uint32_t self_hash;
	//char key[256 - 24 * 4];
};
enum EntryState {
	ENTRY_NORMAL = 0,
	ENTRY_EVICTED,
	ENTRY_DOOMED
};
enum EntryFlags {
	PARENT_ENTRY = 1,
	CHILD_ENTRY = 1 << 1
};
struct RankingsNode {
	uint64_t last_used;
	uint64_t last_modified;
	CacheAddr   next;
	CacheAddr   prev;
	CacheAddr   contents;
	int32_t dirty;
	uint32_t self_hash;
};
struct BlockFileHeader {
	uint32_t magic;
	uint32_t version;
	int16_t this_file;
	int16_t next_file;
	int32_t entry_size;
	int32_t num_entries;
	int32_t max_entries;
	int32_t empty[4];
	int32_t hints[4];
	volatile int32_t updating;
	int32_t user[5];
	AllocBitmap     allocation_map;
};
struct SparseHeader {
	int64_t signature;
	uint32_t magic;
	int32_t parent_key_len;
	int32_t last_block;
	int32_t last_block_len;
	int32_t dummy[10];
};
