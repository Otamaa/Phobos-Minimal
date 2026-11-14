#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <New/Entity/PowerPlantEnhancerClass.h>

#include <InfantryClass.h>
#include <InfantryTypeClass.h>
#include <UnitClass.h>
#include <UnitTypeClass.h>
#include <AircraftClass.h>
#include <AircraftTypeClass.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

void __fastcall Recalc_All_Factories_Of_House(HouseClass* house)
{ JMP_FAST(0x4CA6E0); }

// The host and each installed slot contribute once, including repeated types.
template <typename F>
void ForEachPowerComponent(BuildingClass* pBuilding, F action)
{
	action(pBuilding->Type, true);
	if (pBuilding->UpgradeLevel)
	{
		for (const auto pUpgrade : pBuilding->Upgrades)
		{
			if (pUpgrade)
				action(pUpgrade, false);
		}
	}
}

template <typename T, typename F>
T  CalculatePowerOutput(BuildingClass* pBuilding, F getComponent)
{
	if (!pBuilding->HasPower || pBuilding->IsUnderEMP() || pBuilding->IsBeingWarpedOut() || pBuilding->EMPLockRemaining > 0)
		return 0;

	T power = 0;
	T extraPower = 0;
	ForEachPowerComponent(pBuilding, [&](BuildingTypeClass* pType, bool host)
	{
		const auto [output, extra] = getComponent(pType, host);
		power += output;
		extraPower += extra;
	});

	if (power + extraPower <= 0)
		return 0;

	// Preserve the real-power rounding: sum all components before scaling,
	// and leave the enhancer's additive amount independent of damage.
	const double factor = BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->PowerPlant_DamageFactor;
	if (factor == 1.0)
		power = static_cast<T>(power * pBuilding->GetHealthRatio());
	else if (factor != 0.0)
	{
		const double scale = 1.0 - factor + factor * pBuilding->GetHealthRatio();
		power = static_cast<T>(power * scale);
		// Only damage-induced sign changes are floored; Amount is not scaled.
		if (scale < 0.0)
			power = MaxImpl(power, static_cast<T>(0));
	}

	return power + extraPower;
}

template <typename T>
T GetBasePowerOutput(BuildingClass* pBuilding)
{
	const auto pType = pBuilding->Type;
	T output = pType->PowerBonus;
	if (pBuilding->IsOverpowered)
		output += pType->ExtraPowerBonus;

	if (pBuilding->Absorber() && pType->ExtraPowerBonus > 0 && pBuilding->Passengers.NumPassengers > 0)
	{
		for (auto pPas = pBuilding->Passengers.GetFirstPassenger();
		pPas;
		pPas = flag_cast_to<FootClass*>(pPas->NextObject)) {

			output += Math::abs(GET_TECHNOTYPEEXT(pPas)
				->ExtraPower_Amount.Get(pBuilding->Type->ExtraPowerBonus));
		}
	}

	return output;
}

int FakeBuildingClass::__GetPower()
{
	int powerInitial = GetBasePowerOutput<int>(this);

	return HouseExtData::GetPowerOutput(this, powerInitial);
}

std::pair<std::int64_t, std::int64_t> HouseExtData::GetUnitPower(TechnoTypeClass* pType, std::int64_t power, HouseClass* pHouse, TechnoClass* pSource)
{
	// Match buildings: enhance supply (including Amount on zero supply),
	// retain base drain, and perform the pre-damage supply check.
	const auto [output, extra] = PowerPlantEnhancerClass::GetEnhancedPower<std::int64_t>(pType, MaxImpl(power, 0LL), pHouse, pSource);
	return { MaxImpl(output + extra, 0LL), MaxImpl(-power, 0LL) };
}

int HouseExtData::GetPowerOutput(BuildingClass* pBuilding, int baseOutput)
{
	// Native GetPowerOutput checks warped-out state before reaching our hook.
	return CalculatePowerOutput<int>(pBuilding, [&](BuildingTypeClass* pType, bool host)
	{
		return  PowerPlantEnhancerClass::GetEnhancedPower<int>(pType, host ? baseOutput : pType->PowerBonus, pBuilding->Owner, pBuilding);
	});
}

