#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <New/Type/CursorTypeClass.h>

/*
; \Enum\CursorTypes.cpp
5BDC8C = MouseClass_UpdateCursor, 7
5BDADF = MouseClass_UpdateCursorMinimapState_UseCursor, 0
5BDDC8 = MouseClass_Update_AnimateCursor, 6
5BDE64 = MouseClass_Update_AnimateCursor2, 6
5BDB90 = MouseClass_GetCursorFirstFrame_Minimap, B
5BE974 = MouseClass_GetCursorFirstFrame, 7
5BE994 = MouseClass_GetCursorFrameCount, 7
5BDBC4 = MouseClass_GetCursorCurrentFrame, 7
5BDC1B = MouseClass_GetCursorHotSpot, 7

; \Ext\Techno\Hooks.Cursor.cpp
417E16 = AircraftClass_GetActionOnObject_Dock, 6
70055D = TechnoClass_GetActionOnObject_AttackCursor, 8
700AA8 = TechnoClass_GetActionOnCell_AttackCursor, 8
7000CD = TechnoClass_GetActionOnObject_SelfDeployCursor, 6
7400F0 = UnitClass_GetActionOnObject_SelfDeployCursor_Bunker, 6
6FFEC0 = TechnoClass_GetActionOnObject_Cursors, 5
700600 = TechnoClass_GetActionOnCell_Cursors, 5

; \Misc\Actions.cpp // SW ?
4AB35A = DisplayClass_SetAction_CustomCursor, 6
5BDDC0 = MouseClass_Update_Reset, 5
4D7524 = FootClass_ActionOnObject_Allow, 9
653CA6 = RadarClass_GetMouseAction_AllowMinimap, 5
6929FC = DisplayClass_ChooseAction_CanSell, 7
4ABFBE = DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7

51E4ED = InfantryClass_GetActionOnObject_EngineerRepairable, 6
51FA82 = InfantryClass_GetActionOnCell_EngineerRepairable, 6

51EE6B = InfantryClass_GetActionOnObject_Saboteur, 6
51EB48 = InfantryClass_GetActionOnObject_IvanGrinder, A
51E5BB = InfantryClass_GetActionOnObject_MultiEngineerA, 7
51E5E1 = InfantryClass_GetActionOnObject_MultiEngineerB, 7
51E635 = InfantryClass_GetActionOnObject_EngineerOverFriendlyBuilding, 5
51E7BF = InfantryClass_GetActionOnObject_CanCapture, 6

44725F = BuildingClass_GetActionOnObject_TargetABuilding, 5
44731C = BuildingClass_GetActionOnObject_Tunnel, 6
51ED8E = InfantryClass_GetActionOnObject_Tunnel, 6
7004AD = TechnoClass_GetActionOnObject_Saboteur, 6


417DD2 = AircraftClass_GetActionOnObject_NoManualUnload, 6
740031 = UnitClass_GetActionOnObject_NoManualUnload, 6
7008D4 = TechnoClass_GetActionOnCell_NoManualFire, 6
700536 = TechnoClass_GetActionOnObject_NoManualFire, 6
74031A = UnitClass_GetActionOnObject_NoManualEnter, 6
51E748 = InfantryClass_GetActionOnObject_NoSelfGuardArea, 8

447218 = BuildingClass_GetActionOnObject_Deactivated, 6
73FD5A = UnitClass_GetActionOnObject_Deactivated, 5
51E440 = InfantryClass_GetActionOnObject_Deactivated, 8
417CCB = AircraftClass_GetActionOnObject_Deactivated, 5

447548 = BuildingClass_GetActionOnCell_Deactivated, 6
7404B9 = UnitClass_GetActionOnCell_Deactivated, 6
51F808 = InfantryClass_GetActionOnCell_Deactivated, 6
417F83 = AircraftClass_GetActionOnCell_Deactivated, 6

51E710 = InfantryClass_GetActionOnObject_Heal, 7
73FDBD = UnitClass_GetActionOnObject_Heal, 5
51E3B0 = InfantryClass_GetActionOnObject_EMP, 7

6FFEC0 = TechnoClass_GetActionOnObject_IvanBombsA, 5
6FFF9E = TechnoClass_GetActionOnObject_IvanBombsB, 8

51E488 = InfantryClass_GetActionOnObject2, 5
*/


