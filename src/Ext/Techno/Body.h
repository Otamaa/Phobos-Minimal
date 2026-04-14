#pragma once
#include <TechnoClass.h>
#include <AnimClass.h>

#include <Phobos.Entity.h>

#include <Helpers/Macro.h>

#include <Utilities/TemplateDefB.h>
#include <Utilities/BuildingBrackedPositionData.h>

#include <New/Entity/ShieldClass.h>
#include <New/Entity/LaserTrailClass.h>
#include <New/Entity/PoweredUnitClass.h>
#include <New/Entity/RadarJammerClass.h>
#include <New/Entity/AEProperties.h>
#include <New/Entity/AresAttachedAffects.h>
#include <New/Entity/NewTiberiumStorageClass.h>

#include <New/Type/DigitalDisplayTypeClass.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectClass.h>

#include <TemporalClass.h>
#include <EBolt.h>

#include <Ext/Radio/Body.h>
#include <Ext/FootType/Body.h>

class BulletClass;
class TechnoTypeClass;
class REGISTERS;
struct BurstFLHBundle;
class FakeWeaponTypeClass;
struct HijackerData;

struct TintColors
{
private:

	TechnoClass* Owner;
	int ColorOwner;
	int ColorAllies;
	int ColorEnemies;
	int IntensityOwner;
	int IntensityAllies;
	int IntensityEnemies;


public:

	void SetOwner(TechnoClass* abs) { this->Owner = abs; };

	bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<TintColors*>(this)->Serialize(Stm);
	}

	void Reset()
	{
		this->ColorOwner = 0;
		this->ColorAllies = 0;
		this->ColorEnemies = 0;
		this->IntensityOwner = 0;
		this->IntensityAllies = 0;
		this->IntensityEnemies = 0;
	}

	void Update();

	void GetTints(int* tintColor, int* intensity);

private:

	void Calculate(const int color, const int intensity, const AffectedHouse affectedHouse);

	template <typename T>
	bool FORCEDINLINE Serialize(T& Stm)
	{
		return Stm
			.Process(this->Owner)
			.Process(this->ColorOwner)
			.Process(this->ColorAllies)
			.Process(this->ColorEnemies)
			.Process(this->IntensityOwner)
			.Process(this->IntensityAllies)
			.Process(this->IntensityEnemies)
			.Success()
			;
	}
};

struct OnlyAttackStruct
{
	WeaponTypeClass* Weapon { nullptr };
	TechnoClass* Attacker { nullptr };

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<OnlyAttackStruct*>(this)->Serialize(Stm);
	}

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Weapon)
			.Process(this->Attacker)
			.Success();
	}
};

