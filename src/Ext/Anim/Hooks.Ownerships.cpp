#include "Body.h"
#include <Utilities/Macro.h>

DEFINE_HOOK(0x423991, AnimClass_BounceAI_BounceAnim, 0xA)
{
	GET(AnimTypeClass*, pBounceAnim, ECX);
	GET(AnimClass*, pThis, EBP);

	HouseClass* pHouse = nullptr;
	TechnoClass* pObject = nullptr;

	if (const auto pTypeExt = AnimTypeExt::ExtMap.Find(pBounceAnim))
	{
		pObject = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
		pHouse = pThis->Owner ? pThis->Owner : ((pObject) ? pObject->GetOwningHouse() : pHouse);
	}

	auto nCoord = pThis->GetCenterCoord();
	if (auto pAnim = GameCreate<AnimClass>(pBounceAnim, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
	{
		AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, nullptr, pObject, false);
	}

	return 0x4239D3;
}

// Bruh ,..
DEFINE_JUMP(VTABLE, 0x7E3390, GET_OFFSET(AnimExt::GetOwningHouse_Wrapper));

#ifdef ENABLE_NEWHOOKS
//these cause desync , wtf
DEFINE_HOOK(0x4242F4, AnimClass_Trail_Override, 0x4)
{
	GET(AnimClass*, pMem, EDI);
	GET(AnimClass*, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x8C, 0x4C));

	GameConstruct(pMem, pThis->Type->TrailerAnim, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	{
		if (const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
		{
			const auto pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
			const auto pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
			AnimExt::SetAnimOwnerHouseKind(pMem, pOwner, nullptr, pTech, false);
		}
	}

	return 0x424322;
}

DEFINE_HOOK(0x423F9D, AnimClass_Spawns_Override, 0x6)
{
	GET(AnimClass*, pMem, EAX);
	GET(AnimClass*, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x8C, 0x4C));
	const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	GameConstruct(pMem, pThis->Type->Spawns, nCoord, pAnimTypeExt ? pAnimTypeExt->Spawns_Delay.Get() : 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	{
		if (pAnimTypeExt)
		{
			const auto pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
			const auto pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;

			AnimExt::SetAnimOwnerHouseKind(pMem, pOwner, nullptr, pTech, false);
		}
	}

	return 0x423FC3;
}
#endif