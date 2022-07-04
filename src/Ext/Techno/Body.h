#pragma once
#include <TechnoClass.h>
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>

#include <Utilities/TemplateDef.h>
//#include <Utilities/EventHandler.h>

#include <New/Entity/ShieldClass.h>
#include <New/Entity/LaserTrailClass.h>

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
	using base_type = TechnoClass;

	class ExtData final : public TExtension<TechnoClass>
	{
	public:
		//EventQueue<base_type> GenericFuctions;
		PhobosFixedString<0x100> ID;
		std::unique_ptr<ShieldClass> Shield;
		/*
		struct TestBuildingExt final : public  TExtensionBranch<BuildingClass>
		{
			bool DeployedTechno;
			int LimboID;
			int GrindingWeapon_LastFiredFrame;
			BuildingClass* CurrentAirFactory;
			int AccumulatedGrindingRefund;
			TimerStruct AutoSellTimer;

			TestBuildingExt() : TExtensionBranch { }
				, DeployedTechno { false }
				, LimboID { -1 }
				, GrindingWeapon_LastFiredFrame { 0 }
				, CurrentAirFactory { nullptr }
				, AccumulatedGrindingRefund { 0 }
				, AutoSellTimer { }
			{ }

			virtual ~TestBuildingExt() = default;

			virtual size_t GetSize() const override { return sizeof(*this); }
			virtual void LoadBranchFromStream(PhobosStreamReader& Stm) override { Serialize(Stm); }
			virtual void SaveBranchToStream(PhobosStreamWriter& Stm) override { Serialize(Stm); }
			virtual void InitializeConstants() override { }
			virtual void Uninitialize() override {}
			virtual void InvalidatePointer(void* ptr, bool bRemoved) override
			{
				auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
				switch (abs)
				{
				case AbstractType::Building:
				{
					AnnounceInvalidPointer(CurrentAirFactory, ptr);
				}
				}
			}

			template <typename T>
			void Serialize(T& Stm)
			{
				Debug::Log("Processing Element From TestBuildingExt ! \n");
				Stm
					.Process(this->DeployedTechno)
					.Process(this->LimboID)
					.Process(this->GrindingWeapon_LastFiredFrame)
					.Process(this->CurrentAirFactory)
					.Process(this->AccumulatedGrindingRefund)
					.Process(this->AutoSellTimer)

					;
			}
		};

		TestBuildingExt* BExt;*/
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool ReceiveDamage;
		bool LastKillWasTeamTarget;
		TimerStruct	PassengerDeletionTimer;
		int PassengerDeletionCountDown;
		ShieldTypeClass* CurrentShieldType;
		int LastWarpDistance;
		int Death_Countdown;
		AnimTypeClass* MindControlRingAnimType;
		Nullable<int> DamageNumberOffset;
		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
	// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner;
	#pragma region Otamaa
		bool IsDriverKilled;
		int GattlingDmageDelay;
		bool GattlingDmageSound;
#ifdef COMPILE_PORTED_DP_FEATURES
		bool aircraftPutOffsetFlag;
		bool aircraftPutOffset;
		bool VirtualUnit;
		bool IsMissileHoming;
		bool SkipVoice;

		CoordStruct HomingTargetLocation;
		PhobosMap<WeaponTypeClass*, TimerStruct> ExtraWeaponTimers;
		//std::unique_ptr<GiftBox> MyGiftBox;
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
		std::unique_ptr<GiftBox> MyGiftBox;
		PaintBall PaintBallState;
		FireWeaponManager MyWeaponManager;
		DriveData MyDriveData;
		AircraftDive MyDiveData;
		JJFacingToTarget MyJJData;
		SpawnSupport MySpawnSuport;
		FighterAreaGuard MyFighterData;

#endif;
	#pragma endregion
		ExtData(TechnoClass* OwnerObject) : TExtension<TechnoClass>(OwnerObject)
			//, GenericFuctions { }
			, ID { }
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
			, OriginalPassengerOwner{ nullptr }
			, IsDriverKilled { false }
			, GattlingDmageDelay { -1 }
			, GattlingDmageSound { false }
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
			, MyWeaponManager { }
			, MyDriveData { }
			, MyDiveData { }
			, MyJJData { }
			, MySpawnSuport { }
			, MyFighterData { }
#endif;
		{ InitializeConstants(); }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::House:
			case AbstractType::Building:
			case AbstractType::Aircraft:
			case AbstractType::Unit:
			case AbstractType::Infantry:
			case AbstractType::Anim:
			{

				if (this->GetShield())
					this->GetShield()->InvalidatePointer(ptr);

				//if (auto pBldExt = BExt)
				//	pBldExt->InvalidatePointer(ptr, bRemoved);

#ifdef COMPILE_PORTED_DP_FEATURES
				MyWeaponManager.InvalidatePointer(ptr, bRemoved);
#endif;
				AnnounceInvalidPointer(OriginalPassengerOwner, ptr);
			}
			}
		}

		ShieldClass* GetShield() const {
			return this->Shield.get();
		}

		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
		virtual void Uninitialize() override {
#ifdef COMPILE_PORTED_DP_FEATURES
			ExtraWeaponTimers.clear();
#endif
		//	if (BExt)
			//	GameDelete(BExt);
		}

		void InitFunctionEvents();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	_declspec(noinline) static TechnoExt::ExtData* GetExtData(base_type* pThis)
	{
		return  (pThis && (pThis->WhatAmI() == AbstractType::Building
				|| pThis->WhatAmI() == AbstractType::Unit
				|| pThis->WhatAmI() == AbstractType::Aircraft
				|| pThis->WhatAmI() == AbstractType::Infantry ))
			? reinterpret_cast<TechnoExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	class ExtContainer final : public TExtensionContainer<TechnoExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
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
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct flh, bool turretFLH = false , CoordStruct Overrider = CoordStruct::Empty);
	static std::pair<bool, CoordStruct> GetBurstFLH(TechnoClass* pThis, int weaponIndex);
	static std::pair<bool, CoordStruct> GetInfantryFLH(InfantryClass* pThis, int weaponInde);

	static void FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);

	static void TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo);
	static double GetDamageMult(TechnoClass* pSouce, bool ForceDisable = false);

	static void ApplyMindControlRangeLimit(TechnoClass* pThis);
	static void ApplyInterceptor(TechnoClass* pThis);
	static void ApplySpawn_LimitRange(TechnoClass* pThis);
	static void CheckDeathConditions(TechnoClass* pThis);
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
	static void ApplyGainedSelfHeal(TechnoClass* pThis);
	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawSelectBrd(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry , bool IsDisguised);

	static void PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker);
};