class TechnoExtData : public RadioExtData
{
private:
	template <typename T>
	void Serialize(T& Stm)
	{
		auto debugProcess = [&Stm](auto& field, const char* fieldName)-> auto&
			{
				if constexpr (std::is_same_v<T, PhobosStreamWriter>)
				{
					//	size_t beforeSize = Stm.Getstream()->Size();
					auto& result = Stm.Process(field);
					//	size_t afterSize = Stm.Getstream()->Size();
					//	GameDebugLog::Log("[TechnoExtData] SAVE %s: size %zu -> %zu (+%zu)\n",
					//		fieldName, beforeSize, afterSize, afterSize - beforeSize);
					return result;
				}
				else
				{
					//	size_t beforeOffset = Stm.Getstream()->Offset();
					//	bool beforeSuccess = Stm.Success();
					auto& result = Stm.Process(field);
					//	size_t afterOffset = Stm.Getstream()->Offset();
					//	bool afterSuccess = Stm.Success();

					//	GameDebugLog::Log("[TechnoExtData] LOAD %s: offset %zu -> %zu (+%zu), success: %s -> %s\n",
					//		fieldName, beforeOffset, afterOffset, afterOffset - beforeOffset,
					//		beforeSuccess ? "true" : "false", afterSuccess ? "true" : "false");

					//	if (!afterSuccess && beforeSuccess)
					//	{
					//		GameDebugLog::Log("[TechnoExtData] ERROR: %s caused stream failure!\n", fieldName);
						//}
					return result;
				}
			};

		debugProcess(this->CurrentType, "CurrentType");
		debugProcess(this->TypeExtData, "OriginalType");
		debugProcess(this->AE, "AE");
		debugProcess(this->idxSlot_EMPulse, "idxSlot_EMPulse");
		debugProcess(this->idxSlot_Wave, "idxSlot_Wave");
		debugProcess(this->idxSlot_Beam, "idxSlot_Beam");
		debugProcess(this->idxSlot_Warp, "idxSlot_Warp");
		debugProcess(this->idxSlot_Parasite, "idxSlot_Parasite");
		debugProcess(this->EMPSparkleAnim, "EMPSparkleAnim");
		debugProcess(this->EMPLastMission, "EMPLastMission");
		debugProcess(this->BuildingLight, "BuildingLight");
		debugProcess(this->OriginalHouseType, "OriginalHouseType");
		debugProcess(this->CloakSkipTimer, "CloakSkipTimer");
		debugProcess(this->HijackerHealth, "HijackerHealth");
		debugProcess(this->HijackerOwner, "HijackerOwner");
		debugProcess(this->HijackerVeterancy, "HijackerVeterancy");
		debugProcess(this->Is_SurvivorsDone, "Is_SurvivorsDone");
		debugProcess(this->Is_DriverKilled, "Is_DriverKilled");
		debugProcess(this->Is_Operated, "Is_Operated");
		debugProcess(this->Is_UnitLostMuted, "Is_UnitLostMuted");
		debugProcess(this->TakeVehicleMode, "TakeVehicleMode");
		debugProcess(this->TechnoValueAmount, "TechnoValueAmount");
		debugProcess(this->Pos, "Pos");
		debugProcess(this->LaserTrails, "LaserTrails");
		debugProcess(this->ReceiveDamage, "ReceiveDamage");
		debugProcess(this->LastKillWasTeamTarget, "LastKillWasTeamTarget");
		debugProcess(this->PassengerDeletionTimer, "PassengerDeletionTimer");
		debugProcess(this->CurrentShieldType, "CurrentShieldType");
		debugProcess(this->LastWarpDistance, "LastWarpDistance");
		debugProcess(this->Death_Countdown, "Death_Countdown");
		debugProcess(this->MindControlRingAnimType, "MindControlRingAnimType");
		debugProcess(this->DamageNumberOffset, "DamageNumberOffset");
		debugProcess(this->CurrentLaserWeaponIndex, "CurrentLaserWeaponIndex");
		debugProcess(this->OriginalPassengerOwner, "OriginalPassengerOwner");
		debugProcess(this->IsInTunnel, "IsInTunnel");
		debugProcess(this->IsBurrowed, "IsBurrowed");
		debugProcess(this->DeployFireTimer, "DeployFireTimer");
		debugProcess(this->DisableWeaponTimer, "DisableWeaponTimer");
		debugProcess(this->RevengeWeapons, "RevengeWeapons");
		debugProcess(this->GattlingDmageDelay, "GattlingDmageDelay");
		debugProcess(this->GattlingDmageSound, "GattlingDmageSound");
		debugProcess(this->AircraftOpentoppedInitEd, "AircraftOpentoppedInitEd");
		debugProcess(this->EngineerCaptureDelay, "EngineerCaptureDelay");
		debugProcess(this->FlhChanged, "FlhChanged");
		debugProcess(this->SkipLowDamageCheck, "SkipLowDamageCheck");
		debugProcess(this->aircraftPutOffsetFlag, "aircraftPutOffsetFlag");
		debugProcess(this->aircraftPutOffset, "aircraftPutOffset");
		debugProcess(this->SkipVoice, "SkipVoice");
		debugProcess(this->ExtraWeaponTimers, "ExtraWeaponTimers");
		debugProcess(this->CurrentWeaponIdx, "CurrentWeaponIdx");
		debugProcess(this->WarpedOutDelay, "WarpedOutDelay");
		debugProcess(this->MyOriginalTemporal, "MyOriginalTemporal");
		debugProcess(this->SupressEVALost, "SupressEVALost");
		debugProcess(this->SelfHealing_CombatDelay, "SelfHealing_CombatDelay");
		debugProcess(this->PayloadCreated, "PayloadCreated");
		debugProcess(this->PayloadTriggered, "PayloadTriggered");
		debugProcess(this->LinkedSW, "LinkedSW");
		debugProcess(this->SuperTarget, "SuperTarget");
		debugProcess(this->HijackerLastDisguiseType, "HijackerLastDisguiseType");
		debugProcess(this->HijackerLastDisguiseHouse, "HijackerLastDisguiseHouse");
		debugProcess(this->WHAnimRemainingCreationInterval, "WHAnimRemainingCreationInterval");
		debugProcess(this->IsWebbed, "IsWebbed");
		debugProcess(this->WebbedAnim, "WebbedAnim");
		debugProcess(this->WebbyLastTarget, "WebbyLastTarget");
		debugProcess(this->WebbyLastMission, "WebbyLastMission");
		debugProcess(this->AeData, "AeData");
		debugProcess(this->MergePreventionTimer, "MergePreventionTimer");
		debugProcess(this->TiberiumStorage, "TiberiumStorage");
		debugProcess(this->PhobosAE, "PhobosAE");
		debugProcess(this->FiringObstacleCell, "FiringObstacleCell");
		debugProcess(this->AdditionalRange, "AdditionalRange");
		debugProcess(this->IsDetachingForCloak, "IsDetachingForCloak");
		debugProcess(this->HasRemainingWarpInDelay, "HasRemainingWarpInDelay");
		debugProcess(this->LastWarpInDelay, "LastWarpInDelay");
		debugProcess(this->SubterraneanHarvRallyPoint, "SubterraneanHarvRallyPoint");
		debugProcess(this->IsBeingChronoSphered, "IsBeingChronoSphered");
		debugProcess(this->TiberiumEaterTimer, "TiberiumEaterTimer");
		debugProcess(this->LastDamageWH, "LastDamageWH");
		debugProcess(this->MyTargetingFrame, "MyTargetingFrame");
		debugProcess(this->ChargeTurretTimer, "ChargeTurretTimer");
		debugProcess(this->LastRearmWasFullDelay, "LastRearmWasFullDelay");
		debugProcess(this->DropCrate, "DropCrate");
		debugProcess(this->DropCrateType, "DropCrateType");
		debugProcess(this->LastBeLockedFrame, "LastBeLockedFrame");
		debugProcess(this->BeControlledThreatFrame, "BeControlledThreatFrame");
		debugProcess(this->LastTargetID, "LastTargetID");
		debugProcess(this->LastHurtFrame, "LastHurtFrame");
		debugProcess(this->AccumulatedGattlingValue, "AccumulatedGattlingValue");
		debugProcess(this->ShouldUpdateGattlingValue, "ShouldUpdateGattlingValue");
		debugProcess(this->KeepTargetOnMove, "KeepTargetOnMove");
		debugProcess(this->LastSensorsMapCoords, "LastSensorsMapCoords");
		debugProcess(this->DelayedFireSequencePaused, "DelayedFireSequencePaused");
		debugProcess(this->DelayedFireTimer, "DelayedFireTimer");
		debugProcess(this->DelayedFireWeaponIndex, "DelayedFireWeaponIndex");
		debugProcess(this->CurrentDelayedFireAnim, "CurrentDelayedFireAnim");
		debugProcess(this->LastWeaponType, "LastWeaponType");
		debugProcess(this->AirstrikeTargetingMe, "AirstrikeTargetingMe");
		debugProcess(this->RandomEMPTarget, "RandomEMPTarget");
		debugProcess(this->ForceFullRearmDelay, "ForceFullRearmDelay");
		debugProcess(this->AttackMoveFollowerTempCount, "AttackMoveFollowerTempCount");
		debugProcess(this->JumpjetSpeed, "JumpjetSpeed");
		debugProcess(this->OnlyAttackData, "OnlyAttackData");
		debugProcess(this->IsSelected, "IsSelected");
		debugProcess(this->UndergroundTracked, "UndergroundTracked");
		debugProcess(this->PassiveAquireMode, "PassiveAquireMode");
		debugProcess(this->CurrentSubterraneanHarvStatus, "CurrentSubterraneanHarvStatus");
		debugProcess(this->UnitIdleAction, "UnitIdleAction");
		debugProcess(this->UnitIdleActionSelected, "UnitIdleActionSelected");
		debugProcess(this->UnitIdleIsSelected, "UnitIdleIsSelected");
		debugProcess(this->UnitIdleActionTimer, "UnitIdleActionTimer");
		debugProcess(this->UnitIdleActionGapTimer, "UnitIdleActionGapTimer");
		debugProcess(this->LastTargetCrdClearTimer, "LastTargetCrdClearTimer");
		debugProcess(this->LastTargetCrd, "LastTargetCrd");
		debugProcess(this->Tints, "Tints");
		debugProcess(this->FallingDownTracked, "FallingDownTracked");
		debugProcess(this->ResetLocomotor, "ResetLocomotor");
		debugProcess(this->JumpjetStraightAscend, "JumpjetStraightAscend");
		debugProcess(this->CanFireWeaponType, "CanFireWeaponType");
		debugProcess(this->ExtraTurretRecoil, "ExtraTurretRecoil");
		debugProcess(this->ExtraBarrelRecoil, "ExtraBarrelRecoil");
		debugProcess(this->OnParachuted, "OnParachuted");
		debugProcess(this->HoverShutdown, "HoverShutdown");
	}



public:
	using base_type = TechnoClass;
public:
#pragma region ClassMembers

