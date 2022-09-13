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
	int transffered = (int)(Math::min(vstruct->Veterancy, (float)abs(toBeAdded))
		* (targetCost * RulesClass::Instance->VeteranRatio));

	// Don't do anything when current exp at 0
	if (exp < 0 && transffered <= 0) {
		vstruct->Reset();
		transffered = 0;
	}
	else {
		vstruct->Add(toBeAdded);
	}

	return transffered;
}

int WarheadTypeExt::ExtData::TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType)
{
	if (!pTechno)
		return 0;

	int transferred = 0;
	switch (valueType)
	{
	case TransactValueType::Experience:
		transferred = AddExpCustom(&pTechno->Veterancy,
			pTechnoType ? pTechnoType->GetActualCost(pTechno->Owner) : 0, transactValue);
		break;
	default:
		break;
	}
	return transferred;
}

int WarheadTypeExt::ExtData::TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, bool calcFromTarget)
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

	return abs(percentValue) > abs(flatValue) ? percentValue : flatValue;
}

std::pair<std::vector<int>, std::vector<int>> WarheadTypeExt::ExtData::TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int targets)
{
	std::vector<int> sourceValues;
	std::vector<int> targetValues;

	const auto IsTargetAffected = [this](TechnoClass* pThis, TechnoClass* pTarget , bool DisablepThisCheck ,bool DisablepTargetCheck , bool IsFlipped = false)
	{
		if (!pThis)
			return DisablepThisCheck;

		if (!CanDealDamage(pTarget))
			return IsFlipped;

		if (!pThis->GetOwningHouse())
			return true;

		if (!CanTargetHouse(pThis->GetOwningHouse(), pTarget))
			return false;

		if (!pTarget)
			return DisablepTargetCheck;

		if (!pTarget->GetTechnoType()->Trainable)
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

	return { sourceValues,targetValues };
}

void WarheadTypeExt::ExtData::TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets)
{
	auto const pTargetType = pTarget ? pTarget->GetTechnoType() : nullptr;
	auto const pOwnerType = pOwner ? pOwner->GetTechnoType() : nullptr;

	std::pair<std::vector<int> , std::vector<int>> allValues = this->TransactGetSourceAndTarget(pTarget, pTargetType, pOwner, pOwnerType, targets);

	for (unsigned int i = 0; i < allValues.first.size(); i++) {
		int sourceValue = allValues.first[i];
		int targetValue = allValues.second[i];

		// Transact (A loses B gains)
		if (sourceValue != 0 && targetValue != 0 && targetValue * sourceValue < 0) {
			int transactValue = abs(sourceValue) > abs(targetValue) ? abs(targetValue) : abs(sourceValue);

			if (sourceValue < 0) {
				transactValue = TransactOneValue(pOwner, pOwnerType, -transactValue, TransactValueType::Experience);
				TransactOneValue(pTarget, pTargetType, transactValue, TransactValueType::Experience);
			} else {
				transactValue = TransactOneValue(pTarget, pTargetType, -transactValue, TransactValueType::Experience);
				TransactOneValue(pOwner, pOwnerType, transactValue, TransactValueType::Experience);
			}

			return;
		}
		// Out-of-thin-air grants
		if (sourceValue != 0) {
			TransactOneValue(pOwner, pOwnerType, sourceValue, TransactValueType::Experience);
		}

		if (targetValue != 0) {
			TransactOneValue(pTarget, pTargetType, targetValue, TransactValueType::Experience);
		}
	}
}

void WarheadTypeExt::ExtData::TransactOnAllUnits(std::vector<TechnoClass*>& nVec, HouseClass* pHouse,TechnoClass* pOwner)
{
	//since we are on last chain of the event , we can do these thing
	const auto NotEligible = [this, pHouse , pOwner](TechnoClass* const pTech)
	{
		return !(CanDealDamage(pTech) &&
		pTech->GetTechnoType()->Trainable &&
		CanTargetHouse(pHouse, pTech));
	};

	const auto [rFirst , rEnd] = std::ranges::remove_if(nVec, NotEligible);
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