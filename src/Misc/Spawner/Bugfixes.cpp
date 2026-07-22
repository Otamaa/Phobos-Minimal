/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include <Utilities/Macro.h>
#include <Phobos.h>
#include <Ext/Techno/Body.h>

#include <EBolt.h>

#pragma region GetTechnoType

// Avoid secondary jump
DEFINE_JUMP(VTABLE, 0x7E2328, 0x41C200) // AircraftClass_GetTechnoType -> AircraftClass_GetType
DEFINE_JUMP(VTABLE, 0x7E3F40, 0x459EE0) // BuildingClass_GetTechnoType -> BuildingClass_GetType
DEFINE_JUMP(VTABLE, 0x7EB0DC, 0x51FAF0) // InfantryClass_GetTechnoType -> InfantryClass_GetType
DEFINE_JUMP(VTABLE, 0x7F5CF4, 0x741490) // UnitClass_GetTechnoType -> UnitClass_GetType

#pragma endregion

// Open campaign briefing when pressing Tab
ASMJIT_PATCH(0x55E08F, KeyboardProcess_PressTab, 0x5)
{
	Game::SpecialDialog = SessionClass::IsCampaign() ? 9 : 8;
	return 0x55E099;
}

// Prevents accidental exit when pressing the spacebar while waiting
// Remove focus from the Leave Game button in the player waiting window
ASMJIT_PATCH(0x648CCC, WaitForPlayers_RemoveFocusFromLeaveGameBtn, 0x6)
{
	Imports::SetArchiveTarget.invoke()(0);
	return 0;
}

// DECLARE_PATCH(WaitForPlayers_RemoveFocusFromLeaveGameBtn){
// 	reinterpret_cast<Imports::FP_SetFocus>(0x7E13CC)(0);
// 	_asm { mov ecx, 0x00887640 };
// 	_asm { mov edx, [ecx]};
// 	_asm { call dword ptr[edx + 0x1C]};
// 	JMP_REG(ecx , 0x648CD2);
// }
// DEFINE_FUNCTION_JUMP(LJMP, 0x648CCC ,WaitForPlayers_RemoveFocusFromLeaveGameBtn)

// A patch to prevent framerate drops when a player spams the 'type select' key
// Skip call GScreenClass::FlagToRedraw(1)
DEFINE_JUMP(LJMP, 0x732CED, 0x732CF9); // End_Type_Select_Command

// DECLARE_PATCH(WaitForPlayers_OnlineOptimizations)
// {
// 	reinterpret_cast<Imports::FP_Sleep>(0x7E11F0)(3);
//     JMP(0x6488B0);
// }
// DEFINE_FUNCTION_JUMP(LJMP, 0x649851,WaitForPlayers_OnlineOptimizations)

ASMJIT_PATCH(0x649851, WaitForPlayers_OnlineOptimizations, 0x5)
{
	Imports::Sleep.invoke()(3);// Sleep yields the remaining CPU cycle time to any other processes
	return 0x6488B0;
}

// Otamaa : these block of code seems not really nessesary
//			all the function output were abandoned anyway
// Fix crash at 6F9DB6
 //DEFINE_PATCH_TYPED(BYTE, 0x5F5893
 //	, 0x83, 0xFB, 0x01 // cmp ebx 1
 //	, 0x74, 0x54 // je 0x5F58EC
 //	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
 //	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
 //	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
 //	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
 //	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
 //	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
 //	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
 //	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
 //);

 //ObjectClass_Mark
 DEFINE_JUMP(LJMP, 0x5F5896 , 0x5F58E1);
//  ASMJIT_PATCH(0x5F5893, ObjectClass_Mark_Unessesarycalls, 0x5) {
//  	return R->EBX<int>() == 1 ? 0x5F58EC : 0x5F58E7;
//  }

// Fix crash at 727B48
ASMJIT_PATCH(0x727B44, TriggerTypeClass_ComputeCRC_FixCrash, 0x6)
{
	GET(HouseTypeClass*, pHouseType, EAX);
	return pHouseType ? 0 : 0x727B55;
}

