
#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>

#pragma region Otamaa

DEFINE_HOOK(0x6FE3E3, TechnoClass_FireAt_OccupyDamageBonus, 0xA) //B
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_STACK(int, nDamage, 0x2C);
	GET_BASE(int, weapon_idx, 0xC);
	GET_BASE(AbstractClass*, pTarget, 0x8);

	if(pThis->CanOccupyFire()) {
		if (auto const Building = specific_cast<BuildingClass*>(pThis)) {
			nDamage = int(nDamage * BuildingTypeExtContainer::Instance.Find(Building->Type)->BuildingOccupyDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier));
		} else {
			nDamage = int(nDamage * RulesClass::Instance->OccupyDamageMultiplier);
		}
	}

	if(pThis->WhatAmI() != BuildingClass::AbsID) {
		if (auto const Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem)) {
			nDamage = int(nDamage * BuildingTypeExtContainer::Instance.Find(Building->Type)->BuildingBunkerDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier));
		}
	}

	if (pThis->InOpenToppedTransport) {
		if (auto const  pTransport = pThis->Transporter) {
			float nDamageMult = TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType())->OpenTopped_DamageMultiplier
				.Get(RulesClass::Instance->OpenToppedDamageMultiplier);
			nDamage = int(nDamage * nDamageMult);
		}
		else
		{
			nDamage = int(nDamage * RulesClass::Instance->OpenToppedDamageMultiplier);
		}
	}

	if(!pWeapon->DiskLaser) {
		R->EDI(nDamage);
		R->Stack(0x2C, nDamage);
		return 0x6FE4F6; // continue check
	}

	auto pDiskLaser = GameCreate<DiskLaserClass>();

	++pThis->CurrentBurstIndex;
	int rearm = pThis->GetROF(weapon_idx);
	pThis->ROF = rearm;
	pThis->DiskLaserTimer.Start(rearm);
	pThis->CurrentBurstIndex %= pWeapon->Burst;
	pDiskLaser->Fire(pThis, pTarget, pWeapon, nDamage);

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//this may crash the game , since the object got deleted after killself ,..
	if (pWeaponExt->RemoveTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Vanish);
	else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Explode);

	return 0x6FE4E7; //end of func
}

DEFINE_HOOK(0x6FD15E, TechnoClass_RearmDelay_RofMult, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, nROF, 0x14);

	auto const Building = specific_cast<BuildingClass*>(pThis);

	if (pThis->CanOccupyFire()) {
		const auto occupant = pThis->GetOccupantCount();

		if (occupant > 0) {
			nROF /= occupant;
		}

		auto OccupyRofMult = RulesClass::Instance->OccupyROFMultiplier;

		if (Building) {
			OccupyRofMult = BuildingTypeExtContainer::Instance.Find(Building->Type)->BuildingOccupyROFMult.Get(OccupyRofMult);
		}

		if (OccupyRofMult > 0.0)
			nROF = int(float(nROF) / OccupyRofMult);

	}

	if (pThis->BunkerLinkedItem && !Building) {
		auto BunkerMult = RulesClass::Instance->BunkerROFMultiplier;
		if (auto const pBunkerIsBuilding = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem)) {
			BunkerMult = BuildingTypeExtContainer::Instance.Find(pBunkerIsBuilding->Type)->BuildingBunkerROFMult.Get(BunkerMult);
		}

		if(BunkerMult != 0.0)
			nROF = int(float(nROF) / BunkerMult);
	}

	R->EAX(nROF);
	return 0x6FD1F3;
}

#pragma region BunkerSounds
DEFINE_HOOK(0x45933D, BuildingClass_BunkerWallUpSound, 0x5)
{
	GET(BuildingClass* const, pThis, ESI);
	const auto nSound = BuildingTypeExtContainer::Instance.Find(pThis->Type)->BunkerWallsUpSound.Get(RulesClass::Instance->BunkerWallsUpSound);
	VocClass::PlayIndexAtPos(nSound, pThis->Location);
	return 0x459374;
}

DEFINE_HOOK(0x4595D9, BuildingClass_4595C0_BunkerDownSound, 0x5)
{
	GET(BuildingClass* const, pThis, EDI);
	const auto nSound = BuildingTypeExtContainer::Instance.Find(pThis->Type)->BunkerWallsDownSound.Get(RulesClass::Instance->BunkerWallsDownSound);
	VocClass::PlayIndexAtPos(nSound, pThis->Location);
	return 0x459612;
}

DEFINE_HOOK(0x459494, BuildingClass_459470_BunkerDownSound, 0x5)
{
	GET(BuildingClass* const, pThis, ESI);
	const auto nSound = BuildingTypeExtContainer::Instance.Find(pThis->Type)->BunkerWallsDownSound.Get(RulesClass::Instance->BunkerWallsDownSound);
	VocClass::PlayIndexAtPos(nSound, pThis->Location);
	return 0x4594CD;
}
#pragma endregion

