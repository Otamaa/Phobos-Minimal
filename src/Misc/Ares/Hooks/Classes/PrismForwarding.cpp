#include "PrismForwarding.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <ExtraHeaders/StackVector.h>

int PrismForwarding::AcquireSlaves_SingleStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain)
{
	//set up immediate slaves for this particular tower

	auto const MaxFeeds = TargetTower->GetOwnerData()->GetMaxFeeds();
	auto const MaxNetworkSize = this->GetOwnerData()->GetMaxNetworkSize();
	auto const MaxChainLength = this->GetOwnerData()->MaxChainLength;

	if (MaxFeeds == 0
		|| (MaxChainLength != -1 && MaxChainLength < chain)
		|| (MaxNetworkSize != -1 && MaxNetworkSize <= NetworkSize))
	{
		return 0;
	}

	struct PrismTargetData
	{
		PrismForwarding* Tower;
		int Distance;

		COMPILETIMEEVAL bool operator < (const PrismTargetData& rhs) const
		{
			return this->Distance < rhs.Distance;
		}
	};

	CoordStruct MyPosition, curPosition;
	TargetTower->Owner->GetRenderCoords(&MyPosition);

	//first, find eligible towers
	StackVector<PrismTargetData , 256> EligibleTowers;

	for (auto const SlaveTower : *BuildingClass::Array)
	{
		auto const pSlaveData = BuildingExtContainer::Instance.Find(SlaveTower);
		if (this->ValidateSupportTower(TargetTower, &pSlaveData->PrismForwarding))
		{
			SlaveTower->GetRenderCoords(&curPosition);
			int Distance = static_cast<int>(MyPosition.DistanceFrom(curPosition));
			PrismTargetData pd = { &pSlaveData->PrismForwarding, Distance };
			EligibleTowers->push_back(pd);
		}
	}

	std::stable_sort(EligibleTowers->begin(), EligibleTowers->end());

	//now enslave the towers in order of proximity
	auto iFeeds = 0;
	for (const auto& eligible : EligibleTowers.container())
	{
		// feed limit enabled and reached
		if (MaxFeeds != -1 && iFeeds >= MaxFeeds)
		{
			break;
		}

		// network size limit enabled and reached
		if (MaxNetworkSize != -1 && NetworkSize >= MaxNetworkSize)
		{
			break;
		}

		//we have a slave tower! do the bizzo
		++iFeeds;
		++NetworkSize;

		CoordStruct FLH;
		TargetTower->GetOwner()->GetFLH(&FLH, 0, CoordStruct::Empty);
		eligible.Tower->Owner->DelayBeforeFiring = eligible.Tower->Owner->Type->DelayedFireDelay;
		eligible.Tower->Owner->PrismStage = PrismChargeState::Slave;
		eligible.Tower->Owner->PrismTargetCoords = FLH;

		eligible.Tower->SetSupportTarget(TargetTower);
	}

	if (iFeeds != 0 && chain > LongestChain)
	{
		++LongestChain;
	}

	return iFeeds;
}

bool PrismForwarding::ValidateSupportTower(PrismForwarding* pTargetTower, PrismForwarding* pSlaveTower)
{
	//MasterTower = the firing tower. This might be the same as TargetTower, it might not.
	//TargetTower = the tower that we are forwarding to
	//SlaveTower = the tower being considered to support TargetTower
	auto const TargetTower = pTargetTower->GetOwner();
	auto const SlaveTower = pSlaveTower->GetOwner();

	if (SlaveTower->IsAlive)
	{
		auto const pSlaveType = SlaveTower->Type;
		auto const pSlaveTypeData = BuildingTypeExtContainer::Instance.Find(pSlaveType);
		if (pSlaveTypeData->PrismForwarding.CanForward())
		{
			//building is a prism tower
			//get all the data we need
			auto const pTechnoData = TechnoExtContainer::Instance.Find(SlaveTower);
			//BuildingExt::ExtData *pSlaveData = BuildingExtContainer::Instance.Find(SlaveTower);
			auto const SlaveMission = SlaveTower->GetCurrentMission();
			//now check all the rules
			if (SlaveTower->ReloadTimer.Expired()
				&& SlaveTower != TargetTower
				&& !SlaveTower->DelayBeforeFiring
				&& !SlaveTower->IsBeingDrained()
				&& !SlaveTower->IsBeingWarpedOut()
				&& SlaveMission != Mission::Attack
				&& SlaveMission != Mission::Construction
				&& SlaveMission != Mission::Selling
				&& TechnoExt_ExtData::IsPowered(SlaveTower) //robot control logic
				&& TechnoExt_ExtData::IsOperatedB(SlaveTower) //operator logic
				&& SlaveTower->IsPowerOnline() //base-powered or overpowerer-powered
				&& !SlaveTower->IsUnderEMP()) //EMP logic - I think this should already be checked by IsPowerOnline() but included just to be sure
			{
				auto const pTargetType = TargetTower->Type;
				if (pSlaveTypeData->PrismForwarding.Targets.Contains(pTargetType))
				{
					//valid type to forward from
					const auto pMasterHouse = this->GetOwner()->Owner;
					const auto pTargetHouse = TargetTower->Owner;
					const auto pSlaveHouse = SlaveTower->Owner;
					if ((pSlaveHouse == pTargetHouse && pSlaveHouse == pMasterHouse)
						|| (pSlaveTypeData->PrismForwarding.ToAllies
							&& pSlaveHouse->IsAlliedWith(pTargetHouse)
							&& pSlaveHouse->IsAlliedWith(pMasterHouse)))
					{
						//ownership/alliance rules satisfied
						CoordStruct MyPosition, curPosition;
						TargetTower->GetRenderCoords(&MyPosition);
						SlaveTower->GetRenderCoords(&curPosition);
						auto const Distance = static_cast<int>(MyPosition.DistanceFrom(curPosition));
						int SupportRange = 0;

						const bool IsElite = SlaveTower->Veterancy.IsElite();
						const int idxSupport = IsElite ?
							pSlaveTypeData->PrismForwarding.EliteSupportWeaponIndex : pSlaveTypeData->PrismForwarding.SupportWeaponIndex;

						if (idxSupport != -1)
						{
							const auto weapon_s = IsElite ?
								pSlaveType->GetEliteWeapon(idxSupport) :
								pSlaveType->GetWeapon(idxSupport);

							if (auto const supportWeapon = weapon_s->WeaponType)
							{
								if (Distance < supportWeapon->MinimumRange)
								{
									return false; //below minimum range
								}
								SupportRange = supportWeapon->Range;
							}
						}
						if (SupportRange == 0)
						{
							//not specified on SupportWeapon so use Primary + 1 cell (Marshall chose to add the +1 cell default - see manual for reason)
							if (auto const cPrimary = pSlaveType->GetWeapon(0)->WeaponType)
							{
								SupportRange = cPrimary->Range + 256; //256 leptons == 1 cell
							}
						}
						if (SupportRange < 0 || Distance <= SupportRange)
						{
							return true; //within range
						}
					}
				}
			}
		}
	}
	return false;
}
