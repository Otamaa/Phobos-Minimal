#pragma once

#include <Utilities/SavegameDef.h>

class BuildingClass;
class PrismForwardingData;
class PrismForwarding
{
public:

	static OPTIONALINLINE HelperedVector<PrismForwarding*> Array {};

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

	void RemoveFromNetwork(bool bCease);
	void SetSupportTarget(PrismForwarding* pTargetTower);
	PrismForwardingData* GetOwnerData() const;
	int AcquireSlaves_MultiStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
	int AcquireSlaves_SingleStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
	bool ValidateSupportTower(PrismForwarding* pTargetTower, PrismForwarding* pSlaveTower);
	void SetChargeDelay_Get(int chain, int endChain, int LongestChain, DWORD* LongestCDelay, DWORD* LongestFDelay);
	void SetChargeDelay(int LongestChain);
	void SetChargeDelay_Set(int chain, DWORD const* LongestCDelay, DWORD const* LongestFDelay, int LongestChain);
	void RemoveAllSenders();
	void InvalidatePointer(AbstractClass* ptr, bool bRemove);

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
};

template <>
struct Savegame::ObjectFactory<PrismForwarding>
{
	std::unique_ptr<PrismForwarding> operator() (PhobosStreamReader& Stm) const {
		return std::make_unique<PrismForwarding>();
	}
};