std::int64_t HouseExtData::GetAIPower(BuildingTypeClass* pType)
{
	const auto& power = BuildingTypeExtContainer::Instance.Find(pType)->AI_Power;
	return power.isset() ? power.Fetch() : static_cast<std::int64_t>(pType->PowerBonus) - pType->PowerDrain;
}

std::int64_t HouseExtData::GetAIPower(BuildingClass* pBuilding, bool defaultAffectedByEnhancers, bool outputDisabled, std::int64_t& drain)
{
	drain = 0;
	if (!pBuilding->IsAlive || pBuilding->Health <= 0 || pBuilding->IsBeingWarpedOut() || !pBuilding->HasPower)
		return 0;

	const auto baseOutput = GetBasePowerOutput<std::int64_t>(pBuilding);
	const auto output = CalculatePowerOutput<std::int64_t>(pBuilding, [&](BuildingTypeClass* pType, bool host)
	{
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
		const bool enhance = pExt->AI_Power_AffectedByEnhancers.Get(defaultAffectedByEnhancers);
		const auto nativeOutput = host ? baseOutput : pType->PowerBonus;
		if (!pExt->AI_Power.isset()
			|| pExt->AI_Power.Fetch() == static_cast<std::int64_t>(pType->PowerBonus) - pType->PowerDrain)
		{
			// Preserve the real int/float rounding when the base is unchanged.
			const auto [power, extra] = PowerPlantEnhancerClass::GetEnhancedPower<int>(pType,
				static_cast<int>(nativeOutput), pBuilding->Owner, pBuilding, enhance);
			return std::pair<std::int64_t, std::int64_t> { power, extra };
		}

		const auto value = static_cast<std::int64_t>(pExt->AI_Power.Fetch());
		const std::int64_t componentOutput = MaxImpl(value, 0LL) + (host ? baseOutput - pType->PowerBonus : 0);
		return PowerPlantEnhancerClass::GetEnhancedPower<std::int64_t>(pType, componentOutput, pBuilding->Owner, pBuilding, enhance);
	});

	// Native drain has no health or EMP scaling. AI.Power replaces only its
	// base contribution; the host's extra drain is retained independently.
	ForEachPowerComponent(pBuilding, [&](BuildingTypeClass* pType, bool host)
	{
		const auto& power = TechnoTypeExtContainer::Instance.Find(pType)->AI_Power;
		const auto nativePower = static_cast<std::int64_t>(pType->PowerBonus) - pType->PowerDrain;
		drain += !power.isset() || power.Fetch() == nativePower
			? pType->PowerDrain : MaxImpl(-static_cast<std::int64_t>(power.Fetch()), 0LL);
		if (host && pBuilding->HasExtraPowerDrain)
			drain += pType->ExtraPowerDrain;
	});
	return (outputDisabled ? 0 : output) - drain;
}

std::int64_t HouseExtData::GetAIPowerDelta(BuildingClass* pBuilding, bool defaultAffectedByEnhancers, bool outputDisabled, std::int64_t& drainDelta, bool& aiAdjusted)
{
	drainDelta = 0;
	aiAdjusted = false;
	bool adjusted = false;
	ForEachPowerComponent(pBuilding, [&](BuildingTypeClass* pType, bool)
	{
		const auto pExt = BuildingTypeExtContainer::Instance.Find(pType);
		adjusted |= pExt->AI_Power.isset()
			|| (!pExt->AI_Power_AffectedByEnhancers.Get(defaultAffectedByEnhancers)
				&& !HouseExtContainer::Instance.Find(pBuilding->Owner)->PowerPlantEnhancers.empty());
		if (pExt->AI_Power.isset())
			aiAdjusted |= pExt->AI_Power.Fetch() != static_cast<std::int64_t>(pType->PowerBonus) - pType->PowerDrain;
	});
	if (!adjusted)
		return 0;

	std::int64_t aiDrain;
	const auto power = HouseExtData::GetAIPower(pBuilding, defaultAffectedByEnhancers, outputDisabled, aiDrain);
	const int output = outputDisabled ? 0 : pBuilding->GetPowerOutput();
	const int drain = pBuilding->GetPowerDrain();
	const auto delta = power - output + drain;
	drainDelta = aiDrain - drain;
	aiAdjusted |= delta != 0 || aiDrain != drain;
	return delta;
}

