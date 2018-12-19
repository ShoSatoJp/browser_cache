#pragma once
//#include "stdafx.h"
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

#include "structs.hpp"

enum FileType {
	EXTERNAL = 0,
	RANKINGS = 1,
	BLOCK_256 = 2,
	BLOCK_1K = 3,
	BLOCK_4K = 4,
	BLOCK_FILES = 5,
	BLOCK_ENTRIES = 6,
	BLOCK_EVICTED = 7
};

const int kMaxBlockSize = 4096 * 4;
const int16_t kMaxBlockFile = 255;
const int kMaxNumBlocks = 4;
const int16_t kFirstAdditionalBlockFile = 4;

class Addr {
public:
	Addr() : value_(0) {}
	explicit Addr(CacheAddr address) : value_(address) {}
	Addr(FileType file_type, int max_blocks, int block_file, int index) {
		value_ = ((file_type << kFileTypeOffset) & kFileTypeMask) |
			(((max_blocks - 1) << kNumBlocksOffset) & kNumBlocksMask) |
			((block_file << kFileSelectorOffset) & kFileSelectorMask) |
			(index  & kStartBlockMask) | kInitializedMask;
	}

	CacheAddr value() const { return value_; }
	void set_value(CacheAddr address) {
		value_ = address;
	}

	bool is_initialized() const {
		return (value_ & kInitializedMask) != 0;
	}

	bool is_separate_file() const {
		return (value_ & kFileTypeMask) == 0;
	}

	bool is_block_file() const {
		return !is_separate_file();
	}

	FileType file_type() const {
		return static_cast<FileType>((value_ & kFileTypeMask) >> kFileTypeOffset);
	}

	int FileNumber() const {
		if (is_separate_file())
			return value_ & kFileNameMask;
		else
			return ((value_ & kFileSelectorMask) >> kFileSelectorOffset);
	}

	int num_blocks() const {
		return ((value_ & kNumBlocksMask) >> kNumBlocksOffset);
	}

	//int start_block() const;
	//int num_blocks() const;
	//bool SetFileNumber(int file_number);
	int block_size() const {
		return BlockSizeForFileType(file_type());
	}

	int start_block() const {
		return (value_ & kStartBlockMask);
	}

	bool operator==(Addr other) const {
		return value_ == other.value_;
	}

	bool operator!=(Addr other) const {
		return value_ != other.value_;
	}

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

	static FileType RequiredFileType(int size) {
		if (size < 1024)
			return BLOCK_256;
		else if (size < 4096)
			return BLOCK_1K;
		else if (size <= 4096 * 4)
			return BLOCK_4K;
		else
			return EXTERNAL;
	}

	static int RequiredBlocks(int size, FileType file_type) {
		int block_size = BlockSizeForFileType(file_type);
		return (size + block_size - 1) / block_size;
	}

	// Returns true if this address looks like a valid one.
	//bool SanityCheck() const;
	//bool SanityCheckForEntry() const;
	//bool SanityCheckForRankings() const;

private:
	uint32_t reserved_bits() const { return value_ & kReservedBitsMask; }

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

	CacheAddr value_;
};