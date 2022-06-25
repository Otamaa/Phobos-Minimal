#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/HoverTypeClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Techno/ExtraFire/ExtraFireData.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveData.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutData.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxData.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingData.h>
#include <Misc/DynamicPatcher/Techno/Passengers/Passengers.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportData.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Techno/FighterGuardArea/FighterGuardAreaData.h>
#endif

class Matrix3D;

class TechnoTypeExt
{
public:
	using base_type = TechnoTypeClass;

	class ExtData final : public TExtension<TechnoTypeClass>
	{
	public:
		Valueable<bool> HealthBar_Hide;
		Valueable<CSFText> UIDescription;
		Valueable<bool> LowSelectionPriority;
		PhobosFixedString<0x20> GroupAs;
		Valueable<int> RadarJamRadius;
		Nullable<int> InhibitorRange;
		Valueable<Leptons> MindControlRangeLimit;
		Valueable<bool> Interceptor;
		Valueable<AffectedHouse> Interceptor_CanTargetHouses;
		Promotable<Leptons> Interceptor_GuardRange;
		Promotable<Leptons> Interceptor_MinimumGuardRange;
		Valueable<int> Interceptor_Weapon;
		Nullable<bool> Interceptor_DeleteOnIntercept;
		Nullable<WeaponTypeClass*> Interceptor_WeaponOverride;
		Valueable<bool> Interceptor_WeaponReplaceProjectile;
		Valueable<bool> Interceptor_WeaponCumulativeDamage;
		Valueable<bool> Interceptor_KeepIntact;
		Valueable<CoordStruct> TurretOffset;
		Valueable<bool> Powered_KillSpawns;
		Valueable<bool> Spawn_LimitedRange;
		Valueable<int> Spawn_LimitedExtraRange;
		Nullable<int> Spawner_DelayFrames;
		Nullable<bool> Harvester_Counted;
		Valueable<bool> Promote_IncludeSpawns;
		Valueable<bool> ImmuneToCrit;
		Valueable<bool> MultiMindControl_ReleaseVictim;
		Valueable<int> CameoPriority;
		Valueable<bool> NoManualMove;
		Nullable<int> InitialStrength;
		Valueable<bool> PassengerDeletion_Soylent;
		Valueable<double> PassengerDeletion_SoylentMultiplier;
		Valueable<bool> PassengerDeletion_SoylentFriendlies;
		Valueable<int> PassengerDeletion_Rate;
		NullableIdx<VocClass> PassengerDeletion_ReportSound;
		Valueable<bool> PassengerDeletion_Rate_SizeMultiply;
		Valueable<bool> PassengerDeletion_UseCostAsRate;
		Valueable<double> PassengerDeletion_CostMultiplier;
		Nullable<AnimTypeClass*> PassengerDeletion_Anim;
		Valueable<bool> PassengerDeletion_DisplaySoylent;
		Valueable<AffectedHouse> PassengerDeletion_DisplaySoylentToHouses;
		Valueable<Point2D> PassengerDeletion_DisplaySoylentOffset;
		Valueable<bool> Death_NoAmmo;
		Valueable<int> Death_Countdown;
		Valueable<bool> Death_Peaceful;
		Valueable<bool> Death_WithMaster;
		Valueable<ShieldTypeClass*> ShieldType;

		Nullable<AnimTypeClass*> WarpOut;
		Nullable<AnimTypeClass*> WarpIn;
		Nullable<AnimTypeClass*> WarpAway;
		Nullable<bool> ChronoTrigger;
		Nullable<int> ChronoDistanceFactor;
		Nullable<int> ChronoMinimumDelay;
		Nullable<int> ChronoRangeMinimum;
		Nullable<int> ChronoDelay;

		Nullable<WeaponTypeClass*> WarpInWeapon;
		Nullable<WeaponTypeClass*> WarpInMinRangeWeapon;
		Nullable<WeaponTypeClass*> WarpOutWeapon;
		Valueable<bool> WarpInWeapon_UseDistanceAsDamage;

		ValueableVector<AnimTypeClass*> OreGathering_Anims;
		ValueableVector<int> OreGathering_Tiberiums;
		ValueableVector<int> OreGathering_FramesPerDir;
		Valueable<bool> OpenTopped_IgnoreRangefinding;

		std::vector<DynamicVectorClass<CoordStruct>> WeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteWeaponBurstFLHs;

		Valueable<bool> DestroyAnim_Random;
		Valueable<bool> NotHuman_RandomDeathSequence;