	// ============================================================
	// Shield-related fields (group together for cache locality)
	// ============================================================
	ShieldTypeClass* CurrentShieldType { nullptr };

	// ============================================================
	
	// ============================================================
	// 8-byte aligned: Pointers
	// ============================================================
	TechnoTypeClass* CurrentType { nullptr };
	TechnoTypeExtData* TypeExtData { nullptr };
	BuildingLightClass* BuildingLight { nullptr };
	HouseTypeClass* OriginalHouseType { nullptr };
	HouseClass* HijackerOwner { nullptr };
	HouseClass* OriginalPassengerOwner { nullptr };
	AnimTypeClass* MindControlRingAnimType { nullptr };
	AbstractClass* WebbyLastTarget { nullptr };
	CellClass* FiringObstacleCell { nullptr };
	AbstractClass* SubterraneanHarvRallyPoint { nullptr };
	TemporalClass* MyOriginalTemporal { nullptr };
	SuperClass* LinkedSW { nullptr };
	InfantryTypeClass* HijackerLastDisguiseType { nullptr };
	HouseClass* HijackerLastDisguiseHouse { nullptr };
	WarheadTypeClass* LastDamageWH { nullptr };
	WeaponTypeClass* LastWeaponType { nullptr };
	WeaponTypeClass* CanFireWeaponType { nullptr };
	AirstrikeClass* AirstrikeTargetingMe { nullptr };

	// ============================================================
	// 8-byte aligned: std::unique_ptr
	// ============================================================

	// ============================================================
	// 8-byte aligned: Handle wrappers (pointer-sized)
	// ============================================================
	Handle<AnimClass*, UninitAnim> EMPSparkleAnim { nullptr };
	Handle<AnimClass*, UninitAnim> WebbedAnim { nullptr };
	Handle<AnimClass*, UninitAnim> CurrentDelayedFireAnim { nullptr };

	// ============================================================
	// 8-byte aligned: OptionalStruct (int + bool + padding)
	// ============================================================
	OptionalStruct<int, true> CurrentLaserWeaponIndex {};
	OptionalStruct<int, true> AdditionalRange {};

	// ============================================================
	// 8-byte aligned: std::optional
	// ============================================================

	// ============================================================
	// Large aggregates: Vectors (typically 24 bytes on x64)
	// ============================================================
	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails {};
	HelperedVector<TimedWarheadValue<WeaponTypeClass*>> RevengeWeapons {};
	HelperedVector<std::unique_ptr<PhobosAttachEffectClass>> PhobosAE {};
	HelperedVector<EBolt*> ElectricBolts {};
	HelperedVector<OnlyAttackStruct> OnlyAttackData {};
	std::vector<RecoilData> ExtraTurretRecoil {};
	std::vector<RecoilData> ExtraBarrelRecoil {};

	// ============================================================
	// Large aggregates: Maps
	// ============================================================
	PhobosMap<WeaponTypeClass*, CDTimerClass> ExtraWeaponTimers {};

	// ============================================================
	// Large aggregates: Custom compound types
	// (order these by their internal alignment requirements if known)
	// ============================================================
	AEProperties AE {};
	AresAEData AeData {};
	NewTiberiumStorageClass TiberiumStorage {};
	TintColors Tints {};

	// ============================================================
	// CDTimerClass instances (group together for cache locality)
	// ============================================================
	CDTimerClass CloakSkipTimer {};
	CDTimerClass PassengerDeletionTimer {};
	CDTimerClass Death_Countdown {};
	CDTimerClass DeployFireTimer {};
	CDTimerClass DisableWeaponTimer {};
	CDTimerClass EngineerCaptureDelay {};
	CDTimerClass WarpedOutDelay {};
	CDTimerClass SelfHealing_CombatDelay {};
	CDTimerClass MergePreventionTimer {};
	CDTimerClass TiberiumEaterTimer {};
	CDTimerClass ChargeTurretTimer {};
	CDTimerClass DelayedFireTimer {};
	CDTimerClass UnitIdleActionTimer {};
	CDTimerClass UnitIdleActionGapTimer {};
	CDTimerClass LastTargetCrdClearTimer {};

	//
	CoordStruct LastTargetCrd {};
	// ============================================================
	// CellStruct instances (likely 4-8 bytes each)
	// ============================================================
	CellStruct SuperTarget {};
	CellStruct RandomEMPTarget {};
	CellStruct LastSensorsMapCoords {};

	// ============================================================
	// 4-byte aligned: int, DWORD, float, enums
	// ============================================================
	entt::entity ShieldEntity { entt::null };
	entt::entity PoweredUnitEntity { entt::null };
	entt::entity RadarJammerEntity { entt::null };
	int HijackerHealth {};
	int TechnoValueAmount {};
	int Pos {};
	int LastWarpDistance {};
	int DamageNumberOffset { INT32_MIN };
	int GattlingDmageDelay { -1 };
	int CurrentWeaponIdx { -1 };
	int WHAnimRemainingCreationInterval {};
	int LastWarpInDelay {};
	int MyTargetingFrame {};
	int DropCrate { -1 };
	int LastBeLockedFrame {};
	int BeControlledThreatFrame {};
	int AccumulatedGattlingValue {};
	int DelayedFireWeaponIndex { -1 };
	int LastHurtFrame {};
	int AttackMoveFollowerTempCount;
	int JumpjetSpeed { 14 };

	DWORD LastTargetID { 0xFFFFFFFF };

	float HijackerVeterancy {};

	Mission EMPLastMission { Mission::Sleep };
	Mission WebbyLastMission { Mission::Sleep };
	PowerupEffects DropCrateType { PowerupEffects::Money };
	PassiveAcquireModes PassiveAquireMode { PassiveAcquireModes::Normal };
	SubterraneanHarvStatus CurrentSubterraneanHarvStatus { SubterraneanHarvStatus::None };
	// ============================================================
	// 1-byte aligned: BYTE (group to fill padding)
	// ============================================================
	BYTE idxSlot_EMPulse {};
	BYTE idxSlot_Wave {};
	BYTE idxSlot_Beam {};
	BYTE idxSlot_Warp {};
	BYTE idxSlot_Parasite {};

	BYTE Is_SurvivorsDone {};
	BYTE Is_DriverKilled {};
	BYTE Is_Operated {};
	BYTE Is_UnitLostMuted {};
	BYTE TakeVehicleMode {};