// not working  ?
// hook itself is fine , but it not play globally as it should :s
/*
DEFINE_HOOK(0x44A86A, BuildingClass_Mi_Selling_PackupSound, 0xC)
{
	enum
	{
		Handled = 0x44A89E
	};

	GET(BuildingClass* const, pThis, EBP);
	CoordStruct nBuffer;
	auto const pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	pThis->GetCenterCoord(&nBuffer);
	VocClass::PlayIndexAtPos(pThis->Type->PackupSound, nBuffer, pExt && pExt->PackupSound_PlayGlobal.Get());

	return Handled;
}

*/

DEFINE_HOOK(0x450821, BuildingClass_Repair_AI_Step, 0x5)// B
{
	GET(BuildingClass* const, pThis, ESI);

	R->EAX(int(BuildingTypeExtContainer::Instance.Find(pThis->Type)
			->RepairRate.Get(RulesClass::Instance->RepairRate) * 900.0));

	return 0x450837;
}

// ares replace this
//DEFINE_HOOK(0x70BEF9, TechnoClass_canHealRepair_Building, 0x5) //B
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis)) {
//		R->EAX(int(BuildingTypeExtContainer::Instance.Find(pBuilding->Type)
//			->RepairRate.Get(RulesClass::Instance->RepairRate) * 900.0));
//		return 0x70BF0F;
//	}
//
//	return 0;
//}

//https://modenc.renegadeprojects.com/RepairStep
DEFINE_HOOK(0x712125, TechnoTypeClass_GetRepairStep_Building, 0x6)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET(RulesClass*, pRules, EAX);

	auto nStep = pRules->RepairStep;
	if (auto const pBuildingType = specific_cast<BuildingTypeClass*>(pThis))
		nStep = BuildingTypeExtContainer::Instance.Find(pBuildingType)->RepairStep.Get(nStep);

	R->EAX(nStep);

	return 0x71212B;
}

//was 4
DEFINE_HOOK(0x7120D0, TechnoTypeClass_GetRepairCost_Building, 0x7)
{
	GET(TechnoTypeClass*, pThis, ECX);

	int cost = pThis->GetCost();
	if (!cost || !pThis->Strength)
	{
		R->EAX(1);
		return 0x712119;
	}

	int nStep = RulesClass::Instance->RepairStep;
	if (auto const pBuildingType = specific_cast<BuildingTypeClass*>(pThis))
	{
		if (BuildingTypeExtContainer::Instance.Find(pBuildingType)->RepairStep.isset())
			nStep = BuildingTypeExtContainer::Instance.Find(pBuildingType)->RepairStep;
	}

	if (!nStep)
	{
		R->EAX(1);
		return 0x712119;
	}

	const auto nCalc = int(((double)cost / int((double)pThis->Strength / nStep)) * RulesClass::Instance->RepairPercent);
	R->EAX(MaxImpl(nCalc, 1));
	return 0x712119;
}

//RepairCost
//DEFINE_JUMP(VTABLE, 0x7E2918, GET_OFFSET(GetRepairCost));
//DEFINE_JUMP(VTABLE, 0x7E4620, GET_OFFSET(GetRepairCost));
//DEFINE_JUMP(VTABLE, 0x7F4F88, GET_OFFSET(GetRepairCost));
//DEFINE_JUMP(VTABLE, 0x7F62C8, GET_OFFSET(GetRepairCost));

// DEFINE_HOOK(0x464758, BuildingTypeClass_LoadFromINI_PowerUPZAdjust, 0x8) {
// 	GET(int, nIndex, EBX);
// 	GET(BuildingTypeClass*, pThis, EBP);

// 	char flag[0x800];
// 	sprintf_s(flag, "PowerUp%01dZAdjust", nIndex);
// 	pThis->BuildingAnim[nIndex].ZAdjust = CCINIClass::INI_Art().ReadInteger(pThis->ImageFile, flag, pThis->BuildingAnim[nIndex].ZAdjust);

// 	return 0;
// }

DEFINE_HOOK(0x505F6C, HouseClass_GenerateAIBuildList_AIBuildInstead, 0x6)
{
	GET(HouseClass*, pHouse, ESI);

	if (!pHouse->IsControlledByHuman() && !pHouse->IsNeutral()) {
		for (auto& nNodes : pHouse->Base.BaseNodes) {
			auto nIdx = nNodes.BuildingTypeIndex;
			if (nIdx >= 0) {

				const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(BuildingTypeClass::Array->Items[nIdx]);

				if (!pBldTypeExt->AIBuildInsteadPerDiff.empty() && pBldTypeExt->AIBuildInsteadPerDiff[pHouse->GetCorrectAIDifficultyIndex()] != -1)
					nIdx = pBldTypeExt->AIBuildInsteadPerDiff[pHouse->GetCorrectAIDifficultyIndex()];

				nNodes.BuildingTypeIndex = nIdx;
			}
		}
	}

	return 0;
}

#pragma endregion