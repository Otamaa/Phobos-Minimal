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

#include <New/Type/DigitalDisplayTypeClass.h>

#include <Utilities/BuildingBrackedPositionData.h>
#include <Utilities/MemoryPoolUniquePointer.h>

#include <New/Entity/PoweredUnitClass.h>
#include <New/Entity/RadarJammerClass.h>

#include <Misc/Ares/Hooks/Classes/AttachedAffects.h>

#include <New/Entity/NewTiberiumStorageClass.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectClass.h>

#include <TemporalClass.h>
#include <EBolt.h>

#include <Ext/Radio/Body.h>

class BulletClass;
class TechnoTypeClass;
class REGISTERS;
struct BurstFLHBundle;

struct AEProperties
{
	struct ExtraRange
	{
		struct RangeData
		{
			double rangeMult { 1.0 };
			double extraRange { 0.0 };
			VectorSet<WeaponTypeClass*> allow {};
			VectorSet<WeaponTypeClass*> disallow {};

			bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
			{
				return this->Serialize(Stm);
			}

			bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
			{
				return const_cast<RangeData*>(this)->Serialize(Stm);
			}

			bool Eligible(WeaponTypeClass* who)
			{
				bool allowed = false;

				if (allow.begin() != allow.end())
				{
					for (auto iter_allow = allow.begin(); iter_allow != allow.end(); ++iter_allow)
					{
						if (*iter_allow == who)
						{
							allowed = true;
							break;
						}
					}
				}
				else
				{
					allowed = true;
				}

				if (allowed && disallow.begin() != disallow.end())
				{
					for (auto iter_disallow = disallow.begin(); iter_disallow != disallow.end(); ++iter_disallow)
					{
						if (*iter_disallow == who)
						{
							allowed = false;
							break;
						}
					}
				}

				return allowed;
			}
		private:

			template <typename T>
			bool FORCEDINLINE Serialize(T& Stm)
			{
				return Stm
					.Process(this->rangeMult)
					.Process(this->extraRange)
					.Process(this->allow)
					.Process(this->disallow)
					.Success()
					;
			}
		};

		struct RangeDataOut
		{
			double rangeMult { 1.0 };
			double extraRange { 0.0 };
		};

		HelperedVector<RangeData> ranges { };

		bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return this->Serialize(Stm);
		}

		bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<ExtraRange*>(this)->Serialize(Stm);
		}

		COMPILETIMEEVAL void Clear()
		{
			ranges.clear();
		}

		COMPILETIMEEVAL bool Enabled()
		{
			return !ranges.empty();
		}

		COMPILETIMEEVAL int Get(int initial, WeaponTypeClass* who)
		{
			int add = 0;
			for (auto& ex_range : ranges)
			{

				if (!ex_range.Eligible(who))
					continue;

				initial = static_cast<int>(initial * MaxImpl(ex_range.rangeMult, 0.0));
				add += static_cast<int>(ex_range.extraRange);
			}

			return initial + add;
		}

		COMPILETIMEEVAL void FillEligible(WeaponTypeClass* who, std::vector<RangeDataOut>& eligible)
		{
			for (auto& ex_range : this->ranges)
			{
				if (ex_range.Eligible(who))
				{
					eligible.emplace_back(ex_range.rangeMult, ex_range.extraRange);
				}
			}
		}

		static COMPILETIMEEVAL int Count(int initial, std::vector<RangeDataOut>& eligible)
		{
			int add = 0;
			for (auto& ex_range : eligible)
			{
				initial = static_cast<int>(initial * MaxImpl(ex_range.rangeMult, 0.0));
				add += static_cast<int>(ex_range.extraRange);
			}

			return initial + add;
		}

	private:

		template <typename T>
		bool FORCEDINLINE Serialize(T& Stm)
		{
			return Stm
				.Process(this->ranges)
				.Success()
				;
		}
	} ExtraRange {};

	struct ExtraCrit
	{
		struct CritData
		{
			double Mult { 1.0 };
			double extra { 0.0 };
			VectorSet<WarheadTypeClass*> allow {};
			VectorSet<WarheadTypeClass*> disallow {};

			bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
			{
				return this->Serialize(Stm);
			}

			bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
			{
				return const_cast<CritData*>(this)->Serialize(Stm);
			}

			bool Eligible(WarheadTypeClass* who)
			{

				bool allowed = false;

				if (allow.begin() != allow.end())
				{
					for (auto iter_allow = allow.begin(); iter_allow != allow.end(); ++iter_allow)
					{
						if (*iter_allow == who)
						{
							allowed = true;
							break;
						}
					}
				}
				else
				{
					allowed = true;
				}

				if (allowed && disallow.begin() != disallow.end())
				{
					for (auto iter_disallow = disallow.begin(); iter_disallow != disallow.end(); ++iter_disallow)
					{
						if (*iter_disallow == who)
						{
							allowed = false;
							break;
						}
					}
				}

				return allowed;
			}
		private:

			template <typename T>
			bool FORCEDINLINE Serialize(T& Stm)
			{
				return Stm
					.Process(this->Mult)
					.Process(this->extra)
					.Process(this->allow)
					.Process(this->disallow)
					.Success()
					;
			}
		};

		struct CritDataOut
		{
			double Mult { 1.0 };
			double extra { 0.0 };
		};

		HelperedVector<CritData> ranges { };

		bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return this->Serialize(Stm);
		}

		bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<ExtraCrit*>(this)->Serialize(Stm);
		}

		COMPILETIMEEVAL void Clear()
		{
			ranges.clear();
		}

		COMPILETIMEEVAL bool Enabled()
		{
			return !ranges.empty();
		}

		COMPILETIMEEVAL double Get(double initial, WarheadTypeClass* who)
		{
			double add = 0.0;
			for (auto& ex_range : ranges)
			{

				if (!ex_range.Eligible(who))
					continue;

				initial = initial * ex_range.Mult;
				add += ex_range.extra;
			}

			return initial + add;
		}

		COMPILETIMEEVAL void FillEligible(WarheadTypeClass* who, std::vector<CritDataOut>& eligible)
		{
			for (auto& ex_range : this->ranges)
			{
				if (ex_range.Eligible(who))
				{
					eligible.emplace_back(ex_range.Mult, ex_range.extra);
				}
			}
		}

		static COMPILETIMEEVAL double Count(double initial, std::vector<CritDataOut>& eligible)
		{
			double add = 0.0;
			for (auto& ex_range : eligible)
			{
				initial *= MaxImpl(ex_range.Mult, 0.0);
				add += ex_range.extra;
			}

			return initial + add;
		}

	private:

		template <typename T>
		bool FORCEDINLINE Serialize(T& Stm)
		{
			return Stm
				.Process(this->ranges)
				.Success()
				;
		}
	} ExtraCrit {};

	struct ArmorMult
	{
		struct MultData
		{
			double Mult { 1.0 };
			VectorSet<WarheadTypeClass*> allow {};
			VectorSet<WarheadTypeClass*> disallow {};

			bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
			{
				return this->Serialize(Stm);
			}

			bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
			{
				return const_cast<MultData*>(this)->Serialize(Stm);
			}

			bool Eligible(WarheadTypeClass* who)
			{
				bool allowed = false;

				if (allow.begin() != allow.end())
				{
					for (auto iter_allow = allow.begin(); iter_allow != allow.end(); ++iter_allow)
					{
						if (*iter_allow == who)
						{
							allowed = true;
							break;
						}
					}
				}
				else
				{
					allowed = true;
				}

				if (allowed && disallow.begin() != disallow.end())
				{
					for (auto iter_disallow = disallow.begin(); iter_disallow != disallow.end(); ++iter_disallow)
					{
						if (*iter_disallow == who)
						{
							allowed = false;
							break;
						}
					}
				}

				return allowed;
			}
		private:

			template <typename T>
			bool FORCEDINLINE Serialize(T& Stm)
			{
				return Stm
					.Process(this->Mult)
					.Process(this->allow)
					.Process(this->disallow)
					.Success()
					;
			}
		};

		HelperedVector<MultData> mults { };

		bool FORCEDINLINE Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return this->Serialize(Stm);
		}

		bool FORCEDINLINE Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<ArmorMult*>(this)->Serialize(Stm);
		}

		COMPILETIMEEVAL void Clear()
		{
			mults.clear();
		}

		COMPILETIMEEVAL bool Enabled()
		{
			return !mults.empty();
		}

		COMPILETIMEEVAL double Get(double initial, WarheadTypeClass* who)
		{
			for (auto& ex_range : mults)
			{

				if (!ex_range.Eligible(who))
					continue;

				initial *= ex_range.Mult;
			}

			return initial;
		}

		COMPILETIMEEVAL void FillEligible(WarheadTypeClass* who, std::vector<double>& eligible)
		{
			for (auto& ex_range : this->mults) {
				if (ex_range.Eligible(who)) {
					eligible.emplace_back(ex_range.Mult);
				}
			}
		}

		static COMPILETIMEEVAL double Apply(double initial, std::vector<double>& eligible)
		{
			for (auto& ex_range : eligible) {
				initial *= ex_range;
			}

			return initial;
		}
		private:

		template <typename T>
		bool FORCEDINLINE Serialize(T& Stm)
		{
			return Stm
				.Process(this->mults)
				.Success()
				;
		}
	} ArmorMultData {};

	double FirepowerMultiplier { 1.0 };
	double ArmorMultiplier { 1.0 };
	double SpeedMultiplier { 1.0 };
	double ROFMultiplier { 1.0 };
	double ReceiveRelativeDamageMult { 1.0 };

	//TODO :
	int FirepowerBonus { 0 };
	int ArmorBonus { 0 };
	double SpeedBonus { 0.0 };
	int ROFBonus { 0 };

	//TODO :
	MinMaxValue<int> ReceivedDamage { INT32_MIN , INT32_MAX };
	MinMaxValue<double> Speed { 0.0 ,  INT32_MAX };

	bool Cloakable { false };
	bool ForceDecloak { false };

	bool DisableWeapons { false };
	bool DisableSelfHeal { false };

	bool HasRangeModifier { false };
	bool HasTint { false };
	bool HasOnFireDiscardables { false };
	bool HasExtraWarheads { false };
	bool HasFeedbackWeapon { false };

	bool ReflectDamage { false };
	bool Untrackable { false };

	bool DisableRadar { false };
	bool DisableSpySat { false };
	bool Unkillable { false };

