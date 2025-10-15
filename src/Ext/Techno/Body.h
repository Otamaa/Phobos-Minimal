#pragma once
#include <TechnoClass.h>
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PooledContainer.h>
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
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutState.h>

#include <New/Type/DigitalDisplayTypeClass.h>

#include <Utilities/BuildingBrackedPositionData.h>
#include <Utilities/MemoryPoolUniquePointer.h>

#include <New/Entity/PoweredUnitClass.h>
#include <New/Entity/RadarJammerClass.h>

#include <Misc/Ares/Hooks/Classes/AttachedAffects.h>

#include <New/Entity/NewTiberiumStorageClass.h>
#include <New/Entity/AEProperties.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectClass.h>

#include <TemporalClass.h>
#include <EBolt.h>

#include <Ext/Radio/Body.h>
#include <Utilities/EntityCachePtr.h>

class BulletClass;
class TechnoTypeClass;
class REGISTERS;
struct BurstFLHBundle;

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

struct OnlyAttackEntity {
	HelperedVector<OnlyAttackStruct> Array {};

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm.Process(Array);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return  Stm.Process(Array);
	}
};

struct TechnoStateComponent
{
	// Bitflags - all states in 1 byte!
	union
	{
		struct
		{
			bool AircraftOpentoppedInitEd : 1;
			bool DelayedFireSequencePaused : 1;
			bool FallingDownTracked : 1;
			bool ForceFullRearmDelay : 1;
			bool GattlingDmageSound : 1;
			bool HasRemainingWarpInDelay : 1;
			bool IsBeingChronoSphered : 1;
			bool IsDetachingForCloak : 1;
			bool IsDriverKilled : 1;
			bool IsOperated : 1;
			bool IsSelected : 1;
			bool IsSurvivorsDone : 1;
			bool KeepTargetOnMove : 1;
			bool LastRearmWasFullDelay : 1;
			bool ResetLocomotor : 1;
			bool ShouldUpdateGattlingValue : 1;
			bool SkipLowDamageCheck : 1;
			bool SkipVoice : 1;
			bool TakeVehicleMode : 1;
			bool UndergroundTracked : 1;
			bool UnitIdleActionSelected : 1;
			bool UnitIdleIsSelected : 1;

			// Original flags
			bool IsWebbed : 1;
			bool IsBurrowed : 1;
			bool IsInTunnel : 1;
			bool ReceiveDamage : 1;
			bool LastKillWasTeamTarget : 1;
			bool PayloadCreated : 1;
			bool PayloadTriggered : 1;
			bool SupressEVALost : 1;
			bool HasExtraFireWeapon : 1;
		};
		uint64_t AllFlags; // 64-bit container for all flags
	};

	TechnoStateComponent()
		: AllFlags(0) // Initialize all flags to false
	{ }

	// Serialization
	bool Save(PhobosStreamWriter& writer) const
	{
		return writer.Process(AllFlags);
	}

	bool Load(PhobosStreamReader& reader)
	{
		return reader.Process(AllFlags);
	}

	bool HasAnyState() const { return AllFlags != 0; }
};

struct IdleActionComponent {
	CDTimerClass UnitIdleActionTimer;
	CDTimerClass UnitIdleActionGapTimer;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {
		return Stm
			.Process(UnitIdleActionTimer)
			.Process(UnitIdleActionGapTimer);
	}

	bool Save(PhobosStreamWriter& Stm) const {
		return Stm
			.Process(UnitIdleActionTimer)
			.Process(UnitIdleActionGapTimer);
	}
};

struct HijackerComponent {
	int Health;
	HouseClass* Owner;
	float Veterancy;
	InfantryTypeClass* LastDisguiseType;
	HouseClass* LastDisguiseHouse;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
				.Process(Health)
				.Process(Owner)
				.Process(Veterancy)
				.Process(LastDisguiseType)
				.Process(LastDisguiseHouse)
			;
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(Health)
			.Process(Owner)
			.Process(Veterancy)
			.Process(LastDisguiseType)
			.Process(LastDisguiseHouse)
			;
	}
};


