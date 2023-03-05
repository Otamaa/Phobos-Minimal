#pragma once

#include <TechnoTypeClass.h>

class DECLSPEC_UUID("AE8B33D9-061C-11D2-ACA4-006008055BB5")
	NOVTABLE AircraftTypeClass : public TechnoTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::AircraftType;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<AircraftTypeClass*>, 0xA8B218u> const Array {};

	static NOINLINE AircraftTypeClass* __fastcall Find(const char* pID)
	{
		for (auto pItem : *Array){
			if (!CRT::strcmpi(pItem->ID, pID))
				return pItem;
		}

		return nullptr;
	}

	static AircraftTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x41CEF0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x41CAA0);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x41CEB0);
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x41CE20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x41CE90);

	//Destructor
	virtual ~AircraftTypeClass() JMP_THIS(0x41CFE0);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::AircraftType; }
	virtual int	Size() const { return 0xE10; }
	virtual int GetArrayIndex() const { return this->ArrayIndex; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x41CC20);
	
	//ObjectTypeClass
	virtual void Dimension2(CoordStruct* pDest) JMP_THIS(0x41CBF0);
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) { return false; }
	virtual ObjectClass* CreateObject(HouseClass* pOwner) JMP_THIS(0x41CB20);
	virtual CellStruct* GetFoundationData(bool IncludeBib) const JMP_THIS(0x41CB70);

	//TechnoTypeClass
	virtual bool CanUseWaypointMode() const R0;
	virtual bool CanAttackMove() const { return false; }//JMP_THIS(0x41CB60)

	//Constructor
	AircraftTypeClass(const char* pID) noexcept
		: AircraftTypeClass(noinit_t())
	{ JMP_THIS(0x41C8B0); }

protected:
	explicit __forceinline AircraftTypeClass(noinit_t) noexcept
		: TechnoTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	bool Carryall;
	AnimTypeClass* Trailer;
	int SpawnDelay;
	bool Rotors;
	bool CustomRotor;
	bool Landable;
	bool FlyBy;
	bool FlyBack;
	bool AirportBound;
	bool Fighter;
};
