#include "AttackBeaconFunctional.h"
#include <Helpers/IteratorIdx.h>
#include <Misc/Otamaa/Misc/DynamicPatcher/Helpers/Helpers.h>

void AttackBeaconFunctional::AI(TechnoExt::ExtData* pExt , TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pExt || pTypeExt)
		return;

	const auto& AnotherExtData = pExt->AnotherData;
	const auto& AnotherTypeExtData = pTypeExt->AnotherData;
	const auto& BeaconData = AnotherExtData.MyBeacon;
	const auto& BeaconTypeData = AnotherTypeExtData.MyAttackBeaconData;

	std::map<TechnoTypeClass*, 
	std::map<double, 
	std::vector<TechnoClass*>>> 
		candidates;

	auto const TechnoArr = TechnoClass::Array();
	auto pHouse = pExt->Get()->Owner;
	CoordStruct location = pExt->Get()->GetCoords();
	std::map<double, std::vector<TechnoClass*>> nTargetDummy;

	for (int i = TechnoArr->Count - 1; i > 0; i--)
	{ 
		auto const pTarget = TechnoArr->GetItem((int)i);
		if (Helpers_DP::IsDeadOrInvisible(pTarget)
			|| !pTarget->Owner
			|| (pTarget->Owner == pHouse ? !BeaconTypeData.AffectsOwner :
				(pTarget->Owner->IsAlliedWith_(pHouse) ? !BeaconTypeData.AffectsAllies : !BeaconTypeData.AffectsEnemies)))
		{ continue; }

		auto const pTargetType = pTarget->GetTechnoType();
		
		if ((BeaconTypeData.Types.size() <= 0 || BeaconTypeData.Types.Contains(pTargetType))
			&& BeaconTypeData.Force ? true : BeaconData.EligibleMissions.Contains(pTarget->GetCurrentMission()))
		{
			double nDistance = location.DistanceFrom(pTarget->GetCoords());
			if (nDistance > BeaconTypeData.RangeMin && (BeaconTypeData.RangeMax < 0.0 ? true : nDistance < BeaconTypeData.RangeMax))
			{
				if (BeaconTypeData.Force || !pTarget->Target|| pTarget->Target != pExt->Get())
				{ 
					if (candidates.contains(pTargetType))
					{
						nTargetDummy = candidates[pTargetType];
					}
					else
					{
						nTargetDummy.clear();
						candidates.insert({ pTargetType, nTargetDummy });
					}

					if (nTargetDummy.contains(nDistance))
					{
						nTargetDummy[nDistance].push_back(pTarget);
					}
					else
					{
						nTargetDummy.insert({ nDistance , {pTarget} });
					}

					candidates[pTargetType] = nTargetDummy;
				}
			}
		}
	}

	AbstractClass* pBeacon = pExt->Get();
	if (BeaconTypeData.TargetToCell) {
		pBeacon = pExt->Get()->GetCell();
	}

	bool noLimit = BeaconTypeData.Types.empty() || BeaconTypeData.Types.size() <= 0;

	for(auto const& candidate : candidates)
	{
		auto const type = candidate.first;
		auto technos = candidate.second;
		// check this type is full.
		int count = 0;
		bool isFull = false;
		for(auto const& targets : technos)
		{
			if (isFull)
			{
				break;
			}

			for(auto const& pTarget : targets.second)
			{
				if (!noLimit && ++count > BeaconTypeData.Num[BeaconTypeData.Types.IndexOf(type)])
				{
					isFull = true;
					break;
				}

				// recruit one
				pTarget->SetTarget(pBeacon);
				if (auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget))
				{
					pTargetExt->AnotherData.Recuited = true;
				}			
			}
		}
	}
}

void AttackBeaconFunctional::OnFire(TechnoExt::ExtData* pExt, AbstractClass* pTarget, int nWeapon)
{
	if (pExt->AnotherData.Recuited)
	{
		pExt->AnotherData.Recuited = false;
		pExt->Get()->SetTarget(nullptr);
	}
}