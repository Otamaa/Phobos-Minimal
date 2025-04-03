/*
	TerrainTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>
#include <ColorStruct.h>

class DECLSPEC_UUID("5AF2CE7B-0634-11D2-ACA4-006008055BB5")
	NOVTABLE TerrainTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::TerrainType;
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F5458;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<TerrainTypeClass*>, 0xA8E318u> const Array {};

	IMPL_Find(TerrainTypeClass)

	static TerrainTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x71E2A0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x71DD80);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x71E1D0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x71E240);

	//Destructor
	virtual ~TerrainTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords,HouseClass* pOwner) override R0;
	virtual ObjectClass* CreateObject(HouseClass* owner) override R0;

	//Constructor
	TerrainTypeClass(const char* pID) noexcept
		: TerrainTypeClass(noinit_t())
	{ JMP_THIS(0x71DA80); }

protected:
	explicit __forceinline TerrainTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	int Foundation;
	ColorStruct RadarColor;
	int AnimationRate;
	float AnimationProbability;
	int TemperateOccupationBits;
	int SnowOccupationBits;
	bool WaterBound;
	bool SpawnsTiberium;
	bool IsFlammable;
	bool IsAnimated;
	bool IsVeinhole;
	CellStruct* FoundationData;
};

static_assert(sizeof(TerrainTypeClass) == 0x2BC, "Invalid Size !");