// Fix crash at 6F49DE
ASMJIT_PATCH(0x6F49D8, TechnoClass_Revealed_FixCrash, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return pHouse ? 0 : 0x6F4A31;
}

// Extend IsoMapPack5 decoding size limit
// (Large map support)

// When big sized maps with high details cross about 9750 + few lines in
// IsoMapPack5 section, game doesn't decode those and fills those (typically
// bottom-left corner of the map) with height level 0 clear tiles.
// This patch raises the buffer usage limit to about 3 times the original.
// From 640 (0x280), 400 (0x190) and value of 512000 (= 640 * 400 * 2)
// To 1024 (0x400), 768 (0x300) and 1572864 (= 1024 * 768 * 2).

// Author: E1 Elite

DEFINE_PATCH_TYPED(DWORD, 0x4AD344, 0x300); // 0x190
DEFINE_PATCH_TYPED(DWORD, 0x4AD349, 0x400); // 0x280
DEFINE_PATCH_TYPED(DWORD, 0x4AD357, (0x300 * 0x400 * 2));

ASMJIT_PATCH(0x454174, BuildingClass_Load_SwizzleLighsource, 0xA)
{
	GET(BuildingClass*, pThis, EDI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->LightSource);

	return 0x45417E;
}

ASMJIT_PATCH(0x50C8F4, HouseClass_Flag_To_Chear_Disable ,0x5)
{
	if ((SessionClass::Instance->GameMode == GameMode::LAN && !Game::LANTaunts())
		|| (SessionClass::Instance->GameMode == GameMode::Internet && !Game::WOLTaunts())){
		return 0x50C910;
	}

	return 0x0;
}


#include <DisplayClass.h>
#include <TacticalClass.h>
#include <Ext/Tactical/Body.h>

DEFINE_FUNCTION_JUMP(LJMP, 0x6D8640, FakeTacticalClass::__ClampTacticalPos)

//Start_Mouse_Thread
DEFINE_PATCH_ADDR_OFFSET(DWORD , 0x7B8536, 6, 1);

#pragma region InfBlockTreeFix
#include <Ext/Cell/Body.h>

ASMJIT_PATCH(0x52182A, InfantryClass_MarkAllOccupationBits_SetOwnerIdx, 0x6)
{
	GET(CellClass*, pCell, ESI);

	//avoid invalid cell
	if(auto pExt = CellExtContainer::Instance.TryFind(pCell))
		pExt->InfantryCount++;

	return 0;
}

ASMJIT_PATCH(0x5218C2, InfantryClass_UnmarkAllOccupationBits_ResetOwnerIdx, 0x6)
{
	enum { Reset = 0x5218CC, NoReset = 0x5218D3 };

	GET(CellClass*, pCell, ESI);
	GET(DWORD, newFlag, ECX);

	pCell->OccupationFlags = newFlag;
	bool noInfantry = false;

	if (auto pExt = CellExtContainer::Instance.TryFind(pCell)){
		noInfantry = pExt->InfantryCount-- == 0;
	}

	// Vanilla check only the flag to decide if the InfantryOwnerIndex should be reset.
	// But the tree take one of the flag bit. So if a infantry walk through a cell with a tree, the InfantryOwnerIndex won't be reset.
	return (newFlag & 0x1C) == 0 || noInfantry ? Reset : NoReset;
}

#pragma endregion
/*
#include <Locomotor/TeleportLocomotionClass.h>

*/
#include <Ext/WeaponType/Body.h>

ASMJIT_PATCH(0x6F755A, TechnoClass_IsCloseEnough_CylinderRangefinding, 0x7)
{
	GET_BASE(WeaponTypeClass* const, pWeaponType, 0x10);
	GET(CoordStruct* const, pCoord, ESI);
	GET(TechnoClass* const, pThis, EDI);
	const bool cylinder = WeaponTypeExtContainer::Instance.Find(pWeaponType)->CylinderRangefinding.Get(FakeRulesClass::Instance()->CylinderRangefinding);
	R->EAX(pCoord->X);
	return (cylinder || pThis->WhatAmI() == AbstractType::Aircraft) ? 0x6F75B2 : 0x6F7568;
}

