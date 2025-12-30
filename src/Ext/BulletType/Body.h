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
	Valueable<int> Health;
	Nullable<Armor> Armor;
	Valueable<bool> Interceptable;
	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
	Nullable<double> Gravity;
	Valueable<bool> Gravity_HeightFix;
	Valueable<bool> Shrapnel_AffectsGround;
	Valueable<bool> Shrapnel_AffectsBuildings;
	Nullable<double> Shrapnel_Chance;
	Valueable<bool> Shrapnel_UseWeaponTargeting;
	Valueable<Leptons> Cluster_Scatter_Min;
	Valueable<Leptons> Cluster_Scatter_Max;
	Valueable<bool> Interceptable_DeleteOnIntercept;
	Valueable<WeaponTypeClass*> Interceptable_WeaponOverride;
	Nullable<bool> SubjectToLand;
	Valueable<bool> SubjectToLand_Detonate;
	Nullable<bool> SubjectToWater;
	Valueable<bool> SubjectToWater_Detonate;
	Valueable<bool> AAOnly;
	Valueable<bool> SubjectToSolid;
	Valueable<int> Solid_Level;
	Nullable<AnimTypeClass*> Parachute;
	Nullable<double> MissileROTVar;
	Nullable<int> MissileSafetyAltitude;
	Valueable<bool> Splits;
	Valueable<double> RetargetAccuracy;
	Valueable<bool> RetargetOwner;
	Valueable<double> RetargetSelf_Probability;
	Valueable<double> AirburstSpread;
	Nullable<bool> AroundTarget;
	ValueableVector<WeaponTypeClass*> AirburstWeapons;
	Valueable<bool> Airburst_UseCluster;
	Valueable<bool> Airburst_RandomClusters;
	Valueable<int> Splits_TargetCellRange;
	Valueable<bool> Splits_UseWeaponTargeting;
	Valueable<bool> AirburstWeapon_ApplyFirepowerMult;
	Valueable<double> Splits_Range;
	Valueable<bool> Splits_RandomCellUseHarcodedRange;
	Valueable<bool> Splits_TargetingUseVerses;
	Valueable<bool> Splits_FillRemainingClusterWithRandomcells;
	Valueable<int> BounceAmount;
	Valueable<WeaponTypeClass*> BounceHitWeapon;
	Valueable<bool> BounceOnTerrain;
	Valueable<bool> BounceOnBuilding;
	Valueable<bool> BounceOnInfantry;
	Valueable<bool> BounceOnVehicle;
	Valueable<bool> SubjectToTrenches;
	Nullable<double> PreExplodeRange;
	Nullable <double> Trajectory_Speed;
	Nullable<int> Proximity_Range;
	Valueable<bool> IsScalable;
	Nullable<Leptons> BallisticScatterMin;
	Nullable<Leptons> BallisticScatterMax;
	OptionalStruct<const ConvertClass*> ImageConvert;
	Valueable<bool> Parachuted;
	Valueable<int> AnimLength;
	Valueable<bool> Arcing_AllowElevationInaccuracy;
	Valueable<ParticleSystemTypeClass*> AttachedSystem;
	Valueable<WeaponTypeClass*> ReturnWeapon;
	Valueable<bool> ReturnWeapon_ApplyFirepowerMult;
	TrailsReader Trails;
	std::unique_ptr<PhobosTrajectoryType> TrajectoryType;
	Valueable<bool> SubjectToGround;
	Valueable<bool> Airburst_TargetAsSource;
	Valueable<bool> Airburst_TargetAsSource_SkipHeight;
	Valueable<Leptons> AirburstWeapon_SourceScatterMin;
	Valueable<Leptons> AirburstWeapon_SourceScatterMax;
	Valueable<int> EMPulseCannon_InaccurateRadius;
	Valueable<int> Parachuted_FallRate;
	Nullable<int> Parachuted_MaxFallRate;
	Valueable<bool> Parachuted_Remap;
	Valueable<bool> Vertical_AircraftFix;
	Nullable<bool> VerticalInitialFacing;
	Valueable<bool> AU;
#pragma endregion

public:
	BulletTypeExtData(BulletTypeClass* pObj)
		: ObjectTypeExtData(pObj),
		Health(0),
		Armor(),
		Interceptable(false),
		LaserTrail_Types(),
		Gravity(),
		Gravity_HeightFix(false),
		Shrapnel_AffectsGround(false),
		Shrapnel_AffectsBuildings(false),
		Shrapnel_Chance(),
		Shrapnel_UseWeaponTargeting(false),
		Cluster_Scatter_Min(BulletTypeExtData::DefaultBulletScatterMin),
		Cluster_Scatter_Max(BulletTypeExtData::DefaultBulletScatterMax),
		Interceptable_DeleteOnIntercept(false),
		Interceptable_WeaponOverride(nullptr),
		SubjectToLand(),
		SubjectToLand_Detonate(true),
		SubjectToWater(),
		SubjectToWater_Detonate(true),
		AAOnly(false),
		SubjectToSolid(false),
		Solid_Level(0),
		Parachute(),
		MissileROTVar(),
		MissileSafetyAltitude(),
		Splits(false),
		RetargetAccuracy(0.0),
		RetargetOwner(true),
		RetargetSelf_Probability(0.5),
		AirburstSpread(1.5),
		AroundTarget(),
		AirburstWeapons(),
		Airburst_UseCluster(false),
		Airburst_RandomClusters(false),
		Splits_TargetCellRange(3),
		Splits_UseWeaponTargeting(false),
		AirburstWeapon_ApplyFirepowerMult(false),
		Splits_Range(1280.0),
		Splits_RandomCellUseHarcodedRange(true),
		Splits_TargetingUseVerses(true),
		Splits_FillRemainingClusterWithRandomcells(true),
		BounceAmount(0),
		BounceHitWeapon(nullptr),
		BounceOnTerrain(true),
		BounceOnBuilding(false),
		BounceOnInfantry(false),
		BounceOnVehicle(false),
		SubjectToTrenches(true),
		PreExplodeRange(),
		Trajectory_Speed(),
		Proximity_Range(),
		IsScalable(false),
		BallisticScatterMin(),
		BallisticScatterMax(),
		ImageConvert(),
		Parachuted(false),
		AnimLength(0),
		Arcing_AllowElevationInaccuracy(true),
		AttachedSystem(nullptr),
		ReturnWeapon(nullptr),
		ReturnWeapon_ApplyFirepowerMult(false),
		Trails(),
		TrajectoryType(nullptr),
		SubjectToGround(false),
		Airburst_TargetAsSource(false),
		Airburst_TargetAsSource_SkipHeight(false),
		AirburstWeapon_SourceScatterMin(Leptons(0)),
		AirburstWeapon_SourceScatterMax(Leptons(0)),
		EMPulseCannon_InaccurateRadius(0),
		Parachuted_FallRate(1),
		Parachuted_MaxFallRate(),
		Parachuted_Remap(true),
		Vertical_AircraftFix(true),
		VerticalInitialFacing(),
		AU(false)
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