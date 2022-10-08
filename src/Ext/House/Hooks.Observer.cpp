#include "Body.h"

#include <Utilities/Macro.h>


//DEFINE_HOOK(0x6A557A, SidebarClass_Init_EnableSkirmish, 0x5) //changed
//{
//	auto const nMode = SessionClass::Instance->GameMode;
//	return (nMode == GameMode::Skirmish || nMode == GameMode::Internet || nMode == GameMode::LAN) ? 0x6A558D : 0x6A5830;
//}
//DEFINE_HOOK(0x68811C, AssignHouse_SetObserver, 0x6)
//{
//	GET(HouseClass*, pCur, EBP);
//	HouseExt::ExtMap.Find(pCur)->IsObserver = true;
//	return 0x0;
//}
//
//DEFINE_HOOK(0x4870D0, CellClass_SensedByHouses_ObserverAlwaysSensed, 0x6)
//{
//	GET_STACK(int, nHouseIdx, 0x4);
//
//	const auto pHouse = HouseClass::Array->GetItemOrDefault(nHouseIdx);
//	if (HouseExt::IsObserverPlayer(pHouse))
//	{
//		R->AL(1);
//		return 0x4870DE;
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x70DA6D, TechnoClass_SensorAI_ObserverSkipWarn, 0x6)
//{
//	return HouseExt::IsObserverPlayer() ? 0x70DADC : 0x0;
//}
//
// Disabled , it only valid if only singe observer on the game
//DEFINE_HOOK(0x4FC585, HouseClass_MPlayerDefeated_3, 0x6)
//{
//	REF_STACK(int, nHuman, 0x18);
//
//	for (auto pHouse : *HouseClass::Array)
//	{
//		nHuman += pHouse->IsHumanPlayer && HouseExt::IsObserverPlayer(pHouse);
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x4FC34B, HouseClass_MPlayerDefeated_2, 0xA)
//{
//	GET(HouseClass*, pThis, ESI);
//	return !HouseExt::IsObserverPlayer(pThis) ? 0x4FC34F : 0x4FC3C1;
//}
//
//DEFINE_HOOK(0x658393, RadarClass_658330, 0x9)
//{
//	GET(HouseClass*, pHouses, EBX);
//	return !HouseExt::IsObserverPlayer(pHouses) ? 0x6583A8: 0x658397;
//}
//
//DEFINE_HOOK(0x658478, RadarClass_658330_2, 0x6)
//{
//	GET(HouseClass*, pHouses, EBX);
//	return !HouseExt::IsObserverPlayer(pHouses) ? 0x65848A : 0x658480;
//}
//
//DEFINE_HOOK(0x657EE3, RadarClass_DiplomacyDialog, 0x6)
//{
//	return !HouseExt::IsObserverPlayer() ? 0x657F70 : 0x657EF2;
//}
//
//DEFINE_HOOK(0x4FCD88, HouseClass_FlagToLose, 0x5)
//{
//	return HouseExt::IsObserverPlayer() ?
//		0x4FCDA6 : 0x4FCD97;
//}
//
//DEFINE_HOOK(0x4FC262, HouseClass_MPlayerDefeated, 0x6)
//{
//	return HouseExt::IsObserverPlayer()
//		? 0x4FC2EF : 0x4FC271;
//}
//
//remove ISHuman check
//DEFINE_JUMP(LJMP, 0x6A55BF, 0x6A55C8)
//DEFINE_HOOK(0x6A55B7, SidebarClass_Init_CheckObserver, 0x6)
//{
//	GET(HouseClass*, pOtherHouse, EAX);
//
//	return HouseExt::IsObserverPlayer(pOtherHouse) ?  0x6A55CF :0x6A55C8;
//}
//
//DEFINE_JUMP(LJMP, 0x6A57F6, 0x6A57FF)
//DEFINE_HOOK(0x6A57EE, SidebarClass_Init_CheckObserverB, 0x6)
//{
//	GET(HouseClass*, pOtherHouse, EAX);
//
//	return !HouseExt::IsObserverPlayer(pOtherHouse) ? 0x6A57FF : 0x6A580E;
//}
//
//DEFINE_HOOK(0x4A23A8, CreditClass_GraphicLogic_CheckObserver, 0x8)
//{
//	GET(HouseClass*, pOtherHouse, ECX);
//	return !HouseExt::IsObserverPlayer(pOtherHouse) ? 0x4A24F4 : 0x4A23B0;
//}
//
//DEFINE_HOOK(0x4A2614, CreditClass_AI_CheckObserver, 0x7)
//{
//	return !HouseExt::IsObserverPlayer() ? 0x4A267D : 0x4A261D;
//}
////
//DEFINE_HOOK(0x5C98E5, Multiplayer_Score_CheckObserver, 0x6)
//{
//	GET(HouseClass*, pOtherHouse, EDI);
//	return !HouseExt::IsObserverPlayer(pOtherHouse) ? 0x5C98F1 : 0x5C9A7E;
//}

