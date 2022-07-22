#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/LaserTrailTypeClass.h>
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif
class PhobosTrajectoryType;
class BulletTypeExt
{
public:
	using base_type = BulletTypeClass;

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
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsReader Trails;
#endif
		#pragma endregion

		PhobosTrajectoryType* TrajectoryType;
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

			, BounceAmount { 0 }
			, BounceHitWeapon { nullptr }
			, BounceOnTerrain { true }
			, BounceOnBuilding { false }
			, BounceOnInfantry { false }
			, BounceOnVehicle { false }

			, PreExplodeRange { }
			, Trajectory_Speed { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
			, TrajectoryType { nullptr }
		{ }

		virtual ~ExtData() = default;
		//virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override { LaserTrail_Types.reserve(1); }
		virtual void Uninitialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		bool HasSplitBehavior();

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

	_declspec(noinline) static BulletTypeExt::ExtData* GetExtData(base_type* pThis);

	class ExtContainer final : public Container<BulletTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static double GetAdjustedGravity(BulletTypeClass* pType);
	static BulletTypeClass* GetDefaultBulletType();
};
