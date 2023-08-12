#include "RandomNumberChecker.h"

#include <Ext/House/Body.h>

void RandCheck::Exec() {
	auto const pCurPlayer = HouseClass::CurrentPlayer();

	if (!pCurPlayer || !HouseClass::Array->Count)
		return;

	const auto pCurHouseExt = HouseExt::ExtMap.Find(pCurPlayer);

	if (!pCurHouseExt)
		return;

	if (pCurPlayer->IsControlledByCurrentPlayer())
		pCurHouseExt->RandomNumber = ScenarioClass::Instance->Random.Random();

	std::map<DWORD, int> nCache;

	for (auto const pHouse : *HouseClass::Array)
	{
		if (!pHouse->IsControlledByCurrentPlayer())
			continue;

		if (const auto pExt = HouseExt::ExtMap.Find(pHouse)) {
			nCache[pExt->RandomNumber] = pHouse->ArrayIndex ;
		}
	}

	auto const nIter = nCache.find(pCurHouseExt->RandomNumber);

	if (nIter != nCache.end())
		nCache.erase(nIter);

	if (!nCache.empty()) {
		for (auto const [nRand, nHouseIdx] : nCache) {
			Debug::FatalError("House At [%d] With Different RandomNumber [%x] From [%d] Current Palyer [%x] \n", nHouseIdx, nRand, pCurPlayer->ArrayIndex, pCurHouseExt->RandomNumber);
		}
	}
}

DEFINE_HOOK_AGAIN(0x683E94, ScenarioClass_Start_Check_random, 0x5)
DEFINE_HOOK(0x683D4A, ScenarioClass_Start_Check_random, 0x5)
{
	RandCheck::Exec();
	return 0x0;
}