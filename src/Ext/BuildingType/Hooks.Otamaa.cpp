#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>
#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>

#pragma region Otamaa

DEFINE_HOOK(0x6FE3F1, TechnoClass_FireAt_OccupyDamageBonus, 0xB)
{
	enum
	{
		ApplyDamageBonus = 0x6FE405,
		Nothing = 0x0
	};

	GET(TechnoClass* const, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis)) {
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type)) {
			GET_STACK(int, nDamage, 0x2C);
			R->EAX(Game::F2I(nDamage * TypeExt->BuildingOccupyDamageMult.Get(RulesGlobal->OccupyDamageMultiplier)));
			return ApplyDamageBonus;
		}
	}

	return Nothing;
}

DEFINE_HOOK(0x6FE421, TechnoClass_FireAt_BunkerDamageBonus, 0xB)
{
	enum
	{
		ApplyDamageBonus = 0x6FE435,
		Nothing = 0x0
	};

	GET(TechnoClass* const, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem)) {
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type)) {
			GET_STACK(int, nDamage, 0x2C);
			R->EAX(Game::F2I(nDamage * TypeExt->BuildingBunkerDamageMult.Get(RulesGlobal->OccupyDamageMultiplier)));
			return ApplyDamageBonus;
		}
	}

	return Nothing;
}

DEFINE_HOOK(0x6FD183, TechnoClass_RearmDelay_BuildingOccupyROFMult, 0xC)
{
	enum
	{
		ApplyRofMod = 0x6FD1AB,
		SkipRofMod = 0x6FD1B1,
		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis)) {
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type)) {
			auto const nMult = TypeExt->BuildingOccupyROFMult.Get(RulesGlobal->OccupyROFMultiplier);
			if (nMult != 0.0f) {
				GET_STACK(int, nROF, STACK_OFFS(0x10, -0x4));
				R->EAX(Game::F2I(((double)nROF) / nMult));
				return ApplyRofMod;
			}
			return SkipRofMod;
		}
	}

	return Nothing;
}

DEFINE_HOOK(0x6FD1C7, TechnoClass_RearmDelay_BuildingBunkerROFMult, 0xC)
{
	enum
	{
		ApplyRofMod = 0x6FD1EF,
		SkipRofMod = 0x6FD1F1,
		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem)) {
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type)) {
			auto const nMult = TypeExt->BuildingBunkerROFMult.Get(RulesGlobal->BunkerROFMultiplier);
			if (nMult != 0.0f) {
				GET_STACK(int, nROF, STACK_OFFS(0x10, -0x4));
				R->EAX(Game::F2I(((double)nROF) / nMult));
					return ApplyRofMod;
			}
				return SkipRofMod;
		}
	}

	return Nothing;
}

#pragma region BunkerSounds
DEFINE_HOOK(0x45933D, BuildingClass_BunkerWallUpSound, 0x5)
{
	GET(BuildingClass* const, pThis, ESI);
	BuildingTypeExt::BunkerSound<true>()(pThis);
	return 0x459374;
}

DEFINE_HOOK(0x4595D9, BuildingClass_4595C0_BunkerDownSound, 0x5)
{
	GET(BuildingClass* const, pThis, EDI);
	BuildingTypeExt::BunkerSound<false>()(pThis);
	return 0x459612;
}

DEFINE_HOOK(0x459494, BuildingClass_459470_BunkerDownSound, 0x5)
{
	GET(BuildingClass* const, pThis, ESI);
	BuildingTypeExt::BunkerSound<false>()(pThis);
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
	auto const pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	pThis->GetCenterCoord(&nBuffer);
	VocClass::PlayIndexAtPos(pThis->Type->PackupSound, nBuffer, pExt && pExt->PackupSound_PlayGlobal.Get());

	return Handled;
}

DEFINE_HOOK_AGAIN(0x4426DB, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK_AGAIN(0x702777, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK(0x70272E, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
{
	enum
	{
		BuildingClass_TakeDamage_DamageSound = 0x4426DB,
		BuildingClass_TakeDamage_DamageSound_Handled_ret = 0x44270B,

		TechnoClass_TakeDamage_Building_DamageSound_01 = 0x702777,
		TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret = 0x7027AE,

		TechnoClass_TakeDamage_Building_DamageSound_02 = 0x70272E,
		TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret = 0x702765,

		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		if (pExt && pExt->DisableDamageSound.Get())
		{
			switch (R->Origin())
			{
			case BuildingClass_TakeDamage_DamageSound:
				return BuildingClass_TakeDamage_DamageSound_Handled_ret;
			case TechnoClass_TakeDamage_Building_DamageSound_01:
				return TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret;
			case TechnoClass_TakeDamage_Building_DamageSound_02:
				return TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret;
			}
		}
	}

	return Nothing;
}*/

DEFINE_HOOK(0x450821, BuildingClass_Repair_AI_Step, 0xB)
{
	GET(BuildingClass* const, pThis, ESI);

	if (pThis && pThis->Type) {
		if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type)) {
			R->EAX(Game::F2I(pTypeExt->RepairRate.Get(RulesGlobal->RepairRate) * 900.0));
			return 0x450837;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x70BEF9, TechnoClass_canHealRepair_Building, 0xB)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis)) {
		if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type)) {
			R->EAX(Game::F2I(pTypeExt->RepairRate.Get(RulesGlobal->RepairRate) * 900.0));
			return 0x70BF0F;
		}
	}

	return 0;
}

