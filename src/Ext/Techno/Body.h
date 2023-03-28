#pragma once
#include <TechnoClass.h>
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>

#include <Utilities/TemplateDef.h>
//#include <Utilities/EventHandler.h>

#include <New/Entity/ShieldClass.h>
#include <New/Entity/LaserTrailClass.h>
#include <New/Entity/HomingMissileTargetTracker.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/Trails.h>
#include <Misc/DynamicPatcher/CustomWeapon/CustomWeapon.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDive.h>
#include <Misc/DynamicPatcher/Techno/DamageSelf/DamageSelfType.h>
#include <Misc/DynamicPatcher/Techno/DriveData/DriveData.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBox.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingToTarget.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupport.h>
#include <Misc/DynamicPatcher/Techno/FighterGuardArea/FighterAreaGuard.h>
#include <Misc/DynamicPatcher/AttachedAffects/Effects/PaintBall/PaintBall.h>
#endif

class BulletClass;
class TechnoTypeClass;
class REGISTERS;
class TechnoExt
{
public:
	static constexpr size_t Canary = 0x55555555;
	using base_type = TechnoClass;
	//static constexpr size_t ExtOffset = 0x4FC;
	static constexpr size_t ExtOffset = 0x34C;

	class ExtData : public Extension<TechnoClass>
	{
	public:
		TechnoTypeClass* Type;
		OptionalStruct<AbstractType, true> AbsType;
		std::unique_ptr<ShieldClass> Shield;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool ReceiveDamage;
		bool LastKillWasTeamTarget;
		CDTimerClass	PassengerDeletionTimer;
		ShieldTypeClass* CurrentShieldType;
		int LastWarpDistance;
		CDTimerClass Death_Countdown;
		AnimTypeClass* MindControlRingAnimType;
		OptionalStruct<int, false> DamageNumberOffset;
		OptionalStruct<int, true> CurrentLaserWeaponIndex;
		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
		// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner;
		UniqueGamePtr<AnimClass> DelayedFire_Anim;
		int DelayedFire_Anim_LoopCount;
		int DelayedFire_DurationTimer;
		bool IsInTunnel;
		CDTimerClass DeployFireTimer;

		std::vector<TimedWarheadValue<WeaponTypeClass*>> RevengeWeapons;

	#pragma region Otamaa
		bool IsDriverKilled;
		int GattlingDmageDelay;
		bool GattlingDmageSound;
		bool AircraftOpentoppedInitEd;

		std::vector<int> FireSelf_Count;
		CDTimerClass EngineerCaptureDelay;
		bool FlhChanged;
		//DynamicVectorClass<LineTrail*> TechnoLineTrail;
		bool IsMissisleSpawn;
		TechnoClass* LastAttacker;
		int Attempt;
		OptionalStruct<double , true> ReceiveDamageMultiplier;
		bool SkipLowDamageCheck;
#ifdef COMPILE_PORTED_DP_FEATURES
		bool aircraftPutOffsetFlag;
		bool aircraftPutOffset;
		bool VirtualUnit;
		bool IsMissileHoming;
		bool SkipVoice;

		CoordStruct HomingTargetLocation;
		PhobosMap<WeaponTypeClass*, CDTimerClass> ExtraWeaponTimers;
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
		std::unique_ptr<GiftBox> MyGiftBox;
		std::unique_ptr<PaintBall> PaintBallState;
		std::unique_ptr<DamageSelfState> DamageSelfState;
		int CurrentWeaponIdx;

#ifdef ENABLE_HOMING_MISSILE
		HomingMissileTargetTracker* MissileTargetTracker;
#endif
		FireWeaponManager MyWeaponManager;
		DriveData MyDriveData;
		AircraftDive MyDiveData;
		JJFacingToTarget MyJJData;
		SpawnSupport MySpawnSuport;
		FighterAreaGuard MyFighterData;

#endif;
	#pragma endregion

