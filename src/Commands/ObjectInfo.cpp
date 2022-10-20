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

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/GeneralUtils.h>

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

	auto getMissionName = [](int mID)
	{
		switch (mID)
		{
		case -1:
			return "None";
		case 0:
			return "Sleep";
		case 1:
			return "Attack";
		case 2:
			return "Move";
		case 3:
			return "QMove";
		case 4:
			return "Retreat";
		case 5:
			return "Guard";
		case 6:
			return "Sticky";
		case 7:
			return "Enter";
		case 8:
			return "Capture";
		case 9:
			return "Eaten";
		case 10:
			return "Harvest";
		case 11:
			return "Area_Guard";
		case 12:
			return "Return";
		case 13:
			return "Stop";
		case 14:
			return "Ambush";
		case 15:
			return "Hunt";
		case 16:
			return "Unload";
		case 17:
			return "Sabotage";
		case 18:
			return "Construction";
		case 19:
			return "Selling";
		case 20:
			return "Repair";
		case 21:
			return "Rescue";
		case 22:
			return "Missile";
		case 23:
			return "Harmless";
		case 24:
			return "Open";
		case 25:
			return "Patrol";
		case 26:
			return "ParadropApproach";
		case 27:
			return "ParadropOverfly";
		case 28:
			return "Wait";
		case 29:
			return "AttackMove";
		case 30:
			return "SpyplaneApproach";
		case 31:
			return "SpyplaneOverfly";
		default:
			return "INVALID_MISSION";
		}
	};

	auto display = [&buffer]()
	{
		memset(Phobos::wideBuffer, 0, sizeof Phobos::wideBuffer);
		mbstowcs(Phobos::wideBuffer, buffer, strlen(buffer));
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer, 600, 5, true);
		Debug::Log("%s\n", buffer);
		buffer[0] = 0;
	};

	auto printFoots = [&append, &display, &getMissionName](FootClass* pFoot)
	{
		append("[Phobos] Dump ObjectInfo runs.\n");
		auto pType = pFoot->GetTechnoType();
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

		append("Current HP = (%d / %d)", pFoot->Health, pType->Strength);

		if (auto pTechnoExt = TechnoExt::ExtMap.Find(pFoot))
		{
			auto pShieldData = pTechnoExt->Shield.get();

			if (pTechnoExt->CurrentShieldType && pShieldData)
				append(", Current Shield HP = (%d / %d)", pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);
		}

		if (pType->Ammo > 0)
			append(", Ammo = (%d / %d)", pFoot->Ammo, pType->Ammo);

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

		append("Current HP = (%d / %d)\n", pBuilding->Health, pBuilding->Type->Strength);

		if (auto pTechnoExt = TechnoExt::ExtMap.Find(pBuilding))
		{
			auto pShieldData = pTechnoExt->Shield.get();

			if (pTechnoExt->CurrentShieldType && pShieldData)
				append("Current Shield HP = (%d / %d)\n", pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);
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
			if (auto pCell = Map[WWMouseClass::Instance->GetCellUnderCursor()])
			{
				const auto nTile = pCell->IsoTileTypeIndex;
				if (nTile >= 0 && nTile < IsometricTileTypeClass::Array->Count)
				{
					auto pTile = IsometricTileTypeClass::Array->GetItem(nTile);
					append("[%d]TileType At Cell[%d , %d] is %s ", nTile, pCell->MapCoords.X, pCell->MapCoords.Y, pTile->ID);
					display();
				}
			}
		}
	}
}