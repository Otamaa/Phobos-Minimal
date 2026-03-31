#include "Body.h"

#include <New/Type/CursorTypeClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/SWTypeHandler.h>

std::array<MouseClassExt::MappedActions, (size_t)Action::count + 2> MouseClassExt::CursorIdx;
std::vector<BuildType> MouseClassExt::TabCameos[4u];

const MouseCursor* MouseClassExt::GetCursorData(MouseCursorType nMouse)
{
	if (!CursorTypeClass::Array.empty())
	{
		return CursorTypeClass::Array[(int)nMouse]->CursorData.operator->();
	}

	// if the custom cursor array is empty , then fix then index
	if (nMouse >= MouseCursorType::count)
		nMouse = MouseCursorType::SpyPlane;

	return MouseCursor::DefaultCursors.begin() + (size_t)nMouse;
}

const MouseCursor* MouseClassExt::GetCursorDataFromRawAction(Action nAction)
{
	const auto fixedAction = MouseClassExt::GetActionIndex(nAction);
	const auto Idx = fixedAction < MouseClassExt::CursorIdx.size() ? CursorIdx[fixedAction].Idx : 0;
	return Idx != (size_t)MouseCursorType::Default ? MouseClassExt::GetCursorData((MouseCursorType)Idx) : nullptr;
}

void MouseClassExt::ClearMappedAction()
{
	__stosb(reinterpret_cast<unsigned char*>(CursorIdx.data()), 0, sizeof(MappedActions) * CursorIdx.size());
}

void MouseClassExt::InsertMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded)
{
	CursorIdx[(size_t)nAction].Idx = (size_t)nCursorIdx;
	CursorIdx[(size_t)nAction].AllowShourd = Shrouded;
}

void MouseClassExt::InsertSWMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded)
{
	switch ((PhobosNewActionType)nAction)
	{
	case PhobosNewActionType::SuperWeaponAllowed:
	{
		CursorIdx[(size_t)Action::count].Idx = (size_t)nCursorIdx;
		CursorIdx[(size_t)Action::count].AllowShourd = Shrouded;
	}
	break;
	case PhobosNewActionType::SuperWeaponDisallowed:
	{
		CursorIdx[(size_t)Action::count + 1].Idx = (size_t)nCursorIdx;
		CursorIdx[(size_t)Action::count + 1].AllowShourd = Shrouded;
	}
	break;
	default:
		break;
	}
}

int MouseClassExt::ByWeapon(TechnoClass* pThis, int nWeaponIdx, bool OutOfRange)
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
			const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
			return ((OutOfRange ?
				pWeaponExt->Cursor_AttackOutOfRange : pWeaponExt->Cursor_Attack).Get());
		}
	}

	return int(OutOfRange ?
	MouseCursorType::AttackOutOfRange : MouseCursorType::Attack);
}


//7E198C - vtable
void MouseClassExt::_Update(const int* keyCode, const Point2D* mouseCoords)
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
		WWMouseClass::Instance->Draw(pCursorData->GetMouseHotSpot(pShape), pShape, baseframe);
	}

	this->ScrollClass::Update_(keyCode, mouseCoords);
}

//7E19B0 - vtable
bool MouseClassExt::_Override_Mouse_Shape(MouseCursorType mouse, bool wsmall)
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
		const int nFrame = pCursorData->GetMouseFrame(wsmall);
		const auto pShape = MouseClass::ShapeData();
		WWMouseClass::Instance->Draw(pCursorData->GetMouseHotSpot(pShape), pShape, nFrame);
		this->MouseCursorIndex = mouse;
		this->MouseCursorIsMini = wsmall;
		return true;
	}
	return false;
}

//7E19B8 - vtable
void MouseClassExt::_Mouse_Small(bool wsmall)
{
	if (this->MouseCursorIsMini != wsmall)
	{
		const auto pCursorData = GetCursorData(this->MouseCursorIndex);
		this->MouseCursorIsMini = wsmall;
		const int baseframe = pCursorData->GetMouseFrame(wsmall) + this->MouseCursorCurrentFrame;
		const auto pShape = MouseClass::ShapeData();
		WWMouseClass::Instance->Draw(pCursorData->GetMouseHotSpot(pShape), pShape, baseframe);
	}
}

//5BDBC0 - Not a vtable
int MouseClassExt::_Get_Mouse_Current_Frame(MouseCursorType mouse, bool wsmall) const
{
	return GetCursorData(mouse)->GetMouseFrame(wsmall) + this->MouseCursorCurrentFrame;
}

//5BDB90 - Not a vtable
int MouseClassExt::_Get_Mouse_Frame(MouseCursorType mouse, bool wsmall) const
{
	return GetCursorData(mouse)->GetMouseFrame(wsmall);
}

//5BDC00 - Not a vtable
Point2D MouseClassExt::_Get_Mouse_Hotspot(MouseCursorType mouse) const
{
	return GetCursorData(mouse)->GetMouseHotSpot(MouseClass::ShapeData());
}

//5BE970 - Not a vtable
int MouseClassExt::_Get_Mouse_Start_Frame(MouseCursorType mouse) const
{
	return GetCursorData(this->MouseCursorIndex)->StartFrame;
}

//5BE990 - Not a vtable
int MouseClassExt::_Get_Mouse_Frame_Count(MouseCursorType mouse) const
{
	return GetCursorData(this->MouseCursorIndex)->FrameCount;
}

size_t MouseClassExt::GetActionIndex(Action nAction)
{
	switch ((PhobosNewActionType)nAction)
	{
	case PhobosNewActionType::SuperWeaponAllowed:
	{
		return (size_t)Action::count;
	}
	case PhobosNewActionType::SuperWeaponDisallowed:
	{
		return (size_t)Action::count + 1;
	}
	default:
		return (size_t)nAction;
	}
}

MouseCursorType MouseClassExt::ValidateCursorType(Action nAction)
{
	const auto IdxToSelect = GetActionIndex(nAction);

	if (IdxToSelect < MouseClassExt::CursorIdx.size()
		&& MouseClassExt::CursorIdx[IdxToSelect].Idx != (size_t)MouseCursorType::Default)
	{
		return (MouseCursorType)MouseClassExt::CursorIdx[IdxToSelect].Idx;
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

Action MouseClassExt::ValidateShroudedAction(Action nAction)
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
			if (auto T = flag_cast_to<TechnoClass*>(nObjDvc[0]))
			{
				if (GET_TECHNOTYPE(T)->MoveToShroud)
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
		if (!MouseClassExt::CursorIdx[(size_t)nAction].AllowShourd)
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