//
//DEFINE_OVERRIDE_HOOK(0x6FFEC0, TechnoClass_GetActionOnObject_Cursors, 5)
//{
//	//GET(TechnoClass*, pThis, ECX);
//	GET_STACK(ObjectClass*, pTarget, 0x4);
//
//	MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::Move, false);
//	MouseCursorTypeClass::Insert(MouseCursorType::NoMindControl, Action::NoMove, false);
//
//	if (pTarget->GetTechnoType())
//	{
//		MouseCursorTypeClass::Insert(MouseCursorType::Chronosphere, Action::Repair, false);
//		MouseCursorTypeClass::Insert(MouseCursorType::GeneticMutator, Action::Enter, false);
//		MouseCursorTypeClass::Insert(MouseCursorType::NoForceShield, Action::NoEnter, false);
//	}
//
//	return 0;
//}
//
//DEFINE_OVERRIDE_HOOK(0x700600, TechnoClass_GetActionOnCell_Cursors, 5)
//{
//	//GET(TechnoClass*, pThis, ECX);
//	MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::Move, false);
//	MouseCursorTypeClass::Insert(MouseCursorType::NoMindControl, Action::NoMove, false);
//	return 0;
//}
//
//DEFINE_OVERRIDE_HOOK(0x7000CD, TechnoClass_GetActionOnObject_SelfDeployCursor, 6)
//{
//	//GET(TechnoClass*, pThis, ESI);
//	MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::AreaAttack, false);
//	MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::Self_Deploy, false);
//	MouseCursorTypeClass::Insert(MouseCursorType::NoEnter, Action::NoDeploy, false);
//	return 0;
//}
//
//DEFINE_OVERRIDE_HOOK(0x7400F0, UnitClass_GetActionOnObject_SelfDeployCursor_Bunker, 6)
//{
//	GET(UnitClass*, pThis, ESI);
//	if (pThis->BunkerLinkedItem)
//	{
//		MouseCursorTypeClass::Insert(MouseCursorType::Attack, Action::Self_Deploy, false);
//		return 0x73FFE6;
//	}
//
//	return pThis->unknown_bool_6AC ? 0x7400FA : 0x740115;
//}

class MouseClassExt final : public MouseClass
{
	NOINLINE const MouseCursor* GetCursorData(MouseCursorType nMouse) const
	{
		if (!CursorTypeClass::Array.empty())
		{
			return CursorTypeClass::FindFromIndex((int)nMouse)->CursorData.GetEx()
					;
		}

		// if the custom cursor array is empty , then fix then index
		if (nMouse >= MouseCursorType::count)
			nMouse = MouseCursorType::SpyPlane;

		return &MouseCursor::DefaultCursorsB[(int)nMouse];
	}

public:
#pragma region MappedAction
	static std::unordered_map<Action , std::pair<MouseCursorType, bool>> CursorIdx;

	static void ClearMappedAction()
	{
		CursorIdx.clear();
	}

	static void NOINLINE InsertMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded)
	{
		CursorIdx[nAction].first = nCursorIdx;
		CursorIdx[nAction].second = Shrouded;
	}

	static MouseCursorType NOINLINE ByWeapon(TechnoClass* pThis, int nWeaponIdx, bool OutOfRange)
	{
		if (auto pWeaponS = pThis->GetWeapon(nWeaponIdx))
		{
			if (pWeaponS->WeaponType)
			{
				return (MouseCursorType)(OutOfRange ?
					CursorTypeClass::FindIndexById("NoEnter") : //Dummy
					CursorTypeClass::FindIndexById("Enter")); //Dummy
			}
		}
		return OutOfRange ? MouseCursorType::AttackOutOfRange : MouseCursorType::Attack;
	}
