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
#include <Misc/DynamicPatcher/Techno/DriveData/DriveData.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBox.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingToTarget.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupport.h>
#include <Misc/DynamicPatcher/Techno/FighterGuardArea/FighterAreaGuard.h>
#include <Misc/DynamicPatcher/AttachedAffects/Effects/PaintBall/PaintBall.h>
#endif
/*
class AltBuildingExt final : public TExtension<BuildingClass>
{
public:

	AltBuildingExt(BuildingClass* pOwnerObject) : TExtension<BuildingClass>{ pOwnerObject }
	{}

	virtual ~AltBuildingExt() = default;
	virtual size_t GetSize() const { return sizeof(*this); }
	virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
	virtual void LoadFromStream(PhobosStreamReader& Stm) override {}
	virtual void SaveToStream(PhobosStreamWriter& Stm) override {}
	virtual void InitializeConstants() override {}
	virtual void Uninitialize() override {}
};*/

class BulletClass;
class TechnoTypeExt::ExtData;
class TechnoExt
{
public:
	static constexpr size_t Canary = 0x55555555;
	using base_type = TechnoClass;

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		//EventQueue<base_type> GenericFuctions;
		//std::string ID;
		std::unique_ptr<ShieldClass> Shield;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool ReceiveDamage;
		bool LastKillWasTeamTarget;
		TimerStruct	PassengerDeletionTimer;
		int PassengerDeletionCountDown;
		ShieldTypeClass* CurrentShieldType;
		int LastWarpDistance;
		int Death_Countdown;
		AnimTypeClass* MindControlRingAnimType;
		OptionalStruct<int, false> DamageNumberOffset;
		OptionalStruct<int, true> CurrentLaserWeaponIndex;
		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
	// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner;
	#pragma region Otamaa
		bool IsDriverKilled;
		int GattlingDmageDelay;
		bool GattlingDmageSound;
		bool AircraftOpentoppedInitEd;

		std::vector<int> FireSelf_Count;
		std::vector<WeaponTypeClass*> FireSelf_Weapon;
		std::vector<int> FireSelf_ROF;
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
			//, GenericFuctions { }
			//, ID { }
			, Shield {}
			//, BExt { nullptr }
			, LaserTrails {}
			, ReceiveDamage { false }
			, LastKillWasTeamTarget { false }
			, PassengerDeletionTimer {}
			, PassengerDeletionCountDown { -1 }
			, CurrentShieldType { nullptr }
			, LastWarpDistance {}
			, Death_Countdown {-1}
			, MindControlRingAnimType { nullptr }
			, DamageNumberOffset {}
			, CurrentLaserWeaponIndex {}
			, OriginalPassengerOwner{ nullptr }
			, IsDriverKilled { false }
			, GattlingDmageDelay { -1 }
			, GattlingDmageSound { false }
			, AircraftOpentoppedInitEd { false }
			, FireSelf_Count {}
			, FireSelf_Weapon {}
			, FireSelf_ROF {}
#ifdef COMPILE_PORTED_DP_FEATURES
			, aircraftPutOffsetFlag { false }
			, aircraftPutOffset { false }
			, VirtualUnit { false }
			, IsMissileHoming { false }
			, SkipVoice { false }
			, HomingTargetLocation { 0,0,0 }
			, ExtraWeaponTimers { }
			, Trails { }
			, MyGiftBox {}
			, PaintBallState {}
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

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

		ShieldClass* GetShield() const {
			return this->Shield.get();
		}

		//virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
		virtual void Uninitialize() override {

#ifdef COMPILE_PORTED_DP_FEATURES
			ExtraWeaponTimers.clear();

#ifdef ENABLE_HOMING_MISSILE
			if (MissileTargetTracker)
				HomingMissileTargetTracker::Remove(MissileTargetTracker);
#endif
#endif
		}

		void InitFunctionEvents();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static TechnoExt::ExtData* GetExtData(base_type* pThis);

	class ExtContainer final : public Container<TechnoExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			return true;
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool IsActive(TechnoClass* pThis ,bool bCheckEMP = true , bool bCheckDeactivated = false, bool bIgnoreLimbo = false,bool bIgnoreIsOnMap = false, bool bIgnoreAbsorb = false);
	static bool IsAlive(TechnoClass* pThis, bool bIgnoreLimbo = false, bool bIgnoreIsOnMap = false, bool bIgnoreAbsorb = false);

	static inline bool IsOnLimbo(TechnoClass* pThis, bool bIgnore);
	static inline bool IsDeactivated(TechnoClass* pThis, bool bIgnore);
	static inline bool IsUnderEMP(TechnoClass* pThis, bool bIgnore);

	static int GetSizeLeft(FootClass* const pThis);
	static void Stop(TechnoClass* pThis, Mission const& eMission = Mission::Guard);
	static bool IsHarvesting(TechnoClass* pThis);
	static bool HasAvailableDock(TechnoClass* pThis);

	static void InitializeItems(TechnoClass* pThis);
	static void InitializeLaserTrail(TechnoClass* pThis, bool bIsconverted);
	static Matrix3D GetMatrix(FootClass* pThis);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& flh, bool turretFLH = false , const CoordStruct& Overrider = CoordStruct::Empty);
	static std::pair<bool, CoordStruct> GetBurstFLH(TechnoClass* pThis, int weaponIndex);
	static std::pair<bool, CoordStruct> GetInfantryFLH(InfantryClass* pThis, int weaponInde);

	static void FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);

	static void TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo);
	static double GetDamageMult(TechnoClass* pSouce, bool ForceDisable = false);

	static void ApplyMindControlRangeLimit(TechnoClass* pThis);
	static void ApplyInterceptor(TechnoClass* pThis);
	static void ApplySpawn_LimitRange(TechnoClass* pThis);
	static void CheckDeathConditions(TechnoClass* pThis);
	static void ApplyMobileRefinery(TechnoClass* pThis);
	static void ObjectKilledBy(TechnoClass* pThis, TechnoClass* pKiller);
	static int GetEatPassangersTotalTime(TechnoExt::ExtData const* pExt, TechnoTypeExt::ExtData const* pData, FootClass const* pPassenger);
	static void EatPassengers(TechnoClass* pThis);
	static void UpdateSharedAmmo(TechnoClass* pThis);
	static double GetCurrentSpeedMultiplier(FootClass* pThis);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex);
	static void UpdateMindControlAnim(TechnoClass* pThis);
	static bool CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget);
	static void ForceJumpjetTurnToTarget(TechnoClass* pThis);
	static void DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage);
	static void KillSelf(TechnoClass* pThis, bool isPeaceful = false);
	static void KillSelf(TechnoClass* pThis, const KillMethod& deathOption, bool RegisterKill = true);
	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis);
	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawSelectBrd(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry , bool IsDisguised);
	static void SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo);
	static void PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker);
	static void RunFireSelf(TechnoClass* pThis);
	static void KillSlave(TechnoClass* pThis);
	static void GattlingDamage(TechnoClass* pThis);
	static void HandleRemove(TechnoClass* pThis);
};