class TechnoExtData : public RadioExtData
{
private:

	template<typename entT>
	struct EntitySerializer
	{
		static void Save(PhobosStreamWriter& stm, entt::entity owner)
		{
			auto myComp = Phobos::gEntt->try_get<entT>(owner);

			bool exist = myComp != nullptr;
			Savegame::WritePhobosStream(stm, exist);
			if (myComp) {
				myComp->Save(stm);
			}
		}

		static void Load(PhobosStreamReader& stm, entt::entity owner)
		{
			bool exist;
			Savegame::ReadPhobosStream(stm, exist);
			if (exist) {
				auto& ent_ = Phobos::gEntt->emplace<entT>(owner);
				ent_.Load(stm , true);
			}
		}
	};

	template <typename T>
	void SerializeEntity(T& Stm)
	{
		if constexpr (std::is_same_v<T, PhobosStreamWriter>)
		{
			EntitySerializer<TechnoStateComponent>::Save(Stm, this->MyEntity);
			EntitySerializer<PoweredUnitClass>::Save(Stm, this->MyEntity);
			EntitySerializer<RadarJammerClass>::Save(Stm, this->MyEntity);
			EntitySerializer<ShieldClass>::Save(Stm, this->MyEntity);
			EntitySerializer<AEProperties>::Save(Stm, this->MyEntity);
			EntitySerializer<AEPropertiesExtraRange>::Save(Stm, this->MyEntity);
			EntitySerializer<AEPropertiesExtraCrit>::Save(Stm, this->MyEntity);
			EntitySerializer<AEPropertiesArmorMult>::Save(Stm, this->MyEntity);
			EntitySerializer<GiftBox>::Save(Stm, this->MyEntity);
			EntitySerializer<DamageSelfState>::Save(Stm, this->MyEntity);
			EntitySerializer<WeaponTimers>::Save(Stm, this->MyEntity);
			EntitySerializer<SimulateBurstManager>::Save(Stm, this->MyEntity);
			EntitySerializer<DelayFireManager>::Save(Stm, this->MyEntity);
			EntitySerializer<DriveData>::Save(Stm, this->MyEntity);
			EntitySerializer<AircraftDive>::Save(Stm, this->MyEntity);
			EntitySerializer<SpawnSupport>::Save(Stm, this->MyEntity);
			EntitySerializer<AircraftPutState>::Save(Stm, this->MyEntity);
			EntitySerializer<OnlyAttackEntity>::Save(Stm, this->MyEntity);
			EntitySerializer<IdleActionComponent>::Save(Stm, this->MyEntity);
			EntitySerializer<HijackerComponent>::Save(Stm, this->MyEntity);
			EntitySerializer<AresAEData>::Save(Stm, this->MyEntity);
		}
		else
		{
			EntitySerializer<TechnoStateComponent>::Load(Stm, this->MyEntity);
			EntitySerializer<PoweredUnitClass>::Load(Stm, this->MyEntity);
			EntitySerializer<RadarJammerClass>::Load(Stm, this->MyEntity);
			EntitySerializer<ShieldClass>::Load(Stm, this->MyEntity);
			EntitySerializer<AEProperties>::Load(Stm, this->MyEntity);
			EntitySerializer<AEPropertiesExtraRange>::Load(Stm, this->MyEntity);
			EntitySerializer<AEPropertiesExtraCrit>::Load(Stm, this->MyEntity);
			EntitySerializer<AEPropertiesArmorMult>::Load(Stm, this->MyEntity);
			EntitySerializer<GiftBox>::Load(Stm, this->MyEntity);
			EntitySerializer<DamageSelfState>::Load(Stm, this->MyEntity);
			EntitySerializer<WeaponTimers>::Load(Stm, this->MyEntity);
			EntitySerializer<SimulateBurstManager>::Load(Stm, this->MyEntity);
			EntitySerializer<DelayFireManager>::Load(Stm, this->MyEntity);
			EntitySerializer<DriveData>::Load(Stm, this->MyEntity);
			EntitySerializer<AircraftDive>::Load(Stm, this->MyEntity);
			EntitySerializer<SpawnSupport>::Load(Stm, this->MyEntity);
			EntitySerializer<AircraftPutState>::Load(Stm, this->MyEntity);
			EntitySerializer<OnlyAttackEntity>::Load(Stm, this->MyEntity);
			EntitySerializer<IdleActionComponent>::Load(Stm, this->MyEntity);
			EntitySerializer<HijackerComponent>::Load(Stm, this->MyEntity);
			EntitySerializer<AresAEData>::Load(Stm, this->MyEntity);
		}
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Type)
			.Process(AbsType)
			.Process(Tints)
			.Process(idxSlot_EMPulse)
			.Process(idxSlot_Wave)
			.Process(idxSlot_Beam)
			.Process(idxSlot_Warp)
			.Process(idxSlot_Parasite)
			.Process(EMPSparkleAnim)
			.Process(EMPLastMission)
			.Process(BuildingLight)
			.Process(OriginalHouseType)
			.Process(CloakSkipTimer)
			.Process(LastWarpInDelay)
			.Process(TechnoValueAmount)
			.Process(Pos)
			.Process(LaserTrails)
			.Process(PassengerDeletionTimer)
			.Process(CurrentShieldType)
			.Process(LastWarpDistance)
			.Process(Death_Countdown)
			.Process(MindControlRingAnimType)
			.Process(DamageNumberOffset)
			.Process(CurrentLaserWeaponIndex)
			.Process(OriginalPassengerOwner)
			.Process(DeployFireTimer)
			.Process(DisableWeaponTimer)
			.Process(RevengeWeapons)
			.Process(GattlingDmageDelay)
			.Process(EngineerCaptureDelay)
			.Process(ReceiveDamageMultiplier)
			.Process(Trails)
			.Process(PaintBallStates)
			.Process(CurrentWeaponIdx)
			.Process(MyOriginalTemporal)
			.Process(WarpedOutDelay)
			.Process(SelfHealing_CombatDelay)
			.Process(LinkedSW)
			.Process(SuperTarget)
			.Process(WHAnimRemainingCreationInterval)
			.Process(WebbedAnim)
			.Process(WebbyLastTarget)
			.Process(WebbyLastMission)
			.Process(MergePreventionTimer)
			.Process(TiberiumStorage)
			.Process(PhobosAE)
			.Process(FiringObstacleCell)
			.Process(AdditionalRange)
			.Process(SubterraneanHarvRallyPoint)
			.Process(TiberiumEaterTimer)
			.Process(LastDamageWH)
			.Process(MyTargetingFrame)
			.Process(ChargeTurretTimer)
			.Process(DropCrate)
			.Process(DropCrateType)
			.Process(LastBeLockedFrame)
			.Process(BeControlledThreatFrame)
			.Process(LastTargetID)
			.Process(AccumulatedGattlingValue)
			.Process(LastSensorsMapCoords)
			.Process(DelayedFireWeaponIndex)
			.Process(DelayedFireTimer)
			.Process(CurrentDelayedFireAnim)
			.Process(CustomFiringOffset)
			.Process(LastWeaponType)
			.Process(ElectricBolts)
			.Process(LastHurtFrame)
			.Process(AttachedEffectInvokerCount)
			.Process(AirstrikeTargetingMe)
			.Process(RandomEMPTarget)
			.Process(AttackMoveFollowerTempCount)
			.Process(PassiveAquireMode)
			; // end
	}

