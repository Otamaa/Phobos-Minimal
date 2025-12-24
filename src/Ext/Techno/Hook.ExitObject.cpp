#include <CellStruct.h>
#include <CoordStruct.h>

#include <HouseClass.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>

#include <Ext/Building/Body.h>

#include <Misc/Ares/Hooks/Header.h>

#ifdef hooks

// infantry exiting hospital get their focus reset, but not for armory
ASMJIT_PATCH(0x444D26, BuildingClass_KickOutUnit_ArmoryExitBug, 0x6)
{
	GET(BuildingTypeClass* const, pType, EDX);
	R->AL(pType->Hospital || pType->Armory);
	return 0x444D2C;
}

// BuildingClass_KickOutUnit_PreventClone
DEFINE_JUMP(LJMP, 0x4449DF, 0x444A53);

ASMJIT_PATCH(0x4444B3, BuildingClass_KickOutUnit_NoAlternateKickout, 6)
{
	GET(FakeBuildingClass*, pThis, ESI);
	return pThis->Type->Factory == AbstractType::None
		|| pThis->_GetTypeExtData()->CloningFacility.Get()
		? 0x4452C5 : 0x0;
}

ASMJIT_PATCH(0x445355, BuildingClass_KickOutUnit_Firewall, 6)
{
	GET(BuildingClass*, Factory, ESI);
	GET(BuildingClass*, B, EDI);
	GET_STACK(CellStruct, CenterPos, 0x20);

	FirewallFunctions::BuildLines(B, CenterPos, Factory->Owner);

	return 0;
}

ASMJIT_PATCH(0x444B83, BuildingClass_ExitObject_BarracksExitCell, 0x7)
{
	enum { SkipGameCode = 0x444C7C };
	GET(FakeBuildingClass*, pThis, ESI);
	GET(int, xCoord, EBP);
	GET(int, yCoord, EDX);
	REF_STACK(CoordStruct, resultCoords, STACK_OFFSET(0x140, -0x108));

	if (pThis->_GetTypeExtData()->BarracksExitCell.isset())
	{
		auto const exitCoords = pThis->Type->ExitCoord;
		resultCoords = CoordStruct { xCoord + exitCoords.X, yCoord + exitCoords.Y, exitCoords.Z };
		return SkipGameCode;
	}

	return 0;
}

// request radio contact then get land dir
ASMJIT_PATCH(0x444014, BuildingClass_ExitObject_PoseDir_AirportBound, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, ECX);

	pThis->SendCommand(RadioCommand::RequestLink, pAir);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	pAir->SetLocation(pThis->GetDockCoords(pAir));
	pAir->DockedTo = pThis;
	FacingType result = BuildingExtData::GetPoseDir(pAir, pThis);
	const DirStruct dir { result };

	if (RulesExtData::Instance()->ExpandAircraftMission)
		pAir->PrimaryFacing.Set_Current(dir);

	pAir->SecondaryFacing.Set_Current(dir);

	//if (pAir->GetHeight() > 0)
	//	AircraftTrackerClass::Instance->Add(pAir);

	return 0x444053;
}

// there no radio contact happening here
// so the result mostlikely building facing
ASMJIT_PATCH(0x443FD8, BuildingClass_ExitObject_PoseDir_NotAirportBound, 0x8)
{
	enum { RetCreationFail = 0x444EDE, RetCreationSucceeded = 0x443FE0 };

	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, EBP);

	if (R->AL())
	{
		pAir->DockedTo = pThis;
		const DirStruct dir { ((int)BuildingExtData::GetPoseDir(pAir, pThis) << 13) };

		if (RulesExtData::Instance()->ExpandAircraftMission)
			pAir->PrimaryFacing.Set_Current(dir);

		pAir->SecondaryFacing.Set_Current(dir);

		//if (pAir->GetHeight() > 0)
		//	AircraftClass::AircraftTracker_4134A0(pAir);

		return RetCreationSucceeded;
	}

	return RetCreationFail;
}

ASMJIT_PATCH(0x444113, BuildingClass_ExitObject_NavalProductionFix1, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(UnitClass* const, pObject, EDI);

	if (pObject->Type->Naval)
	{
		HouseExtContainer::Instance.Find(pThis->Owner)->ProducingNavalUnitTypeIndex = -1;
	}
	else
	{
		pThis->Owner->ProducingUnitTypeIndex = -1;
	}

	return 0x44411F;
}

ASMJIT_PATCH(0x44540D, BuildingClass_ExitObject_WallTowers, 0x5)
{
	GET(BuildingClass* const, pThis, EDI);
	R->EDX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x445424 : 0x4454D4;
}

ASMJIT_PATCH(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(FakeHouseClass*, pHouse, EDX);
	GET(BuildingClass*, pThis, ESI);

	auto pExt = pHouse->_GetExtData();

	if (pThis == pExt->Factory_AircraftType)
		pExt->Factory_AircraftType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(FakeHouseClass*, pHouse, EAX);
	GET(BuildingClass*, pThis, ESI);

	auto pExt = pHouse->_GetExtData();

	if (pThis == pExt->Factory_BuildingType)
		pExt->Factory_BuildingType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(FakeHouseClass*, pHouse, EAX);
	GET(BuildingClass*, pThis, ESI);

	auto pExt = pHouse->_GetExtData();

	if (pThis == pExt->Factory_InfantryType)
		pExt->Factory_InfantryType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	auto pHouseExt = HouseExtContainer::Instance.Find(pFactory->Owner);

	if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == pFactory)
		pHouseExt->Factory_NavyType = nullptr;
	else if (!pUnit->Type->Naval && pHouseExt->Factory_VehicleType == pFactory)
		pHouseExt->Factory_VehicleType = nullptr;

	return 0;
}

// Should not kick out units if the factory building is in construction process
ASMJIT_PATCH(0x4444A0, BuildingClass_KickOutUnit_NoKickOutInConstruction, 0xA)
{
	enum { ThisIsOK = 0x444565, ThisIsNotOK = 0x4444B3 };

	GET(BuildingClass* const, pThis, ESI);

	const auto mission = pThis->GetCurrentMission();

	return (mission == Mission::Unload || mission == Mission::Construction) ? ThisIsNotOK : ThisIsOK;
}

