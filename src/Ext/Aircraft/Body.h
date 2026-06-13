#pragma once

#include <Utilities/Enum.h>
#include <AircraftClass.h>
#include <Ext/Foot/Body.h>


class AircraftTypeExtData;
class AbstractClass;
class AircraftClass;
class WeaponTypeClass;
class AircraftExtData : public FootExtData
{
public:
	using base_type = AircraftClass;
	static COMPILETIMEEVAL const char* ClassName = CLASS_NAME(AircraftExtData);
	static COMPILETIMEEVAL const char* BaseClassName = "AircraftClass";
	static COMPILETIMEEVAL DWORD Canary = 0xB78D268D;

public:

	int Strafe_BombsDroppedThisRound {};
	int CurrentAircraftWeaponIndex {};
	CellClass* Strafe_TargetCell {};

	AircraftExtData(AircraftClass* pObj);
	AircraftExtData() = default;

	virtual ~AircraftExtData() = default;

	FORCEDINLINE AircraftClass* This() const { return reinterpret_cast<AircraftClass*>(this->AttachedToObject); }
	FORCEDINLINE const AircraftClass* This_Const() const { return reinterpret_cast<const AircraftClass*>(this->AttachedToObject); }
	FORCEDINLINE AircraftTypeExtData* GetTypeExtData() const { return ((AircraftTypeExtData*)TypeExtData); }

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) {
		this->FootExtData::InvalidatePointer(ptr, bRemoved, type);
	}


	virtual void LoadFromStream(PhobosStreamReader& Stm) {
		this->FootExtData::LoadFromStream(Stm);
		Stm.Process(this->Strafe_BombsDroppedThisRound);
		Stm.Process(this->CurrentAircraftWeaponIndex);
		Stm.Process(this->Strafe_TargetCell);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) {
		this->FootExtData::SaveToStream(Stm);
		Stm.Process(this->Strafe_BombsDroppedThisRound);
		Stm.Process(this->CurrentAircraftWeaponIndex);
		Stm.Process(this->Strafe_TargetCell);
	}

	virtual void CalculateCRC(CRCEngine& crc) const {
		this->FootExtData::CalculateCRC(crc);
	}

public :

	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber);
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber , int WeaponIdx);
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon);
	static void TriggerCrashWeapon(AircraftClass* pThis , int nMult);
	static bool IsValidLandingZone(AircraftClass* pThis);

	static bool PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell);

	static bool FireWeapon(AircraftClass* pAir, AbstractClass* pTarget);
	static int GetDelay(AircraftClass* pThis, bool isLastShot);
};

class AircraftExtContainer final : public Container<AircraftExtData> 
	, public ContainerSaveLoad<AircraftExtContainer, AircraftExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "AircraftExtContainer";

	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

public:
	static AircraftExtContainer Instance;
};

class AbstractClass;
class NOVTABLE FakeAircraftClass : public AircraftClass
{
public:

	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

	void __Look(bool incremental, int arg_4);

	WeaponStruct* _GetWeapon(int weaponIndex);
	void _SetTarget(AbstractClass* pTarget);
	void _Destroyed(int mult);
	AbstractClass* _GreatestThreat(ThreatType threatType, CoordStruct* pSelectCoords, bool onlyTargetHouseEnemy);
	int _Mission_Sleep();

	bool  __Unlimbo_Wrapper(const CoordStruct& coords, DirType facing);

	int _Mission_ParadropOverfly();
	int _Mission_ParadropApproach();
	int _Mission_SpyPlaneOverfly();
	int _Mission_SpyPlaneApproach();
	int _Mission_Move_ForCarryAll();
	int _Mission_Move();

	bool _Enter_Idle_Mode(bool initial, bool bool2);

	void _FootClass_Update_Wrapper();
	DamageState __Take_Damage(int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse);

	BulletClass* _FireAt(AbstractClass* target, int which);
	int _Mission_Attack();

	void _Detach(AbstractClass* target, bool all);

	FORCEDINLINE AircraftExtData* _GetExtData() {
		return *reinterpret_cast<AircraftExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE const AircraftExtData* _GetExtData() const {
		return *reinterpret_cast<const AircraftExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};

static_assert(sizeof(FakeAircraftClass) == sizeof(AircraftClass), "Invalid Size !");
