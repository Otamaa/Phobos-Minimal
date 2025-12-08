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

	void Reset() {
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

	struct AEFlags {
		union {
			struct {
				unsigned Cloakable : 1;
				unsigned ForceDecloak : 1;

				unsigned DisableWeapons : 1;
				unsigned DisableSelfHeal : 1;

				unsigned HasRangeModifier : 1;
				unsigned HasTint : 1;
				unsigned HasOnFireDiscardables : 1;
				unsigned HasExtraWarheads : 1;
				unsigned HasFeedbackWeapon : 1;

				unsigned ReflectDamage : 1;
				unsigned Untrackable : 1;

				unsigned DisableRadar : 1;
				unsigned DisableSpySat : 1;
				unsigned Unkillable : 1;

				unsigned _reserved : 18;  // leave unused room for future flags
			};

			uint32_t bits; // raw access
		};

	public:

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {
			return Serialize(Stm);
		}

		bool Save(PhobosStreamWriter& Stm) const {
			return const_cast<AEFlags*>(this)->Serialize(Stm);
		}

	private:

		template <typename T>
		bool Serialize(T& Stm) {
			return Stm
				.Process(this->bits)
				.Success()
				;
		}

	} flags;

public :

	static void Recalculate(TechnoClass* pTechno);
	static void UpdateAEAnimLogic(TechnoClass* pTechno);

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
			.Process(this->ArmorMultData)
			.Process(this->flags)

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

		debugProcess(this->Type, "Type");
		debugProcess(this->AE, "AE");
		debugProcess(this->idxSlot_EMPulse, "idxSlot_EMPulse");
		debugProcess(this->idxSlot_Wave, "idxSlot_Wave");
		debugProcess(this->idxSlot_Beam, "idxSlot_Beam");
		debugProcess(this->idxSlot_Warp, "idxSlot_Warp");
		debugProcess(this->idxSlot_Parasite, "idxSlot_Parasite");
		debugProcess(this->EMPSparkleAnim, "EMPSparkleAnim");
		debugProcess(this->EMPLastMission, "EMPLastMission");
		debugProcess(this->PoweredUnit, "PoweredUnit");
		debugProcess(this->RadarJammer, "RadarJammer");
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
		debugProcess(this->Shield, "Shield");
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
		//debugProcess(this->ReceiveDamageMultiplier, "ReceiveDamageMultiplier");
		debugProcess(this->SkipLowDamageCheck, "SkipLowDamageCheck");
		debugProcess(this->aircraftPutOffsetFlag, "aircraftPutOffsetFlag");
		debugProcess(this->aircraftPutOffset, "aircraftPutOffset");
		debugProcess(this->SkipVoice, "SkipVoice");
		debugProcess(this->ExtraWeaponTimers, "ExtraWeaponTimers");
		debugProcess(this->Trails, "Trails");
		debugProcess(this->MyGiftBox, "MyGiftBox");
		debugProcess(this->PaintBallStates, "PaintBallStates");
		debugProcess(this->DamageSelfState, "DamageSelfState");
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
		debugProcess(this->MyWeaponManager, "MyWeaponManager");
		debugProcess(this->MyDriveData, "MyDriveData");
		debugProcess(this->MyDiveData, "MyDiveData");
		debugProcess(this->MySpawnSuport, "MySpawnSuport");
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
		debugProcess(this->CustomFiringOffset, "CustomFiringOffset");
		debugProcess(this->LastWeaponType, "LastWeaponType");
		debugProcess(this->AirstrikeTargetingMe, "AirstrikeTargetingMe");
		debugProcess(this->RandomEMPTarget, "RandomEMPTarget");
		debugProcess(this->ForceFullRearmDelay, "ForceFullRearmDelay");
		debugProcess(this->AttackMoveFollowerTempCount, "AttackMoveFollowerTempCount");
		debugProcess(this->OnlyAttackData, "OnlyAttackData");
		debugProcess(this->IsSelected, "IsSelected");
		debugProcess(this->UndergroundTracked, "UndergroundTracked");
		debugProcess(this->PassiveAquireMode, "PassiveAquireMode");
		debugProcess(this->UnitIdleAction, "UnitIdleAction");
		debugProcess(this->UnitIdleActionSelected, "UnitIdleActionSelected");
		debugProcess(this->UnitIdleIsSelected, "UnitIdleIsSelected");
		debugProcess(this->UnitIdleActionTimer, "UnitIdleActionTimer");
		debugProcess(this->UnitIdleActionGapTimer, "UnitIdleActionGapTimer");
		debugProcess(this->Tints, "Tints");
		debugProcess(this->FallingDownTracked, "FallingDownTracked");
		debugProcess(this->ResetLocomotor, "ResetLocomotor");
		debugProcess(this->JumpjetStraightAscend, "JumpjetStraightAscend");
		debugProcess(this->CanFireWeaponType, "CanFireWeaponType");
		debugProcess(this->ExtraTurretRecoil, "ExtraTurretRecoil");
		debugProcess(this->ExtraBarrelRecoil, "ExtraBarrelRecoil");
	}



