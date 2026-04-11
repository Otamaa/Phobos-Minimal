#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

static inline bool IsAThreatToMe(TechnoClass* const pTechno, AbstractClass* const pTarget, int weaponIndex = -1)
{
	if (const auto pTechnoTarget = flag_cast_to<TechnoClass*>(pTarget))
	{
		auto pTypeExt = TechnoExtContainer::Instance.Find(pTechnoTarget)->TypeExtData;

		if (pTypeExt->AlwaysConsideredThreat)
			return true;

		if (weaponIndex < 0)
			weaponIndex = pTechnoTarget->SelectWeapon(pTechno);

		if (!pTechnoTarget->GetWeapon(weaponIndex)->WeaponType)
			return false;

		const auto error = pTechnoTarget->GetFireError(pTechno, weaponIndex, true);
		return pTechnoTarget->WhatAmI() == AbstractType::Building ? (error != FireError::ILLEGAL) && (error != FireError::RANGE) : (error != FireError::ILLEGAL);
	}

	return false;
}

// Decide the facing to check for firing.
static inline FacingClass* GetFireFacing(TechnoClass* const pTechno)
{
	switch (pTechno->WhatAmI())
	{
	case AbstractType::Building:
		return &pTechno->PrimaryFacing;
	case AbstractType::Unit:
	{
		if (pTechno->GetTechnoType()->Turret)
			return &pTechno->SecondaryFacing;
		else
			return &pTechno->PrimaryFacing;
	}
	case AbstractType::Aircraft:
		return &pTechno->SecondaryFacing;
	default:
		return nullptr;
	}
}

void ApplyExtraThreat(TechnoClass* pThis, TechnoTypeExtData* pTypeExt, TechnoClass* pTarget, double& totalThreat) {

	if (!pTypeExt->ExtraThreat_Enabled || !pTarget)
		return;

	auto pRules = RulesExtData::Instance();
	double bonus = pTypeExt->ExtraThreat_IsThreat.Get(pRules->ExtraThreat_IsThreat);

	if (bonus > 0.0 && IsAThreatToMe(pThis, pTarget)) {
		totalThreat += bonus;
	}

	double bonus1 = pTypeExt->ExtraThreat_InRange.Get(pRules->ExtraThreat_InRange);
	double dist = pThis->DistanceFrom(pTarget) / 256.0;
	double bonus2 = dist * pTypeExt->ExtraThreatCoefficient_InRangeDistance.Get(pRules->ExtraThreatCoefficient_InRangeDistance);
	double bonushere = bonus1 + bonus2;

	if (bonushere > 0.0 && pThis->IsCloseEnoughToAttack(pTarget)) {
		totalThreat += bonushere;
	}

	if (auto pFacing = GetFireFacing(pThis)) {
		double bonushere2 = pTypeExt->ExtraThreatCoefficient_Facing.Get(pRules->ExtraThreatCoefficient_Facing);

		if (bonushere2 > 0.0) {
			DirStruct dir = DirStruct();
			int deltaFacing = 32768 - std::abs(std::abs(pThis->GetDirectionOverObject(&dir, pTarget)->Raw - pFacing->Current().Raw) - 32768);
			totalThreat += deltaFacing * bonushere2;
		}
	}

	double bonushere3 = pTypeExt->ExtraThreatCoefficient_DistanceToLastTarget.Get(pRules->ExtraThreatCoefficient_DistanceToLastTarget);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (bonushere3 > 0.0 && pExt->LastTargetCrd.IsValid()) {
		double distToLastTarget = pTarget->GetCoords().DistanceFrom(pExt->LastTargetCrd) / 256.0;
		totalThreat += distToLastTarget * bonushere3;
	}
}

