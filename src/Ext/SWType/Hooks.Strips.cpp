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

//ASMJIT_PATCH(0x653B3A, RadarClass_GetMouseAction_CustomSWAction, 7)
//{
//	GET_STACK(CellStruct, MapCoords, STACK_OFFS(0x54, 0x3C));
//	REF_STACK(MouseEvent, flag, 0x58);
//
//	enum
//	{
//		CheckOtherCases = 0x653CA3,
//		DrawMiniCursor = 0x653CC0,
//		NothingToDo = 0,
//		DifferentEventFlags = 0x653D6F
//	};
//
//	if (Unsorted::CurrentSWType() < 0)
//		return NothingToDo;
//
//	if ((flag & (MouseEvent::RightDown | MouseEvent::RightUp)))
//		return DifferentEventFlags;
//
//	const auto nResult = SWTypeExtData::GetAction(SuperWeaponTypeClass::Array->Items[Unsorted::CurrentSWType], &MapCoords);
//
//	if (nResult == Action::None)
//		return NothingToDo; //vanilla action
//
//	R->ESI(nResult);
//	return CheckOtherCases;
//}
//
//ASMJIT_PATCH(0x653CA6, RadarClass_GetMouseAction_AllowMinimap, 5)
//{
//	GET(int, nAction, EAX);
//	enum { AllowMini = 0x653CC0, DisAllowMini = 0x653CBA, ReConsiderAllowMini = 0x653CAB };
//	// + 1 because it get negative one and leaed
//	if (const MouseCursor* pCursor = MouseClassExt::GetCursorDataFromRawAction(Action(nAction + 1)))
//	{
//		return pCursor->SmallFrame >= 0 ? AllowMini : DisAllowMini;
//	}
//
//	return nAction > (int)Action::PsychicReveal ? DisAllowMini : ReConsiderAllowMini;
//}


// RadarClass::RTacticalClass::Action - Clean refactored from 0x6539D0
// With Ares/Phobos-style hooks integrated inline.

#include <GadgetClass.h>
#include <MapClass.h>
#include <GameOptionsClass.h>
#include <PlanningTokenClass.h>
#include <WWKeyboardClass.h>
#include <WWMouseClass.h>

/**
 * Determines ActionType for a radar click based on targeting mode and selection.
 * Returns false if action is unresolvable (mouse should reset to default).
 */
static bool Resolve_Radar_Action(
	SuperWeaponType targetingMode,
	bool hasSelection,
	CellStruct& cell,
	ObjectClass* pObject,
	Action& outAction)
{
	switch (targetingMode)
	{
	case SuperWeaponType::Nuke:  outAction = Action::Nuke; return true;
	case SuperWeaponType::IronCurtain:  outAction = Action::IronCurtain; return true;
	case SuperWeaponType::LightningStorm:  outAction = Action::LightningStorm; return true;
	case SuperWeaponType::ParaDrop:  outAction = Action::ParaDrop; return true;
	case SuperWeaponType::AmerParaDrop:  outAction = Action::AmerParaDrop; return true;
	case SuperWeaponType::PsychicDominator:  outAction = Action::PsychicDominator; return true;
	case SuperWeaponType::SpyPlane:  outAction = Action::SpyPlane; return true;
	case SuperWeaponType::GeneticMutator:  outAction = Action::GeneticConverter; return true;
	case SuperWeaponType::PsychicReveal: outAction = Action::PsychicReveal; return true;
	default: break;
	}

	if (!hasSelection)
		return false;

	if (pObject) {
		outAction = DisplayClass::SelectLeadingTechno(nullptr, pObject)->MouseOverObject(pObject, false);
	}
	else {
		outAction = DisplayClass::SelectLeadingTechno(&cell, nullptr)->MouseOverCell(cell, false, false);
	}

	return true;
}

/**
 * Validates that the given action is allowed on the radar minimap.
 *
 * Extended validation (replaces vanilla jump table at 0x653CA3):
 *   - First checks custom cursor data via MouseClassExt. If the action has
 *     a registered cursor with SmallFrame >= 0, it is allowed on the minimap.
 *   - Falls back to the vanilla known-action set for standard actions.
 *
 * @param action     The action to validate.
 * @param pObject    Object ref, zeroed out if action is disallowed.
 * @return true if the action is allowed on the minimap.
 *
 * Corresponds to: ASMJIT_PATCH(0x653CA6, RadarClass_GetMouseAction_AllowMinimap)
 */
