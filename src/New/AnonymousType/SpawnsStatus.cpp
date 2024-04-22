#include "SpawnsStatus.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>

#include <TechnoClass.h>

void SpawnsStatus::OnUpdate(AnimClass* pLinked)
{
	const auto pData = AnimTypeExtContainer::Instance.Find(pLinked->Type);

	if (pData->SpawnsData.Enable && pData->SpawnsData.TriggerOnStart && pData->SpawnsData.Count != 0)
	{
		if (!_initSpawnFlag)
		{
			ResetLoopSpawn();
			_initSpawnFlag = true;
			int _initDelay = 0;
			if ((_initDelay = pData->SpawnsData.GetInitDelay()) > 0)
			{
				_spawnInitDelayTimer.Start(_initDelay);
			}
			else
			{
				_spawnInitDelayTimer.Stop();
			}
		}

		if ((pData->SpawnsData.Count < 0 || _spawnCount < pData->SpawnsData.Count) && _spawnInitDelayTimer.Expired() && _spawnDelayTimer.Expired())
		{
			const auto pObject = AnimExtData::GetTechnoInvoker(pLinked);
			const auto pHouse = !pLinked->Owner && pObject ? pObject->Owner : pLinked->Owner;
			auto nCoord = pLinked->GetCoords();

			pData->SpawnsData.SpawnAnims(pObject, pHouse, nCoord);
			_spawnCount++;
			int delay = 0;
			if ((delay = pData->SpawnsData.GetDelay()) > 0)
			{
				_spawnDelayTimer.Start(delay);
			}
		}
	}
	else
	{
		ResetLoopSpawn();
	}
}

void SpawnsStatus::OnLoop(AnimClass* pLinked)
{
	const auto pData = AnimTypeExtContainer::Instance.Find(pLinked->Type);
	if (pData->SpawnsData.Enable && pData->SpawnsData.TriggerOnLoop)
	{
		const auto pObject = AnimExtData::GetTechnoInvoker(pLinked);
		const auto pHouse = !pLinked->Owner && pObject ? pObject->Owner : pLinked->Owner;
		auto nCoord = pLinked->GetCoords();

		pData->SpawnsData.SpawnAnims(pObject, pHouse, nCoord);
	}
}

void SpawnsStatus::OnDone(AnimClass* pLinked)
{
	const auto pData = AnimTypeExtContainer::Instance.Find(pLinked->Type);
	if (pData->SpawnsData.Enable && pData->SpawnsData.TriggerOnDone)
	{
		const auto pObject = AnimExtData::GetTechnoInvoker(pLinked);
		const auto pHouse = !pLinked->Owner && pObject ? pObject->Owner : pLinked->Owner;
		auto nCoord = pLinked->GetCoords();

		pData->SpawnsData.SpawnAnims(pObject, pHouse, nCoord);
	}
}

void SpawnsStatus::OnNext(AnimClass* pLinked, AnimTypeClass* pNext)
{
	const auto pData = AnimTypeExtContainer::Instance.Find(pLinked->Type);
	ResetLoopSpawn();
	if (pData->SpawnsData.Enable && pData->SpawnsData.TriggerOnNext)
	{
		const auto pObject = AnimExtData::GetTechnoInvoker(pLinked);
		const auto pHouse = !pLinked->Owner && pObject ? pObject->Owner : pLinked->Owner;
		auto nCoord = pLinked->GetCoords();

		pData->SpawnsData.SpawnAnims(pObject, pHouse, nCoord);
	}
}