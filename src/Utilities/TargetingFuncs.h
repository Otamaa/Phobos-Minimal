#pragma once

#include <Utilities/Iterator.h>
#include <Utilities/Enum.h>
#include <Utilities/TargetResult.h>

#include <DiscreteSelectionClass.h>
#include <DiscreteDistributionClass.h>
#include <ScenarioClass.h>

class TechnoClass;
struct TargetingData;
class SWTypeHandler;
class TechnoTypeExtData;
class FootClass;
struct TargetingFuncs
{
#pragma region Helpers

	template<typename Valuator>
	static COMPILETIMEEVAL TechnoClass* GetTargetFirstMax(Iterator<TechnoClass*> iter, Valuator value)
	{
		TechnoClass* pTarget = nullptr;
		int maxValue = 0;

		for (auto item : iter)
		{
			auto curVal = value(item, maxValue);
			if (curVal > maxValue)
			{
				pTarget = item;
				maxValue = curVal;
			}
		}

		return pTarget;
	}

	template<typename Valuator>
	static TechnoClass* GetTargetAnyMax(Iterator<TechnoClass*> iter, Valuator value)
	{
		if (!iter) { return nullptr; }

		DiscreteSelectionClass<TechnoClass*> targets {};

		for (auto item : iter)
		{
			int val = value(item, targets.rating());
			targets.add(item, val);
		}

		return targets.select_or(ScenarioClass::Instance->Random, nullptr);
	}

	template<typename Valuator>
	static TechnoClass* GetTargetShareAny(Iterator<TechnoClass*> iter, Valuator value)
	{
		if (!iter) { return nullptr; }

		DiscreteDistribution<TechnoClass*> targets {};

		for (auto item : iter)
		{
			int val = value(item);
			targets.add(item, val);
		}

		return targets.select_or(ScenarioClass::Instance->Random, nullptr);
	}

	static bool IsTargetAllowed(TechnoClass* pTechno);

	static bool IgnoreThis(TechnoClass* pTechno);

	static COMPILETIMEEVAL TargetResult NoTarget()
	{
		return { CellStruct::Empty , SWTargetFlags::AllowEmpty };
	}

#pragma endregion

#pragma region MainTargeting

	static int GetIonCannonValue(TechnoClass* pTechno, TechnoTypeExtData* pTypeExt, const TargetingData* pTargeting);
	static TargetResult GetIonCannonTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting, HouseClass* pEnemy, CloakHandling cloak);
	static TargetResult PickByHouseType(HouseClass* pThis, QuarryType type);
	static bool IsEligibleDominatorTarget(const TargetingData* pTargeting, FootClass* pTarget);
	static TargetResult GetDominatorTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetParadropTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetMutatorTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetForceShieldTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetOffensiveTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetNukeAndLighningTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetStealthTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetDroppodTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetLighningRandomTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetOwnerBuildingAsTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting, bool checkLauchsite = false);
	static TargetResult GetBaseTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetMultiMissileTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetEnemyBaseTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);
	static TargetResult GetAuxTechnoTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting);

#pragma endregion

};