public :

	static void Recalculate(TechnoClass* pTechno);

public :

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEProperties*>(this)->Serialize(Stm);
	}

protected:
	template <typename T>
	bool Serialize(T& Stm) {
		return Stm
			.Process(this->ExtraRange)
			.Process(this->ExtraCrit)
			.Process(this->FirepowerMultiplier)
			.Process(this->ArmorMultiplier)
			.Process(this->SpeedMultiplier)
			.Process(this->ROFMultiplier)
			.Process(this->ReceiveRelativeDamageMult)
			.Process(this->FirepowerBonus)
			.Process(this->ArmorBonus)
			.Process(this->SpeedBonus)
			.Process(this->ROFBonus)
			.Process(this->ReceivedDamage)
			.Process(this->Speed)
			.Process(this->Cloakable)
			.Process(this->ForceDecloak)
			.Process(this->DisableWeapons)
			.Process(this->DisableSelfHeal)
			.Process(this->HasRangeModifier)
			.Process(this->HasTint)
			.Process(this->HasOnFireDiscardables)
			.Process(this->HasExtraWarheads)
			.Process(this->HasFeedbackWeapon)
			.Process(this->ReflectDamage)
			.Process(this->Untrackable)
			.Process(this->DisableRadar)
			.Process(this->DisableSpySat)
			.Process(this->ArmorMultData)
			.Process(this->Unkillable)
			.Success() && Stm.RegisterChange(this)
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
		Stm
			.Process(this->Type, true)
			.Process(this->AbsType)
			.Process(this->AE)
			.Process(this->idxSlot_EMPulse)
			.Process(this->idxSlot_Wave)
			.Process(this->idxSlot_Beam)
			.Process(this->idxSlot_Warp)
			.Process(this->idxSlot_Parasite)
			.Process(this->GarrisonedIn, true)
			.Process(this->EMPSparkleAnim)
			.Process(this->EMPLastMission)
			.Process(this->PoweredUnit)
			.Process(this->RadarJammer)
			.Process(this->BuildingLight)
			.Process(this->OriginalHouseType, true)
			.Process(this->CloakSkipTimer)
			.Process(this->HijackerHealth)
			.Process(this->HijackerOwner, true)
			.Process(this->HijackerVeterancy)
			.Process(this->Is_SurvivorsDone)
			.Process(this->Is_DriverKilled)
			.Process(this->Is_Operated)
			.Process(this->Is_UnitLostMuted)
			.Process(this->TakeVehicleMode)
			.Process(this->TechnoValueAmount)
			.Process(this->Pos)
			.Process(this->Shield)
			.Process(this->LaserTrails)
			.Process(this->ReceiveDamage)
			.Process(this->LastKillWasTeamTarget)
			.Process(this->PassengerDeletionTimer)
			.Process(this->CurrentShieldType)
			.Process(this->LastWarpDistance)
			.Process(this->Death_Countdown)
			.Process(this->MindControlRingAnimType)
			.Process(this->DamageNumberOffset)
			.Process(this->CurrentLaserWeaponIndex)
			.Process(this->OriginalPassengerOwner, true)
			.Process(this->IsInTunnel)
			.Process(this->IsBurrowed)
			.Process(this->DeployFireTimer)
			.Process(this->DisableWeaponTimer)
			.Process(this->RevengeWeapons)
			.Process(this->GattlingDmageDelay)
			.Process(this->GattlingDmageSound)
			.Process(this->AircraftOpentoppedInitEd)
			.Process(this->EngineerCaptureDelay)
			.Process(this->FlhChanged)
			.Process(this->ReceiveDamageMultiplier)
			.Process(this->SkipLowDamageCheck)
			.Process(this->aircraftPutOffsetFlag)
			.Process(this->aircraftPutOffset)
			.Process(this->SkipVoice)
			.Process(this->ExtraWeaponTimers)
			.Process(this->Trails)
			.Process(this->MyGiftBox)
			.Process(this->PaintBallStates, true)
			.Process(this->DamageSelfState)
			.Process(this->CurrentWeaponIdx)
			.Process(this->MyFighterData)
			.Process(this->WarpedOutDelay)
			.Process(this->AltOccupation)
			.Process(this->MyOriginalTemporal, true)
			.Process(this->SupressEVALost)
			.Process(this->SelfHealing_CombatDelay)
			.Process(this->PayloadCreated)
			.Process(this->PayloadTriggered)
			.Process(this->LinkedSW, true)
			.Process(this->SuperTarget)
			.Process(this->HijackerLastDisguiseType, true)
			.Process(this->HijackerLastDisguiseHouse, true)
			.Process(this->Convert_Deploy_Delay)
			.Process(this->WHAnimRemainingCreationInterval)
			.Process(this->IsWebbed)
			.Process(this->WebbedAnim, true)
			.Process(this->WebbyLastTarget, true)
			.Process(this->WebbyLastMission)
			.Process(this->FreeUnitDone)
			.Process(this->AeData)
			.Process(this->Strafe_BombsDroppedThisRound)
			.Process(this->MergePreventionTimer)
			.Process(this->TiberiumStorage)
			.Process(this->MyWeaponManager)
			.Process(this->MyDriveData)
			.Process(this->MyDiveData)
			.Process(this->MySpawnSuport)
			.Process(this->PhobosAE)
			.Process(this->ShootCount)
			.Process(this->FiringObstacleCell)
			.Process(this->AdditionalRange)
			.Process(this->IsDetachingForCloak)
			.Process(this->CurrentAircraftWeaponIndex)
			.Process(this->HasRemainingWarpInDelay)
			.Process(this->LastWarpInDelay)
			.Process(this->UnitAutoDeployTimer)
			.Process(this->SubterraneanHarvRallyPoint, true)
			.Process(this->IsBeingChronoSphered)
			.Process(this->TiberiumEaterTimer)
			.Process(this->LastDamageWH)
			.Process(this->UnitIdleAction)
			.Process(this->UnitIdleActionSelected)
			.Process(this->UnitIdleIsSelected)
			.Process(this->UnitIdleActionTimer)
			.Process(this->UnitIdleActionGapTimer)
			.Process(this->MyTargetingFrame)
			.Process(this->ChargeTurretTimer)
			.Process(this->LastRearmWasFullDelay)
			.Process(this->DropCrate)
			.Process(this->DropCrateType)
			.Process(this->LastBeLockedFrame)
			.Process(this->BeControlledThreatFrame)
			.Process(this->LastTargetID)
			.Process(this->LastHurtFrame)
			.Process(this->AttachedEffectInvokerCount)
			.Process(this->AccumulatedGattlingValue)
			.Process(this->ShouldUpdateGattlingValue)
			.Process(this->KeepTargetOnMove)
			.Process(this->LastSensorsMapCoords)
			.Process(this->DelayedFireSequencePaused)
			.Process(this->DelayedFireTimer)
			.Process(this->DelayedFireWeaponIndex)
			.Process(this->CurrentDelayedFireAnim)
			.Process(this->CustomFiringOffset)
			.Process(this->LastWeaponType)
			.Process(this->AirstrikeTargetingMe)
			.Process(this->RandomEMPTarget)
			.Process(this->FiringAnimationTimer)
			.Process(this->ForceFullRearmDelay)
			.Process(this->AttackMoveFollowerTempCount)
			.Process(this->OnlyAttackData)
			.Process(this->Strafe_TargetCell)
			.Process(this->IsSelected)
			.Process(this->SimpleDeployerAnimationTimer)
			;
	}


