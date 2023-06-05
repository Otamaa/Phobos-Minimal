#include <Ext/Techno/Body.h>

//; ====
//; Show disguised units(Spy and Mirage) for observer
//; ====

//DEFINE_HOOK(0x51F2F3 ,InfantryClass_GetFullName_Observer , 0x6)
//{
//	enum { Show = 0x51F31A, Continue = 0x0 };
//
//	return HouseClass::IsCurrentPlayerObserver() ? 
//		Show : Continue;
//}
//
//DEFINE_HOOK(0x4DEDC3, FootClass_GetImageData_Observer, 0x6)
//{
//	enum { Show = 0x4DEE15, Continue = 0x0 };
//
//	return HouseClass::IsCurrentPlayerObserver() ?
//		Show : Continue;
//}
//
//DEFINE_HOOK(0x457188, BuildingClass_Radar_Cloak_Observer_A, 0xA)
//{
//	enum { Show = 0x4571C4, Continue = 0x0 };
//
//	GET(TechnoClass* const, pTechno, EDI);
//	const auto pCurPlayer = HouseClass::CurrentPlayer();
//
//	if (pCurPlayer && pCurPlayer->IsObserver())
//		return Show;
//
//	if (pTechno->Owner && pTechno->Owner->IsAlliedWith_(pCurPlayer))
//		return Show;
//
//	return Continue;
//}
//
//DEFINE_HOOK(0x70D386, BuildingClass_Radar_Cloak_Observer_B, 0xA)
//{
//	enum { Show = 0x70D3CD, Continue = 0x0, };
//
//	GET(TechnoClass* const, pTechno, ESI);
//	const auto pCurPlayer = HouseClass::CurrentPlayer();
//
//	if (pCurPlayer && pCurPlayer->IsObserver())
//		return Show;
//
//	if (pTechno->Owner && pTechno->Owner->IsAlliedWith_(pCurPlayer))
//		return Show;
//
//	return Continue;
//}
//
//DEFINE_HOOK(0x4AE62B, DisplayClass_HelpText_CheckCloak_Observer, 0x5)
//{
//	enum { CheckInvisible = 0x4AE654, SetpCurPlayer = 0x4AE630, Continue = 0x0	};
//
//	GET(TechnoClass* const, pTechno, ECX);
//	const auto pCurPlayer = HouseClass::CurrentPlayer();
//
//	if (pCurPlayer && pCurPlayer->IsObserver())
//		return CheckInvisible;
//
//	if (pTechno->Owner && pTechno->Owner->IsAlliedWith_(pCurPlayer))
//		return CheckInvisible;
//
//	return Continue;
//}
//
//DEFINE_HOOK(0x4ABE3C, DisplayClass_Mouse_Left_Release_Cloak_Observer, 0xA)
//{
//	enum { Select = 0x4ABE4A, Continue = 0x0 };
//
//	GET(TechnoClass* const, pTechno, ESI);
//	const auto pCurPlayer = HouseClass::CurrentPlayer();
//
//	if (pCurPlayer && pCurPlayer->IsObserver())
//		return Select;
//
//	if (pTechno->Owner && pTechno->Owner->IsAlliedWith_(pCurPlayer))
//		return Select;
//
//	return Continue;
//}
//
//DEFINE_HOOK(0x6F4F10, TechnoClass_6F4EB0_Cloak_Observer, 0x5)
//{
//	enum { DonotUnselect = 0x6F4F3A, CheckSensed = 0x6F4F21 };
//
//	GET(TechnoClass* const, pTechno, ESI);
//	const auto pCurPlayer = HouseClass::CurrentPlayer();
//
//	if(!pCurPlayer)
//		return DonotUnselect;
//
//	if (pCurPlayer->IsObserver())
//		return DonotUnselect;
//
//	if (pCurPlayer == pTechno->Owner)
//		return DonotUnselect;
//
//	if (pTechno->Owner && pTechno->Owner->IsAlliedWith_(pCurPlayer))
//		return DonotUnselect;
//
//	R->EAX(pCurPlayer);
//	return CheckSensed;
//}
//
//DEFINE_HOOK(0x692540, ScrollClass_Coordthing_TechnoClass_Cloak_Observer, 0x5)
//{
//	enum { Allow =  0x69256B , Continue = 0x0};
//
//	GET(TechnoClass* const, pTechno, ESI);
//	const auto pCurPlayer = HouseClass::CurrentPlayer();
//
//	if (pCurPlayer && pCurPlayer->IsObserver())
//		return Allow;
//
//	if (pTechno->Owner && pTechno->Owner->IsAlliedWith_(pCurPlayer))
//		return Allow;
//
//	return Continue;
//}
//
//DEFINE_HOOK(0x6925AA, ScrollClass_Coordthing_BuildingClass_Cloak_Observer, 0x5)
//{
//	enum { Allow = 0x6925F0, Continue = 0x0 };
//
//	GET(TechnoClass* const, pTechno, ESI);
//	const auto pCurPlayer = HouseClass::CurrentPlayer();
//
//	if (pCurPlayer && pCurPlayer->IsObserver())
//		return Allow;
//
//	if (pTechno->Owner && pTechno->Owner->IsAlliedWith_(pCurPlayer))
//		return Allow;
//
//	return Continue;
//}
//
//DEFINE_HOOK(0x6DA412, Tactical_Select_At_Observer, 0x6)
//{
//	enum { Allow = 0x6DA43E, Continue = 0x0 };
//
//	GET(TechnoClass* const, pTechno, EAX);
//	const auto pCurPlayer = HouseClass::CurrentPlayer();
//
//	if (pCurPlayer && pCurPlayer->IsObserver())
//		return Allow;
//
//	if (pTechno->Owner && pTechno->Owner->IsAlliedWith_(pCurPlayer))
//		return Allow;
//
//	return Continue;
//}