// Consumed later in the same UpdatePower call, after Ares has applied its
// house-wide zero floor. It includes auxiliary power and all local drains.
HouseClass* powerOutputContext = nullptr;
int powerOutputBeforeDrainClamp = 0;

void HouseExtData::RecordPowerOutputBeforeDrainClamp(HouseClass* const pThis)
{
	powerOutputContext = pThis;
	powerOutputBeforeDrainClamp = pThis->PowerOutput;
}

PowerAdjustment HouseExtData::UpdateUnitPower(HouseClass* const pThis)
{
	if (!Phobos::Config::UnitPowerDrain)
		return { };

	std::int64_t output = pThis->PowerOutput;
	std::int64_t drain = pThis->PowerDrain;
	std::int64_t aiDelta = 0;
	std::int64_t aiDrainDelta = 0;
	bool aiAdjusted = false;
	const bool affectedByEnhancers = FakeRulesClass::Instance->AI_Power_AffectedByEnhancers;
	
	std::vector<const TechnoTypeClass*> enhancedTypes;
	enhancedTypes.reserve(HouseExtContainer::Instance.Find(pThis)->PowerPlantEnhancers.size());

	for (const auto pEnhancer : HouseExtContainer::Instance.Find(pThis)->PowerPlantEnhancers)
	{
		if (!TechnoExtData::IsActive(pEnhancer) || !pEnhancer->HasPower)
			continue;

		for (const auto pType : BuildingTypeExtContainer::Instance.Find(pEnhancer->Type)->PowerPlantEnhancer_AffectTypes)
		{
			if (pType && pType->WhatAmI() != AbstractType::BuildingType)
			{
				if (std::ranges::find(enhancedTypes, pType) == enhancedTypes.end())
					enhancedTypes.push_back(pType);
			}
		}
	}

	auto addPower = [&output, &drain](std::int64_t power, int count)
		{
			if (power > 0)
				output += power * count;
			else
				drain -= power * count;
		};

	struct CountedPower
	{
		int Remaining;
		int Power;
		int AIPower;
		bool EnhanceAI;
	};

	auto updatePower = [&](const auto& types, const auto& instances)
		{
			std::unordered_map<const TechnoTypeClass*, CountedPower> counted;
			for (const auto pType : types)
			{
				const int count = pThis->CountOwnedAndPresent(pType);
	
				if (count == 0)
					continue;
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

				const int power = pExt->Power.Get(0);
				const int aiPower = pExt->AI_Power.Get(power);
				addPower(power, count);
				aiDelta += (static_cast<std::int64_t>(aiPower) - power) * count;
				aiDrainDelta += (MaxImpl(-static_cast<std::int64_t>(aiPower), 0LL)
					- MaxImpl(-static_cast<std::int64_t>(power), 0LL)) * count;
				aiAdjusted |= aiPower != power;
				if (std::ranges::find(enhancedTypes, pType) != enhancedTypes.end())
					counted.emplace(pType, CountedPower { count, power, aiPower,
						pExt->AI_Power_AffectedByEnhancers.Get(affectedByEnhancers) });
			}
			if (counted.empty())
				return;

			// The counters remain authoritative, including entries with no matching
			// live instance. Replace at most one counted contribution per instance.
			for (const auto pUnit : instances)
			{
				if (!pUnit || !pUnit->Type || pUnit->Owner != pThis || !pUnit->IsAlive || pUnit->Health <= 0 || pUnit->InLimbo)
					continue;
				if constexpr (std::is_same_v<std::remove_cvref_t<decltype(*pUnit)>, InfantryClass>)
				{
					// Native infantry gain does not add either category to its active
					// counter. They must not consume another infantry's counted slot.
					if (pUnit->Technician || pUnit->Type->DontScore)
						continue;
				}

				const auto it = counted.find(pUnit->Type);
				if (it == counted.end() || it->second.Remaining <= 0)
					continue;

				auto& countedPower = it->second;
				--countedPower.Remaining;
				const auto [realOutput, realDrain] = HouseExtData::GetUnitPower(pUnit->Type,
					static_cast<std::int64_t>(countedPower.Power), pThis, pUnit);
				const auto realPower = realOutput - realDrain;
				std::int64_t aiOutput = MaxImpl(static_cast<std::int64_t>(countedPower.AIPower), 0LL);
				std::int64_t aiDrain = MaxImpl(-static_cast<std::int64_t>(countedPower.AIPower), 0LL);
				if (countedPower.EnhanceAI)
				{
					if (countedPower.AIPower == countedPower.Power)
					{
						aiOutput = realOutput;
						aiDrain = realDrain;
					}
					else
					{
						const auto enhanced = HouseExtData::GetUnitPower(pUnit->Type,
							static_cast<std::int64_t>(countedPower.AIPower), pThis, pUnit);
						aiOutput = enhanced.first;
						aiDrain = enhanced.second;
					}
				}
				const auto aiPower = aiOutput - aiDrain;
				// Replace the same counted slot on both grids; unmatched counter
				// entries retain their respective unenhanced base contributions.
				aiDelta += aiPower - realPower
					- (static_cast<std::int64_t>(countedPower.AIPower) - countedPower.Power);
				aiDrainDelta += aiDrain - realDrain
					- (MaxImpl(-static_cast<std::int64_t>(countedPower.AIPower), 0LL)
						- MaxImpl(-static_cast<std::int64_t>(countedPower.Power), 0LL));
				aiAdjusted |= aiPower != realPower || aiDrain != realDrain;
				addPower(countedPower.Power, -1);
				output += realOutput;
				drain += realDrain;
			}
		};

	updatePower(*InfantryTypeClass::Array, *InfantryClass::Array);
	updatePower(*UnitTypeClass::Array, *UnitClass::Array);
	updatePower(*AircraftTypeClass::Array, *AircraftClass::Array);
	// Buildings already contributed through native UpdatePower.
	pThis->PowerOutput = static_cast<int>(std::clamp(output,
		static_cast<std::int64_t>(std::numeric_limits<int>::min()), static_cast<std::int64_t>(std::numeric_limits<int>::max())));
	pThis->PowerDrain = static_cast<int>(std::clamp(drain,
		static_cast<std::int64_t>(std::numeric_limits<int>::min()), static_cast<std::int64_t>(std::numeric_limits<int>::max())));
	// Retain the raw-to-cache correction even when only a building overrides
	// its AI contribution. The actual grid stores saturated ints.
	return { aiDelta, aiDrainDelta,
		output - drain - (static_cast<std::int64_t>(pThis->PowerOutput) - pThis->PowerDrain),
		drain - pThis->PowerDrain, aiAdjusted };
}

