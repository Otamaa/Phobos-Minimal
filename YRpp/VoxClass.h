/*
	EVA Messages!
*/

#pragma once
#include <CRT.h>
#include <ArrayClasses.h>
#include <CoordStruct.h>
#include <GeneralDefinitions.h>

#include <Helpers/CompileTime.h>

class CCINIClass;
class VoxClass
{
public:

	static HRESULT __fastcall Load(IStream* pStm)
	{ JMP_THIS(0x7533F0); }

	static HRESULT __fastcall Save(IStream* pStm)
	{ JMP_THIS(0x7533B0); }

	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<VoxClass*>, 0xB1D4A0u> const Array{};

	static COMPILETIMEEVAL reference<int, 0xB1D4C8u> const EVAIndex{};

	static NOINLINE VoxClass* __fastcall Find(const char* pName)
	{
		for(int i = 0; i < Array->Count; ++i) {
			if(!CRT::strcmpi(Array->Items[i]->Name, pName)) {
				return Array->Items[i];
			}
		}
		return nullptr;
	}

	static NOINLINE int __fastcall FindIndexById(const char* pName)
	{
		for(int i = 0; i < Array->Count; ++i) {
			if(!CRT::strcmpi(Array->Items[i]->Name, pName)) {
				return i;
			}
		}
		return -1;
	}

	static void __fastcall Play(const char* pName, VoxType nUnk = VoxType::none, VoxPriority nUnk2 = VoxPriority::none)
		{ JMP_FAST(0x752700); }

	static void __fastcall PlayIndex(int index, VoxType nUnk = VoxType::none, VoxPriority nUnk2 = VoxPriority::none)
		{ JMP_FAST(0x752480); }

	// no idea what this does, but Super::Launch uses it on "SW Ready" events right after firing said SW
	static void __fastcall SilenceIndex(int index)
		{ JMP_FAST(0x752A40); }

	static const char* __fastcall GetName(int index)
		{ JMP_FAST(0x753330); }

	static void __fastcall DeleteAll()
		{ JMP_FAST(0x7531A0); }

	//Properties

public:

	char Name[0x28];
	float Volume;			//as in eva.ini
	char Yuri [0x9];		//as in eva.ini
	char Russian [0x9];		//as in eva.ini
	char Allied [0x9];		//as in eva.ini
	VoxPriority Priority;	//as in eva.ini
	VoxType Type;			//as in eva.ini
	int unknown_int_50;

	//constructor and destructor should never be needed
	VoxClass(char* pName)
		{ JMP_THIS(0x752CB0) }

	~VoxClass()
		{ JMP_THIS(0x752D60) }

	const char* GetFilename() const
		{ JMP_THIS(0x753380) }

	bool LoadFromINI(CCINIClass *pINI)
		{ JMP_THIS(0x752DB0) }
};

static_assert(sizeof(VoxClass) == 0x54, "Invalid size.");