ASMJIT_PATCH(0x71153C, TechnoTypeClass_DefaultToGuardArea_GlobalDefault, 0x6)
{
	GET(TechnoTypeClass*, pThis, ESI);

	if (FakeRulesClass::Instance()->DefaultToGuardArea.isset())
		pThis->DefaultToGuardArea = FakeRulesClass::Instance()->DefaultToGuardArea.Get();
	else
		pThis->DefaultToGuardArea = false;

	pThis->LeptonMindControlOffset = FakeRulesClass::Instance->LeptonMindControlOffset;
	pThis->MindControlRingOffset = FakeRulesClass::Instance->MindControlRingOffset;

	return 0x711542;
}

// ============================================================
// Aircraft shadow desync fix
// ============================================================
// AircraftClass::Draw_It temporarily calls Set_Height(0) before drawing the
// shadow, which triggers MapClass::Place_Down and Pick_Up, which call
// RecalcAttributes. This only runs for players who can see the aircraft,
// causing LandType to diverge based on fog-of-war state.
// Fix: no-op all three Set_Height calls during shadow drawing.
// 0x5F4300 = ObjectClass::Record_The_Kill_House (empty body, safe NOP target).
DEFINE_JUMP(CALL6, 0x4147D5, 0x5F4300); // Set_Height bridge path
DEFINE_JUMP(CALL6, 0x4147F3, 0x5F4300); // Set_Height non-bridge path
DEFINE_JUMP(CALL6, 0x4148AB, 0x5F4300); // Set_Height restore

// ============================================================
// Crate removal RecalcAttributes
// ============================================================

// Single-player crate removal path.
ASMJIT_PATCH(0x4A1BEF, CrateClass_Get_Crate_RecalcAttributes, 0x6)
{
	GET(CellClass*, pCell, EBX);
	pCell->RecalcAttributes(DWORD(-1));
	return 0;
}

// Multiplayer crate removal path.
ASMJIT_PATCH(0x56C1DA, MapClass_Remove_Crate_RecalcAttributes, 0x6)
{
	GET(CellClass*, pCell, EBX);
	pCell->RecalcAttributes(DWORD(-1));
	return 0;
}

// ============================================================
// Flag removal RecalcAttributes
// ============================================================

// HouseClass::Flag_Remove: clears the flag home cell overlay without RecalcAttributes.
ASMJIT_PATCH(0x4FBF3C, HouseClass_Flag_Remove_RecalcAttributes, 0x5)
{
	GET(CellClass*, pCell, EAX);
	pCell->RecalcAttributes(DWORD(-1));
	return 0;
}

// ============================================================
// Veinhole constructor RecalcAttributes
// ============================================================

// VeinholeMonsterClass constructor directly writes SlopeIndex, IsoTileTypeIndex,
// Height, and Level for a 3x3 cell grid without calling RecalcAttributes.
// Hook after the last per-cell write (Level) each iteration to fix all 9 cells.
ASMJIT_PATCH(0x74C775, VeinholeMonster_Constructor_RecalcAttributes, 0x6)
{
	GET(CellClass*, pCell, EAX);
	pCell->Level = (char)R->DL(); // write Level-1 before RecalcAttributes
	pCell->RecalcAttributes(DWORD(-1));
	MapClass::Instance->ResetZones(pCell->MapCoords);
	MapClass::Instance->RecalculateSubZones(pCell->MapCoords);
	return 0x74C77B;
}

// ============================================================
// Bridge RecalcAttributes helpers
// ============================================================

// Called when a bridge section overlay is fully removed (OverlayTypeIndex = -1).
static void BridgeCellDestroyed(CellClass* pCell)
{
	pCell->RecalcAttributes(DWORD(-1));
	MapClass::Instance->ResetZones(pCell->MapCoords);
	MapClass::Instance->RecalculateSubZones(pCell->MapCoords);
}