public:
	using base_type = TechnoClass;
public:
#pragma region ClassMembers
	TechnoTypeClass* Type; //original Type pointer
	OptionalStruct<AbstractType, true> AbsType;

	AEProperties AE;
	BYTE idxSlot_EMPulse;
	BYTE idxSlot_Wave; //5
	BYTE idxSlot_Beam; //6
	BYTE idxSlot_Warp; //7
	BYTE idxSlot_Parasite; //8
	BuildingClass* GarrisonedIn; //C
	Handle<AnimClass*, UninitAnim> EMPSparkleAnim;
	Mission EMPLastMission; //

	std::unique_ptr<PoweredUnitClass> PoweredUnit;
	std::unique_ptr<RadarJammerClass> RadarJammer;

	BuildingLightClass* BuildingLight;

	HouseTypeClass* OriginalHouseType;
	CDTimerClass CloakSkipTimer; //
	int HijackerHealth;
	HouseClass* HijackerOwner;
	float HijackerVeterancy;
	BYTE Is_SurvivorsDone;
	BYTE Is_DriverKilled;
	BYTE Is_Operated;
	BYTE Is_UnitLostMuted;
	BYTE TakeVehicleMode;
	int TechnoValueAmount;
	int Pos;
	std::unique_ptr<ShieldClass> Shield;
	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails;
	bool ReceiveDamage;
	bool LastKillWasTeamTarget;
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

	bool IsInTunnel;
	bool IsBurrowed;
	CDTimerClass DeployFireTimer;
	CDTimerClass DisableWeaponTimer;

	HelperedVector<TimedWarheadValue<WeaponTypeClass*>> RevengeWeapons;

	int GattlingDmageDelay;
	bool GattlingDmageSound;
	bool AircraftOpentoppedInitEd;

	CDTimerClass EngineerCaptureDelay;

	bool FlhChanged;
	OptionalStruct<double, true> ReceiveDamageMultiplier;
	bool SkipLowDamageCheck;

	bool aircraftPutOffsetFlag;
	bool aircraftPutOffset;
	bool SkipVoice;

	PhobosMap<WeaponTypeClass*, CDTimerClass> ExtraWeaponTimers;

	HelperedVector<UniversalTrail> Trails;
	std::unique_ptr<GiftBox> MyGiftBox;
	PhobosMap<WarheadTypeClass*, PaintBall> PaintBallStates;
	std::unique_ptr<DamageSelfState> DamageSelfState;

	int CurrentWeaponIdx;

	FireWeaponManager MyWeaponManager;
	DriveData MyDriveData;
	AircraftDive MyDiveData;

	SpawnSupport MySpawnSuport;

	std::unique_ptr<FighterAreaGuard> MyFighterData;

	CDTimerClass WarpedOutDelay;

	OptionalStruct<bool, true> AltOccupation; // if the unit marks cell occupation flags, this is set to whether it uses the "high" occupation members
	TemporalClass* MyOriginalTemporal;

	bool SupressEVALost;
	CDTimerClass SelfHealing_CombatDelay;
	bool PayloadCreated;
	bool PayloadTriggered;
	SuperClass* LinkedSW;
	CellStruct SuperTarget;

	InfantryTypeClass* HijackerLastDisguiseType;
	HouseClass* HijackerLastDisguiseHouse;

	CDTimerClass Convert_Deploy_Delay;

	int WHAnimRemainingCreationInterval;

	//====
	bool IsWebbed;
	Handle<AnimClass*, UninitAnim> WebbedAnim;
	AbstractClass* WebbyLastTarget;
	Mission WebbyLastMission;

	bool FreeUnitDone;
	AresAEData AeData;

	int Strafe_BombsDroppedThisRound;
	CDTimerClass MergePreventionTimer;

	NewTiberiumStorageClass TiberiumStorage;

	HelperedVector<std::unique_ptr<PhobosAttachEffectClass>> PhobosAE;

	int ShootCount;
	int CurrentAircraftWeaponIndex;

	CellClass* FiringObstacleCell; // Set on firing if there is an obstacle cell between target and techno, used for updating WaveClass target etc.
	OptionalStruct<int, true> AdditionalRange;
	bool IsDetachingForCloak; // After TechnoClass::Cloak() has been called but before detaching everything from the object & before CloakState has been updated.

	bool HasRemainingWarpInDelay; // Converted from object with Teleport Locomotor to one with a different Locomotor while still phasing in.
	int LastWarpInDelay;          // Last-warp in delay for this unit, used by HasRemainingWarpInDelay
	bool IsBeingChronoSphered; // Set to true on units currently being ChronoSphered, does not apply to Ares-ChronoSphere'd buildings or Chrono reinforcements.

	CDTimerClass UnitAutoDeployTimer;
	CellClass* SubterraneanHarvRallyPoint;

	CDTimerClass TiberiumEaterTimer;
	WarheadTypeClass* LastDamageWH;

	bool UnitIdleAction;
	bool UnitIdleActionSelected;
	bool UnitIdleIsSelected;
	CDTimerClass UnitIdleActionTimer;
	CDTimerClass UnitIdleActionGapTimer;

	int MyTargetingFrame;

	CDTimerClass ChargeTurretTimer;// Used for charge turrets instead of RearmTimer if weapon has ChargeTurret.Delays set.
	bool LastRearmWasFullDelay;

	int DropCrate; // Drop crate on death, modified by map action
	PowerupEffects DropCrateType;

	int LastBeLockedFrame;
	int BeControlledThreatFrame;

	DWORD LastTargetID;
	int AccumulatedGattlingValue;
	bool ShouldUpdateGattlingValue;

	bool KeepTargetOnMove;
	CellStruct LastSensorsMapCoords;
	bool DelayedFireSequencePaused;
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

	CDTimerClass FiringAnimationTimer;
	bool ForceFullRearmDelay;
	int AttackMoveFollowerTempCount;
	HelperedVector<OnlyAttackStruct> OnlyAttackData;
	CellClass* Strafe_TargetCell;
	bool IsSelected;
	// Replaces use of TechnoClass->Animation StageClass timer for IsSimpleDeployer to simplify
	// the deploy animation timer calcs and eliminate possibility of outside interference.
	CDTimerClass SimpleDeployerAnimationTimer;