static bool Is_Valid_Radar_Action(Action action, ObjectClass*& pObject)
{
	// Hook: RadarClass_GetMouseAction_AllowMinimap @ 0x653CA6
	// Check if a custom cursor exists for this action.
	if (const MouseCursor* pCursor =
			MouseClassExt::GetCursorDataFromRawAction(action))
	{
		if (pCursor->SmallFrame >= 0)
			return true;

		// Cursor exists but has no minimap frame — disallow.
		pObject = nullptr;
		return false;
	}

	// No custom cursor data — fall back to vanilla known-action set.
	// Actions beyond PsychicReveal with no cursor data are disallowed.
	if ((int)action > (int)Action::PsychicReveal)
	{
		pObject = nullptr;
		return false;
	}

	// Original vanilla switch cases.
	switch (action)
	{
	case Action::Move:
	case Action::NoMove:
	case Action::Enter:
	case Action::Attack:
	case Action::Harvest:
	case Action::Capture:
	case Action::Sabotage:
	case Action::GuardArea:
	case Action::Nuke:
	case Action::LightningStorm:
	case Action::ParaDrop:
	case Action::AmerParaDrop:
	case Action::SpyPlane:
	case Action::PsychicDominator:
	case Action::PsychicReveal:
		return true;
	default:
		pObject = nullptr;
		return false;
	}
}

/**
 * Clamps a cell position so the tactical view stays within isometric map bounds.
 */
static void Clamp_Radar_Scroll_Cell(CellStruct& cell)
{
	int viewCellsWide = DSurface::ViewBounds->Width / 60 + 2;
	int viewCellsHigh = DSurface::ViewBounds->Height / 60;
	int halfWidth = viewCellsWide / 2;

	int maxDiagDiff = MapClass::MapSize->Width - halfWidth - 1;
	int minDiagSum = viewCellsHigh + MapClass::MapSize->Width + 1;
	int maxDiagSum = 2 * MapClass::MapSize->Height - viewCellsHigh
		+ MapClass::MapSize->Width - 1;

	short cx = cell.X;
	short cy = cell.Y;

	int diff = cy - cx;
	if (diff > maxDiagDiff)
	{
		short adjust = (short)(diff - maxDiagDiff);
		cy -= adjust;
		cx += adjust;
	}

	int revDiff = cx - cy;
	if (revDiff > maxDiagDiff - 1)
	{
		short adjust = (short)(revDiff - maxDiagDiff + 1);
		cy += adjust;
		cx -= adjust;
	}

	int sum = cx + cy;
	if (sum < minDiagSum)
	{
		short adjust = (short)(minDiagSum - sum);
		cx += adjust;
		cy += adjust;
	}

	sum = cx + cy;
	if (sum > maxDiagSum)
	{
		short adjust = (short)(sum - maxDiagSum);
		cx -= adjust;
		cy -= adjust;
	}

	cell.X = cx;
	cell.Y = cy;
}

// ====================================================================