	// ============================================================
	// 1-byte aligned: bool (pack together to minimize padding)
	// ============================================================
	bool ReceiveDamage {};
	bool LastKillWasTeamTarget {};
	bool IsInTunnel {};
	bool IsBurrowed {};
	bool GattlingDmageSound {};
	bool AircraftOpentoppedInitEd {};
	bool FlhChanged {};
	bool SkipLowDamageCheck {};
	bool aircraftPutOffsetFlag {};
	bool aircraftPutOffset {};
	bool SkipVoice {};
	bool SupressEVALost {};
	bool PayloadCreated {};
	bool PayloadTriggered {};
	bool IsWebbed {};
	bool IsDetachingForCloak {};
	bool HasRemainingWarpInDelay {};
	bool IsBeingChronoSphered {};
	bool LastRearmWasFullDelay {};
	bool ShouldUpdateGattlingValue {};
	bool KeepTargetOnMove {};
	bool DelayedFireSequencePaused {};
	bool ForceFullRearmDelay {};
	bool IsSelected {};
	bool UndergroundTracked {};
	bool UnitIdleAction {};
	bool UnitIdleActionSelected {};
	bool UnitIdleIsSelected {};
	bool FallingDownTracked {};
	bool ResetLocomotor {};
	bool JumpjetStraightAscend {};
	bool OnParachuted {};
	bool HoverShutdown {};
	// 33 bools = 33 bytes, add 1 padding byte for 32 (4-byte alignment)

#pragma endregion

public:

	TechnoExtData(TechnoClass* abs) : RadioExtData(abs)
	{
		TiberiumStorage.m_values.resize(TiberiumClass::Array->Count);
		MyTargetingFrame = ScenarioClass::Instance->Random.RandomRanged(0, 15);
		Tints.SetOwner(abs);
		ShieldEntity = PhobosEntity::Create();
		PoweredUnitEntity = PhobosEntity::Create();
		RadarJammerEntity = PhobosEntity::Create();
	};

	TechnoExtData(TechnoClass* abs, noinit_t& noint) : RadioExtData(abs, noint) {};

	virtual ~TechnoExtData();
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->RadioExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		this->RadioExtData::SaveToStream(Stm);
		this->Serialize(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	FORCEDINLINE TechnoClass* This() const { return reinterpret_cast<TechnoClass*>(AttachedToObject); }
	FORCEDINLINE const TechnoClass* This_Const() const { return reinterpret_cast<const TechnoClass*>(AttachedToObject); }

	virtual void CalculateCRC(CRCEngine& crc) const override
	{
		this->RadioExtData::CalculateCRC(crc);
	}

public:

	FORCEDINLINE ShieldClass* GetShield() const
	{
		if (!PhobosEntity::Has<ShieldClass>(this->ShieldEntity))
			return nullptr;

		return PhobosEntity::Get<ShieldClass>(this->ShieldEntity);
	}

	FORCEDINLINE PoweredUnitClass* GetPoweredUnit() const
	{
		if (!PhobosEntity::Has<PoweredUnitClass>(this->PoweredUnitEntity))
			return nullptr;

		return PhobosEntity::Get<PoweredUnitClass>(this->PoweredUnitEntity);
	}

	FORCEDINLINE RadarJammerClass* GetRadarJammer() const
	{
		if (!PhobosEntity::Has<RadarJammerClass>(this->RadarJammerEntity))
			return nullptr;

		return PhobosEntity::Get<RadarJammerClass>(this->RadarJammerEntity);
	}

	void ClearElectricBolts()
	{
		for (auto const pBolt : this->ElectricBolts)
		{
			if (pBolt)
				pBolt->Owner = nullptr;
		}

		this->ElectricBolts.clear();
	}

	void StopIdleAction();
	void ApplyIdleAction();
	void ManualIdleAction();

	bool CheckDeathConditions();
	bool UpdateKillSelf_Slave();
	void UpdateGattlingRateDownReset();
	void UpdateEatPassengers();
	void UpdateMindControlAnim();

	void UpdateGattlingOverloadDamage();
	void UpdateOnTunnelEnter();

	void UpdateShield();
	void UpdateType(TechnoTypeClass* currentType);
	void UpdateBuildingLightning();
	void UpdateInterceptor();
	void UpdateTiberiumEater();
	void UpdateMCRangeLimit();
	void UpdateSpawnLimitRange();
	void UpdateRevengeWeapons();
	void UpdateRearmInEMPState();
	void UpdateRearmInTemporal();
	void UpdateRecountBurst();
	void UpdateLaserTrails();
	//
	void UpdateAircraftOpentopped();

	void DepletedAmmoActions();

	bool IsInterceptor();
	void CreateInitialPayload(bool forced = false);

	void StopRotateWithNewROT(int ROT = -1);

	void ResetDelayedFireTimer();

	void CreateDelayedFireAnim(AnimTypeClass* pAnimType, int weaponIndex, bool attach, bool center, bool removeOnNoDelay, bool useOffsetOverride, CoordStruct offsetOverride);

	void AddFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker);
	bool ContainFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker) const;
	int FindFirer(WeaponTypeClass* const Weapon) const;

	void InitPassiveAcquireMode();
	PassiveAcquireModes GetPassiveAcquireMode() const;
	void TogglePassiveAcquireMode(PassiveAcquireModes mode);
	bool CanTogglePassiveAcquireMode();

	static void InitializeRecoilData(TechnoClass* pThis, TechnoTypeClass* pType);

	void UpdateRecoilData();
	void RecordRecoilData();
	void UpdateLastTargetCrd();

public:

	static void InitializeUnitIdleAction(TechnoClass* pThis, TechnoTypeClass* pType);

	static bool HandleDelayedFireWithPauseSequence(TechnoClass* pThis, WeaponTypeClass* pWeapon, int weaponIndex, int frame, int firingFrame);

	static bool IsOnBridge(FootClass* pUnit);
	static int GetJumpjetIntensity(FootClass* pThis);
	static void GetLevelIntensity(TechnoClass* pThis, int level, int& levelIntensity, int& cellIntensity, double levelMult, double cellMult, bool applyBridgeBonus = false);
	static int GetDeployingAnimIntensity(FootClass* pThis);

	static int CalculateBlockDamage(TechnoClass* pThis, TechnoClass* pSource, int* pDamage, WarheadTypeClass* WH);
	static std::vector<double> GetBlockChance(TechnoClass* pThis, std::vector<double>& blockChance);

protected:
	std::pair<const std::vector<WeaponTypeClass*>*, const std::vector<int>*> GetFireSelfData();
	int GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData, FootClass* pPassenger);

