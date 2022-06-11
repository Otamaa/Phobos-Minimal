#pragma once

#include <GenericList.h>
#include <ArrayClasses.h>
#include <Helpers/CompileTime.h>
#include <PKey.h>

struct MixHeaderData
{
	DWORD ID;
	DWORD Offset;
	DWORD Size;
};

class MixFileClass : public Node<MixFileClass>
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
	static constexpr reference<List<MixFileClass*>, 0xABEFD8u> const MIXes{};

	static constexpr reference<DynamicVectorClass<MixFileClass*>, 0x884D90u> const Array{};
	static constexpr reference<DynamicVectorClass<MixFileClass*>, 0x884DC0u> const Array_Alt{};
	static constexpr reference<DynamicVectorClass<MixFileClass*>, 0x884DA8u> const Maps{};
	static constexpr reference<DynamicVectorClass<MixFileClass*>, 0x884DE0u> const Movies{};
	static constexpr reference<PKey*, 0x886980u> const Key {};
	static constexpr reference<MixFileClass, 0x884DD8u> const MULTIMD{};
	static constexpr reference<MixFileClass, 0x884DDCu> const MULTI{};

	static constexpr reference<GenericMixFiles, 0x884DF8u> const Generics{};

	static void Bootstrap()
		{ JMP_THIS(0x5301A0); }

	virtual ~MixFileClass() RX;

    void Free()
		{ JMP_THIS(0x5B4400); }

    static bool __fastcall Free(const char *pFilename)
		{ JMP_STD(0x5B3E90); }

    bool Cache(const MemoryBuffer * buffer = nullptr)
		{ JMP_THIS(0x5B43F0); }

    static bool __fastcall Cache(const char *pFilename, MemoryBuffer const * buffer = nullptr)
		{ JMP_STD(0x5B43E0); }

    static bool __fastcall Offset(const char *pFilename, void ** realptr = nullptr, MixFileClass ** mixfile = nullptr, long * offset = nullptr, long * size = nullptr)
		{ JMP_STD(0x5B4430); }

    static void* __fastcall Retrieve(const char *pFilename, bool bLoadAsSHP = false)
		{ JMP_STD(0x5B40B0); }


	MixFileClass(const char* pFileName)
		: Node<MixFileClass>()
	{
		PUSH_IMM(0x886980);
		PUSH_VAR32(pFileName);
		THISCALL(0x5B3C20);
	}

	static void* Retrieve(char* name, bool forceShapeCache)
		{ JMP_STD(0x5B40B0); }

	static bool __fastcall Offset(const char* filename, void*& data,
		MixFileClass*& mixfile, int& offset, int& length)
	{ JMP_STD(0x5B4430); }

protected:
	/*PROPERTY(MixFileClass*, Next);
	MixFileClass* Prev;*/

public:

	const char* FileName;
	bool Blowfish;
	bool Encryption;
	int CountFiles;
	int FileSize;
	int FileStartOffset;
	MixHeaderData* Headers;
	int field_24;
};

//static_assert(sizeof(MixFileClass) == 0x28);