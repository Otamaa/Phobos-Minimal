#include "Body.h"

#include "NewSuperWeaponType/SWTypeHandler.h"

#include <Utilities/Enum.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/HouseType/Body.h>

#include <Misc/Ares/Hooks/Header.h>
#include <Utilities/Macro.h>

// 4AC20C, 7
// translates SW click to type
ASMJIT_PATCH(0x4AC20C, DisplayClass_LeftMouseButtonUp, 7)
{
	GET_STACK(Action, nAction, 0x9C);

	if (nAction < (Action)PhobosNewActionType::SuperWeaponDisallowed)
	{
		// get the actual firing SW type instead of just the first type of the
		// requested action. this allows clones to work for legacy SWs (the new
		// ones use SW_*_CURSORs). we have to check that the action matches the
		// action of the found type as the no-cursor represents a different
		// action and we don't want to start a force shield even tough the UI
		// says no.
		auto pSW = SuperWeaponTypeClass::Array->get_or_default(Unsorted::CurrentSWType());
		if (pSW && (pSW->Action != nAction))
		{
			pSW = nullptr;
		}

		R->EAX(pSW);
		return pSW ? 0x4AC21C : 0x4AC294;
	}
	else if (nAction == (Action)PhobosNewActionType::SuperWeaponDisallowed)
	{
		R->EAX(0);
		return 0x4AC294;
	}

	R->EAX(SWTypeExtData::CurrentSWType);
	return 0x4AC21C;
}

ASMJIT_PATCH(0x653B3A, RadarClass_GetMouseAction_CustomSWAction, 7)
{
	GET_STACK(CellStruct, MapCoords, STACK_OFFS(0x54, 0x3C));
	REF_STACK(MouseEvent, flag, 0x58);

	enum
	{
		CheckOtherCases = 0x653CA3,
		DrawMiniCursor = 0x653CC0,
		NothingToDo = 0,
		DifferentEventFlags = 0x653D6F
	};

	if (Unsorted::CurrentSWType() < 0)
		return NothingToDo;

	if ((flag & (MouseEvent::RightDown | MouseEvent::RightUp)))
		return DifferentEventFlags;

	const auto nResult = SWTypeExtData::GetAction(SuperWeaponTypeClass::Array->Items[Unsorted::CurrentSWType], &MapCoords);

	if (nResult == Action::None)
		return NothingToDo; //vanilla action

	R->ESI(nResult);
	return CheckOtherCases;
}

// bugfix #277 revisited: VeteranInfantry and friends don't show promoted cameos
ASMJIT_PATCH(0x712045, TechnoTypeClass_GetCameo, 5)
{
	GET(TechnoTypeClass*, pThis, ECX);

	R->EAX(TechnoTypeExt_ExtData::CameoIsElite(pThis, HouseClass::CurrentPlayer)
		? pThis->AltCameo : pThis->Cameo);
	return 0x7120C6;
}