		Nullable<InfantryTypeClass*> DefaultDisguise;

		Nullable<int> OpenTopped_RangeBonus;
		Nullable<float> OpenTopped_DamageMultiplier;
		Nullable<int> OpenTopped_WarpDistance;

		Valueable<bool> AutoFire;
		Valueable<bool> AutoFire_TargetSelf;

		Valueable<bool> NoSecondaryWeaponFallback;

		Valueable<int> NoAmmoWeapon;
		Valueable<int> NoAmmoAmount;

		Nullable<bool> JumpjetAllowLayerDeviation;
		Nullable<bool> JumpjetTurnToTarget;

		Valueable<bool> DeployingAnim_AllowAnyDirection;
		Valueable<bool> DeployingAnim_KeepUnitVisible;
		Valueable<bool> DeployingAnim_ReverseForUndeploy;
		Valueable<bool> DeployingAnim_UseUnitDrawer;

		Nullable<SelfHealGainType> SelfHealGainType;

		Valueable<int> ForceWeapon_Naval_Decloaked;

		Valueable<bool> Ammo_Shared;
		Valueable<int> Ammo_Shared_Group;
		Valueable<bool> Passengers_SyncOwner;
		Valueable<bool> Passengers_SyncOwner_RevertOnExit;

		struct LaserTrailDataEntry
		{
			int idxType;
			CoordStruct FLH;
			bool IsOnTurret;

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

			// For some Fcking unknown reason `emplace_back` doesnt knowh the default contructor for this
			LaserTrailDataEntry(int nIdx , const CoordStruct& nFlh , bool OnTur) :
				idxType { nIdx }
				, FLH { nFlh }
				, IsOnTurret { OnTur }
			{ }

			LaserTrailDataEntry() :
				idxType { -1 }
				, FLH { 0,0,0 }
				, IsOnTurret { false }
			{ }
		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		ValueableVector<LaserTrailDataEntry> LaserTrailData;
		Valueable<CSFText> EnemyUIName;
		Valueable<bool> UseDisguiseMovementSpeed;
		Promotable<SHPStruct*> Insignia;
		Promotable<int> InsigniaFrame;
		Nullable<bool> Insignia_ShowEnemy;
		Valueable<Point3D> InsigniaFrames;

		Valueable<Vector2D<double>> InitialStrength_Cloning;

		Nullable<SHPStruct*> SHP_SelectBrdSHP;
		CustomPalette SHP_SelectBrdPAL;
		Nullable<bool> UseCustomSelectBrd;
		Nullable<Point3D> SelectBrd_Frame;
		Nullable<Point2D> SelectBrd_DrawOffset;
		Nullable<int> SelectBrd_TranslucentLevel;
		Nullable<bool> SelectBrd_ShowEnemy;

		Nullable<CoordStruct> PronePrimaryFireFLH;
		Nullable<CoordStruct> ProneSecondaryFireFLH;
		Nullable<CoordStruct> DeployedPrimaryFireFLH;
		Nullable<CoordStruct> DeployedSecondaryFireFLH;

		Nullable<CoordStruct> E_PronePrimaryFireFLH;
		Nullable<CoordStruct> E_ProneSecondaryFireFLH;
		Nullable<CoordStruct> E_DeployedPrimaryFireFLH;
		Nullable<CoordStruct> E_DeployedSecondaryFireFLH;

		std::vector<DynamicVectorClass<CoordStruct>> CrouchedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteCrouchedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> DeployedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteDeployedWeaponBurstFLHs;

#pragma region Otamaa
		Valueable<bool> FacingRotation_Disable;
		Valueable<bool> FacingRotation_DisalbeOnEMP;
		Valueable<bool> FacingRotation_DisalbeOnDeactivated;
		Valueable<bool> Is_Cow;
		Valueable<bool> DontShake;
		NullableIdx<VocClass>DiskLaserChargeUp;
		Nullable<AnimTypeClass*>DrainAnimationType;
		Nullable<float> TalkBubbleTime;
		Nullable <int> AttackingAircraftSightRange;
		NullableIdx<VoxClass>SpyplaneCameraSound;
		Nullable<int> ParadropRadius;
		Nullable<int> ParadropOverflRadius;
		Valueable<bool> Paradrop_DropPassangers;
		Nullable<BYTE> Paradrop_MaxAttempt;
		Valueable<bool> IsCustomMissile;
		Valueable<RocketStruct> CustomMissileData;
		Promotable<bool> CustomMissileRaise;
		Valueable<bool> Draw_MindControlLink;
		NullableVector<int> Overload_Count;
		NullableVector<int> Overload_Damage;
		NullableVector<int> Overload_Frames;
		NullableIdx<VocClass> Overload_DeathSound;
		Nullable<ParticleSystemTypeClass*> Overload_ParticleSys;
		Nullable<int> Overload_ParticleSysCount;
		Nullable<AnimTypeClass*> Landing_Anim;
		Valueable<AnimTypeClass*> Landing_AnimOnWater;
		Nullable<AnimTypeClass*> TakeOff_Anim;
		std::vector<CoordStruct> HitCoordOffset;
		Valueable<bool> HitCoordOffset_Random;
		Promotable<WeaponTypeClass*> DeathWeapon;
		Valueable<WeaponTypeClass*> CrashWeapon_s;
		Promotable<WeaponTypeClass*> CrashWeapon;
		Valueable<bool> Disable_C4WarheadExp;
		Valueable<double> CrashSpinLevelRate;
		Valueable<double> CrashSpinVerticalRate;
		ValueableIdx<VoxClass> ParasiteExit_Sound;