#pragma endregion
public:

	//7E198C - vtable
	void _Update(const int* keyCode, const Point2D* mouseCoords)
	{
		ClearMappedAction();
		const auto pCursorData = GetCursorData(this->MouseCursorIndex);
		const auto nFrameRate = pCursorData->GetFrameRate();

		if (nFrameRate && !MouseClass::Timer->GetTimeLeft())
		{
			this->MouseCursorCurrentFrame++;
			this->MouseCursorCurrentFrame %= pCursorData->GetMouseFrameCount(this->MouseCursorIsMini);
			MouseClass::Timer->Start(nFrameRate);
			const int baseframe = pCursorData->GetMouseFrame(this->MouseCursorIsMini) + this->MouseCursorCurrentFrame;
			const auto pShape = MouseClass::ShapeData();
			WWMouseClass::Instance->DrawSHPFrame(pCursorData->GetMouseHotSpot(pShape), pShape, baseframe);
		}

		this->ScrollClass::Update_(keyCode, mouseCoords);
	}

	//7E19B0 - vtable
	bool _Override_Mouse_Shape(MouseCursorType mouse, bool wsmall = false)
	{
		const auto pCursorData = GetCursorData(mouse);

		if (pCursorData->SmallFrame == -1 || !pCursorData->SmallFrameCount)
			wsmall = false;

		if (!MouseClass::ShapeOverride()
			|| MouseClass::ShapeData()
				&& (mouse != this->MouseCursorIndex || wsmall != this->MouseCursorIsMini))
		{

			MouseClass::ShapeOverride = true;
			MouseClass::Timer->Start(pCursorData->GetFrameRate());
			this->MouseCursorCurrentFrame = 0;
			const int nFrame = pCursorData->GetMouseFrame(wsmall) ;
			const auto pShape = MouseClass::ShapeData();
			WWMouseClass::Instance->DrawSHPFrame(pCursorData->GetMouseHotSpot(pShape), pShape, nFrame);
			this->MouseCursorIndex = mouse;
			this->MouseCursorIsMini = wsmall;
			return true;
		}
		return false;
	}

	//7E19B8 - vtable
	void _Mouse_Small(bool wsmall = true)
	{
		if (this->MouseCursorIsMini != wsmall)
		{
			const auto pCursorData = GetCursorData(this->MouseCursorIndex);
			this->MouseCursorIsMini = wsmall;
			const int baseframe = pCursorData->GetMouseFrame(wsmall) + this->MouseCursorCurrentFrame;
			const auto pShape = MouseClass::ShapeData();
			WWMouseClass::Instance->DrawSHPFrame(pCursorData->GetMouseHotSpot(pShape), pShape, baseframe);
		}
	}

	//5BDBC0 - Not a vtable
	int _Get_Mouse_Current_Frame(MouseCursorType mouse, bool wsmall = false) const
	{
		return GetCursorData(mouse)->GetMouseFrame(wsmall) + this->MouseCursorCurrentFrame;
	}
	
	//5BDB90 - Not a vtable
	int _Get_Mouse_Frame(MouseCursorType mouse, bool wsmall = false) const
	{
		return GetCursorData(mouse)->GetMouseFrame(wsmall);
	}

	//5BDC00 - Not a vtable
	Point2D _Get_Mouse_Hotspot(MouseCursorType mouse) const
	{
		return GetCursorData(mouse)->GetMouseHotSpot(MouseClass::ShapeData());
	}

	//5BE970 - Not a vtable
	int _Get_Mouse_Start_Frame(MouseCursorType mouse) const
	{
		return GetCursorData(this->MouseCursorIndex)->StartFrame;
	}

	//5BE990 - Not a vtable
	int _Get_Mouse_Frame_Count(MouseCursorType mouse) const
	{
		return GetCursorData(this->MouseCursorIndex)->FrameCount;
	}
};
static_assert(sizeof(MouseClassExt) == sizeof(MouseClass), "Invalid Size !");

std::unordered_map<Action, std::pair<MouseCursorType, bool>> MouseClassExt::CursorIdx;

