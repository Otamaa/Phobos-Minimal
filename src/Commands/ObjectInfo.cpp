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
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* ObjectInfoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_OBJECT_INFO_DESC", L"Dump ObjectInfo to log file and display it.");
}

const char* const getMissionName (int mID)
{
	if (mID == -1)
		return GameStrings::NoneStr();

	if (mID < (int)MissionControlClass::Names.c_size())
		return MissionControlClass::Names[mID];

	return "INVALID_MISSION";
}

void ObjectInfoCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	char buffer[0x800] = { 0 };

	auto append = [&buffer](const char* pFormat, ...)
	{
		va_list args;
		va_start(args, pFormat);
		vsprintf(Phobos::readBuffer, pFormat, args);
		va_end(args);
		strcat_s(buffer, Phobos::readBuffer);
	};

	auto display = [&buffer]()
	{
		memset(Phobos::wideBuffer, 0, sizeof Phobos::wideBuffer);
		CRT::mbstowcs(Phobos::wideBuffer, buffer, strlen(buffer));
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer, 600, 5, true);
		Debug::Log("%s\n", buffer);
		buffer[0] = 0;
	};

	auto printFoots = [&append, &display](FootClass* pFoot)
	{
		append("[Phobos] Dump ObjectInfo runs.\n");
		const auto pType = pFoot->GetTechnoType();
		append("ID = %s, ", pType->ID);
		append("Owner = %s (%s), ", pFoot->Owner->get_ID(), pFoot->Owner->PlainName);
		append("Location = (%d, %d), ", pFoot->GetMapCoords().X, pFoot->GetMapCoords().Y);
		append("Current Mission = %d (%s)\n", pFoot->CurrentMission, getMissionName((int)pFoot->CurrentMission));
		append("Group = %d, RecruitA = %d, RecruitB = %d\n", pFoot->Group, (int)pFoot->RecruitableA, (int)pFoot->RecruitableB);

		if (pFoot->BelongsToATeam())
		{
			auto pTeam = pFoot->Team;

			auto pTeamType = pFoot->Team->Type;
			bool found = false;
			for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
			{
				auto pTriggerTeam1Type = AITriggerTypeClass::Array->GetItem(i)->Team1;
				auto pTriggerTeam2Type = AITriggerTypeClass::Array->GetItem(i)->Team2;

				if (pTeamType && ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType) || (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
				{
					found = true;
					auto pTriggerType = AITriggerTypeClass::Array->GetItem(i);
					append("Trigger ID = %s, weights [Current, Min, Max]: %f, %f, %f", pTriggerType->ID, pTriggerType->Weight_Current, pTriggerType->Weight_Minimum, pTriggerType->Weight_Maximum);
				}
			}
			display();

			append("Team ID = %s, Script ID = %s, Taskforce ID = %s",
				pTeam->Type->ID, pTeam->CurrentScript->Type->get_ID(), pTeam->Type->TaskForce->ID);
			display();

			if (pTeam->CurrentScript->CurrentMission >= 0)
				append("Current Script [Line = Action, Argument]: %d = %d,%d", pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->GetCurrentAction().Action, pTeam->CurrentScript->GetCurrentAction().Argument);
			else
				append("Current Script [Line = Action, Argument]: %d", pTeam->CurrentScript->CurrentMission);

			display();
		}

		if (pFoot->Passengers.NumPassengers > 0)
		{
			auto pFirst = pFoot->Passengers.FirstPassenger;
			if (pFoot->Passengers.NumPassengers == 1)
			{
				auto pTarget = generic_cast<ObjectClass*>(pFirst->Target);
				append("Passengers: %s , Mission %s , Target %s ", pFirst->GetTechnoType()->ID, getMissionName((int)pFirst->CurrentMission), pTarget ? pTarget->get_ID() : NONE_STR2);
			}
			else
			{
				append("Passengers: %s", pFirst->GetTechnoType()->ID);
				for (NextObject j(pFoot->Passengers.FirstPassenger->NextObject); j && abstract_cast<FootClass*>(*j); ++j)
				{

					auto passenger = static_cast<FootClass*>(*j);
					append(", %s", passenger->GetTechnoType()->ID);
				}
			}
			append("\n");
		}

		if (pFoot->CurrentMission == Mission::Attack
			|| pFoot->CurrentMission == Mission::AttackMove
			|| pFoot->CurrentMission == Mission::Hunt
			|| pFoot->CurrentMission == Mission::Sabotage
			|| pFoot->CurrentMission == Mission::Enter)
		{
			auto pTarget = abstract_cast<ObjectClass*>(pFoot->Target);
			auto pTargetCell = specific_cast<CellClass*>(pFoot->Target);
			if (pTarget)
			{
				append("Target = %s, Distance = %d, Location = (%d, %d)\n", pTarget->get_ID(), (pTarget->DistanceFrom(pFoot) / 256), pTarget->GetMapCoords().X, pTarget->GetMapCoords().Y);
			}
			else
				if (pTargetCell)
				{
					append("Target = Cell, Distance = %d, Location = (%d, %d)\n", static_cast<int>(pTargetCell->GetCoords().DistanceFrom(pFoot->GetCoords()) / 256), pTargetCell->MapCoords.X, pTargetCell->MapCoords.Y);
				}
		}
		else if (pFoot->CurrentMission == Mission::Move)
		{
			auto pDest = abstract_cast<ObjectClass*>(pFoot->Destination);
			auto pDestCell = specific_cast<CellClass*>(pFoot->Destination);
			if (pDest)
			{
				append("Destination = %s, Distance = %d, Location = (%d, %d)\n", pDest->get_ID(), (pDest->DistanceFrom(pFoot) / 256), pDest->GetMapCoords().X, pDest->GetMapCoords().Y);
			}
			else
				if (pDestCell)
				{
					append("Destination = Cell, Distance = %d, Location = (%d, %d)\n", static_cast<int>(pDestCell->GetCoords().DistanceFrom(pFoot->GetCoords()) / 256), pDestCell->MapCoords.X, pDestCell->MapCoords.Y);
				}
		}

		auto const pArmor = ArmorTypeClass::FindFromIndex((int)pType->Armor);
		auto const nLevel = pFoot->Veterancy.GetRemainingLevel();
		append("Current HP = (%d / %d) , Armor = %s (%d) , Experience = %s ( %fl / %fl ) ", pFoot->Health, pType->Strength , pArmor->Name.data() , (int)pType->Armor , EnumFunctions::Rank_ToStrings.at((int)nLevel), pFoot->Veterancy.Veterancy, RulesClass::Instance->VeteranCap);
		
		if (pType->Ammo > 0)
			append(" , Ammo = (%d / %d) \n", pFoot->Ammo, pType->Ammo);
		else
			append("\n");

		if (auto pTechnoExt = TechnoExt::ExtMap.Find(pFoot))
		{
			auto pShieldData = pTechnoExt->Shield.get();

			if (pTechnoExt->CurrentShieldType && pShieldData)
			{
				auto const pShieldArmor = ArmorTypeClass::FindFromIndex((int)pTechnoExt->CurrentShieldType->Armor);
				append("Current Shield (%s) , Armor = %s (%d) , HP = (%d / %d) ", pShieldData->GetType()->Name.data(), pShieldArmor->Name.data(), (int)pTechnoExt->CurrentShieldType->Armor, pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);

			}
		}



		append("\n");
		display();
	};

	auto printBuilding = [&append, &display](BuildingClass* pBuilding)
	{
		append("[Phobos] Dump ObjectInfo runs.\n");
		auto pType = pBuilding->GetTechnoType();
		append("ID = %s, ", pType->ID);
		append("Owner = %s (%s), ", pBuilding->Owner->get_ID(), pBuilding->Owner->PlainName);
		append("Location = (%d, %d)\n", pBuilding->GetMapCoords().X, pBuilding->GetMapCoords().Y);

		if (pBuilding->Factory && pBuilding->Factory->Object)
		{
			append("Production: %s (%d%%)\n", pBuilding->Factory->Object->GetTechnoType()->ID, (pBuilding->Factory->GetProgress() * 100 / 54));
		}

		if (pBuilding->Type->Refinery || pBuilding->Type->ResourceGatherer)
		{
			append("Money: %d\n", pBuilding->Owner->Available_Money());
		}

		if (pBuilding->Occupants.Count > 0)
		{
			append("Occupants: %s", pBuilding->Occupants.GetItem(0)->Type->ID);
			for (int i = 1; i < pBuilding->Occupants.Count; i++)
			{
				append(", %s", pBuilding->Occupants.GetItem(i)->Type->ID);
			}
			append("\n");
		}

		if (pBuilding->Type->Ammo > 0)
			append("Ammo = (%d / %d)\n", pBuilding->Ammo, pBuilding->Type->Ammo);


		auto pTarget = abstract_cast<ObjectClass*>(pBuilding->Target);
		auto pTargetCell = specific_cast<CellClass*>(pBuilding->Target);
		if (pTarget)
		{
			append("Target = %s, Distance = %d, Location = (%d, %d)\n", pTarget->get_ID(), (pTarget->DistanceFrom(pBuilding) / 256), pTarget->GetMapCoords().X, pTarget->GetMapCoords().Y);
		}
		else
			if (pTargetCell)
			{
				append("Target = Cell, Distance = %d, Location = (%d, %d)\n", static_cast<int>(pTargetCell->GetCoords().DistanceFrom(pBuilding->GetCoords()) / 256), pTargetCell->MapCoords.X, pTargetCell->MapCoords.Y);
			}

		auto const pArmor = ArmorTypeClass::FindFromIndex((int)pBuilding->Type->Armor);
		auto const nLevel = pBuilding->Veterancy.GetRemainingLevel();
		append("Current HP = (%d / %d) , Armor = %s (%d) , Experience = %s ( %fl / %fl ) \n", pBuilding->Health, pBuilding->Type->Strength , pArmor->Name.data() , (int)pBuilding->Type->Armor , EnumFunctions::Rank_ToStrings.at((int)nLevel), pBuilding->Veterancy.Veterancy, RulesClass::Instance->VeteranCap);

		if (auto pTechnoExt = TechnoExt::ExtMap.Find(pBuilding))
		{
			auto pShieldData = pTechnoExt->Shield.get();

			if (pTechnoExt->CurrentShieldType && pShieldData)
			{
				auto const pShieldArmor = ArmorTypeClass::FindFromIndex((int)pTechnoExt->CurrentShieldType->Armor);
				append("Current Shield (%s) , Armor = %s (%d) , HP = (%d / %d) ", pShieldData->GetType()->Name.data(), pShieldArmor->Name.data(), (int)pTechnoExt->CurrentShieldType->Armor, pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);

			}
		}

		display();
	};

	bool dumped = false;
	auto dumpInfo = [&printFoots, &printBuilding, &append, &display, &dumped](ObjectClass* pObject)
	{
		switch (pObject->WhatAmI())
		{
		case AbstractType::Infantry:
		case AbstractType::Unit:
		case AbstractType::Aircraft:
			printFoots(abstract_cast<FootClass*>(pObject));
			break;
		case AbstractType::Building:
			printBuilding(abstract_cast<BuildingClass*>(pObject));
			break;
		default:
			append("INVALID ITEM!");
			display();
			break;
		}
		dumped = true;
	};

	for (auto pTechno : *TechnoClass::Array)
	{
		if (dumped) break;
		if (pTechno->IsMouseHovering)
		{
			dumpInfo(pTechno);
		}
	}

	if (!dumped)
	{
		if (ObjectClass::CurrentObjects->Count > 0)
		{
			if (ObjectClass::CurrentObjects->Count != 1)
				MessageListClass::Instance->PrintMessage(L"This command will only dump one of these selected object", 600, 5, true);
			dumpInfo(ObjectClass::CurrentObjects->GetItem(ObjectClass::CurrentObjects->Count - 1));
		}
		else
		{
			if (auto pCell = MapClass::Instance->GetCellAt(WWMouseClass::Instance->GetCellUnderCursor()))
			{

				const auto nTile = pCell->IsoTileTypeIndex;
				if (nTile >= 0 && nTile < IsometricTileTypeClass::Array->Count)
				{
					auto pTile = IsometricTileTypeClass::Array->GetItem(nTile);
					append("[%d]TileType At Cell[%d , %d] is %s \n", nTile, pCell->MapCoords.X, pCell->MapCoords.Y, pTile->ID);
				}

				const auto nOverlay = pCell->OverlayTypeIndex;
				if (nOverlay >= 0 && nOverlay < OverlayTypeClass::Array->Count)
				{
					auto pOverlay = OverlayTypeClass::Array->GetItem(nOverlay);
					append("[%d]OverlayType is %s ", nOverlay, pOverlay->ID);
				}

				for (auto pTerrain : *TerrainClass::Array) {
					if (pTerrain->Type && pTerrain->GetCell() == pCell)
					{
						auto const pArmor = ArmorTypeClass::FindFromIndex((int)pTerrain->Type->Armor);
						append("[%x]TerrainType is %s , Armor = %s (%d) , str = %d / %d \n", pTerrain, pTerrain->get_ID() , pArmor->Name.data() , (int)pTerrain->Type->Armor, pTerrain->Health , pTerrain->Type->Strength);
					}
				}

				display();
			}
		}
	}
}