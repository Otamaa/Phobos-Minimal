#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>

#include <Ext/LineTrail/Body.h>

#include <New/Type/LaserTrailTypeClass.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

class BulletTypeExt
{
public:
	static const Leptons DefaultBulletScatterMin;
	static const Leptons DefaultBulletScatterMax;

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xF00DF00D;
		using base_type = BulletTypeClass;
		//static constexpr size_t ExtOffset = 0x2EC;

	public:

		Valueable<int> Health;
		Nullable<Armor> Armor;
		Valueable<bool> Interceptable;
		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
		Nullable<double> Gravity;
		Valueable<bool> Gravity_HeightFix;
		Valueable<bool> Shrapnel_AffectsGround;
		Valueable<bool> Shrapnel_AffectsBuildings;
		Nullable<Leptons> Cluster_Scatter_Min;
		Nullable<Leptons> Cluster_Scatter_Max;

		// Ares 0.7
		Nullable<Leptons> BallisticScatter_Min;
		Nullable<Leptons> BallisticScatter_Max;
		Valueable<bool> Interceptable_DeleteOnIntercept;
		Nullable<WeaponTypeClass*> Interceptable_WeaponOverride;

		Nullable<bool> SubjectToLand;
		Valueable<bool> SubjectToLand_Detonate;
		Nullable<bool> SubjectToWater;
		Valueable<bool> SubjectToWater_Detonate;

		Valueable<bool> AAOnly;

		// solid
		Valueable<bool> SubjectToSolid;
		Valueable<int> Solid_Level;

		#pragma region Otamaa
		Nullable<AnimTypeClass*> Parachute;
		Nullable<double> MissileROTVar;
		Nullable<int> MissileSafetyAltitude;

		Valueable<bool> Splits;
		Valueable<double> RetargetAccuracy;
		Valueable<bool> RetargetOwner;
		Valueable<double> AirburstSpread;
		Nullable<bool> AroundTarget; // aptly named, for both Splits and Airburst, defaulting to Splits

		ValueableVector<WeaponTypeClass*> AirburstWeapons;

		Valueable<double> Splits_Range;
		Valueable<bool> Splits_RandomCellUseHarcodedRange;
		Valueable<bool> Splits_TargetingUseVerses;
		Valueable<bool> Splits_FillRemainingClusterWithRandomcells;

		//
		Valueable<int> BounceAmount;
		Valueable<WeaponTypeClass*> BounceHitWeapon;
		Valueable<bool> BounceOnTerrain;
		Valueable<bool> BounceOnBuilding;
		Valueable<bool> BounceOnInfantry;
		Valueable<bool> BounceOnVehicle;
		//

		Valueable<bool> SubjectToTrenches; //! if false, this projectile/weapon *always* passes through to the occupants, regardless of UC.PassThrough

		Nullable<double> PreExplodeRange;
		Nullable <double> Trajectory_Speed;
		Nullable<int> Proximity_Range;
		Valueable<bool> IsScalable;
		std::vector<LineTrailData> LineTrailData;

		Nullable<Leptons> BallisticScatterMin;
		Nullable<Leptons> BallisticScatterMax;

		// cache for the image animation's palette convert
		OptionalStruct<const ConvertClass*> ImageConvert;

		TrailsReader Trails;

		#pragma endregion

		std::unique_ptr<PhobosTrajectoryType> TrajectoryType;
		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject)
			, Health { 0 }
			, Armor { }
			, Interceptable { false }
			, LaserTrail_Types {}
			, Gravity {}
			, Gravity_HeightFix { false }
			, Shrapnel_AffectsGround { false }
			, Shrapnel_AffectsBuildings { false }
			, Cluster_Scatter_Min{}
			, Cluster_Scatter_Max{}
			, BallisticScatter_Min{}
			, BallisticScatter_Max{}
			, Interceptable_DeleteOnIntercept { false }
			, Interceptable_WeaponOverride {}

			, SubjectToLand {}
			, SubjectToLand_Detonate { true }
			, SubjectToWater {}
			, SubjectToWater_Detonate { true }
			, AAOnly { false }
			, SubjectToSolid { false }
			, Solid_Level { 0 }
			, Parachute { }
			, MissileROTVar { }
			, MissileSafetyAltitude { }
			, Splits { false }
			, RetargetAccuracy { 0.0 }
			, RetargetOwner { true }
			, AirburstSpread { 1.5 }
			, AroundTarget { }
			, AirburstWeapons { }
			, Splits_Range { 1280.0 }
			, Splits_RandomCellUseHarcodedRange { true }
			, Splits_TargetingUseVerses { true }
			, Splits_FillRemainingClusterWithRandomcells { true }

			, BounceAmount { 0 }
			, BounceHitWeapon { nullptr }
			, BounceOnTerrain { true }
			, BounceOnBuilding { false }
			, BounceOnInfantry { false }
			, BounceOnVehicle { false }

			, SubjectToTrenches { true }

			, PreExplodeRange { }
			, Trajectory_Speed { }
			, Proximity_Range { }
			, IsScalable { false }
			, LineTrailData { }
			, BallisticScatterMin { }
			, BallisticScatterMax { }
			, ImageConvert { }
			, Trails { }
			, TrajectoryType { }
		{ }

		virtual ~ExtData() override = default;
		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void Uninitialize();

		bool HasSplitBehavior();

		double GetMissileROTVar(const RulesClass* const pRules) const
		{
			if (MissileROTVar.isset())
				return MissileROTVar.Get();

			return pRules->MissileROTVar;
		}

		double GetMissileSaveAltitude(const RulesClass* const pRules) const
		{
			if (MissileSafetyAltitude.isset())
				return MissileSafetyAltitude.Get();

			return pRules->MissileSafetyAltitude;
		}

		BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const;
		BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright , bool addDamage) const;

		double GetAdjustedGravity() const
		{
			auto const nGravity = this->Gravity.Get(RulesClass::Instance->Gravity);
			return this->Get()->Floater ? nGravity * 0.5 : nGravity;
		}

		const ConvertClass* GetBulletConvert();
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletTypeExt::ExtData> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static double GetAdjustedGravity(BulletTypeClass* pType);
	static BulletTypeClass* GetDefaultBulletType(const char* pBullet = nullptr);
};