		UniqueGamePtr<AnimClass> AttachedAnim;
		bool KillActionCalled;
		CDTimerClass ToProtectDelay;

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject)
			, Type { nullptr }
			, AbsType {}
			, Shield {}
			, LaserTrails {}
			, ReceiveDamage { false }
			, LastKillWasTeamTarget { false }
			, PassengerDeletionTimer {}
			, CurrentShieldType { nullptr }
			, LastWarpDistance {}
			, Death_Countdown {}
			, MindControlRingAnimType { nullptr }
			, DamageNumberOffset {}
			, CurrentLaserWeaponIndex {}
			, OriginalPassengerOwner{ nullptr }
			, DelayedFire_Anim { }
			, DelayedFire_Anim_LoopCount { 1 }
			, DelayedFire_DurationTimer { 0 }
			, IsInTunnel { false }
			, DeployFireTimer {}
			, RevengeWeapons {}
			, IsDriverKilled { false }
			, GattlingDmageDelay { -1 }
			, GattlingDmageSound { false }
			, AircraftOpentoppedInitEd { false }
			, FireSelf_Count {}
			, EngineerCaptureDelay {}
			, FlhChanged { false }
			//, TechnoLineTrail { }
			, IsMissisleSpawn { false }
			, LastAttacker { nullptr }
			, Attempt { 5 }
			, ReceiveDamageMultiplier { }
			, SkipLowDamageCheck { false }
#ifdef COMPILE_PORTED_DP_FEATURES
			, aircraftPutOffsetFlag { false }
			, aircraftPutOffset { false }
			, VirtualUnit { false }
			, IsMissileHoming { false }
			, SkipVoice { false }
			, HomingTargetLocation { 0,0,0 }
			, ExtraWeaponTimers {}
			, Trails {}
			, MyGiftBox {}
			, PaintBallState {}
			, DamageSelfState {}
			, CurrentWeaponIdx { -1}


#ifdef ENABLE_HOMING_MISSILE
			, MissileTargetTracker { nullptr }
#endif
			, MyWeaponManager { }
			, MyDriveData { }
			, MyDiveData { }
			, MyJJData { }
			, MySpawnSuport { }
			, MyFighterData { }
			, AttachedAnim { }
			, KillActionCalled { false }
			, ToProtectDelay { }
#endif;
		{ 		
			MyWeaponManager.CWeaponManager = std::make_unique<CustomWeaponManager>();
		}

		virtual ~ExtData() override = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override;

