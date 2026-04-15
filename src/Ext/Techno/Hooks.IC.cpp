#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

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