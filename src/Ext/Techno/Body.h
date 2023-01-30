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
#ifndef ENABLE_NEWEXT
	//static constexpr size_t ExtOffset = 0x4FC;
	static constexpr size_t ExtOffset = 0x34C;
#endif

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		//EventQueue<base_type> GenericFuctions;
		TechnoTypeClass* Type;
		OptionalStruct<AbstractType, true> AbsType;
		std::unique_ptr<ShieldClass> Shield;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool ReceiveDamage;
		bool LastKillWasTeamTarget;
		TimerStruct	PassengerDeletionTimer;
		int PassengerDeletionCountDown;
		ShieldTypeClass* CurrentShieldType;
		int LastWarpDistance;
		CDTimerClass Death_Countdown;
		AnimTypeClass* MindControlRingAnimType;
		OptionalStruct<int, false> DamageNumberOffset;
		OptionalStruct<int, true> CurrentLaserWeaponIndex;
		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
		// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner;
		AnimClass* DelayedFire_Anim;
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
		TimerStruct EngineerCaptureDelay;
		bool FlhChanged;
		DynamicVectorClass<LineTrail*> TechnoLineTrail;
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
		PhobosMap<WeaponTypeClass*, TimerStruct> ExtraWeaponTimers;
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
		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject)
			, Type { nullptr }
			, AbsType {}
			, Shield {}
			, LaserTrails {}
			, ReceiveDamage { false }
			, LastKillWasTeamTarget { false }
			, PassengerDeletionTimer {}
			, PassengerDeletionCountDown { -1 }
			, CurrentShieldType { nullptr }
			, LastWarpDistance {}
			, Death_Countdown {}
			, MindControlRingAnimType { nullptr }
			, DamageNumberOffset {}
			, CurrentLaserWeaponIndex {}
			, OriginalPassengerOwner{ nullptr }
			, DelayedFire_Anim { nullptr }
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
			, TechnoLineTrail { }
			, IsMissisleSpawn { false }
			, LastAttacker { nullptr }
			, Attempt { 5 }
			, ReceiveDamageMultiplier { }
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
#endif;
		{ }

		virtual ~ExtData()
		{
#ifdef COMPILE_PORTED_DP_FEATURES
			ExtraWeaponTimers.clear();

#ifdef ENABLE_HOMING_MISSILE
			if (MissileTargetTracker)
				HomingMissileTargetTracker::Remove(MissileTargetTracker);
#endif
#endif
			TechnoExt::ResetDelayFireAnim(this->Get());
		}

		void InvalidatePointer(void* ptr, bool bRemoved);

		ShieldClass* GetShield() const {
			return this->Shield.get();
		}

		//virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();

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

	private:
		template <typename T>
		void Serialize(T& Stm);
	protected:
		std::pair<std::vector<WeaponTypeClass*>, std::vector<int>> GetFireSelfData();
		int GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData , FootClass const* pPassenger);
	};

	class ExtContainer final : public Container<TechnoExt
#ifndef ENABLE_NEWEXT
, true
, true
#endif
	>

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

	static inline bool IsOnLimbo(TechnoClass* pThis, bool bIgnore);
	static inline bool IsDeactivated(TechnoClass* pThis, bool bIgnore);
	static inline bool IsUnderEMP(TechnoClass* pThis, bool bIgnore);

	static int GetSizeLeft(FootClass* const pThis);
	static void Stop(TechnoClass* pThis, Mission const& eMission = Mission::Guard);
	static bool IsHarvesting(TechnoClass* pThis);
	static bool HasAvailableDock(TechnoClass* pThis);

	static Matrix3D GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey = nullptr);
	static Matrix3D TransformFLHForTurret(TechnoClass* pThis, const Matrix3D& mtx, bool isOnTurret);
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

	static void DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage);
	static void KillSelf(TechnoClass* pThis, bool isPeaceful = false);
	static void KillSelf(TechnoClass* pThis, const KillMethod& deathOption, bool RegisterKill = true);
	static void ForceJumpjetTurnToTarget(TechnoClass* pThis);
	static bool CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex);
	static double GetCurrentSpeedMultiplier(FootClass* pThis);
	static void FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);
	static bool ReplaceArmor(REGISTERS* R, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static void UpdateSharedAmmo(TechnoClass* pThis);

	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis);

	static void ResetDelayFireAnim(TechnoClass* pThis);

	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawSelectBrd(const TechnoClass* pThis, TechnoTypeClass* pType, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry , bool IsDisguised);
	static void SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo);
	static void PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker);
	static void HandleRemove(TechnoClass* pThis , TechnoClass* pSource = nullptr);
	static void PutPassengersInCoords(TechnoClass* pTransporter, const CoordStruct& nCoord, AnimTypeClass* pAnimToPlay, int nSound, bool bForce);
	static int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback = true);

	static std::pair<WeaponTypeClass*, int> GetDeployFireWeapon(TechnoClass* pThis , AbstractClass* pTarget);

	static NOINLINE bool IsChronoDelayDamageImmune(FootClass* pThis);
	static NOINLINE int GetInitialStrength(TechnoTypeClass* pType, int nHP);

	struct Helper {
		template<bool CheckHouse ,bool CheckVisibility>
		static NOINLINE std::pair<TechnoTypeClass*,HouseClass*> GetDisguiseType(TechnoClass* pTarget)
		{

			HouseClass* pHouseOut = pTarget->GetOwningHouse();
			TechnoTypeClass* pTypeOut = pTarget->GetTechnoType();

			bool bIsVisible = true;
			if constexpr (CheckVisibility)
				bIsVisible = pTarget->IsClearlyVisibleTo(HouseClass::CurrentPlayer);

			if (pTarget->IsDisguised() && !bIsVisible) {

				if constexpr (CheckHouse) {
					if(const auto pDisguiseHouse = pTarget->GetDisguiseHouse(false))
						if(pDisguiseHouse->Type)
							pHouseOut = pDisguiseHouse;
				}

				if(const auto pDisguiseType = type_cast<TechnoTypeClass*, true>(pTarget->GetDisguise(false)))
					pTypeOut = pDisguiseType;
			}

			return { pTypeOut, pHouseOut };
		}

	};

	static CoordStruct PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts = 1);
	static bool EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select);
	static bool EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select);
	static CoordStruct GetPutLocation(CoordStruct current, int distance);
};