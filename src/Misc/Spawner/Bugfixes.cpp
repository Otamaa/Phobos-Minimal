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

// skip error "А mouse is required for playing Yurts Revenge" - remove the GetSystemMetrics check
DEFINE_JUMP(LJMP, 0x6BD8A4, 0x6BD8C2); // WinMain

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

// Fix crash at 70AF6C
ASMJIT_PATCH(0x70AF6C, TechnoClass_70AF50_FixCrash, 0x9)
{
	GET(TechnoClass*, pTechno, EAX);
	return pTechno ? 0 : 0x70B1C7;
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
	if ((SessionClass::Instance->GameMode == GameMode::LAN && !Game::LANTaunts)
		|| (SessionClass::Instance->GameMode == GameMode::Internet && !Game::WOLTaunts)){
		return 0x50C910;
	}

	return 0x0;
}


#include <DisplayClass.h>
#include <TacticalClass.h>
#include <Ext/Tactical/Body.h>

DEFINE_FUNCTION_JUMP(LJMP, 0x6D8640, FakeTacticalClass::__ClampTacticalPos)

// canEnter and ignoreForce should come before GetFireError().
DEFINE_JUMP(LJMP, 0x70054D, 0x70056C)

namespace WhatActionObjectTemp
{
	bool Skip = false;
}

#include <Ext/TechnoType/Body.h>

ASMJIT_PATCH(0x700536, TechnoClass_WhatAction_Object_AllowAttack, 0x6)
{
	enum { CanAttack = 0x70055D, Continue = 0x700548 };

	GET_STACK(bool, canEnter, STACK_OFFSET(0x1C, 0x4));
	GET_STACK(bool, ignoreForce, STACK_OFFSET(0x1C, 0x8));
	GET(TechnoClass const* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if(TechnoTypeExtContainer::Instance.Find(pType)
		->NoManualFire)
		return 0x70056Cu;

	if (canEnter || ignoreForce)
		return CanAttack;

	GET(ObjectClass*, pObject, EDI);
	GET_STACK(int, WeaponIndex, STACK_OFFSET(0x1C, -0x8));

	WhatActionObjectTemp::Skip = true;
	R->EAX(pThis->GetFireError(pObject, WeaponIndex, true));
	WhatActionObjectTemp::Skip = false;

	return Continue;
}

ASMJIT_PATCH(0x6FC8F5, TechnoClass_CanFire_SkipROF, 0x6)
{
	return WhatActionObjectTemp::Skip ? 0x6FC981 : 0;
}

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