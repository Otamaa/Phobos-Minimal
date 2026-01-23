#pragma once

#include <TechnoTypeClass.h>

class TechnoClass;

class DECLSPEC_UUID("DCBD42EA-0546-11D2-ACA4-006008055BB5")
	NOVTABLE UnitTypeClass : public TechnoTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::UnitType;
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F6218;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<UnitTypeClass*>, 0xA83CE0u> const Array {};

	IMPL_Find(UnitTypeClass)

	static UnitTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_FAST(0x7480D0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_FAST(0x747370);
	}

	//static
	static void __fastcall InitOneTimeData() { JMP_FAST(0x7473E0); }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x747F30);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x748010);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x7480B0);

	//Destructor
	virtual ~UnitTypeClass() JMP_THIS(0x748190);

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int ClassSize() const override R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x747620);

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override R0;
	virtual ObjectClass* CreateObject(HouseClass* pOwner) override R0;

	//TechnoTypeClass
	static void __fastcall OneTimeInit()
		{ JMP_FAST(0x7473E0); }

	//Constructor
	UnitTypeClass(const char* pID) noexcept
		: UnitTypeClass(noinit_t())
	{ JMP_THIS(0x7470D0); }

protected:
	explicit __forceinline UnitTypeClass(noinit_t) noexcept
		: TechnoTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	LandType MovementRestrictedTo;
	CoordStruct HalfDamageSmokeLocation;
	bool Passive;
	bool CrateGoodie;
	bool Harvester;
	bool Weeder;
	bool FireAnim; //E10
	bool HasTurret; //LockTurrent
	bool DeployToFire;
	bool IsSimpleDeployer;
	bool IsTilter;
	bool UseTurretShadow;
	bool TooBigToFitUnderBridge;
	bool CanBeach;
	bool SmallVisceroid;
	bool LargeVisceroid;
	bool CarriesCrate;
	bool NonVehicle;
	int StandingFrames;
	int DeathFrames;
	int DeathFrameRate;
	int StartStandFrame;
	int StartWalkFrame;
	int StartFiringFrame;
	int StartDeathFrame;
	int MaxDeathCounter;
	int Facings;
	int FiringSyncFrame0;
	int FiringSyncFrame1;
	int BurstDelay0;
	int BurstDelay1;
	int BurstDelay2;
	int BurstDelay3;
	SHPStruct* AltImage;
	char WalkFrames;
	char FiringFrames;
	char AltImageFile [0x19];
};
static_assert(sizeof(UnitTypeClass) == 0xE78,"Invalid size.");