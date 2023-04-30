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

template<typename T>
void Append(T& buffer, const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf(Phobos::readBuffer, pFormat, args);
	va_end(args);
	strcat_s(buffer, Phobos::readBuffer);
}

template<typename T>
void Display(T& buffer)
{
	memset(Phobos::wideBuffer, 0, sizeof Phobos::wideBuffer);
	mbstowcs(Phobos::wideBuffer, buffer, strlen(buffer));
	MessageListClass::Instance->PrintMessage(Phobos::wideBuffer, 600, 5, true);
	Debug::Log("%s\n", buffer);
	buffer[0] = 0;
}

template<typename T>
void PrintFoots(T& buffer, FootClass* pFoot)
{
	Append(buffer, "[Phobos] Dump ObjectInfo runs.\n");
	const auto pType = pFoot->GetTechnoType();
	const auto nFootMapCoords = pFoot->InlineMapCoords();
	Append(buffer, "ID = %s, ", pType->ID);
	Append(buffer, "Owner = %s (%s), ", pFoot->Owner->get_ID(), pFoot->Owner->PlainName);
	Append(buffer, "Loc = (%d, %d), ", nFootMapCoords.X, nFootMapCoords.Y);
	Append(buffer, "Current Mission = %d (%s)\n", pFoot->CurrentMission, MissionClass::MissionToString(pFoot->CurrentMission));
	Append(buffer, "sp = %fl, fp = %fl , am = %fl , DKilled= %d ,", pFoot->SpeedMultiplier, pFoot->FirepowerMultiplier, pFoot->ArmorMultiplier, (*(bool*)((char*)pFoot->align_154 + 0x9C)));
	Append(buffer, "Grp = %d, RectA = %d, RectB = %d\n", pFoot->Group, (int)pFoot->RecruitableA, (int)pFoot->RecruitableB);

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
				Append(buffer, "TriggerID = %s, weights [Current, Min, Max]: %f, %f, %f", pTriggerType->ID, pTriggerType->Weight_Current, pTriggerType->Weight_Minimum, pTriggerType->Weight_Maximum);
			}
		}

		Display(buffer);

		Append(buffer, "TeamID = %s, ScriptID = %s, TaskforceID = %s",
			pTeam->Type->ID, pTeam->CurrentScript->Type->get_ID(), pTeam->Type->TaskForce->ID);

		Display(buffer);

		if (pTeam->CurrentScript->CurrentMission >= 0)
			Append(buffer, "Current Script [Line = Action, Argument]: %d = %d,%d", pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->GetCurrentAction().Action, pTeam->CurrentScript->GetCurrentAction().Argument);
		else
			Append(buffer, "Current Script [Line = Action, Argument]: %d", pTeam->CurrentScript->CurrentMission);

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
					const auto pTarget = static_cast<ObjectClass*>(pFirst->Target);
					pTargetStr = pTarget->get_ID();
				}
				else if ((((DWORD*)pFirst->Target)[0]) == CellClass::vtable)
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
			Append(buffer, "Passengers: %s", pFirst->GetTechnoType()->ID);
			for (NextObject j(pFoot->Passengers.FirstPassenger->NextObject);
				j && ((*j)->AbstractFlags & AbstractFlags::Foot);
				++j)
			{
				Append(buffer, ", %s", static_cast<FootClass*>(*j)->GetTechnoType()->ID);
			}
		}

		Append(buffer, "\n");
	}

	if (pFoot->CurrentMission == Mission::Attack
		|| pFoot->CurrentMission == Mission::AttackMove
		|| pFoot->CurrentMission == Mission::Hunt
		|| pFoot->CurrentMission == Mission::Sabotage
		|| pFoot->CurrentMission == Mission::Enter
		)
	{

		if (pFoot->Target)
		{
			if (pFoot->Target->AbstractFlags & AbstractFlags::Object)
			{
				const auto pTarget = static_cast<ObjectClass*>(pFoot->Target);
				Append(buffer, "Target = %s, Dist = %d, Loc = (%d, %d)\n", pTarget->get_ID(), (pTarget->DistanceFrom(pFoot) / 256), pTarget->InlineMapCoords().X, pTarget->InlineMapCoords().Y);
			}
			else if ((((DWORD*)pFoot->Target)[0]) == CellClass::vtable)
			{
				const auto pTargetCell = static_cast<CellClass*>(pFoot->Target);
				Append(buffer, "Target = Cell, Dist = %d, Loc = (%d, %d)\n", static_cast<int>(pTargetCell->GetCoords().DistanceFrom(pFoot->GetCoords()) / 256), pTargetCell->MapCoords.X, pTargetCell->MapCoords.Y);
			}
		}
	}
	else if (pFoot->CurrentMission == Mission::Move || pFoot->CurrentMission == Mission::Capture)
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

	const auto pArmor = ArmorTypeClass::FindFromIndex((int)pType->Armor);
	const auto nLevel = pFoot->Veterancy.GetRemainingLevel();
	Append(buffer, "CurHP = (%d / %d) , Armor = %s (%d) , Exp = %s ( %fl / %fl ) ", pFoot->Health, pType->Strength, pArmor->Name.data(), (int)pType->Armor, EnumFunctions::Rank_ToStrings[(int)nLevel], pFoot->Veterancy.Veterancy, RulesClass::Instance->VeteranCap);

	if (pType->Ammo > 0)
		Append(buffer, " , Ammo = (%d / %d) \n", pFoot->Ammo, pType->Ammo);
	else
		Append(buffer, "\n");

	if (const auto pTechnoExt = TechnoExt::ExtMap.Find(pFoot))
	{
		const auto pShieldData = pTechnoExt->Shield.get();

		if (pTechnoExt->CurrentShieldType && pShieldData)
		{
			auto const pShieldArmor = ArmorTypeClass::FindFromIndex((int)pTechnoExt->CurrentShieldType->Armor);
			Append(buffer, "CurShield (%s) , Armor = %s (%d) , HP = (%d / %d) ", pShieldData->GetType()->Name.data(), pShieldArmor->Name.data(), (int)pTechnoExt->CurrentShieldType->Armor, pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);
		}
	}

	Append(buffer, "\n");
	Display(buffer);
}

