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
#include <HouseClass.h>
#include <Ext/House/Body.h>

// Allow showing the select cursor on the object
// ASMJIT_PATCH(0x700594, TechnoClass_WhatAction_AllowAllies, 0x5)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	GET(ObjectClass*, pObject, EDI);
//
// 	return pThis->Owner->IsAlliedWith(pObject->GetOwningHouse()) ? 0x70059D : 0x7005E6;
// }

ASMJIT_PATCH(0x703A09, TechnoClass_VisualCharacter_CloakVisibility, 0x7)
{
	enum { UseShadowyVisual = 0x703A5A, CheckMutualAlliance = 0x703A16 };

	return HouseClass::IsCurrentPlayerObserver()
		? UseShadowyVisual
		: CheckMutualAlliance;
}

ASMJIT_PATCH(0x45455B, BuildingClass_VisualCharacter_CloakVisibility, 0x5)
{
	enum { UseShadowyVisual = 0x45452D, CheckMutualAlliance = 0x454564 };

	return HouseClass::IsCurrentPlayerObserver()
		? UseShadowyVisual
		: CheckMutualAlliance;
}

// Allow observers and mutual allies coordthing cloaked Technos
ASMJIT_PATCH(0x692540, ScrollClass_Coordthing_TechnoClass_Cloak, 0x5)
{
	enum { AllowCoordthing = 0x69256B, CheckSensedByHouses = 0 };

	if (HouseClass::IsCurrentPlayerObserver())
		return AllowCoordthing;

	GET(TechnoClass*, pTechno, ESI);
	const auto pTechnoOwner = pTechno->Owner;

	if (pTechnoOwner && HouseExtData::IsMutualAllies(pTechnoOwner , HouseClass::CurrentPlayer))
		return AllowCoordthing;

	return CheckSensedByHouses;
}

// Allow observers and mutual allies coordthing cloaked Buildings
ASMJIT_PATCH(0x6925AA, ScrollClass_Coordthing_BuildingClass_Cloak, 0x6)
{
	enum { AllowCoordthing = 0x6925F0, CheckSensedByHouses = 0 };

	if (HouseClass::IsCurrentPlayerObserver())
		return AllowCoordthing;

	GET(TechnoClass*, pTechno, ESI);
	const auto pTechnoOwner = pTechno->Owner;

	if (pTechnoOwner && HouseExtData::IsMutualAllies(pTechnoOwner ,HouseClass::CurrentPlayer))
		return AllowCoordthing;

	return CheckSensedByHouses;
}

ASMJIT_PATCH(0x6DA412, Tactical_SelectAt_Cloak, 0x6)
{
	enum { AllowSelect = 0x6DA43E, CheckSensedByHouses = 0 };

	if (HouseClass::IsCurrentPlayerObserver())
		return AllowSelect;

	GET(TechnoClass*, pTechno, EAX);
	const auto pTechnoOwner = pTechno->Owner;

	if (pTechnoOwner && HouseExtData::IsMutualAllies(pTechnoOwner ,HouseClass::CurrentPlayer))
		return AllowSelect;

	return CheckSensedByHouses;
}

// Allow observers and mutual allies selected cloaked Technos
ASMJIT_PATCH(0x6F4F19, TechnoClass_6F4EB0_Cloak, 0x6)
{
	enum { DontUnselect = 0x6F4F3A, CheckSensedByHouses = 0x6F4F21 };

	if (HouseClass::IsCurrentPlayerObserver())
		return DontUnselect;

	GET(TechnoClass*, pTechno, ESI);
	const auto pTechnoOwner = pTechno->Owner;

	if (pTechnoOwner && HouseExtData::IsMutualAllies(pTechnoOwner ,HouseClass::CurrentPlayer))
		return DontUnselect;

	return CheckSensedByHouses;
}