public:
	static bool IsActive(TechnoClass* pThis, bool bCheckEMP = true, bool bCheckDeactivated = false, bool bIgnoreLimbo = false, bool bIgnoreIsOnMap = false, bool bIgnoreAbsorb = false);
	static bool IsAlive(TechnoClass* pThis, bool bIgnoreLimbo = false, bool bIgnoreIsOnMap = false, bool bIgnoreAbsorb = false);
	static bool IsInWarfactory(TechnoClass* pThis, bool bCheckNaval = false);

	static bool IsCrushable(ObjectClass* pVictim, TechnoClass* pAttacker);

	static bool IsOnLimbo(TechnoClass* pThis, bool bIgnore);
	static bool IsDeactivated(TechnoClass* pThis, bool bIgnore);
	static bool IsUnderEMP(TechnoClass* pThis, bool bIgnore);

	static void Stop(TechnoClass* pThis, Mission const& eMission = Mission::Guard);
	static bool IsHarvesting(TechnoClass* pThis);
	static bool HasAvailableDock(TechnoClass* pThis);

	static Matrix3D GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey = nullptr, bool isShadow = false);
	static void TransformFLHForTurret(TechnoClass* pThis, Matrix3D& mtx, bool isOnTurret, double factor = 1.0, int turIdx = -1);
	static Matrix3D GetFLHMatrix(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret, double factor = 1.0, bool isShadow = false, int turIdx = -1);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& flh, bool isOnTurret = false, int turIdx = -1);
	static std::pair<bool, CoordStruct> GetBurstFLH(TechnoClass* pThis, int weaponIndex);
	static std::pair<bool, CoordStruct> GetInfantryFLH(InfantryClass* pThis, int weaponInde);

	static void TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo);
	static double GetDamageMult(TechnoClass* pSouce, double damageIn, bool ForceDisable = false);
	static double GetArmorMult(TechnoClass* pSouce, double damageIn, WarheadTypeClass* pWarhead);

	static void InitializeItems(TechnoClass* pThis, TechnoTypeClass* pType);
	static void InitializeLaserTrail(TechnoClass* pThis, bool bIsconverted);
	static void UpdateLaserTrails(TechnoClass* pThis);
	static void InitializeAttachEffects(TechnoClass* pThis, TechnoTypeClass* pType);

	static void ObjectKilledBy(TechnoClass* pThis, TechnoClass* pKiller);
	static void ObjectKilledBy(TechnoClass* pThis, HouseClass* pKiller);

	static void DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage, WarheadTypeClass* pWH);
	static void Kill(TechnoClass* pThis, TechnoClass* pKiller);
	static void KillSelf(TechnoClass* pThis, bool isPeaceful = false);
	static void KillSelf(TechnoClass* pThis, const KillMethod& deathOption, bool RegisterKill = true, AnimTypeClass* pVanishAnim = nullptr);
	static void ForceJumpjetTurnToTarget(TechnoClass* pThis);
	static bool CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex);
	static double GetCurrentSpeedMultiplier(FootClass* pThis);
	static double GetROFMult(TechnoClass const* pTech);
	static bool FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);

	static void ReplaceArmor(Armor& armor, ObjectClass* pTarget, WeaponTypeClass* pWeapon);
	static void ReplaceArmor(Armor& armor, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static void ReplaceArmor(Armor& armor, ObjectClass* pTarget, WarheadTypeClass* pWH);
	static void ReplaceArmor(Armor& armor, TechnoClass* pTarget, WarheadTypeClass* pWH);

	static void UpdateSharedAmmo(TechnoClass* pThis);

	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds, SHPStruct* shape, ConvertClass* convert);
	static void DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis, bool wasDamaged);
	static void ApplyDrainMoney(TechnoClass* pThis);

	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawSelectBox(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds, bool drawBefore = false);
	//static void DrawSelectBrd(const TechnoClass* pThis, TechnoTypeClass* pType, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry, bool IsDisguised);
	static void SyncInvulnerability(TechnoClass* pFrom, TechnoClass* pTo);
	static void PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker);
	static void HandleRemove(TechnoClass* pThis, TechnoClass* pSource = nullptr, bool SkipTrackingRemove = false, bool Delete = true);
	static void PutPassengersInCoords(TechnoClass* pTransporter, const CoordStruct& nCoord, AnimTypeClass* pAnimToPlay, int nSound, bool bForce);
	static int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback = true, bool allowAAFallback = true);

	static std::pair<WeaponTypeClass*, int> GetDeployFireWeapon(TechnoClass* pThis, AbstractClass* pTarget);

	static int GetInitialStrength(TechnoTypeClass* pType, int nHP);

	static std::pair<TechnoTypeClass*, HouseClass*> GetDisguiseType(TechnoClass* pTarget, bool CheckHouse, bool CheckVisibility, bool bVisibleResult = false);
	static TechnoTypeClass* GetSimpleDisguiseType(TechnoClass* pTarget, bool CheckVisibility, bool bVisibleResult = false);

	static CoordStruct PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts = 1);
	static void EjectPassengers(FootClass* pThis, int howMany);
	// Return false if the `location` is invalid
	static bool EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select, std::optional<bool> InAir = std::nullopt);
	// Always Make Sure `CoordStruct` is valid before using this
	static bool EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select, std::optional<bool> InAir = std::nullopt);
	// Return `Empty` if the next location on distance is invalid
	static CoordStruct GetPutLocation(CoordStruct current, int distance);

	static bool AllowedTargetByZone(TechnoClass* pThis, ObjectClass* pTarget, const TargetZoneScanType& zoneScanType, WeaponTypeClass* pWeapon = nullptr, std::optional<std::reference_wrapper<const ZoneType>> zone = std::nullopt);

	static void UpdateMCOverloadDamage(TechnoClass* pOwner);
	static bool IsCritImmune(TechnoClass* pThis);
	static bool IsPsionicsImmune(TechnoClass* pThis);
	static bool IsCullingImmune(TechnoClass* pThis);
public:
	//only check the veterancy part
	static bool IsEMPImmune(TechnoClass* pThis);
public:
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
	static bool IsWebImmune(TechnoClass* pThis);
	static bool IsDriverKillProtected(TechnoClass* pThis);
	static bool IsUntrackable(TechnoClass* pThis);
	static bool ISC4Holder(InfantryClass* pThis);

	static bool HasAbility(TechnoClass* pThis, PhobosAbilityType nType);
	static bool HasImmunity(TechnoClass* pThis, int nType);

	static bool IsCritImmune(Rank vet, TechnoClass* pThis);
	static bool IsPsionicsImmune(Rank vet, TechnoClass* pThis);
	static bool IsCullingImmune(Rank vet, TechnoClass* pThis);
public:
	//only check the veterancy part
	static bool IsEMPImmune(Rank vet, TechnoClass* pThis);