template<typename T>
void PrintBuilding(T& buffer, BuildingClass* pBuilding)
{
	Append(buffer, "[Phobos] Dump ObjectInfo runs.\n");
	const auto pType = pBuilding->GetTechnoType();
	const auto nFootMapCoords = pBuilding->InlineMapCoords();
	Append(buffer, "ID = %s, ", pType->ID);
	Append(buffer, "Owner = %s (%s), ", pBuilding->Owner->get_ID(), pBuilding->Owner->PlainName);
	Append(buffer, "fp = %fl , am = %fl ,", pBuilding->FirepowerMultiplier, pBuilding->ArmorMultiplier);
	Append(buffer, "Loc = (%d, %d)\n", nFootMapCoords.X, nFootMapCoords.Y);

	if (pBuilding->Factory && pBuilding->Factory->Object)
	{
		Append(buffer, "Production: %s (%d%%)\n", pBuilding->Factory->Object->GetTechnoType()->ID, (pBuilding->Factory->GetProgress() * 100 / 54));
	}

	if (pBuilding->Type->Refinery || pBuilding->Type->ResourceGatherer)
	{
		Append(buffer, "Money: %d\n", pBuilding->Owner->Available_Money());
	}

	if (pBuilding->Occupants.Count > 0)
	{
		Append(buffer, "Occupants: %s", pBuilding->Occupants.GetItem(0)->Type->ID);
		for (int i = 1; i < pBuilding->Occupants.Count; i++)
		{
			Append(buffer, ", %s", pBuilding->Occupants.GetItem(i)->Type->ID);
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

	auto const pArmor = ArmorTypeClass::FindFromIndex((int)pBuilding->Type->Armor);
	auto const nLevel = pBuilding->Veterancy.GetRemainingLevel();
	Append(buffer, "CurHP = (%d / %d) , Armor = %s (%d) , Exp = %s ( %fl / %fl ) \n", pBuilding->Health, pBuilding->Type->Strength, pArmor->Name.data(), (int)pBuilding->Type->Armor, EnumFunctions::Rank_ToStrings[(int)nLevel], pBuilding->Veterancy.Veterancy, RulesClass::Instance->VeteranCap);

	if (auto pTechnoExt = TechnoExt::ExtMap.Find(pBuilding))
	{
		auto pShieldData = pTechnoExt->Shield.get();

		if (pTechnoExt->CurrentShieldType && pShieldData)
		{
			auto const pShieldArmor = ArmorTypeClass::FindFromIndex((int)pTechnoExt->CurrentShieldType->Armor);
			Append(buffer, "CurShield (%s) , Armor = %s (%d) , HP = (%d / %d) ", pShieldData->GetType()->Name.data(), pShieldArmor->Name.data(), (int)pTechnoExt->CurrentShieldType->Armor, pShieldData->GetHP(), pTechnoExt->CurrentShieldType->Strength);

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
			DumpInfo(buffer, ObjectClass::CurrentObjects->GetItem(ObjectClass::CurrentObjects->Count - 1), dumped);
		}
		else
		{
			if (auto pCell = MapClass::Instance->GetCellAt(WWMouseClass::Instance->GetCellUnderCursor()))
			{

				const auto nTile = pCell->IsoTileTypeIndex;
				if (const auto pTile = IsometricTileTypeClass::Array->GetItemOrDefault(nTile))
				{
					Append(buffer, "[%d]TileType At Cell[%d , %d] is %s \n", nTile, pCell->MapCoords.X, pCell->MapCoords.Y, pTile->ID);
				}

				const auto nOverlay = pCell->OverlayTypeIndex;
				if (const auto pOverlay = OverlayTypeClass::Array->GetItemOrDefault(nOverlay))
				{
					Append(buffer, "[%d]OverlayType is %s ", nOverlay, pOverlay->ID);
				}

				for (auto pTerrain : *TerrainClass::Array)
				{
					if (pTerrain->Type && pTerrain->GetCell() == pCell)
					{
						auto const pArmor = ArmorTypeClass::FindFromIndex((int)pTerrain->Type->Armor);
						Append(buffer, "[%x]TerrainType is %s , Armor = %s (%d) , str = %d / %d \n", pTerrain, pTerrain->get_ID(), pArmor->Name.data(), (int)pTerrain->Type->Armor, pTerrain->Health, pTerrain->Type->Strength);
					}
				}

				Display(buffer);
			}
		}
	}
}
