#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <Misc/Hooks.Otamaa.h>

#ifdef IC_AFFECT
//https://github.com/Phobos-developers/Phobos/pull/674
ASMJIT_PATCH(0x457C90, BuildingClass_IronCuratin, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x8);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->IronCurtain_Affect.isset())
	{
		if (pTypeExt->IronCurtain_Affect == IronCurtainAffects::Kill)
		{
			R->EAX(pThis->ReceiveDamage(&pThis->Type->Strength, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
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

ASMJIT_PATCH(0x4DEAEE, FootClass_IronCurtain, 0x6)
{
	GET(FootClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, STACK_OFFS(0x10, -0x8));
	const TechnoTypeClass* pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	IronCurtainAffects ironAffect = IronCurtainAffects::Affect;

	if (pType->Organic || Is_Infantry(pThis))
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
		else
			ironAffect = RulesExtData::Instance()->IronCurtainToOrganic.Get();
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

ASMJIT_PATCH(0x4DEAEE, TechnoClass_IronCurtain_Flags, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	GET_STACK(HouseClass*, pSource, STACK_OFFSET(0x10, 0x8));
	GET_STACK(bool, forceshield, STACK_OFFSET(0x10, 0xC));

	enum { MakeInvunlnerable = 0x4DEB38, SkipGameCode = 0x4DEBA2 };
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto what = pThis->WhatAmI();
	bool isOrganic = pType->Organic || what == InfantryClass::AbsID;
	const IronCurtainFlag defaultaffect = (!isOrganic ? IronCurtainFlag::Invulnerable : (forceshield ? &RulesExtData::Instance()->ForceShield_EffectOnOrganics : &RulesExtData::Instance()->IronCurtain_EffectOnOrganics)->Get());
	const IronCurtainFlag affect = (forceshield ? &pTypeExt->ForceShield_Effect : &pTypeExt->IronCurtain_Effect)->Get(defaultaffect);

	switch (EnumFunctions::GetICFlagResult(affect))
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
		const auto killWH = (forceshield ? &pTypeExt->ForceShield_KillWarhead : &pTypeExt->IronCurtain_KillWarhead);
		const auto killWH_org = forceshield ? &RulesExtData::Instance()->ForceShield_KillOrganicsWarhead : &RulesExtData::Instance()->IronCurtain_KillOrganicsWarhead;
		auto killWH_result = killWH->Get(!isOrganic ? RulesClass::Instance->C4Warhead : killWH_org->Get());

		R->EAX
		(
			pThis->ReceiveDamage
			(
				&pThis->Health,
				0,
				killWH_result,
				nullptr,
				true,
				false,
				pSource
			)
		);

	}break;
	default:
	{
		if (!pType->Organic || what != InfantryClass::AbsID) {
			if(forceshield && what != BuildingClass::AbsID){
				R->EAX(DamageState::Unaffected);
			} else {
				return MakeInvunlnerable;
			}
		} else {
			const auto killWH = (forceshield ? &pTypeExt->ForceShield_KillWarhead : &pTypeExt->IronCurtain_KillWarhead);
			const auto killWH_org = forceshield ? &RulesExtData::Instance()->ForceShield_KillOrganicsWarhead : &RulesExtData::Instance()->IronCurtain_KillOrganicsWarhead;
			auto killWH_result = killWH->Get(!isOrganic ? RulesClass::Instance->C4Warhead : killWH_org->Get());

			R->EAX (
			pThis->ReceiveDamage (
				&pThis->Health,
				0,
				killWH_result,
				nullptr,
				true,
				false,
				pSource
			));
		}

	}break;
	}

	return SkipGameCode;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB1AC, FakeInfantryClass::_IronCurtain);
