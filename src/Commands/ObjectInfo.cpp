#include "ObjectInfo.h"

#include <AbstractClass.h>
#include <ObjectClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <FootClass.h>
#include <TeamClass.h>
#include <HouseClass.h>
#include <ScriptClass.h>
#include <AITriggerTypeClass.h>
#include <Helpers/Enumerators.h>
#include <IsometricTileTypeClass.h>
#include <TerrainTypeClass.h>
#include <VeinholeMonsterClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/EnumFunctions.h>
#include <New/Type/ArmorTypeClass.h>

const char* ObjectInfoCommandClass::GetName() const
{
	return "Dump ObjectInfo";
}

const wchar_t* ObjectInfoCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_OBJECT_INFO", L"Dump Object Info");
}

const wchar_t* ObjectInfoCommandClass::GetUICategory() const
{
	return CATEGORY_GUIDEBUG;
}

const wchar_t* ObjectInfoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_OBJECT_INFO_DESC", L"Dump ObjectInfo to log file and display it.");
}

template<typename T>
void Append(T& buffer, const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf(Phobos::readBuffer, pFormat, args);
	va_end(args);
	strcat_s(buffer, Phobos::readBuffer);
}

static bool WhiteColorSearched = false;
int ColorIdx = 5;

template<typename T>
void Display(T& buffer)
{
	memset(Phobos::wideBuffer, 0, sizeof(Phobos::wideBuffer));
	mbstowcs(Phobos::wideBuffer, buffer, strlen(buffer));
	if (!WhiteColorSearched) {
		const auto WhiteIndex = ColorScheme::FindIndex("White",53);

		if (WhiteIndex != -1) {
			ColorIdx = WhiteIndex;
		}

		WhiteColorSearched = true;
	}

	MessageListClass::Instance->PrintMessage(Phobos::wideBuffer, 600, ColorIdx, true);
	Debug::Log("%s\n", buffer);
	buffer[0] = 0;
}