double __fastcall FakeTechnoClass::__GetThreatCoeff(TechnoClass* pThis, discard_t, ObjectClass* pTarget, CoordStruct* pTargetCoord)
{
	// Safety check: Ensure target exists and is a valid object
	if (!pTarget || !pTarget->GetType()) {
		return 0.0;
	}

	double myEffCoeff {};
	double targetEffCoeff {};
	double targetSpecialThreatCoeff {};
	double targetStrengthCoeff {};
	double targetDistanceCoeff {};
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	auto pThisType = pExt->TypeExtData->This();

	// 1. Load Coefficients based on Threat Node status
	if (pThis->Owner->HasThreatNode) {
		myEffCoeff = pThisType->MyEffectivenessCoefficient;
		targetEffCoeff = pThisType->TargetEffectivenessCoefficient;
		targetSpecialThreatCoeff = pThisType->TargetSpecialThreatCoefficient;
		targetStrengthCoeff = pThisType->TargetStrengthCoefficient;
		targetDistanceCoeff = pThisType->TargetDistanceCoefficient;
	} else {
		// Use "Dumb" (fallback) coefficients from Rules
		myEffCoeff = RulesClass::Instance->DumbMyEffectivenessCoefficient;
		targetEffCoeff = RulesClass::Instance->DumbTargetEffectivenessCoefficient;
		targetSpecialThreatCoeff = RulesClass::Instance->DumbTargetSpecialThreatCoefficient;
		targetStrengthCoeff = RulesClass::Instance->DumbTargetStrengthCoefficient;
		targetDistanceCoeff = RulesClass::Instance->DumbTargetDistanceCoefficient;
	}

	// 2. Determine Weapons
	int myWeaponIndex = pThis->SelectWeapon(pTarget);
	double threatValue = 0.0;
	AbstractType targetKind = pTarget->WhatAmI();
	TechnoClass* pTargetTechno = nullptr;

	// 3. Counter-Threat Logic (Is the target dangerous to me?)
	// Checks if target is a vehicle, infantry, or building (Kinds 1, 2, 6, 15)
	if (targetKind == AbstractType::Building || targetKind == AbstractType::Infantry || targetKind == AbstractType::Unit || targetKind == AbstractType::Aircraft)
	{
			pTargetTechno = static_cast<TechnoClass*>(pTarget);
		int targetWeaponIdx = pTargetTechno->SelectWeapon(pThis);
		WeaponTypeClass* pTargetWeapon = pTargetTechno->GetWeapon(targetWeaponIdx)->WeaponType;

		if (pTargetWeapon && pTargetWeapon->Warhead)
		{
			WarheadTypeClass* pTargetWH = pTargetWeapon->Warhead;
			auto pTargetWHExt = WarheadTypeExtContainer::Instance.Find(pTargetWH);
			Armor Myarmor = TechnoExtData::GetTechnoArmor(pThis, pTargetWH);
			double modifier = pTargetWHExt->GetVerses(Myarmor).Verses;

			if (pTargetTechno->Target == pThis)
				threatValue = -(targetEffCoeff * modifier); // High priority if attacking me
			else
				threatValue = targetEffCoeff * modifier;
		}

		// Add special threat value weighted by coefficient
		threatValue += targetSpecialThreatCoeff * pTarget->GetTechnoType()->SpecialThreatValue;

		// Bonus for being an actual enemy house
		if (pThis->Owner->EnemyHouseIndex != -1 && pThis->Owner->EnemyHouseIndex == pTargetTechno->Owner->ArrayIndex) {
			threatValue += RulesClass::Instance->EnemyHouseThreatBonus;
		}
	}

	WeaponTypeClass* pMyWeapon = pThis->GetWeapon(myWeaponIndex)->WeaponType;

	// 4. Effectiveness Logic (How well can I hurt the target?)
	if (pMyWeapon && pMyWeapon->Warhead)
	{
		Armor pTargetarmor = TechnoExtData::GetTechnoArmor(pTarget, pMyWeapon->Warhead);
		auto pThisWHExt = WarheadTypeExtContainer::Instance.Find(pMyWeapon->Warhead);
		threatValue += myEffCoeff * pThisWHExt->GetVerses(pTargetarmor).Verses;
	}

	// 5. Target Health Weighting
	double finalScore = (pTarget->GetHealthPercentage_() * targetStrengthCoeff) + threatValue;

	ApplyExtraThreat(pThis, pExt->TypeExtData, pTargetTechno, finalScore);

	// 6. Range and Distance Calculation
	const int weaponRange = pMyWeapon ? WeaponTypeExtData::GetRangeWithModifiers(pMyWeapon, pThis) : pThisType->GuardRange;
	const int cellRange = weaponRange / 256;
	int distanceInCells = 0;

	if (!pTargetCoord->IsValid()) // Check against techno_defaulcoord
	{
		// Distance between this and target
		CoordStruct myPos = pThis->GetCoords();
		CoordStruct targetPos = pTarget->GetCoords();
		distanceInCells = (int)myPos.DistanceFrom(targetPos) / 256;
	} else {
		// Distance between specified coord and target
		CoordStruct targetPos = pTarget->GetCoords();
		distanceInCells = (int)pTargetCoord->DistanceFrom(targetPos) / 256;
	}

	// 7. Distance Weighting
	// The original bit-hack: ((dist - range) & ((dist - range < 0) - 1)) 
	// is equivalent to: max(0, distance - range)
	int rangeDiff = distanceInCells - cellRange;
	if (rangeDiff < 0) rangeDiff = 0;

	return (double)rangeDiff * targetDistanceCoeff + finalScore + 100000.0;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x70CD10, FakeTechnoClass::__GetThreatCoeff)