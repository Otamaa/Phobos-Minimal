#include "Body.h"
#include <Utilities/Macro.h>

//TODO : retest for desync
DEFINE_HOOK(0x423991, AnimClass_BounceAI_BounceAnim, 0x5)
{
	GET(AnimTypeClass*, pBounceAnim, ECX);
	GET(AnimClass*, pThis, EBP);

	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pBounceAnim);
	TechnoClass* pObject = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
	HouseClass* pHouse = pThis->Owner ? pThis->Owner : ((pObject) ? pObject->GetOwningHouse() : nullptr);

	auto nCoord = pThis->GetCoords();
	if (auto pAnim = GameCreate<AnimClass>(pBounceAnim, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
	{
		AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, nullptr, pObject, false);
	}

	return 0x4239D3;
}

//TODO : retest for desync
#ifdef COMPILE_PORTED_DP_FEATURES
DEFINE_HOOK(0x423F8A, AnimClass_Spawns_Override, 0x7)
{
	GET(const AnimClass* const, pThis, ESI);
	GET(int, nVal, EAX);
	GET_STACK(const CoordStruct, nCoord, 0x4C);

	const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	TechnoClass* pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
	HouseClass* pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
	const int nDelay = pAnimTypeExt->Spawns_Delay.Get();

	//auto nCoord = pThis->Bounce.GetCoords();
	for (int i = nVal; i > 0; --i) {
		if(auto pMem = GameCreate<AnimClass>(pThis->Type->Spawns, nCoord, nDelay, 1, 0x600 , 0, false)) {
			AnimExt::SetAnimOwnerHouseKind(pMem, pOwner, nullptr, pTech, false);
		}
	}

	return 0x423FC6;
}
#endif

// Bruh ,..
DEFINE_JUMP(VTABLE, 0x7E3390, GET_OFFSET(AnimExt::GetOwningHouse_Wrapper));