#include <TriggerTypeClass.h>

//Fix crash 6F9DB6
//DEFINE_HOOK(0x727B3E, TriggerTypeClass_CalculateCRC_ValidateHouse, 0x6)
//{
//	GET(TriggerTypeClass* const, pThis, ESI);
//	return pThis->House ? 0x0 : 0x727B55;
//}

//DEFINE_HOOK(0x70AF66, TechnoClass_70AF50_Sight_ValidateHouse, 0x6)
//{
//	GET(TechnoClass* const, pThis, ESI);
//	return pThis->Owner ? 0x0 : 0x70B1C7;
//}

DEFINE_HOOK(0x70F820, TechnoClass_GetOriginalOwner_ValidateCaptureManager, 0x6)
{
	GET(TechnoClass* const, pThis, ECX);

	if (pThis->MindControlledBy && pThis->MindControlledBy->CaptureManager)
	{
		R->EAX(pThis->MindControlledBy);
		return 0x70F82A;
	}

	if (pThis->MindControlledByAUnit)
	{
		return 0x70F841;
	}

	return 0x70F837;
}

DEFINE_HOOK(0x65DC11, Do_Reinforcement_ValidateHouse, 0x6)
{
	GET(FootClass* const, pReinforcee, EBP);

	if (!pReinforcee->Owner)
	{
		R->EAX(Edge::North);
		return 0x65DC2B;
	}

	const Edge nEdge =
		pReinforcee->Owner->StartingEdge < Edge::North || pReinforcee->Owner->StartingEdge > Edge::West
		? pReinforcee->Owner->GetHouseEdge() : pReinforcee->Owner->StartingEdge;

	R->EAX(nEdge);
	return 0x65DC2B;
}

//DEFINE_HOOK(0x6F49CA, TechnoClass_Reveal_ValidateHouse, 0x6)
//{
//	GET(TechnoClass* const, pThis, ESI);
//	return pThis->Owner ? 0x0 : 0x6F4A31;
//}

//DEFINE_HOOK(0x5F5895, ObjectClass_Mark_SkipDiscardedVtableCalls, 0x6) {
//	return 0x5F58E1;
//}

//DEFINE_HOOK(0x447563, BuildingClass_WhatAction_SellExploitFix , 0x6)
//{
//	enum { ActionNoneRet = 0x447558 , Continue =  0x0};
//	GET(BuildingClass* const, pThis, ESI);
//
//	if (!pThis->CanBeSold() && pThis->Type->ConstructionYard) {
//		return ActionNoneRet;
//	}
//
//	return Continue;
//}