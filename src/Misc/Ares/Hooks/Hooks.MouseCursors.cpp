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

#include <Misc/AresData.h>

#include <New/Type/CursorTypeClass.h>

/*TODO : handle all of these i suppose
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
700600 = TechnoClass_GetActionOnCell_Cursors, 5

; \Misc\Actions.cpp // SW ?
4AB35A = DisplayClass_SetAction_CustomCursor, 6
5BDDC0 = MouseClass_Update_Reset, 5
4D7524 = FootClass_ActionOnObject_Allow, 9
653CA6 = RadarClass_GetMouseAction_AllowMinimap, 5
6929FC = DisplayClass_ChooseAction_CanSell, 7
4ABFBE = DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7

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

447218 = BuildingClass_GetActionOnObject_Deactivated, 6
73FD5A = UnitClass_GetActionOnObject_Deactivated, 5
51E440 = InfantryClass_GetActionOnObject_Deactivated, 8
417CCB = AircraftClass_GetActionOnObject_Deactivated, 5

447548 = BuildingClass_GetActionOnCell_Deactivated, 6
7404B9 = UnitClass_GetActionOnCell_Deactivated, 6
417F83 = AircraftClass_GetActionOnCell_Deactivated, 6

51E710 = InfantryClass_GetActionOnObject_Heal, 7
73FDBD = UnitClass_GetActionOnObject_Heal, 5
51E3B0 = InfantryClass_GetActionOnObject_EMP, 7

6FFF9E = TechnoClass_GetActionOnObject_IvanBombsB, 8

51E488 = InfantryClass_GetActionOnObject2, 5
*/

