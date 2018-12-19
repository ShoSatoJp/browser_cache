#pragma once
#include <stdio.h>
#include <tchar.h>
//#include <iostream>
//#include <fstream>
//#include <bitset>
//#include <vector>
//#include <string>
//#include <sstream>
//#include <filesystem>
//#include <map>

const uint32_t kBlockVersion2 = 0x20000;        // Version 2.0.
const uint32_t kBlockCurrentVersion = 0x30000;  // Version 3.0.
const uint32_t kBlockMagic = 0xC104CAC3;
const int kBlockHeaderSize = 8192;  // Two pages: almost 64k entries
const int kMaxBlocks = (kBlockHeaderSize - 80) * 8;
const int kNumExtraBlocks = 1024;  // How fast files grow.
typedef uint32_t AllocBitmap[kMaxBlocks / 32];
typedef uint32_t CacheAddr;
const int kIndexTablesize = 0x10000;
const uint32_t kIndexMagic = 0xC103CAC3;
const uint32_t kCurrentVersion = 0x20000;  // Version 2.0.
const int kNumSparseBits = 1024;
// Header for the master index file.
struct LruData {
	int32_t pad1[2];
	int32_t filled;  // Flag to tell when we filled the cache.
	int32_t sizes[5];
	CacheAddr heads[5];
	CacheAddr tails[5];
	CacheAddr transaction;     // In-flight operation target.
	int32_t operation;         // Actual in-flight operation.
	int32_t operation_list;    // In-flight operation list.
	int32_t pad2[7];
};
struct IndexHeader {
	uint32_t magic;
	uint32_t version;
	int32_t num_entries;       // Number of entries currently stored.
	int32_t num_bytes;         // Total size of the stored data.
	int32_t last_file;         // Last external file created.
	int32_t this_id;           // Id for all entries being changed (dirty flag).
	CacheAddr   stats;         // Storage for usage data.
	int32_t table_len;         // Actual size of the table (0 == kIndexTablesize).
	int32_t crash;             // Signals a previous crash.
	int32_t experiment;        // Id of an ongoing test.
	uint64_t create_time;      // Creation time for this set of files.
	int32_t pad[52];
	LruData     lru;           // Eviction control data.
};
struct Index {
	IndexHeader header;
	CacheAddr   table[kIndexTablesize];  // Default size. Actual size controlled
										 // by header.table_len.
};
struct EntryStore {
	uint32_t hash;                  // Full hash of the key.
	CacheAddr   next;               // Next entry with the same hash or bucket.
	CacheAddr   rankings_node;      // Rankings node for this entry.
	int32_t reuse_count;            // How often is this entry used.
	int32_t refetch_count;          // How often is this fetched from the net.
	int32_t state;                  // Current state.
	uint64_t creation_time;
	int32_t key_len;
	CacheAddr   long_key;           // Optional address of a long key.
	int32_t data_size[4];           // We can store up to 4 data streams for each
	CacheAddr   data_addr[4];       // entry.
	uint32_t flags;                 // Any combination of EntryFlags.
	int32_t pad[4];
	uint32_t self_hash;             // The hash of EntryStore up to this point.
	//char key[256 - 24 * 4];  // null terminated
};
enum EntryState {
	ENTRY_NORMAL = 0,
	ENTRY_EVICTED,    // The entry was recently evicted from the cache.
	ENTRY_DOOMED      // The entry was doomed.
};
enum EntryFlags {
	PARENT_ENTRY = 1,         // This entry has children (sparse) entries.
	CHILD_ENTRY = 1 << 1      // Child entry that stores sparse data.
};
struct RankingsNode {
	uint64_t last_used;           // LRU info.
	uint64_t last_modified;       // LRU info.
	CacheAddr   next;             // LRU list.
	CacheAddr   prev;             // LRU list.
	CacheAddr   contents;         // Address of the EntryStore.
	int32_t dirty;                // The entry is being modifyied.
	uint32_t self_hash;           // RankingsNode's hash.
};
struct BlockFileHeader {
	uint32_t magic;
	uint32_t version;
	int16_t this_file;          // Index of this file.
	int16_t next_file;          // Next file when this one is full.
	int32_t entry_size;         // Size of the blocks of this file.
	int32_t num_entries;        // Number of stored entries.
	int32_t max_entries;        // Current maximum number of entries.
	int32_t empty[4];           // Counters of empty entries for each type.
	int32_t hints[4];           // Last used position for each entry type.
	volatile int32_t updating;  // Keep track of updates to the header.
	int32_t user[5];
	AllocBitmap     allocation_map;
};
struct SparseHeader {
	int64_t signature;       // The parent and children signature.
	uint32_t magic;          // Structure identifier (equal to kIndexMagic).
	int32_t parent_key_len;  // Key length for the parent entry.
	int32_t last_block;      // Index of the last written block.
	int32_t last_block_len;  // Length of the last written block.
	int32_t dummy[10];
};
