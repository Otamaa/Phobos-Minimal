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

class BulletClass;
class TechnoTypeClass;
class REGISTERS;
class TechnoExt
{
public:
	class ExtData : public Extension<TechnoClass>
	{
	public:
		static constexpr size_t Canary = 0x55555555;
		using base_type = TechnoClass;
		//static constexpr size_t ExtOffset = 0x4FC;
		static constexpr size_t ExtOffset = 0x34C;

	public:
		TechnoTypeClass* Type { nullptr };
		OptionalStruct<AbstractType, true> AbsType {};
		std::unique_ptr<ShieldClass> Shield {};
		std::vector<LaserTrailClass> LaserTrails {};
		bool ReceiveDamage { false };
		bool LastKillWasTeamTarget { false };
		CDTimerClass PassengerDeletionTimer {};
		ShieldTypeClass* CurrentShieldType { nullptr };
		int LastWarpDistance {};
		CDTimerClass Death_Countdown {};
		AnimTypeClass* MindControlRingAnimType { nullptr };
		OptionalStruct<int, false> DamageNumberOffset {};
		OptionalStruct<int, true> CurrentLaserWeaponIndex {};
		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
		// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner { nullptr };
		UniqueGamePtr<AnimClass> DelayedFire_Anim {};
		int DelayedFire_Anim_LoopCount { 1 };
		int DelayedFire_DurationTimer { 0 };
		bool IsInTunnel { false };
		CDTimerClass DeployFireTimer {};
		CDTimerClass DisableWeaponTimer {};

		std::vector<TimedWarheadValue<WeaponTypeClass*>> RevengeWeapons {};

		bool IsDriverKilled { false };
		int GattlingDmageDelay { -1 };
		bool GattlingDmageSound { false };
		bool AircraftOpentoppedInitEd { false };

		std::vector<int> FireSelf_Count {};
		CDTimerClass EngineerCaptureDelay {};
		bool FlhChanged { false };
		//std::vector<LineTrail*> TechnoLineTrail {};
		bool IsMissisleSpawn { false };
		TechnoClass* LastAttacker { nullptr };
		int Attempt { 5 };
		OptionalStruct<double , true> ReceiveDamageMultiplier {};
		bool SkipLowDamageCheck { false };

		bool aircraftPutOffsetFlag { false };
		bool aircraftPutOffset { false };
		bool VirtualUnit { false };
		bool IsMissileHoming { false };
		bool SkipVoice { false };

		CoordStruct HomingTargetLocation { 0,0,0 };
		PhobosMap<WeaponTypeClass*, CDTimerClass> ExtraWeaponTimers {};
		std::vector<UniversalTrail> Trails {};
		std::unique_ptr<GiftBox> MyGiftBox {};
		std::unique_ptr<PaintBall> PaintBallState {};
		std::unique_ptr<DamageSelfState> DamageSelfState {};
		int CurrentWeaponIdx { -1 };

#ifdef ENABLE_HOMING_MISSILE
		HomingMissileTargetTracker* MissileTargetTracker { nullptr };
#endif
		FireWeaponManager MyWeaponManager { };
		DriveData MyDriveData { };
		AircraftDive MyDiveData { };
		//JJFacingToTarget MyJJData { };
		SpawnSupport MySpawnSuport { };
		std::unique_ptr<FighterAreaGuard> MyFighterData { };

		UniqueGamePtr<AnimClass> AttachedAnim { };
		bool KillActionCalled { false };
		CDTimerClass WarpedOutDelay { };
		OptionalStruct<bool, true> AltOccupation { }; // if the unit marks cell occupation flags, this is set to whether it uses the "high" occupation members
		TemporalClass* MyOriginalTemporal { nullptr };
		Armor CurrentArmor { Armor::None };
		bool SupressEVALost { false };
		CDTimerClass SelfHealing_CombatDelay { };
		bool PayloadCreated { false };
		SuperClass* LinkedSW { nullptr };
		CellStruct SuperTarget { };

		InfantryTypeClass* HijackerLastDisguiseType { nullptr };
		HouseClass* HijackerLastDisguiseHouse { nullptr };

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void InvalidatePointer(void* ptr, bool bRemoved);
		bool InvalidateIgnorable(void* ptr) const;

		ShieldClass* GetShield() const {
			return this->Shield.get();
		}

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

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

		bool IsInterceptor();
		void CreateInitialPayload();
private:
		template <typename T>
		void Serialize(T& Stm);

