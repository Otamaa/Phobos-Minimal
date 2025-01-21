#pragma once

#include <Utilities/SavegameDef.h>
#include <Ext/BuildingType/Body.h>

class BuildingClass;
class PrismForwardingData;
class PrismForwarding
{
public:

	static inline HelperedVector<PrismForwarding*> Array;

	BuildingClass* Owner;
	HelperedVector<PrismForwarding*> Senders;		//the prism towers that are forwarding to this one
	PrismForwarding* SupportTarget;			//what tower am I sending to?
	int PrismChargeDelay;					//current delay charge
	double ModifierReserve;					//current modifier reservoir
	int DamageReserve;					//current flat reservoir

	// constructor
	PrismForwarding() : Owner(nullptr),
		Senders(),
		SupportTarget(nullptr),
		PrismChargeDelay(0),
		ModifierReserve(0.0),
		DamageReserve(0)
	{
		Array.push_back(this);
	}

	~PrismForwarding() {
		this->RemoveFromNetwork(true);
		this->Owner = nullptr;
		this->Senders.clear();

		for (auto& pr : Array) {
			if (pr != this && pr->SupportTarget == this) {
				pr->SetSupportTarget(nullptr);
			}
		}

		Array.remove(this);
	}

	COMPILETIMEEVAL BuildingClass* GetOwner() const
	{
		return this->Owner;
	}

	COMPILETIMEEVAL PrismForwardingData* GetOwnerData() const
	{
		return BuildingTypeExtContainer::Instance.Find(this->Owner->Type)->PrismForwarding.AsPointer();
	}

	COMPILETIMEEVAL int AcquireSlaves_MultiStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain)
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
			for (auto& sender : TargetTower->Senders)
			{
				countSlaves += this->AcquireSlaves_MultiStage(sender, stage - 1, chain + 1, NetworkSize, LongestChain);
			}
		}

		return countSlaves;
	}

	int AcquireSlaves_SingleStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
	bool ValidateSupportTower(PrismForwarding* pTargetTower, PrismForwarding* pSlaveTower);

	COMPILETIMEEVAL void SetChargeDelay(int LongestChain)
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

	COMPILETIMEEVAL void SetChargeDelay_Get(int chain, int endChain, int LongestChain, DWORD* LongestCDelay, DWORD* LongestFDelay)
	{
		auto const TargetTower = this->GetOwner();

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
			for (auto& SenderTower : this->Senders)
			{
				SenderTower->SetChargeDelay_Get(chain + 1, endChain, LongestChain, LongestCDelay, LongestFDelay);
			}
		}
	}

	COMPILETIMEEVAL void SetChargeDelay_Set(int chain, DWORD const* LongestCDelay, DWORD const* LongestFDelay, int LongestChain)
	{
		auto const pTargetTower = this->GetOwner();

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

		for (auto& Sender : this->Senders)
		{
			Sender->SetChargeDelay_Set(chain + 1, LongestCDelay, LongestFDelay, LongestChain);
		}
	}

	COMPILETIMEEVAL void RemoveFromNetwork(bool bCease)
	{
		if (this->PrismChargeDelay || bCease)
		{
			//either hasn't started charging yet or animations have been reset so should go idle immediately
			this->PrismChargeDelay = 0;
			this->ModifierReserve = 0.0;
			this->DamageReserve = 0;

			auto pSlave = this->GetOwner();
			pSlave->PrismStage = PrismChargeState::Idle;
			pSlave->DelayBeforeFiring = 0;
			//animations should be controlled by whatever incapacitated the tower so no need to mess with anims here
		}

		this->SetSupportTarget(nullptr);

		//finally, remove all the preceding slaves from the network
		for (int senderIdx = ((int)this->Senders.size()) - 1; senderIdx > 0; --senderIdx)
		{
			this->Senders[senderIdx]->RemoveFromNetwork(false);
		}
	}

	COMPILETIMEEVAL void SetSupportTarget(PrismForwarding* pTargetTower)
	{
		// meet the new tower, same as the old tower
		if (this->SupportTarget == pTargetTower)
		{
			return;
		}

		// if the target tower is already set, disconnect it by removing it from the old target tower's sender list
		if (auto const pOldTarget = this->SupportTarget)
		{
			if (!pOldTarget->Senders.remove(this))
			{
				Debug::Log("PrismForwarding::SetSupportTarget: Old target tower (%p) did not consider this tower (%p) as its sender.\n",
					pOldTarget->GetOwner(), this->GetOwner());
			}
		}

		this->SupportTarget = pTargetTower;

		// set the new tower as support target
		if (pTargetTower)
		{
			pTargetTower->Senders.push_back_unique(this);
		}
	}

	COMPILETIMEEVAL void RemoveAllSenders()
	{
		// disconnect all sender towers from their support target, which is me
		for (auto senderIdx = ((int)this->Senders.size()) - 1; senderIdx > 0; senderIdx--)
		{
			this->Senders[senderIdx]->SetSupportTarget(nullptr);
		}

		// log if not all senders could be removed
		if (this->Senders.size())
		{
			Debug::Log("PrismForwarding::RemoveAllSenders: Tower (%p) still has %d senders after removal completed.\n",
				this->GetOwner(), this->Senders.size());

			for (size_t i = 0; i < this->Senders.size(); ++i)
			{
				Debug::Log("Sender %03d: %p\n", i, this->Senders[i]->GetOwner());
			}

			this->Senders.clear();
		}
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		// support pointer to this type
		return Stm
			.Process(this->Owner, true)
			.Process(this->Senders, RegisterForChange)
			.Process(this->SupportTarget, RegisterForChange)
			.Process(this->PrismChargeDelay)
			.Process(this->ModifierReserve)
			.Process(this->DamageReserve)
			.Success() && Stm.RegisterChange(this)
			;
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		// remember this object address
		return Stm
			.Process(this->Owner)
			.Process(this->Senders)
			.Process(this->SupportTarget)
			.Process(this->PrismChargeDelay)
			.Process(this->ModifierReserve)
			.Process(this->DamageReserve)
			.Success() && Stm.RegisterChange(this)
			;
	}

	COMPILETIMEEVAL void InvalidatePointer(AbstractClass* ptr, bool bRemove)
	{
		if (bRemove && this->SupportTarget && this->SupportTarget->Owner == ptr)
			this->SupportTarget = nullptr;
	}

public:
	PrismForwarding(const PrismForwarding&) = delete;
	PrismForwarding& operator = (const PrismForwarding&) = delete;
	PrismForwarding& operator = (PrismForwarding&&) = delete;
};

template <>
struct Savegame::ObjectFactory<PrismForwarding>
{
	std::unique_ptr<PrismForwarding> operator() (PhobosStreamReader& Stm) const {
		return std::make_unique<PrismForwarding>();
	}
};