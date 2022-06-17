#include "Body.h"

enum CellFailure
{
	No = -1145,
	NotClear = No,
	InvalidCell = No,
	DifferentLevel = No
};

//Don't know if such thing exists already or not, maybe more efficient (or less?)
//DO NOT USE THIS
static int _FlatAreaLevel(ObjectClass* ignoreMe, CellClass* cell, short spacex, short spacey, int previousLevel)
{
	if (!cell)
		return CellFailure::InvalidCell;
	bool isClear = false;
	if (cell->OverlayTypeIndex == -1)
	{
		if (auto content = cell->FirstObject)
		{
			if (ignoreMe && content->UniqueID == ignoreMe->UniqueID && !content->NextObject)
				isClear = true;
		}
		else isClear = true;
	}
	if (!isClear)
		return CellFailure::NotClear;

	const int level = cell->GetLevel();
	if (level != previousLevel)
		return CellFailure::DifferentLevel;

	if (spacex <= 1 && spacey <= 1)
		return level;

	int Slevel = _FlatAreaLevel(ignoreMe, cell->GetNeighbourCell(Direction::S), 1, spacey - 1, level);

	if (Slevel == CellFailure::No)
		return CellFailure::No;

	if (spacex == 1)
		return Slevel == previousLevel ? Slevel : CellFailure::DifferentLevel;


	int Elevel = _FlatAreaLevel(ignoreMe, cell->GetNeighbourCell(Direction::E), spacex - 1, spacey, level);
	if (Elevel != CellFailure::No
		&& Slevel == Elevel && Elevel == level && level == previousLevel)
		return level;
	else
		return CellFailure::DifferentLevel;
}

static inline bool EnoughSpaceToExpand(ObjectClass* ignoreThis, CellClass* cell, short spacex, short spacey)
{
	return _FlatAreaLevel(ignoreThis, cell, spacex, spacey, cell->GetLevel()) != CellFailure::No;
}

//issue #621: let the mcv in hunt mission deploy asap
void MCVFindBetterPlace(TechnoClass* pThis)
{
	if (pThis->WhatAmI() == AbstractType::Unit &&
	pThis->GetTechnoType()->Category == Category::Support &&
	!pThis->GetOwningHouse()->ControlledByHuman()
		)
	{// All mcv at the skirmish beginning is hunting
		const auto pFoot = abstract_cast<UnitClass*>(pThis);
		const auto deployType = pFoot->Type->DeploysInto;
		if (pFoot && deployType)
		{
			short XWidth = deployType->GetFoundationWidth();
			short YWidth = deployType->GetFoundationHeight(true);
			if (!pFoot->Destination &&
				(pFoot->GetCurrentMission() == Mission::Hunt || pFoot->GetCurrentMission() == Mission::Unload) &&
				!EnoughSpaceToExpand(pFoot, pFoot->GetCell(), XWidth, YWidth)
				)
			{//for other locomotors they don't have a destination, so give it the nearest location
				CellStruct coord = pFoot->GetCell()->MapCoords;
				coord = MapClass::Instance->NearByLocation(
					coord, pFoot->Type->SpeedType, -1, MovementZone::Normal, false,
					XWidth, YWidth,
					true, false, false, false, CellStruct::Empty, false, true);

				if (const auto tgtCell = MapClass::Instance->TryGetCellAt(coord))
				{
					pFoot->SetDestination(tgtCell, true);
					pFoot->QueueMission(Mission::Guard, false);
				}
			}
		}
	}
}

//Not working for no ore map? whatever
void HarvesterLocoFix(TechnoClass* pThis)
{
	if (pThis->WhatAmI() == AbstractType::Unit)
	{
		const auto pFoot = abstract_cast<UnitClass*>(pThis);
		if (pFoot && !pThis->IsSelected && pFoot->Type->Harvester &&
			(pFoot->Type->Locomotor == LocomotionClass::CLSIDs::Tunnel ||
				pFoot->Type->Locomotor == LocomotionClass::CLSIDs::Jumpjet) &&
			(pFoot->GetCurrentMission() == Mission::Guard ||
				pFoot->GetCurrentMission() == Mission::Area_Guard) &&
			!TechnoExt::IsHarvesting(pThis) && !pFoot->Locomotor->Is_Really_Moving_Now()
		)
		{
			pFoot->QueueMission(Mission::Harvest, true);
		}
	}
}