		Nullable<SHPStruct*> PipShapes01;
		Nullable<SHPStruct*> PipShapes02;
		Nullable<SHPStruct*> PipGarrison;
		Valueable<int> PipGarrison_FrameIndex;
		CustomPalette PipGarrison_Palette;

		Valueable<bool> HealthNumber_Show;
		Valueable<bool> HealthNumber_Percent;
		Nullable<Point2D> Healnumber_Offset;
		Nullable<SHPStruct*> HealthNumber_SHP;
		Nullable<Point2D> Healnumber_Decrement;
		Nullable<SHPStruct*> HealthBarSHP;
		Valueable<Point2D> HealthBarSHP_PointOffset;
		Nullable<SHPStruct*> HealthBarSHP_Selected;
		Valueable<int> HealthBarSHPBracketOffset;
		Valueable<CoordStruct> HealthBarSHP_HealthFrame;
		CustomPalette HealthBarSHP_Palette;
		Valueable<bool> HealthbarRemap;

		Nullable<SHPStruct*> GClock_Shape;
		Nullable<int> GClock_Transculency;
		CustomPalette GClock_Palette;

		Valueable<bool> ROF_Random;
		Nullable<Point2D> Rof_RandomMinMax;

		PhobosFixedString<0x32> Eva_Complete;
		ValueableIdx<VocClass> VoiceCreate;
		Valueable<bool>CreateSound_Enable;

		Valueable<bool> SlaveFreeSound_Enable;
		NullableIdx<VocClass>SlaveFreeSound;
		Valueable<bool> NoAirportBound_DisableRadioContact;
		Nullable<AnimTypeClass*> SinkAnim;
		Nullable<double> Tunnel_Speed;
		Nullable<HoverTypeClass*> HoverType;
#ifdef COMPILE_PORTED_DP_FEATURES
		Valueable <bool> VirtualUnit;

		Nullable<CoordStruct> PrimaryCrawlFLH;
		Nullable<CoordStruct> Elite_PrimaryCrawlFLH;
		Nullable<CoordStruct> SecondaryCrawlFLH;
		Nullable<CoordStruct> Elite_SecondaryCrawlFLH;

		Valueable<bool> MissileHoming;
		ExtraFireData MyExtraFireData;
		AircraftDiveData MyDiveData;
		AircraftPutData MyPutData;
		GiftBoxData MyGiftBoxData;
		JJFacingData  MyJJData;
		PassengersData MyPassangersData;
		SpawnSupportFLHData MySpawnSupportFLH;
		SpawnSupportData MySpawnSupportDatas;
		TrailsReader Trails;
		FighterAreaGuardData MyFighterData;
#endif

#pragma endregion
		ExtData(TechnoTypeClass* OwnerObject) : TExtension<TechnoTypeClass>(OwnerObject)