ASMJIT_PATCH(0x4440B0, BuildingClass_KickOutUnit_CloningFacility, 0x6)
{
	enum { CheckFreeLinks = 0x4440BA, ContinueIn = 0x4440D7 };

	GET(BuildingTypeClass*, pFactoryType, EAX);

	if (!pFactoryType->WeaponsFactory
		|| BuildingTypeExtContainer::Instance.Find(pFactoryType)->CloningFacility)
		return CheckFreeLinks;

	return ContinueIn;
}

ASMJIT_PATCH(0x444159, BuildingClass_KickoutUnit_WeaponFactory_Rubble, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pObj, EDI);

	if (!pThis->Type->WeaponsFactory)
		return 0x4445FB; //not a weapon factory

	const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	if (pExt->RubbleDestroyed)
	{
		if (pThis->Type->Factory == pObj->GetTechnoType()->WhatAmI() && pThis->Factory && pThis->Factory->Object == pObj)
			return 0x444167; //continue check

		if (pObj->WhatAmI() == AbstractType::Infantry)
			return 0x4445FB; // just eject
	}

	return 0x444167; //continue check
}

ASMJIT_PATCH(0x444DC9, BuildingClass_KickOutUnit_Barracks, 0x9)
{
	GET(BuildingClass*, pThis, ESI);
	GET(FootClass*, pProduct, EDI);
	GET(RadioCommand, respond, EAX);

	if (respond == RadioCommand::AnswerPositive)
	{
		pThis->SendCommand(RadioCommand::RequestUnload, pProduct);

		if (auto pDest = pProduct->ArchiveTarget)
		{
			pProduct->SetDestination(pDest, true);
			return 0x444971;
		}

		pProduct->Scatter(CoordStruct::Empty, true, false);
	}

	return 0x444971;
}

ASMJIT_PATCH(0x444DBC, BuildingClass_KickOutUnit_Infantry, 5)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	// turn it off
	--Unsorted::ScenarioInit;

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	// turn it back on so the game can turn it off again
	++Unsorted::ScenarioInit;

	return 0;
}

ASMJIT_PATCH(0x4445F6, BuildingClass_KickOutUnit_Clone_NonNavalUnit, 5)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	// turn it off
	--Unsorted::ScenarioInit;

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	// turn it back on so the game can turn it off again
	++Unsorted::ScenarioInit;

	return 0x444971;
}

ASMJIT_PATCH(0x44441A, BuildingClass_KickOutUnit_Clone_NavalUnit, 6)
{
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	TechnoExt_ExtData::KickOutClones(Factory, Production);

	return 0;
}

ASMJIT_PATCH(0x4444E2, BuildingClass_KickOutUnit_FindAlternateKickout, 6)
{
	GET(BuildingClass*, Src, ESI);
	GET(BuildingClass*, Tst, EBP);
	GET(TechnoClass*, Production, EDI);

	if (Src != Tst
	 && Tst->GetCurrentMission() == Mission::Guard
	 && Tst->Type->Factory == Src->Type->Factory
	 && Tst->Type->Naval == Src->Type->Naval
	 && TechnoTypeExtData::CanBeBuiltAt(Production->GetTechnoType(), Tst->Type)
	 && !Tst->Factory)
	{
		return 0x44451F;
	}

	return 0x444508;
}

