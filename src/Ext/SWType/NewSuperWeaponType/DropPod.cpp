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
	auto pData = SWTypeExtContainer::Instance.Find(pSW);

	const auto nDeferement = pData->SW_Deferment.Get(-1);

	if (nDeferement <= 0)
		DroppodStateMachine::SendDroppods(pThis, pData, this, Coords);
	else
		this->newStateMachine(nDeferement < 0 ? 20 : nDeferement, Coords, pThis);

	return true;
}

void SW_DropPod::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action(AresNewActionType::SuperWeaponAllowed);

	pData->EVA_Detected = VoxClass::FindIndexById("EVA_DropPodDetected");
	pData->EVA_Ready = VoxClass::FindIndexById("EVA_DropPodReady");
	pData->EVA_Activated = VoxClass::FindIndexById("EVA_DropPodActivated");

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::DropPod;
	pData->CursorType = (int)MouseCursorType::ParaDrop;

	pData->Droppod_PodImage_Infantry = FileSystem::LoadSHPFile(GameStrings::POD_SHP);
	pData->Droppod_Puff = RulesClass::Instance->DropPodPuff;
	pData->Droppod_Angle = RulesClass::Instance->DropPodAngle;
	pData->Droppod_Speed = RulesClass::Instance->DropPodSpeed;
	pData->Droppod_Height = RulesClass::Instance->DropPodHeight;
	pData->Droppod_Weapon = RulesClass::Instance->DropPodWeapon;

	for (auto const Pod : RulesClass::Instance->DropPod)
		pData->Droppod_GroundPodAnim.push_back(Pod);

	pData->Droppod_Trailer = RulesExtData::Instance()->DropPodTrailer;
	pData->Droppod_AtmosphereEntry = RulesClass::Instance->AtmosphereEntry;
}

void SW_DropPod::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->AttachedToObject->ID;

	INI_EX exINI(pINI);
	pData->DropPod_Minimum.Read(exINI, section, "DropPod.Minimum");
	pData->DropPod_Maximum.Read(exINI, section, "DropPod.Maximum");
	pData->DropPod_Veterancy.Read(exINI, section, "DropPod.Veterancy");
	pData->DropPod_Types.Read(exINI, section, "DropPod.Types");

	Helpers::Alex::remove_non_paradroppables(pData->DropPod_Types, section, "DropPod.Types");

	pData->Droppod_RetryCount.Read(exINI, section, "DropPod.RetryCount");

	pData->Droppod_PodImage_Infantry.Read(exINI, section, "DropPod.PodImageInfantry");
	pData->Droppod_Puff.Read(exINI, section, "DropPod.Puff");
	pData->Droppod_Angle.Read(exINI, section, "DropPod.Angle");

	if (pData->Droppod_Angle >= 1.178097245096172) {
		pData->Droppod_Angle = 1.178097245096172;
	}

	if (pData->Droppod_Angle <= 0.3926990816987241) {
		pData->Droppod_Angle = 0.3926990816987241;
	}
	pData->Droppod_Speed.Read(exINI, section, "DropPod.Speed");
	pData->Droppod_Height.Read(exINI, section, "DropPod.Height");
	pData->Droppod_Weapon.Read(exINI, section, "DropPod.Weapon");
	pData->Droppod_GroundPodAnim.Read(exINI, section, "DropPod.Pods");

	pData->Droppod_Trailer.Read(exINI, section, "DropPod.Trailer");
	pData->Droppod_AtmosphereEntry.Read(exINI, section, "DropPod.AtmosphereEntry");
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
		this->SendDroppods(this->Super , this->GetTypeExtData() , this->Type , this->Coords);
	}
}

void DroppodStateMachine::SendDroppods(SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, const CellStruct& loc)
{
	pData->PrintMessage(pData->Message_Activate, pSuper->Owner);

	auto const sound = pData->SW_ActivationSound.Get(-1);
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

	DroppodStateMachine::PlaceUnits(pSuper, pData->DropPod_Veterancy.Get(), Types, cMin, cMax, loc, false);
}

void DroppodStateMachine::PlaceUnits(SuperClass* pSuper , double veterancy , Iterator<TechnoTypeClass*> const Types, int cMin ,int cMax , const CellStruct& Coords, bool retries)
{
	const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
	// three times more tries than units to place.
	const int count = ScenarioClass::Instance->Random.RandomRanged(cMin, cMax);
	CellStruct cell = Coords;
	std::vector<std::pair<bool , int>> Succeededs(count , {false , pData->Droppod_RetryCount });
	const bool needRandom = Types.size() > 1;

	for (auto&[status , retrycount] : Succeededs)
	{
		if (!status && retrycount)
		{
			// get a random type from the list and create an instance
			TechnoTypeClass* pType = Types[needRandom ? ScenarioClass::Instance->Random.RandomFromMax(Types.size() - 1) : 0];

			if (!pType || pType->Strength <= 0 || pType->WhatAmI() == BuildingTypeClass::AbsID)
			{
				continue;
			}

			FootClass* pFoot = static_cast<FootClass*>(pType->CreateObject(pSuper->Owner));
			if (!pFoot)
				continue;

			// update veterancy only if higher
			if (veterancy > pFoot->Veterancy.Veterancy)
			{
				pFoot->Veterancy.Add(veterancy);
			}

			// select a free cell the unit can enter
			CellStruct tmpCell = MapClass::Instance->NearByLocation(cell, pType->SpeedType, -1,
				pType->MovementZone, false, 1, 1, false, false, false, false, CellStruct::Empty, false, false);

			CoordStruct crd = CellClass::Cell2Coord(tmpCell);

			// let the locomotor take care of the rest
			TechnoExtContainer::Instance.Find(pFoot)->LinkedSW = pSuper;
			if (TechnoExtData::CreateWithDroppod(pFoot, crd)) {
				status = true;
			} else {
				--retrycount;
			}

			// randomize the target coodinates
			CellClass* pCell = MapClass::Instance->GetCellAt(tmpCell);
			int rnd = ScenarioClass::Instance->Random.RandomFromMax(7);

			for (int j = 0; j < 8; ++j)
			{

				// get the direction in an overly verbose way
				FacingType dir = FacingType(((j + rnd) % 8) & 7);

				CellClass* pNeighbour = pCell->GetNeighbourCell(dir);
				if (pFoot->IsCellOccupied(pNeighbour, FacingType::None, -1, nullptr, true) == Move::OK)
				{
					cell = pNeighbour->MapCoords;
					break;
				}
			}

			// failed to place
			if (pFoot->InLimbo)
			{
				pFoot->UnInit();
			}
		}
	}
}