// Keep the AI adjustment in the same snapshot as the real power totals. Reading
// live buildings during a later decision could mix new state with an old cache.
std::int64_t HouseExtData::UpdateAIPower(HouseClass* const pThis, const PowerAdjustment& unitPower)
{
	std::int64_t delta = unitPower.Net;
	std::int64_t aiDrain = static_cast<std::int64_t>(pThis->PowerDrain) + unitPower.Drain;
	bool adjusted = unitPower.Adjusted;
	const bool affectedByEnhancers = FakeRulesClass::Instance->AI_Power_AffectedByEnhancers;
	const bool blackout = pThis->PowerBlackoutTimer.GetTimeLeft() != 0;
	const bool outputDisabled = blackout || pThis->IsBeingDrained;
	const bool capturedOutput = powerOutputContext == pThis;
	const auto realOutputBeforeClamp = powerOutputBeforeDrainClamp;
	powerOutputContext = nullptr;
	std::int64_t buildingOutputDelta = 0;
	std::int64_t localDrainDelta = 0;
	bool aiFullDrain = false;

	for (auto const pBuilding : pThis->Buildings)
	{
		if (!pBuilding || pBuilding->Owner != pThis || pBuilding->InLimbo || !pBuilding->IsOnMap)
		{
			continue;
		}

		std::int64_t drainDelta;
		bool buildingAdjusted;
		const auto buildingDelta = HouseExtData::GetAIPowerDelta(pBuilding, affectedByEnhancers,
			capturedOutput ? false : outputDisabled, drainDelta, buildingAdjusted);
		delta += buildingDelta;
		aiDrain += drainDelta;
		adjusted |= buildingAdjusted;
		buildingOutputDelta += buildingDelta + drainDelta;

		if (capturedOutput && pBuilding->IsBeingDrained())
		{
			const auto pDrainer = pBuilding->DrainingMe;
			if (!pDrainer)
				continue;
			const auto pBuildingExt = TechnoTypeExtContainer::Instance.Find(pBuilding->Type);
			const auto pDrainerExt = TechnoTypeExtContainer::Instance.Find(pDrainer->GetTechnoType());
			const std::int64_t realOutput = pBuilding->GetPowerOutput();
			std::int64_t buildingAIDrain;
			const auto aiNet = HouseExtData::GetAIPower(pBuilding, affectedByEnhancers, false, buildingAIDrain);
			const auto aiOutput = aiNet + buildingAIDrain;
			if (pBuildingExt->Drain_Local || pDrainerExt->Drain_Local)
			{
				auto localDrain = [&](std::int64_t output)
					{
						if (output <= 0)
							return 0LL;
						for (const std::int64_t amount : { pDrainerExt->Drain_Amount.Get(), pBuildingExt->Drain_Amount.Get() })
						{
							if (amount > 0)
								output = MinImpl(output, amount);
							else if (amount != std::numeric_limits<int>::min())
								output = MaxImpl(output, -amount);
							// Ares negates its signed int before comparison; INT_MIN
							// stays negative and cannot raise a positive drain amount.
						}
						return output;
					};
				localDrainDelta += localDrain(realOutput) - localDrain(aiOutput);
			}
			else
				aiFullDrain |= aiOutput > 0;
		}
	}

	if (capturedOutput)
	{
		const auto aiOutput = blackout || aiFullDrain ? 0LL : MaxImpl(0LL,
			static_cast<std::int64_t>(realOutputBeforeClamp) + buildingOutputDelta + localDrainDelta);
		// Clamp both output lines independently; actual overdrain was discarded
		// by Ares before this hook. Unit power is added after that native step.
		const auto realOutput = outputDisabled ? 0LL : MaxImpl(0LL, static_cast<std::int64_t>(realOutputBeforeClamp));
		delta += aiOutput - realOutput - buildingOutputDelta;
	}

	if (adjusted)
	{
		delta += unitPower.OverflowNet;
		aiDrain += unitPower.OverflowDrain;
	}

	HouseExtContainer::Instance.Find(pThis)->AIPowerDelta = delta;
	return aiDrain;
}

