#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

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
			R->EAX(pThis->ReceiveDamage(&pThis->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
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

	if (pType->Organic || Is_Infantry(pThis))
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
		R->EAX(pThis->ReceiveDamage(&pThis->GetType()->Strength, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
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

static DamageState __fastcall InfantryClass_IronCurtain(InfantryClass* pThis, void* _, int nDur, HouseClass* pSource, bool bIsFC) {

	if (pThis->Type->Engineer && pThis->TemporalTargetingMe && pThis->Destination) {
		if (auto const pCell = pThis->GetCell()) {
			if (auto const pBld = pCell->GetBuilding()) {
				if (pThis->Destination == pBld && pBld->Type->BridgeRepairHut) {
					return DamageState::Unaffected;
				}
			}
		}
	}

	return pThis->TechnoClass::IronCurtain(nDur, pSource, bIsFC);
}

DEFINE_HOOK(0x4DEAEE, TechnoClass_IronCurtain_Flags, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	GET_STACK(HouseClass*, pSource, STACK_OFFSET(0x10, 0x8));

	enum { MakeInvunlnerable = 0x4DEB38, SkipGameCode = 0x4DEBA2 };
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	switch (EnumFunctions::GetICFlagResult(pTypeExt->IronCurtain_Effect.Get()))
	{
	case IronCurtainFlag::Ignore:
	{
		R->EAX(DamageState::Unaffected);
	}break;
	case IronCurtainFlag::Invulnerable:
	{
		return MakeInvunlnerable;
	}
	case IronCurtainFlag::Kill:
	{
	Kill:
		R->EAX
		(
			pThis->ReceiveDamage
			(
				&pThis->Health,
				0,
				pTypeExt->IronCurtain_KillWarhead.Get(RulesClass::Instance->C4Warhead),
				nullptr,
				true,
				false,
				pSource
			)
		);

	}break;
	default:
	{
		if (!pType->Organic && pThis->WhatAmI() != InfantryClass::AbsID)
			return MakeInvunlnerable;
		else
			goto Kill;

	}break;
	}

	return SkipGameCode;
}

DEFINE_JUMP(VTABLE, 0x7EB1AC, GET_OFFSET(InfantryClass_IronCurtain));