			, HealthBar_Hide { false }
			, UIDescription {}
			, LowSelectionPriority { false }
			, GroupAs { NONE_STR }
			, RadarJamRadius { 0 }
			, InhibitorRange { }
			, MindControlRangeLimit {}
			, Interceptor { false }
			, Interceptor_CanTargetHouses { AffectedHouse::Enemies }
			, Interceptor_GuardRange {}
			, Interceptor_MinimumGuardRange {}
			, Interceptor_Weapon { 0 }
			, Interceptor_DeleteOnIntercept {}
			, Interceptor_WeaponOverride {}
			, Interceptor_WeaponReplaceProjectile { false }
			, Interceptor_WeaponCumulativeDamage { false }
			, Interceptor_KeepIntact { false }
			, TurretOffset { { 0, 0, 0 } }
			, Powered_KillSpawns { false }
			, Spawn_LimitedRange { false }
			, Spawn_LimitedExtraRange { 0 }
			, Spawner_DelayFrames{}
			, Harvester_Counted {}
			, Promote_IncludeSpawns { false }
			, ImmuneToCrit { false }
			, MultiMindControl_ReleaseVictim { false }
			, CameoPriority { 0 }
			, NoManualMove { false }
			, InitialStrength {}
			, PassengerDeletion_Soylent { false }
			, PassengerDeletion_SoylentMultiplier { 1.0 }
			, PassengerDeletion_SoylentFriendlies { false }
			, PassengerDeletion_Rate { 0 }
			, PassengerDeletion_ReportSound {}
			, PassengerDeletion_Rate_SizeMultiply { true }
			, PassengerDeletion_UseCostAsRate{ false }
			, PassengerDeletion_CostMultiplier{ 1.0 }
			, PassengerDeletion_Anim {}
			, PassengerDeletion_DisplaySoylent{ false }
			, PassengerDeletion_DisplaySoylentToHouses{ AffectedHouse::All }
			, PassengerDeletion_DisplaySoylentOffset{ { 0, 0 } }
			, Death_NoAmmo { false }
			, Death_Countdown { 0 }
			, Death_Peaceful { false }
			, Death_WithMaster{ false }
			, ShieldType { nullptr }
			, WarpOut {}
			, WarpIn {}
			, WarpAway {}
			, ChronoTrigger {}
			, ChronoDistanceFactor {}
			, ChronoMinimumDelay {}
			, ChronoRangeMinimum {}
			, ChronoDelay {}
			, WarpInWeapon {}
			, WarpInMinRangeWeapon {}
			, WarpOutWeapon {}
			, WarpInWeapon_UseDistanceAsDamage { false }
			, OreGathering_Anims {}
			, OreGathering_Tiberiums {}
			, OreGathering_FramesPerDir {}
			, WeaponBurstFLHs {}
			, EliteWeaponBurstFLHs {}
			, DestroyAnim_Random { true }
			, NotHuman_RandomDeathSequence { false }
			, DefaultDisguise {}
			, OpenTopped_RangeBonus {}
			, OpenTopped_DamageMultiplier {}
			, OpenTopped_WarpDistance {}
			, OpenTopped_IgnoreRangefinding { false }
			, AutoFire { false }
			, AutoFire_TargetSelf { false }
			, NoSecondaryWeaponFallback { false }
			, NoAmmoWeapon { -1 }
			, NoAmmoAmount { 0 }
			, JumpjetAllowLayerDeviation {}
			, JumpjetTurnToTarget {}
			, DeployingAnim_AllowAnyDirection { false }
			, DeployingAnim_KeepUnitVisible { false }
			, DeployingAnim_ReverseForUndeploy { true }
			, DeployingAnim_UseUnitDrawer { true }
			, SelfHealGainType {}
			, ForceWeapon_Naval_Decloaked { -1 }
			, Ammo_Shared { false }
			, Ammo_Shared_Group { -1 }
			, Passengers_SyncOwner{ false }
			, Passengers_SyncOwner_RevertOnExit{ true }
			, EnemyUIName {}
			, LaserTrailData {}
			, UseDisguiseMovementSpeed {}
			, Insignia {}
			, InsigniaFrame { -1 }
			, Insignia_ShowEnemy {}
			, InsigniaFrames { { -1, -1, -1 } }
			, InitialStrength_Cloning{ { 0.0, 0.0 } }
			, SHP_SelectBrdSHP{  }
			, SHP_SelectBrdPAL{ CustomPalette::PaletteMode::Temperate }
			, UseCustomSelectBrd{}
			, SelectBrd_Frame{ {-1,-1,-1} }
			, SelectBrd_DrawOffset{}
			, SelectBrd_TranslucentLevel{}
			, SelectBrd_ShowEnemy{}

			, PronePrimaryFireFLH{ }
			, ProneSecondaryFireFLH{ }
			, DeployedPrimaryFireFLH{ }
			, DeployedSecondaryFireFLH{ }
			, CrouchedWeaponBurstFLHs{ }