ASMJIT_PATCH(0x4FE713, HouseClass_AI_BaseConstructionUpdate_AIPower, 0x6)
{
	enum { Produce = 0x4FE953, CheckExemptions = 0x4FE723 };

	GET(HouseClass* const, pThis, EBP);
	GET(BuildingTypeClass* const, pType, EDI);

	auto const& power = BuildingTypeExtContainer::Instance.Find(pType)->AI_Power;
	auto const delta = HouseExtContainer::Instance.Find(pThis)->AIPowerDelta;
	if (!power.isset() && delta == 0)
		return 0;

	const auto nodeDrain = MaxImpl(-HouseExtData::GetAIPower(pType), 0LL);
	const auto available = static_cast<std::int64_t>(pThis->PowerOutput) - pThis->PowerDrain + delta;
	const auto required = nodeDrain + pThis->PowerSurplus;

	// The following exemption chain only tests the sign of ESI. Keep even the
	// magnitude of -INT_MIN safe while comparing the full value above in 64 bits.
	R->ESI(static_cast<int>(MinImpl(nodeDrain, static_cast<std::int64_t>(std::numeric_limits<int>::max()))));
	return available >= required ? Produce : CheckExemptions;
}

// AITakeover arranges the initial base plan by its power budget. Recompute only
// the prefix already visited by the engine, never credit a later planned plant.
static bool GetAIPowerPrefix(HouseClass* const pThis, int nodeIndex, std::int64_t& balance)
{
	auto const& nodes = pThis->Base.BaseNodes;
	auto const& types = BuildingTypeClass::Array;
	bool overridden = false;
	balance = 0;

	for (int i = 0; i <= nodeIndex && i < nodes.Count; ++i)
	{
		const int index = nodes.Items[i].BuildingTypeIndex;
		if (index < 0 || index >= types->Count)
			continue;

		auto const pType = types->Items[index];
		if (!pType)
			continue;

		auto const& power = BuildingTypeExtContainer::Instance.Find(pType)->AI_Power;
		balance += HouseExtData::GetAIPower(pType);
		overridden |= power.isset();
	}

	return overridden;
}