DEFINE_OVERRIDE_HOOK(0x70055D, TechnoClass_GetActionOnObject_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, nWeapon, STACK_OFFS(0x1C, 0x8));

	const auto nCursor = MouseClassExt::ByWeapon(pThis, nWeapon, false);
	MouseClassExt::InsertMappedAction(nCursor, Action::Attack, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x700AA8, TechnoClass_GetActionOnCell_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nWeapon, EBP);
	const auto nCursor = MouseClassExt::ByWeapon(pThis, nWeapon, false);
	MouseClassExt::InsertMappedAction(nCursor, Action::Attack, false);
	return 0;
}

DEFINE_HOOK(0x531703, Game_InitBulkData_LoadCursorData, 0x5)
{
	// issue #198: animate the paradrop cursor
	MouseCursor::GetCursor(MouseCursorType::ParaDrop).FrameRate = 4;

	// issue #214: also animate the chronosphere cursor
	MouseCursor::GetCursor(MouseCursorType::Chronosphere).FrameRate = 4;

	// issue #1380: the iron curtain cursor
	MouseCursor::GetCursor(MouseCursorType::IronCurtain).FrameRate = 4;

	// animate the engineer damage cursor
	MouseCursor::GetCursor(MouseCursorType::Detonate).FrameRate = 4;

	//Load the default regardless
	CursorTypeClass::AddDefaults();

	if (auto pINI = CCINIClass::LoadINIFile("Mouse.ini"))
	{
		CursorTypeClass::LoadFromINIList_New(pINI, true);
		CCINIClass::UnloadINIFile(pINI);
	}

	return 0x0;
}

