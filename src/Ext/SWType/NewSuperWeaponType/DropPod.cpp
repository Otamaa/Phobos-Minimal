#include "DropPod.h"

#include <Ext/Techno/Body.h>
#include <Ext/Rules/Body.h>

// Constants for better maintainability
static constexpr int MAX_NEIGHBOR_DIRECTIONS = 8;
static constexpr int DEFAULT_RETRY_MULTIPLIER = 3;

std::vector<const char*> SW_DropPod::GetTypeString() const
{
	return { "DropPod" };
}

bool SW_DropPod::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (!pThis || !pThis->Type)
		return false;

	SuperWeaponTypeClass* pSW = pThis->Type;
	auto pData = SWTypeExtContainer::Instance.Find(pSW);
	
	if (!pData)
		return false;

	const auto nDeferement = pData->SW_Deferment.Get(-1);

	if (nDeferement <= 0)
		DroppodStateMachine::SendDroppods(pThis, pData, this, Coords);
	else
		this->newStateMachine(nDeferement, Coords, pThis);

	return true;
}

void SW_DropPod::Initialize(SWTypeExtData* pData)
{
	if (!pData || !pData->AttachedToObject)
		return;

	pData->AttachedToObject->Action = Action(AresNewActionType::SuperWeaponAllowed);

	pData->EVA_Detected = VoxClass::FindIndexById("EVA_DropPodDetected");
	pData->EVA_Ready = VoxClass::FindIndexById("EVA_DropPodReady");
	pData->EVA_Activated = VoxClass::FindIndexById("EVA_DropPodActivated");

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::DropPod;
	pData->CursorType = static_cast<int>(MouseCursorType::ParaDrop);

	pData->DroppodProp.Initialize();
}

void SW_DropPod::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	if (!pData || !pData->AttachedToObject || !pINI)
		return;

	const char* section = pData->AttachedToObject->ID;
	if (!section)
		return;

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
	if (!pData || !pBuilding)
		return false;

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
		auto pTypeExtData = this->GetTypeExtData();
		if (pTypeExtData)
		{
			SendDroppods(this->Super, pTypeExtData, this->Type, this->Coords);
		}
	}
}

void DroppodStateMachine::SendDroppods(SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, const CellStruct& loc)
{
	if (!pSuper || !pData || !pSuper->Owner)
		return;

	pData->PrintMessage(pData->Message_Activate, pSuper->Owner);

	const auto sound = pData->SW_ActivationSound.Get(-1);
	if (sound != -1) {
		VocClass::PlayGlobal(sound, Panning::Center, 1.0);
	}

	// collect the options
	const auto& Types = !pData->DropPod_Types.empty()
		? pData->DropPod_Types
		: RulesExtData::Instance()->DropPodTypes;

	// quick way out
	if (Types.empty()) {
		return;
	}

	int cMin = pData->DropPod_Minimum.Get(RulesExtData::Instance()->DropPodMinimum);
	int cMax = pData->DropPod_Maximum.Get(RulesExtData::Instance()->DropPodMaximum);

	// Validate min/max values
	if (cMin < 0) cMin = 0;
	if (cMax < cMin) cMax = cMin;

	const double veterancy = std::max(0.0, pData->DropPod_Veterancy.Get());
	
	DroppodStateMachine::PlaceUnits(pSuper, veterancy, Types, cMin, cMax, loc, false);
}

void DroppodStateMachine::PlaceUnits(SuperClass* pSuper, double veterancy, Iterator<TechnoTypeClass*> const Types, int cMin, int cMax, const CellStruct& Coords, bool retries)
{
	if (!pSuper || !pSuper->Type || !pSuper->Owner || Types.empty() || cMin < 0 || cMax < cMin)
		return;

	const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
	if (!pData)
		return;

	const int totalUnitsToPlace = ScenarioClass::Instance->Random.RandomRanged(cMin, cMax);
	if (totalUnitsToPlace <= 0)
		return;

	const int maxRetries = std::max(1, pData->Droppod_RetryCount.Get());
	const size_t typesCount = Types.size();
	const bool needRandom = typesCount > 1;

	CellStruct currentCell = Coords;
	int successfulPlacements = 0;

	for (int unitIndex = 0; unitIndex < totalUnitsToPlace; ++unitIndex)
	{
		bool unitPlaced = false;
		
		for (int attempt = 0; attempt < maxRetries && !unitPlaced; ++attempt)
		{
			// Get a random type from the list
			const size_t typeIndex = needRandom ? 
				ScenarioClass::Instance->Random.RandomFromMax(static_cast<int>(typesCount - 1)) : 0;
			TechnoTypeClass* pType = Types[typeIndex];
			
			if (!pType)
				continue;

			// Create unit instance
			FootClass* pFoot = static_cast<FootClass*>(pType->CreateObject(pSuper->Owner));
			if (!pFoot)
				continue;

			// Update veterancy only if higher
			if (veterancy > pFoot->Veterancy.Veterancy)
			{
				pFoot->Veterancy.Add(veterancy);
			}

			// Find suitable placement location
			CellStruct targetCell = MapClass::Instance->NearByLocation(
				currentCell, pType->SpeedType, ZoneType::None,
				pType->MovementZone, false, 1, 1, false, false, false, false, 
				CellStruct::Empty, false, false);

			CoordStruct coords = CellClass::Cell2Coord(targetCell);

			// Link to superweapon and attempt placement
			TechnoExtContainer::Instance.Find(pFoot)->LinkedSW = pSuper;
			
			if (TechnoExtData::CreateWithDroppod(pFoot, coords))
			{
				unitPlaced = true;
				++successfulPlacements;
				
				// Find next placement location for subsequent units
				CellClass* pCell = MapClass::Instance->GetCellAt(targetCell);
				if (pCell)
				{
					const int randomOffset = ScenarioClass::Instance->Random.RandomFromMax(MAX_NEIGHBOR_DIRECTIONS - 1);
					
					for (int direction = 0; direction < MAX_NEIGHBOR_DIRECTIONS; ++direction)
					{
						const FacingType facingDir = static_cast<FacingType>((direction + randomOffset) % MAX_NEIGHBOR_DIRECTIONS);
						CellClass* pNeighbor = pCell->GetNeighbourCell(facingDir);
						
						if (pNeighbor && pFoot->IsCellOccupied(pNeighbor, FacingType::None, -1, nullptr, true) == Move::OK)
						{
							currentCell = pNeighbor->MapCoords;
							break;
						}
					}
				}
			}
			else
			{
				// Cleanup failed unit
				if (pFoot->InLimbo)
				{
					pFoot->UnInit();
				}
			}
		}
	}
}