int __fastcall  RadarClass_RTacticalClass_Action(GadgetClass* pThis, discard_t
	, GadgetFlag flags, WWKey* key, KeyModifier a4)
{
	if (flags & (GadgetFlag::RightHeld | GadgetFlag::LeftHeld))
		return 0;

	if (RadarClass::Radar_control() == 3 && RadarClass::Radar_controltype() == 1)
		return 0;

	if (RadarClass::Radar_control() != 1 || RadarClass::Radar_controltype() != 1)
		return 0;

	/*
	** Set some working variables that depend on the mouse position.
	** For press/release events, use the queued mouse position.
	** For other events (hover), use the live mouse position.
	*/
	int mouseX, mouseY;
	if (flags & (GadgetFlag::RightRelease | GadgetFlag::RightPress | GadgetFlag::LeftRelease | GadgetFlag::LeftPress))
	{
		mouseX = WWKeyboardClass::Instance->MouseQX;
		mouseY = WWKeyboardClass::Instance->MouseQY;
	}
	else
	{
		mouseX = WWMouseClass::Instance->GetX();
		mouseY = WWMouseClass::Instance->GetY();
	}

	const int radarX = mouseX
		- (GameOptionsClass::Instance->SidebarMode ? DSurface::ViewBounds->Width : 0);
	if (radarX < RadarClass::Radar_Rect149C->X
		|| radarX >= RadarClass::Radar_Rect149C->X + RadarClass::Radar_Rect149C->Width
		|| mouseY < RadarClass::Radar_Rect149C->Y
		|| mouseY >= RadarClass::Radar_Rect149C->Y + RadarClass::Radar_Rect149C->Height)
	{
		MouseClass::Instance->MouseClass::UpdateCursor(MouseCursorType::Default, false);
		return 0;
	}

	Point2D packedPos = { radarX, mouseY };
	CellStruct cell = { 0, 0 };
	ObjectClass* pObject = nullptr;

	MapClass::Instance->Click_Cell_Calc_In_Radar(&packedPos, &cell, &pObject);

	if (!cell.IsValid())
	{
		pThis->GadgetClass::Action(GadgetFlag::None, key, KeyModifier::None);
		return 1;
	}

	CoordStruct cellCoord = CellClass::Cell2Coord(cell);
				cellCoord.Z = MapClass::Instance->GetCellFloorHeight(cellCoord);

	const bool isMapped = MapClass::Instance->IsLocationShrouded(cellCoord) && Game::hWnd() != nullptr;

	// ---------------------------------------------------------------
	// Hook: RadarClass_GetMouseAction_CustomSWAction @ 0x653B3A
	//
	// If a custom superweapon is active, intercept early:
	//   - Right-click → skip to scroll handler.
	//   - Otherwise query the SW extension for its radar action.
	//   - If the SW provides a valid action, use it directly
	//     (bypassing the normal targeting mode resolution).
	//   - If Action::None, fall through to vanilla logic.
	// ---------------------------------------------------------------
	bool doScroll = (flags & (GadgetFlag::RightRelease | GadgetFlag::RightPress)) != GadgetFlag::None;
	bool leftActionHandled = false;
	bool customSWHandled = false;
	Action action = Action::None;

	int currentSWType = Unsorted::CurrentSWType();

	if (currentSWType >= 0) {
		if (flags & (GadgetFlag::RightPress | GadgetFlag::RightRelease))
		{
			// Custom SW active but right-click — go straight to scroll.
			doScroll = true;
		}
		else {
			Action swAction = SWTypeExtData::GetAction(
				SuperWeaponTypeClass::Array->Items[currentSWType],
				&cell);

			if (swAction != Action::None)
			{
				// Custom SW provided a specific action — use it directly,
				// skip normal targeting mode resolution.
				action = swAction;
				customSWHandled = true;
			}
			// else: swAction == None, fall through to vanilla resolution.
		}
	}

	if (!doScroll) {
		/*
		** Resolve the action. Either we already have one from the custom SW
		** hook above, or we go through normal targeting mode logic.
		*/
		if (!customSWHandled) {
			const bool hasSelection = ObjectClass::CurrentObjects->Count > 0;
			const bool resolved = Resolve_Radar_Action((SuperWeaponType)currentSWType,
				hasSelection,
				cell,
				pObject,
				action);

			if (!resolved) {
				MouseClass::Instance->MouseClass::SetCursor(MouseCursorType::Default, true);
				doScroll = true;
			}
		}

		if (!doScroll)
		{
			/*
			** Validate the action against the minimap-allowed set.
			** Extended by RadarClass_GetMouseAction_AllowMinimap hook
			** to support custom cursor actions.
			*/
			if (!Is_Valid_Radar_Action(action, pObject)) {
				action = Action::None;
			}

			bool blockedByPlanning =
				PlanningNodeClass::PlanningModeActive() && DisplayClass::PlanningManager_639DA0() != -1;

			if (action != Action::None && !blockedByPlanning) {
				if (flags & GadgetFlag::LeftUp) {
					DisplayClass::Instance->DisplayClass::ConvertAction(
						cell, isMapped, pObject, action, true);
				}

				if ((flags & GadgetFlag::LeftRelease) && !GadgetClass::DragSelectAborted()) {
					CoordStruct releaseCoord = CellClass::Cell2Coord(cell);
					DisplayClass::Instance->DisplayClass::LeftMouseButtonUp(
						releaseCoord, cell, pObject, action, 1);
				}

				leftActionHandled = true;
			} else {
				MouseClass::Instance->MouseClass::UpdateCursor(MouseCursorType::Default, true);
				doScroll = true;
			}
		}
	}

	/*
	** Scroll / reposition handler.
	*/
	if (doScroll && (flags & (GadgetFlag::RightPress | GadgetFlag::LeftPress))) {
		if (cell.IsValid()) {
			Clamp_Radar_Scroll_Cell(cell);
			CellClass* pCell = MapClass::Instance->GetCellAt(cell);
			CoordStruct scrollTarget = pCell->Cell2Coord();
			TacticalClass::Instance->SetTacticalPosition(&scrollTarget);
			MapClass::Instance->RedrawSidebar(1);

			if (ZBuffer::Instance() != nullptr) {
				ZBuffer::Instance->MaxValue = 0x8000;
			}
		}
	}

	pThis->GadgetClass::Action(GadgetFlag::None, key, KeyModifier::None);
	return 1;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6539D0, RadarClass_RTacticalClass_Action);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F0338, RadarClass_RTacticalClass_Action);