template<typename T>
void PrintFoots(T& buffer, FootClass* pFoot)
{
	Append(buffer, "[Phobos] Dump ObjectInfo runs.\n");
	const auto pType = pFoot->GetTechnoType();
	const auto nFootMapCoords = pFoot->InlineMapCoords();
	Append(buffer, "ID = %s, ", pType->get_ID());
	Append(buffer, "Owner = %s (%s), ", pFoot->Owner->get_ID(), pFoot->Owner->PlainName);
	Append(buffer, "Loc = (%d, %d), ", nFootMapCoords.X, nFootMapCoords.Y);

	//if(pFoot->Health < 0){
	//	auto pLastWH = TechnoExtContainer::Instance.Find(pFoot);
	//	if(pLastWH->LastDamageWH) {
	//		Append(buffer, "LastDamagingWH = (%s), ", pLastWH->LastDamageWH->ID);
	//	}
	//}

	Append(buffer, "Current Mission = %d (%s)\n", pFoot->CurrentMission, MissionClass::MissionToString(pFoot->CurrentMission));
	Append(buffer, "sp = %fl, fp = %fl , am = %fl , EMP: %d ", pFoot->SpeedMultiplier, pFoot->FirepowerMultiplier, pFoot->ArmorMultiplier, pFoot->EMPLockRemaining);
	if (pFoot->WhatAmI() != InfantryClass::AbsID)
		Append(buffer, ", DKilled= %d , ", TechnoExtContainer::Instance.Find(pFoot)->Is_DriverKilled);

	Append(buffer, "Grp = %d, RectA = %d, RectB = %d\n", pFoot->Group, (int)pFoot->RecruitableA, (int)pFoot->RecruitableB);

	if (pFoot->BelongsToATeam())
	{
		const auto pTeam = pFoot->Team;
		const auto pTeamType = pFoot->Team->Type;
		bool found = false;

		for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
		{
			const auto pTriggerType = AITriggerTypeClass::Array->Items[i];

			if (pTeamType && ((pTriggerType->Team1 && pTriggerType->Team1 == pTeamType) || (pTriggerType->Team2 && pTriggerType->Team2 == pTeamType)))
			{
				found = true;
				Append(buffer, "TriggerID = %s, weights [Current, Min, Max]: %f, %f, %f",
					pTriggerType->ID, pTriggerType->Weight_Current,
					pTriggerType->Weight_Minimum,
					pTriggerType->Weight_Maximum);
			}
		}

		Display(buffer);

		Append(buffer, "TeamID = %s, ScriptID = %s, TaskforceID = %s",
			pTeam->Type->ID, pTeam->CurrentScript->Type->get_ID(), pTeam->Type->TaskForce->ID);

		Display(buffer);

		if (pTeam->CurrentScript->CurrentMission >= 0)
			Append(buffer, "Current Script ([Line - %d] = [Action : %d - Argument : %d])", pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->GetCurrentAction().Action, pTeam->CurrentScript->GetCurrentAction().Argument);
		else
			Append(buffer, "Current Script ([Line - %d])", pTeam->CurrentScript->CurrentMission);

		Display(buffer);
	}

	if (pFoot->Passengers.NumPassengers > 0)
	{
		const auto pFirst = pFoot->Passengers.FirstPassenger;
		if (pFoot->Passengers.NumPassengers == 1)
		{
			const char* pTargetStr = NONE_STR;

			if (pFirst->Target)
			{
				if (pFirst->Target->AbstractFlags & AbstractFlags::Object)
				{
					pTargetStr = static_cast<ObjectClass*>(pFirst->Target)->get_ID();
				}
				else if (pFirst->Target->WhatAmI() == CellClass::AbsID)
				{
					pTargetStr = "Cell";
				}
			}

			const auto pFirstName = pFirst->get_ID();
			const auto pFirstMissionStr = MissionClass::MissionToString(pFirst->CurrentMission);
			Append(buffer, "Passengers: %s , Mission %s , Target %s ", pFirstName, pFirstMissionStr, pTargetStr);
		}
		else
		{
			Append(buffer, "Passengers: %s", pFirst->get_ID());
			for (NextObject j(pFoot->Passengers.FirstPassenger->NextObject);
				j && ((*j)->AbstractFlags & AbstractFlags::Foot);
				++j)
			{
				Append(buffer, ", %s", static_cast<FootClass*>(*j)->get_ID());
			}
		}

		Append(buffer, "\n");
	}

	if (pFoot->CurrentMission == Mission::Move || pFoot->CurrentMission == Mission::Capture)
	{
		if (pFoot->Destination)
		{
			if (pFoot->Destination->AbstractFlags & AbstractFlags::Object)
			{
				const auto pDest = static_cast<ObjectClass*>(pFoot->Destination);
				const auto pDestLoc = pDest->InlineMapCoords();
				Append(buffer, "Destination = %s, Dist = %d, Loc = (%d, %d)\n", pDest->get_ID(), (pDest->DistanceFrom(pFoot) / 256), pDestLoc.X, pDestLoc.Y);
			}
			else if ((((DWORD*)pFoot->Destination)[0]) == CellClass::vtable)
			{
				const auto pDestCell = static_cast<CellClass*>(pFoot->Destination);
				Append(buffer, "Destination = Cell, Dist = %d, Loc = (%d, %d)\n", static_cast<int>(pDestCell->GetCoords().DistanceFrom(pFoot->GetCoords()) / 256), pDestCell->MapCoords.X, pDestCell->MapCoords.Y);
			}
		}
	}
	else
	{
		if (pFoot->Target)
		{
			if (pFoot->Target->AbstractFlags & AbstractFlags::Object)
			{
				const auto pTarget = static_cast<ObjectClass*>(pFoot->Target);
					Append(buffer, "Target = %s, Dist = %d, Loc = (%d, %d)\n", pTarget->get_ID(), (pTarget->DistanceFrom(pFoot) / 256), pTarget->InlineMapCoords().X, pTarget->InlineMapCoords().Y);
			}
			else if (pFoot->Target->WhatAmI() == CellClass::AbsID)
			{
				const auto pTargetCell = static_cast<CellClass*>(pFoot->Target);
				Append(buffer, "Target = Cell, Dist = %d, Loc = (%d, %d)\n", static_cast<int>(pTargetCell->GetCoords().DistanceFrom(pFoot->GetCoords()) / 256), pTargetCell->MapCoords.X, pTargetCell->MapCoords.Y);
			}
		}
	}

	const auto Armor_ = TechnoExtData::GetArmor(pFoot);
	const auto pArmor = ArmorTypeClass::FindFromIndex((int)Armor_);
	const auto nLevel = pFoot->Veterancy.GetRemainingLevel();
	Append(buffer, "CurHP = (%d / %d) , Armor = %s (%d) , Exp = %s ( %fl / %fl ) ", pFoot->Health, pType->Strength, pArmor->Name.data(), (int)Armor_, EnumFunctions::Rank_ToStrings[(int)nLevel], pFoot->Veterancy.Veterancy, RulesClass::Instance->VeteranCap);

	if (pType->Ammo > 0)
		Append(buffer, " , Ammo = (%d / %d) \n", pFoot->Ammo, pType->Ammo);
	else
		Append(buffer, "\n");

	if (const auto pTechnoExt = TechnoExtContainer::Instance.Find(pFoot))
	{
		const auto pShieldData = pTechnoExt->Shield.get();

		if (pTechnoExt->CurrentShieldType && pShieldData)
		{
			auto const pShieldArmor = ArmorTypeClass::FindFromIndex((int)pTechnoExt->CurrentShieldType->Armor.Get());
			Append(buffer, "CurShield (%s) , Armor = %s (%d) , HP = (%d / %d) ", pShieldData->GetType()->Name.data(), pShieldArmor->Name.data(), (int)pTechnoExt->CurrentShieldType->Armor.Get(), pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);
		}
	}

	Append(buffer, "\n");
	Display(buffer);
}

