/*
	LightSource - used for light posts and radiation
*/

#pragma once

#include <AbstractClass.h>

#include <CoordStruct.h>
#include <BasicStructures.h>

class DECLSPEC_UUID("6F9C48F0-1207-11D2-8174-006008055BB5")
	NOVTABLE LightSourceClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::LightSource;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) R0;

	//Destructor
	virtual ~LightSourceClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//non-virtual
	void Activate(DWORD dwZero = 0)	//Start lighting
		{ JMP_THIS(0x554A60); }

	void Deactivate(DWORD dwZero = 0)	//Stop lighting
		{ JMP_THIS(0x554A80); }

	void ChangeLevels(int nIntensity, TintStruct Tint, char mode)
		{ JMP_THIS(0x554AA0); }

	static void __fastcall UpdateLightConverts(int value)
		{ JMP_STD(0x554D50); }

	//Constructor
	LightSourceClass(
		int X, int Y, int Z, int nVisibility, int nIntensity, int Red, int Green, int Blue) noexcept
		: LightSourceClass(noinit_t())
	{ JMP_THIS(0x554760); }

	LightSourceClass(
		CoordStruct Crd, int nVisibility, int nIntensity, TintStruct Tint) noexcept
		: LightSourceClass(noinit_t())
	{ JMP_THIS(0x554760); }

	LightSourceClass() noexcept
		: LightSourceClass(noinit_t())
	{ JMP_THIS(0x554830); }

protected:
	explicit __forceinline LightSourceClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

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