	protected:
		std::pair<const std::vector<WeaponTypeClass*>*, const std::vector<int>*> GetFireSelfData();
		int GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData , FootClass const* pPassenger);
	};

	class ExtContainer final : public Container<TechnoExt::ExtData>

	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool IsActive(TechnoClass* pThis ,bool bCheckEMP = true , bool bCheckDeactivated = false, bool bIgnoreLimbo = false,bool bIgnoreIsOnMap = false, bool bIgnoreAbsorb = false);
	static bool IsAlive(TechnoClass* pThis, bool bIgnoreLimbo = false, bool bIgnoreIsOnMap = false, bool bIgnoreAbsorb = false);
	static bool IsInWarfactory(TechnoClass* pThis , bool bCheckNaval = false);

	static bool IsCrushable(ObjectClass* pVictim, TechnoClass* pAttacker);

	static bool IsOnLimbo(TechnoClass* pThis, bool bIgnore);
	static bool IsDeactivated(TechnoClass* pThis, bool bIgnore);
	static bool IsUnderEMP(TechnoClass* pThis, bool bIgnore);

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
	static void HandleRemove(TechnoClass* pThis , TechnoClass* pSource = nullptr , bool SkipTrackingRemove = false , bool Delete = true);
	static void PutPassengersInCoords(TechnoClass* pTransporter, const CoordStruct& nCoord, AnimTypeClass* pAnimToPlay, int nSound, bool bForce);
	static int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback = true , bool allowAAFallback = true);

	static std::pair<WeaponTypeClass*, int> GetDeployFireWeapon(TechnoClass* pThis , AbstractClass* pTarget);

	static int GetInitialStrength(TechnoTypeClass* pType, int nHP);

	static std::pair<TechnoTypeClass*,HouseClass*> GetDisguiseType(TechnoClass* pTarget , bool CheckHouse , bool CheckVisibility, bool bVisibleResult = false);

	static CoordStruct PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts = 1);
	// Return false if the `location` is invalid
	static bool EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select);
	// Always Make Sure `CoordStruct` is valid before using this
	static bool EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select);
	// Return `Empty` if the next location on distance is invalid
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
	static bool IsAbductorImmune(TechnoClass* pThis);
	static bool IsAssaulter(InfantryClass* pThis);
	static bool IsParasiteImmune(TechnoClass* pThis);
	static bool IsUnwarpable(TechnoClass* pThis);
	static bool IsBountyHunter(TechnoClass* pThis);

	static bool HasAbility(TechnoClass* pThis , PhobosAbilityType nType);
	static bool HasImmunity(TechnoClass* pThis, int nType);

	static bool IsCritImmune(Rank vet, TechnoClass* pThis);
	static bool IsPsionicsImmune(Rank vet, TechnoClass* pThis);
	static bool IsCullingImmune(Rank vet, TechnoClass* pThis);
	static bool IsEMPImmune(Rank vet, TechnoClass* pThis);
	static bool IsChronoDelayDamageImmune(Rank vet, FootClass* pThis);
	static bool IsRadImmune(Rank vet, TechnoClass* pThis);
	static bool IsPsionicsWeaponImmune(Rank vet, TechnoClass* pThis);
	static bool IsPoisonImmune(Rank vet, TechnoClass* pThis);
	static bool IsBerserkImmune(Rank vet, TechnoClass* pThis);
	static bool IsAbductorImmune(Rank vet, TechnoClass* pThis);
	static bool IsAssaulter(Rank vet, InfantryClass* pThis);
	static bool IsParasiteImmune(Rank vet, TechnoClass* pThis);
	static bool IsUnwarpable(Rank vet, TechnoClass* pThis);
	static bool IsBountyHunter(Rank vet, TechnoClass* pThis);

	static bool HasAbility(Rank vet , TechnoClass* pThis, PhobosAbilityType nType);
	static bool HasImmunity(Rank vet, TechnoClass* pThis, int nType);

	static bool ObjectHealthAllowFiring(ObjectClass* pTargetObj, WeaponTypeClass* pWeapon);
	static bool CheckCellAllowFiring(CellClass* pCell, WeaponTypeClass* pWeapon);
	static bool TechnoTargetAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static bool FireOnceAllowFiring(TechnoClass* pThis, WeaponTypeClass* pWeapon, AbstractClass* pTarget);
	static bool CheckFundsAllowFiring(TechnoClass* pThis, WarheadTypeClass* pWH);
	static bool InterceptorAllowFiring(TechnoClass* pThis, ObjectClass* pTarget);
	static bool TargetTechnoShieldAllowFiring(TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static bool TargetFootAllowFiring(TechnoClass* pThis , TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static std::pair<TechnoClass*, CellClass*> GetTargets(ObjectClass* pObjTarget, AbstractClass* pTarget);
	static int GetDeployFireWeapon(UnitClass* pThis);

	static void SetMissionAfterBerzerk(TechnoClass* pThis ,bool Immediete = false);

	static AreaFireReturnFlag ApplyAreaFire(TechnoClass* pThis, CellClass*& pTargetCell, WeaponTypeClass* pWeapon);
	static int GetThreadPosed(TechnoClass* pThis);

	static bool IsReallyTechno(TechnoClass* pThis);

	static const std::vector<std::vector<CoordStruct>>* PickFLHs(TechnoClass* pThis);
	static const Nullable<CoordStruct>* GetInfrantyCrawlFLH(InfantryClass* pThis, int weaponIndex);
	static const Armor GetTechnoArmor(TechnoClass* pThis, WarheadTypeClass* pWarhead);

	static bool ConvertToType(FootClass* pThis, TechnoTypeClass* toType);
	static bool IsEligibleSize(TechnoClass* pThis, TechnoClass* pPassanger);
	static bool IsAbductable(TechnoClass* pThis, WeaponTypeClass* pWeapon, FootClass* pFoot);

	static void SendPlane(AircraftTypeClass* Aircraft, size_t Amount , HouseClass* pOwner, Rank SendRank, Mission SendMission, AbstractClass* pTarget, AbstractClass* pDest);
	static bool CreateWithDroppod(FootClass* Object, const CoordStruct& XYZ);

	static void StoreHijackerLastDisguiseData(InfantryClass* pThis , FootClass* pVictim);
	static void RestoreStoreHijackerLastDisguiseData(InfantryClass* pThis , FootClass* pVictim);

	static WeaponTypeClass* GetCurrentWeapon(TechnoClass* pThis, int& weaponIndex, bool getSecondary = false);
	static WeaponTypeClass* GetCurrentWeapon(TechnoClass* pThis, bool getSecondary = false);
};