public:
	using base_type = TechnoClass;
public:

#pragma region ClassMembers
	entt::entity MyEntity;

	TechnoTypeClass* Type; //original Type pointer
	OptionalStruct<AbstractType, true> AbsType;
	TintColors Tints;

	BYTE idxSlot_EMPulse;
	BYTE idxSlot_Wave; //5
	BYTE idxSlot_Beam; //6
	BYTE idxSlot_Warp; //7
	BYTE idxSlot_Parasite; //8

	Handle<AnimClass*, UninitAnim> EMPSparkleAnim;
	Mission EMPLastMission; //
	BuildingLightClass* BuildingLight;
	HouseTypeClass* OriginalHouseType;
	CDTimerClass CloakSkipTimer; //
	int LastWarpInDelay;
	int TechnoValueAmount;
	int Pos;
	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails;
	CDTimerClass PassengerDeletionTimer;
	ShieldTypeClass* CurrentShieldType;
	int LastWarpDistance;
	CDTimerClass Death_Countdown;
	AnimTypeClass* MindControlRingAnimType;
	int DamageNumberOffset;
	OptionalStruct<int, true> CurrentLaserWeaponIndex;

	// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
	// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
	HouseClass* OriginalPassengerOwner;
	CDTimerClass DeployFireTimer;
	CDTimerClass DisableWeaponTimer;
	HelperedVector<TimedWarheadValue<WeaponTypeClass*>> RevengeWeapons;
	int GattlingDmageDelay;
	CDTimerClass EngineerCaptureDelay;
	OptionalStruct<double, true> ReceiveDamageMultiplier;
	HelperedVector<std::unique_ptr<UniversalTrail>> Trails;
	PhobosMap<WarheadTypeClass*, PaintBall> PaintBallStates;
	int CurrentWeaponIdx;
	TemporalClass* MyOriginalTemporal;
	CDTimerClass WarpedOutDelay;
	CDTimerClass SelfHealing_CombatDelay;
	SuperClass* LinkedSW;
	CellStruct SuperTarget;
	int WHAnimRemainingCreationInterval;
	Handle<AnimClass*, UninitAnim> WebbedAnim;
	AbstractClass* WebbyLastTarget;
	Mission WebbyLastMission;
	CDTimerClass MergePreventionTimer;
	NewTiberiumStorageClass TiberiumStorage;
	HelperedVector<std::unique_ptr<PhobosAttachEffectClass>> PhobosAE;
	CellClass* FiringObstacleCell; // Set on firing if there is an obstacle cell between target and techno, used for updating WaveClass target etc.
	OptionalStruct<int, true> AdditionalRange;
	CellClass* SubterraneanHarvRallyPoint;
	CDTimerClass TiberiumEaterTimer;
	WarheadTypeClass* LastDamageWH;
	int MyTargetingFrame;
	CDTimerClass ChargeTurretTimer;// Used for charge turrets instead of RearmTimer if weapon has ChargeTurret.Delays set.
	int DropCrate; // Drop crate on death, modified by map action
	PowerupEffects DropCrateType;
	int LastBeLockedFrame;
	int BeControlledThreatFrame;
	DWORD LastTargetID;
	int AccumulatedGattlingValue;
	CellStruct LastSensorsMapCoords;
	int DelayedFireWeaponIndex;
	CDTimerClass DelayedFireTimer;
	Handle<AnimClass*, UninitAnim> CurrentDelayedFireAnim;
	std::optional<CoordStruct> CustomFiringOffset; // If set any calls to GetFLH() will use this coordinate as
	WeaponTypeClass* LastWeaponType;
	HelperedVector<EBolt*> ElectricBolts;
	int LastHurtFrame;
	int AttachedEffectInvokerCount;
	AirstrikeClass* AirstrikeTargetingMe;
	CellStruct RandomEMPTarget;
	int AttackMoveFollowerTempCount;
	PassiveAcquireMode PassiveAquireMode;

