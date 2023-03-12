#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

#ifdef IC_AFFECT
//https://github.com/Phobos-developers/Phobos/pull/674
DEFINE_HOOK(0x457C90, BuildingClass_IronCuratin, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x8);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->IronCurtain_Affect.isset())
	{
		if (pTypeExt->IronCurtain_Affect == IronCurtainAffects::Kill)
		{
			R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
			return 0x457CDB;
		}
		else if (pTypeExt->IronCurtain_Affect == IronCurtainAffects::NoAffect)
		{
			R->EAX(DamageState::Unaffected);
			return 0x457CDB;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain, 0x6)
{
	GET(FootClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, STACK_OFFS(0x10, -0x8));
	const TechnoTypeClass* pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	IronCurtainAffects ironAffect = IronCurtainAffects::Affect;

	if (pType->Organic || pThis->WhatAmI() == AbstractType::Infantry)
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
		else
			ironAffect = RulesExt::Global()->IronCurtainToOrganic.Get();
	}
	else
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
	}

	if (ironAffect == IronCurtainAffects::Kill)
	{
		R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
	}
	else if (ironAffect == IronCurtainAffects::Affect)
	{
		R->ESI(pThis);
		return 0x4DEB38;
	}

	R->EAX(DamageState::Unaffected);
	return 0x4DEBA2;
}
#endif

/*
DEFINE_HOOK(0x522600, InfantryClass_IronCurtain, 0x6)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(int, nDuration, 0x4);
	GET_STACK(HouseClass*, pSource, 0x8);
	GET_STACK(bool, ForceShield, 0xC);

	R->EAX(pThis->FootClass::IronCurtain(nDuration, pSource, ForceShield));
	return 0x522639;
}


static bool Kill = false;

DEFINE_HOOK(0x457C90, BuildingClass_IronCuratin, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x8);

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
		if (Kill && pThis->IsAlive && pThis->Health > 0 && !pThis->TemporalTargetingMe) {
			R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
			return 0x457CDB;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain, 0x6)
{
	GET(FootClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x0);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	IronCurtainAffects ironAffect = IronCurtainAffects::Affect;

	if (pThis->WhatAmI() != AbstractType::Infantry)
		pSource = R->Stack<HouseClass*>(0x18);

	if (pType->Organic || pThis->WhatAmI() == AbstractType::Infantry)
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
		else
			ironAffect = RulesExt::Global()->IronCurtainToOrganic.Get();
	}
	else
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
	}

	if (ironAffect == IronCurtainAffects::Kill)
	{
		R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
	}
	else if (ironAffect == IronCurtainAffects::Affect)
	{
		R->ESI(pThis);
		return 0x4DEB38;
	}

	return 0x4DEBA2;
}*/


static DamageState __fastcall InfantryClass_IronCurtain(InfantryClass* pThis, void* _, int nDur, HouseClass* pSource, bool bIsFC)
{

	if (pThis->TemporalTargetingMe && pThis->Destination) {
		if (auto const pCell = MapClass::Instance->GetCellAt(pThis->GetMapCoords())) {
			if (auto const pBld = pCell->GetBuilding()) {
				if (pThis->Destination == pBld && pBld->Type->BridgeRepairHut) {
					return DamageState::Unaffected;
				}
			}
		}
	}

	return pThis->FootClass::IronCurtain(nDur, pSource, bIsFC);
}

DEFINE_JUMP(VTABLE, 0x7EB1AC, GET_OFFSET(InfantryClass_IronCurtain));