#pragma once

#include <Base/Always.h>

#include <PKey.h>

#include <string>
#include <vector>

struct PhobosMixHeaderData
{
	DWORD ID;
	DWORD Offset;
	DWORD Size;
};

struct PhobosMixPeekHeader
{
	short FileCount;  // nonzero = legacy format (IS the file count)
	short FormatFlags; // bit 0 = HasDigest, bit 1 = IsEncrypted
};
static_assert(sizeof(PhobosMixPeekHeader) == 0x4, "Size Miss Match !");

#pragma pack (push, 1)
struct PhobosMixFileHeader {
	short Count;    // number of files
	union {
		int   DataSize;      // read as whole int after full 6-byte read
		struct {
			short DataSizeLo; // low word — overlaps with MixPeekHeader copy
			short DataSizeHi; // high word — filled by second Get() in legacy path
		};
	};

	// Populate from a peek header (legacy format — DataSizeHi still needs a second read)
	COMPILETIMEEVAL PhobosMixFileHeader& operator=(const PhobosMixPeekHeader& peek) noexcept {
		Count = peek.FileCount;
		DataSizeLo = peek.FormatFlags; // low word of DataSize happens to alias FormatFlags slot
		DataSizeHi = 0;
		return *this;
	}

};
#pragma pack (pop)

class RawFileClass;
class PhobosMixFileClass
{
public:

	static std::vector<PhobosMixFileClass*> Array;

public:

	PhobosMixFileClass(RawFileClass* pFile, PKey* pKey);
	~PhobosMixFileClass();

	static bool Offset(const char* pFilename, void** realptr = nullptr, PhobosMixFileClass** mixfile = nullptr, int* offset = nullptr, int* size = nullptr);
	static PhobosMixHeaderData* BinarySearchHeaders(std::vector<PhobosMixHeaderData>* headers, int crc);

public:

	std::string Filename;
	bool IsDigest;
	bool IsEncrypted;
	bool IsAllocated;
	int DataSize;
	int DataStart;
	std::vector<PhobosMixHeaderData> Headers;
	void* Data;
};