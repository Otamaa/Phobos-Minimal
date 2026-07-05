#pragma once

#include <GenericList.h>
#include <ArrayClasses.h>
#include <Helpers/CompileTime.h>
#include <PKey.h>

class CCFileClass;
struct MixHeaderData
{
	DWORD ID;
	DWORD Offset;
	DWORD Size;
};

struct MixPeekHeader
{
	short FileCount;  // nonzero = legacy format (IS the file count)
	short FormatFlags; // bit 0 = HasDigest, bit 1 = IsEncrypted
};
static_assert(sizeof(MixPeekHeader) == 0x4, "Size Miss Match !");

#pragma pack (push, 1)
struct MixFileHeader {
	short Count;    // number of files
	union {
		int   DataSize;      // read as whole int after full 6-byte read
		struct {
			short DataSizeLo; // low word — overlaps with MixPeekHeader copy
			short DataSizeHi; // high word — filled by second Get() in legacy path
		};
	};

	// Populate from a peek header (legacy format — DataSizeHi still needs a second read)
	COMPILETIMEEVAL MixFileHeader& operator=(const MixPeekHeader& peek) noexcept {
		Count = peek.FileCount;
		DataSizeLo = peek.FormatFlags; // low word of DataSize happens to alias FormatFlags slot
		DataSizeHi = 0;
		return *this;
	}

};
#pragma pack (pop)

static_assert(sizeof(MixFileHeader) == 0x6, "Size Miss Match !");

class LoadedFileCache
{
public:
	static COMPILETIMEEVAL reference<LoadedFileCache*, 0xABF00C> const Head {};

	static void __fastcall Insert(LoadedFileCache* new_node, LoadedFileCache* node) {

		for (LoadedFileCache* i = node->Prev; node->Prev; i = node->Prev) {
			node = i->Next;

			if (i->CRC >= new_node->CRC) {
				node = i;
			}
		}

		node->Prev = new_node;
		new_node->Prev = 0;
		new_node->Next = 0;
	}

	static LoadedFileCache* __fastcall Find(unsigned int crc, LoadedFileCache* node) {

		bool v4 = false;
		LoadedFileCache* result = node;

		while (result) {
			int v3 = result->CRC;
			v4 = v3 < (int)crc;
			if (v3 == (int)crc) {
				if (result->FilePtr) {
					return result;
				}

				v4 = v3 < (int)crc;
			}

			if (v4) {
				result = result->Next;
			} else {
				result = result->Prev;
			}
		}

		return 0;
	}

	static LoadedFileCache* __fastcall Invalidate(unsigned int crc, LoadedFileCache* a2) {
		bool v4 = false;

		LoadedFileCache* result = a2;
		while (result) {
			int v3 = result->CRC;
			v4 = v3 < (int)crc;

			if (v3 == (int)crc) {
				if (result->FilePtr) {
					result->FilePtr = 0;
					return result;
				}

				v4 = v3 < (int)crc;
			}

			if (v4) {
				result = result->Next;
			} else {
				result = result->Prev;
			}
		}

		return result;
	}

	static void __fastcall Delete(LoadedFileCache* node) {
		if (node) {
			LoadedFileCache* v2 = node->Prev;
			if (node->Prev) {
				Delete(v2);
			}

			LoadedFileCache*  v3 = node->Next;
			if (v3) {
				Delete(v3);
			}

			YRMemory::free(node);
		}
	}

	static uint32_t __fastcall CacheFile(char* filename);

	static void __fastcall Destroy();

public:
	LoadedFileCache* Prev;
	LoadedFileCache* Next;
	unsigned int CRC;
	void* FilePtr;
};

static_assert(sizeof(LoadedFileCache) == 0x10);

