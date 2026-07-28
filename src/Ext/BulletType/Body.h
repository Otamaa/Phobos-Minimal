#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>

#include <Utilities/OptionalStruct.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/SavegameDef.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

#include <Ext/ObjectType/Body.h>

#include <ConvertClass.h>

class BulletTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = BulletTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "BulletTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BulletTypeClass";
	static COMPILETIMEEVAL DWORD Canary = 0xAD2BF3E6;

public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregates (unknown internal alignment)
	// ============================================================;

	// ============================================================
	// 8-byte aligned: unique_ptr
	// ============================================================
	std::unique_ptr<PhobosTrajectoryType> TrajectoryType { nullptr };

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types {};
	ValueableVector<WeaponTypeClass*> AirburstWeapons {};

	// ============================================================
	// OptionalStruct<pointer> (pointer + bool + padding)
	// ============================================================
	OptionalStruct<const ConvertClass*> ImageConvert {};

	// ============================================================
	// Valueable<pointer> (8 bytes each)
	// ============================================================
	Valueable<WeaponTypeClass*> Interceptable_WeaponOverride { nullptr };
	Valueable<WeaponTypeClass*> BounceHitWeapon { nullptr };
	Valueable<ParticleSystemTypeClass*> AttachedSystem { nullptr };
	Valueable<WeaponTypeClass*> ReturnWeapon { nullptr };

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<AnimTypeClass*> Parachute {};

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> RetargetAccuracy { 0.0 };
	Valueable<double> RetargetSelf_Probability { 0.5 };
	Valueable<double> AirburstSpread { 1.5 };
	Valueable<double> Splits_Range { 1280.0 };

	// ============================================================
	// Nullable<double> (double + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<double> Gravity {};
	Nullable<double> Shrapnel_Chance {};
	Nullable<double> MissileROTVar {};
	Nullable<double> PreExplodeRange {};
	Nullable<double> Trajectory_Speed {};

	// ============================================================
	// Nullable<int/enum/Leptons> (4 bytes + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<Armor> Armor {};
	Nullable<int> MissileSafetyAltitude {};
	Nullable<int> Proximity_Range {};
	Nullable<int> Parachuted_MaxFallRate {};
	Nullable<Leptons> BallisticScatterMin {};
	Nullable<Leptons> BallisticScatterMax {};

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2-4 bytes)
	// ============================================================
	Nullable<bool> SubjectToLand {};
	Nullable<bool> SubjectToWater {};
	Nullable<bool> AroundTarget {};
	Nullable<bool> VerticalInitialFacing {};
	Nullable<bool> UpdateImmediately {};
	Nullable<bool> Shrapnel_IgnoreHitBuildings {};

	// ============================================================
	// Valueable<Leptons> (4 bytes each, assuming Leptons = int)
	// ============================================================
	Valueable<Leptons> Cluster_Scatter_Min { BulletTypeExtData::DefaultBulletScatterMin };
	Valueable<Leptons> Cluster_Scatter_Max { BulletTypeExtData::DefaultBulletScatterMax };
	Valueable<Leptons> AirburstWeapon_SourceScatterMin {};
	Valueable<Leptons> AirburstWeapon_SourceScatterMax {};

	// ============================================================
	// Valueable<int> (4 bytes each)
	// ============================================================
	Valueable<int> Health { 0 };
	Valueable<int> Solid_Level { 0 };
	Valueable<int> Splits_TargetCellRange { 3 };
	Valueable<int> BounceAmount { 0 };
	Valueable<int> AnimLength { 0 };
	Valueable<int> EMPulseCannon_InaccurateRadius { 0 };
	Valueable<int> Parachuted_FallRate { 1 };
	Valueable<int> ZAdjust { 0 };

	// ============================================================
	// Valueable<bool> (1 byte each, packed together at the end)
	// ============================================================
	Nullable<bool> Interceptable { };
	Valueable<bool> Gravity_HeightFix { false };
	Nullable<bool> Shrapnel_AffectsGround {  };
	Nullable<bool> Shrapnel_AffectsBuildings {  };
	Valueable<bool> Shrapnel_UseWeaponTargeting { false };
	Valueable<bool> Interceptable_DeleteOnIntercept { false };
	Valueable<bool> SubjectToLand_Detonate { true };
	Valueable<bool> SubjectToWater_Detonate { true };
	Valueable<bool> AAOnly { false };
	Valueable<bool> SubjectToSolid { false };
	Valueable<bool> Splits { false };
	Valueable<bool> RetargetOwner { true };
	Nullable<bool> Airburst_UseCluster { };
	Valueable<bool> Airburst_RandomClusters { false };
	Nullable<bool> Splits_UseWeaponTargeting {  };
	Nullable<bool> AirburstWeapon_ApplyFirepowerMult {  };
	Valueable<bool> Splits_RandomCellUseHarcodedRange { true };
	Valueable<bool> Splits_TargetingUseVerses { true };
	Valueable<bool> Splits_FillRemainingClusterWithRandomcells { true };
	Nullable<bool> Splits_TargetingDistance_Cylindrical { };
	Nullable<bool> Splits_AllowRepeatTargets { };
	Nullable<bool> AirburstWeapon_UseFiringEffects { };
	Nullable<bool> AirburstWeapon_HeadToTarget {};
	Valueable<int> AirburstWeapon_RadialFireSegments {};
	Valueable<bool> BounceOnTerrain { true };
	Valueable<bool> BounceOnBuilding { false };
	Valueable<bool> BounceOnInfantry { false };
	Valueable<bool> BounceOnVehicle { false };
	Valueable<bool> SubjectToTrenches { true };
	Valueable<bool> IsScalable { false };
	Valueable<bool> Parachuted { false };
	Nullable<bool> Arcing_AllowElevationInaccuracy { };
	Nullable<bool> ReturnWeapon_ApplyFirepowerMult { };
	Valueable<bool> SubjectToGround { false };
	Valueable<bool> Airburst_TargetAsSource { false };
	Nullable<bool> Airburst_TargetAsSource_SkipHeight { };
	Valueable<bool> Parachuted_Remap { true };
	Nullable<bool> Vertical_AircraftFix { };
	Valueable<bool> AU { false };
	Nullable<bool> Shrapnel_ObeyWarheadTriggerConditions {};
	// 35 Valueable<bool> = 35 bytes, pads to 36 for 4-byte alignment
