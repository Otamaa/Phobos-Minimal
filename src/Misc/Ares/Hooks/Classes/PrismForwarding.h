#pragma once

#include <Utilities/SavegameDef.h>

class BuildingClass;
class PrismForwardingData;
class PrismForwarding
{
public:

	BuildingClass* Owner;
	DynamicVectorClass<PrismForwarding*> Senders;		//the prism towers that are forwarding to this one
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
	}

	~PrismForwarding() {
		this->RemoveFromNetwork(true);
	}

	BuildingClass* GetOwner() const;
	PrismForwardingData* GetOwnerData() const;

	int AcquireSlaves_MultiStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
	int AcquireSlaves_SingleStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
	bool ValidateSupportTower(PrismForwarding* pTargetTower, PrismForwarding* pSlaveTower);
	void SetChargeDelay(int LongestChain);
	void SetChargeDelay_Get(int chain, int endChain, int LongestChain, DWORD* LongestCDelay, DWORD* LongestFDelay);
	void SetChargeDelay_Set(int chain, DWORD const* LongestCDelay, DWORD const* LongestFDelay, int LongestChain);
	void RemoveFromNetwork(bool bCease);
	void SetSupportTarget(PrismForwarding* pTargetTower);
	void RemoveAllSenders();

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		// support pointer to this type
		Stm.RegisterChange(this);
		return Stm
			.Process(this->Owner, RegisterForChange)
			.Process(this->Senders, RegisterForChange)
			.Process(this->SupportTarget, RegisterForChange)
			.Process(this->PrismChargeDelay)
			.Process(this->ModifierReserve)
			.Process(this->DamageReserve)
			.Success()
				&& Stm.RegisterChange(this)
			;
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		// remember this object address
		Stm.RegisterChange(this);
		return Stm
			.Process(this->Owner)
			.Process(this->Senders)
			.Process(this->SupportTarget)
			.Process(this->PrismChargeDelay)
			.Process(this->ModifierReserve)
			.Process(this->DamageReserve)
			.Success()
				&& Stm.RegisterChange(this)
			;
	}
};