class MemoryBuffer;
class ALIGN(4) NOVTABLE MixFileClass : public Node<MixFileClass>
{
	struct GenericMixFiles
	{
		MixFileClass* RA2MD;
		MixFileClass* RA2;
		MixFileClass* LANGUAGE;
		MixFileClass* LANGMD;
		MixFileClass* THEATER_TEMPERAT;
		MixFileClass* THEATER_TEMPERATMD;
		MixFileClass* THEATER_TEM;
		MixFileClass* GENERIC;
		MixFileClass* GENERMD;
		MixFileClass* THEATER_ISOTEMP;
		MixFileClass* THEATER_ISOTEM;
		MixFileClass* ISOGEN;
		MixFileClass* ISOGENMD;
		MixFileClass* MOVIES02D;
		MixFileClass* UNKNOWN_1;
		MixFileClass* MAIN;
		MixFileClass* CONQMD;
		MixFileClass* CONQUER;
		MixFileClass* CAMEOMD;
		MixFileClass* CAMEO;
		MixFileClass* CACHEMD;
		MixFileClass* CACHE;
		MixFileClass* LOCALMD;
		MixFileClass* LOCAL;
		MixFileClass* NTRLMD;
		MixFileClass* NEUTRAL;
		MixFileClass* MAPSMD02D;
		MixFileClass* MAPS02D;
		MixFileClass* UNKNOWN_2;
		MixFileClass* UNKNOWN_3;
		MixFileClass* SIDEC02DMD;
		MixFileClass* SIDEC02D;
	};

public:
	using AsNode = Node<MixFileClass>;

	static COMPILETIMEEVAL reference<List<MixFileClass*>, 0xABEFD8u> const MIXes{};

	static COMPILETIMEEVAL reference<DynamicVectorClass<MixFileClass*>, 0x884D90u> const Array{};
	static COMPILETIMEEVAL reference<DynamicVectorClass<MixFileClass*>, 0x884DC0u> const Array_Alt{};
	static COMPILETIMEEVAL reference<DynamicVectorClass<MixFileClass*>, 0x884DA8u> const Maps{};
	static COMPILETIMEEVAL reference<DynamicVectorClass<MixFileClass*>, 0x884DE0u> const Movies{};
	static COMPILETIMEEVAL constant_ptr<PKey, 0x886980u> const Key {};
	static COMPILETIMEEVAL reference<MixFileClass, 0x884DD8u> const MULTIMD{};
	static COMPILETIMEEVAL reference<MixFileClass, 0x884DDCu> const MULTI{};

	static COMPILETIMEEVAL reference<GenericMixFiles, 0x884DF8u> const Generics{};

	static bool Bootstrap()
		{ JMP_THIS(0x5301A0); }

	virtual ~MixFileClass()
	{
		YRMemory::free((void*)this->Filename);
		this->Free();
		this->FreeHeader();
		this->Node<MixFileClass>::~Node<MixFileClass>();
	}

    void Free() {

		if (this->Data && this->IsAllocated){
			YRMemory::free(this->Data);
		}

		this->Data = 0;
		this->IsAllocated = 0;
	}

	void FreeHeader() {
		if (auto pHeader = this->Headers) {
			YRMemory::free(pHeader);
		}

		this->Headers = 0;
	}

	bool IsValid() const noexcept
	{
		return this->Filename && this->Headers != nullptr;
	}

	bool Contains(const char* pName);

	bool Cache(const MemoryBuffer* buffer = nullptr) { return true; }


	static bool __fastcall Free(const char* pFilename);

    static bool __fastcall Cache(const char *pFilename, MemoryBuffer const * buffer = nullptr) { return true; }
	static bool __fastcall Offset(const char* pFilename, void** realptr = nullptr, MixFileClass** mixfile = nullptr, int* offset = nullptr, int* size = nullptr);

	static void DumpAllEntries();

	MixFileClass(const char* pFileName, PKey* pKey);
	MixFileClass(CCFileClass* pFile, PKey* pKey);

private:
	void ReadFromCCFile(CCFileClass* pFile, PKey* pKey);
	void ReadFromCCFIleWithoutStraws(CCFileClass* pFile, PKey* pKey);

public:
	const char* Filename;
	bool IsDigest;
	bool IsEncrypted;
	bool IsAllocated;
	int Count;
	int DataSize;
	int DataStart;
	MixHeaderData* Headers;
	void* Data;
};
static_assert(offsetof(MixFileClass, Filename) == 0xC, "Filename");
static_assert(offsetof(MixFileClass, IsDigest) == 0x10, "IsDigest");
static_assert(offsetof(MixFileClass, IsEncrypted) == 0x11, "IsEncrypted");
static_assert(offsetof(MixFileClass, IsAllocated) == 0x12, "IsAllocated");
static_assert(offsetof(MixFileClass, Count) == 0x14, "Count");
static_assert(offsetof(MixFileClass, DataSize) == 0x18, "DataSize");
static_assert(offsetof(MixFileClass, DataStart) == 0x1C, "DataStart");
static_assert(offsetof(MixFileClass, Headers) == 0x20, "Headers");
static_assert(offsetof(MixFileClass, Data) == 0x24, "Data");

static_assert(sizeof(MixFileClass) == 0x28);