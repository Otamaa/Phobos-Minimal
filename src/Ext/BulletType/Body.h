#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>

#include <Utilities/TemplateDefB.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/SavegameDef.h>

#include <New/Type/LaserTrailTypeClass.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

class BulletTypeExtData final
{
public:
	static constexpr size_t Canary = 0xF00DF00D;
	using base_type = BulletTypeClass;

	static constexpr size_t ExtOffset = 0x2C4; //ares

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
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

	// Ares 0.7
	Valueable<bool> Interceptable_DeleteOnIntercept { false };
	Valueable<WeaponTypeClass*> Interceptable_WeaponOverride { nullptr };

	Nullable<bool> SubjectToLand { };
	Valueable<bool> SubjectToLand_Detonate { true };
	Nullable<bool> SubjectToWater { };
	Valueable<bool> SubjectToWater_Detonate { true };

	Valueable<bool> AAOnly { false };

	// solid
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
	Nullable<bool> AroundTarget { }; // aptly named, for both Splits and Airburst, defaulting to Splits

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

	//
	Valueable<int> BounceAmount { 0 };
	Valueable<WeaponTypeClass*> BounceHitWeapon { nullptr };
	Valueable<bool> BounceOnTerrain { true };
	Valueable<bool> BounceOnBuilding { false };
	Valueable<bool> BounceOnInfantry { false };
	Valueable<bool> BounceOnVehicle { false };
	//

	Valueable<bool> SubjectToTrenches { true }; //! if false, this projectile/weapon *always* passes through to the occupants, regardless of UC.PassThrough

	Nullable<double> PreExplodeRange { };
	Nullable <double> Trajectory_Speed { };
	Nullable<int> Proximity_Range { };
	Valueable<bool> IsScalable { false };
	//std::vector<LineTrailData> LineTrailData { };

	Nullable<Leptons> BallisticScatterMin { };
	Nullable<Leptons> BallisticScatterMax { };

	// cache for the image animation's palette convert
	OptionalStruct<const ConvertClass*> ImageConvert { };

	Valueable<bool> Parachuted { false };
	Valueable<int> AnimLength { 0 };

	Valueable<bool> Arcing_AllowElevationInaccuracy { true };

	Valueable<ParticleSystemTypeClass*> AttachedSystem { nullptr };
	Valueable<WeaponTypeClass*> ReturnWeapon { nullptr };

	TrailsReader Trails { };

	std::unique_ptr<PhobosTrajectoryType> TrajectoryType { };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	constexpr bool FORCEINLINE HasSplitBehavior() const
	{
		// behavior in FS: Splits defaults to Airburst.
		return this->AttachedToObject->Airburst || this->Splits;
	}

	constexpr double FORCEINLINE GetMissileROTVar(const RulesClass* const pRules) const
	{
		if (MissileROTVar.isset())
			return MissileROTVar.Get();

		return pRules->MissileROTVar;
	}

	constexpr double FORCEINLINE GetMissileSaveAltitude(const RulesClass* const pRules) const
	{
		if (MissileSafetyAltitude.isset())
			return MissileSafetyAltitude.Get();

		return pRules->MissileSafetyAltitude;
	}

	BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon, bool addDamage, bool SetWeaponType) const;
	BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const;
	BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright, bool addDamage) const;

	constexpr double FORCEINLINE GetAdjustedGravity() const
	{
		const auto nGravity = this->Gravity.Get(RulesClass::Instance->Gravity);
		return this->AttachedToObject->Floater ? nGravity * 0.5 : nGravity;
	}

	const ConvertClass* GetBulletConvert();

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(BulletTypeExtData) -
			(4u //AttachedToObject
			 );
	}

	static FORCEINLINE double GetAdjustedGravity(BulletTypeClass* pType);

	static BulletTypeClass* GetDefaultBulletType();
	static CoordStruct CalculateInaccurate(BulletTypeClass* pBulletType);

private:
	template <typename T>
	void Serialize(T& Stm);
public:
	static constexpr Leptons DefaultBulletScatterMin { 256 };
	static constexpr Leptons DefaultBulletScatterMax { 512 };

};

class BulletTypeExtContainer final : public Container<BulletTypeExtData>
{
public:
	static BulletTypeExtContainer Instance;
	PhobosMap<BulletTypeClass*, BulletTypeExtData*> Map {};

	virtual bool Load(BulletTypeClass* key, IStream* pStm);

	void Clear() {
		this->Map.clear();
	}

	//BulletTypeExtContainer() : Container<BulletTypeExtData> { "BulletTypeClass" }
	//	, Map {}
	//{ }

	//virtual ~BulletTypeExtContainer() override = default;
	//CONSTEXPR_NOCOPY_CLASSB(BulletTypeExtContainer, BulletTypeExtData, "BulletTypeClass");

//private:
//	BulletTypeExtContainer(const BulletTypeExtContainer&) = delete;
//	BulletTypeExtContainer(BulletTypeExtContainer&&) = delete;
//	BulletTypeExtContainer& operator=(const BulletTypeExtContainer& other) = delete;
};

double BulletTypeExtData::GetAdjustedGravity(BulletTypeClass* pType)
{
	return BulletTypeExtContainer::Instance.Find(pType)->GetAdjustedGravity();
}