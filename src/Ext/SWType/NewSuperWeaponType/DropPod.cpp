#include "DropPod.h"

#include <Ext/Techno/Body.h>
#include <Ext/Rules/Body.h>

std::vector<const char*> SW_DropPod::GetTypeString() const
{
	return { "DropPod" };
}

bool SW_DropPod::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	SuperWeaponTypeClass* pSW = pThis->Type;
	auto pData = SWTypeExt::ExtMap.Find(pSW);

	HouseClass* pOwner = pThis->Owner;
	CellStruct cell = Coords;

	// collect the options
	auto const& Types = !pData->DropPod_Types.empty()
		? pData->DropPod_Types
		: RulesExt::Global()->DropPodTypes;

	// quick way out
	if (Types.empty()) {
		return false;
	}

	int cMin = pData->DropPod_Minimum.Get(RulesExt::Global()->DropPodMinimum);
	int cMax = pData->DropPod_Maximum.Get(RulesExt::Global()->DropPodMaximum);
	double veterancy = pData->DropPod_Veterancy.Get();

	// three times more tries than units to place.
	int count = ScenarioClass::Instance->Random.RandomRanged(cMin, cMax);
	for (int i = 3 * count; i; --i)
	{

		// get a random type from the list and create an instance
		TechnoTypeClass* pType = Types[ScenarioClass::Instance->Random.RandomFromMax(Types.size() - 1)];

		if (!pType)
			break;

		if (pType->WhatAmI() == BuildingTypeClass::AbsID) {
			break;
		}

		FootClass* pFoot = static_cast<FootClass*>(pType->CreateObject(pOwner));
		if (!pFoot)
			continue;

		// update veterancy only if higher
		if (veterancy > pFoot->Veterancy.Veterancy) {
			pFoot->Veterancy.Add(veterancy);
		}

		// select a free cell the unit can enter
		CellStruct tmpCell = MapClass::Instance->NearByLocation(cell, pType->SpeedType, -1,
			pType->MovementZone, false, 1, 1, false, false, false, false, CellStruct::Empty, false, false);

		CoordStruct crd = CellClass::Cell2Coord(tmpCell);

		// let the locomotor take care of the rest
		if (TechnoExt::CreateWithDroppod(pFoot, crd)) {
			if (!--count) {
				break;
			}
		}

		// randomize the target coodinates
		CellClass* pCell = MapClass::Instance->GetCellAt(tmpCell);
		int rnd = ScenarioClass::Instance->Random.RandomFromMax(7);

		for (int j = 0; j < 8; ++j) {

			// get the direction in an overly verbose way
			int dir = ((j + rnd) % 8) & 7;

			CellClass* pNeighbour = pCell->GetNeighbourCell(dir);
			if (pFoot->IsCellOccupied(pNeighbour, -1, -1, nullptr, true) == Move::OK)
			{
				cell = pNeighbour->MapCoords;
				break;
			}
		}

		// failed to place
		if (pFoot->InLimbo) {
			pFoot->UnInit();
		}
	}

	return true;
}

void SW_DropPod::Initialize(SWTypeExt::ExtData* pData)
{
	pData->EVA_Detected = VoxClass::FindIndexById("EVA_DropPodDetected");
	pData->EVA_Ready = VoxClass::FindIndexById("EVA_DropPodReady");
	pData->EVA_Activated = VoxClass::FindIndexById("EVA_DropPodActivated");

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Droppod;
	pData->CursorType = (int)MouseCursorType::ParaDrop;
}

void SW_DropPod::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ 
	const char* section = pData->Get()->ID;

	INI_EX exINI(pINI);
	pData->DropPod_Minimum.Read(exINI, section, "DropPod.Minimum");
	pData->DropPod_Maximum.Read(exINI, section, "DropPod.Maximum");
	pData->DropPod_Veterancy.Read(exINI, section, "DropPod.Veterancy");
	pData->DropPod_Types.Read(exINI, section, "DropPod.Types");
}
