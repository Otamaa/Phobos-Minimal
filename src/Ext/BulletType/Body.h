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

public:

#pragma region ClassMembers

	Valueable<int> Health { 0 };
	Nullable<Armor> Armor { };
	Valueable<bool> Interceptable { false };
	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types { };
	Nullable<double> Gravity { };
	Valueable<bool> Gravity_HeightFix { false };
	Valueable<bool> Shrapnel_AffectsGround { false };
	Valueable<bool> Shrapnel_AffectsBuildings { false };
	Nullable<double> Shrapnel_Chance { };
	Valueable<bool> Shrapnel_UseWeaponTargeting { false };
	Valueable<Leptons> Cluster_Scatter_Min { BulletTypeExtData::DefaultBulletScatterMin };
	Valueable<Leptons> Cluster_Scatter_Max { BulletTypeExtData::DefaultBulletScatterMax };
	Valueable<bool> Interceptable_DeleteOnIntercept { false };
	Valueable<WeaponTypeClass*> Interceptable_WeaponOverride { nullptr };
	Nullable<bool> SubjectToLand { };
	Valueable<bool> SubjectToLand_Detonate { true };
	Nullable<bool> SubjectToWater { };
	Valueable<bool> SubjectToWater_Detonate { true };
	Valueable<bool> AAOnly { false };
	Valueable<bool> SubjectToSolid { false };
	Valueable<int> Solid_Level { 0 };
	Nullable<AnimTypeClass*> Parachute { };
	Nullable<double> MissileROTVar { };
	Nullable<int> MissileSafetyAltitude { };
	Valueable<bool> Splits { false };
	Valueable<double> RetargetAccuracy { 0.0 };
	Valueable<bool> RetargetOwner { true };
	Valueable<double> RetargetSelf_Probability { 0.5 };
	Valueable<double> AirburstSpread { 1.5 };
	Nullable<bool> AroundTarget { };
	ValueableVector<WeaponTypeClass*> AirburstWeapons { };
	Valueable<bool> Airburst_UseCluster { false };
	Valueable<bool> Airburst_RandomClusters { false };
	Valueable<int> Splits_TargetCellRange { 3 };
	Valueable<bool> Splits_UseWeaponTargeting { false };
	Valueable<bool> AirburstWeapon_ApplyFirepowerMult { false };
	Valueable<double> Splits_Range { 1280.0 };
	Valueable<bool> Splits_RandomCellUseHarcodedRange { true };
	Valueable<bool> Splits_TargetingUseVerses { true };
	Valueable<bool> Splits_FillRemainingClusterWithRandomcells { true };
	Valueable<int> BounceAmount { 0 };
	Valueable<WeaponTypeClass*> BounceHitWeapon { nullptr };
	Valueable<bool> BounceOnTerrain { true };
	Valueable<bool> BounceOnBuilding { false };
	Valueable<bool> BounceOnInfantry { false };
	Valueable<bool> BounceOnVehicle { false };
	Valueable<bool> SubjectToTrenches { true };
	Nullable<double> PreExplodeRange { };
	Nullable <double> Trajectory_Speed { };
	Nullable<int> Proximity_Range { };
	Valueable<bool> IsScalable { false };
	Nullable<Leptons> BallisticScatterMin { };
	Nullable<Leptons> BallisticScatterMax { };
	OptionalStruct<const ConvertClass*> ImageConvert { };
	Valueable<bool> Parachuted { false };
	Valueable<int> AnimLength { 0 };
	Valueable<bool> Arcing_AllowElevationInaccuracy { true };
	Valueable<ParticleSystemTypeClass*> AttachedSystem { nullptr };
	Valueable<WeaponTypeClass*> ReturnWeapon { nullptr };
	Valueable<bool> ReturnWeapon_ApplyFirepowerMult {};
	TrailsReader Trails { };
	std::unique_ptr<PhobosTrajectoryType> TrajectoryType { };
	Valueable<bool> SubjectToGround { };
	Valueable<bool> Airburst_TargetAsSource {};
	Valueable<bool> Airburst_TargetAsSource_SkipHeight {};
	Valueable<Leptons> AirburstWeapon_SourceScatterMin {};
	Valueable<Leptons> AirburstWeapon_SourceScatterMax {};
	Valueable<int> EMPulseCannon_InaccurateRadius {};
	Valueable<int> Parachuted_FallRate { 1 };
	Nullable<int> Parachuted_MaxFallRate {};
	Valueable<bool> Parachuted_Remap { true };
	Valueable<bool> Vertical_AircraftFix { true };
	Nullable<bool> VerticalInitialFacing {};
#pragma endregion

public:

	BulletTypeExtData(BulletTypeClass* pObj) : ObjectTypeExtData(pObj) { }
	BulletTypeExtData(BulletTypeClass* pObj, noinit_t& nn) : ObjectTypeExtData(pObj, nn) { }

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

	virtual BulletTypeClass* This() const override { return reinterpret_cast<BulletTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const BulletTypeClass* This_Const() const override { return reinterpret_cast<const BulletTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true;  }

public:

	COMPILETIMEEVAL bool FORCEDINLINE HasSplitBehavior() const
	{
		// behavior in FS: Splits defaults to Airburst.
		return This()->Airburst || this->Splits;
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

};

class BulletTypeExtContainer final : public Container<BulletTypeExtData>
{
public:
	static BulletTypeExtContainer Instance;

	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(BulletTypeExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(BulletTypeExtData::base_type* key, IStream* pStm) { return true;  };
};

double BulletTypeExtData::GetAdjustedGravity(BulletTypeClass* pType)
{
	return BulletTypeExtContainer::Instance.Find(pType)->GetAdjustedGravity();
}

class NOVTABLE FakeBulletTypeClass : public BulletTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	BulletTypeExtData* _GetExtData() {
		return *reinterpret_cast<BulletTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeBulletTypeClass) == sizeof(BulletTypeClass), "Invalid Size !");