public:
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
	static bool IsWebImmune(Rank vet, TechnoClass* pThis);
	static bool IsDriverKillProtected(Rank vet, TechnoClass* pThis);
	static bool IsUntrackable(Rank vet, TechnoClass* pThis);

	static bool HasAbility(Rank vet, const TechnoTypeExtData* pTypeExt, PhobosAbilityType nType);
	static bool HasImmunity(Rank vet, TechnoClass* pThis, int nType);

	static bool IsTypeImmune(TechnoClass* pThis, TechnoClass* pSource);

	static bool AllowFiring(AbstractClass* pTargetObj, WeaponTypeClass* pWeapon);

	static bool ObjectHealthAllowFiring(ObjectClass* pTargetObj, WeaponTypeClass* pWeapon);
	static bool CheckCellAllowFiring(TechnoClass* pThis, CellClass* pCell, WeaponTypeClass* pWeapon);
	static bool TechnoTargetAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static bool FireOnceAllowFiring(TechnoClass* pThis, WeaponTypeClass* pWeapon, AbstractClass* pTarget);
	static bool CheckFundsAllowFiring(TechnoClass* pThis, WarheadTypeClass* pWH);
	static bool InterceptorAllowFiring(TechnoClass* pThis, ObjectClass* pTarget);
	static bool TargetTechnoShieldAllowFiring(TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static bool TargetFootAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static std::pair<TechnoClass*, CellClass*> GetTargets(ObjectClass* pObjTarget, AbstractClass* pTarget);
	static int GetDeployFireWeapon(UnitClass* pThis);
	static int GetWeaponIndexAgainstWall(TechnoClass* pThis, OverlayTypeClass* pWallOverlayType);

	static void SetMissionAfterBerzerk(TechnoClass* pThis, bool Immediete = false);

	static AreaFireReturnFlag ApplyAreaFire(TechnoClass* pThis, CellClass*& pTargetCell, WeaponTypeClass* pWeapon);
	static int __fastcall GetThreadPosed(FootClass* pThis);
	static int __fastcall GetBuildingThreadPosed(BuildingClass* pThis);

	static bool IsReallyTechno(TechnoClass* pThis);

	static const BurstFLHBundle* PickFLHs(TechnoClass* pThis, int weaponidx);
	static const Nullable<CoordStruct>* GetInfrantyCrawlFLH(InfantryClass* pThis, int weaponIndex);

	static const Armor GetTechnoArmor(TechnoClass* pThis, WarheadTypeClass* pWarhead);
	static const Armor GetTechnoArmor(ObjectClass* pThis, WarheadTypeClass* pWarhead);

	static bool IsEligibleSize(TechnoClass* pThis, TechnoClass* pPassanger);
	static bool IsAbductable(TechnoClass* pThis, WeaponTypeClass* pWeapon, FootClass* pFoot);

	static void SendPlane(AircraftTypeClass* Aircraft, size_t Amount, HouseClass* pOwner, Rank SendRank, Mission SendMission, AbstractClass* pTarget, AbstractClass* pDest);
	static bool CreateWithDroppod(FootClass* Object, const CoordStruct& XYZ);

	static void StoreHijackerLastDisguiseData(InfantryClass* pThis, FootClass* pVictim);
	static void RestoreStoreHijackerLastDisguiseData(InfantryClass* pThis, FootClass* pVictim);

	static WeaponTypeClass* GetCurrentWeapon(TechnoClass* pThis, int& weaponIndex, bool getSecondary = false);
	static WeaponTypeClass* GetCurrentWeapon(TechnoClass* pThis, bool getSecondary = false);

	static Point2D GetScreenLocation(TechnoClass* pThis);
	static Point2D GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor);
	static Point2D GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition bracketPosition, Point2D offset = Point2D::Empty);
	static void ProcessDigitalDisplays(TechnoClass* pThis);
	static void GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& value, int& maxValue, int infoIndex, ShieldClass* pShield);
	static std::vector<DigitalDisplayTypeClass*>* GetDisplayType(TechnoClass* pThis, TechnoTypeClass* pType, int& length);

	static void RestoreLastTargetAndMissionAfterWebbed(InfantryClass* pThis);
	static void StoreLastTargetAndMissionAfterWebbed(InfantryClass* pThis);

	static NOINLINE Armor GetArmor(ObjectClass* pThis);
	static bool CanDeployIntoBuilding(UnitClass* pThis, bool noDeploysIntoDefaultValue);
	static bool CanDeployIntoBuilding(UnitClass* pThis);

	static void SetChargeTurretDelay(TechnoClass* pThis, int rearmDelay, WeaponTypeClass* pWeapon);

	static bool TryToCreateCrate(CoordStruct location, PowerupEffects selectedPowerup = PowerupEffects::Money, int maxCellRange = 10);

	static void ApplyKillWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH);
	static bool MultiWeaponCanFire(TechnoClass* const pThis, AbstractClass* const pTarget, WeaponTypeClass* const pWeaponType);

	static bool IsHealthInThreshold(ObjectClass* pObject, double min, double max);
	static std::tuple<bool, bool, bool> CanBeAffectedByFakeEngineer(TechnoClass* pThis, TechnoClass* pTarget, bool checkBridge = false, bool checkCapturableBuilding = false, bool checkAttachedBombs = false);

	static bool CannotMove(UnitClass* pThis);

	static bool HasAmmoToDeploy(TechnoClass* pThis);
	static void HandleOnDeployAmmoChange(TechnoClass* pThis, int maxAmmoOverride = -1);

	static bool SimpleDeployerAllowedToDeploy(UnitClass* pThis, bool defaultValue, bool alwaysCheckLandTypes);

	static int ApplyTintColor(TechnoClass* pThis, bool invulnerability, bool airstrike, bool berserk);
	static void ApplyCustomTint(TechnoClass* pThis, int* tintColor, int* intensity);

	static void Fastenteraction(FootClass* pThis);

	static bool CanRetaliateICUnit(TechnoClass* pThis, FakeWeaponTypeClass* pWP, TechnoClass* pTarget);
	static bool CanTargetICUnit(TechnoClass* pThis, FakeWeaponTypeClass* pWP, TechnoClass* pTarget);

	static void ShakeScreen(TechnoClass* pThis, int nValToCalc, int nRules);

	static void AddAirstrikeFactor(TechnoClass*& pKiller, double& d_factor);
	static bool KillerInTransporterFactor(TechnoClass* pKiller, TechnoClass*& pExpReceiver, double& d_factor, bool& promoteImmediately);
	static void AddExperience(TechnoClass* pExtReceiver, TechnoClass* pVictim, int victimCost, double factor);
	static void MCControllerGainExperince(TechnoClass* pExpReceiver, TechnoClass* pVictim, double& d_factor, int victimCost);
	static void GetSpawnerData(TechnoClass*& pSpawnOut, TechnoClass*& pExpReceiver, double& d_spawnFacor, double& d_ExpFactor);
	static void PromoteImmedietely(TechnoClass* pExpReceiver, bool bSilent, bool Flash);
	static void UpdateVeterancy(TechnoClass*& pExpReceiver, TechnoClass* pKiller, TechnoClass* pVictim, int VictimCost, double& d_factor, bool promoteImmediately);
	static void EvaluateExtReceiverData(TechnoClass*& pExpReceiver, TechnoClass* pKiller, double& d_factor, bool& promoteImmediately);

	static void AddPassengers(BuildingClass* const Grinder, FootClass* Vic, bool ParentReversed);

	static bool IsSabotagable(BuildingClass const* const pThis);
	static Action GetiInfiltrateActionResult(InfantryClass* pInf, BuildingClass* pBuilding);
	static bool ApplyC4ToBuilding(InfantryClass* const pThis, BuildingClass* const pBuilding, const bool IsSaboteur);

	static bool IsOperated(TechnoClass* pThis);
	static bool IsOperatedB(TechnoClass* pThis);
	static bool IsPowered(TechnoClass* pThis);
	static void EvalRaidStatus(BuildingClass* pThis);
	static bool IsUnitAlive(UnitClass* pUnit);

	static void SetSpotlight(TechnoClass* pThis, BuildingLightClass* pSpotlight);
	static bool CanSelfCloakNow(TechnoClass* pThis);
	static bool IsCloakable(TechnoClass* pThis, bool allowPassive);
	static bool CloakDisallowed(TechnoClass* pThis, bool allowPassive);
	static bool CloakAllowed(TechnoClass* pThis);

	static InfantryTypeClass* GetBuildingCrew(BuildingClass* pThis, int nChance);
	static void UpdateFactoryQueues(BuildingClass const* const pBuilding);
	static bool IsBaseNormal(BuildingClass* pBuilding);

	static int GetVictimBountyValue(TechnoClass* pVictim, TechnoClass* pKiller);
	static bool KillerAllowedToEarnBounty(TechnoClass* pKiller, TechnoClass* pVictim);
	static void GiveBounty(TechnoClass* pVictim, TechnoClass* pKiller);

	static AresHijackActionResult GetActionHijack(InfantryClass* pThis, TechnoClass* const pTarget);
	static bool PerformActionHijack(TechnoClass* pFrom, TechnoClass* const pTarget);
	static bool FindAndTakeVehicle(FootClass* pThis);

	static Action GetEngineerEnterEnemyBuildingAction(BuildingClass* const pBld);

	static bool CloneBuildingEligible(BuildingClass* pBuilding, bool requirePower);
	static void KickOutClone(BuildingClass* pBuilding, TechnoTypeClass* ProductionType, HouseClass* FactoryOwner);
	static void KickOutClones(BuildingClass* pFactory, TechnoClass* const Production);

	static void InitWeapon(
		TechnoClass* pThis,
		TechnoTypeClass* pType,
		WeaponTypeClass* pWeapon,
		int idxWeapon,
		CaptureManagerClass*& pCapture,
		ParasiteClass*& pParasite,
		TemporalClass*& pTemporal,
		const char* pTagName,
		bool IsFoot
	);

	static InfantryClass* RecoverHijacker(FootClass* const pThis);
	static void SpawnSurvivors(
		FootClass* const pThis,
		TechnoClass* const pKiller,
		const bool Select,
		const bool IgnoreDefenses,
		const bool PreventPassengersEscape
	);

	static int GetWarpPerStep(TemporalClass* pThis, int nStep);
	static bool Warpable(TemporalClass* pTemp, TechnoClass* pTarget);

	static void DepositTiberium(TechnoClass* pThis, HouseClass* pHouse, float const amount, float const bonus, int const idxType);
	static void RefineTiberium(TechnoClass* pThis, HouseClass* pHouse, float const amount, int const idxType);

	static bool FiringAllowed(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon);

	static UnitTypeClass* GetUnitTypeImage(UnitClass* const pThis);
	static TechnoTypeClass* GetImage(FootClass* pThis);

	static void HandleTunnelLocoStuffs(FootClass* pOwner, bool DugIN = false, bool PlayAnim = false);

	static bool IsSameTrech(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static bool canTraverseTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static void doTraverseTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding);

	static bool AcquireHunterSeekerTarget(TechnoClass* pThis);
	static void UpdateAlphaShape(ObjectClass* pSource);

	static int GetAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon);
	static void DecreaseAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon);

	static AnimClass* SpawnAnim(CoordStruct& crd, AnimTypeClass* pType, int dist);

	static void PlantBomb(TechnoClass* pSource, ObjectClass* pTarget, WeaponTypeClass* pWeapon);
	static bool CanDetonate(TechnoClass* pThis, ObjectClass* pThat);
	static Action GetAction(TechnoClass* pThis, ObjectClass* pThat);

	static int GetFirstSuperWeaponIndex(BuildingClass* pThis);
	static void UpdateDisplayTo(BuildingClass* pThis);
	static void InfiltratedBy(BuildingClass* EnteredBuilding, HouseClass* Enterer);
	static DirStruct UnloadFacing(UnitClass* pThis);
	static CellStruct UnloadCell(BuildingClass* pThis);
	static BuildingClass* BuildingUnload(UnitClass* pThis);

	static void KickOutHospitalArmory(BuildingClass* pThis);
	static void KickOutOfRubble(BuildingClass* pBld);
	static void UpdateSensorArray(BuildingClass* pBld);
	static BuildingClass* CreateBuilding(
		BuildingClass* pBuilding,
		bool remove,
		BuildingTypeClass* pNewType,
		OwnerHouseKind owner,
		int strength,
		AnimTypeClass* pAnimType
	);

	static void Destroy(TechnoClass* pTechno, TechnoClass* pKiller, HouseClass* pKillerHouse, WarheadTypeClass* pWarhead);
	static bool IsDriverKillable(TechnoClass* pThis, double KillBelowPercent);
	static void ApplyKillDriver(TechnoClass* pTarget, TechnoClass* pKiller, HouseClass* pToOwner, bool ResetVet, Mission passiveMission);
	static bool ConvertToType(TechnoClass* pThis, TechnoTypeClass* pToType, bool AdjustHealth = true, bool IsChangeOwnership = false);

	static int GetSelfHealAmount(TechnoClass* pThis);
	static void SpawnVisceroid(CoordStruct& crd, UnitTypeClass* pType, int chance, bool ignoreTibDeathToVisc, HouseClass* Owner);

	static void TransferOriginalOwner(TechnoClass* pFrom, TechnoClass* pTo);
	static void TransferIvanBomb(TechnoClass* From, TechnoClass* To);
	static void Ares_technoUpdate(TechnoClass* pThis);
	static void Ares_AddMoneyStrings(TechnoClass* pThis, bool forcedraw);