public:
	using base_type = TechnoClass;
public:
#pragma region ClassMembers
	TechnoTypeClass* Type; //original Type pointer

	AEProperties AE;
	BYTE idxSlot_EMPulse;
	BYTE idxSlot_Wave; //5
	BYTE idxSlot_Beam; //6
	BYTE idxSlot_Warp; //7
	BYTE idxSlot_Parasite; //8

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
	//OptionalStruct<double, true> ReceiveDamageMultiplier;
	bool SkipLowDamageCheck;

	bool aircraftPutOffsetFlag;
	bool aircraftPutOffset;
	bool SkipVoice;

	PhobosMap<WeaponTypeClass*, CDTimerClass> ExtraWeaponTimers;

	HelperedVector<std::unique_ptr<UniversalTrail>> Trails;
	std::unique_ptr<GiftBox> MyGiftBox;
	PhobosMap<WarheadTypeClass*, PaintBall> PaintBallStates;
	std::unique_ptr<DamageSelfState> DamageSelfState;

	int CurrentWeaponIdx;

	FireWeaponManager MyWeaponManager;
	DriveData MyDriveData;
	AircraftDive MyDiveData;

	SpawnSupport MySpawnSuport;

	CDTimerClass WarpedOutDelay;

	TemporalClass* MyOriginalTemporal;

	bool SupressEVALost;
	CDTimerClass SelfHealing_CombatDelay;
	bool PayloadCreated;
	bool PayloadTriggered;
	SuperClass* LinkedSW;
	CellStruct SuperTarget;

	InfantryTypeClass* HijackerLastDisguiseType;
	HouseClass* HijackerLastDisguiseHouse;

	int WHAnimRemainingCreationInterval;

	//====
	bool IsWebbed;
	Handle<AnimClass*, UninitAnim> WebbedAnim;
	AbstractClass* WebbyLastTarget;
	Mission WebbyLastMission;

	AresAEData AeData;

	CDTimerClass MergePreventionTimer;

	NewTiberiumStorageClass TiberiumStorage;

	HelperedVector<std::unique_ptr<PhobosAttachEffectClass>> PhobosAE;

	CellClass* FiringObstacleCell; // Set on firing if there is an obstacle cell between target and techno, used for updating WaveClass target etc.
	OptionalStruct<int, true> AdditionalRange;
	bool IsDetachingForCloak; // After TechnoClass::Cloak() has been called but before detaching everything from the object & before CloakState has been updated.

	bool HasRemainingWarpInDelay; // Converted from object with Teleport Locomotor to one with a different Locomotor while still phasing in.
	int LastWarpInDelay;          // Last-warp in delay for this unit, used by HasRemainingWarpInDelay
	bool IsBeingChronoSphered; // Set to true on units currently being ChronoSphered, does not apply to Ares-ChronoSphere'd buildings or Chrono reinforcements.

	CellClass* SubterraneanHarvRallyPoint;

	CDTimerClass TiberiumEaterTimer;
	WarheadTypeClass* LastDamageWH;

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

	AirstrikeClass* AirstrikeTargetingMe;
	CellStruct RandomEMPTarget;

	bool ForceFullRearmDelay;
	int AttackMoveFollowerTempCount;
	HelperedVector<OnlyAttackStruct> OnlyAttackData;
	bool IsSelected;

	bool UndergroundTracked;
	PassiveAcquireMode PassiveAquireMode;

	bool UnitIdleAction;
	bool UnitIdleActionSelected;
	bool UnitIdleIsSelected;
	CDTimerClass UnitIdleActionTimer;
	CDTimerClass UnitIdleActionGapTimer;
	TintColors Tints;

	bool FallingDownTracked;
	bool ResetLocomotor;
	bool JumpjetStraightAscend;

	WeaponTypeClass* CanFireWeaponType;

	std::vector<RecoilData> ExtraTurretRecoil;
	std::vector<RecoilData> ExtraBarrelRecoil;

#pragma endregion