		ShieldClass* GetShield() const {
			return this->Shield.get();
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;

		bool CheckDeathConditions();
		bool UpdateKillSelf_Slave();

		void UpdateEatPassengers();
		void UpdateMindControlAnim();

		void UpdateGattlingOverloadDamage();
		void UpdateOnTunnelEnter();
		void InitFunctionEvents();

		void UpdateShield();
		void UpdateType(TechnoTypeClass* currentType);
		void UpdateBuildingLightning();
		void UpdateInterceptor();
		void UpdateFireSelf();
		void UpdateMobileRefinery();
		void UpdateMCRangeLimit();
		void UpdateSpawnLimitRange();
		void UpdateDelayFireAnim();
		void UpdateRevengeWeapons();

		void UpdateLaserTrails();
		//
		void UpdateAircraftOpentopped();
		
		bool NOINLINE IsInterceptor();
	
private:
		template <typename T>
		void Serialize(T& Stm);

	protected:
		std::pair<std::vector<WeaponTypeClass*>*, std::vector<int>*> GetFireSelfData();
		int GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData , FootClass const* pPassenger);
	};

	class ExtContainer final : public Container<TechnoExt>

	{
	public:
		ExtContainer();
		~ExtContainer();

		bool InvalidateExtDataIgnorable(void* const ptr) const {
			return false;
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool IsActive(TechnoClass* pThis ,bool bCheckEMP = true , bool bCheckDeactivated = false, bool bIgnoreLimbo = false,bool bIgnoreIsOnMap = false, bool bIgnoreAbsorb = false);
	static bool IsAlive(TechnoClass* pThis, bool bIgnoreLimbo = false, bool bIgnoreIsOnMap = false, bool bIgnoreAbsorb = false);
	static bool IsInWarfactory(TechnoClass* pThis);

	static bool IsCrushable(ObjectClass* pVictim, TechnoClass* pAttacker);

	static inline bool IsOnLimbo(TechnoClass* pThis, bool bIgnore);
	static inline bool IsDeactivated(TechnoClass* pThis, bool bIgnore);
	static inline bool IsUnderEMP(TechnoClass* pThis, bool bIgnore);

	static int GetSizeLeft(FootClass* const pThis);
	static void Stop(TechnoClass* pThis, Mission const& eMission = Mission::Guard);
	static bool IsHarvesting(TechnoClass* pThis);
	static bool HasAvailableDock(TechnoClass* pThis);

	static Matrix3D GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey = nullptr);
	static void TransformFLHForTurret(TechnoClass* pThis, Matrix3D& mtx, bool isOnTurret);
	static Matrix3D GetFLHMatrix(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret);
	static CoordStruct GetFLHAbsoluteCoordsB(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& flh, bool turretFLH = false , const CoordStruct& Overrider = CoordStruct::Empty);
	static std::pair<bool, CoordStruct> GetBurstFLH(TechnoClass* pThis, int weaponIndex);
	static std::pair<bool, CoordStruct> GetInfantryFLH(InfantryClass* pThis, int weaponInde);

	static void TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo);
	static double GetDamageMult(TechnoClass* pSouce, bool ForceDisable = false);

	static void InitializeItems(TechnoClass* pThis ,TechnoTypeClass* pType);
	static void InitializeLaserTrail(TechnoClass* pThis, bool bIsconverted);

	static void ObjectKilledBy(TechnoClass* pThis, TechnoClass* pKiller);

	static void DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage , WarheadTypeClass* pWH);
	static void KillSelf(TechnoClass* pThis, bool isPeaceful = false);
	static void KillSelf(TechnoClass* pThis, const KillMethod& deathOption, bool RegisterKill = true, AnimTypeClass* pVanishAnim = nullptr);
	static void ForceJumpjetTurnToTarget(TechnoClass* pThis);
	static bool CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex);
	static double GetCurrentSpeedMultiplier(FootClass* pThis);
	static double GetROFMult(TechnoClass const* pTech);
	static bool FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);
	static bool ReplaceArmor(REGISTERS* R, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static void UpdateSharedAmmo(TechnoClass* pThis);

	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis);
	static void ApplyDrainMoney(TechnoClass* pThis);
	static void ResetDelayFireAnim(TechnoClass* pThis);

	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawSelectBrd(const TechnoClass* pThis, TechnoTypeClass* pType, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry , bool IsDisguised);
	static void SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo);
	static void PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker);
	static void HandleRemove(TechnoClass* pThis , TechnoClass* pSource = nullptr , bool SkipTrackingRemove = false);
	static void PutPassengersInCoords(TechnoClass* pTransporter, const CoordStruct& nCoord, AnimTypeClass* pAnimToPlay, int nSound, bool bForce);
	static int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback = true , bool allowAAFallback = true);

	static std::pair<WeaponTypeClass*, int> GetDeployFireWeapon(TechnoClass* pThis , AbstractClass* pTarget);

	static int GetInitialStrength(TechnoTypeClass* pType, int nHP);

	static std::pair<TechnoTypeClass*,HouseClass*> GetDisguiseType(TechnoClass* pTarget , bool CheckHouse , bool CheckVisibility);

	static CoordStruct PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts = 1);
	static bool EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select);
	static bool EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select);
	static CoordStruct GetPutLocation(CoordStruct current, int distance);

	static bool AllowedTargetByZone(TechnoClass* pThis, TechnoClass* pTarget, const TargetZoneScanType& zoneScanType, WeaponTypeClass* pWeapon = nullptr, std::optional<std::reference_wrapper<const ZoneType>> zone = std::nullopt);

	static void UpdateMCOverloadDamage(TechnoClass* pOwner);
	static ObjectTypeClass* SetInfDefaultDisguise(TechnoClass* const pThis, TechnoTypeClass* const pType);
	static bool IsCritImmune(TechnoClass* pThis);
	static bool IsPsionicsImmune(TechnoClass* pThis);
	static bool IsCullingImmune(TechnoClass* pThis);
	static bool IsEMPImmune(TechnoClass* pThis);
	static bool IsChronoDelayDamageImmune(FootClass* pThis);
	static bool IsRadImmune(TechnoClass* pThis);

	static bool IsPsionicsWeaponImmune(TechnoClass* pThis);
	static bool IsPoisonImmune(TechnoClass* pThis);
	static bool IsBerserkImmune(TechnoClass* pThis);

	static bool HasAbility(TechnoClass* pThis , PhobosAbilityType nType);

	static bool ObjectHealthAllowFiring(ObjectClass* pTargetObj, WeaponTypeClass* pWeapon);
	static bool CheckCellAllowFiring(CellClass* pCell, WeaponTypeClass* pWeapon);
	static bool TechnoTargetAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static bool FireOnceAllowFiring(TechnoClass* pThis, WeaponTypeClass* pWeapon, AbstractClass* pTarget);
	static bool CheckFundsAllowFiring(TechnoClass* pThis, WarheadTypeClass* pWH);
	static bool InterceptorAllowFiring(TechnoClass* pThis, ObjectClass* pTarget);
	static bool TargetTechnoShieldAllowFiring(TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static bool TargetFootAllowFiring(TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static std::pair<TechnoClass*, CellClass*> TechnoExt::GetTargets(ObjectClass* pObjTarget, AbstractClass* pTarget);
	static int GetDeployFireWeapon(UnitClass* pThis);

	static void SetMissionAfterBerzerk(TechnoClass* pThis ,bool Immediete = false);

	static AreaFireReturnFlag ApplyAreaFire(TechnoClass* pThis, CellClass*& pTargetCell, WeaponTypeClass* pWeapon);
	static int GetThreadPosed(TechnoClass* pThis);

	static bool IsReallyTechno(TechnoClass* pThis);
protected:
	static const std::vector<std::vector<CoordStruct>>* PickFLHs(TechnoClass* pThis);
	static const Nullable<CoordStruct>* GetInfrantyCrawlFLH(InfantryClass* pThis, int weaponIndex);
};