#pragma endregion

public:

	TechnoExtData(TechnoClass* abs) : RadioExtData(abs),
		MyEntity(),

		Type(nullptr),
		AbsType(), // OptionalStruct<AbstractType,true>
		//AeData(),
		Tints(),

		idxSlot_EMPulse(0),
		idxSlot_Wave(0),
		idxSlot_Beam(0),
		idxSlot_Warp(0),
		idxSlot_Parasite(0),

		EMPSparkleAnim(nullptr),
		EMPLastMission(Mission::Sleep),
		BuildingLight(nullptr),
		OriginalHouseType(nullptr),
		CloakSkipTimer(),
		TechnoValueAmount(0),
		Pos(0),
		LaserTrails(),
		PassengerDeletionTimer(),
		CurrentShieldType(nullptr),
		LastWarpDistance(0),
		Death_Countdown(),
		MindControlRingAnimType(nullptr),
		DamageNumberOffset(INT32_MIN),
		CurrentLaserWeaponIndex(),
		OriginalPassengerOwner(nullptr),
		DeployFireTimer(),
		DisableWeaponTimer(),
		RevengeWeapons(),
		GattlingDmageDelay(-1),
		EngineerCaptureDelay(),
		ReceiveDamageMultiplier(),
		Trails(),
		PaintBallStates(),
		CurrentWeaponIdx(-1),
		MyOriginalTemporal(nullptr),
		WarpedOutDelay(),
		SelfHealing_CombatDelay(),
		LinkedSW(nullptr),
		SuperTarget(),
		WHAnimRemainingCreationInterval(0),
		WebbedAnim(nullptr),
		WebbyLastTarget(nullptr),
		WebbyLastMission(Mission::Sleep),
		MergePreventionTimer(),
		TiberiumStorage(),
		PhobosAE(),
		FiringObstacleCell(nullptr),
		AdditionalRange(),
		SubterraneanHarvRallyPoint(nullptr),
		TiberiumEaterTimer(),
		LastDamageWH(nullptr),
		MyTargetingFrame(0),
		ChargeTurretTimer(),
		DropCrate(-1),
		DropCrateType(PowerupEffects::Money),
		LastBeLockedFrame(0),
		BeControlledThreatFrame(0),
		LastTargetID(0xFFFFFFFF),
		AccumulatedGattlingValue(0),
		LastSensorsMapCoords(),
		DelayedFireWeaponIndex(-1),
		DelayedFireTimer(),
		CurrentDelayedFireAnim(nullptr),
		CustomFiringOffset(),
		LastWeaponType(nullptr),
		ElectricBolts(),
		LastHurtFrame(0),
		AttachedEffectInvokerCount(0),
		AirstrikeTargetingMe(nullptr),
		RandomEMPTarget(),
		AttackMoveFollowerTempCount(0),
		PassiveAquireMode(PassiveAcquireMode::Normal)
	{
		// ensure tib storage sized properly
		TiberiumStorage.m_values.resize(TiberiumClass::Array->Count);

		// randomized initial targeting frame
		MyTargetingFrame = ScenarioClass::Instance->Random.RandomRanged(0, 15);
		this->MyEntity = Phobos::gEntt->create();
		Phobos::gEntt->emplace<TechnoStateComponent>(this->MyEntity);
		Phobos::gEntt->emplace<AEProperties>(this->MyEntity);
		Phobos::gEntt->emplace<AresAEData>(this->MyEntity);
		this->Tints.SetOwner(abs);
	}

	TechnoExtData(TechnoClass* abs, noinit_t& noint) : RadioExtData(abs, noint)
	{
		this->MyEntity = Phobos::gEntt->create();
	};

	virtual ~TechnoExtData();
	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;

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

	virtual TechnoClass* This() const override { return reinterpret_cast<TechnoClass*>(RadioExtData::This()); }
	virtual const TechnoClass* This_Const() const override { return reinterpret_cast<const TechnoClass*>(RadioExtData::This_Const()); }

	virtual void CalculateCRC(CRCEngine& crc) const override
	{
		this->RadioExtData::CalculateCRC(crc);
	}