public:

	TechnoExtData(TechnoClass* abs) : RadioExtData(abs),
		Type(nullptr),

		AE(),
		idxSlot_EMPulse(0),
		idxSlot_Wave(0),
		idxSlot_Beam(0),
		idxSlot_Warp(0),
		idxSlot_Parasite(0),

		EMPSparkleAnim(nullptr),
		EMPLastMission(Mission::Sleep),

		PoweredUnit(nullptr),
		RadarJammer(nullptr),

		BuildingLight(nullptr),

		OriginalHouseType(nullptr),
		CloakSkipTimer(),
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
		LaserTrails(),
		ReceiveDamage(false),
		LastKillWasTeamTarget(false),
		PassengerDeletionTimer(),
		CurrentShieldType(nullptr),
		LastWarpDistance(0),
		Death_Countdown(),
		MindControlRingAnimType(nullptr),
		DamageNumberOffset(INT32_MIN),
		CurrentLaserWeaponIndex(),
		OriginalPassengerOwner(nullptr),

		IsInTunnel(false),
		IsBurrowed(false),
		DeployFireTimer(),
		DisableWeaponTimer(),

		RevengeWeapons(),

		GattlingDmageDelay(-1),
		GattlingDmageSound(false),
		AircraftOpentoppedInitEd(false),

		EngineerCaptureDelay(),

		FlhChanged(false),
		//ReceiveDamageMultiplier(),
		SkipLowDamageCheck(false),

		aircraftPutOffsetFlag(false),
		aircraftPutOffset(false),
		SkipVoice(false),

		ExtraWeaponTimers(),

		Trails(),
		MyGiftBox(nullptr),
		PaintBallStates(),
		DamageSelfState(nullptr),

		CurrentWeaponIdx(-1),

		MyWeaponManager(),
		MyDriveData(),
		MyDiveData(),

		MySpawnSuport(),

		WarpedOutDelay(),

		MyOriginalTemporal(nullptr),

		SupressEVALost(false),
		SelfHealing_CombatDelay(),
		PayloadCreated(false),
		PayloadTriggered(false),
		LinkedSW(nullptr),
		SuperTarget(),

		HijackerLastDisguiseType(nullptr),
		HijackerLastDisguiseHouse(nullptr),

		WHAnimRemainingCreationInterval(0),

		IsWebbed(false),
		WebbedAnim(nullptr),
		WebbyLastTarget(nullptr),
		WebbyLastMission(Mission::Sleep),

		AeData(),

		MergePreventionTimer(),

		TiberiumStorage(),

		PhobosAE(),

		FiringObstacleCell(nullptr),
		AdditionalRange(),
		IsDetachingForCloak(false),

		HasRemainingWarpInDelay(false),
		LastWarpInDelay(0),
		IsBeingChronoSphered(false),

		SubterraneanHarvRallyPoint(nullptr),

		TiberiumEaterTimer(),
		LastDamageWH(nullptr),

		MyTargetingFrame(0),

		ChargeTurretTimer(),
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
		DelayedFireTimer(),
		CurrentDelayedFireAnim(nullptr),
		CustomFiringOffset(),

		LastWeaponType(nullptr),
		ElectricBolts(),
		LastHurtFrame(0),

		AirstrikeTargetingMe(nullptr),
		RandomEMPTarget(),

		ForceFullRearmDelay(false),
		AttackMoveFollowerTempCount(0),
		OnlyAttackData(),
		IsSelected(false),

		UndergroundTracked(false),
		PassiveAquireMode(PassiveAcquireMode::Normal),

		UnitIdleAction(false),
		UnitIdleActionSelected(false),
		UnitIdleIsSelected(false),
		UnitIdleActionTimer(),
		UnitIdleActionGapTimer(),
		Tints(),
		FallingDownTracked { false },
		ResetLocomotor { false } ,
		JumpjetStraightAscend { },
		CanFireWeaponType { },
		ExtraTurretRecoil { },
		ExtraBarrelRecoil { }
	{
		// ensure tib storage sized properly
		TiberiumStorage.m_values.resize(TiberiumClass::Array->Count);

		// randomized initial targeting frame
		MyTargetingFrame = ScenarioClass::Instance->Random.RandomRanged(0, 15);

		// set tint owner
		Tints.SetOwner(abs);
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

	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds , SHPStruct* shape , ConvertClass* convert);
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

	static bool SimpleDeployerAllowedToDeploy(UnitClass* pThis, bool defaultValue, bool alwaysCheckLandTypes);

	static int ApplyTintColor(TechnoClass* pThis, bool invulnerability, bool airstrike, bool berserk);
	static void ApplyCustomTint(TechnoClass* pThis, int* tintColor, int* intensity);

	static void Fastenteraction(FootClass* pThis);

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
	static void __fastcall __DrawAirstrikeFlare(TechnoClass* pThis, discard_t, const CoordStruct& startCoord, int startHeight, int endHeight, const CoordStruct& endCoord);
	static AbstractClass* __fastcall __Greatest_Threat(TechnoClass* techno, discard_t, ThreatType method, CoordStruct* location, bool a4);
	static void __fastcall __Draw_Pips(TechnoClass* techno, discard_t, Point2D* position, Point2D* unused, RectangleStruct* clipRect);
	static void __fastcall  __Draw_Stuff_When_Selected(TechnoClass* pThis, discard_t, Point2D* pPoint, Point2D* pOriginalPoint, RectangleStruct* pRect);
	static void __fastcall __DrawHealthBar_Selection(TechnoClass* techno, discard_t, Point2D* position, RectangleStruct* clipRect, bool unused);
	static void __fastcall __Draw_Airstrike_Flare(TechnoClass* techno, discard_t, CoordStruct* startCoord, int startZ, int endZ, CoordStruct* endCoord);

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
};