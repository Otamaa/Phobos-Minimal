#include "Body.h"


// Note:
/*
Ares has a hook at 0x4571E0 (the beginning of BuildingClass::Infiltrate) and completely overwrites the function.
Our logic has to be executed at the end (0x4575A2). The hook there assumes that registers have the exact content
they had in the beginning (when Ares hook started, executed, and jumped) in order to work when Ares logic is used.
However, this will fail if Ares is not involved (either DLL not included or with SpyEffect.Custom=no on BuildingType),
because by the time we reach our hook, the registers will be different and we'll be reading garbage. That's why
there is a second hook at 0x45759D, which is only executed when Ares doesn't jump over this function. There,
we execute our custom logic and then use EAX (which isn't used later, so it's safe to write to it) to "mark"
that we're done with 0x77777777. This way, when we reach the other hook, we check for this very specific value
to prevent spy effects from happening twice.
The value itself doesn't matter, it just needs to be unique enough to not be accidentally produced by the game there.
*/

//#define INFILTRATE_HOOK_MAGIC 0x77777777
//DEFINE_HOOK(0x45759D, BuildingClass_Infiltrate_NoAres, 0x5)
//{
//	GET_STACK(HouseClass*, pInfiltratorHouse, STACK_OFFSET(0x14, -0x4));
//	GET(BuildingClass*, pBuilding, EBP);
//
//	BuildingExt::HandleInfiltrate(pBuilding, pInfiltratorHouse);
//	R->EAX<int>(INFILTRATE_HOOK_MAGIC);
//	return 0;
//}
//
//DEFINE_HOOK(0x4575A2, BuildingClass_Infiltrate_AfterAres, 0xE)
//{
//	 Check if we've handled it already
//	if (R->EAX<int>() == INFILTRATE_HOOK_MAGIC)
//	{
//		R->EAX<int>(0);
//		return 0;
//	}
//
//	GET_STACK(HouseClass*, pInfiltratorHouse, -0x4);
//	GET(BuildingClass*, pBuilding, ECX);
//
//	BuildingExt::HandleInfiltrate(pBuilding, pInfiltratorHouse);
//	return 0;
//}
//
//#undef INFILTRATE_HOOK_MAGIC
//
//DEFINE_HOOK(0x51A002, InfantryClass_PCP_InfitrateBuilding, 0x6)
//{
//	GET(InfantryClass*, pThis, ESI);
//	GET(BuildingClass*, pBuilding, EDI);
//
//	auto const pHouse = pThis->Owner;
//	pBuilding->Infiltrate(pHouse);
//
//	//if (
//	BuildingExt::HandleInfiltrate(pBuilding, pHouse)
//		//	)
//			//Debug::Log("Phobos CustomSpy Affect Return True ! \n")
//		;
//
//	return 0x51A010;
//}