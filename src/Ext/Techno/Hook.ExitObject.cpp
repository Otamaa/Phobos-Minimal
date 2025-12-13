#include <CellStruct.h>
#include <CoordStruct.h>

#include <HouseClass.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>

#include <Ext/Building/Body.h>

#include <Misc/Ares/Hooks/Header.h>

#pragma region hooks
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

#pragma endregion

NOINLINE DirType Calculate_Exit_Direction(BuildingClass* pBuilding, const CoordStruct& targetCoord)
{
	CoordStruct buildingCenter = pBuilding->GetCoords();
	float angle = Math::atan2(double(targetCoord.Y - buildingCenter.Y), double(targetCoord.X - buildingCenter.X));
		  angle = (angle - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC;

	return static_cast<DirType>((((static_cast<int>(angle) >> 7) + 1) >> 1));
	
}

NOINLINE DirType Calculate_Exit_Direction(BuildingClass* pBuilding, const CellStruct& targetCoord)
{
	CellStruct buildingCenter = pBuilding->GetMapCoords();
	float angle = Math::atan2(double(targetCoord.Y - buildingCenter.Y), double(targetCoord.X - buildingCenter.X));
		  angle = (angle - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC;

	return static_cast<DirType>(((((static_cast<int>(angle) >> 12) + 1) >> 1) & 7));
}

NOINLINE CellStruct Adjust_Exit_Cell(BuildingClass* pBuilding, const CellStruct& targetCell)
{
	CellStruct buildingCell = pBuilding->GetMapCoords();
	CellStruct result = targetCell;

	int buildingWidth = pBuilding->Type->GetFoundationWidth();
	int buildingHeight = pBuilding->Type->GetFoundationHeight(false);

	// Adjust X coordinate if needed
	if (targetCell.X < buildingCell.X + buildingWidth) {
		if (targetCell.X < buildingCell.X) {
			result.X = targetCell.X + 1;
		}
	} else {
		result.X = targetCell.X - 1;
	}

	// Adjust Y coordinate if needed
	if (targetCell.Y < buildingCell.Y + buildingHeight) {
		if (targetCell.Y < buildingCell.Y) {
			result.Y = targetCell.Y + 1;
		}
	} else {
		result.Y = targetCell.Y - 1;
	}

	return result;
}

NOINLINE CoordStruct Apply_Barracks_Exit_Offset(BuildingClass* pBuilding,
													   const CoordStruct& baseCoord,
													   const CellStruct& exitCell,
													   const CellStruct& buildingCell)
{
	CoordStruct result = baseCoord;
	BuildingTypeClass* buildingType = pBuilding->Type;

	// GDI Barracks special exit
	if (buildingType->GDIBarracks &&
		exitCell.X == buildingCell.X + 1 &&
		exitCell.Y == buildingCell.Y + 2) {

		result.X += buildingType->ExitCoord.X;
		result.Y += buildingType->ExitCoord.Y;
		result.Z = buildingType->ExitCoord.Z;
	}

	// NOD Barracks special exit
	if (buildingType->NODBarracks &&
		exitCell.X == buildingCell.X + 2 &&
		exitCell.Y == buildingCell.Y + 2) {
		result += buildingType->ExitCoord;
	}

	// Yuri Barracks special exit
	if (buildingType->YuriBarracks &&
		exitCell.X == buildingCell.X + 2 &&
		exitCell.Y == buildingCell.Y + 1) {
		result += buildingType->ExitCoord;
	}

	return result;
}

NOINLINE bool Place_And_Configure_Unit(BuildingClass* pFactory ,
											 FootClass* unit,
											 const CoordStruct& coord,
											 DirType direction,
											 const CellStruct& destinationCell)
{
	++Unsorted::ScenarioInit;

	bool placed = unit->Unlimbo(coord, direction);

	--Unsorted::ScenarioInit;

	if (!placed) {
		return false;
	}

	// Check if unit has a move animation and needs to move to destination
	if (unit->Destination &&
		!unit->GetTechnoType()->JumpJet &&
		!unit->GetTechnoType()->Teleporter) {

		if (unit->Destination) {
			unit->SetArchiveTarget(unit->Destination);
		}

		unit->QueueMission(Mission::Move, 0);
		unit->SetDestination(MapClass::Instance->GetCellAt(destinationCell), true);
	}

	// Handle AI unit behavior
	if (!pFactory->Owner->IsControlledByHuman() || pFactory->Type->Hospital)
	{
		unit->QueueMission(Mission::Area_Guard, 0);

		CellStruct rallyPoint;
		pFactory->Owner->WhereToGo(&rallyPoint, unit);

		if (!rallyPoint.IsValid() || pFactory->Type->Factory == AbstractType::None) {
			unit->SetArchiveTarget(nullptr);
		} else {
			CellClass* rallyCell = MapClass::Instance->GetCellAt(rallyPoint);
			unit->SetArchiveTarget(rallyCell);
			unit->QueueNavList(rallyCell);
		}
	}

	// Establish radio contact
	if (pFactory->SendCommand(RadioCommand::RequestLink, unit) == RadioCommand::AnswerPositive) {
		pFactory->SendCommand(RadioCommand::RequestUnload, unit);
	}

	return true;
}

NOINLINE void Handle_Cloning_Vats(BuildingClass* pFactory, TechnoClass* infantry)
{
	if (pFactory->Type->Factory != AbstractType::Infantry || pFactory->Type->Cloning) {
		return;
	}

	TechnoTypeClass* infantryType = infantry->GetTechnoType();
	HouseClass* house = pFactory->Owner;

	// Duplicate infantry in all cloning vats
	house->CloningVats.for_each([infantryType](BuildingClass* cloningVat) {
		TechnoClass* clone = (TechnoClass*)infantryType->CreateObject(cloningVat->Owner);
		cloningVat->KickOutUnit(clone, CellStruct::Empty);
	});
}

NOINLINE KickOutResult Exit_Aircraft(BuildingClass* pThis, AircraftClass* aircraft)
{
	// Mark aircraft as down (on ground)
	aircraft->MarkDownSetZ(0);

	++Unsorted::ScenarioInit;

	bool placed = false;
	DirType exitDirection = (DirType)RulesClass::Instance->PoseDir;

	// Check if in radio contact or if ion storm is active
	if (pThis->ContainsLink(aircraft) ||
		pThis->Owner->IonSensitivesShouldBeOffline() && !aircraft->Type->AirportBound) {

		if (pThis->Owner->IonSensitivesShouldBeOffline()) {
			// Ion storm active - place at nearby location	
			CellStruct nearbyCell;
			aircraft->NearbyLocation(&nearbyCell,pThis);
			CoordStruct exitCoord = CellClass::Cell2Coord(nearbyCell);
			placed = aircraft->Unlimbo(exitCoord, exitDirection);
		} else {
			// Normal exit - place at docking coordinate		
			CoordStruct dockCoord = pThis->GetDockCoords(aircraft);

			placed = aircraft->Unlimbo(dockCoord, exitDirection);

			if (placed) {
				// Establish radio contact with building
				pThis->SendCommand(RadioCommand::RequestLink, aircraft);
				pThis->SendCommand(RadioCommand::RequestTether, aircraft);

				// Set aircraft position precisely
				CoordStruct finalDockCoord = pThis->GetDockCoords(aircraft);
				aircraft->SetLocation(finalDockCoord);
				aircraft->DockedTo = pThis;

				// Assign destination if not airport-bound
				AbstractClass* target = pThis->ArchiveTarget;

				if (target && !aircraft->Type->AirportBound) {
					aircraft->SetDestination(target, true);
					aircraft->QueueMission(Mission::Move, 0);
				}
			}
		}
	} else {

		// Not in radio contact - must be non-airport-bound aircraft
		if (aircraft->Type->AirportBound) {
			--Unsorted::ScenarioInit;
			return KickOutResult::Failed; // Airport-bound aircraft need radio contact
		}

		// Calculate edge of map exit position
		CoordStruct buildingCenter = pThis->GetCoords();
		CellStruct centerCell = CellClass::Coord2Cell(buildingCenter);

		int mapLocalX = MapClass::MapLocalSize->X;
		int mapLocalY = MapClass::MapLocalSize->Y;
		int mapWidth = MapClass::MapLocalSize->Width;
		int mapHeight = MapClass::MapLocalSize->Height;

		// Determine which map edge to exit from
		CellStruct exitCell;
		int diagonal1 = (mapLocalX + 1) + mapLocalY;
		int diagonal2 = (mapWidth - mapLocalX) + mapLocalY;

		if ((centerCell.X - diagonal1) - (centerCell.Y - diagonal2) <= mapWidth) {
			// Exit from one diagonal edge
			exitCell.X = diagonal1 - 1;
			exitCell.Y = diagonal2;
		} else {
			// Exit from opposite diagonal edge
			exitCell.X = mapWidth + diagonal1 - 1;
			exitCell.Y = diagonal2 - mapWidth;
		}

		// Add random offset along edge
		int randomOffset = ScenarioClass::Instance->Random.RandomFromMax(mapHeight);
		exitCell.X += randomOffset;
		exitCell.Y += randomOffset;

		CoordStruct exitCoord = CellClass::Cell2Coord(exitCell);

		placed = aircraft->Unlimbo(exitCoord, DirType::Min);

		if (placed) {
			if (AbstractClass* target = pThis->ArchiveTarget) {
				aircraft->SetDestination(target, true);
				aircraft->QueueMission(Mission::Move, 0);
			} else {
				// Find nearby location for non-targeted aircraft
				CellStruct nearbyCell;
				aircraft->NearbyLocation(&nearbyCell, pThis);

				if (!nearbyCell.IsValid()) {
					aircraft->SetDestination(nullptr, true);
					aircraft->QueueMission(Mission::Move, 0);
				} else {
					aircraft->SetDestination(MapClass::Instance->GetCellAt(nearbyCell), true);
					aircraft->QueueMission(Mission::Move, 0);
				}
			}
		}
	}

	--Unsorted::ScenarioInit;

	return placed ? KickOutResult::Succeeded : KickOutResult::Failed;
}

NOINLINE KickOutResult Exit_Building(BuildingClass* pThis, BuildingClass* building) {
	// Player cannot use this function
	if (pThis->Owner->IsControlledByHuman()) {
		return KickOutResult::Failed;
	}

	pThis->Owner->WhimpOnMoney(AbstractType::Building);
	pThis->Owner->ProducingBuildingTypeIndex = -1;

	BaseNodeClass* baseNode = pThis->Owner->Base.NextBuildable(building->Type->ArrayIndex);
	CoordStruct placementCoord = CoordStruct::Empty;

	// Determine placement location
	if (baseNode && baseNode->MapCoords.IsValid()) {
		// Has a predefined base location
		CellStruct targetCell = baseNode->MapCoords;

		if (building->Type->PowersUpBuilding[0] ||
			pThis->Owner->HasSpaceFor(building->Type, &targetCell)) {		
			// Use the base node location directly
			placementCoord = CellClass::Cell2Coord(targetCell);
		} else {
			// Find a suitable build location
			CellStruct buildCell;
			pThis->Owner->FindBuildLocation(&buildCell, building->Type, *reinterpret_cast<HouseClass::placement_callback*>(0x505F80), -1);

			placementCoord = CellClass::Cell2Coord(buildCell);

			if (placementCoord == CoordStruct::Empty) {
				return KickOutResult::Failed; // No valid location found
			}

			// Update base node with found location
			baseNode->MapCoords = CellClass::Coord2Cell(placementCoord);
		}
	} else {
		// No base node - find location dynamically
		if (building->Type->PowersUpBuilding[0]) {
			// Find power-up location
			CellStruct powerUpCell;
			pThis->Owner->GetPoweups(&powerUpCell, building->Type);

			if (powerUpCell != CellStruct::Empty) {
				placementCoord = CellClass::Cell2Coord(powerUpCell);
			}
		} else {
			// Find general build location
			CellStruct buildCell;
			pThis->Owner->FindBuildLocation(&buildCell, building->Type, *reinterpret_cast<HouseClass::placement_callback*>(0x505F80), -1);

			placementCoord = CellClass::Cell2Coord(buildCell);
		}

		// Update base node if exists
		if (baseNode && placementCoord != CoordStruct::Empty) {
			baseNode->MapCoords = CellClass::Coord2Cell(placementCoord);
		}
	}

	// Validate placement location
	if (placementCoord == CoordStruct::Empty) {
		// Failed to find location - handle power-up building special case
		if (building->Type->PowersUpBuilding[0]) {
			if (pThis->Owner->Base.NextBuildable(-1) == baseNode) {
				pThis->Owner->Base.BaseNodes.erase_at(pThis->Owner->Base.NextBuildableIdx(-1));
			}
		}

		return KickOutResult::Failed;
	}

	// Try to flush any obstacles at placement site
	CellStruct placementCell = CellClass::Coord2Cell(placementCoord);
	const int flushResult = building->Type->FlushPlacement(&placementCell, pThis->Owner);

	if (flushResult == 1) {
		// Temporary blockage - increment failure counter
		int failureCount = pThis->Owner->Base.FailedToPlaceNode(baseNode);

		if (SessionClass::Instance->GameMode != GameMode::Campaign && failureCount > RulesClass::Instance->MaximumBuildingPlacementFailures) {
			// Too many failures - remove from build queue
			if (baseNode) {
				pThis->Owner->Base.BaseNodes.erase_at(pThis->Owner->Base.BaseNodes.index_of(baseNode));
			}
		}

		return KickOutResult::Busy; // Temporary failure
	}

	if (flushResult == 2) {
		return KickOutResult::Failed; // Permanent blockage
	}

	// Try to place building
	if (!building->Unlimbo(placementCoord, DirType::Min)) {
		// Placement failed - update base node for walls/gates
		if (building->Type->Wall || building->Type->Gate) {
			if (baseNode) {
				pThis->Owner->Base.BaseNodes.erase_at(pThis->Owner->Base.BaseNodes.index_of(baseNode));
			}
		} else {
			// For other buildings, clear the cell in base nodes
			CellStruct failedCell = CellClass::Coord2Cell(placementCoord);

			pThis->Owner->Base.BaseNodes.for_each([&](BaseNodeClass& node) {
				if (node.MapCoords.IsValid() &&
					node.MapCoords == failedCell) {
					node.MapCoords = CellStruct::Empty;
				}
			});
		}

		return KickOutResult::Failed;
	}

	// Successfully placed building

	// Deploy slave miners if applicable
	if (SlaveManagerClass* slaveManager = building->SlaveManager) {
		slaveManager->Deploy2();
	}

	// Clear build structure flag if this was the building being built
	if (baseNode && building->Type->ArrayIndex == pThis->Owner->ProducingBuildingTypeIndex) {
		pThis->Owner->ProducingBuildingTypeIndex = -1;
	}

	// Commence construction if queued
	if (building->CurrentMission == Mission::Move &&
		building->QueuedMission == Mission::Construction) {
		building->NextMission();
	}

	// Handle special wall placement
	{
		if (building->Type->FirestormWall) {
			CellStruct wallCell = CellClass::Coord2Cell(placementCoord);
			MapClass::Instance->BuildingToFirestormWall(wallCell,pThis->Owner, building->Type);
		} else if (building->Type->ToOverlay && building->Type->ToOverlay->Wall) {
			CellStruct wallCell = CellClass::Coord2Cell(placementCoord);
			MapClass::Instance->BuildingToWall(wallCell, pThis->Owner, building->Type);
		}
	}

	// Handle wall tower special logic
	if (building->Type == RulesClass::Instance->WallTower)
	{
		int nodeIndex = pThis->Owner->Base.BaseNodes.index_of(baseNode);
		int nextIndex = nodeIndex + 1;

		// Find next base defense building in the list
		if (nextIndex < pThis->Owner->Base.BaseNodes.Count) {
			for (int i = nextIndex; i < pThis->Owner->Base.BaseNodes.Count; ++i) {
				BaseNodeClass* nextNode = &pThis->Owner->Base.BaseNodes.Items[i];

				if (nextNode->BuildingTypeIndex >= 0 && BuildingTypeClass::Array->Items[nextNode->BuildingTypeIndex]->IsBaseDefense) {
					// Update next defense building's cell to this tower's location
					nextNode->MapCoords = CellClass::Coord2Cell(building->Location);
					break;
				}
			}
		}
	}

	return KickOutResult::Succeeded; // Successfully placed
}

NOINLINE KickOutResult Exit_Naval_Vehicle(BuildingClass* pThis, FootClass* ship)
{
	// Check if building is already in radio contact
	if (!pThis->HasAnyLink()) {
		pThis->QueueMission(Mission::Unload, 0);
	}

	// Get building center
	CoordStruct buildingCenter = pThis->GetCoords();
	CellStruct exitCell = CellClass::Coord2Cell(buildingCenter);

	// If there's an archive target, calculate direction towards it
	if (AbstractClass* archiveTarget = pThis->ArchiveTarget)
	{
		CoordStruct targetCoord = archiveTarget->GetCoords();
		CellStruct targetCell = CellClass::Coord2Cell(targetCoord);

		// Calculate direction angle
		int dirIndex= (int)Calculate_Exit_Direction(pThis, targetCoord);

		// Move exit cell in calculated direction until clear of building
		CellClass* cell = MapClass::Instance->GetCellAt(exitCell);

		while (cell->GetBuilding() == pThis)
		{
			exitCell += CellSpread::AdjacentCell[dirIndex];
			cell = MapClass::Instance->GetCellAt(exitCell);
		}
	}

	// Validate exit cell
	CellClass* exitCellClass = MapClass::Instance->GetCellAt(exitCell);

	// Check if we need to find a better location
	if (!pThis->ArchiveTarget ||
		exitCellClass->LandType != LandType::Water ||
		exitCellClass->FindTechnoNearestTo(Point2D::Empty, 0, 0) ||
		!MapClass::Instance->IsWithinUsableArea(exitCell, true)) {

		// Find nearby valid water location
		CellStruct searchCell = CellClass::Coord2Cell(buildingCenter);
		TechnoTypeClass* shipType = ship->GetTechnoType();

		exitCell = MapClass::Instance->NearByLocation(searchCell,
											  shipType->SpeedType, ZoneType::None, MovementZone::Normal,
											  false, true, true, false, false, false, true,
											  CellStruct::Empty, false, false);
	}

	// Get final exit coordinates
	CellClass* finalCell = MapClass::Instance->GetCellAt(exitCell);
	CoordStruct exitCoord = finalCell->GetCoords();

	// Try to place ship
	if (!ship->Unlimbo(exitCoord, DirType::East)) {
		return KickOutResult::Failed; // Failed to place
	}

	// Configure ship after placement
	if (AbstractClass* target = pThis->ArchiveTarget) {
		ship->SetDestination(target, true);
		ship->QueueMission(Mission::Move, 0);
	}

	// Update ship position
	ship->Mark(MarkType::Up);
	ship->SetLocation(finalCell->Cell2Coord());
	ship->Mark(MarkType::Down);

	return KickOutResult::Succeeded; // Successfully exited
}

NOINLINE KickOutResult Exit_Ground_Vehicle(BuildingClass* pThis, FootClass* unit)
{
	unit->SetArchiveTarget(pThis->ArchiveTarget);

	// Get exit cell from building
	CellStruct exitCell = pThis->FindBuildingExitCell(unit, CellStruct::Empty);

	if (exitCell == CellStruct::Empty) {
		return KickOutResult::Failed; // No valid exit
	}

	// Convert cell to coordinates (centered)
	CoordStruct exitCoord = CellClass::Cell2Coord(exitCell);

	// Calculate exit direction
	DirType exitDirection = Calculate_Exit_Direction(pThis, exitCoord);

	// Adjust exit cell based on building dimensions
	CellStruct adjustedCell = Adjust_Exit_Cell(pThis, exitCell);

	// Convert adjusted cell to coordinates (centered)
	CoordStruct adjustedCoord = CellClass::Cell2Coord(adjustedCell);

	// Apply faction-specific barracks offsets (also used for some vehicle factories)
	CellStruct buildingCell = pThis->GetMapCoords();
	adjustedCoord = Apply_Barracks_Exit_Offset(pThis, adjustedCoord, exitCell, buildingCell);

	++Unsorted::ScenarioInit;

	bool placed = unit->Unlimbo(adjustedCoord, exitDirection);

	--Unsorted::ScenarioInit;

	if (!placed) {
		return KickOutResult::Failed; // Failed to place
	}

	// Assign move mission to exit destination
	unit->QueueMission(Mission::Move, 0);
	CellClass* destCell = MapClass::Instance->GetCellAt(exitCell);
	unit->SetDestination(destCell, true);

	// Handle AI unit behavior
	if (!pThis->Owner->IsControlledByHuman())
	{
		unit->QueueMission(Mission::Area_Guard, 0);
		CellStruct rallyPoint;
		pThis->Owner->WhereToGo(&rallyPoint, unit);
		if (rallyPoint == CellStruct::Empty || pThis->Type->Factory == AbstractType::None) {
			unit->SetArchiveTarget(nullptr);
		} else {
			unit->SetArchiveTarget(MapClass::Instance->GetCellAt(rallyPoint));
		}
	}

	return KickOutResult::Succeeded; // Successfully exited
}

NOINLINE KickOutResult Exit_Harvester(BuildingClass* pThis, FootClass* harvester)
{
	// Get building center and calculate exit position (adjacent cell to the south)
	CoordStruct buildingCenter = pThis->GetCoords();

	CellStruct exitCell = CellClass::Coord2Cell(buildingCenter);
	exitCell += CellSpread::AdjacentCell[5];

	// Calculate final exit position (one cell further south)
	exitCell += CellSpread::AdjacentCell[4];

	CoordStruct exitCoord = CellClass::Cell2Coord(exitCell);

	++Unsorted::ScenarioInit;
	bool placed = harvester->Unlimbo(exitCoord, DirType::SouthWest);
	--Unsorted::ScenarioInit;

	if (!placed) {
		return KickOutResult::Failed; // Failed to place
	}

	// Set harvester facing south
	DirStruct facing(0x8000);
	harvester->PrimaryFacing.Set_Current(facing);
	
	// Assign harvest mission
	harvester->QueueMission(Mission::Harvest, 0);

	return KickOutResult::Succeeded; // Successfully exited
}

NOINLINE KickOutResult Exit_Infantry_Type(BuildingClass* pThis, FootClass* unit)
{
	unit->SetArchiveTarget(pThis->ArchiveTarget);

	// Get exit cell from building
	CellStruct exitCell = pThis->FindBuildingExitCell(unit, CellStruct::Empty);

	if (exitCell == CellStruct::Empty) {
		return KickOutResult::Failed; // No valid exit
	}

	// Handle cloning vats if applicable
	Handle_Cloning_Vats(pThis, unit);

	// Convert cell to coordinates (centered)
	CoordStruct exitCoord = CellClass::Cell2Coord(exitCell);

	// Calculate exit direction
	DirType exitDirection = Calculate_Exit_Direction(pThis, exitCoord);

	// Adjust exit cell based on building dimensions
	CellStruct adjustedCell = Adjust_Exit_Cell(pThis, exitCell);

	// Convert adjusted cell to coordinates (centered)
	CoordStruct adjustedCoord = CellClass::Cell2Coord(adjustedCell);

	// Apply faction-specific barracks offsets
	CellStruct buildingCell = pThis->GetMapCoords();
	adjustedCoord = Apply_Barracks_Exit_Offset(pThis, adjustedCoord, exitCell, buildingCell);

	// Try to place unit at exit
	if (Place_And_Configure_Unit(pThis, unit, adjustedCoord, exitDirection, exitCell)) {
		return KickOutResult::Succeeded; // Successfully exited
	}

	return KickOutResult::Failed; // Failed to place
}

KickOutResult FakeBuildingClass::__ExitObject(TechnoClass* object, int exitFlags)
{
	if (!object) {
		return KickOutResult::Failed;
	}

	// Lock the building during exit operations
	object->IsInPlayfield = true;
	AbstractType objectType = object->WhatAmI();

	// Route to appropriate exit handler based on object type
	switch (objectType)
	{
	case AbstractType::Unit:
	case AbstractType::Infantry:
	{
		BuildingTypeClass* buildingType = this->Type;

		// Check if building can exit units
		if (!buildingType->Hospital &&
			!buildingType->Armory &&
			!buildingType->WeaponsFactory &&
			!this->HasFreeLink()) {
			return KickOutResult::Busy; // Building is full
		}

		// Handle production buildings
		if (!buildingType->Hospital && !buildingType->Armory) {

			// Deduct money for production
			this->Owner->WhimpOnMoney(objectType);
	
			if (objectType == AbstractType::Unit) {
				this->Owner->ProducingUnitTypeIndex = -1;
			}

			if (objectType == AbstractType::Infantry) {
				this->Owner->ProducingInfantryTypeIndex = -1;
			}
		}

		// Check for special building types
		if (buildingType->Refinery || buildingType->Weeder) {
			// Harvester exit
			if (objectType != AbstractType::Unit) {
				object->Scatter(CoordStruct::Empty, true, false);
				return KickOutResult::Failed;
			}

			return Exit_Harvester(this, static_cast<FootClass*>(object));
		}

		if (!buildingType->WeaponsFactory)
		{
			// Infantry/healing facility exit
			if (buildingType->Factory == AbstractType::InfantryType ||
				buildingType->Hospital ||
				buildingType->Armory ||
				buildingType->Cloning)
			{
				return Exit_Infantry_Type(this, static_cast<FootClass*>(object));
			}

			// Non-infantry unit exit (regular ground vehicles)
			return Exit_Ground_Vehicle(this, static_cast<FootClass*>(object));
		}

		// War factory exit
		if (!buildingType->Naval) {
			// Ground vehicle factory
			object->SetArchiveTarget(this->ArchiveTarget);
	
			// Check if unloading and try to delegate to another factory
			if (this->GetMission() == Mission::Unload) {
				HouseClass* house = this->Owner;
				for (int i = 0; i < house->Buildings.Count; ++i) {
					BuildingClass* otherFactory = house->Buildings.Items[i];

					if (otherFactory->Type == this->Type && otherFactory != this) {
						if (otherFactory->GetMission() == Mission::Guard && !otherFactory->Factory) {

							// Transfer factory to other building temporarily
							FactoryClass* tempFactory = this->Factory;
							otherFactory->Factory = tempFactory;
							this->Factory = nullptr;

							KickOutResult result = otherFactory->KickOutUnit(object, CellStruct::Empty);

							// Restore factory ownership
							otherFactory->Factory = nullptr;
							this->Factory = tempFactory;

							return result;
						}
					}
				}
				return KickOutResult::Busy; // No available factory found
			}

			// Standard ground vehicle exit through war factory
			++Unsorted::ScenarioInit;

			
			CoordStruct exitCoord;
			this->GetExitCoords(&exitCoord, 0);

			bool placed = object->Unlimbo(exitCoord, DirType::East);

			if (placed)
			{
				object->Mark(MarkType::Up);
				CoordStruct finalCoord;
				this->GetExitCoords(&finalCoord, 0);
				object->SetLocation(finalCoord);
				object->Mark(MarkType::Down);
				this->SendCommand(RadioCommand::RequestLink, object);
				this->SendCommand(RadioCommand::RequestTether, object);
				this->QueueMission(Mission::Unload, 0);

				--Unsorted::ScenarioInit;
				return KickOutResult::Succeeded;
			}

			--Unsorted::ScenarioInit;
			return KickOutResult::Failed;
		}

		// Naval yard exit
		return Exit_Naval_Vehicle(this, static_cast<FootClass*>(object));
	}
	case AbstractType::Aircraft:
	{
		this->Owner->WhimpOnMoney(AbstractType::Aircraft);
		this->Owner->ProducingAircraftTypeIndex = -1;

		return Exit_Aircraft(this, static_cast<AircraftClass*>(object));
	}
	case AbstractType::Building:
	{
		return Exit_Building(this, static_cast<BuildingClass*>(object));
	}
	default:
		return KickOutResult::Failed;
	}
}