//https://modenc.renegadeprojects.com/RepairStep
DEFINE_HOOK(0x712120, TechnoTypeClass_GetRepairStep_Building, 0x5)
{
	GET(TechnoTypeClass*, pThis, ECX);

	auto nStep = RulesGlobal->RepairStep;
	if (auto const pBuildingType = type_cast<BuildingTypeClass*>(pThis))
		if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType))
			nStep = pTypeExt->RepairStep.Get(nStep);

	R->EAX(nStep);

	return 0x71212B;
}

DEFINE_HOOK(0x7120D0, TechnoTypeClass_GetRepairCost_Building, 0x4)
{
	GET(TechnoTypeClass*, pThis, ECX);

	int nVal = 1;

	if (pThis) {
		auto nStep = RulesGlobal->RepairStep;
		if (auto const pBuildingType = type_cast<BuildingTypeClass*>(pThis))
			if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType))
				nStep = pTypeExt->RepairStep.Get(nStep);

		nVal = (Math::clamp(((int)((pThis->GetCost() / (pThis->Strength / nStep) * RulesGlobal->RepairPercent))), 1, MAX_VAL(int)));
	}

	R->EAX(nVal);
	return 0x712119;
}

DEFINE_HOOK(0x464758, BuildingTypeClass_LoadFromINI_PowerUPZAdjust, 0x8) {
	GET(int, nIndex, EBX);
	GET(BuildingTypeClass*, pThis, EBP);

	char flag[0x800];
	sprintf_s(flag, "PowerUp%01dZAdjust", nIndex);
	pThis->BuildingAnim[nIndex].ZAdjust = CCINIClass::INI_Art().ReadInteger(pThis->ImageFile, flag, pThis->BuildingAnim[nIndex].ZAdjust);

	return 0;
}

DEFINE_HOOK(0x706389, TechnoClass_Draw_Object_NormalLight, 0x6) {
	GET(TechnoClass*, pThis, ESI);

	if (const auto pBuilding = specific_cast<BuildingClass*>(pThis)) {
		if ((pBuilding->CurrentMission == Mission::Construction)
			&& !pBuilding->BState
			&& pBuilding->Type->Buildup
			) {
			auto const pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
			if (pExt && pExt->BuildUp_UseNormalLIght.Get()) {
				R->EBP(1000);
			}
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x505F6C, HouseClass_GenerateAIBuildList_AIBuildInstead, 0x6)
{
	GET(HouseClass*, pHouse, ESI);

	if (!pHouse->ControlledByHuman() && !pHouse->IsNeutral()) {
		for (auto& nNodes : pHouse->Base.BaseNodes) {
			auto nIdx = nNodes.BuildingTypeIndex;
			if (nIdx >= 0) {
				if (auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(BuildingTypeClass::Array->GetItem(nIdx)))
					if (!pBldTypeExt->AIBuildInsteadPerDiff.empty() && pBldTypeExt->AIBuildInsteadPerDiff.at(pHouse->GetCorrectAIDifficultyIndex()) != -1)
						nIdx = pBldTypeExt->AIBuildInsteadPerDiff.at(pHouse->GetCorrectAIDifficultyIndex());

				nNodes.BuildingTypeIndex = nIdx;
			}
		}
	}

	return 0;
}

#define GET_TIMETO_BUILD(ret)\
GET(TechnoClass*, pTech, ECX);\
GET(int, nTime, EDX);\
if (pTech) {\
	nTime = static_cast<int>(BuildingTypeExt::GetExternalFactorySpeedBonus(pTech) * pTech->TimeToBuild()); }\
R->EDX(nTime);\
return ret;

DEFINE_HOOK(0x4CA702, HouseClass_RecalcAllFactory_ExternalBonus, 0x4)
{
	GET_TIMETO_BUILD(0x4CA70D);
}

DEFINE_HOOK(0x4C9FB5, FactoryClass_TimeToBuild_ExternalBonus, 0x4)
{
	GET_TIMETO_BUILD(0x4C9FC0);
}

DEFINE_HOOK(0x4C9EEB, FactoryClass_Start_ExternalBonus, 0x4)
{
	GET_TIMETO_BUILD(0x4C9EF6);
}
#undef GET_TIMETO_BUILD
#pragma endregion