#pragma endregion

public:

	TechnoExtData(TechnoClass* abs) : RadioExtData(abs),
		Type(nullptr),
		idxSlot_EMPulse(0),
		idxSlot_Wave(0),
		idxSlot_Beam(0),
		idxSlot_Warp(0),
		idxSlot_Parasite(0),
		GarrisonedIn(nullptr),
		EMPSparkleAnim(nullptr),
		EMPLastMission(Mission(0)),
		PoweredUnit(nullptr),
		RadarJammer(nullptr),
		BuildingLight(nullptr),
		OriginalHouseType(nullptr),
		HijackerHealth(0),
		HijackerOwner(nullptr),
		HijackerVeterancy(0.0f),
		Is_SurvivorsDone(0),
		Is_DriverKilled(0),
		Is_Operated(0),
		Is_UnitLostMuted(0),
		TakeVehicleMode(0),
		TechnoValueAmount(0),
		Pos(0),
		Shield(nullptr),
		ReceiveDamage(false),
		LastKillWasTeamTarget(false),
		CurrentShieldType(nullptr),
		LastWarpDistance(0),
		MindControlRingAnimType(nullptr),
		DamageNumberOffset(INT32_MIN),
		OriginalPassengerOwner(nullptr),
		IsInTunnel(false),
		IsBurrowed(false),
		GattlingDmageDelay(-1),
		GattlingDmageSound(false),
		AircraftOpentoppedInitEd(false),
		FlhChanged(false),
		SkipLowDamageCheck(false),
		aircraftPutOffsetFlag(false),
		aircraftPutOffset(false),
		SkipVoice(false),
		CurrentWeaponIdx(-1),
		MyOriginalTemporal(nullptr),
		SupressEVALost(false),
		PayloadCreated(false),
		PayloadTriggered(false),
		LinkedSW(nullptr),
		HijackerLastDisguiseType(nullptr),
		HijackerLastDisguiseHouse(nullptr),
		WHAnimRemainingCreationInterval(0),
		IsWebbed(false),
		WebbedAnim(nullptr),
		WebbyLastTarget(nullptr),
		WebbyLastMission(Mission::Sleep),
		FreeUnitDone(false),
		Strafe_BombsDroppedThisRound(0),
		ShootCount(0),
		CurrentAircraftWeaponIndex(0),
		FiringObstacleCell(nullptr),
		IsDetachingForCloak(false),
		HasRemainingWarpInDelay(false),
		LastWarpInDelay(0),
		IsBeingChronoSphered(false),
		SubterraneanHarvRallyPoint(nullptr),
		LastDamageWH(nullptr),
		UnitIdleAction(false),
		UnitIdleActionSelected(false),
		UnitIdleIsSelected(false),
		LastRearmWasFullDelay(false),
		DropCrate(-1),
		DropCrateType(PowerupEffects::Money),
		LastBeLockedFrame(0),
		BeControlledThreatFrame(0),
		LastTargetID(0xFFFFFFFF),
		AccumulatedGattlingValue(0),
		ShouldUpdateGattlingValue(false),
		KeepTargetOnMove(false),
		LastSensorsMapCoords(),
		DelayedFireSequencePaused(false),
		DelayedFireWeaponIndex(-1),
		CurrentDelayedFireAnim(nullptr),
		LastWeaponType(nullptr),
		LastHurtFrame(0),
		AttachedEffectInvokerCount(0),
		AirstrikeTargetingMe(nullptr),
		ForceFullRearmDelay(false),
		AttackMoveFollowerTempCount(0),
		Strafe_TargetCell(),
		IsSelected(false)
	{
		MyTargetingFrame = ScenarioClass::Instance->Random.RandomRanged(0, 15);
	}

	TechnoExtData(TechnoClass* abs, noinit_t& noint) : RadioExtData(abs, noint) { };

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

	virtual void CalculateCRC(CRCEngine& crc) const override {
		this->RadioExtData::CalculateCRC(crc);
	}

