#include "Body.h"

#include <Utilities/Enum.h>
#include <Utilities/Helpers.h>
#include <Ext/Techno/Body.h>

#include <algorithm>

// Add or substract experience for real
int AddExpCustom(VeterancyStruct* vstruct, int targetCost, int exp)
{
	double toBeAdded = (double)exp / (targetCost * RulesClass::Instance->VeteranRatio);
	// Used in experience transfer to get the actual amount substracted
	int transffered = (int)(MinImpl(vstruct->Veterancy, (float)Math::abs(toBeAdded))
		* (targetCost * RulesClass::Instance->VeteranRatio));

	// Don't do anything when current exp at 0
	if (exp < 0 && transffered <= 0) {
		vstruct->Reset();
		transffered = 0;
	}
	else {
		vstruct->Add(toBeAdded);
	}

	// Prevent going above elite level of 2.0
	if (vstruct->IsElite()) {
		vstruct->SetElite();
	}
	return transffered;
}

int WarheadTypeExtData::TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType)
{
	if (pTechno) {

	switch (valueType) {
	case TransactValueType::Experience: {
		return AddExpCustom(&pTechno->Veterancy,
			pTechnoType ? pTechnoType->GetActualCost(pTechno->Owner) : 0, transactValue);
		}
	  }
	}

	return 0;
}

int WarheadTypeExtData::TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, bool calcFromTarget)
{
	int flatValue = 0, percentValue = 0;

	// Flat
	flatValue = flat;

	// Percent
	if (!CLOSE_ENOUGH(percent, 0.0))
	{
		if (calcFromTarget)
			percentValue = pTarget ? (int)(pTarget->GetTechnoType()->GetActualCost(pTarget->Owner) * percent):0;
		else if (!calcFromTarget)
			percentValue = pOwner ? (int)(pOwner->GetTechnoType()->GetActualCost(pOwner->Owner) * percent):0;
	}

	return Math::abs(percentValue) > Math::abs(flatValue) ? percentValue : flatValue;
}

TransactData WarheadTypeExtData::TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int targets)
{
	TransactData allVal;
	std::vector<int> sourceValues;
	std::vector<int> targetValues;

	const auto IsTargetAffected = [this](TechnoClass* pThis, TechnoClass* pTarget , bool DisablepThisCheck ,bool DisablepTargetCheck , bool IsFlipped = false)
	{
		if (!pThis)
			return DisablepThisCheck;

		if (!CanDealDamage(pTarget , true))
			return IsFlipped;

		if (!pThis->GetOwningHouse())
			return true;

		if (!CanTargetHouse(pThis->GetOwningHouse(), pTarget))
			return false;

		if (!pTarget)
			return DisablepTargetCheck;

		if (!pTarget->GetTechnoType()->Trainable && this->Transact_Experience_IgnoreNotTrainable.Get())
			return false;

		return true;
	};

	// SOURCE
	//		Experience
	int sourceExp = IsTargetAffected(pOwner, pTarget , !this->Transact_Experience_Target_Percent_CalcFromSource, !this->Transact_Experience_Source_Percent_CalcFromTarget) ?
		TransactGetValue(pTarget, pOwner,
		this->Transact_Experience_Source_Flat,
		this->Transact_Experience_Source_Percent,
		this->Transact_Experience_Source_Percent_CalcFromTarget) : 0;

	sourceValues.push_back(sourceExp / targets);
	// TARGET
	//		Experience
	int targetExp = IsTargetAffected(pTarget, pOwner , !this->Transact_Experience_Source_Percent_CalcFromTarget , !this->Transact_Experience_Target_Percent_CalcFromSource , true) ?
		TransactGetValue(pOwner, pTarget,
		this->Transact_Experience_Target_Flat, this->Transact_Experience_Target_Percent,
		this->Transact_Experience_Target_Percent_CalcFromSource) : 0;

	targetValues.push_back(targetExp / targets);

	allVal.emplace_back(sourceValues, targetValues, TransactValueType::Experience) ;

	return allVal;
}

void WarheadTypeExtData::TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets)
{
	auto const pTargetType = pTarget ? pTarget->GetTechnoType() : nullptr;
	auto const pOwnerType = pOwner ? pOwner->GetTechnoType() : nullptr;

	TransactData allValues = this->TransactGetSourceAndTarget(pTarget, pTargetType, pOwner, pOwnerType, targets);

	for (const auto& [vecsourceValue, vectargetValue, nTransactType] : allValues)
	{
		for (unsigned int i = 0; i < vecsourceValue.size(); i++)
		{
			const int sourceValue = vecsourceValue[i];
			const int targetValue = vectargetValue[i];

			// Transact (A loses B gains)
			if (sourceValue != 0 && targetValue != 0 && targetValue * sourceValue < 0)
			{
				int transactValue = Math::abs(sourceValue) > Math::abs(targetValue) ? Math::abs(targetValue) : Math::abs(sourceValue);

				if (sourceValue < 0)
				{
					transactValue = TransactOneValue(pOwner, pOwnerType, -transactValue, nTransactType);
					TransactOneValue(pTarget, pTargetType, transactValue, nTransactType);
				}
				else
				{
					transactValue = TransactOneValue(pTarget, pTargetType, -transactValue, nTransactType);
					TransactOneValue(pOwner, pOwnerType, transactValue, nTransactType);
				}

				return;
			}
			// Out-of-thin-air grants
			if (sourceValue != 0)
			{
				TransactOneValue(pOwner, pOwnerType, sourceValue, nTransactType);
			}

			if (targetValue != 0)
			{
				TransactOneValue(pTarget, pTargetType, targetValue, nTransactType);
			}
		}
	}
}

void WarheadTypeExtData::TransactOnAllUnits(std::vector<TechnoClass*>& nVec, HouseClass* pHouse,TechnoClass* pOwner)
{
	//since we are on last chain of the event , we can do these thing
	const auto NotEligible = [this, pHouse , pOwner](TechnoClass* const pTech)
	{
		if (!CanDealDamage(pTech))
			return true;

		if (!pTech->GetTechnoType()->Trainable && this->Transact_Experience_IgnoreNotTrainable.Get())
			return true;

		return !CanTargetHouse(pHouse, pTech);
	};

	const auto& [rFirst , rEnd] = std::ranges::remove_if(nVec, NotEligible);
	nVec.erase(rFirst, rEnd);

	if (!nVec.empty()) {

		const int count = !this->Transact_SpreadAmongTargets ? 1: nVec.size();

		std::for_each(nVec.begin(), nVec.end(), [this, pOwner, pHouse ,&count](TechnoClass* const pTech) {
			TransactOnOneUnit(pTech, pOwner, count);
		});

	} else {
		TransactOnOneUnit(nullptr, pOwner, 1);
	}

}