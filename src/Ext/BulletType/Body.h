#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>

#include <Utilities/TemplateDefB.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/SavegameDef.h>

#include <New/Type/LaserTrailTypeClass.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

#include <Ext/ObjectType/Body.h>

class BulletTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = BulletTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "BulletTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BulletTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregates (unknown internal alignment)
	// ============================================================
	TrailsReader Trails;

	// ============================================================
	// 8-byte aligned: unique_ptr
	// ============================================================
	std::unique_ptr<PhobosTrajectoryType> TrajectoryType;

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
	ValueableVector<WeaponTypeClass*> AirburstWeapons;

	// ============================================================
	// OptionalStruct<pointer> (pointer + bool + padding)
	// ============================================================
	OptionalStruct<const ConvertClass*> ImageConvert;

	// ============================================================
	// Valueable<pointer> (8 bytes each)
	// ============================================================
	Valueable<WeaponTypeClass*> Interceptable_WeaponOverride;
	Valueable<WeaponTypeClass*> BounceHitWeapon;
	Valueable<ParticleSystemTypeClass*> AttachedSystem;
	Valueable<WeaponTypeClass*> ReturnWeapon;

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<AnimTypeClass*> Parachute;

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> RetargetAccuracy;
	Valueable<double> RetargetSelf_Probability;
	Valueable<double> AirburstSpread;
	Valueable<double> Splits_Range;

	// ============================================================
	// Nullable<double> (double + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<double> Gravity;
	Nullable<double> Shrapnel_Chance;
	Nullable<double> MissileROTVar;
	Nullable<double> PreExplodeRange;
	Nullable<double> Trajectory_Speed;

	// ============================================================
	// Nullable<int/enum/Leptons> (4 bytes + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<Armor> Armor;
	Nullable<int> MissileSafetyAltitude;
	Nullable<int> Proximity_Range;
	Nullable<int> Parachuted_MaxFallRate;
	Nullable<Leptons> BallisticScatterMin;
	Nullable<Leptons> BallisticScatterMax;

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2-4 bytes)
	// ============================================================
	Nullable<bool> SubjectToLand;
	Nullable<bool> SubjectToWater;
	Nullable<bool> AroundTarget;
	Nullable<bool> VerticalInitialFacing;
	Nullable<bool> UpdateImmediately;

	// ============================================================
	// Valueable<Leptons> (4 bytes each, assuming Leptons = int)
	// ============================================================
	Valueable<Leptons> Cluster_Scatter_Min;
	Valueable<Leptons> Cluster_Scatter_Max;
	Valueable<Leptons> AirburstWeapon_SourceScatterMin;
	Valueable<Leptons> AirburstWeapon_SourceScatterMax;

	// ============================================================
	// Valueable<int> (4 bytes each)
	// ============================================================
	Valueable<int> Health;
	Valueable<int> Solid_Level;
	Valueable<int> Splits_TargetCellRange;
	Valueable<int> BounceAmount;
	Valueable<int> AnimLength;
	Valueable<int> EMPulseCannon_InaccurateRadius;
	Valueable<int> Parachuted_FallRate;

	// ============================================================
	// Valueable<bool> (1 byte each, packed together at the end)
	// ============================================================
	Valueable<bool> Interceptable;
	Valueable<bool> Gravity_HeightFix;
	Valueable<bool> Shrapnel_AffectsGround;
	Valueable<bool> Shrapnel_AffectsBuildings;
	Valueable<bool> Shrapnel_UseWeaponTargeting;
	Valueable<bool> Interceptable_DeleteOnIntercept;
	Valueable<bool> SubjectToLand_Detonate;
	Valueable<bool> SubjectToWater_Detonate;
	Valueable<bool> AAOnly;
	Valueable<bool> SubjectToSolid;
	Valueable<bool> Splits;
	Valueable<bool> RetargetOwner;
	Valueable<bool> Airburst_UseCluster;
	Valueable<bool> Airburst_RandomClusters;
	Valueable<bool> Splits_UseWeaponTargeting;
	Valueable<bool> AirburstWeapon_ApplyFirepowerMult;
	Valueable<bool> Splits_RandomCellUseHarcodedRange;
	Valueable<bool> Splits_TargetingUseVerses;
	Valueable<bool> Splits_FillRemainingClusterWithRandomcells;
	Valueable<bool> BounceOnTerrain;
	Valueable<bool> BounceOnBuilding;
	Valueable<bool> BounceOnInfantry;
	Valueable<bool> BounceOnVehicle;
	Valueable<bool> SubjectToTrenches;
	Valueable<bool> IsScalable;
	Valueable<bool> Parachuted;
	Valueable<bool> Arcing_AllowElevationInaccuracy;
	Valueable<bool> ReturnWeapon_ApplyFirepowerMult;
	Valueable<bool> SubjectToGround;
	Valueable<bool> Airburst_TargetAsSource;
	Valueable<bool> Airburst_TargetAsSource_SkipHeight;
	Valueable<bool> Parachuted_Remap;
	Valueable<bool> Vertical_AircraftFix;
	Valueable<bool> AU;
	// 35 Valueable<bool> = 35 bytes, pads to 36 for 4-byte alignment

