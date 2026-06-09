#include "Body.h"

#include <EventClass.h>

// DisplayClass::Mouse_Left_Release — backport + Phobos hooks integrated
// Vanilla range: 0x004AB9B0 – 0x004AC2A7
// All gotos eliminated. Hooks inlined at their assembly addresses.

// ─────────────────────────────────────────────────────────────────────────────
// EnqueueEvent — repeated inline pattern (0x4ABB5A / 0x4AC18C / 0x4AC23F / 0x4AC1B7)
// ─────────────────────────────────────────────────────────────────────────────
static void __forceinline EnqueueEvent(EventClass* ev)
{
	if (OutList.Count >= 0x80)
		return;
	OutList.List[OutList.Tail] = *ev;            // ORIG: rep movsd/movsw/movsb (0x6E bytes)
	OutList.Timings[OutList.Tail] = timeGetTime();
	OutList.Tail = (OutList.Tail + 1) & 0x7F;
	++OutList.Count;
}

// ─────────────────────────────────────────────────────────────────────────────
// PendingObjectIsNaval — RTDynamicCast + IsNaval check (0x4ABAC0–0x4ABAF4)
// ─────────────────────────────────────────────────────────────────────────────
static bool PendingObjectIsNaval(ObjectTypeClass* pType)
{
	auto* pTT = static_cast<TechnoTypeClass*>(
		__RTDynamicCast(pType, 0,
			&ObjectTypeClass_RTTIDescriptor,
			&TechnoTypeClass_RTTIDescriptor, 0));
	return pTT && pTT->IsNaval;   // ORIG: [eax+0CCEh]   VERIFY: IsNaval @ 0xCCE
}