ASMJIT_PATCH(0x50ACAF, HouseClass_AITakeover_AIPower, 0x6)
{
	enum
	{
		ContinueFirst = 0x50ACBB, InsertFirst = 0x50ACDC,
		ContinueSecond = 0x50AEC9, InsertSecond = 0x50AED9
	};

	GET(HouseClass* const, pThis, EBP);
	const bool first = R->Origin() == 0x50ACAF;
	const int nodeIndex = first ? R->EBX<int>() : R->ECX<int>();
	std::int64_t balance;
	if (!GetAIPowerPrefix(pThis, nodeIndex, balance))
		return 0;

	const bool sufficient = balance >= pThis->PowerSurplus;
	return first ? (sufficient ? ContinueFirst : InsertFirst) : (sufficient ? ContinueSecond : InsertSecond);
}ASMJIT_PATCH_AGAIN(0x50AEBD, HouseClass_AITakeover_AIPower, 0x6)

void CalculatePowerSurplus(HouseClass* pThis, std::int64_t aiPowerDrain)
{
	auto const pRulesExt = FakeRulesClass::Instance();

	if (!pRulesExt->EnablePowerSurplus)
		return;

	int scaleToDrainAmount = pRulesExt->PowerSurplus_ScaleToDrainAmount;

	if (scaleToDrainAmount <= 0)
	{
		pThis->PowerSurplus = RulesClass::Instance->PowerSurplus;
	}
	else
	{
		double factor = aiPowerDrain / static_cast<double>(scaleToDrainAmount);
		const auto surplus = RulesClass::Instance->PowerSurplus * factor;
		pThis->PowerSurplus = static_cast<int>(std::clamp(surplus,
			static_cast<double>(std::numeric_limits<int>::min()), static_cast<double>(std::numeric_limits<int>::max())));
	}
}

void CalculatePowerSurplus(HouseClass* pThis)
{
	CalculatePowerSurplus(pThis, pThis->PowerDrain);
}

ASMJIT_PATCH(0x4F84D9, HouseClass_AI_RecheckUnitPower, 0x6)
{
	GET(HouseClass*, pThis, ESI);

	PowerPlantEnhancerClass::RecheckPower(pThis);
	return 0;
}

#pragma region UpdatePower Hooks 

//ASMJIT_PATCH(0x508D8D, HouseClass_UpdatePower_Techno, 0x6)
//{
//	GET(HouseClass*, pThis, ESI);
//
//	const auto unitPower = HouseExtData::UpdateUnitPower(pThis);
//	const auto aiPowerDrain = HouseExtData::UpdateAIPower(pThis, unitPower);
//	CalculatePowerSurplus(pThis, aiPowerDrain);
//
//	return 0x0;
//}

