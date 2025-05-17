#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/MemoryPoolUniquePointer.h>

class BuildingClass;
class PrismForwardingData;
class PrismForwarding  final : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(PrismForwarding, "PrismForwarding")

public:

	static HelperedVector<PrismForwarding*> Array;

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

	void RemoveFromNetwork(bool bCease);
	void SetSupportTarget(PrismForwarding* pTargetTower);
	PrismForwardingData* GetOwnerData() const;
	int AcquireSlaves_MultiStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
	int AcquireSlaves_SingleStage(PrismForwarding* TargetTower, int stage, int chain, int& NetworkSize, int& LongestChain);
	bool ValidateSupportTower(PrismForwarding* pTargetTower, PrismForwarding* pSlaveTower);
	void SetChargeDelay_Get(int chain, int endChain, int LongestChain, std::vector<DWORD>& LongestCDelay, std::vector<DWORD>& LongestFDelay);
	void SetChargeDelay(int LongestChain);
	void SetChargeDelay_Set(int chain, std::vector<DWORD>& LongestCDelay, std::vector<DWORD>& LongestFDelay, int LongestChain);
	void RemoveAllSenders();
	void OPTIONALINLINE InvalidatePointer(AbstractClass* ptr, bool bRemove)
	{
		if (bRemove && this->SupportTarget && (AbstractClass*)this->SupportTarget->Owner == ptr)
			this->SupportTarget = nullptr;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		// support pointer to this type

		return Stm
			.Process(this->Owner)
			.Process(this->Senders)
			.Process(this->SupportTarget)
			.Process(this->PrismChargeDelay)
			.Process(this->ModifierReserve)
			.Process(this->DamageReserve)
			.Success() && Stm
			.RegisterChange(this)
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
			.Success() && Stm
			.RegisterChange(this)
			;
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return Stm
			.Process(Array);
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return Stm
			.Process(Array);
	}
};

template <>
struct Savegame::ObjectFactory<PrismForwarding>
{
	MemoryPoolUniquePointer<PrismForwarding> operator() (PhobosStreamReader& Stm) const {
		return PrismForwarding::createInstance();
	}
};
