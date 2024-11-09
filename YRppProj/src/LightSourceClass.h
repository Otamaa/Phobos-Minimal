/*
	LightSource - used for light posts and radiation
*/

#pragma once

#include <AbstractClass.h>

#include <CoordStruct.h>
#include <BasicStructures.h>
class PendingCellClass;
class DECLSPEC_UUID("6F9C48F0-1207-11D2-8174-006008055BB5")
	NOVTABLE LightSourceClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::LightSource;
	static constexpr inline DWORD vtable = 0x7ED028;

	static constexpr reference<DynamicVectorClass<LightSourceClass*>, 0xABCA10u> const Array {};
	static constexpr reference<DynamicVectorClass<PendingCellClass*>, 0xABCA40u> const Unknown_0xABCA40 {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override;// JMP_STD(0x555080);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override;// JMP_STD(0x5550C0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override;//JMP_STD(0x555110);

	//Destructor
	virtual ~LightSourceClass() override;

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::LightSource; }
	virtual int Size() const override { return 0x4C; }

	//non-virtual
	//static
	//static void sub_5549A0()
	//	{ JMP_STD(0x5549A0); }

	static void __fastcall UpdateLightConverts(int value)
		;//{ JMP_STD(0x554D50); }

	//static int Init_Unknown_0xABCA40()
	//	{ JMP_STD(0x5546C0); }

	//static inline NAKED void __cdecl Uninit_Unknown_0xABCA40()
	//	{ JMP(0xABCA40); }

	void Activate(DWORD dwZero = 0)	//Start lighting
		;//{ JMP_THIS(0x554A60); }

	void Deactivate(DWORD dwZero = 0)	//Stop lighting
		;//{ JMP_THIS(0x554A80); }

	void ChangeLevels(int nIntensity, TintStruct Tint, char mode)
		;//{ JMP_THIS(0x554AA0); }

	//Constructor
	LightSourceClass(int X, int Y, int Z, int nVisibility, int nIntensity, int Red, int Green, int Blue);
	LightSourceClass(CoordStruct Crd, int nVisibility, int nIntensity, TintStruct Tint);
	LightSourceClass();

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int LightIntensity;
	TintStruct LightTint;
	int DetailLevel;
	CoordStruct Location;
	int LightVisibility;
	bool Activated;
};

struct UninitLightSource
{
	void operator() (LightSourceClass* pConvert) const
	{
		GameDelete<true, true>(pConvert);
		pConvert = nullptr;
	}
};