// Allow observers and mutual allies selected cloaked Technos
ASMJIT_PATCH(0x4ABE3C, DisplayClass_MouseLeftRelease_Cloak, 0xA)
{
	enum { AllowSelect = 0x4ABE4A, Unselect = 0x4ABE88 };

	if (HouseClass::IsCurrentPlayerObserver())
		return AllowSelect;

	GET(TechnoClass*, pTechno, ESI);
	const auto pTechnoOwner = pTechno->Owner;

	if (pTechnoOwner && HouseExtData::IsMutualAllies(pTechnoOwner ,HouseClass::CurrentPlayer))
		return AllowSelect;

	if (pTechno->IsSensorVisibleToPlayer())
		return AllowSelect;

	return Unselect;
}

// Show cloaked Technos on radar for observers and mutual allies
ASMJIT_PATCH(0x70D386, TechnoClass_Radar_Cloak, 0xA)
{
	enum { Show = 0x70D3CD, DontShow = 0x70D407 };

	if (HouseClass::IsCurrentPlayerObserver())
		return Show;

	GET(TechnoClass*, pTechno, ESI);
	const auto pTechnoOwner = pTechno->Owner;

	if (pTechnoOwner && HouseExtData::IsMutualAllies(pTechnoOwner ,HouseClass::CurrentPlayer))
		return Show;

	if (pTechno->IsSensorVisibleToPlayer())
		return Show;

	return DontShow;
}

// Show tooltips for observers and mutual allies
ASMJIT_PATCH(0x4AE62B, DisplayClass_HelpText_Cloak, 0x5)
{
	enum { CheckIsInvisible = 0x4AE654, CheckSensedByHouses = 0 };

	if (HouseClass::IsCurrentPlayerObserver())
		return CheckIsInvisible;

	GET(TechnoClass*, pTechno, ECX);
	const auto pTechnoOwner = pTechno->Owner;

	if (pTechnoOwner && HouseExtData::IsMutualAllies(pTechnoOwner ,HouseClass::CurrentPlayer))
		return CheckIsInvisible;

	if (pTechno->IsSensorVisibleToPlayer())
		return CheckIsInvisible;

	return CheckSensedByHouses;
}

// Show disguised units (Spy and Mirage) for observer
#pragma region
// Show spy for observer
ASMJIT_PATCH(0x4DEDC3, FootClass_GetImageData_Disguise, 0x6)
{
	return HouseClass::IsCurrentPlayerObserver()
		? 0x4DEE15
		: 0;
}

// Show real name of spy for observer
ASMJIT_PATCH(0x51F2F3, InfantryClass_FullName_Disguise, 0x6)
{
	return HouseClass::IsCurrentPlayerObserver()
		? 0x51F31A
		: 0;
}

// Flash disguise for Observer
ASMJIT_PATCH(0x70EE6A, TechnoClass_DisguiseBeenSeen, 0x6)
{
	return HouseClass::IsCurrentPlayerObserver()
		? 0x70EE79
		: 0;
}

// Allow observers selected disguise
ASMJIT_PATCH(0x7467CA, UnitClass_CantTargetDisguise, 0x5)
{
	GET(HouseClass*, pHouse, EDI);
	return pHouse->IsObserver()
		? 0x7467FE
		: 0;
}
#pragma endregion

// Allow Observer to see Pips
#pragma region
ASMJIT_PATCH(0x6F677D, TechnoClass_DrawSelection_Observer1, 0x5)
{
	return HouseClass::IsCurrentPlayerObserver()
		? 0x6F67B2
		: 0;
}
#pragma endregion


ASMJIT_PATCH(0x70DA7D, TechnoClass_RadarTrackingUpdate_Detected_ObserverSkip, 0x6) {
	return HouseClass::IsCurrentPlayerObserver() ? 0x70DADC : 0;
}

ASMJIT_PATCH(0x703819, TechnoClass_Cloak_Deselect_IsCurrentPlayerOberver, 0x6)  {
	return HouseClass::IsCurrentPlayerObserver() ? 0x70383C : 0x0;
}

ASMJIT_PATCH(0x65FA70, RadarEventClass_Create_ObserverSkipSensed, 0x6) {
	GET(RadarEventType, nType, ECX);
	return nType == RadarEventType::EnemySensed && HouseClass::IsCurrentPlayerObserver() ? 0x65FB52 : 0;
}
