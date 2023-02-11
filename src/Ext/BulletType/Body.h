#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/LineTrail/Body.h>

#include <New/Type/LaserTrailTypeClass.h>
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

class BulletTypeExt
{
public:
	static constexpr size_t Canary = 0xF00DF00D;
	using base_type = BulletTypeClass;
	static constexpr size_t ExtOffset = 0x2EC;

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
		Valueable<int> Health;
		ArmorType Armor;
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
		//
		Valueable<int> BounceAmount;
		Valueable<WeaponTypeClass*> BounceHitWeapon;
		Valueable<bool> BounceOnTerrain;
		Valueable<bool> BounceOnBuilding;
		Valueable<bool> BounceOnInfantry;
		Valueable<bool> BounceOnVehicle;
		//

		Nullable<double> PreExplodeRange;
		Nullable <double> Trajectory_Speed;
		Nullable<int> Proximity_Range;
		Valueable<bool> IsScalable;
		std::vector<LineTrailData> LineTrailData;
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsReader Trails;
#endif
		#pragma endregion

		std::unique_ptr<PhobosTrajectoryType> TrajectoryType;
		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject)
			, Health { 0 }
			, Armor { -1 }
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

			, BounceAmount { 0 }
			, BounceHitWeapon { nullptr }
			, BounceOnTerrain { true }
			, BounceOnBuilding { false }
			, BounceOnInfantry { false }
			, BounceOnVehicle { false }

			, PreExplodeRange { }
			, Trajectory_Speed { }
			, Proximity_Range { }
			, IsScalable { false }
			, LineTrailData { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
			, TrajectoryType { }
		{ }

		virtual ~ExtData() override = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override  { LaserTrail_Types.reserve(1); }

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override { return false; }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

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
		BulletClass* CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright) const;

		double GetAdjustedGravity() const
		{
			auto const nGravity = this->Gravity.Get(RulesGlobal->Gravity);
			return this->Get()->Floater ? nGravity * 0.5 : nGravity;
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static double GetAdjustedGravity(BulletTypeClass* pType);
	static BulletTypeClass* GetDefaultBulletType(const char* pBullet = nullptr);
};
