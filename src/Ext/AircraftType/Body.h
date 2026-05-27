#pragma once

#include <AircraftTypeClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/FootType/Body.h>

class AircraftTypeExtData final : public FootTypeExtData
{
public:
	using base_type = AircraftTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "AircraftTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "AircraftTypeClass";
	

public:
	Nullable<AnimTypeClass*> TakeOff_Anim {};

	Nullable<bool> ExtendedAircraftMissions {};
	Nullable<bool> ExtendedAircraftMissions_SmoothMoving {};
	Nullable<bool> ExtendedAircraftMissions_EarlyDescend {};
	Nullable<bool> ExtendedAircraftMissions_RearApproach {};
	Nullable<bool> ExtendedAircraftMissions_FastScramble {};
	Nullable<int> ExtendedAircraftMissions_UnlandDamage {};

	Nullable<bool> FiringForceScatter {};

	Nullable<int> AttackingAircraftSightRange {};
	NullableIdx<VoxClass> SpyplaneCameraSound {};
	Nullable<int> ParadropRadius {};
	Nullable<int> ParadropOverflRadius {};
	Valueable<bool> Paradrop_DropPassangers { true };
	Valueable<int> Paradrop_MaxAttempt { 5 };

	Valueable<bool> IsCustomMissile { false };
	Valueable<RocketStruct> CustomMissileData { RocketStruct() };
	Valueable<WarheadTypeClass*> CustomMissileWarhead { nullptr };
	Valueable<WarheadTypeClass*> CustomMissileEliteWarhead { nullptr };
	Valueable<AnimTypeClass*> CustomMissileTakeoffAnim { nullptr };
	Valueable<AnimTypeClass*> CustomMissilePreLauchAnim { nullptr };
	Valueable<AnimTypeClass*> CustomMissileTrailerAnim { nullptr };
	Valueable<int> CustomMissileTrailerSeparation { 3 };
	Valueable<WeaponTypeClass*> CustomMissileWeapon { nullptr };
	Valueable<WeaponTypeClass*> CustomMissileEliteWeapon { nullptr };
	Valueable<int> CustomMissileInaccuracy { 0 };
	Valueable<int> CustomMissileTrailAppearDelay { 2 };
	Valueable<double> CustomMissileCloseEnoughFactor { 1.0 };
	NullablePromotable<bool> CustomMissileRaise { };
	Nullable<Point2D> CustomMissileOffset {};

	AircraftTypeExtData(AircraftTypeClass* pObj) : FootTypeExtData(pObj)
	{
		this->AbsType = AircraftTypeClass::AbsID;
		this->CustomMissileData->Type = pObj;
		this->InitializeConstant();
	}

	void Initialize() override;

	AircraftTypeExtData(AircraftTypeClass* pObj, noinit_t nn) : FootTypeExtData(pObj, nn) { }

	virtual ~AircraftTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->FootTypeExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->FootTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<AircraftTypeExtData*>(this)->FootTypeExtData::SaveToStream(Stm);
		const_cast<AircraftTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {
		this->FootTypeExtData::CalculateCRC(crc);
	}

	AircraftTypeClass* This() const { return reinterpret_cast<AircraftTypeClass*>(this->AttachedToObject); }
	const AircraftTypeClass* This_Const() const { return reinterpret_cast<const AircraftTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr) override;
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

	AircraftTypeExtData(AircraftTypeExtData&&) noexcept = default;
	AircraftTypeExtData& operator=(AircraftTypeExtData&&) noexcept = default;
	AircraftTypeExtData(const AircraftTypeExtData&) = delete;
	AircraftTypeExtData& operator=(const AircraftTypeExtData&) = delete;

public:
	static bool ExtendedAircraftMissionsEnabled(AircraftClass* pAircraft);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class AircraftTypeExtContainer final : public Container<AircraftTypeExtData>
	, public ReadWriteContainerInterfaces<AircraftTypeExtData>, public ContainerSaveLoad<AircraftTypeExtContainer, true>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "AircraftTypeExtContainer";

public:
	static AircraftTypeExtContainer Instance;

	virtual void LoadFromINI(AircraftTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(AircraftTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeAircraftTypeClass : public AircraftTypeClass {
public:

	HRESULT __stdcall _Load(IStream* pStm);

	bool _CanUseWaypoint(){
		return !this->Spawned;
	}

	bool _CanAttackMove();
	bool _ReadFromINI(CCINIClass* pINI);
};

static_assert(sizeof(FakeAircraftTypeClass) == sizeof(AircraftTypeClass), "Invalid Size !");