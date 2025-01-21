#include "PrismForwarding.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <ExtraHeaders/StackVector.h>

PrismForwardingData* PrismForwarding::GetOwnerData() const
{
	return BuildingTypeExtContainer::Instance.Find(this->Owner->Type)->PrismForwarding.AsPointer();
}

void PrismForwarding::InvalidatePointer(AbstractClass* ptr, bool bRemove)
{
	if (bRemove && this->SupportTarget && this->SupportTarget->Owner == ptr)
		this->SupportTarget = nullptr;
}

void PrismForwarding::RemoveAllSenders()
{
	// disconnect all sender towers from their support target, which is me
	for (auto sender : this->Senders)
	{
		sender->SetSupportTarget(nullptr);
	}

	// log if not all senders could be removed
	if (!this->Senders.empty())
	{
		Debug::Log("PrismForwarding::RemoveAllSenders: Tower (%p) still has %d senders after removal completed.\n",
			this->Owner, this->Owner);

		for (size_t i = 0; i < this->Senders.size(); ++i)
		{
			Debug::Log("Sender %03d: %p\n", i, this->Senders[i]->Owner);
		}

		this->Senders.clear();
	}
}

void PrismForwarding::SetChargeDelay_Set(int chain, DWORD const* LongestCDelay, DWORD const* LongestFDelay, int LongestChain)
{
	auto const pTargetTower = this->Owner;

	this->PrismChargeDelay = (LongestFDelay[chain] - pTargetTower->DelayBeforeFiring) + LongestCDelay[chain];
	pTargetTower->SupportingPrisms = (LongestChain - chain);

	if (this->PrismChargeDelay == 0)
	{
		//no delay, so start animations now
		if (pTargetTower->Type->GetBuildingAnim(BuildingAnimSlot::Special).Anim[0])
		{ //only if it actually has a special anim
			pTargetTower->DestroyNthAnim(BuildingAnimSlot::Active);
			pTargetTower->Game_PlayNthAnim(BuildingAnimSlot::Special, !pTargetTower->IsGreenHP(), pTargetTower->GetOccupantCount() > 0, 0);
		}
	}

	for (auto Sender : this->Senders)
	{
		Sender->SetChargeDelay_Set(chain + 1, LongestCDelay, LongestFDelay, LongestChain);
	}
}

void PrismForwarding::SetChargeDelay(int LongestChain)
{
	auto const ArrayLen = LongestChain + 1;
	std::vector<DWORD> LongestCDelay(ArrayLen, 0);
	std::vector<DWORD> LongestFDelay(ArrayLen, 0);

	for (auto endChain = LongestChain; endChain >= 0; --endChain)
	{
		this->SetChargeDelay_Get(0, endChain, LongestChain, LongestCDelay.data(), LongestFDelay.data());
	}

	this->SetChargeDelay_Set(0, LongestCDelay.data(), LongestFDelay.data(), LongestChain);
}

void PrismForwarding::SetChargeDelay_Get(int chain, int endChain, int LongestChain, DWORD* LongestCDelay, DWORD* LongestFDelay)
{
	auto const TargetTower = this->Owner;

	if (chain == endChain)
	{
		if (chain != LongestChain)
		{

			auto const pTypeData = BuildingTypeExtContainer::Instance.Find(TargetTower->Type);
			//update the delays for this chain
			auto const thisDelay = pTypeData->PrismForwarding.ChargeDelay + LongestCDelay[chain + 1];
			if (thisDelay > LongestCDelay[chain])
			{
				LongestCDelay[chain] = thisDelay;
			}
		}

		if (TargetTower->DelayBeforeFiring > LongestFDelay[chain])
		{
			LongestFDelay[chain] = TargetTower->DelayBeforeFiring;
		}
	}
	else
	{
		//ascend to the next chain
		for (auto SenderTower : this->Senders)
		{
			SenderTower->SetChargeDelay_Get(chain + 1, endChain, LongestChain, LongestCDelay, LongestFDelay);
		}
	}
}

void PrismForwarding::SetSupportTarget(PrismForwarding* pTargetTower)
{
	// meet the new tower, same as the old tower
	if (this->SupportTarget == pTargetTower)
	{
		return;
	}

	// if the target tower is already set, disconnect it by removing it from the old target tower's sender list
	if (auto const pOldTarget = this->SupportTarget)
	{
		pOldTarget->Senders.remove(this);
	}

	this->SupportTarget = pTargetTower;

	// set the new tower as support target
	if (pTargetTower)
	{
		pTargetTower->Senders.push_back_unique(this);
	}
}

void PrismForwarding::RemoveFromNetwork(bool bCease)
{
	if (this->PrismChargeDelay || bCease)
	{
		//either hasn't started charging yet or animations have been reset so should go idle immediately
		this->PrismChargeDelay = 0;
		this->ModifierReserve = 0.0;
		this->DamageReserve = 0;

		if (auto pSlave = this->Owner)
		{
			pSlave->PrismStage = PrismChargeState::Idle;
			pSlave->DelayBeforeFiring = 0;
			//animations should be controlled by whatever incapacitated the tower so no need to mess with anims here
		}
	}

	this->SetSupportTarget(nullptr);

	//finally, remove all the preceding slaves from the network
	for (auto& send : this->Senders)
		send->RemoveFromNetwork(false);

}

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
	StackVector<PrismTargetData, 256> EligibleTowers {};

	for (auto const SlaveTower : *BuildingClass::Array)
	{
		auto const pSlaveData = BuildingExtContainer::Instance.Find(SlaveTower);

		if (auto& pSlve = pSlaveData->MyPrismForwarding) {
			if (this->ValidateSupportTower(TargetTower, pSlve.get())) {
				SlaveTower->GetRenderCoords(&curPosition);
				int Distance = static_cast<int>(MyPosition.DistanceFrom(curPosition));
				PrismTargetData pd = { pSlve.get(), Distance };
				EligibleTowers->push_back(pd);
			}
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
		TargetTower->Owner->GetFLH(&FLH, 0, CoordStruct::Empty);
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
	auto const TargetTower = pTargetTower->Owner;
	auto const SlaveTower = pSlaveTower->Owner;

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
					const auto pMasterHouse = this->Owner->Owner;
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

int PrismForwarding::AcquireSlaves_MultiStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain)
{
	//get all slaves for a specific stage in the prism chain
	//this is done for all sibling chains in parallel, so we prefer multiple short chains over one really long chain
	//towers should be added in the following way:
	// 1---2---4---6
	// |        \
		// |         7
		// |
		// 3---5--8
		// as opposed to
		// 1---2---3---4
		// |          /
		// |         5
		// |
		// 6---7--8
		// ...which would not be as good.
	int  countSlaves = 0;

	if (stage == 0)
	{
		countSlaves += this->AcquireSlaves_SingleStage(TargetTower, stage, chain + 1, NetworkSize, LongestChain);
	}
	else
	{
		// do not think of using iterators or a ranged-for here. Senders grows and might reallocate.
		for (auto sender : TargetTower->Senders)
		{
			countSlaves += this->AcquireSlaves_MultiStage(sender, stage - 1, chain + 1, NetworkSize, LongestChain);
		}
	}

	return countSlaves;
}