//ares reset their cursor here 
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

	MouseCursorType nType = MouseCursorType::Default;

	auto SetAttackCursorOrRetAction = [&pTarget, &nType](Action& nAction)
	{
		auto const& nObjectVect = ObjectClass::CurrentObjects();

		if (nAction == Action::Attack)
		{
			if (pTarget && nObjectVect.Count == 1)
			{
				if (!Is_Techno(nObjectVect[0]))
					return;

				const auto pSelected = static_cast<TechnoClass*>(nObjectVect[0]);
				const auto nWeaponIdx = pSelected->SelectWeapon(pTarget);

				if (!pSelected->IsCloseEnough(pTarget, nWeaponIdx)) {
						//if too far from the target
						nType = MouseClassExt::ByWeapon(pSelected,nWeaponIdx,true);
				}
			}
			else
			{
				nType = Action::Harvest;
			}
		}
	};

	//EvakuateFinalAction part 1
	if (bIsShrouded)
	{
		switch (nAction)
		{
		case Action::NoMove:
		{
			auto const& nObjDvc = ObjectClass::CurrentObjects(); //current object with cursor
			if (nObjDvc.Count)
			{
				if (auto T = generic_cast<TechnoClass*>(nObjDvc[0]))
				{
					if (T->GetTechnoType()->MoveToShroud)
					{
						nAction = Action::Move;
						break;
					}
				}
			}

			nAction = Action::NoMove;
		}

		break;
		case Action::Enter:
		case Action::Self_Deploy:
		case Action::Harvest:
		case Action::Select:
		case Action::ToggleSelect:
		case Action::Capture:
		case Action::Repair:
		case Action::Sabotage:
		case Action::DontUse2:
		case Action::DontUse3:
		case Action::DontUse4:
		case Action::DontUse5:
		case Action::DontUse6:
		case Action::DontUse7:
		case Action::DontUse8:
		case Action::Damage:
		case Action::GRepair:
		case Action::EnterTunnel:
		case Action::DragWaypoint:
		case Action::AreaAttack:
		case Action::IvanBomb:
		case Action::NoIvanBomb:
		case Action::Detonate:
		case Action::DetonateAll:
		case Action::DisarmBomb:
		case Action::SelectNode:
		case Action::AttackSupport:
		case Action::Demolish:
		case Action::Airstrike:
			if (MouseClassExt::CursorIdx.contains(nAction) && !MouseClassExt::CursorIdx[nAction].second)
				nAction = Action::None;
			break;
		case Action::Eaten:
		case Action::NoGRepair:
		case Action::NoRepair:
			nAction = Action::NoRepair;
			break;
		case Action::SellUnit:
		case Action::Sell:
		case Action::NoSell:
			nAction = Action::NoSell;
			break;
		case Action::TogglePower:
			nAction = Action::NoTogglePower;
			break;
		case Action::NoEnterTunnel:
			nAction = Action::NoEnter;
			break;
		case Action::ChronoWarp:
			nAction = Action::ChronoSphere;
			break;
		case Action::SelectBeacon:
			nAction = Action::Select;
			break;
		case Action::ParaDrop:
		case Action::AmerParaDrop:
			nAction = Action::ParaDrop;
			break;
		default:
			SetAttackCursorOrRetAction(nAction);
			break;
		}
	}
	else
	{
		SetAttackCursorOrRetAction(nAction);
	}

	//evaluation part 2
	if(MouseClassExt::CursorIdx.contains(nAction))
		nType = MouseClassExt::CursorIdx[nAction].first;

	//Evaluate MouseCursorType part 3
	//if default cursor is still present 
	//re-check the cases and set the cursor type
	if (nType == MouseCursorType::Default)
	{
		switch (nAction)
		{
		case Action::Move:
			nType = MouseCursorType::Move;
			break;
		case Action::NoMove:
		case Action::NoIvanBomb:
			nType = MouseCursorType::NoMove;
			break;
		case Action::Enter:
		case Action::Capture:
		case Action::Repair:
		case Action::EnterTunnel:
			nType = MouseCursorType::Enter;
			break;
		case Action::Self_Deploy:
		case Action::AreaAttack:
			nType = MouseCursorType::Deploy;
			break;
		case Action::Attack:
			nType = MouseCursorType::Attack;
			break;
		case Action::Harvest:
			nType = MouseCursorType::AttackOutOfRange;
			break;
		case Action::Select:
		case Action::ToggleSelect:
		case Action::SelectBeacon:
			nType = MouseCursorType::Select;
			break;
		case Action::Eaten:
			nType = MouseCursorType::EngineerRepair;
			break;
		case Action::Sell:
			nType = MouseCursorType::Sell;
			break;
		case Action::SellUnit:
			nType = MouseCursorType::SellUnit;
			break;
		case Action::NoSell:
			nType = MouseCursorType::NoSell;
			break;
		case Action::NoRepair:
		case Action::NoGRepair:
			nType = MouseCursorType::NoRepair;
			break;
		case Action::Sabotage:
		case Action::Demolish:
			nType = MouseCursorType::Demolish;
			break;
		case Action::Tote:
			nType = MouseCursorType::PsychicReveal | MouseCursorType::Move_NE; // custom 87
			break;
		case Action::Nuke:
			nType = MouseCursorType::Nuke;
			break;
		case Action::GuardArea:
			nType = MouseCursorType::Protect;
			break;
		case Action::Heal:
		case Action::PlaceWaypoint:
		case Action::TibSunBug:
		case Action::EnterWaypointMode:
		case Action::FollowWaypoint:
		case Action::SelectWaypoint:
		case Action::LoopWaypointPath:
		case Action::AttackWaypoint:
		case Action::EnterWaypoint:
		case Action::PatrolWaypoint:
			nType = MouseCursorType::Disallowed;
			break;
		case Action::GRepair:
			nType = MouseCursorType::Repair;
			break;
		case Action::NoDeploy:
			nType = MouseCursorType::NoDeploy;
			break;
		case Action::NoEnterTunnel:
		case Action::NoEnter:
			nType = MouseCursorType::NoEnter;
			break;
		case Action::TogglePower:
			nType = MouseCursorType::NoForceShield | MouseCursorType::Move_NW; // custom 88
			break;
		case Action::NoTogglePower:
			nType = MouseCursorType::GeneticMutator | MouseCursorType::Move_NW; // custom 89
			break;
		case Action::IronCurtain:
			nType = MouseCursorType::IronCurtain;
			break;
		case Action::LightningStorm:
			nType = MouseCursorType::LightningStorm;
			break;
		case Action::ChronoSphere:
		case Action::ChronoWarp:
			nType = MouseCursorType::Chronosphere;
			break;
		case Action::ParaDrop:
		case Action::AmerParaDrop:
			nType = MouseCursorType::ParaDrop;
			break;
		case Action::IvanBomb:
			nType = MouseCursorType::IvanBomb;
			break;
		case Action::Detonate:
		case Action::DetonateAll:
			nType = MouseCursorType::Detonate;
			break;
		case Action::DisarmBomb:
			nType = MouseCursorType::Disarm;
			break;
		case Action::PlaceBeacon:
			nType = MouseCursorType::Beacon;
			break;
		case Action::AttackMoveNav:
		case Action::AttackMoveTar:
			nType = MouseCursorType::AttackOutOfRange2;
			break;
		case Action::PsychicDominator:
			nType = MouseCursorType::PsychicDominator;
			break;
		case Action::SpyPlane:
			nType = MouseCursorType::SpyPlane;
			break;
		case Action::GeneticConverter:
			nType = MouseCursorType::GeneticMutator;
			break;
		case Action::ForceShield:
			nType = MouseCursorType::ForceShield;
			break;
		case Action::NoForceShield:
			nType = MouseCursorType::NoForceShield;
			break;
		case Action::Airstrike:
			nType = MouseCursorType::AirStrike;
			break;
		case Action::PsychicReveal:
			nType = MouseCursorType::PsychicReveal;
			break;
		default:
			break;
		}
	}

	pThis->SetCursor(nType, bMini);
	return 0x4AB78F;
}