public:

	FORCEINLINE AresAEData* Get_AresAEData(){
		return Phobos::gEntt->try_get<AresAEData>(this->MyEntity);
	}

	FORCEDINLINE ShieldClass* GetShield()
	{
		return Phobos::gEntt->try_get<ShieldClass>(this->MyEntity);
	}

	FORCEDINLINE TechnoStateComponent* Get_TechnoStateComponent()
	{
		return Phobos::gEntt->try_get<TechnoStateComponent>(this->MyEntity);
	}

	FORCEDINLINE PoweredUnitClass* Get_PoweredUnitClass()
	{
		return Phobos::gEntt->try_get<PoweredUnitClass>(this->MyEntity);
	}

	FORCEDINLINE RadarJammerClass* Get_RadarJammerClass()
	{
		return Phobos::gEntt->try_get<RadarJammerClass>(this->MyEntity);
	}

	FORCEDINLINE AEProperties* Get_AEProperties()
	{
		return Phobos::gEntt->try_get<AEProperties>(this->MyEntity);
	}

	FORCEDINLINE AEPropertiesExtraRange* Get_AEPropertiesExtraRange()
	{
		return Phobos::gEntt->try_get<AEPropertiesExtraRange>(this->MyEntity);
	}

	FORCEDINLINE AEPropertiesExtraCrit* Get_AEPropertiesExtraCrit()
	{
		return Phobos::gEntt->try_get<AEPropertiesExtraCrit>(this->MyEntity);
	}

	FORCEDINLINE AEPropertiesArmorMult* Get_AEPropertiesArmorMult()
	{
		return Phobos::gEntt->try_get<AEPropertiesArmorMult>(this->MyEntity);
	}

	FORCEDINLINE GiftBox* Get_GiftBox()
	{
		return Phobos::gEntt->try_get<GiftBox>(this->MyEntity);
	}

	FORCEDINLINE DamageSelfState* Get_DamageSelfState()
	{
		return Phobos::gEntt->try_get<DamageSelfState>(this->MyEntity);
	}

	FORCEDINLINE WeaponTimers* Get_WeaponTimers()
	{
		return Phobos::gEntt->try_get<WeaponTimers>(this->MyEntity);
	}

	FORCEDINLINE SimulateBurstManager* Get_SimulateBurstManager()
	{
		return Phobos::gEntt->try_get<SimulateBurstManager>(this->MyEntity);
	}

	FORCEDINLINE DelayFireManager* Get_DelayFireManager()
	{
		return Phobos::gEntt->try_get<DelayFireManager>(this->MyEntity);
	}

	FORCEDINLINE DriveData* Get_DriveData()
	{
		return Phobos::gEntt->try_get<DriveData>(this->MyEntity);
	}

	FORCEDINLINE AircraftDive* Get_AircraftDive()
	{
		return Phobos::gEntt->try_get<AircraftDive>(this->MyEntity);
	}

	FORCEDINLINE SpawnSupport* Get_SpawnSupport()
	{
		return Phobos::gEntt->try_get<SpawnSupport>(this->MyEntity);
	}

	FORCEDINLINE AircraftPutState* Get_AircraftPutState()
	{
		return Phobos::gEntt->try_get<AircraftPutState>(this->MyEntity);
	}

	FORCEDINLINE OnlyAttackEntity* Get_OnlyAttackEntity()
	{
		return Phobos::gEntt->try_get<OnlyAttackEntity>(this->MyEntity);
	}

	FORCEDINLINE IdleActionComponent* Get_IdleActionComponent()
	{
		return Phobos::gEntt->try_get<IdleActionComponent>(this->MyEntity);
	}

	FORCEDINLINE HijackerComponent* Get_HijackerComponent()
	{
		return Phobos::gEntt->try_get<HijackerComponent>(this->MyEntity);
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
	bool ContainFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker);
	int FindFirer(WeaponTypeClass* const Weapon);

	void InitPassiveAcquireMode();
	PassiveAcquireMode GetPassiveAcquireMode() const;
	void TogglePassiveAcquireMode(PassiveAcquireMode mode);
	bool CanTogglePassiveAcquireMode();