#pragma endregion

public:
	BulletTypeExtData(BulletTypeClass* pObj)
		: ObjectTypeExtData(pObj)
		// Large aggregates
		, Trails()
		// unique_ptr
		, TrajectoryType(nullptr)
		// Vectors
		, LaserTrail_Types()
		, AirburstWeapons()
		// OptionalStruct
		, ImageConvert()
		// Valueable<pointer>
		, Interceptable_WeaponOverride(nullptr)
		, BounceHitWeapon(nullptr)
		, AttachedSystem(nullptr)
		, ReturnWeapon(nullptr)
		// Nullable<pointer>
		, Parachute()
		// Valueable<double>
		, RetargetAccuracy(0.0)
		, RetargetSelf_Probability(0.5)
		, AirburstSpread(1.5)
		, Splits_Range(1280.0)
		// Nullable<double>
		, Gravity()
		, Shrapnel_Chance()
		, MissileROTVar()
		, PreExplodeRange()
		, Trajectory_Speed()
		// Nullable<int/enum/Leptons>
		, Armor()
		, MissileSafetyAltitude()
		, Proximity_Range()
		, Parachuted_MaxFallRate()
		, BallisticScatterMin()
		, BallisticScatterMax()
		// Nullable<bool>
		, SubjectToLand()
		, SubjectToWater()
		, AroundTarget()
		, VerticalInitialFacing()
		, UpdateImmediately()
		// Valueable<Leptons>
		, Cluster_Scatter_Min(BulletTypeExtData::DefaultBulletScatterMin)
		, Cluster_Scatter_Max(BulletTypeExtData::DefaultBulletScatterMax)
		, AirburstWeapon_SourceScatterMin()
		, AirburstWeapon_SourceScatterMax()
		// Valueable<int>
		, Health(0)
		, Solid_Level(0)
		, Splits_TargetCellRange(3)
		, BounceAmount(0)
		, AnimLength(0)
		, EMPulseCannon_InaccurateRadius(0)
		, Parachuted_FallRate(1)
		// Valueable<bool>
		, Interceptable(false)
		, Gravity_HeightFix(false)
		, Shrapnel_AffectsGround(false)
		, Shrapnel_AffectsBuildings(false)
		, Shrapnel_UseWeaponTargeting(false)
		, Interceptable_DeleteOnIntercept(false)
		, SubjectToLand_Detonate(true)
		, SubjectToWater_Detonate(true)
		, AAOnly(false)
		, SubjectToSolid(false)
		, Splits(false)
		, RetargetOwner(true)
		, Airburst_UseCluster(false)
		, Airburst_RandomClusters(false)
		, Splits_UseWeaponTargeting(false)
		, AirburstWeapon_ApplyFirepowerMult(false)
		, Splits_RandomCellUseHarcodedRange(true)
		, Splits_TargetingUseVerses(true)
		, Splits_FillRemainingClusterWithRandomcells(true)
		, BounceOnTerrain(true)
		, BounceOnBuilding(false)
		, BounceOnInfantry(false)
		, BounceOnVehicle(false)
		, SubjectToTrenches(true)
		, IsScalable(false)
		, Parachuted(false)
		, Arcing_AllowElevationInaccuracy(true)
		, ReturnWeapon_ApplyFirepowerMult(false)
		, SubjectToGround(false)
		, Airburst_TargetAsSource(false)
		, Airburst_TargetAsSource_SkipHeight(false)
		, Parachuted_Remap(true)
		, Vertical_AircraftFix(true)
		, AU(false)
	{
		this->AbsType = BulletTypeClass::AbsID;
	}

	BulletTypeExtData(BulletTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~BulletTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
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
			return MissileROTVar.Get();

		return pRules->MissileROTVar;
	}

	COMPILETIMEEVAL double FORCEDINLINE GetMissileSaveAltitude(const RulesClass* const pRules) const
	{
		if (MissileSafetyAltitude.isset())
			return MissileSafetyAltitude.Get();

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
{
public:
	static COMPILETIMEEVAL const char* ClassName = "BulletTypeExtContainer";

public:
	static BulletTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

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

	bool _ReadFromINI(CCINIClass* pINI);

	BulletTypeExtData* _GetExtData() {
		return *reinterpret_cast<BulletTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeBulletTypeClass) == sizeof(BulletTypeClass), "Invalid Size !");