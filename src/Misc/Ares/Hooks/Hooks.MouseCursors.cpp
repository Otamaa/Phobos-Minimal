#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/SWType/Body.h>

#include <Misc/Ares/Hooks/Header.h>

#include <New/Type/CursorTypeClass.h>

#ifndef Dum
//DEFINE_DISABLE_HOOK(0x653CA6, RadarClass_GetMouseAction_AllowMinimap_ares) //, 5)
DEFINE_DISABLE_HOOK(0x5BDDC0, MouseClass_Update_Reset_ares) //, 5)
DEFINE_DISABLE_HOOK(0x4AB35A, DisplayClass_SetAction_CustomCursor_ares) //, 6)
DEFINE_DISABLE_HOOK(0x5BDC8C, MouseClass_UpdateCursor_ares) //, 7
DEFINE_DISABLE_HOOK(0x5BDADF, MouseClass_UpdateCursorMinimapState_UseCursor_ares) //, 0
DEFINE_DISABLE_HOOK(0x5BDDC8, MouseClass_Update_AnimateCursor_ares) //, 6
DEFINE_DISABLE_HOOK(0x5BDE64, MouseClass_Update_AnimateCursor2_ares) //, 6
DEFINE_DISABLE_HOOK(0x5BDB90, MouseClass_GetCursorFirstFrame_Minimap_ares) //, B
DEFINE_DISABLE_HOOK(0x5BE974, MouseClass_GetCursorFirstFrame_ares) //, 7
DEFINE_DISABLE_HOOK(0x5BE994, MouseClass_GetCursorFrameCount_ares) //, 7
DEFINE_DISABLE_HOOK(0x5BDBC4, MouseClass_GetCursorCurrentFrame_ares) //, 7
DEFINE_DISABLE_HOOK(0x5BDC1B, MouseClass_GetCursorHotSpot_ares) //, 7

DEFINE_OVERRIDE_HOOK(0x653CA6, RadarClass_GetMouseAction_AllowMinimap, 5)
{
	GET(int, nAction, EAX);
	enum { AllowMini = 0x653CC0  , DisAllowMini = 0x653CBA , ReConsiderAllowMini = 0x653CAB };
	// + 1 because it get negative one and leaed
	if (const MouseCursor* pCursor = MouseClassExt::GetCursorDataFromRawAction(Action(nAction + 1))) {
		return pCursor->SmallFrame >= 0 ? AllowMini : DisAllowMini;
	}

	return nAction > (int)Action::PsychicReveal ? DisAllowMini : ReConsiderAllowMini;
}

DEFINE_HOOK(0x5BDDC0, MouseClass_Update_Replace, 0x5)
{
	GET(MouseClassExt*, pMouse, ECX);
	GET_STACK(const  int*, keyCode, 0x4);
	GET_STACK(const Point2D*, mouseCoords, 0x8);

	pMouse->_Update(keyCode, mouseCoords);
	return 0x5BDF2A;
}

DEFINE_HOOK(0x5BDC80, MouseClass_Override_MouseShape_Replace, 0x7)
{
	GET(MouseClassExt*, pMouse, ECX);
	GET_STACK(MouseCursorType, mouse, 0x4);
	GET_STACK(bool, wsmall, 0x8);

	R->AL(pMouse->_Override_Mouse_Shape(mouse, wsmall));
	return 0x5BDDB3;
}

DEFINE_HOOK(0x5BDAB0, MouseClass_Small_Replace, 0x7)
{
	GET(MouseClassExt*, pMouse, ECX);
	GET_STACK(bool, wsmall, 0x4);

	pMouse->_Mouse_Small(wsmall);
	return 0x5BDB82;

}

DEFINE_HOOK(0x5BDBC0, MouseClass_Get_Mouse_Current_Frame_Replace, 0xB)
{
	GET(MouseClassExt*, pMouse, ECX);
	GET_STACK(MouseCursorType, mouse, 0x4);
	GET_STACK(bool, wsmall, 0x8);

	R->EAX(pMouse->_Get_Mouse_Current_Frame(mouse, wsmall));
	return 0x5BDBEE;
}

DEFINE_HOOK(0x5BDB90, MouseClass_Get_Mouse_Frame_Replace, 0xB)
{
	GET(MouseClassExt*, pMouse, ECX);
	GET_STACK(MouseCursorType, mouse, 0x4);
	GET_STACK(bool, wsmall, 0x8);

	R->EAX(pMouse->_Get_Mouse_Frame(mouse, wsmall));
	return 0x5BDBB6;
}