// Called when a bridge cell's damage state changes (OverlayData only, overlay
// type index still valid).
static void BridgeCellDamaged(CellClass* pCell)
{
	pCell->RecalcAttributes(DWORD(-1));
	MapClass::Instance->RecalculateZones(pCell->MapCoords);
	MapClass::Instance->RecalculateSubZones(pCell->MapCoords);
}

// ============================================================
// Bridge hooks - Group A: ESI = CellClass*, OverlayTypeIndex = -1 (7 bytes)
// ============================================================

ASMJIT_PATCH(0x56EFF2, MapBridge_56EF50_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x56EFF9;
}

ASMJIT_PATCH(0x56F392, MapBridge_56F2F0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x56F399;
}

ASMJIT_PATCH(0x56F956, MapBridge_Destroy_56F8B0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x56F95D;
}

ASMJIT_PATCH(0x56FD26, MapBridge_56FCC0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x56FD2D;
}

ASMJIT_PATCH(0x5721C2, MapBridge_572100_RecalcCell_A, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x5721C9;
}

ASMJIT_PATCH(0x5724E2, MapBridge_572480_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x5724E9;
}

ASMJIT_PATCH(0x572882, MapBridge_572820_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x572889;
}

ASMJIT_PATCH(0x572E46, MapBridge_572DE0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x572E4D;
}

ASMJIT_PATCH(0x573216, MapBridge_5731B0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x57321D;
}

ASMJIT_PATCH(0x57779F, MapBridge_577740_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x5777A6;
}

ASMJIT_PATCH(0x5778BB, MapBridge_577860_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x5778C2;
}

// ============================================================
// Bridge hooks - Group B: EAX = CellClass*, OverlayData change (7 bytes)
// ============================================================

ASMJIT_PATCH(0x56F712, MapBridge_56F690_Damaged_EAX_F1, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x56F719;
}

ASMJIT_PATCH(0x56F71B, MapBridge_56F690_Damaged_EAX_E1, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xE;
	BridgeCellDamaged(pCell);
	return 0x56F722;
}

ASMJIT_PATCH(0x56F822, MapBridge_56F7A0_Damaged_EAX_F2, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x56F829;
}

ASMJIT_PATCH(0x56F82B, MapBridge_56F7A0_Damaged_EAX_D2, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xD;
	BridgeCellDamaged(pCell);
	return 0x56F832;
}

ASMJIT_PATCH(0x572C02, MapBridge_572BC0_Damaged_EAX_F3, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x572C09;
}

ASMJIT_PATCH(0x572C0B, MapBridge_572BC0_Damaged_EAX_E3, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xE;
	BridgeCellDamaged(pCell);
	return 0x572C12;
}

ASMJIT_PATCH(0x572D12, MapBridge_572CD0_Damaged_EAX_F4, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x572D19;
}

ASMJIT_PATCH(0x572D1B, MapBridge_572CD0_Damaged_EAX_D4, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xD;
	BridgeCellDamaged(pCell);
	return 0x572D22;
}

// ============================================================
// Bridge hooks - Group C: ESI = CellClass*, OverlayData change (7 bytes)
// ============================================================

ASMJIT_PATCH(0x572101, MapBridge_572100_Damaged_ESI, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x572108;
}

ASMJIT_PATCH(0x5777FC, MapBridge_577740_Damaged_ESI, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x577803;
}

// ============================================================
// Bridge hooks - Group D: EBP = CellClass*, OverlayTypeIndex = -1 (7 bytes)
// ============================================================

// VeinholeMonsterClass area bridge cell clear (0x74CBEE).
ASMJIT_PATCH(0x74CBEE, VeinholeArea_Bridge_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, EBP);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x74CBF5;
}

// ============================================================
// Bridge hooks - Group E: ESI = CellClass*, OverlayData + OverlayTypeIndex (10 bytes)
// ============================================================

ASMJIT_PATCH(0x576721, MapBridge_576200_RecalcCell, 0xA)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayData = 0;
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x57672B;
}