public:

	static void InitializeUnitIdleAction(TechnoClass* pThis, TechnoTypeClass* pType);

	static bool HandleDelayedFireWithPauseSequence(TechnoClass* pThis, WeaponTypeClass* pWeapon, int weaponIndex, int frame, int firingFrame);

	static bool IsOnBridge(FootClass* pUnit);
	static int GetJumpjetIntensity(FootClass* pThis);
	static void GetLevelIntensity(TechnoClass* pThis, int level, int& levelIntensity, int& cellIntensity, double levelMult, double cellMult, bool applyBridgeBonus = false);
	static int GetDeployingAnimIntensity(FootClass* pThis);

	static int CalculateBlockDamage(TechnoClass* pThis, args_ReceiveDamage* args);
	static std::vector<double> GetBlockChance(TechnoClass* pThis, std::vector<double>& blockChance);

protected:
	std::pair<const std::vector<WeaponTypeClass*>*, const std::vector<int>*> GetFireSelfData();
	int GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData, FootClass const* pPassenger);

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

	static Matrix3D GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey = nullptr);
	static void TransformFLHForTurret(TechnoClass* pThis, Matrix3D& mtx, bool isOnTurret);
	static Matrix3D GetFLHMatrix(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret);
	static CoordStruct GetFLHAbsoluteCoordsB(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& flh, bool turretFLH = false, const CoordStruct& Overrider = CoordStruct::Empty);
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
	//only check the veterancy part , please use the complete check from `Ares/Hooks/Header.h`
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
	//only check the veterancy part , please use the complete check from `Ares/Hooks/Header.h`
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

	static bool HasAbility(Rank vet, TechnoClass* pThis, PhobosAbilityType nType);
	static bool HasImmunity(Rank vet, TechnoClass* pThis, int nType);

	static bool IsTypeImmune(TechnoClass* pThis, TechnoClass* pSource);

	static bool AllowFiring(AbstractClass* pTargetObj, WeaponTypeClass* pWeapon);

	static bool ObjectHealthAllowFiring(ObjectClass* pTargetObj, WeaponTypeClass* pWeapon);
	static bool CheckCellAllowFiring(CellClass* pCell, WeaponTypeClass* pWeapon);
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
	static int GetThreadPosed(TechnoClass* pThis);

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
	static void GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& value, int& maxValue, int infoIndex);
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

	virtual TechnoTypeClass* GetTechnoType() { JMP_THIS(0x6F3270); }

	static int __fastcall _EvaluateJustCell(TechnoClass* pThis, discard_t, CellStruct* where);
	static bool __fastcall __TargetSomethingNearby(TechnoClass* pThis, discard_t, CoordStruct* coord, ThreatType threat);
	static int __fastcall __AdjustDamage(TechnoClass* pThis, discard_t, TechnoClass* pTarget, WeaponTypeClass* pWeapon);
	static void __fastcall __DrawAirstrikeFlare(TechnoClass* pThis, discard_t, const CoordStruct& startCoord, int startHeight, int endHeight, const CoordStruct& endCoord);
	static AbstractClass* __fastcall __Greatest_Threat(TechnoClass* techno, discard_t, ThreatType method, CoordStruct* location, bool a4);
	static void __fastcall __Draw_Pips(TechnoClass* techno, discard_t, Point2D* position, Point2D* unused, RectangleStruct* clipRect);
	static void __fastcall  __Draw_Stuff_When_Selected(TechnoClass* pThis, discard_t, Point2D* pPoint, Point2D* pOriginalPoint, RectangleStruct* pRect);
	static void __fastcall __DrawHealthBar_Selection(TechnoClass* techno, discard_t, Point2D* position, RectangleStruct* clipRect, bool unused);

};