DEFINE_HOOK(0x5BDC00, MouseClass_Get_Mouse_Hotspot_Replace, 0x5)
{
	GET(MouseClassExt*, pMouse, ECX);
	GET_STACK(Point2D*, pRet, 0x4);
	GET_STACK(MouseCursorType, mouse, 0x8);

	const auto nResult = pMouse->_Get_Mouse_Hotspot(mouse);
	pRet->X = nResult.X;
	pRet->Y = nResult.Y;
	R->EAX(pRet);
	return 0x5BDC77;
}

DEFINE_HOOK(0x5BE970, MouseClass_Get_Mouse_Start_Frame_Replace, 0xB)
{
	GET(MouseClassExt*, pMouse, ECX);
	GET_STACK(MouseCursorType, mouse, 0x4);

	R->EAX(pMouse->_Get_Mouse_Start_Frame(mouse));
	return 0x5BE984;
}

DEFINE_HOOK(0x5BE990, MouseClass_Get_Mouse_Frame_Count_Replace, 0xB)
{
	GET(MouseClassExt*, pMouse, ECX);
	GET_STACK(MouseCursorType, mouse, 0x4);
	R->EAX(pMouse->_Get_Mouse_Frame_Count(mouse));
	return 0x5BE9A4;
}

DEFINE_OVERRIDE_HOOK(0x4AB35A, DisplayClass_SetAction_CustomCursor, 0x6)
{
	GET(DisplayClass*, pThis, ESI);
	GET(Action, nAction, EAX);
	GET_STACK(bool, bMini, STACK_OFFS(0x20, -0x14));//0x34
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, 0x8)); //0x18
	GET_STACK(bool, bIsShrouded, 0x28);

	if (bIsShrouded) {
		nAction = MouseClassExt::ValidateShroudedAction(nAction);
	}

	if (nAction == Action::Attack) {
		// WeaponCursor
		// doesnt work with `Deployed` techno
		auto const& nObjectVect = ObjectClass::CurrentObjects();

		if (pTarget && nObjectVect.Count == 1 && (nObjectVect[0]->AbstractFlags & AbstractFlags::Techno)) {
			if (!static_cast<TechnoClass*>(nObjectVect[0])->IsCloseEnoughToAttack(pTarget)) {
					//if too far from the target
					MouseClassExt::InsertMappedAction(
						MouseCursorType::NoEnter
						, Action::Attack, false);
			}

		} else {
				nAction = Action::Harvest;
		}
	}

	pThis->SetCursor(MouseClassExt::ValidateCursorType(nAction), bMini);
	return 0x4AB78F;
}
#endif

DEFINE_OVERRIDE_HOOK(0x6929FC, DisplayClass_ChooseAction_CanSell, 7)
{
	GET(TechnoClass*, Target, ESI);
	switch (Target->WhatAmI())
	{
	case AbstractType::Aircraft:
	case AbstractType::Unit:
		R->Stack(0x10, Action::SellUnit);
		return 0x692B06;
	case AbstractType::Building:
		R->Stack(0x10, Target->IsStrange() ? Action::NoSell : Action::Sell);
		return 0x692B06;
	default:
		return 0x692AFE;
	}
}

// WeaponCursor
DEFINE_OVERRIDE_HOOK(0x70055D, TechnoClass_GetActionOnObject_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, nWeapon, STACK_OFFS(0x1C, 0x8));

	const auto nCursor = MouseClassExt::ByWeapon(pThis, nWeapon, false);
	MouseCursorFuncs::SetMouseCursorAction(nCursor, Action::Attack, false);
	return 0;
}

 //WeaponCursor
DEFINE_OVERRIDE_HOOK(0x700AA8, TechnoClass_GetActionOnCell_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nWeapon, EBP);
	const auto nCursor = MouseClassExt::ByWeapon(pThis, nWeapon, false);
	MouseCursorFuncs::SetMouseCursorAction(nCursor, Action::Attack, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x700600, TechnoClass_GetActionOnCell_Cursors, 5)
{
	GET(TechnoClass*, pThis, ECX);
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	// Cursor Move
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Move.Get(), Action::Move, false);

	// Cursor NoMove
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_NoMove.Get(), Action::NoMove, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7000CD, TechnoClass_GetActionOnObject_SelfDeployCursor, 6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	//Cursor Deploy
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::AreaAttack, false);
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::Self_Deploy, false);
	//

	//Cursor NoDeploy
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_NoDeploy.Get(), Action::NoDeploy, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7400F0, UnitClass_GetActionOnObject_SelfDeployCursor_Bunker, 6)
{
	GET(UnitClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->BunkerLinkedItem) {
		//Cursor Deploy
		MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::Self_Deploy, false);
		return 0x73FFE6;
	}

	return pThis->Type->DeployFire ? 0x7400FA : 0x740115;
}