// need to handle more stuffs before finalize this 
// everything tested working as it should , other than S/L 
// keep this away untill i finish the major SW stuffs ported 

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

	static int NOINLINE ByWeapon(TechnoClass* pThis, int nWeaponIdx, bool OutOfRange)
	{
		//TODO :
		//if (auto pBuilding = specific_cast<BuildingClass*>(pThis)) {
		//	if(!pBuilding->IsPowerOnline())
		//		return OutOfRange ? MouseCursorType::NoEnter : MouseCursorType::Enter;
		//}

		if (const auto pWeaponS = pThis->GetWeapon(nWeaponIdx))
		{
			if (const auto pWeapon = pWeaponS->WeaponType)
			{
				const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
				return ((OutOfRange ?
					pWeaponExt->Cursor_AttackOutOfRange : pWeaponExt->Cursor_Attack).Get());
			}
		}

		return int(OutOfRange ? 
		MouseCursorType::AttackOutOfRange : MouseCursorType::Attack);
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

	static MouseCursorType ValidateCursorType(Action nAction)
	{
		if (MouseClassExt::CursorIdx.contains(nAction)  
			&& MouseClassExt::CursorIdx[nAction].first != MouseCursorType::Default) {
			return MouseClassExt::CursorIdx[nAction].first;
		}

		switch (nAction)
		{
		case Action::Move:
			return MouseCursorType::Move;
		case Action::NoMove:
		case Action::NoIvanBomb:
			return MouseCursorType::NoMove;
		case Action::Enter:
		case Action::Capture:
		case Action::Repair:
		case Action::EnterTunnel:
			return MouseCursorType::Enter;
		case Action::Self_Deploy:
		case Action::AreaAttack:
			return MouseCursorType::Deploy;
		case Action::Attack:
			return MouseCursorType::Attack;
		case Action::Harvest:
			return MouseCursorType::AttackOutOfRange;
		case Action::Select:
		case Action::ToggleSelect:
		case Action::SelectBeacon:
			return MouseCursorType::Select;
		case Action::Eaten:
			return MouseCursorType::EngineerRepair;
		case Action::Sell:
			return MouseCursorType::Sell;
		case Action::SellUnit:
			return MouseCursorType::SellUnit;
		case Action::NoSell:
			return MouseCursorType::NoSell;
		case Action::NoRepair:
		case Action::NoGRepair:
			return MouseCursorType::NoRepair;
		case Action::Sabotage:
		case Action::Demolish:
			return MouseCursorType::Demolish;
		case Action::Tote:
			return MouseCursorType::PsychicReveal | MouseCursorType::Move_NE; // custom 87
		case Action::Nuke:
			return MouseCursorType::Nuke;
		case Action::GuardArea:
			return MouseCursorType::Protect;
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
			return MouseCursorType::Disallowed;
		case Action::GRepair:
			return MouseCursorType::Repair;
		case Action::NoDeploy:
			return MouseCursorType::NoDeploy;
		case Action::NoEnterTunnel:
		case Action::NoEnter:
			return MouseCursorType::NoEnter;
		case Action::TogglePower:
			return MouseCursorType::NoForceShield | MouseCursorType::Move_NW; // custom 88
		case Action::NoTogglePower:
			return MouseCursorType::GeneticMutator | MouseCursorType::Move_NW; // custom 89
		case Action::IronCurtain:
			return MouseCursorType::IronCurtain;
		case Action::LightningStorm:
			return MouseCursorType::LightningStorm;
		case Action::ChronoSphere:
		case Action::ChronoWarp:
			return MouseCursorType::Chronosphere;
		case Action::ParaDrop:
		case Action::AmerParaDrop:
			return MouseCursorType::ParaDrop;
		case Action::IvanBomb:
			return MouseCursorType::IvanBomb;
		case Action::Detonate:
		case Action::DetonateAll:
			return MouseCursorType::Detonate;
		case Action::DisarmBomb:
			return MouseCursorType::Disarm;
		case Action::PlaceBeacon:
			return MouseCursorType::Beacon;
		case Action::AttackMoveNav:
		case Action::AttackMoveTar:
			return MouseCursorType::AttackOutOfRange2;
		case Action::PsychicDominator:
			return MouseCursorType::PsychicDominator;
		case Action::SpyPlane:
			return MouseCursorType::SpyPlane;
		case Action::GeneticConverter:
			return MouseCursorType::GeneticMutator;
		case Action::ForceShield:
			return MouseCursorType::ForceShield;
		case Action::NoForceShield:
			return MouseCursorType::NoForceShield;
		case Action::Airstrike:
			return MouseCursorType::AirStrike;
		case Action::PsychicReveal:
			return MouseCursorType::PsychicReveal;
		}

		return MouseCursorType::Default;
	}

	static Action ValidateShroudedAction(Action nAction)
	{
		switch (nAction)
		{
		case Action::Attack:
		{
			return Action::Move;
		}
		case Action::NoMove:
		{
			auto const& nObjDvc = ObjectClass::CurrentObjects(); //current object with cursor
			if (nObjDvc.Count)
			{
				if (auto T = generic_cast<TechnoClass*>(nObjDvc[0]))
				{
					if (T->GetTechnoType()->MoveToShroud)
					{
						return Action::Move;
					}
				}
			}

			return Action::NoMove;
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
		{
			if (MouseClassExt::CursorIdx.contains(nAction)
				&& !MouseClassExt::CursorIdx[nAction].second)
				return Action::None;
			else
				break;
		}
		case Action::Eaten:
		case Action::NoGRepair:
		case Action::NoRepair:
			return Action::NoRepair;
		case Action::SellUnit:
		case Action::Sell:
			return Action::NoSell;
		case Action::TogglePower:
			return Action::NoTogglePower;
		case Action::NoEnterTunnel:
			return Action::NoEnter;
		case Action::ChronoWarp:
			return Action::ChronoSphere;
		case Action::SelectBeacon:
			return Action::Select;
		case Action::AmerParaDrop:
			return Action::ParaDrop;
		default:
			break;
		}

		return nAction;
	}
};
static_assert(sizeof(MouseClassExt) == sizeof(MouseClass), "Invalid Size !");

//std::unordered_map<Action, std::pair<MouseCursorType, bool>> MouseClassExt::CursorIdx;
//
////ares reset their cursor here 
//DEFINE_HOOK(0x5BDDC0, MouseClass_Update_Replace, 0x5)
//{
//	GET(MouseClassExt*, pMouse, ECX);
//	GET_STACK(const  int*, keyCode, 0x4);
//	GET_STACK(const Point2D*, mouseCoords, 0x8);
//
//	pMouse->_Update(keyCode, mouseCoords);
//	return 0x5BDF2A;
//}
//
//DEFINE_HOOK(0x5BDC80, MouseClass_Override_MouseShape_Replace, 0x7)
//{
//	GET(MouseClassExt*, pMouse, ECX);
//	GET_STACK(MouseCursorType, mouse, 0x4);
//	GET_STACK(bool, wsmall, 0x8);
//
//	R->AL(pMouse->_Override_Mouse_Shape(mouse, wsmall));
//	return 0x5BDDB3;
//}
//
//DEFINE_HOOK(0x5BDAB0, MouseClass_Small_Replace, 0x7)
//{
//	GET(MouseClassExt*, pMouse, ECX);
//	GET_STACK(bool, wsmall, 0x4);
//
//	pMouse->_Mouse_Small(wsmall);
//	return 0x5BDB82;
//
//}
//
//DEFINE_HOOK(0x5BDBC0, MouseClass_Get_Mouse_Current_Frame_Replace, 0xB)
//{
//	GET(MouseClassExt*, pMouse, ECX);
//	GET_STACK(MouseCursorType, mouse, 0x4);
//	GET_STACK(bool, wsmall, 0x8);
//
//	R->EAX(pMouse->_Get_Mouse_Current_Frame(mouse, wsmall));
//	return 0x5BDBEE;
//}
//
//DEFINE_HOOK(0x5BDB90, MouseClass_Get_Mouse_Frame_Replace, 0xB)
//{
//	GET(MouseClassExt*, pMouse, ECX);
//	GET_STACK(MouseCursorType, mouse, 0x4);
//	GET_STACK(bool, wsmall, 0x8);
//
//	R->EAX(pMouse->_Get_Mouse_Frame(mouse, wsmall));
//	return 0x5BDBB6;
//}
//
//DEFINE_HOOK(0x5BDC00, MouseClass_Get_Mouse_Hotspot_Replace, 0x5)
//{
//	GET(MouseClassExt*, pMouse, ECX);
//	GET_STACK(Point2D*, pRet, 0x4);
//	GET_STACK(MouseCursorType, mouse, 0x8);
//
//	const auto nResult = pMouse->_Get_Mouse_Hotspot(mouse);
//	pRet->X = nResult.X;
//	pRet->Y = nResult.Y;
//	R->EAX(pRet);
//	return 0x5BDC77;
//}
//
//DEFINE_HOOK(0x5BE970, MouseClass_Get_Mouse_Start_Frame_Replace, 0xB)
//{
//	GET(MouseClassExt*, pMouse, ECX);
//	GET_STACK(MouseCursorType, mouse, 0x4);
//
//	R->EAX(pMouse->_Get_Mouse_Start_Frame(mouse));
//	return 0x5BE984;
//}
//
//DEFINE_HOOK(0x5BE990, MouseClass_Get_Mouse_Frame_Count_Replace, 0xB)
//{
//	GET(MouseClassExt*, pMouse, ECX);
//	GET_STACK(MouseCursorType, mouse, 0x4);
//	R->EAX(pMouse->_Get_Mouse_Frame_Count(mouse));
//	return 0x5BE9A4;
//}
//
//DEFINE_OVERRIDE_HOOK(0x4AB35A, DisplayClass_SetAction_CustomCursor, 0x6)
//{
//	GET(DisplayClass*, pThis, ESI);
//	GET(Action, nAction, EAX);
//	GET_STACK(bool, bMini, STACK_OFFS(0x20, -0x14));//0x34
//	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, 0x8)); //0x18
//	GET_STACK(bool, bIsShrouded, 0x28);
//
//	if (bIsShrouded) {
//		nAction = MouseClassExt::ValidateShroudedAction(nAction);
//	}
//
//	if (nAction == Action::Attack) {
//		// WeaponCursor
//		// doesnt work with `Deployed` techno
//		auto const& nObjectVect = ObjectClass::CurrentObjects();
//
//		if (pTarget && nObjectVect.Count == 1 && Is_Techno(nObjectVect[0])) {
//			if (!static_cast<TechnoClass*>(nObjectVect[0])->IsCloseEnoughToAttack(pTarget)) {
//					//if too far from the target
//					MouseClassExt::InsertMappedAction(
//						MouseCursorType::NoEnter
//						, Action::Attack, false);
//			}
//
//		} else {
//				nAction = Action::Harvest;
//		}
//	}
//
//	pThis->SetCursor(MouseClassExt::ValidateCursorType(nAction), bMini);
//	return 0x4AB78F;
//}

// WeaponCursor
DEFINE_OVERRIDE_HOOK(0x70055D, TechnoClass_GetActionOnObject_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, nWeapon, STACK_OFFS(0x1C, 0x8));

	const auto nCursor = MouseClassExt::ByWeapon(pThis, nWeapon, false);
	AresData::SetMouseCursorAction(nCursor, Action::Attack, false);
	return 0;
}

 //WeaponCursor
