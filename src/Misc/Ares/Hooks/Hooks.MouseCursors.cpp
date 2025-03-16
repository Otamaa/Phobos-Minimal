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

#include "Hooks.MouseCursors.h"

ASMJIT_PATCH(0x653CA6, RadarClass_GetMouseAction_AllowMinimap, 5)
{
	GET(int, nAction, EAX);
	enum { AllowMini = 0x653CC0  , DisAllowMini = 0x653CBA , ReConsiderAllowMini = 0x653CAB };
	// + 1 because it get negative one and leaed
	if (const MouseCursor* pCursor = MouseClassExt::GetCursorDataFromRawAction(Action(nAction + 1))) {
		return pCursor->SmallFrame >= 0 ? AllowMini : DisAllowMini;
	}

	return nAction > (int)Action::PsychicReveal ? DisAllowMini : ReConsiderAllowMini;
}

void MouseHooker::Exec()
{
	//Patch_Jump(0x5BDDC0, &MouseClassExt::_Update);
	//Patch_Jump(0x5BDC80, &MouseClassExt::_Override_Mouse_Shape);
	//Patch_Vtable(0x7E19B8, &MouseClassExt::_Mouse_Small);
	//Patch_Jump(0x5BDAB0, &MouseClassExt::_Mouse_Small);
	//Patch_Jump(0x5BDBC0, &MouseClassExt::_Get_Mouse_Current_Frame);
	//Patch_Jump(0x5BDB90, &MouseClassExt::_Get_Mouse_Frame);
	//Patch_Jump(0x5BDC00, &MouseClassExt::_Get_Mouse_Hotspot);
	//Patch_Jump(0x5BE970, &MouseClassExt::_Get_Mouse_Start_Frame);
	//Patch_Jump(0x5BE990, &MouseClassExt::_Get_Mouse_Frame_Count);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5BDDC0, MouseClassExt::_Update);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E198C, MouseClassExt::_Update);

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E19B0, MouseClassExt::_Override_Mouse_Shape);
DEFINE_FUNCTION_JUMP(LJMP, 0x5BDC80, MouseClassExt::_Override_Mouse_Shape);

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E19B8, MouseClassExt::_Mouse_Small);
DEFINE_FUNCTION_JUMP(LJMP, 0x5BDAB0, MouseClassExt::_Mouse_Small);

DEFINE_FUNCTION_JUMP(LJMP, 0x5BDBC0, MouseClassExt::_Get_Mouse_Current_Frame);
DEFINE_FUNCTION_JUMP(LJMP, 0x5BDB90, MouseClassExt::_Get_Mouse_Frame);
DEFINE_FUNCTION_JUMP(LJMP, 0x5BDC00, MouseClassExt::_Get_Mouse_Hotspot);
DEFINE_FUNCTION_JUMP(LJMP, 0x5BE970, MouseClassExt::_Get_Mouse_Start_Frame);
DEFINE_FUNCTION_JUMP(LJMP, 0x5BE990, MouseClassExt::_Get_Mouse_Frame_Count);

ASMJIT_PATCH(0x4AB35A, DisplayClass_SetAction_CustomCursor, 0x6)
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

		if (pTarget && nObjectVect.Count == 1 && (nObjectVect.Items[0]->AbstractFlags & AbstractFlags::Techno)) {
			const auto pTechno = static_cast<TechnoClass*>(nObjectVect[0]);
			const auto weaponidx = pTechno->SelectWeapon(pTarget);
			if(!pTechno->IsCloseEnough(pTarget, weaponidx)) {
				MouseCursorFuncs::SetMouseCursorAction(MouseClassExt::ByWeapon(pTechno, weaponidx, true), Action::Attack, false);
			}
		} else {
				nAction = Action::Harvest;
		}
	}

	pThis->SetCursor(MouseClassExt::ValidateCursorType(nAction), bMini);
	return 0x4AB78F;
}

//void DrawMouseShape(WWMouseClass* pMouse, Surface* pSurface, Point2D offs)
//{
//	if (pMouse->Image && pSurface && FileSystem::MOUSE_PAL())
//	{
//		int width = pMouse->MouseBuffRect.Width;
//		int height = pMouse->MouseBuffRect.Height;
//		int x = offs.X + pMouse->MouseBuffRect.X;
//		int Y = offs.Y + pMouse->MouseBuffRect.Y;
//		auto imageframe = pMouse->Image->GetPixels(pMouse->ImageFrameIndex);
//		BSurface surf {width , height , 1 , (void*)imageframe};
//
//		if(pSurface == pMouse->sc)
//	}
//}

ASMJIT_PATCH(0x6929FC, DisplayClass_ChooseAction_CanSell, 7)
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
ASMJIT_PATCH(0x70055D, TechnoClass_GetActionOnObject_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, nWeapon, STACK_OFFS(0x1C, 0x8));

	const auto nCursor = MouseClassExt::ByWeapon(pThis, nWeapon, false);
	MouseCursorFuncs::SetMouseCursorAction(nCursor, Action::Attack, false);
	return 0;
}

 //WeaponCursor
ASMJIT_PATCH(0x700AA8, TechnoClass_GetActionOnCell_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nWeapon, EBP);
	const auto nCursor = MouseClassExt::ByWeapon(pThis, nWeapon, false);
	MouseCursorFuncs::SetMouseCursorAction(nCursor, Action::Attack, false);
	return 0;
}

ASMJIT_PATCH(0x700600, TechnoClass_GetActionOnCell_Cursors, 5)
{
	GET(TechnoClass*, pThis, ECX);
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// Cursor Move
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Move.Get(), Action::Move, false);

	// Cursor NoMove
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_NoMove.Get(), Action::NoMove, false);
	return 0;
}

ASMJIT_PATCH(0x7000CD, TechnoClass_GetActionOnObject_SelfDeployCursor, 6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	//Cursor Deploy
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::AreaAttack, false);
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::Self_Deploy, false);
	//

	//Cursor NoDeploy
	MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_NoDeploy.Get(), Action::NoDeploy, false);
	return 0;
}

ASMJIT_PATCH(0x7400F0, UnitClass_GetActionOnObject_SelfDeployCursor_Bunker, 6)
{
	GET(UnitClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pThis->BunkerLinkedItem) {
		//Cursor Deploy
		MouseCursorFuncs::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::Self_Deploy, false);
		return 0x73FFE6;
	}

	return pThis->Type->DeployFire ? 0x7400FA : 0x740115;
}
