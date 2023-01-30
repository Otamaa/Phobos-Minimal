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

	//Array
	static constexpr constant_ptr<DynamicVectorClass<TerrainTypeClass*>, 0xA8E318u> const Array {};

	static NOINLINE TerrainTypeClass* __fastcall Find(const char* pID)
	{
		for (auto pItem : *Array){
			if (!CRT::strcmpi(pItem->ID, pID))
				return pItem;
		}

		return nullptr;
	}

	static TerrainTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x71E2A0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x71DD80);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~TerrainTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords,HouseClass* pOwner) R0;
	virtual ObjectClass* CreateObject(HouseClass* owner) R0;

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
	PROTECTED_PROPERTY(BYTE, TTypePadA);
	int AnimationRate;
	float AnimationProbability;
	int TemperateOccupationBits;
	int SnowOccupationBits;
	bool WaterBound;
	bool SpawnsTiberium;
	bool IsFlammable;
	bool IsAnimated;
	bool IsVeinhole;
	PROTECTED_PROPERTY(BYTE, TTypePadB[3]);
	CellStruct* FoundationData;
};

static_assert(sizeof(TerrainTypeClass) == 0x2BC, "Invalid Size !");