// drain affecting only the drained power plant
//ASMJIT_PATCH(0x508D32, HouseClass_UpdatePower_LocalDrain1, 5)
//{
//	GET(HouseClass*, pThis, ESI);
//	GET(BuildingClass*, pBld, EDI);
//
//	bool fullDrain = true;
//
//	auto output = pBld->GetPowerOutput();
//
//	if (output > 0)
//	{
//		auto pBldTypeExt = TechnoTypeExtContainer::Instance.Find(pBld->Type);
//		auto pDrainTypeExt = GET_TECHNOTYPEEXT(pBld->DrainingMe);
//
//		// local, if any of the participants in the drain is local
//		if (pBldTypeExt->Drain_Local || pDrainTypeExt->Drain_Local)
//		{
//			fullDrain = false;
//
//			// use the sign to select min or max.
//			// 0 means no change (maximum of 0 and a positive value)
//			COMPILETIMEEVAL auto limit = [](int value, int limit)
//				{
//					if (limit <= 0)
//					{
//						return MaxImpl(value, -limit);
//					}
//					else
//					{
//						return MinImpl(value, limit);
//					}
//				};
//
//			// drains the entire output of this building by default
//			// (the local output). building has the last word though.
//			auto drain = limit(output, pDrainTypeExt->Drain_Amount);
//			drain = limit(drain, pBldTypeExt->Drain_Amount);
//
//			if (drain > 0)
//			{
//				pThis->PowerOutput -= drain;
//			}
//		}
//	}
//
//	return fullDrain ? 0 : 0x508D37;
//}

// sanitize the power output
//ASMJIT_PATCH(0x508D4A, HouseClass_UpdatePower_LocalDrain2, 6)
//{
//	GET(HouseClass*, pThis, ESI);
//	if (pThis->PowerOutput < 0)
//	{
//		pThis->PowerOutput = 0;
//	}
//	return 0;
//}

//ASMJIT_PATCH(0x508CE6, HouseClass_UpdatePower_LimboDeliver, 0x6)
//{
//	GET(BuildingClass*, pBld, EDI);
//
//	if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
//		return 0x508CEE; // add the power
//
//	return 0x0;
//}

//ASMJIT_PATCH(0x508C7F, HouseClass_UpdatePower_Auxiliary, 6)
//{
//	GET(HouseClass*, pThis, ESI);
//
//	const auto curAux = HouseExtContainer::Instance.Find(pThis)->AuxPower;
//
//	int AuxPowerOutput = 0;
//	if (curAux >= 0)
//		AuxPowerOutput = curAux;
//	pThis->PowerOutput = AuxPowerOutput;
//
//	int AuxPowerDrain = 0;
//	if (curAux <= 0)
//		AuxPowerDrain = -curAux;
//	pThis->PowerDrain = AuxPowerDrain;
//
//	return 0x508C8B;
//}

//ASMJIT_PATCH(0x508D45, HouseClass_UpdatePower_BeforeDrainClamp, 0x5)
//{
//	GET(HouseClass*, pThis, ESI);
//	HouseExtData::RecordPowerOutputBeforeDrainClamp(pThis);
//	return 0;
//}

#pragma endregion


