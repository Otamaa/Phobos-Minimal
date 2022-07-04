#include <TechnoClass.h>
#include <HouseClass.h>
#include <AnimClass.h>
#include <ScenarioClass.h>
#include <SpecificStructures.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>

#include <Utilities/GeneralUtils.h>

DEFINE_HOOK(0x62A16A, ParasiteClass_AI_DisableRocking, 0x7)
{
	//GET(ParasiteClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EDI);

	auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	return pWarheadExt && pWarheadExt->Parasite_DisableRocking.Get()
		? 0x62A222 : 0x0;
}

DEFINE_HOOK(0x62A71C, ParasiteClass_ExitUnit_ExitSound, 0x6)
{
	GET(TechnoClass* const, pParasiteOwner, EAX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x3C, 0x18));

	if (pParasiteOwner)
		if (auto pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pParasiteOwner->GetTechnoType()))
			VoxClass::PlayAtPos(pOwnerTypeExt->ParasiteExit_Sound.Get(), &nCoord);

	return 0;
}

DEFINE_HOOK(0x629B50, ParasiteClass_SquiddyGrab_DeharcodeSplash, 0x7)
{
	enum { Handled = 0x629B9C, Continue = 0x0 };

	GET_STACK(WeaponTypeClass* const, pWeapon, STACK_OFFS(0x70, 0x4C));
	GET_STACK(CoordStruct*, pCoord, STACK_OFFS(0x70, 0xC));
	GET(ParasiteClass* const, pThis, ESI);

	if (auto pWhExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead)) {
		if (auto const AnimType = pWhExt->SquidSplash.GetElements(RulesClass::Instance->SplashList)) {
			if (auto pSplashType = AnimType.at(ScenarioClass::Instance->Random(0, (AnimType.size() - 1)))) {
				if (auto pAnim = GameCreate<AnimClass>(pSplashType, *pCoord)) {
					auto const Invoker = (pThis->Owner) ? pThis->Owner->GetOwningHouse() : pThis->GetOwningHouse();
					AnimExt::SetAnimOwnerHouseKind(pAnim, Invoker, (pThis->Victim) ? pThis->Victim->GetOwningHouse() : nullptr, pThis->Owner, false);
					return Handled;
				}
			}
		}
	}

	return Continue;
}