// ─────────────────────────────────────────────────────────────────────────────
// IsDispatchableAction — 20+ cmp/jz chain (0x4ABEF9–0x4ABF8E) as a switch
// ─────────────────────────────────────────────────────────────────────────────
static bool IsDispatchableAction(ActionType action)
{
	switch (action)
	{
	case ACTION_EATEN:            // 0x08
	case ACTION_SELL:             // 0x0A
	case ACTION_SELL_UNIT:        // 0x0C
	case ACTION_NO_SELL:          // 0x0D
	case ACTION_NO_REPAIR:        // 0x0E
	case ACTION_TOGGLE_POWER:     // 0x0F
	case ACTION_NO_TOGGLE_POWER:  // 0x21
	case ACTION_NUKE_BOMB:        // 0x22
	case ACTION_IRON_CURTAIN:     // 0x14
	case ACTION_FORCE_SHIELD:     // 0x25
	case ACTION_NO_FORCE_SHIELD:  // 0x45
	case ACTION_LIGHTNING_STORM:  // 0x46
	case ACTION_CHRONOSPHERE:     // 0x26
	case ACTION_CHRONOWARP:       // 0x27
	case ACTION_PARADROP:         // 0x28
	case ACTION_AMERICAN_PARADROP:// 0x29
	case ACTION_PSYCHIC_DOMINATOR:// 0x41
	case ACTION_SPY_PLANE:        // 0x42
	case ACTION_GENETIC_CONVERTER:// 0x43
	case ACTION_PSYCHIC_REVEAL:   // 0x44
	case ACTION_SELECT_NODE:      // 0x48
	case ACTION_PLACE_BEACON:     // 0x3A
	case ACTION_SELECT_BEACON:    // 0x3C
		return true;
	default:
		// ORIG: PlanningManager_639040 && PlanningManager_639130
		return PlanningManager_639040() && PlanningManager_639130();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// CreateEvent — Phobos helper used by hook @ 0x4AC20C
// (replaces vanilla SuperWeaponTypeClass::From_Action + EventClass_SPECIALPLACE)
// ─────────────────────────────────────────────────────────────────────────────
void NOINLINE CreateEvent(SuperClass* pSuper, CellStruct* pWhere)
{
	EventClass Event {
		HouseClass::CurrentPlayer->ArrayIndex,
		EventType::SPECIAL_PLACE,
		pSuper->GetArrayIndex(),
		*pWhere
	};
	EventClass::AddEvent(&Event);
}

// ─────────────────────────────────────────────────────────────────────────────
// TryFireSuperWeaponEvent — SW tail (0x4AC20C–0x4AC28F)
// HOOK: ASMJIT_PATCH(0x4AC20C, DisplayClass_LeftMouseButtonUp, 7)
//   Replaces vanilla SuperWeaponTypeClass::From_Action with Phobos SW lookup.
//   Handles PhobosNewActionType::SuperWeaponDisallowed range.
//   Uses HouseClass::CurrentPlayer->Supers + Unsorted::CurrentSWType()
//   and SWTypeExtData::CurrentSWType for new-style SWs.
// ─────────────────────────────────────────────────────────────────────────────
static void TryFireSuperWeaponEvent(ActionType action, CellStruct* pCell)
{
	// [0x4AC20C] HOOK: DisplayClass_LeftMouseButtonUp
	if (action < static_cast<ActionType>(PhobosNewActionType::SuperWeaponDisallowed))
	{
		// Legacy SW: get the actual firing instance (not just the first type for this action),
		// so clones work. Verify the found SW's action still matches to avoid
		// triggering force shield when UI shows "no cursor".
		auto* pSW = HouseClass::CurrentPlayer->Supers.get_or_default(Unsorted::CurrentSWType());
		if (pSW && pSW->Type->Action != action)
			pSW = nullptr;
		if (pSW)
			CreateEvent(pSW, pCell);
		return;
	}

	if (action == static_cast<ActionType>(PhobosNewActionType::SuperWeaponDisallowed))
		return;

	// New-style SW (action >= SuperWeaponDisallowed+1): use SWTypeExtData cursor
	if (auto* pSW = SWTypeExtData::CurrentSWType)
		CreateEvent(pSW, pCell);
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildActionEvent — per-action event construction (0x4ABFCE–0x4AC0FC)
// HOOK inlined inside ACTION_TOGGLE_POWER case:
//   ASMJIT_PATCH(0x4ABFBE, DisplayClass_LeftMouseButtonUp_ExecPowerToggle, 7)
//   Guard: only proceed if Target is a Building owned by human player.
//   Vanilla checked Kind_Of==6 only; Phobos adds Owner->IsControlledByHuman().
// ─────────────────────────────────────────────────────────────────────────────
static bool BuildActionEvent(ActionType action, ObjectClass* pObj, CellStruct* pCell, EventClass* ev)
{
	switch (action)
	{
	case ACTION_TOGGLE_POWER:
	{
		// [0x4ABFBE] HOOK: DisplayClass_LeftMouseButtonUp_ExecPowerToggle
		// Vanilla: cmp Kind_Of, 6 — Phobos adds IsControlledByHuman() guard.
		auto* pTechno = static_cast<TechnoClass*>(pObj);
		if (!pTechno
			|| !pTechno->Owner->IsControlledByHuman()
			|| pTechno->WhatAmI() != AbstractType::Building)
		{
			return false;
		}
		// [0x4ABFCE] vanilla resumes here
		xTargetClass tgt(pObj);
		EventType evType = pObj->HasPower ? E_POWEROFF : E_POWERON;  // ORIG: [esi+660h]  VERIFY: HasPower @ 0x660
		EventClass::EventClass(ev, PlayerPtr->ID, evType, tgt.m_ID, tgt.m_RTTI);
		EnqueueEvent(ev);
		return true;
	}
	case ACTION_PLACE_BEACON:
	{
		if (!obiwanordebugshroudbeacon_A8B538)
		{
			auto* pCC = MapClass::operator[](Map, *pCell);
			int   lx = (pCell->X << 8) + 0x80;
			int   ly = (pCell->Y << 8) + 0x80;
			int   z = pCC->Level * Map_LeptonsPerCellZ;  // ORIG: [eax+11Bh]
			BeaconPlacement::Place(&Beacons, PlayerPtr->ID, lx, ly, z, -1);
		}
		DisplayClass::Beacon_Mode_Control(&Map, 0);
		Map.vtable->Set_Default_Mouse(&Map, MOUSE_NORMAL, false); // ORIG: [edx+48h]
		return false;
	}
	case ACTION_EATEN:
	{
		if (!pObj || pObj->GetKindOf() != RTTI_BUILDING)
			return false;
		xTargetClass tgt(pObj);
		EventClass::EventClass(ev, PlayerPtr->ID, E_REPAIR, tgt.m_ID, tgt.m_RTTI);
		EnqueueEvent(ev);
		return true;
	}
	case ACTION_SELL_UNIT:
	{
		if (!pObj)
			return false;
		int kind = pObj->GetKindOf();
		if (kind <= 0 || kind > 2)   // ORIG: jle / jg → skip
			return false;
		xTargetClass tgt(pObj);
		EventClass::EventClass(ev, PlayerPtr->ID, E_SELL, tgt.m_ID, tgt.m_RTTI);
		EnqueueEvent(ev);
		return true;
	}
	case ACTION_SELL:
	{
		if (pObj)
		{
			xTargetClass tgt(pObj);
			EventClass::EventClass(ev, PlayerPtr->ID, E_SELL, tgt.m_ID, tgt.m_RTTI);
		}
		else
		{
			EventClass::EventClass_SELLCELL(ev, PlayerPtr->ID, E_SELLCELL, pCell);
		}
		EnqueueEvent(ev);
		return true;
	}
	default:
		return false;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// DoActiveClick — shared sink for all paths reaching loc_4ABF94
// ─────────────────────────────────────────────────────────────────────────────
static void DoActiveClick(
	DisplayClass* pThis,
	ObjectClass* pObj,
	CellStruct* cell,
	ActionType    action,
	EventClass* ev)
{
	DisplayClass___Active_Click_With(pThis, pObj, *cell, action);
	Reset_Action_Line_Timer();
	BuildActionEvent(action, pObj, cell, ev);
	TryFireSuperWeaponEvent(action, cell);
}

// ─────────────────────────────────────────────────────────────────────────────
// GetSelectionGroupID — Phobos type-select group lookup
// Used by hook @ 0x4ABD6C / 0x4ABD9D / 0x4ABE58
// HOOK: ASMJIT_PATCH(0x4ABD6C, DisplayClass_LeftMouseButtonUp_GroupAs, 0xA)
//       ASMJIT_PATCH_AGAIN(0x4ABD9D, ...)
//       ASMJIT_PATCH_AGAIN(0x4ABE58, ...)
//   Vanilla: Class_Of() + offset 0x24 (IniName ptr) passed to type-select fns.
//   Phobos: replaces that with TechnoTypeExtData::GetSelectionGroupID.
// ─────────────────────────────────────────────────────────────────────────────
static const char* GetSelectionGroupName(ObjectClass* pObj)
{
	// [0x4ABD6C / 0x4ABD9D / 0x4ABE58] HOOK: DisplayClass_LeftMouseButtonUp_GroupAs
	// Vanilla did: pObj->GetTypeClass() + 0x24 (IniName)
	// Phobos replaces with group ID lookup so grouped types share type-select.
	return TechnoTypeExtData::GetSelectionGroupID(pObj->GetType());
}

// ─────────────────────────────────────────────────────────────────────────────
// DoSelectLogic — shared for ACTION_SELECT / ACTION_NONE (loc_4ABE18)
//
// Hooks integrated:
//   [0x4ABE3C] ASMJIT_PATCH(DisplayClass_MouseLeftRelease_Cloak, 0xA)
//     Vanilla: cmp [esi+6EDh], 0Fh → if enemy building in cloaked state, jump to Unselect.
//     Phobos: also allows observers and mutual allies to select cloaked technos.
//
//   [0x4ABD6C / 0x4ABD9D / 0x4ABE58] GroupAs hooks applied via GetSelectionGroupName().
//
// Returns true → caller proceeds to DoActiveClick.
// Returns false → caller clears tentative and returns.
// ─────────────────────────────────────────────────────────────────────────────
static bool DoSelectLogic(ObjectClass* pObj, ActionType action, DisplayClass* pThis, bool wsmall)
{
	bool canSelect = true;

	// [0x4ABE18–0x4ABE48] Enemy-building cloaked-state guard
	if (pObj && pObj->GetKindOf() == RTTI_BUILDING)
	{
		HouseClass* pHouse = static_cast<BuildingClass*>(pObj)->House;
		bool enemyOwned = !pHouse->Player_Has_Control();
		bool cloakedState = (pObj->Visual_Character_6ED == 0x0F); // VERIFY: offset 0x6ED

		if (enemyOwned && cloakedState)
		{
			// [0x4ABE3C] HOOK: DisplayClass_MouseLeftRelease_Cloak
			// Vanilla: call vtable[0x328] → if returns false, jump to Unselect.
			// Phobos: allow observers + mutual allies + sensor-visible to bypass unselect.
			bool allowSelect = false;
			if (HouseClass::IsCurrentPlayerObserver())
			{
				allowSelect = true;
			}
			else
			{
				auto* pTechno = static_cast<TechnoClass*>(pObj);
				auto* pOwner = pTechno->Owner;
				if (pOwner && HouseExtData::IsMutualAllies(pOwner, HouseClass::CurrentPlayer))
					allowSelect = true;
				else if (pTechno->IsSensorVisibleToPlayer())
					allowSelect = true;
			}

			if (!allowSelect)
			{
				// [0x4ABE88] Unselect branch — vanilla jumped here on cloaked enemy building
				// that failed the vtable[0x328] check
				canSelect = false;
			}
			// if allowSelect: fall through to canSelect=true (Phobos extension)
		}
	}

	if (canSelect)
	{
		Unselect_All();

		if (Is_Type_Selecting())
		{
			// [0x4ABE58] HOOK: DisplayClass_LeftMouseButtonUp_GroupAs
			// Vanilla: pObj->GetTypeClass() + 0x24
			// Phobos: GetSelectionGroupID
			UICommands_TypeSelect_7327D0(GetSelectionGroupName(pObj));
		}
		else
		{
			pObj->vtable->Select(pObj); // ORIG: [eax+14Ch]
		}

		Map.vtable->Set_Default_Mouse(&Map, MOUSE_NORMAL, false); // ORIG: [edx+48h]
		Reset_Action_Line_Timer();
	}

	return IsDispatchableAction(action);
}

// ─────────────────────────────────────────────────────────────────────────────
//  DisplayClass::Mouse_Left_Release
//  0x004AB9B0 – 0x004AC2A7
// ─────────────────────────────────────────────────────────────────────────────
void __thiscall DisplayClass::Mouse_Left_Release(
	Point2D* xy,
	CellStruct* cell,
	ObjectClass* object,
	ActionType   action,
	bool         wsmall)
{
	// §1  Stealth-object nullification (0x4AB9B0–0x4AB9EF)
	// ORIG: SpecialFlag && GameActive && !Debug_Map && Visual_Character==VISUAL_HIDDEN(5)
	ObjectClass* pObj = object;
	if (pObj
		&& SpecialFlag
		&& GameActive
		&& !Debug_Map_DEBUGDEBUG
		&& pObj->vtable->Visual_Character(pObj, false, false) == VISUAL_HIDDEN)
	{
		pObj = nullptr;
	}

	EventClass ev; // stack buffer for event construction (ORIG: var_70)

	// ══════════════════════════════════════════════════════════════════════════
	// §2  PLACE path — PendingObjectPtr is set (0x4AB9F1–0x4ABC73)
	// ══════════════════════════════════════════════════════════════════════════
	if (this->PendingObjectPtr)
	{
		// §2a  Proximity check for building placement (0x4ABA05–0x4ABA64)
		if (this->PendingObject->GetKindOf() == RTTI_BUILDINGTYPE)
		{
			// [0x4ABA47] HOOK: ASMJIT_PATCH(DisplayClass_PreparePassesProximityCheck_ReplaceBuildingType, 0x6)
			// Phobos: before Passes_Proximity_Check, redirect CurrentBuildingType to
			// the "another placing type" (multi-building placement extension) or to the
			// existing building's type if we're upgrading, so the foundation is correct.
			{
				auto* pDisplay = DisplayClass::Instance();
				if (const auto pAnotherType = GetAnotherPlacingType(pDisplay))
				{
					if (pDisplay->CurrentBuildingType && pDisplay->CurrentBuildingType != pAnotherType)
					{
						pDisplay->CurrentBuildingType = pAnotherType;
						pDisplay->SetActiveFoundation(pAnotherType->GetFoundationData(true));
					}
				}
				else if (const auto pCurrentBuilding = cast_to<BuildingClass*>(pDisplay->CurrentBuilding))
				{
					if (pDisplay->CurrentBuildingType
						&& pDisplay->CurrentBuildingType != pCurrentBuilding->Type)
					{
						pDisplay->CurrentBuildingType = pCurrentBuilding->Type;
						pDisplay->SetActiveFoundation(pCurrentBuilding->Type->GetFoundationData(true));
					}
				}
			}

			CellStruct trycell;
			trycell.X = short(this->ZoneOffset.X + cell->X);
			trycell.Y = short(this->ZoneOffset.Y + cell->Y);
			this->__PassedProximityCheck = Passes_Proximity_Check(
				this->PendingObject, this->PendingHouse, this->CursorSize, &trycell);
		}

		// §2b  Can_Upgrade override (0x4ABA64–0x4ABA9D)
		if (this->PendingObject->GetKindOf() == RTTI_BUILDINGTYPE
			&& pObj
			&& pObj->GetKindOf() == RTTI_BUILDING
			&& BuildingClass::Can_Upgrade(pObj, this->PendingObject, PlayerPtr))
		{
			this->__PassedProximityCheck = true;
		}

		// §2c  Proximity fail → EVA bark + bail (0x4ABAAC–0x4ABC91)
		if (!this->__PassedProximityCheck || !this->__PassedProximityShroudCheck)
		{
			VoxClass_Speak_From_Name("EVA_CannotDeployHere", -1, -1);
			FlagAsNotSelected();
			return;
		}

		// §2d  Naval flag (0x4ABAC0–0x4ABAF4)
		bool isNaval = PendingObjectIsNaval(this->PendingObject);

		// §2e  Build + enqueue PLACE event (0x4ABAF5–0x4ABBB2)
		{
			CellStruct placecell;
			placecell.X = short(this->ZoneOffset.X + cell->X);
			placecell.Y = short(this->ZoneOffset.Y + cell->Y);
			auto* pType = this->PendingObject;
			int   heapID = pType->vtable->Get_Heap_ID(pType);  // ORIG: [eax+40h]
			RTTIType kind = this->PendingObjectPtr->GetKindOf(); // ORIG: [eax+2Ch]
			EventClass::EventClass_PLACE(&ev, PlayerPtr->ID, E_PLACE, kind, heapID,
										 isNaval ? 1 : 0, &placecell);
			EnqueueEvent(&ev);
		}

		// §2f  Clear sidebar tab (0x4ABBB3–0x4ABBD6)
		{
			ObjectClass* sidebarObj =
				(pObj && pObj->GetKindOf() == RTTI_BUILDING) ? pObj : nullptr;
			Clear_Sidebar_Tab_Object(sidebarObj);
		}

		// [0x4ABBD5] HOOK: DEFINE_JUMP(LJMP, 0x4ABBD5, 0x4ABBDC)
		// issue #250: Building placement hotkey not responding (Author: Uranusian)
		// Vanilla fell through from Clear_Sidebar_Tab_Object into a block that read
		// Debug_Map_DEBUGDEBUG and branched; the jump skips 7 bytes of dead setup
		// so the debug-map check is the next thing executed — same as the fix below.

		// §2g  Debug-map → skip cursor setup, just flag and return (0x4ABBE3)
		if (!Debug_Map_DEBUGDEBUG)
		{
			// §2h  Commit pending-2 state + cursor shapes (0x4ABBE9–0x4ABC4C)
			this->__PendingObjectTypePtr2 = this->PendingObject;
			this->__PendingObjectPtr2 = this->PendingObjectPtr;
			this->ZoneCell2 = *cell;
			this->ZoneOffset2 = this->ZoneOffset;
			this->CursorSize2 = this->CursorSize;
			this->__PendingObjectHouse2 = this->PendingHouse;

			if (this->__PendingObjectTypePtr2)
			{
				// ORIG: [eax+90h] = Occupy_List(1)
				const CellStruct* pShape = this->__PendingObjectTypePtr2
					->vtable->Occupy_List(this->__PendingObjectTypePtr2, true);
				DisplayClass::Set_Cursor_Shape2(&Map, pShape);
			}
			DisplayClass::Set_Cursor_Shape(&Map, nullptr);

			// §2i  Clear pending globals (0x4ABC51–0x4ABC73)
			Display_PendingObjectPtr = nullptr;
			Display_PendingObject = nullptr;
			Display_PendingHouse = -1;
			FlagAsNotSelected();
			return;
		}

		FlagAsNotSelected();
		return;
	}

	// ══════════════════════════════════════════════════════════════════════════
	// §3  No-placement path (0x4ABC94–0x4AC2A7)
	// ══════════════════════════════════════════════════════════════════════════

	// §3a  Rubber-band drag-select (0x4ABC94–0x4ABD17)
	if (this->IsRubberBand)
	{
		TacticalMap->__TacticalPosUpdated__IsToRedraw = true; // ORIG: [ecx+0D7Dh]=1

		bool shiftHeld = Keyboard->Down(VK_SHIFT); // ORIG: push 0x10

		if (!shiftHeld)
		{
			if (Tactical::Is_Anything_Selected(TacticalMap))
				Unselect_All();
			else
			{
				// Nothing selected + no shift → make selection then flag-only return
				// [0x4ABCEB] HOOK: DEFINE_FUNCTION_JUMP(CALL, 0x4ABCEB, FakeTacticalClass::Tactical_MakeFilteredSelection)
				// Replaces vanilla Tactical::Make_Selection call with Phobos filtered version.
				FakeTacticalClass::Tactical_MakeFilteredSelection(TacticalMap, Is_Selectable);
				Reset_Action_Line_Timer();
				this->IsRubberBand = false;
				Map.vtable->Set_Default_Mouse(&Map, MOUSE_NORMAL, false); // ORIG: [edx+48h]
				this->IsTentative = false;
				DragSelectAborted = true;
				FlagAsNotSelected();
				return;
			}
		}

		// [0x4ABCEB] HOOK: DEFINE_FUNCTION_JUMP(CALL, 0x4ABCEB, FakeTacticalClass::Tactical_MakeFilteredSelection)
		FakeTacticalClass::Tactical_MakeFilteredSelection(TacticalMap, Is_Selectable);
		Reset_Action_Line_Timer();
		this->IsRubberBand = false;
		Map.vtable->Set_Default_Mouse(&Map, MOUSE_NORMAL, false);
		this->IsTentative = false;
		DragSelectAborted = true;
		// fall through to action dispatch
	}

	// §3b  ACTION_TOGGLE_SELECT (0x4ABD1D–0x4ABDC0)
	if (action == ACTION_TOGGLE_SELECT)
	{
		bool handled = false;
		if (pObj
			&& CurrentObject.ActiveCount > 0
			&& CurrentObject.Vector[0]->GetHouse()->Player_Has_Control()) // ORIG: [edx+3Ch]+Player_Has_Control
		{
			if (pObj->IsSelected) // ORIG: [esi+83h]
			{
				if (Is_Type_Selecting())
				{
					// [0x4ABD6C] HOOK: DisplayClass_LeftMouseButtonUp_GroupAs
					// Vanilla: pObj->GetTypeClass() + 0x24 (IniName)
					// Phobos: TechnoTypeExtData::GetSelectionGroupID
					Mouse_Left_Release_732600(GetSelectionGroupName(pObj));
				}
				else
				{
					pObj->vtable->Unselect(pObj); // ORIG: [edx+150h]
				}
			}
			else
			{
				if (Is_Type_Selecting())
				{
					// [0x4ABD9D] HOOK: ASMJIT_PATCH_AGAIN(DisplayClass_LeftMouseButtonUp_GroupAs)
					UICommands_TypeSelect_7327D0(GetSelectionGroupName(pObj));
				}
				else
				{
					pObj->vtable->Select(pObj); // ORIG: [edx+14Ch]
				}
			}
			handled = true;
		}

		if (handled)
		{
			DoActiveClick(this, pObj, cell, action, &ev);
			this->IsTentative = false;
			FlagAsNotSelected();
			return;
		}

		action = ACTION_SELECT; // ORIG: mov ebx,7 / mov [action],ebx
	}

	// §3c  ACTION_SELECT (0x4ABDD8 + loc_4ABE18 path)
	if (action == ACTION_SELECT)
	{
		if (!pObj)
		{
			this->IsTentative = false;
			FlagAsNotSelected();
			return;
		}

		bool doClick = DoSelectLogic(pObj, action, this, wsmall);
		if (doClick)
			DoActiveClick(this, pObj, cell, action, &ev);

		this->IsTentative = false;
		FlagAsNotSelected();
		return;
	}

	// §3d  ACTION_NONE (0x4ABDE1–0x4ABE17)
	if (action == ACTION_NONE)
	{
		bool clearOnly =
			!pObj
			|| !pObj->vtable->Is_Selectable1(pObj)  // ORIG: [eax+138h]
			|| pObj->IsSelected;                     // ORIG: [esi+83h]

		if (!clearOnly)
		{
			bool doClick = DoSelectLogic(pObj, action, this, wsmall);
			if (doClick)
				DoActiveClick(this, pObj, cell, action, &ev);
		}

		this->IsTentative = false;
		FlagAsNotSelected();
		return;
	}

	// §3e  ACTION_SELECT_BEACON (0x4ABE88–0x4ABEDC)
	if (action == ACTION_SELECT_BEACON)
	{
		auto* pCC = MapClass::operator[](Map, *cell);
		int   lx = (cell->X << 8) + 0x80;
		int   ly = (cell->Y << 8) + 0x80;
		int   z = pCC->Level * Map_LeptonsPerCellZ; // ORIG: [eax+11Bh]
		BeaconPlacement_430F70(&Beacons, lx, ly, z);
		this->IsTentative = false;
		FlagAsNotSelected();
		return;
	}

	// §3f  Non-dispatchable actions → clear and return (0x4ABEE1–0x4ABF8E)
	if (!action
		|| action == ACTION_SELECT
		|| !IsDispatchableAction(action))
	{
		this->IsTentative = false;
		FlagAsNotSelected();
		return;
	}

	// §3g  Dispatchable: Active_Click_With + event tail (0x4ABF94–0x4AC294)
	DoActiveClick(this, pObj, cell, action, &ev);
	this->IsTentative = false;
	FlagAsNotSelected();
}