			, E_PronePrimaryFireFLH{ }
			, E_ProneSecondaryFireFLH{ }
			, E_DeployedPrimaryFireFLH{ }
			, E_DeployedSecondaryFireFLH{ }

			, EliteCrouchedWeaponBurstFLHs{ }
			, DeployedWeaponBurstFLHs{ }
			, EliteDeployedWeaponBurstFLHs{ }

#pragma region Otamaa
			, FacingRotation_Disable { false }
			, FacingRotation_DisalbeOnEMP { false }
			, FacingRotation_DisalbeOnDeactivated { false }

			, Is_Cow { false }

			, DontShake { true }

			, DiskLaserChargeUp {}
			, DrainAnimationType {}

			, TalkBubbleTime {}

			, AttackingAircraftSightRange {}

			, SpyplaneCameraSound {}

			, ParadropRadius {}
			, ParadropOverflRadius {}
			, Paradrop_DropPassangers { true }
			, Paradrop_MaxAttempt { }

			, IsCustomMissile { false }
			, CustomMissileRaise { true }
			, CustomMissileData {}

			, Draw_MindControlLink { true }

			, Overload_Count {}
			, Overload_Damage {}
			, Overload_Frames {}
			, Overload_DeathSound {}
			, Overload_ParticleSys {}
			, Overload_ParticleSysCount {}

			, Landing_Anim { }
			, Landing_AnimOnWater { nullptr }
			, TakeOff_Anim { nullptr }

			, HitCoordOffset { }
			, HitCoordOffset_Random { true }

			, DeathWeapon { }
			, CrashWeapon { }
			, CrashWeapon_s { nullptr }
			, Disable_C4WarheadExp { false }



			, CrashSpinLevelRate { 1.0 }
			, CrashSpinVerticalRate { 1.0 }

			, ParasiteExit_Sound {}

			, PipShapes01 {}
			, PipShapes02 {}
			, PipGarrison {}
			, PipGarrison_FrameIndex { 0 }
			, PipGarrison_Palette {}

			, HealthNumber_Show { false }
			, HealthNumber_Percent { false }
			, Healnumber_Offset {}
			, HealthNumber_SHP {}
			, Healnumber_Decrement {}

			, HealthBarSHP {}
			, HealthBarSHP_Selected {}
			, HealthBarSHPBracketOffset { 0 }
			, HealthBarSHP_HealthFrame { { 18,16,17 } }
			, HealthBarSHP_Palette {}
			, HealthBarSHP_PointOffset { { 0,0 } }
			, HealthbarRemap { false }

			, GClock_Shape { }
			, GClock_Transculency { }
			, GClock_Palette { }

			, ROF_Random { true }
			, Rof_RandomMinMax { }

			, Eva_Complete { }
			, VoiceCreate { -1 }
			, CreateSound_Enable { true }

			, SlaveFreeSound_Enable { true }
			, SlaveFreeSound { }
			, NoAirportBound_DisableRadioContact { false }
			, SinkAnim { }
			, Tunnel_Speed { }
			, HoverType { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, VirtualUnit { false }

			, PrimaryCrawlFLH { }
			, Elite_PrimaryCrawlFLH { }
			, SecondaryCrawlFLH { }
			, Elite_SecondaryCrawlFLH { }
			, MissileHoming { false }

			, MyExtraFireData { }
			, MyDiveData { }
			, MyPutData { }
			, MyGiftBoxData { }
			, MyJJData { }
			, MyPassangersData { }
			, MySpawnSupportFLH { }
			, MySpawnSupportDatas { }
			, Trails { }
			, MyFighterData { }
#endif

#pragma endregion

		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void ApplyTurretOffset(Matrix3D* mtx, double factor = 1.0);
		bool IsCountedAsHarvester();

		// Ares 0.A
		const char* GetSelectionGroupID() const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	_declspec(noinline) static TechnoTypeExt::ExtData* GetExtData(base_type* pThis)
	{
		return  (pThis && (pThis->WhatAmI() == AbstractType::BuildingType
			|| pThis->WhatAmI() == AbstractType::UnitType
			|| pThis->WhatAmI() == AbstractType::AircraftType
			|| pThis->WhatAmI() == AbstractType::InfantryType))
			? reinterpret_cast<TechnoTypeExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	class ExtContainer final : public TExtensionContainer<TechnoTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static void ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor = 1.0);

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);

	static void GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection, std::vector<DynamicVectorClass<CoordStruct>>& nFLH, std::vector<DynamicVectorClass<CoordStruct>>& nEFlh, const char* pPrefixTag);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};