DEFINE_OVERRIDE_HOOK(0x700AA8, TechnoClass_GetActionOnCell_AttackCursor, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, nWeapon, EBP);
	const auto nCursor = MouseClassExt::ByWeapon(pThis, nWeapon, false);
	AresData::SetMouseCursorAction(nCursor, Action::Attack, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x700600, TechnoClass_GetActionOnCell_Cursors, 5)
{
	GET(TechnoClass*, pThis, ECX);
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	// Cursor Move
	AresData::SetMouseCursorAction(pTypeExt->Cursor_Move.Get(), Action::Move, false);

	// Cursor NoMove
	AresData::SetMouseCursorAction(pTypeExt->Cursor_NoMove.Get(), Action::NoMove, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7000CD, TechnoClass_GetActionOnObject_SelfDeployCursor, 6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	//Cursor Deploy
	AresData::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::AreaAttack, false);
	AresData::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::Self_Deploy, false);
	//

	//Cursor NoDeploy
	AresData::SetMouseCursorAction(pTypeExt->Cursor_NoDeploy.Get(), Action::NoDeploy, false);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7400F0, UnitClass_GetActionOnObject_SelfDeployCursor_Bunker, 6)
{
	GET(UnitClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->BunkerLinkedItem) {
		//Cursor Deploy
		AresData::SetMouseCursorAction(pTypeExt->Cursor_Deploy.Get(), Action::Self_Deploy, false);
		return 0x73FFE6;
	}

	return pThis->Type->DeployFire ? 0x7400FA : 0x740115;
}