public:

	FORCEDINLINE ShieldClass* GetShield() const {
		return this->Shield.get();
	}

	void ClearElectricBolts()
	{
		for (auto const pBolt : this->ElectricBolts) {
			if(pBolt)
				pBolt->Owner = nullptr;
		}

		this->ElectricBolts.clear();
	}

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

	void StopIdleAction();
	void ApplyIdleAction();
	void ManualIdleAction();
	void StopRotateWithNewROT(int ROT = -1);

	void ResetDelayedFireTimer();

	void CreateDelayedFireAnim(AnimTypeClass* pAnimType, int weaponIndex, bool attach, bool center, bool removeOnNoDelay, bool useOffsetOverride, CoordStruct offsetOverride);

	void AddFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker);
	bool ContainFirer(WeaponTypeClass* const Weapon, TechnoClass* const Attacker) const;
	int FindFirer(WeaponTypeClass* const Weapon) const;

public:

	static void InitializeUnitIdleAction(TechnoClass* pThis, TechnoTypeClass* pType);

	static bool HandleDelayedFireWithPauseSequence(TechnoClass* pThis, int weaponIndex, int firingFrame);

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

	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawParasitedPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis, bool wasDamaged);
	static void ApplyDrainMoney(TechnoClass* pThis);

	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawSelectBox(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds , bool drawBefore = false);
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
	static Point2D GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition bracketPosition , Point2D offset = Point2D::Empty);
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
	static std::tuple<bool, bool , bool> CanBeAffectedByFakeEngineer(TechnoClass* pThis, TechnoClass* pTarget, bool checkBridge = false, bool checkCapturableBuilding = false, bool checkAttachedBombs = false);

	static bool CannotMove(UnitClass* pThis);

	static bool HasAmmoToDeploy(TechnoClass* pThis);
	static void HandleOnDeployAmmoChange(TechnoClass* pThis, int maxAmmoOverride = -1);
public:
	static UnitClass* Deployer;

};

class TechnoExtContainer {
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

	static int __fastcall _EvaluateJustCell(TechnoClass* pThis , discard_t, CellStruct* where);
	static bool __fastcall __TargetSomethingNearby(TechnoClass* pThis, discard_t, CoordStruct* coord, ThreatType threat);
	static int __fastcall __AdjustDamage(TechnoClass* pThis, discard_t, TechnoClass* pTarget, WeaponTypeClass* pWeapon);

};