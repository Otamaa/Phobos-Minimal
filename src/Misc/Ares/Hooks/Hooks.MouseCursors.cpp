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

/*

// Helper for shadow mode cursor selection
void DisplayClass::SetMouseForAction_WithObject(Action finalAction, int mouse, ObjectClass* pObject)
{
	switch (finalAction)
	{
	case Action::Move:
	case Action::Attack:
		this->Set_Default_Mouse(MouseCursorType::Move, mouse);
		break;

	case Action::NoMove:
		if (CurrentObject.Count > 0)
		{
			auto const pFirst = CurrentObject[0];
			if (pFirst && (pFirst->AbstractFlags & AbstractFlags::Techno) &&
				pFirst->GetTechnoType()->MoveToShroud)
			{
				this->Set_Default_Mouse(MouseCursorType::Move, mouse);
				return;
			}
		}
		this->Set_Default_Mouse(MouseCursorType::NoMove, mouse);
		break;

	case Action::Eaten:
	case Action::NoRepair:
	case Action::NoGRepair:
		this->Set_Default_Mouse(MouseCursorType::NoRepair, mouse);
		break;

	case Action::Sell:
	case Action::SellUnit:
	case Action::NoSell:
		this->Set_Default_Mouse(MouseCursorType::NoSell, mouse);
		break;

	case Action::Tote:
	case Action::Heal:
	case Action::TogglePower:
	case Action::NoTogglePower:
	case Action::PlaceWaypoint:
	case Action::TibSunBug:
	case Action::EnterWaypointMode:
	case Action::FollowWaypoint:
	case Action::SelectWaypoint:
	case Action::LoopWaypointPath:
	case Action::AttackWaypoint:
	case Action::PatrolWaypoint:
		this->Set_Default_Mouse(MouseCursorType::Waypoint, mouse);
		break;

	case Action::Nuke:
		this->Set_Default_Mouse(MouseCursorType::Nuke, mouse);
		break;

	case Action::GuardArea:
		this->Set_Default_Mouse(MouseType_AreaGuard(), mouse);
		break;

	case Action::NoDeploy:
		this->Set_Default_Mouse(MouseCursorType::NoDeploy, mouse);
		break;

	case Action::NoEnter:
	case Action::NoEnterTunnel:
		this->Set_Default_Mouse(MouseCursorType::NoEnter, mouse);
		break;

	case Action::IronCurtain:
		this->Set_Default_Mouse(MouseCursorType::IronCurtain, mouse);
		break;

	case Action::LightningStorm:
		this->Set_Default_Mouse(MouseCursorType::LightningStorm, mouse);
		break;

	case Action::ChronoSphere:
	case Action::ChronoWarp:
		this->Set_Default_Mouse(MouseCursorType::Chronosphere, mouse);
		break;

	case Action::ParaDrop:
	case Action::AmerParaDrop:
		this->Set_Default_Mouse(MouseCursorType::ParaDrop, mouse);
		break;

	case Action::EnterWaypoint:
		this->Set_Default_Mouse(MouseCursorType::Waypoint, mouse);
		this->Set_Default_Mouse(MouseCursorType::Beacon, mouse);
		break;

	case Action::DisarmBomb:
		this->Set_Default_Mouse(MouseCursorType::Disarm, mouse);
		break;

	case Action::PlaceBeacon:
		this->Set_Default_Mouse(MouseCursorType::Beacon, mouse);
		break;

	case Action::SelectBeacon:
		this->Set_Default_Mouse(MouseCursorType::Select, mouse);
		break;

	case Action::AttackMoveNav:
	case Action::AttackMoveTar:
		this->Set_Default_Mouse(cursor_731CB0(), mouse);
		break;

	case Action::PsychicDominator:
		this->Set_Default_Mouse(MouseCursorType::PsychicDominator, mouse);
		break;

	case Action::SpyPlane:
		this->Set_Default_Mouse(MouseCursorType::SpyPlane, mouse);
		break;

	case Action::GeneticConverter:
		this->Set_Default_Mouse(MouseCursorType::GeneticMutator, mouse);
		break;

	case Action::ForceShield:
		this->Set_Default_Mouse(MouseCursorType::ForceShield, mouse);
		break;

	case Action::NoForceShield:
		this->Set_Default_Mouse(MouseCursorType::NoForceShield, mouse);
		break;

	case Action::PsychicReveal:
		this->Set_Default_Mouse(MouseCursorType::PsychicReveal, mouse);
		break;

	default:
		this->Set_Default_Mouse(MouseCursorType::Default, mouse);
		break;
	}
}

// Helper for no-shadow mode cursor selection
void DisplayClass::SetMouseForAction_NoObject(Action finalAction, int mouse, int waypointPathIndex)
{
	switch (finalAction)
	{
	case Action::Move:
		this->Set_Default_Mouse(MouseCursorType::Move, mouse);
		break;

	case Action::NoMove:
	case Action::NoIvanBomb:
		this->Set_Default_Mouse(MouseCursorType::NoMove, mouse);
		break;

	case Action::Enter:
	case Action::Capture:
	case Action::Repair:
	case Action::EnterTunnel:
		this->Set_Default_Mouse(MouseCursorType::Enter, mouse);
		break;

	case Action::Self_Deploy:
	case Action::AreaAttack:
		this->Set_Default_Mouse(MouseCursorType::Deploy, mouse);
		break;

	case Action::Attack:
	{
		// Check if in range for attack cursor
		if (waypointPathIndex && CurrentObject.Count == 1)
		{
			auto const pFirst = CurrentObject[0];
			if (pFirst && (pFirst->AbstractFlags & AbstractFlags::Techno))
			{
				if (pFirst->In_Range(waypointPathIndex))
				{
					this->Set_Default_Mouse(MouseCursorType::Attack, mouse);
					return;
				}
			}
		}
		this->Set_Default_Mouse(MouseCursorType::AttackOutOfRange, mouse);
		break;
	}

	case Action::Harvest:
		this->Set_Default_Mouse(MouseCursorType::AttackOutOfRange, mouse);
		break;

	case Action::Select:
	case Action::ToggleSelect:
	case Action::SelectBeacon:
		this->Set_Default_Mouse(MouseCursorType::Select, mouse);
		break;

	case Action::Eaten:
		this->Set_Default_Mouse(MouseCursorType::Demolish, mouse);
		break;

	case Action::Sell:
		this->Set_Default_Mouse(MouseCursorType::Sell, mouse);
		break;

	case Action::SellUnit:
		this->Set_Default_Mouse(MouseCursorType::SellUnit, mouse);
		break;

	case Action::NoSell:
		this->Set_Default_Mouse(MouseCursorType::NoSell, mouse);
		break;

	case Action::NoRepair:
	case Action::NoGRepair:
		this->Set_Default_Mouse(MouseCursorType::NoRepair, mouse);
		break;

	case Action::Sabotage:
	case Action::Detonate:
	case Action::DetonateAll:
	case Action::Demolish:
		this->Set_Default_Mouse(MouseCursorType::Demolish, mouse);
		break;

	case Action::Tote:
	case Action::Heal:
	case Action::TogglePower:
	case Action::NoTogglePower:
	case Action::PlaceWaypoint:
	case Action::TibSunBug:
	case Action::EnterWaypointMode:
	case Action::FollowWaypoint:
	case Action::SelectWaypoint:
	case Action::LoopWaypointPath:
	case Action::AttackWaypoint:
	case Action::EnterWaypoint:
	case Action::PatrolWaypoint:
		this->Set_Default_Mouse(MouseCursorType::Waypoint, mouse);
		break;

	case Action::Nuke:
		this->Set_Default_Mouse(MouseCursorType::Nuke, mouse);
		break;

	case Action::GuardArea:
		this->Set_Default_Mouse(MouseType_AreaGuard(), mouse);
		break;

	case Action::Damage:
		// No cursor change
		return;

	case Action::GRepair:
		this->Set_Default_Mouse(MouseCursorType::EngineerRepair, mouse);
		break;

	case Action::NoDeploy:
		this->Set_Default_Mouse(MouseCursorType::NoDeploy, mouse);
		break;

	case Action::NoEnter:
		this->Set_Default_Mouse(MouseCursorType::NoEnter, mouse);
		break;

	case Action::IronCurtain:
		this->Set_Default_Mouse(MouseCursorType::IronCurtain, mouse);
		break;

	case Action::LightningStorm:
		this->Set_Default_Mouse(MouseCursorType::LightningStorm, mouse);
		break;

	case Action::ChronoSphere:
	case Action::ChronoWarp:
		this->Set_Default_Mouse(MouseCursorType::Chronosphere, mouse);
		break;

	case Action::ParaDrop:
	case Action::AmerParaDrop:
		this->Set_Default_Mouse(MouseCursorType::ParaDrop, mouse);
		break;

	case Action::IvanBomb:
		this->Set_Default_Mouse(MouseCursorType::IvanBomb, mouse);
		break;

	case Action::DisarmBomb:
		this->Set_Default_Mouse(MouseCursorType::Disarm, mouse);
		break;

	case Action::PlaceBeacon:
		this->Set_Default_Mouse(MouseCursorType::Beacon, mouse);
		break;

	case Action::AttackMoveNav:
	case Action::AttackMoveTar:
		this->Set_Default_Mouse(cursor_731CB0(), mouse);
		break;

	case Action::PsychicDominator:
		this->Set_Default_Mouse(MouseCursorType::PsychicDominator, mouse);
		break;

	case Action::SpyPlane:
		this->Set_Default_Mouse(MouseCursorType::SpyPlane, mouse);
		break;

	case Action::GeneticConverter:
		this->Set_Default_Mouse(MouseCursorType::GeneticMutator, mouse);
		break;

	case Action::ForceShield:
		this->Set_Default_Mouse(MouseCursorType::ForceShield, mouse);
		break;

	case Action::NoForceShield:
		this->Set_Default_Mouse(MouseCursorType::NoForceShield, mouse);
		break;

	case Action::Airstrike:
		this->Set_Default_Mouse(MouseCursorType::AirStrike, mouse);
		break;

	case Action::PsychicReveal:
		this->Set_Default_Mouse(MouseCursorType::PsychicReveal, mouse);
		break;

	default:
		this->Set_Default_Mouse(MouseCursorType::Default, mouse);
		break;
	}
}

void DisplayClass::ConvertAction(
	CellStruct& cell,
	bool shadow,
	ObjectClass* pObject,
	Action action,
	bool wsmall)
{
	ObjectClass* pTargetObject = nullptr;
	this->IsTentative = false;

	// Determine target object
	if (pObject)
	{
		pTargetObject = pObject;
		auto const pTechno = AbstractClass::As_Techno(pObject);

		if (pTechno)
		{
			bool const isInvisibleBuilding =
				pTechno->WhatAmI() == AbstractType::Building &&
				static_cast<BuildingClass*>(pTechno)->Type->InvisibleInGame;

			if (!isInvisibleBuilding && !Debug_Map)
			{
				pTechno->MousedOver = true;
			}
		}
	}
	else if (cell.X != DisplayClass::DefaultZoneOffset.X || cell.Y != DisplayClass::DefaultZoneOffset.Y)
	{
		pTargetObject = MapClass::Instance->GetCellAt(cell);
	}

	// Planning mode waypoint placement
	if (this->PlanningMode && this->Waypoint11BC &&
		!HouseClass::GetWaypointAt(HouseClass::CurrentPlayer, &cell) &&
		MapClass::Instance->In_Radar(cell, true))
	{
		auto const pWaypoint = this->Waypoint11BC;
		pWaypoint->X = (cell.X << 8) + 128;
		pWaypoint->Y = (cell.Y << 8) + 128;
		pWaypoint->Z = 0;

		auto const pCell = MapClass::Instance->GetCellAt(cell);
		int const zAdjust = (pCell->Bitfield2 & 0x100) ? Map_LeptonsPerCellZ : 0;
		pWaypoint->Z = MapClass::Instance->Get_Z_Pos(pWaypoint) + zAdjust;
	}

	// Modify action based on waypoint at cell
	Action modifiedAction = action;
	if (HouseClass::GetWaypointAt(HouseClass::CurrentPlayer, &cell))
	{
		switch (action)
		{
		case Action::Move:
		case Action::NoMove:
			modifiedAction = Action::FollowWaypoint;
			break;
		case Action::Enter:
		case Action::Capture:
		case Action::Repair:
			modifiedAction = Action::EnterWaypoint;
			break;
		case Action::Attack:
		case Action::Harvest:
			modifiedAction = Action::AttackWaypoint;
			break;
		default:
			break;
		}
	}

	// Initialize RGB if not set
	ColorStruct* pRGB = &this->RGB11CC;
	if (!pRGB->R && !pRGB->G && !pRGB->B)
	{
		int const paletteEntry = *(MouseDrawer->Palette + 1);
		pRGB->R = static_cast<BYTE>((paletteEntry >> RedShiftLeft) << RedShiftRight);
		pRGB->G = static_cast<BYTE>((paletteEntry >> GreenShiftLeft) << GreenShiftRight);
		pRGB->B = static_cast<BYTE>((paletteEntry >> BlueShiftLeft) << BlueShiftRight);
	}

	// Handle waypoint-related actions (palette updates)
	int waypointPathIndex = -1;

	switch (modifiedAction)
	{
	case Action::PlaceWaypoint:
	case Action::TibSunBug:
	case Action::FollowWaypoint:
	{
		// Get selected waypoint path index for palette offset
		int const selectedIndex = HouseClass::CurrentPlayer->SelectedWaypointPathIndex;
		int const paletteOffset = (selectedIndex == -1) ? 0 : (selectedIndex % 12) * 8;

		// Update mouse palette with waypoint colors
		auto pPalette = MouseDrawer->Palette + 2;
		for (int i = 0; i < 8; ++i)
		{
			int const colorIndex = (i + paletteOffset) % 256;
			auto const& entry = WaypointCursorPalette.Entries[colorIndex];

			int const r = (entry.R >> RedShiftRight) << RedShiftLeft;
			int const g = (entry.G >> GreenShiftRight) << GreenShiftLeft;
			int const b = (entry.B >> BlueShiftRight) << BlueShiftLeft;

			*pPalette = static_cast<WORD>(r | g | b);
			++pPalette;
		}
		break;
	}

	case Action::EnterWaypointMode:
	case Action::SelectWaypoint:
	case Action::LoopWaypointPath:
	case Action::AttackWaypoint:
	case Action::EnterWaypoint:
	case Action::PatrolWaypoint:
	{
		// Place waypoint and get path index
		auto const pWaypointAt = HouseClass::GetWaypointAt(HouseClass::CurrentPlayer, &cell);
		HouseClass::PlaceWaypoint(HouseClass::CurrentPlayer, pWaypointAt, &waypointPathIndex, reinterpret_cast<int*>(&action));

		if (!this->PlanningMode)
		{
			HouseClass::CurrentPlayer->SelectedWaypointPathIndex = waypointPathIndex;
		}

		int const paletteOffset = (waypointPathIndex == -1) ? 0 : (waypointPathIndex % 12) * 8;

		auto pPalette = MouseDrawer->Palette + 2;
		for (int i = 0; i < 8; ++i)
		{
			int const colorIndex = (i + paletteOffset) % 256;
			auto const& entry = WaypointCursorPalette.Entries[colorIndex];

			int const r = (entry.R >> RedShiftRight) << RedShiftLeft;
			int const g = (entry.G >> GreenShiftRight) << GreenShiftLeft;
			int const b = (entry.B >> BlueShiftRight) << BlueShiftLeft;

			*pPalette = static_cast<WORD>(r | g | b);
			++pPalette;
		}
		break;
	}

	default:
	{
		// Default palette handling
		int const r = (pRGB->R >> RedShiftRight) << RedShiftLeft;
		int const g = (pRGB->G >> GreenShiftRight) << GreenShiftLeft;
		int const b = (pRGB->B >> BlueShiftRight) << BlueShiftLeft;

		*(MouseDrawer->Palette + 1) = static_cast<WORD>(r | g | b);

		auto pPalette = MouseDrawer->Palette + 2;
		for (int i = 0; i < 8; ++i)
		{
			int const colorIndex = i % 256;
			auto const& entry = WaypointCursorPalette.Entries[colorIndex];

			int const pr = (entry.R >> RedShiftRight) << RedShiftLeft;
			int const pg = (entry.G >> GreenShiftRight) << GreenShiftLeft;
			int const pb = (entry.B >> BlueShiftRight) << BlueShiftLeft;

			*pPalette = static_cast<WORD>(pr | pg | pb);
			++pPalette;
		}

		if (!this->PlanningMode)
		{
			HouseClass::CurrentPlayer->SelectedWaypointPathIndex = -1;
		}
		break;
	}
	}

	// Determine final action and set mouse cursor
	Action const finalAction = check_ActionType(modifiedAction, pTargetObject, pObject, wsmall);

	if (shadow)
	{
		// Shadow mode (object present) - limited cursor options
		SetMouseForAction_WithObject(finalAction, action, pObject);
	}
	else
	{
		// No shadow - full cursor handling
		SetMouseForAction_NoObject(finalAction, action, waypointPathIndex);
	}
}
*/

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
