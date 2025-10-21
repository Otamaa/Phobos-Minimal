#pragma once

#include <Utilities/Enum.h>
#include <AircraftClass.h>
#include <Ext/Foot/Body.h>

class AbstractClass;
class AircraftClass;
class WeaponTypeClass;
class AircraftExtData : public FootExtData
{
public:
	using base_type = AircraftClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:
	int Strafe_BombsDroppedThisRound;
	int CurrentAircraftWeaponIndex;
	CellClass* Strafe_TargetCell;

	AircraftExtData(AircraftClass* pObj) : FootExtData(pObj),
		Strafe_BombsDroppedThisRound(0),
		CurrentAircraftWeaponIndex(0),
		Strafe_TargetCell()
	{
		auto pIdent = Phobos::gEntt->try_get<ExtensionIdentifierComponent>(this->MyEntity);
		pIdent->Name = pObj->Type->ID;
		pIdent->AbsType = AircraftClass::AbsID;
	}

	AircraftExtData(AircraftClass * pObj, noinit_t nn) : FootExtData(pObj, nn) { }
	virtual ~AircraftExtData() = default;

	virtual AircraftClass* This() const override { return reinterpret_cast<AircraftClass*>(this->FootExtData::This()); }
	virtual const AircraftClass* This_Const() const override { return reinterpret_cast<const AircraftClass*>(this->FootExtData::This_Const()); }

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) {
		this->FootExtData::InvalidatePointer(ptr, bRemoved);
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
};

class AircraftExtContainer final : public Container<AircraftExtData>
{
public:
	static AircraftExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array) {
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class AbstractClass;
class NOVTABLE FakeAircraftClass : public AircraftClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	WeaponStruct* _GetWeapon(int weaponIndex);
	void _SetTarget(AbstractClass* pTarget);
	void _Destroyed(int mult);
	AbstractClass* _GreatestThreat(ThreatType threatType, CoordStruct* pSelectCoords, bool onlyTargetHouseEnemy);
	int _Mission_Sleep();
	void _FootClass_Update_Wrapper();

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