#pragma endregion

public:
	BulletTypeExtData(BulletTypeClass* pObj) : ObjectTypeExtData(pObj)		
	{
		this->AbsType = BulletTypeClass::AbsID;
	}

	BulletTypeExtData() = default;

	virtual ~BulletTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<BulletTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<BulletTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	BulletTypeClass* This() const { return reinterpret_cast<BulletTypeClass*>(this->AttachedToObject); }
	const BulletTypeClass* This_Const() const { return reinterpret_cast<const BulletTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true;  }

public:

	COMPILETIMEEVAL bool FORCEDINLINE HasSplitBehavior() const
	{
		// behavior in FS: Splits defaults to Airburst.
		return  ((BulletTypeClass*)this->AttachedToObject)->Airburst || this->Splits;
	}

	COMPILETIMEEVAL double FORCEDINLINE GetMissileROTVar(const RulesClass* const pRules) const
	{
		if (MissileROTVar.isset())
			return MissileROTVar.Fetch();

		return pRules->MissileROTVar;
	}

	COMPILETIMEEVAL int FORCEDINLINE GetMissileSaveAltitude(const RulesClass* const pRules) const
	{
		if (MissileSafetyAltitude.isset())
			return MissileSafetyAltitude.Fetch();

		return pRules->MissileSafetyAltitude;
	}

	COMPILETIMEEVAL double FORCEDINLINE GetAdjustedGravity() const
	{
		const auto nGravity = this->Gravity.Get(RulesClass::Instance->Gravity);
		return This()->Floater ? nGravity * 0.5 : nGravity;
	}

	BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, HouseClass* pHouse , WeaponTypeClass* pWeapon, bool addDamage, bool SetWeaponType) const;
	BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon, bool addDamage, bool SetWeaponType) const;
	BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const;
	BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright, bool addDamage) const;


	const ConvertClass* GetBulletConvert();
public:


	static FORCEDINLINE double GetAdjustedGravity(BulletTypeClass* pType);

	static BulletTypeClass* GetDefaultBulletType();
	static CoordStruct CalculateInaccurate(BulletTypeClass* pBulletType);

private:

	template <typename T>
	void Serialize(T& Stm);

public:
	static COMPILETIMEEVAL Leptons DefaultBulletScatterMin { 256 };
	static COMPILETIMEEVAL Leptons DefaultBulletScatterMax { 512 };
	//static COMPILETIMEEVAL int _base_1 = (Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell);
	//static COMPILETIMEEVAL int _base_2 = (Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell);

};

class BulletTypeExtContainer final : public Container<BulletTypeExtData>
	, public ReadWriteContainerInterfaces<BulletTypeExtData>
	, public ContainerSaveLoad<BulletTypeExtContainer, BulletTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "BulletTypeExtContainer";

public:
	static BulletTypeExtContainer Instance;

	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

	virtual void LoadFromINI(BulletTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(BulletTypeClass* key, CCINIClass* pINI);
};

double BulletTypeExtData::GetAdjustedGravity(BulletTypeClass* pType)
{
	return BulletTypeExtContainer::Instance.Find(pType)->GetAdjustedGravity();
}

class NOVTABLE FakeBulletTypeClass : public BulletTypeClass
{
public:
		static COMPILETIMEEVAL const char* ClassName = "FakeBulletTypeClass";

public:

	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	BulletTypeExtData* _GetExtData() {
		return *reinterpret_cast<BulletTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeBulletTypeClass) == sizeof(BulletTypeClass), "Invalid Size !");