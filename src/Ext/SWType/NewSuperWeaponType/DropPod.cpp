#include "DropPod.h"

#include <Ext/Techno/Body.h>
#include <Ext/Rules/Body.h>

bool SW_DropPod::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	SuperWeaponTypeClass* pSW = pThis->Type;
	auto pData = SWTypeExtContainer::Instance.Find(pSW);

	const auto nDeferement = pData->SW_Deferment.Get(-1);

	if (nDeferement <= 0)
		DroppodStateMachine::SendDroppods(pThis, pData, this, Coords);
	else
		this->newStateMachine(nDeferement, Coords, pThis);

	return true;
}

void SW_DropPod::Initialize(SWTypeExtData* pData)
{
	pData->This()->Action = Action(PhobosNewActionType::SuperWeaponAllowed);

	pData->EVA_Detected = VoxClass::FindIndexById("EVA_DropPodDetected");
	pData->EVA_Ready = VoxClass::FindIndexById("EVA_DropPodReady");
	pData->EVA_Activated = VoxClass::FindIndexById("EVA_DropPodActivated");

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::DropPod;
	pData->CursorType = (int)MouseCursorType::ParaDrop;

	pData->DroppodProp.Initialize();
}

void SW_DropPod::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->This()->ID;

	INI_EX exINI(pINI);
	pData->DropPod_Minimum.Read(exINI, section, "DropPod.Minimum");
	pData->DropPod_Maximum.Read(exINI, section, "DropPod.Maximum");
	pData->DropPod_Veterancy.Read(exINI, section, "DropPod.Veterancy");
	pData->DropPod_Types.Read(exINI, section, "DropPod.Types");

	pData->Droppod_RetryCount.Read(exINI, section, "DropPod.RetryCount");

	pData->DroppodProp.Read(exINI, section);
}

bool SW_DropPod::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

void DroppodStateMachine::Update()
{
	if (this->Finished())
	{
		SendDroppods(this->Super, this->GetTypeExtData(), this->GetTypeExtData()->GetNewSWType(), this->Coords);
	}
}

void DroppodStateMachine::SendDroppods(SuperClass* pSuper, SWTypeExtData* pData, SWTypeHandler* pNewType, const CellStruct& loc)
{
	pData->PrintMessage(pData->Message_Activate, pSuper->Owner);

	auto const sound = pData->SW_ActivationSound.Get(-1);
	if (sound != -1)
	{
		VocClass::PlayGlobal(sound, Panning::Center, 1.0);
	}

	const auto& Types = !pData->DropPod_Types.empty()
		? pData->DropPod_Types
		: RulesExtData::Instance()->DropPodTypes;

	if (Types.empty())
		return;

	int cMin = pData->DropPod_Minimum.Get(RulesExtData::Instance()->DropPodMinimum);
	int cMax = pData->DropPod_Maximum.Get(RulesExtData::Instance()->DropPodMaximum);

	DroppodStateMachine::PlaceUnits(pSuper, pData->DropPod_Veterancy.Get(), Types, cMin, cMax, loc);
}

// ---------------------------------------------------------------------------
// PlaceUnits — spawns drop pod units around a target cell
//
// Rolls a random count between cMin and cMax, then for each unit:
//   1. Picks a random TechnoType from the Types list
//   2. Finds a nearby free cell the unit can enter
//   3. Attempts to place it via CreateWithDroppod
//   4. On failure, retries up to RetryCount times with a new nearby cell
//   5. If all retries fail, the unit is destroyed (not leaked)
//
// After each successful placement, the search origin shifts to a
// neighbouring cell so subsequent pods spread out naturally.
// ---------------------------------------------------------------------------
void DroppodStateMachine::PlaceUnits(
	SuperClass* pSuper,
	double veterancy,
	Iterator<TechnoTypeClass*> const Types,
	int cMin,
	int cMax,
	const CellStruct& Coords)
{
	const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
	const int maxRetries = pData->Droppod_RetryCount;
	const int count = ScenarioClass::Instance->Random.RandomRanged(cMin, cMax);
	const bool needRandom = Types.size() > 1;

	CellStruct cell = Coords;

	for (int i = 0; i < count; ++i)
	{
		// --- Pick a random type ---
		const auto typeIndex = needRandom
			? ScenarioClass::Instance->Random.RandomFromMax(Types.size() - 1)
			: 0;

		if (typeIndex >= Types.size())
			continue;

		TechnoTypeClass* pType = Types[typeIndex];
		if (!pType)
			continue;

		// --- Create the unit ---
		FootClass* pFoot = static_cast<FootClass*>(pType->CreateObject(pSuper->Owner));
		if (!pFoot)
			continue;

		if (veterancy > pFoot->Veterancy.Veterancy)
			pFoot->Veterancy.Add(veterancy);

		TechnoExtContainer::Instance.Find(pFoot)->LinkedSW = pSuper;

		// --- Try to place, with retries ---
		bool placed = false;
		CellStruct searchOrigin = cell;

		for (int attempt = 0; attempt <= maxRetries; ++attempt)
		{
			CellStruct tmpCell = MapClass::Instance->NearByLocation(
				searchOrigin,
				pType->SpeedType,
				ZoneType::None,
				pType->MovementZone,
				false, 1, 1, false, false, false, false,
				CellStruct::Empty, false, false);

			CoordStruct crd = CellClass::Cell2Coord(tmpCell);

			if (TechnoExtData::CreateWithDroppod(pFoot, crd))
			{
				placed = true;
				cell = tmpCell; // shift origin for next unit
				break;
			}

			// Placement failed — nudge the search origin for next attempt
			// by picking a random neighbouring cell
			CellClass* pCell = MapClass::Instance->GetCellAt(tmpCell);
			int rnd = ScenarioClass::Instance->Random.RandomFromMax(7);

			for (int d = 0; d < 8; ++d)
			{
				FacingType dir = FacingType(((d + rnd) % 8) & 7);
				CellClass* pNeighbour = pCell->GetNeighbourCell(dir);

				if (pFoot->IsCellOccupied(pNeighbour, FacingType::None, -1, nullptr, true) == Move::OK)
				{
					searchOrigin = pNeighbour->MapCoords;
					break;
				}
			}
		}

		// --- Clean up if all attempts failed ---
		if (!placed)
		{
			if (pFoot->InLimbo)
				pFoot->UnInit();
			else
				GameDelete<true, false>(pFoot);
		}
		else
		{
			// Shift the base cell for the next unit so pods spread out.
			// Try to find a walkable neighbour to avoid clustering.
			CellClass* pPlacedCell = MapClass::Instance->GetCellAt(cell);
			int rnd = ScenarioClass::Instance->Random.RandomFromMax(7);

			for (int d = 0; d < 8; ++d)
			{
				FacingType dir = FacingType(((d + rnd) % 8) & 7);
				CellClass* pNeighbour = pPlacedCell->GetNeighbourCell(dir);

				if (pFoot->IsCellOccupied(pNeighbour, FacingType::None, -1, nullptr, true) == Move::OK)
				{
					cell = pNeighbour->MapCoords;
					break;
				}
			}
		}
	}
}