#pragma warning( push )
#pragma warning (disable : 4245)
#pragma warning (disable : 4838)
DEFINE_DISABLE_HOOK(0x5BDC8C, MouseClass_UpdateCursor)
DEFINE_DISABLE_HOOK(0x5BDADF, MouseClass_UpdateCursorMinimapState_UseCursor)
DEFINE_DISABLE_HOOK(0x5BDDC8, MouseClass_Update_AnimateCursor)
DEFINE_DISABLE_HOOK(0x5BDE64, MouseClass_Update_AnimateCursor2)
DEFINE_DISABLE_HOOK(0x5BDB90, MouseClass_GetCursorFirstFrame_Minimap)
DEFINE_DISABLE_HOOK(0x5BE974, MouseClass_GetCursorFirstFrame)
DEFINE_DISABLE_HOOK(0x5BE994, MouseClass_GetCursorFrameCount)
DEFINE_DISABLE_HOOK(0x5BDBC4, MouseClass_GetCursorCurrentFrame)
DEFINE_DISABLE_HOOK(0x5BDC1B, MouseClass_GetCursorHotSpot)
DEFINE_DISABLE_HOOK(0x653B3A, RadarClass_GetMouseAction_CustomSWAction)
DEFINE_DISABLE_HOOK(0x4AC20C, DisplayClass_LeftMouseButtonUp)
DEFINE_DISABLE_HOOK(0x5BDDC0, MouseClass_Update_Reset)
DEFINE_DISABLE_HOOK(0x4D7524, FootClass_ActionOnObject_Allow)
DEFINE_DISABLE_HOOK(0x653CA6, RadarClass_GetMouseAction_AllowMinimap)
DEFINE_DISABLE_HOOK(0x7000CD, TechnoClass_GetActionOnObject_SelfDeployCursor)
DEFINE_DISABLE_HOOK(0x7400F0, UnitClass_GetActionOnObject_SelfDeployCursor_Bunker)
DEFINE_DISABLE_HOOK(0x6FFEC0, TechnoClass_GetActionOnObject_Cursors)
DEFINE_DISABLE_HOOK(0x700600, TechnoClass_GetActionOnCell_Cursors)
#pragma warning( pop )