void FakeHouseClass::_RecalcPower()
{
	auto GetCurrentPowerState = [this]()
		{
			int _currentPowerB = this->PowerOutput;
			int _currentDrainB = this->PowerDrain;
			return _currentPowerB < _currentDrainB && _currentDrainB && (!_currentPowerB
				|| (double)this->PowerOutput / (double)this->PowerDrain < 1.0);
		};

	const bool lastPowerState = GetCurrentPowerState();
	this->RecheckPower = false;

	// 0x508C7F, HouseClass_UpdatePower_Auxiliary, 6
	{
		const auto curAux = this->_GetExtData()->AuxPower;

		int AuxPowerOutput = 0;
		if (curAux >= 0)
			AuxPowerOutput = curAux;
		this->PowerOutput = AuxPowerOutput;

		int AuxPowerDrain = 0;
		if (curAux <= 0)
			AuxPowerDrain = -curAux;
		this->PowerDrain = AuxPowerDrain;
	}
	//

	bool has_power = false;

	//0x508D32, HouseClass_UpdatePower_LocalDrain1, 5
	auto UpdatePower_LocalDrain = [this](BuildingClass* pBld)
		{
			bool fullDrain = true;

			auto output = pBld->GetPowerOutput();

			if (output > 0)
			{
				auto pBldTypeExt = TechnoTypeExtContainer::Instance.Find(pBld->Type);
				auto pDrainTypeExt = GET_TECHNOTYPEEXT(pBld->DrainingMe);

				// local, if any of the participants in the drain is local
				if (pBldTypeExt->Drain_Local || pDrainTypeExt->Drain_Local)
				{
					fullDrain = false;

					// use the sign to select min or max.
					// 0 means no change (maximum of 0 and a positive value)
					COMPILETIMEEVAL auto limit = [](int value, int limit)
						{
							if (limit <= 0)
							{
								return MaxImpl(value, -limit);
							}
							else
							{
								return MinImpl(value, limit);
							}
						};

					// drains the entire output of this building by default
					// (the local output). building has the last word though.
					auto drain = limit(output, pDrainTypeExt->Drain_Amount);
					drain = limit(drain, pBldTypeExt->Drain_Amount);

					if (drain > 0)
					{
						this->PowerOutput -= drain;
					}
				}
			}

			return fullDrain;
		};
	//

	for (auto& pBld : this->Buildings)
	{
		if (pBld && !pBld->InLimbo && pBld->IsOnMap)
		{
			if (SessionClass::Instance->GameMode != GameMode::Campaign)
			{
				if (this != HouseClass::CurrentPlayer()
					|| !this->IsHumanPlayer && !this->IsInPlayerControl
					|| pBld->DiscoveredByCurrentPlayer
					|| SessionClass::Instance->GameMode != GameMode::Campaign
					// i forgot what it for but i suppose to fix DiscoveredByCurrentPlayer part
					// it was long time ago 
					// 0x508CE6, HouseClass_UpdatePower_LimboDeliver, 0x6
					|| BuildingExtContainer::Instance.Find(pBld)->LimboID != -1
					//
					)
				{

					this->PowerOutput += pBld->GetPowerOutput();
					this->PowerDrain += pBld->GetPowerDrain();

					if (pBld->IsBeingDrained() && pBld->GetPowerOutput() > 0)
					{
						if (UpdatePower_LocalDrain(pBld))
							has_power = 1;
					}
				}
			}
		}
	}

	//0x508D45, HouseClass_UpdatePower_BeforeDrainClamp, 0x5
	HouseExtData::RecordPowerOutputBeforeDrainClamp(this);
	//

	//0x508D4A, HouseClass_UpdatePower_LocalDrain2, 6
	if (this->PowerOutput < 0)
	{
		this->PowerOutput = 0;
	}
	//

	this->IsBeingDrained = has_power;
	if (!this->PowerBlackoutTimer.GetTimeLeft())
	{
		if (has_power)
		{
			this->PowerOutput = 0;
		}
	}

	Recalc_All_Factories_Of_House(this);

	//0x508D8D, HouseClass_UpdatePower_Techno, 0x6
	{
		const auto unitPower = HouseExtData::UpdateUnitPower(this);
		const auto aiPowerDrain = HouseExtData::UpdateAIPower(this, unitPower);
		CalculatePowerSurplus(this, aiPowerDrain);
	}
	//

	if (lastPowerState != GetCurrentPowerState())
		FakeHouseClass::__SuperWeaponHandler(this);

	this->RecheckRadar = true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x508C30, FakeHouseClass::_RecalcPower);
DEFINE_FUNCTION_JUMP(CALL, 0x4F84E5, FakeHouseClass::_RecalcPower);