template<typename T>
void PrintBuilding(T& buffer, BuildingClass* pBuilding)
{
	Append(buffer, "[Phobos] Dump ObjectInfo runs.\n");
	const auto nFootMapCoords = pBuilding->InlineMapCoords();
	Append(buffer, "ID = %s, ", pBuilding->get_ID());
	Append(buffer, "Owner = %s (%s), ", pBuilding->Owner->get_ID(), pBuilding->Owner->PlainName);
	Append(buffer, "Current Mission = %d (%s)\n", pBuilding->CurrentMission, MissionClass::MissionToString(pBuilding->CurrentMission));
	Append(buffer, "fp = %fl , am = %fl  , EMP: %d ,", pBuilding->FirepowerMultiplier, pBuilding->ArmorMultiplier, pBuilding->EMPLockRemaining);
	Append(buffer, "Loc = (%d, %d)\n", nFootMapCoords.X, nFootMapCoords.Y);

	if (pBuilding->Factory && pBuilding->Factory->Object)
	{
		Append(buffer, "Production: %s (%d%%)\n", pBuilding->Factory->Object->get_ID(), (pBuilding->Factory->GetProgress() * 100 / 54));
	}

	if (pBuilding->Type->Refinery || pBuilding->Type->ResourceGatherer)
	{
		Append(buffer, "Money: %d\n", pBuilding->Owner->Available_Money());
	}

	if (pBuilding->Occupants.Count > 0)
	{
		Append(buffer, "Occupants: %s", pBuilding->Occupants.Items[0]->Type->ID);
		for (int i = 1; i < pBuilding->Occupants.Count; i++)
		{
			Append(buffer, ", %s", pBuilding->Occupants.Items[i]->Type->ID);
		}
		Append(buffer, "\n");
	}

	if (pBuilding->Type->Upgrades)
	{
		Append(buffer, "Upgrades (%d/%d): ", pBuilding->UpgradeLevel, pBuilding->Type->Upgrades);
		for (int i = 0; i < 3; i++)
		{
			if (i != 0)
				Append(buffer, ", ");

			Append(buffer, "Slot %d = %s", i + 1, pBuilding->Upgrades[i] ? pBuilding->Upgrades[i]->get_ID() : "<none>");
		}
		Append(buffer, "\n");
	}

	if (pBuilding->Type->Ammo > 0)
		Append(buffer, "Ammo = (%d / %d)\n", pBuilding->Ammo, pBuilding->Type->Ammo);

	if (pBuilding->Target)
	{
		if (pBuilding->Target->AbstractFlags & AbstractFlags::Object)
		{
			const auto pTarget = static_cast<ObjectClass*>(pBuilding->Target);
			const auto pTargetLoc = pTarget->InlineMapCoords();
			Append(buffer, "Target = %s, Dist = %d, Loc = (%d, %d)\n", pTarget->get_ID(), (pTarget->DistanceFrom(pBuilding) / 256), pTargetLoc.X, pTargetLoc.Y);
		}
		else if ((((DWORD*)pBuilding->Target)[0]) == CellClass::vtable)
		{
			const auto pTargetCell = static_cast<CellClass*>(pBuilding->Target);
			Append(buffer, "Target = Cell, Dist = %d, Loc = (%d, %d)\n", static_cast<int>(pTargetCell->GetCoords().DistanceFrom(pBuilding->GetCoords()) / 256), pTargetCell->MapCoords.X, pTargetCell->MapCoords.Y);
		}
	}

	auto const armor_ = TechnoExtData::GetArmor(pBuilding);
	auto const pArmor = ArmorTypeClass::FindFromIndex((int)armor_);
	auto const nLevel = pBuilding->Veterancy.GetRemainingLevel();
	Append(buffer, "CurHP = (%d / %d) , Armor = %s (%d) , Exp = %s ( %fl / %fl ) \n", pBuilding->Health, pBuilding->Type->Strength, pArmor->Name.data(), (int)armor_, EnumFunctions::Rank_ToStrings[(int)nLevel], pBuilding->Veterancy.Veterancy, RulesClass::Instance->VeteranCap);

	if (auto pTechnoExt = TechnoExtContainer::Instance.Find(pBuilding))
	{
		auto pShieldData = pTechnoExt->Shield.get();

		if (pTechnoExt->CurrentShieldType && pShieldData)
		{
			auto const pShieldArmor = ArmorTypeClass::FindFromIndex((int)pTechnoExt->CurrentShieldType->Armor.Get());
			Append(buffer, "CurShield (%s) , Armor = %s (%d) , HP = (%d / %d) ", pShieldData->GetType()->Name.data(), pShieldArmor->Name.data(), (int)pTechnoExt->CurrentShieldType->Armor.Get(), pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);

		}
	}

	Display(buffer);
}

