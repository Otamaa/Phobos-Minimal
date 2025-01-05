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

// Open campaign briefing when pressing Tab
DEFINE_HOOK(0x55E08F, KeyboardProcess_PressTab, 0x5)
{
	Game::SpecialDialog = SessionClass::IsCampaign() ? 9 : 8;
	return 0x55E099;
}

// skip error "А mouse is required for playing Yurts Revenge" - remove the GetSystemMetrics check
DEFINE_JUMP(LJMP, 0x6BD8A4, 0x6BD8C2); // WinMain

// Prevents accidental exit when pressing the spacebar while waiting
// Remove focus from the Leave Game button in the player waiting window
DEFINE_HOOK(0x648CCC, WaitForPlayers_RemoveFocusFromLeaveGameBtn, 0x6)
{
	Imports::SetArchiveTarget.get()(0);
	return 0;
}

// A patch to prevent framerate drops when a player spams the 'type select' key
// Skip call GScreenClass::FlagToRedraw(1)
DEFINE_JUMP(LJMP, 0x732CED, 0x732CF9); // End_Type_Select_Command

DEFINE_HOOK(0x649851, WaitForPlayers_OnlineOptimizations, 0x5)
{
	Sleep(3); // Sleep yields the remaining CPU cycle time to any other processes
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

 DEFINE_HOOK(0x5F5893, ObjectClass_Mark_Unessesarycalls, 0x5) {
 	return R->EBX<int>() == 1 ? 0x5F58EC : 0x5F58E7;
 }

// Fix crash at 727B48
DEFINE_HOOK(0x727B44, TriggerTypeClass_ComputeCRC_FixCrash, 0x6)
{
	GET(HouseTypeClass*, pHouseType, EAX);
	return pHouseType ? 0 : 0x727B55;
}

// Fix crash at 6F49DE
DEFINE_HOOK(0x6F49D8, TechnoClass_Revealed_FixCrash, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return pHouse ? 0 : 0x6F4A31;
}

// Fix crash at 70AF6C
DEFINE_HOOK(0x70AF6C, TechnoClass_70AF50_FixCrash, 0x9)
{
	GET(TechnoClass*, pTechno, EAX);
	return pTechno ? 0 : 0x70B1C7;
}

// Fix crash at 4C2C19
void __fastcall EBolt_SetOwnerAndWeapon_FixCrash(EBolt* pThis, void*, TechnoClass* pTechno, int pWeapon)
{
	// vanilla code
	if (pTechno && pTechno->WhatAmI() == AbstractType::Unit && pTechno->IsAlive && !pTechno->InLimbo)
	{
		pThis->Owner = pTechno;
		pThis->WeaponSlot = pWeapon;
	}
	// correction code
	else
	{
		pThis->Owner = 0;
		pThis->WeaponSlot = 0;
	}
}

DEFINE_JUMP(CALL, 0x6FD606, MiscTools::to_DWORD(&EBolt_SetOwnerAndWeapon_FixCrash)); // Replace single call
DEFINE_JUMP(LJMP, 0x4C2BD0, MiscTools::to_DWORD(&EBolt_SetOwnerAndWeapon_FixCrash)); // For in case another module tries to call function

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

DEFINE_HOOK(0x454174, BuildingClass_Load_SwizzleLighsource, 0xA)
{
	GET(BuildingClass*, pThis, EDI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->LightSource);

	return 0x45417E;
}