public:
	static UnitClass* Deployer;

};

class TechnoExtContainer
{
public:
	static TechnoExtContainer Instance;

	COMPILETIMEEVAL FORCEDINLINE  TechnoExtData* GetExtAttribute(TechnoClass* key)
	{
		return (TechnoExtData*)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	COMPILETIMEEVAL FORCEDINLINE TechnoExtData* Find(TechnoClass* key)
	{
		return this->GetExtAttribute(key);
	}

	COMPILETIMEEVAL FORCEDINLINE TechnoExtData* TryFind(TechnoClass* key)
	{
		if (!key)
			return nullptr;

		return this->GetExtAttribute(key);
	}

};

//we cannot inherit this
class NOVTABLE FakeTechnoClass //final: TechnoClass
{
public:

	//virtual TechnoTypeClass* GetTechnoType() { JMP_THIS(0x6F3270); }

	static int __fastcall _EvaluateJustCell(TechnoClass* pThis, discard_t, CellStruct* where);
	static bool __fastcall __TargetSomethingNearby(TechnoClass* pThis, discard_t, CoordStruct* coord, ThreatType threat);
	static int __fastcall __AdjustDamage(TechnoClass* pThis, discard_t, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static void __fastcall __DrawAirstrikeFlare(TechnoClass* pThis, discard_t, const CoordStruct& startCoord, int startHeight, int endHeight, const CoordStruct& endCoord);
	static AbstractClass* __fastcall __Greatest_Threat(TechnoClass* techno, discard_t, ThreatType method, CoordStruct* location, bool a4);
	static 	bool __fastcall __EvaluateObject(
		TechnoClass* pThis,
		discard_t,
		ThreatType targetFlags,
		int mask,
		int wantedDistance,
		ObjectClass* pTarget,
		int* pThreatPosed,
		ZoneType dwUnk,
		CoordStruct* pSourceCoords);

	static bool __EvaluateObjectB(TechnoClass* pThis,
		ThreatType   method,
		int          mask,
		int          range,
		ObjectClass* target,
		int* value,
		ZoneType     zone,
		CoordStruct* coord,
		bool         attackUnderground);

	static FireError __fastcall __CanFireMod(
	TechnoClass* pThis,
	AbstractClass* pTarget,
	int nWeaponIdx,
	bool bCheckRange,
	bool bSkipROF = false);

	static double __fastcall __GetThreatCoeff(TechnoClass* pThis, discard_t, ObjectClass* pTarget, CoordStruct* pTargetCoord);

	static FireError __fastcall __CanFireOriginal(TechnoClass* pThis, discard_t, AbstractClass* pTarget, int nWeaponIdx, bool bCheckRange);

	static void __fastcall __Draw_Pips(TechnoClass* techno, discard_t, Point2D* position, Point2D* unused, RectangleStruct* clipRect);
	static void __fastcall  __Draw_Stuff_When_Selected(TechnoClass* pThis, discard_t, Point2D* pPoint, Point2D* pOriginalPoint, RectangleStruct* pRect);
	static void __fastcall __DrawHealthBar_Selection(TechnoClass* techno, discard_t, Point2D* position, RectangleStruct* clipRect, bool unused);
	static void __fastcall __Draw_Airstrike_Flare(TechnoClass* techno, discard_t, CoordStruct startCoord, CoordStruct endCoord);
	static CoordStruct* __fastcall __Get_FLH(TechnoClass* pThis, discard_t, CoordStruct* pBuffer, int weaponIndex, CoordStruct offset);

	static DamageState __fastcall __Take_Damage(TechnoClass* pThis, discard_t, int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse);
	static bool __fastcall __Is_Allowed_To_Retaliate(TechnoClass* pThis, discard_t, TechnoClass* pSource, WarheadTypeClass* pWarhead);

	static int __fastcall __WhatWeaponShouldIUse(TechnoClass* pThis, discard_t, AbstractClass* pTarget);
	static void __fastcall __DoUncloak(TechnoClass* pThis, discard_t, char quiet);
	static void __fastcall __DoCloak(TechnoClass* pThis, discard_t, char quiet);
	static int __fastcall __HowManySurvivors(TechnoClass* pThis);
	static bool __fastcall __ShouldSelfHealOneStep(TechnoClass* pThis);
	static int __fastcall __TimeToBuild(TechnoClass* pThis);

	static InfantryTypeClass* __fastcall __GetCrew(TechnoClass* pThis);
	static void __fastcall __DrawExtras(TechnoClass* pThis, discard_t, Point2D* pLocation, RectangleStruct* pBounds);
	static void __fastcall __Activate(TechnoClass* pThis);
	static void __fastcall __Deactivate(TechnoClass* pThis);
	//AI
	static void __HandleGattlingAudio(TechnoClass* pThis);
	static void __HandleVoicePlayback(TechnoClass* pThis);
	static void __HandleBerzerkState(TechnoClass* pThis);
	static void __HandleStrengthSmoothing(TechnoClass* pThis);
	static void __HandleTurretAudio(TechnoClass* pThis);
	static void __HandleVeterancyPromotion(TechnoClass* pThis);
	static void __HandleMoneyDrain(TechnoClass* pThis);
	static void __HandleDrainTarget(TechnoClass* pThis);
	static void __HandleHiddenState(TechnoClass* pThis);
	static void __ClearInvalidAllyTarget(TechnoClass* pThis);
	static void __CheckTargetInRange(TechnoClass* pThis);
	static void __HandleTurretRecoil(TechnoClass* pThis);
	static void __HandleChargeTurret(TechnoClass* pThis);
	static void __HandleDoorAndTimers(TechnoClass* pThis);
	static void __ClearTargetForInvalidMissions(TechnoClass* pThis);
	static void __HandleTargetAcquisition(TechnoClass* pThis);
	static void __HandleAttachedBomb(TechnoClass* pThis);
	static void __HandleManagers(TechnoClass* pThis);
	static void __HandleSelfHealing(TechnoClass* pThis);
	static void __HandleCloaking(TechnoClass* pThis);
	static void __ClearTargetIfNoDamage(TechnoClass* pThis);
	static void __ClearAircraftTarget(TechnoClass* pThis);
	static void __CheckTargetReachability(TechnoClass* pThis);
	static void __UpdateAnimationStage(TechnoClass* pThis);
	static void __HandleFlashing(TechnoClass* pThis);
	static void __HandleDamageSparks(TechnoClass* pThis);
	static void __HandleEMPEffect(TechnoClass* pThis);
	static void __fastcall __AI(TechnoClass* pThis);
	static void __fastcall _Cloaking_AI(TechnoClass* pThis, discard_t, bool something);
	static bool __fastcall _ShouldNotBeCloaked(TechnoClass* pThis);
	static bool __fastcall _ShouldBeCloaked(TechnoClass* pThis);
};

#define GET_TECHNOTYPE(techno) techno->GetTechnoType()
#define GET_TECHNOTYPEEXT(techno) TechnoTypeExtContainer::Instance.Find(techno->GetTechnoType())
#define GET_FOOTTYPEEXT(techno) FootTypeExtContainer::Instance.Find(techno->GetTechnoType())