template<typename T>
void DumpInfo(T& buffer, ObjectClass* pObject, bool& bIsDumpedAlready)
{
	switch ((((DWORD*)pObject)[0]))
	{
	case InfantryClass::vtable:
	case UnitClass::vtable:
	case AircraftClass::vtable:
		PrintFoots(buffer, static_cast<FootClass*>(pObject));
		break;
	case BuildingClass::vtable:
		PrintBuilding(buffer, static_cast<BuildingClass*>(pObject));
		break;
	default:
		Append(buffer, "INVALID ITEM!");
		Display(buffer);
		break;
	}
	bIsDumpedAlready = true;
};

void ObjectInfoCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	char buffer[0x800] = { 0 };
	bool dumped = false;

	for (auto pTechno : *TechnoClass::Array)
	{
		if (dumped) break;
		if (pTechno->IsMouseHovering)
		{
			DumpInfo(buffer, pTechno, dumped);
		}
	}

	if (!dumped)
	{
		if (ObjectClass::CurrentObjects->Count > 0)
		{
			if (ObjectClass::CurrentObjects->Count != 1)
				MessageListClass::Instance->PrintMessage(L"This command will only dump one of these selected object", 600, 5, true);
			DumpInfo(buffer, ObjectClass::CurrentObjects->Items[ObjectClass::CurrentObjects->Count - 1], dumped);
		}
		else
		{
			if (auto pCell = MapClass::Instance->GetCellAt(WWMouseClass::Instance->GetCellUnderCursor()))
			{
				const auto nCoord = pCell->GetCoordsWithBridge();
				Append(buffer, "Cell At [%d - %d - %d]/[%d - %d] OverlayData [%d]\n", nCoord.Y, nCoord.Y, nCoord.Z, pCell->MapCoords.X, pCell->MapCoords.Y, pCell->OverlayData);

				const auto nTile = pCell->IsoTileTypeIndex;
				if (const auto pTile = IsometricTileTypeClass::Array->GetItemOrDefault(nTile))
				{
					Append(buffer, "[%d]TileType is %s\n", nTile, pTile->ID);
				}

				const auto nOverlay = pCell->OverlayTypeIndex;
				if (const auto pOverlay = OverlayTypeClass::Array->GetItemOrDefault(nOverlay))
				{
					Append(buffer, "[%d]OverlayType is %s\n", nOverlay, pOverlay->ID);

					const auto tibIdx = pCell->GetContainedTiberiumIndex();
					if(tibIdx != -1)
						Append(buffer, "[%d]TiberiumType is %s\n", tibIdx, TiberiumClass::Array->Items[tibIdx]->ID);
					else if(pOverlay->Wall){
						const auto pHouse = HouseClass::Array->Items[pCell->WallOwnerIndex];
						const char* pPlainName = pHouse ? pHouse->PlainName : GameStrings::NoneStr();
						const char* pID = pHouse ? pHouse->Type->ID : GameStrings::NoneStr();

						Append(buffer, "[%d]Wall Owner Is [%s - %s(%d)]\n" , nOverlay , pPlainName , pID , pCell->WallOwnerIndex);
					}
				}

				if (auto pTerrain = pCell->GetTerrain(false))
				{
					auto const pArmor = ArmorTypeClass::FindFromIndex((int)pTerrain->Type->Armor);
					Append(buffer, "[%x]TerrainType is %s , Armor = %s (%d) , str = %d / %d\n", pTerrain, pTerrain->Type->ID, pArmor->Name.data(), (int)pTerrain->Type->Armor, pTerrain->Health, pTerrain->Type->Strength);
				}

				Display(buffer);
			}
		}
	}
}