// Buildable-upon TechnoTypes Hook #6 -> sub_443C60 - Try to clean up the building space when AI is building
ASMJIT_PATCH(0x4451F8, BuildingClass_KickOutUnit_CleanUpAIBuildingSpace, 0x6)
{
	enum
	{
		CanBuild = 0x4452F0,
		TemporarilyCanNotBuild = 0x445237,
		CanNotBuild = 0x4454E6,
		BuildSucceeded = 0x4454D4,
		BuildFailed = 0x445696
	};

	GET(BaseNodeClass*, pBaseNode, EBX);
	GET(BuildingClass*, pBuilding, EDI);
	GET(BuildingClass*, pFactory, ESI);
	GET(CellStruct, topLeftCell, EDX);

	const auto pBuildingType = pBuilding->Type;

	if (RulesExtData::Instance()->AIForbidConYard && pBuildingType->ConstructionYard)
	{
		if (pBaseNode)
		{
			pBaseNode->Placed = true;
			pBaseNode->Attempts = 0;
		}
		return BuildFailed;
	}

	// Clean up invalid walls nodes
	if (RulesExtData::Instance()->AICleanWallNode && pBuildingType->Wall)
	{
		auto notValidWallNode = [topLeftCell]()
			{
				const auto pCell = MapClass::Instance->GetCellAt(topLeftCell);

				for (int i = 0; i < 8; ++i)
				{
					if (const auto pAdjBuilding = pCell->GetNeighbourCell(static_cast<FacingType>(i))->GetBuilding())
					{
						if (pAdjBuilding->Type->ProtectWithWall)
							return false;
					}
				}

				return true;
			};

		if (notValidWallNode())
			return CanNotBuild;
	}

	const auto pHouse = pFactory->Owner;
	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

	if (pTypeExt->LimboBuild)
	{
		BuildingTypeExtData::CreateLimboBuilding(pBuilding, pBuildingType, pHouse, pTypeExt->LimboBuildID);
		if (pBaseNode)
		{
			pBaseNode->Placed = true;
			pBaseNode->Attempts = 0;

			if (pHouse->ProducingBuildingTypeIndex == pBuildingType->ArrayIndex)
				pHouse->ProducingBuildingTypeIndex = -1;
		}
		BuildingExtData::PlayConstructionYardAnim<true>(pFactory);
		return BuildSucceeded;
	}

	if (!RulesExtData::Instance()->ExtendedBuildingPlacing)
		return 0;

	if (topLeftCell != CellStruct::Empty && !pBuildingType->PlaceAnywhere)
	{
		if (!pBuildingType->PowersUpBuilding[0])
		{
			bool noOccupy = true;
			bool canBuild = BuildingExtData::CheckBuildingFoundation(pBuildingType, topLeftCell, pHouse, noOccupy);
			const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
			auto& place = pBuildingType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

			do
			{
				if (canBuild)
				{
					if (noOccupy)
						break; // Can Build

					do
					{
						if (topLeftCell != place.TopLeft || pBuildingType != place.Type) // New command
						{
							place.Type = pBuildingType;
							place.DrawType = pBuildingType;
							place.TopLeft = topLeftCell;
						}

						if (!place.Timer.HasTimeLeft())
						{
							place.Timer.Start(40);

							if (BuildingTypeExtData::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
								break; // No place for cleaning
						}

						return TemporarilyCanNotBuild;
					}
					while (false);
				}

				BuildingExtData::ClearPlacingBuildingData(&place);
				return CanNotBuild;
			}
			while (false);

			BuildingExtData::ClearPlacingBuildingData(&place);
		}
		else
		{
			const auto pCell = MapClass::Instance->GetCellAt(topLeftCell);
			const auto pCellBuilding = pCell->GetBuilding();

			if (!pCellBuilding || !pCellBuilding->CanUpgrade(pBuildingType, pHouse)) // CanUpgradeBuilding
				return CanNotBuild;
		}
	}

	if (pBuilding->Unlimbo(CoordStruct { (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		BuildingExtData::PlayConstructionYardAnim(pFactory);
		return CanBuild;
	}

	return CanNotBuild;
}

#endif

KickOutResult FakeBuildingClass::__ExitObject(TechnoClass* pObject, CellStruct exitCell)
{
	if (!pObject)
		return KickOutResult::Failed;

	int flushResult = 0;
	pObject->IsInPlayfield = true;

	auto absType = pObject->WhatAmI();
	auto pHouseExt = HouseExtContainer::Instance.Find(this->Owner);

	switch (absType)
	{
	case AbstractType::Aircraft: // 0x443CB4
	{
		AircraftClass* pAircraft = cast_to<AircraftClass*>(pObject);

		this->Owner->WhimpOnMoney(absType);

		// --- HOOK 0x443CCA (0xA) ---
		if (this == pHouseExt->Factory_AircraftType)
			pHouseExt->Factory_AircraftType = nullptr;
		// --- END HOOK ---

		this->Owner->ProducingAircraftTypeIndex = -1;

		const bool inRadioContact = this->HasLinkOrFreeSlot(pObject);
		const bool ionStormActive = this->Owner->IonSensitivesShouldBeOffline();
		DirType facing = (DirType)RulesClass::Instance->PoseDir;

		if (inRadioContact || ionStormActive && !pAircraft->Type->AirportBound) {
			pAircraft->MarkDownSetZ(0);

			++Unsorted::ScenarioInit;

			if (ionStormActive) {
				CellStruct nearbyCell;
				pAircraft->NearbyLocation(&nearbyCell, this);

				CoordStruct unlimboCoord;
				unlimboCoord.X = (nearbyCell.X << 8) + 128;
				unlimboCoord.Y = (nearbyCell.Y << 8) + 128;
				unlimboCoord.Z = 0;

				// --- HOOK 0x443FD8 (0x8) ---
				if (pAircraft->Unlimbo(unlimboCoord, facing)) {
					pAircraft->DockedTo = this;
					FacingType poseDir = BuildingExtData::GetPoseDir(pAircraft, this);
					DirStruct dir { static_cast<int>(poseDir) << 13 };

					if (RulesExtData::Instance()->ExpandAircraftMission)
						pAircraft->PrimaryFacing.Set_Current(dir);

					pAircraft->SecondaryFacing.Set_Current(dir);
					--Unsorted::ScenarioInit;
					return KickOutResult::Succeeded;
				}
				// --- END HOOK ---

				--Unsorted::ScenarioInit;
				return KickOutResult::Failed;
			} else {
				CoordStruct dockingCoord;
				this->GetDockCoords(&dockingCoord, pAircraft);

				if (pAircraft->Unlimbo(dockingCoord, facing)) {

					// --- HOOK 0x444014 (0x5) ---
					this->SendCommand(RadioCommand::RequestLink, pAircraft);
					this->SendCommand(RadioCommand::RequestTether, pAircraft);
					pAircraft->SetLocation(this->GetDockCoords(pAircraft));
					pAircraft->DockedTo = this;

					FacingType poseDir = BuildingExtData::GetPoseDir(pAircraft, this);
					DirStruct dir { poseDir };

					if (RulesExtData::Instance()->ExpandAircraftMission)
						pAircraft->PrimaryFacing.Set_Current(dir);

					pAircraft->SecondaryFacing.Set_Current(dir);
					// --- END HOOK ---

					if (this->ArchiveTarget && !pAircraft->Type->AirportBound)
					{
						pAircraft->SetDestination(this->ArchiveTarget, true);
						pAircraft->QueueMission(Mission::Move, 0);
					}

					--Unsorted::ScenarioInit;
					return KickOutResult::Succeeded;
				}
			}

			--Unsorted::ScenarioInit;
			return KickOutResult::Failed;
		} else {
			if (pAircraft->Type->AirportBound)
				return KickOutResult::Failed;

			int mapLocalY = MapClass::MapLocalSize->Y;
			int mapWidth = MapClass::MapSize->Width;
			int mapLocalX = MapClass::MapLocalSize->X;
			int mapLocalWidth = MapClass::MapLocalSize->Width;
			int mapLocalHeight = MapClass::MapLocalSize->Height;

			CellStruct spawnCell;
			spawnCell.X = static_cast<short>(mapLocalY + mapLocalX + 1);
			spawnCell.Y = static_cast<short>(mapWidth - mapLocalX + mapLocalY);

			CoordStruct centerCoord= this->GetCoords();

			int buildingCellX = centerCoord.X / 256;
			int buildingCellY = centerCoord.Y / 256;

			if ((buildingCellX - spawnCell.X) - (buildingCellY - spawnCell.Y) > mapLocalWidth) {
				spawnCell.X = static_cast<short>(mapLocalWidth + spawnCell.X - 1);
				spawnCell.Y = static_cast<short>(spawnCell.Y - mapLocalWidth);
			} else {
				spawnCell.X--;
			}

			int randomOffset = ScenarioClass::Instance->Random.RandomFromMax(mapLocalHeight);
			spawnCell.X += randomOffset;
			spawnCell.Y += randomOffset;

			CoordStruct spawnCoord = CellClass::Cell2Coord(spawnCell);

			++Unsorted::ScenarioInit;

			if (pAircraft->Unlimbo(spawnCoord, DirType::North)) {
				AbstractClass* archiveTarget = this->ArchiveTarget;

				if (archiveTarget) {
					pAircraft->SetDestination(archiveTarget, true);
					pAircraft->QueueMission(Mission::Move, 0);
				} else {
					CellStruct nearbyCell;
					pAircraft->NearbyLocation(&nearbyCell, this);

					if (nearbyCell == CellStruct::Empty)
						pAircraft->SetDestination(nullptr, true);
					else {
						pAircraft->SetDestination(MapClass::Instance->GetCellAt(nearbyCell), true);
					}

					pAircraft->QueueMission(Mission::Move, 0);
				}

				--Unsorted::ScenarioInit;
				return KickOutResult::Succeeded;
			}

			--Unsorted::ScenarioInit;
			return KickOutResult::Failed;
		}
	}
	case AbstractType::Unit:     // 0x444096
	case AbstractType::Infantry:
	{
		BuildingTypeClass* pType = this->Type;

		// --- HOOK 0x4440B0 (0x6) ---
		if (!pType->Hospital && !pType->Armory) {
			if (!pType->WeaponsFactory || BuildingTypeExtContainer::Instance.Find(pType)->CloningFacility) {
				if (!this->HasFreeLink())
					return KickOutResult::Busy;
			}
		}
		// --- END HOOK ---

		if (!pType->Hospital && !pType->Armory) {
			this->Owner->WhimpOnMoney(absType);

			if (absType == AbstractType::Unit) {
				// --- HOOK 0x444113 (0x6) ---
				UnitClass* pUnit = static_cast<UnitClass*>(pObject);
				if (pUnit->Type->Naval)
					HouseExtContainer::Instance.Find(this->Owner)->ProducingNavalUnitTypeIndex = -1;
				else
					this->Owner->ProducingUnitTypeIndex = -1;
				// --- END HOOK ---

				// --- HOOK 0x444119 (0x6) ---
				if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == this)
					pHouseExt->Factory_NavyType = nullptr;
				else if (!pUnit->Type->Naval && pHouseExt->Factory_VehicleType == this)
					pHouseExt->Factory_VehicleType = nullptr;
				// --- END HOOK ---
			}

			if (absType == AbstractType::Infantry) {
				// --- HOOK 0x444131 (0x6) ---
				if (this == pHouseExt->Factory_InfantryType)
					pHouseExt->Factory_InfantryType = nullptr;
				// --- END HOOK ---

				this->Owner->ProducingInfantryTypeIndex = -1;
			}
		}

		// Refinery / Weeder - 0x444145
		if (pType->Refinery || pType->Weeder) {
			if (absType != AbstractType::Unit) {
				pObject->Scatter(CoordStruct::Empty, true, false);
				return KickOutResult::Failed;
			}

			CoordStruct centerCoord = this->Location;

			CellStruct exitCell4 = CellClass::Coord2Cell(centerCoord) + CellSpread::AdjacentCell[5];

			++Unsorted::ScenarioInit;

			CoordStruct unlimboCoord = CellClass::Cell2Coord(exitCell4 + CellSpread::AdjacentCell[4]);

			if (pObject->Unlimbo(unlimboCoord, DirType::SouthWest)) {
				DirStruct facing;
				facing.Raw = 0x8000;
				pObject->PrimaryFacing.Set_Current(facing);
				pObject->QueueMission(Mission::Harvest, 0);
			}

			--Unsorted::ScenarioInit;
			return KickOutResult::Failed;
		}

		// Weapons Factory - 0x444159
		// --- HOOK 0x444159 (0x6) ---
		if (pType->WeaponsFactory) {
			auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);

			if (pTypeExt->RubbleDestroyed) {
				if (!(pType->Factory == pObject->GetTechnoType()->WhatAmI()
					&& this->Factory && this->Factory->Object == pObject)) {
					if (absType == AbstractType::Infantry)
						goto BarracksExit;
				}
			}
			// --- END HOOK ---

			// Naval Factory - 0x444167
			if (pType->Naval) {

				if (!this->HasAnyLink())
					this->QueueMission(Mission::Unload, 0);

				CoordStruct centerCoord = this->GetCoords();
				CellStruct targetCell = CellClass::Coord2Cell(centerCoord);

				AbstractClass* archiveTarget = this->ArchiveTarget;

				if (archiveTarget) {
					CoordStruct targetCoord = archiveTarget->GetCoords();
					CellStruct targetCell2 = CellClass::Coord2Cell(targetCoord);
					CellStruct buildingCell = this->GetMapCoords();

					double angle = Math::atan2(
						static_cast<double>(buildingCell.Y - targetCell2.Y),
						static_cast<double>(targetCell2.X - buildingCell.X)
					);
					angle -= Math::DEG90_AS_RAD;
					int rawDir = static_cast<int>(angle * Math::BINARY_ANGLE_MAGIC);
					int dirIndex = (((rawDir >> 12) + 1) >> 1) & 7;

					CellClass* pCellClass = MapClass::Instance->GetCellAt(targetCell2);

					if (pCellClass->GetBuilding() == this) {

						const CellStruct* pAdjacentDir = &CellSpread::AdjacentCell[dirIndex & 7];
						do {
							targetCell += *pAdjacentDir;
							pCellClass = MapClass::Instance->GetCellAt(targetCell);
						}

						while (pCellClass->GetBuilding() == this);
					}
				}

				bool needNearbyLocation = true;

				if (archiveTarget) {
					CellClass* pTargetCell = MapClass::Instance->GetCellAt(targetCell);

					if (pTargetCell->LandType == LandType::Water) {
						if (!pTargetCell->FindTechnoNearestTo(Point2D::Empty, false, nullptr)) {
							if (MapClass::Instance->IsWithinUsableArea(targetCell, true))
								needNearbyLocation = false;
						}
					}
				}

				if (needNearbyLocation) {
					CoordStruct coord = this->GetCoords();
					CellStruct searchCell = CellClass::Coord2Cell(coord);
					TechnoTypeClass* pTechnoType = pObject->GetTechnoType();
					 
					MapClass::Instance->NearByLocation(targetCell,
						searchCell, pTechnoType->SpeedType, ZoneType::None, MovementZone::Normal,
						false, 1, 1, false, false, false, true, CellStruct::Empty, false, false
					);
				}

				CellClass* pTargetCellClass = MapClass::Instance->GetCellAt(targetCell);
				CoordStruct cellCenterCoord = pTargetCellClass->GetCoords();

				if (pObject->Unlimbo(cellCenterCoord, DirType::North)) {
					if (archiveTarget) {
						pObject->SetDestination(archiveTarget, true);
						pObject->QueueMission(Mission::Move, 0);
					}

					pObject->Mark(MarkType::Up);
					CoordStruct finalCoord = MapClass::Instance->GetCellAt(targetCell)->Cell2Coord();
					pObject->SetLocation(finalCoord);
					pObject->Mark(MarkType::Down);

					// --- HOOK 0x44441A (0x6) ---
					TechnoExt_ExtData::KickOutClones(this, pObject);
					// --- END HOOK ---

					return KickOutResult::Succeeded;
				}

				return KickOutResult::Failed;
			}
			// Land Vehicle Factory - 0x444492
			pObject->SetArchiveTarget(this->ArchiveTarget);

			// --- HOOK 0x4444A0 (0xA) ---
			Mission currentMission = this->GetCurrentMission();
			if (currentMission == Mission::Unload || currentMission == Mission::Construction)
				// --- END HOOK ---
			{
				// --- HOOK 0x4444B3 (0x6) ---
				if (this->Type->Factory == AbstractType::None
					|| BuildingTypeExtContainer::Instance.Find(this->Type)->CloningFacility.Get())
				{
					return KickOutResult::Busy;
				}
				// --- END HOOK ---

				int buildingCount = this->Owner->Buildings.Count;

				for (int i = 0; i < buildingCount; ++i) {
					BuildingClass* pOtherBuilding = this->Owner->Buildings[i];

					// --- HOOK 0x4444E2 (0x6) ---
					if (this != pOtherBuilding
						&& pOtherBuilding->GetCurrentMission() == Mission::Guard
						&& pOtherBuilding->Type->Factory == this->Type->Factory
						&& pOtherBuilding->Type->Naval == this->Type->Naval
						&& TechnoTypeExtData::CanBeBuiltAt(pObject->GetTechnoType(), pOtherBuilding->Type)
						&& !pOtherBuilding->Factory)
						// --- END HOOK ---
					{
						FactoryClass* pFactory = this->Factory;
						pOtherBuilding->Factory = pFactory;
						this->Factory = nullptr;

						KickOutResult result = pOtherBuilding->KickOutUnit(pObject, CellStruct::Empty);

						pOtherBuilding->Factory = nullptr;
						this->Factory = pFactory;
						return result;
					}
				}

				return KickOutResult::Busy;
			}

			++Unsorted::ScenarioInit;

			CoordStruct exitCoord;
			this->GetExitCoords(&exitCoord, 0);

			if (pObject->Unlimbo(exitCoord, DirType::East)) {
				pObject->Mark(MarkType::Up);
				pObject->SetLocation(exitCoord);
				pObject->Mark(MarkType::Down);
				this->SendCommand(RadioCommand::RequestLink, pObject);
				this->SendCommand(RadioCommand::RequestTether, pObject);
				this->QueueMission(Mission::Unload, 0);

				// --- HOOK 0x4445F6 (0x5) ---
				--Unsorted::ScenarioInit;
				TechnoExt_ExtData::KickOutClones(this, pObject);
				++Unsorted::ScenarioInit;
				// --- END HOOK ---

				--Unsorted::ScenarioInit;
				return KickOutResult::Succeeded;
			}

			--Unsorted::ScenarioInit;
			return KickOutResult::Failed;
		}

		// Barracks / Infantry Production - 0x4445FB
	BarracksExit:
		if (pType->Factory == AbstractType::InfantryType
			|| pType->Hospital
			|| pType->Armory
			|| pType->Cloning)
		{
			pObject->SetArchiveTarget(this->ArchiveTarget);

			CellStruct exitCell3 = this->FindExitCell(pObject, CellStruct::Empty);

			if (exitCell3 == CellStruct::Empty)
				return KickOutResult::Failed;

			// --- HOOK 0x4449DF: LJMP to 0x444A53 ---
			// Original cloning vat logic SKIPPED entirely
			// Cloning now handled by KickOutClones below
			// --- END HOOK ---

			int destY = (exitCell3.Y << 8) + 128;
			int destX = (exitCell3.X << 8) + 128;

			CoordStruct centerCoord = this->GetCoords();

			double angle = Math::atan2(
				static_cast<double>(centerCoord.Y - destY),
				static_cast<double>(destX - centerCoord.X)
			);
			angle -= Math::DEG90_AS_RAD;
			int rawFacing = static_cast<int>(angle * Math::BINARY_ANGLE_MAGIC);
			int facing = (((rawFacing >> 7) + 1) >> 1) & 0xFF;

			CellStruct buildingCell = this->GetMapCoords();

			int buildingWidth = this->Type->GetFoundationWidth();
			int buildingHeight = this->Type->GetFoundationHeight(false);

			CellStruct intermediateCell = exitCell3;

			if (exitCell3.X >= buildingCell.X + buildingWidth)
				intermediateCell.X = exitCell3.X - 1;
			else if (exitCell3.X < buildingCell.X)
				intermediateCell.X = exitCell3.X + 1;

			if (exitCell3.Y >= buildingCell.Y + buildingHeight)
				intermediateCell.Y = exitCell3.Y - 1;
			else if (exitCell3.Y < buildingCell.Y)
				intermediateCell.Y = exitCell3.Y + 1;

			CoordStruct unlimboCoord = CellClass::Cell2Coord(intermediateCell);

			// --- HOOK 0x444B83 (0x7) ---
			BuildingTypeClass* pBuildingType = this->Type;
			auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

			if (pBuildingTypeExt->BarracksExitCell.isset()) {
				auto exitCoords = pBuildingType->ExitCoord;
				unlimboCoord.X = unlimboCoord.X + exitCoords.X;
				unlimboCoord.Y = unlimboCoord.Y + exitCoords.Y;
				unlimboCoord.Z = exitCoords.Z;
			} else
				// --- END HOOK ---
			{
				if (pBuildingType->GDIBarracks) {
					if (exitCell3.X == buildingCell.X + 1 && exitCell3.Y == buildingCell.Y + 2) {
						unlimboCoord.X += pBuildingType->ExitCoord.X;
						unlimboCoord.Y += pBuildingType->ExitCoord.Y;
						unlimboCoord.Z = pBuildingType->ExitCoord.Z;
					}
				}

				if (pBuildingType->NODBarracks) {
					if (exitCell3.X == buildingCell.X + 2 && exitCell3.Y == buildingCell.Y + 2) {
						unlimboCoord.X += pBuildingType->ExitCoord.X;
						unlimboCoord.Y += pBuildingType->ExitCoord.Y;
						unlimboCoord.Z += pBuildingType->ExitCoord.Z;
					}
				}

				if (pBuildingType->YuriBarracks) {
					if (exitCell3.X == buildingCell.X + 2 && exitCell3.Y == buildingCell.Y + 1) {
						unlimboCoord.X += pBuildingType->ExitCoord.X;
						unlimboCoord.Y += pBuildingType->ExitCoord.Y;
						unlimboCoord.Z += pBuildingType->ExitCoord.Z;
					}
				}
			}

			++Unsorted::ScenarioInit;

			if (pObject->Unlimbo(unlimboCoord, static_cast<DirType>(facing))) {
				if (auto pNavcom = ((FootClass*)pObject)->Destination) {
					TechnoTypeClass* pTechnoType = pObject->GetTechnoType();
					if (!pTechnoType->JumpJet && !pTechnoType->Teleporter) {
						if (pNavcom)
							pObject->SetArchiveTarget(pNavcom);

						pObject->QueueMission(Mission::Move, 0);
						pObject->SetDestination(MapClass::Instance->GetCellAt(exitCell3), true);
					}
				}

				// --- HOOK 0x444D26 (0x6) ---
				bool skipArchiveReset = this->Type->Hospital || this->Type->Armory;
				// --- END HOOK ---

				if (!this->Owner->IsControlledByHuman() || skipArchiveReset) {
					pObject->QueueMission(Mission::Area_Guard, 0);

					CellStruct whereToGo;
					this->Owner->WhereToGo(&whereToGo, pObject);

					if (whereToGo == CellStruct::Empty || this->Type->Factory == AbstractType::None)
						pObject->SetArchiveTarget(nullptr);
					else {
						CellClass* pDestCell = MapClass::Instance->GetCellAt(whereToGo);
						pObject->SetArchiveTarget(pDestCell);
						static_cast<FootClass*>(pObject)->QueueNavList(pDestCell);
					}
				}

				// --- HOOK 0x444DBC (0x5) ---
				--Unsorted::ScenarioInit;
				TechnoExt_ExtData::KickOutClones(this, pObject);
				++Unsorted::ScenarioInit;
				// --- END HOOK ---

				// --- HOOK 0x444DC9 (0x9) ---
				RadioCommand respond = this->SendCommand(RadioCommand::RequestLink, pObject);
				if (respond == RadioCommand::AnswerPositive) {
					this->SendCommand(RadioCommand::RequestUnload, pObject);

					if (auto pDest = pObject->ArchiveTarget)
						pObject->SetDestination(pDest, true);
					else
						pObject->Scatter(CoordStruct::Empty, true, false);
				}
				// --- END HOOK ---

				--Unsorted::ScenarioInit;
				return KickOutResult::Succeeded;
			}

			--Unsorted::ScenarioInit;
			return KickOutResult::Failed;
		}
		// Generic Unit Exit - 0x444638
		{
			CellStruct exitCell2 = this->FindExitCell(pObject, exitCell);

			if (exitCell2 == CellStruct::Empty)
				return KickOutResult::Failed;

			int destY = (exitCell2.Y << 8) + 128;
			int destX = (exitCell2.X << 8) + 128;

			CoordStruct centerCoord = this->GetCoords();
	
			double angle = Math::atan2(
				static_cast<double>(centerCoord.Y - destY),
				static_cast<double>(destX - centerCoord.X)
			);
			angle -= Math::DEG90_AS_RAD;
			int rawFacing = static_cast<int>(angle * Math::BINARY_ANGLE_MAGIC);
			int facing = (((rawFacing >> 7) + 1) >> 1) & 0xFF;

			CellStruct buildingCell = this->GetMapCoords();

			int buildingWidth = this->Type->GetFoundationWidth();
			int buildingHeight = this->Type->GetFoundationHeight(false);

			CellStruct intermediateCell = exitCell2;

			if (exitCell2.X >= buildingCell.X + buildingWidth)
				intermediateCell.X = exitCell2.X - 1;
			else if (exitCell2.X < buildingCell.X)
				intermediateCell.X = exitCell2.X + 1;

			if (exitCell2.Y >= buildingCell.Y + buildingHeight)
				intermediateCell.Y = exitCell2.Y - 1;
			else if (exitCell2.Y < buildingCell.Y)
				intermediateCell.Y = exitCell2.Y + 1;

			CoordStruct unlimboCoord = CellClass::Cell2Coord(intermediateCell);
			BuildingTypeClass* pBuildingType = this->Type;

			if (pBuildingType->GDIBarracks) {
				if (exitCell2.X == buildingCell.X + 1 && exitCell2.Y == buildingCell.Y + 2) {
					unlimboCoord.X += pBuildingType->ExitCoord.X;
					unlimboCoord.Y += pBuildingType->ExitCoord.Y;
					unlimboCoord.Z = pBuildingType->ExitCoord.Z;
				}
			}

			if (pBuildingType->NODBarracks) {
				if (exitCell2.X == buildingCell.X + 2 && exitCell2.Y == buildingCell.Y + 2) {
					unlimboCoord.X += pBuildingType->ExitCoord.X;
					unlimboCoord.Y += pBuildingType->ExitCoord.Y;
					unlimboCoord.Z += pBuildingType->ExitCoord.Z;
				}
			}

			if (pBuildingType->YuriBarracks) {
				if (exitCell2.X == buildingCell.X + 2 && exitCell2.Y == buildingCell.Y + 1) {
					unlimboCoord.X += pBuildingType->ExitCoord.X;
					unlimboCoord.Y += pBuildingType->ExitCoord.Y;
					unlimboCoord.Z += pBuildingType->ExitCoord.Z;
				}
			}

			++Unsorted::ScenarioInit;

			if (pObject->Unlimbo(unlimboCoord, static_cast<DirType>(facing))) {
				pObject->QueueMission(Mission::Move, 0);
				pObject->SetDestination(MapClass::Instance->GetCellAt(exitCell2), true);

				if (!this->Owner->IsControlledByHuman()) {
					pObject->QueueMission(Mission::Area_Guard, 0);

					CellStruct whereToGo;
					this->Owner->WhereToGo(&whereToGo, pObject);

					if (whereToGo == CellStruct::Empty || this->Type->Factory == AbstractType::None)
						pObject->SetArchiveTarget(nullptr);
					else {
						pObject->SetArchiveTarget(MapClass::Instance->GetCellAt(whereToGo));
					}
				}

				--Unsorted::ScenarioInit;
				return KickOutResult::Succeeded;
			}

			--Unsorted::ScenarioInit;
			return KickOutResult::Failed;
		}
	}
	case AbstractType::Building: // 0x444F19
	{
		if (this->Owner->IsControlledByHuman())
			return KickOutResult::Failed;

		this->Owner->WhimpOnMoney(AbstractType::Building);

		if (pHouseExt->Factory_BuildingType == this)
			pHouseExt->Factory_BuildingType = nullptr;

		this->Owner->ProducingBuildingTypeIndex = -1;

		BuildingClass* pBuilding = static_cast<BuildingClass*>(pObject);
		BuildingTypeClass* pBuildingType = pBuilding->Type;

		BaseNodeClass* pNode = this->Owner->Base.NextBuildable(pBuildingType->ArrayIndex);

		CellStruct placeCell = CellStruct::Empty;

		if (pNode && pNode->MapCoords.IsValid()) {
			CellStruct* pNodeCell = &pNode->MapCoords;

			if (pBuildingType->PowersUpBuilding[0] || this->Owner->HasSpaceFor(pBuildingType, pNodeCell)) {
				placeCell = *pNodeCell;
			} else {
				this->Owner->FindBuildLocation(&placeCell, pBuildingType, HouseClass::Func_placement_callback.asT(), -1);
				
				if (!placeCell.IsValid()) {
					return KickOutResult::Failed;
				}

				pNode->MapCoords = placeCell;
			}
		} else {
			if (pBuildingType->PowersUpBuilding[0]) {
				this->Owner->GetPoweups(&placeCell, pBuildingType);
			} else {
				this->Owner->FindBuildLocation(&placeCell, pBuildingType, HouseClass::Func_placement_callback.asT(), -1);
			}

			if (pNode && placeCell.IsValid()) {
				pNode->MapCoords = placeCell;
			}
		}

		if (!placeCell.IsValid()) {
			if (pBuildingType->PowersUpBuilding[0]) {
				if (this->Owner->Base.NextBuildable(-1) == pNode) {
					this->Owner->Base.BaseNodes.erase_at(this->Owner->Base.NextBuildableIdx(-1));
				}
			}
			return KickOutResult::Failed;
		}

		CoordStruct placeCoord = CellClass::Cell2Coord(placeCell);

		// --- HOOK 0x4451F8 (0x6) - MAJOR ---
		auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

		if (RulesExtData::Instance()->AIForbidConYard && pBuildingType->ConstructionYard) {
			if (pNode) {
				pNode->Placed = true;
				pNode->Attempts = 0;
			}
			return KickOutResult::Failed;
		}

		CellStruct flushCell = placeCell;
		flushResult = pBuilding->Type->FlushPlacement(&flushCell, this->Owner);

		if (RulesExtData::Instance()->AICleanWallNode && pBuildingType->Wall) {
			CellClass* pCell = MapClass::Instance->GetCellAt(placeCell);
			bool hasProtectedNeighbor = false;

			for (int i = 0; i < 8; ++i) {
				BuildingClass* pAdjBuilding = pCell->GetNeighbourCell(static_cast<FacingType>(i))->GetBuilding();
				if (pAdjBuilding && pAdjBuilding->Type->ProtectWithWall) {
					hasProtectedNeighbor = true;
					break;
				}
			}

			if (!hasProtectedNeighbor)
				goto PlacementFailed;
		}

		if (pTypeExt->LimboBuild){
			BuildingTypeExtData::CreateLimboBuilding(pBuilding, pBuildingType, this->Owner, pTypeExt->LimboBuildID);

			if (pNode) {
				pNode->Placed = true;
				pNode->Attempts = 0;

				if (this->Owner->ProducingBuildingTypeIndex == pBuildingType->ArrayIndex)
					this->Owner->ProducingBuildingTypeIndex = -1;
			}

			BuildingExtData::PlayConstructionYardAnim<true>(this);
			return KickOutResult::Succeeded;
		}

		if(RulesExtData::Instance()->ExtendedBuildingPlacing){
			if (placeCell != CellStruct::Empty && !pBuildingType->PlaceAnywhere) {
				if (!pBuildingType->PowersUpBuilding[0]) {
					bool noOccupy = true;
					bool canBuild = BuildingExtData::CheckBuildingFoundation(pBuildingType, placeCell, this->Owner, noOccupy);

					auto& place = pBuildingType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

					if (canBuild) {
						if (!noOccupy) {
							if (placeCell != place.TopLeft || pBuildingType != place.Type) {
								place.Type = pBuildingType;
								place.DrawType = pBuildingType;
								place.TopLeft = placeCell;
							}

							if (!place.Timer.HasTimeLeft()) {
								place.Timer.Start(40);

								if (!BuildingTypeExtData::CleanUpBuildingSpace(pBuildingType, placeCell, this->Owner))
									goto TemporarilyBlocked;
							} else {
								goto TemporarilyBlocked;
							}
						}

						BuildingExtData::ClearPlacingBuildingData(&place);
					} else {
						BuildingExtData::ClearPlacingBuildingData(&place);
						goto PlacementFailed;
					}
				} else {
					CellClass* pCell = MapClass::Instance->GetCellAt(placeCell);
					BuildingClass* pCellBuilding = pCell->GetBuilding();

					if (!pCellBuilding || !pCellBuilding->CanUpgrade(pBuildingType, this->Owner))
						goto PlacementFailed;
				}
			}

			CoordStruct unlimboCoord = CellClass::Cell2Coord(placeCell);

			if (pBuilding->Unlimbo(unlimboCoord, DirType::North)) {
				BuildingExtData::PlayConstructionYardAnim(this);
				goto PlacementSucceeded;
			}

			goto PlacementFailed;
		}
		// --- END HOOK ---

		if (flushResult == 0) {
			//Debug::Log("Unlimbo attempt: Coord={%d,%d,%d}\n",
			//	placeCoord.X, placeCoord.Y, placeCoord.Z);

			if (pBuilding->Unlimbo(placeCoord, DirType::North)) {
			PlacementSucceeded:
				if (pBuilding->SlaveManager)
					pBuilding->SlaveManager->Deploy2();

				// --- HOOK 0x44531F (0xA) ---
				if (pNode) {
					if (this == pHouseExt->Factory_BuildingType)
						pHouseExt->Factory_BuildingType = nullptr;

					if (pBuilding->Type->ArrayIndex == this->Owner->ProducingBuildingTypeIndex)
						this->Owner->ProducingBuildingTypeIndex = -1;
				}
				// --- END HOOK ---

				if (pBuilding->CurrentMission == Mission::None
					&& pBuilding->QueuedMission == Mission::Construction) {
					pBuilding->NextMission();
				}

				//if (pBuilding->WhatAmI() == AbstractType::Building) 
				{
					// --- HOOK 0x445355 (0x6) ---
					if (pBuilding->Type->FirestormWall) {
						FirewallFunctions::BuildLines(pBuilding, placeCell, this->Owner);
						MapClass::Instance->BuildingToFirestormWall(placeCell, this->Owner, pBuilding->Type);
					}
					// --- END HOOK ---
					else if (pBuilding->Type->ToOverlay && pBuilding->Type->ToOverlay->Wall) {
						MapClass::Instance->BuildingToWall(placeCell, this->Owner, pBuilding->Type);
					}
				}

				// --- HOOK 0x44540D (0x5) ---
				if (RulesExtData::Instance()->WallTowers.Contains(pBuilding->Type)) {
				// --- END HOOK --- 

					int nodeIndex = this->Owner->Base.BaseNodes.index_of(pNode);

					for (int i = nodeIndex + 1; i < this->Owner->Base.BaseNodes.Count; ++i) {
						BaseNodeClass* pDefenseNode = &this->Owner->Base.BaseNodes.Items[i];
						if (pDefenseNode->BuildingTypeIndex >= 0) {
							BuildingTypeClass* pDefenseType = BuildingTypeClass::Array->Items[pDefenseNode->BuildingTypeIndex];
							if (pDefenseType->IsBaseDefense) {
								pDefenseNode->MapCoords = CellClass::Coord2Cell(pBuilding->Location);
								break;
							}
						}
					}
				}

				return KickOutResult::Succeeded;
			}
		}
		else if (flushResult == 1)
		{
		TemporarilyBlocked:
			int attempts = this->Owner->Base.FailedToPlaceNode(pNode);

			if (SessionClass::Instance->GameMode != GameMode::Campaign) {
				if (attempts > RulesClass::Instance->MaximumBuildingPlacementFailures) {
					if (pNode) {
						this->Owner->Base.BaseNodes.erase_at(this->Owner->Base.BaseNodes.index_of(pNode));
					}
				}
			}

			return KickOutResult::Busy;
		}

	PlacementFailed:
		if (!pNode)
			return KickOutResult::Failed;

		int nodeIndex = this->Owner->Base.BaseNodes.index_of(pNode);
		BuildingTypeClass* pNodeType = BuildingTypeClass::Array->Items[pNode->BuildingTypeIndex];

		if (pNodeType->Wall || pNodeType->Gate) {
			this->Owner->Base.BaseNodes.erase_at(nodeIndex);
			return KickOutResult::Failed;
		}

		for (int i = 0; i < this->Owner->Base.BaseNodes.Count; ++i) {
			BaseNodeClass* pOtherNode = &this->Owner->Base.BaseNodes.Items[i];

			if (pOtherNode->MapCoords == placeCell) {
				pOtherNode->MapCoords = CellStruct::Empty;
			}
		}

		return KickOutResult::Failed;
	}
	default:
		return KickOutResult::Failed;
	}
}

DEFINE_FUNCTION_JUMP(LJMP , 0x443C60 ,FakeBuildingClass::__ExitObject)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3FBC,FakeBuildingClass::__ExitObject)