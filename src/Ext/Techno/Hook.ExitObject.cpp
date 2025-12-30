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

// ============================================================================
// HELPER ENUMS AND FORWARD DECLARATIONS
// ============================================================================

enum class BuildingPlacementOutcome
{
	Success,
	Failed,
	TemporarilyBlocked
};

// ============================================================================
// HELPER: Aircraft Exit Logic
// ============================================================================
static KickOutResult HandleAircraftDockedExit(FakeBuildingClass* pBuilding, AircraftClass* pAircraft, DirType facing);
static KickOutResult HandleAircraftIonStormExit(FakeBuildingClass* pBuilding, AircraftClass* pAircraft, DirType facing);
static KickOutResult HandleAircraftMapEdgeSpawn(FakeBuildingClass* pBuilding, AircraftClass* pAircraft);

static KickOutResult HandleAircraftExit(
	FakeBuildingClass* pBuilding,
	AircraftClass* pAircraft,
	HouseExtData* pHouseExt)
{
	pBuilding->Owner->WhimpOnMoney(AbstractType::Aircraft);

	if (pBuilding == pHouseExt->Factory_AircraftType)
		pHouseExt->Factory_AircraftType = nullptr;

	pBuilding->Owner->ProducingAircraftTypeIndex = -1;

	const bool inRadioContact = pBuilding->HasLinkOrFreeSlot(pAircraft);
	const bool ionStormActive = pBuilding->Owner->IonSensitivesShouldBeOffline();
	DirType facing = (DirType)RulesClass::Instance->PoseDir;

	// -----------------------------------------------------------------------
	// Path A: Has radio contact OR (ion storm active AND not airport bound)
	// -----------------------------------------------------------------------
	if (inRadioContact || (ionStormActive && !pAircraft->Type->AirportBound))
	{
		pAircraft->MarkDownSetZ(0);
		++Unsorted::ScenarioInit;

		const KickOutResult result = ionStormActive ? 
			  HandleAircraftIonStormExit(pBuilding, pAircraft, facing) 
			: HandleAircraftDockedExit(pBuilding, pAircraft, facing);

		--Unsorted::ScenarioInit;
		return result;
	}

	// -----------------------------------------------------------------------
	// Path B: No radio contact, spawn from map edge
	// -----------------------------------------------------------------------
	if (pAircraft->Type->AirportBound)
		return KickOutResult::Failed;

	return HandleAircraftMapEdgeSpawn(pBuilding, pAircraft);
}

// ============================================================================
// HELPER: Aircraft Ion Storm Exit
// ============================================================================

static KickOutResult HandleAircraftIonStormExit(
	FakeBuildingClass* pBuilding,
	AircraftClass* pAircraft,
	DirType facing)
{
	CellStruct nearbyCell;
	pAircraft->NearbyLocation(&nearbyCell, pBuilding);

	CoordStruct unlimboCoord = CellClass::Cell2Coord(nearbyCell);

	if (!pAircraft->Unlimbo(unlimboCoord, facing))
		return KickOutResult::Failed;

	pAircraft->DockedTo = pBuilding;
	FacingType poseDir = BuildingExtData::GetPoseDir(pAircraft, pBuilding);
	DirStruct dir { static_cast<int>(poseDir) << 13 };

	if (RulesExtData::Instance()->ExpandAircraftMission)
		pAircraft->PrimaryFacing.Set_Current(dir);

	pAircraft->SecondaryFacing.Set_Current(dir);

	return KickOutResult::Succeeded;
}

// ============================================================================
// HELPER: Aircraft Docked Exit
// ============================================================================

static KickOutResult HandleAircraftDockedExit(
	FakeBuildingClass* pBuilding,
	AircraftClass* pAircraft,
	DirType facing)
{
	CoordStruct dockingCoord;
	pBuilding->GetDockCoords(&dockingCoord, pAircraft);

	if (!pAircraft->Unlimbo(dockingCoord, facing))
		return KickOutResult::Failed;

	pBuilding->SendCommand(RadioCommand::RequestLink, pAircraft);
	pBuilding->SendCommand(RadioCommand::RequestTether, pAircraft);
	pAircraft->SetLocation(pBuilding->GetDockCoords(pAircraft));
	pAircraft->DockedTo = pBuilding;

	FacingType poseDir = BuildingExtData::GetPoseDir(pAircraft, pBuilding);
	DirStruct dir { poseDir };

	if (RulesExtData::Instance()->ExpandAircraftMission)
		pAircraft->PrimaryFacing.Set_Current(dir);

	pAircraft->SecondaryFacing.Set_Current(dir);

	if (pBuilding->ArchiveTarget && !pAircraft->Type->AirportBound)
	{
		pAircraft->SetDestination(pBuilding->ArchiveTarget, true);
		pAircraft->QueueMission(Mission::Move, 0);
	}

	return KickOutResult::Succeeded;
}

// ============================================================================
// HELPER: Aircraft Map Edge Spawn
// ============================================================================

static KickOutResult HandleAircraftMapEdgeSpawn(
	FakeBuildingClass* pBuilding,
	AircraftClass* pAircraft)
{
	int mapLocalY = MapClass::MapLocalSize->Y;
	int mapWidth = MapClass::MapSize->Width;
	int mapLocalX = MapClass::MapLocalSize->X;
	int mapLocalWidth = MapClass::MapLocalSize->Width;
	int mapLocalHeight = MapClass::MapLocalSize->Height;

	CellStruct spawnCell {
		.X = static_cast<short>(mapLocalY + mapLocalX + 1),
		.Y = static_cast<short>(mapWidth - mapLocalX + mapLocalY)
	};

	CoordStruct centerCoord = pBuilding->GetCoords();
	CellStruct buildingCell = CellClass::Coord2Cell(centerCoord);

	if ((buildingCell.X - spawnCell.X) - (buildingCell.Y - spawnCell.Y) > mapLocalWidth)
	{
		spawnCell.X = static_cast<short>(mapLocalWidth + spawnCell.X - 1);
		spawnCell.Y = static_cast<short>(spawnCell.Y - mapLocalWidth);
	}
	else
	{
		spawnCell.X--;
	}

	short randomOffset = (short)ScenarioClass::Instance->Random.RandomFromMax(mapLocalHeight);
	spawnCell.X += randomOffset;
	spawnCell.Y += randomOffset;

	CoordStruct spawnCoord = CellClass::Cell2Coord(spawnCell);

	++Unsorted::ScenarioInit;

	if (!pAircraft->Unlimbo(spawnCoord, DirType::North))
	{
		--Unsorted::ScenarioInit;
		return KickOutResult::Failed;
	}

	AbstractClass* archiveTarget = pBuilding->ArchiveTarget;

	if (archiveTarget)
	{
		pAircraft->SetDestination(archiveTarget, true);
		pAircraft->QueueMission(Mission::Move, 0);
	}
	else
	{
		CellStruct nearbyCell;
		pAircraft->NearbyLocation(&nearbyCell, pBuilding);

		if (nearbyCell == CellStruct::Empty)
			pAircraft->SetDestination(nullptr, true);
		else
			pAircraft->SetDestination(MapClass::Instance->GetCellAt(nearbyCell), true);

		pAircraft->QueueMission(Mission::Move, 0);
	}

	--Unsorted::ScenarioInit;
	return KickOutResult::Succeeded;
}

// ============================================================================
// HELPER: Calculate Facing From Building To Cell
// ============================================================================

static int CalculateFacingToCell(const CoordStruct& fromCoord, const CellStruct& toCell)
{
	CoordStruct dest = CellClass::Cell2Coord(toCell);

	double angle = Math::atan2(
		static_cast<double>(fromCoord.Y - dest.Y),
		static_cast<double>(dest.X - fromCoord.X)
	);
	angle -= Math::DEG90_AS_RAD;

	int rawFacing = static_cast<int>(angle * Math::BINARY_ANGLE_MAGIC);
	return (((rawFacing >> 7) + 1) >> 1) & 0xFF;
}

// ============================================================================
// HELPER: Calculate Intermediate Cell (one step closer to building)
// ============================================================================

static CellStruct CalculateIntermediateCell(
	const CellStruct& exitCell,
	const CellStruct& buildingCell,
	int buildingWidth,
	int buildingHeight)
{
	CellStruct result = exitCell;

	if (exitCell.X >= buildingCell.X + buildingWidth)
		result.X = exitCell.X - 1;
	else if (exitCell.X < buildingCell.X)
		result.X = exitCell.X + 1;

	if (exitCell.Y >= buildingCell.Y + buildingHeight)
		result.Y = exitCell.Y - 1;
	else if (exitCell.Y < buildingCell.Y)
		result.Y = exitCell.Y + 1;

	return result;
}

// ============================================================================
// HELPER: Apply Barracks Exit Coord Offset
// ============================================================================

static void ApplyBarracksExitOffset(
	CoordStruct& unlimboCoord,
	BuildingTypeClass* pBuildingType,
	const CellStruct& exitCell,
	const CellStruct& buildingCell)
{
	auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

	// Custom exit cell override
	if (pBuildingTypeExt->BarracksExitCell.isset())
	{
		auto exitCoords = pBuildingType->ExitCoord;
		unlimboCoord.X += exitCoords.X;
		unlimboCoord.Y += exitCoords.Y;
		unlimboCoord.Z = exitCoords.Z;
		return;
	}

	// GDI Barracks special case
	if (pBuildingType->GDIBarracks)
	{
		if (exitCell.X == buildingCell.X + 1 && exitCell.Y == buildingCell.Y + 2)
		{
			unlimboCoord.X += pBuildingType->ExitCoord.X;
			unlimboCoord.Y += pBuildingType->ExitCoord.Y;
			unlimboCoord.Z = pBuildingType->ExitCoord.Z;
			return;
		}
	}

	// NOD Barracks special case
	if (pBuildingType->NODBarracks)
	{
		if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 2)
		{
			unlimboCoord.X += pBuildingType->ExitCoord.X;
			unlimboCoord.Y += pBuildingType->ExitCoord.Y;
			unlimboCoord.Z += pBuildingType->ExitCoord.Z;
			return;
		}
	}

	// Yuri Barracks special case
	if (pBuildingType->YuriBarracks)
	{
		if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 1)
		{
			unlimboCoord.X += pBuildingType->ExitCoord.X;
			unlimboCoord.Y += pBuildingType->ExitCoord.Y;
			unlimboCoord.Z += pBuildingType->ExitCoord.Z;
		}
	}
}

// ============================================================================
// HELPER: Check Factory Availability (can we defer to another factory?)
// ============================================================================

static bool CanDeferToAnotherFactory(FakeBuildingClass* pBuilding, TechnoClass* pObject)
{
	Mission currentMission = pBuilding->GetCurrentMission();

	if (currentMission != Mission::Unload && currentMission != Mission::Construction)
		return false;

	if (pBuilding->Type->Factory == AbstractType::None)
		return true;

	if (BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->CloningFacility.Get())
		return true;

	return false;
}

// ============================================================================
// HELPER: Try Defer Production To Another Factory
// ============================================================================

static KickOutResult TryDeferToAnotherFactory(FakeBuildingClass* pBuilding, TechnoClass* pObject)
{
	for (int i = 0; i < pBuilding->Owner->Buildings.Count; ++i)
	{
		BuildingClass* pOtherBuilding = pBuilding->Owner->Buildings[i];

		bool canUseOther = (pBuilding != pOtherBuilding)
			&& (pOtherBuilding->GetCurrentMission() == Mission::Guard)
			&& (pOtherBuilding->Type->Factory == pBuilding->Type->Factory)
			&& (pOtherBuilding->Type->Naval == pBuilding->Type->Naval)
			&& TechnoTypeExtData::CanBeBuiltAt(pObject->GetTechnoType(), pOtherBuilding->Type)
			&& !pOtherBuilding->Factory;

		if (!canUseOther)
			continue;

		FactoryClass* pFactory = pBuilding->Factory;
		pOtherBuilding->Factory = pFactory;
		pBuilding->Factory = nullptr;

		KickOutResult result = pOtherBuilding->KickOutUnit(pObject, CellStruct::Empty);

		pOtherBuilding->Factory = nullptr;
		pBuilding->Factory = pFactory;

		return result;
	}

	return KickOutResult::Busy;
}

// ============================================================================
// HELPER: Handle Refinery/Weeder Exit
// ============================================================================

static KickOutResult HandleRefineryExit(FakeBuildingClass* pBuilding, TechnoClass* pObject)
{
	if (pObject->WhatAmI() != AbstractType::Unit)
	{
		pObject->Scatter(CoordStruct::Empty, true, false);
		return KickOutResult::Failed;
	}

	CoordStruct centerCoord = pBuilding->Location;
	CellStruct exitCell4 = CellClass::Coord2Cell(centerCoord) + CellSpread::AdjacentCell[5];

	++Unsorted::ScenarioInit;

	CoordStruct unlimboCoord = CellClass::Cell2Coord(exitCell4 + CellSpread::AdjacentCell[4]);

	if (pObject->Unlimbo(unlimboCoord, DirType::SouthWest))
	{
		DirStruct facing;
		facing.Raw = 0x8000;
		pObject->PrimaryFacing.Set_Current(facing);
		pObject->QueueMission(Mission::Harvest, 0);
	}

	--Unsorted::ScenarioInit;
	return KickOutResult::Failed;
}

// ============================================================================
// HELPER: Handle Naval Factory Exit
// ============================================================================

static KickOutResult HandleNavalFactoryExit(
	FakeBuildingClass* pBuilding,
	TechnoClass* pObject)
{
	if (!pBuilding->HasAnyLink())
		pBuilding->QueueMission(Mission::Unload, 0);

	CoordStruct centerCoord = pBuilding->GetCoords();
	CellStruct targetCell = CellClass::Coord2Cell(centerCoord);

	AbstractClass* archiveTarget = pBuilding->ArchiveTarget;

	// Calculate target cell based on archive target direction
	if (archiveTarget)
	{
		CoordStruct targetCoord = archiveTarget->GetCoords();
		CellStruct targetCell2 = CellClass::Coord2Cell(targetCoord);
		CellStruct buildingCell = pBuilding->GetMapCoords();

		double angle = Math::atan2(
			static_cast<double>(buildingCell.Y - targetCell2.Y),
			static_cast<double>(targetCell2.X - buildingCell.X)
		);
		angle -= Math::DEG90_AS_RAD;
		int rawDir = static_cast<int>(angle * Math::BINARY_ANGLE_MAGIC);
		int dirIndex = (((rawDir >> 12) + 1) >> 1) & 7;

		CellClass* pCellClass = MapClass::Instance->GetCellAt(targetCell2);

		if (pCellClass->GetBuilding() == pBuilding)
		{
			const CellStruct* pAdjacentDir = &CellSpread::AdjacentCell[dirIndex & 7];
			do
			{
				targetCell += *pAdjacentDir;
				pCellClass = MapClass::Instance->GetCellAt(targetCell);
			}
			while (pCellClass->GetBuilding() == pBuilding);
		}
	}

	// Check if we need a nearby location instead
	bool needNearbyLocation = true;

	if (archiveTarget)
	{
		CellClass* pTargetCell = MapClass::Instance->GetCellAt(targetCell);

		bool isWater = (pTargetCell->LandType == LandType::Water);
		bool isEmpty = !pTargetCell->FindTechnoNearestTo(Point2D::Empty, false, nullptr);
		bool isUsable = MapClass::Instance->IsWithinUsableArea(targetCell, true);

		if (isWater && isEmpty && isUsable)
			needNearbyLocation = false;
	}

	if (needNearbyLocation)
	{
		CoordStruct coord = pBuilding->GetCoords();
		CellStruct searchCell = CellClass::Coord2Cell(coord);
		TechnoTypeClass* pTechnoType = pObject->GetTechnoType();

		MapClass::Instance->NearByLocation(
			targetCell,
			searchCell,
			pTechnoType->SpeedType,
			ZoneType::None,
			MovementZone::Normal,
			false, 1, 1, false, false, false, true,
			CellStruct::Empty, false, false
		);
	}

	CellClass* pTargetCellClass = MapClass::Instance->GetCellAt(targetCell);
	CoordStruct cellCenterCoord = pTargetCellClass->GetCoords();

	if (!pObject->Unlimbo(cellCenterCoord, DirType::North))
		return KickOutResult::Failed;

	if (archiveTarget)
	{
		pObject->SetDestination(archiveTarget, true);
		pObject->QueueMission(Mission::Move, 0);
	}

	pObject->Mark(MarkType::Up);
	CoordStruct finalCoord = MapClass::Instance->GetCellAt(targetCell)->Cell2Coord();
	pObject->SetLocation(finalCoord);
	pObject->Mark(MarkType::Down);

	TechnoExt_ExtData::KickOutClones(pBuilding, pObject);

	return KickOutResult::Succeeded;
}

// ============================================================================
// HELPER: Handle Land Vehicle Factory Exit
// ============================================================================

static KickOutResult HandleLandVehicleFactoryExit(
	FakeBuildingClass* pBuilding,
	TechnoClass* pObject)
{
	pObject->SetArchiveTarget(pBuilding->ArchiveTarget);

	// Check if we should defer to another factory
	if (CanDeferToAnotherFactory(pBuilding, pObject))
		return TryDeferToAnotherFactory(pBuilding, pObject);

	++Unsorted::ScenarioInit;

	CoordStruct exitCoord;
	pBuilding->GetExitCoords(&exitCoord, 0);

	if (!pObject->Unlimbo(exitCoord, DirType::East))
	{
		--Unsorted::ScenarioInit;
		return KickOutResult::Failed;
	}

	pObject->Mark(MarkType::Up);
	pObject->SetLocation(exitCoord);
	pObject->Mark(MarkType::Down);

	pBuilding->SendCommand(RadioCommand::RequestLink, pObject);
	pBuilding->SendCommand(RadioCommand::RequestTether, pObject);
	pBuilding->QueueMission(Mission::Unload, 0);

	--Unsorted::ScenarioInit;
	TechnoExt_ExtData::KickOutClones(pBuilding, pObject);
	++Unsorted::ScenarioInit;

	--Unsorted::ScenarioInit;
	return KickOutResult::Succeeded;
}

// ============================================================================
// HELPER: Handle Barracks/Infantry Exit
// ============================================================================

static KickOutResult HandleBarracksExit(
	FakeBuildingClass* pBuilding,
	TechnoClass* pObject)
{
	pObject->SetArchiveTarget(pBuilding->ArchiveTarget);

	CellStruct exitCell = pBuilding->FindExitCell(pObject, CellStruct::Empty);
	if (exitCell == CellStruct::Empty)
		return KickOutResult::Failed;

	CoordStruct centerCoord = pBuilding->GetCoords();
	int facing = CalculateFacingToCell(centerCoord, exitCell);

	CellStruct buildingCell = pBuilding->GetMapCoords();
	int buildingWidth = pBuilding->Type->GetFoundationWidth();
	int buildingHeight = pBuilding->Type->GetFoundationHeight(false);

	CellStruct intermediateCell = CalculateIntermediateCell(
		exitCell, buildingCell, buildingWidth, buildingHeight);

	CoordStruct unlimboCoord = CellClass::Cell2Coord(intermediateCell);

	ApplyBarracksExitOffset(unlimboCoord, pBuilding->Type, exitCell, buildingCell);

	++Unsorted::ScenarioInit;

	if (!pObject->Unlimbo(unlimboCoord, static_cast<DirType>(facing)))
	{
		--Unsorted::ScenarioInit;
		return KickOutResult::Failed;
	}

	// Handle navigation
	if (auto pNavcom = static_cast<FootClass*>(pObject)->Destination)
	{
		TechnoTypeClass* pTechnoType = pObject->GetTechnoType();

		if (!pTechnoType->JumpJet && !pTechnoType->Teleporter)
		{
			pObject->SetArchiveTarget(pNavcom);
			pObject->QueueMission(Mission::Move, 0);
			pObject->SetDestination(MapClass::Instance->GetCellAt(exitCell), true);
		}
	}

	// Handle AI or hospital/armory behavior
	bool skipArchiveReset = pBuilding->Type->Hospital || pBuilding->Type->Armory;

	if (!pBuilding->Owner->IsControlledByHuman() || skipArchiveReset)
	{
		pObject->QueueMission(Mission::Area_Guard, 0);

		CellStruct whereToGo;
		pBuilding->Owner->WhereToGo(&whereToGo, pObject);

		if (whereToGo == CellStruct::Empty || pBuilding->Type->Factory == AbstractType::None)
		{
			pObject->SetArchiveTarget(nullptr);
		}
		else
		{
			CellClass* pDestCell = MapClass::Instance->GetCellAt(whereToGo);
			pObject->SetArchiveTarget(pDestCell);
			static_cast<FootClass*>(pObject)->QueueNavList(pDestCell);
		}
	}

	--Unsorted::ScenarioInit;
	TechnoExt_ExtData::KickOutClones(pBuilding, pObject);
	++Unsorted::ScenarioInit;

	// Final radio commands
	RadioCommand respond = pBuilding->SendCommand(RadioCommand::RequestLink, pObject);

	if (respond == RadioCommand::AnswerPositive)
	{
		pBuilding->SendCommand(RadioCommand::RequestUnload, pObject);

		if (auto pDest = pObject->ArchiveTarget)
			pObject->SetDestination(pDest, true);
		else
			pObject->Scatter(CoordStruct::Empty, true, false);
	}

	--Unsorted::ScenarioInit;
	return KickOutResult::Succeeded;
}

// ============================================================================
// HELPER: Handle Generic Unit Exit
// ============================================================================

static KickOutResult HandleGenericUnitExit(
	FakeBuildingClass* pBuilding,
	TechnoClass* pObject,
	const CellStruct& requestedExitCell)
{
	CellStruct exitCell = pBuilding->FindExitCell(pObject, requestedExitCell);
	if (exitCell == CellStruct::Empty)
		return KickOutResult::Failed;

	CoordStruct centerCoord = pBuilding->GetCoords();
	int facing = CalculateFacingToCell(centerCoord, exitCell);

	CellStruct buildingCell = pBuilding->GetMapCoords();
	int buildingWidth = pBuilding->Type->GetFoundationWidth();
	int buildingHeight = pBuilding->Type->GetFoundationHeight(false);

	CellStruct intermediateCell = CalculateIntermediateCell(
		exitCell, buildingCell, buildingWidth, buildingHeight);

	CoordStruct unlimboCoord = CellClass::Cell2Coord(intermediateCell);
	BuildingTypeClass* pBuildingType = pBuilding->Type;

	// Apply barracks-style exit offsets (same logic, different context)
	if (pBuildingType->GDIBarracks)
	{
		if (exitCell.X == buildingCell.X + 1 && exitCell.Y == buildingCell.Y + 2)
		{
			unlimboCoord.X += pBuildingType->ExitCoord.X;
			unlimboCoord.Y += pBuildingType->ExitCoord.Y;
			unlimboCoord.Z = pBuildingType->ExitCoord.Z;
		}
	}

	if (pBuildingType->NODBarracks)
	{
		if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 2)
		{
			unlimboCoord.X += pBuildingType->ExitCoord.X;
			unlimboCoord.Y += pBuildingType->ExitCoord.Y;
			unlimboCoord.Z += pBuildingType->ExitCoord.Z;
		}
	}

	if (pBuildingType->YuriBarracks)
	{
		if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 1)
		{
			unlimboCoord.X += pBuildingType->ExitCoord.X;
			unlimboCoord.Y += pBuildingType->ExitCoord.Y;
			unlimboCoord.Z += pBuildingType->ExitCoord.Z;
		}
	}

	++Unsorted::ScenarioInit;

	if (!pObject->Unlimbo(unlimboCoord, static_cast<DirType>(facing)))
	{
		--Unsorted::ScenarioInit;
		return KickOutResult::Failed;
	}

	pObject->QueueMission(Mission::Move, 0);
	pObject->SetDestination(MapClass::Instance->GetCellAt(exitCell), true);

	if (!pBuilding->Owner->IsControlledByHuman())
	{
		pObject->QueueMission(Mission::Area_Guard, 0);

		CellStruct whereToGo;
		pBuilding->Owner->WhereToGo(&whereToGo, pObject);

		if (whereToGo == CellStruct::Empty || pBuilding->Type->Factory == AbstractType::None)
			pObject->SetArchiveTarget(nullptr);
		else
			pObject->SetArchiveTarget(MapClass::Instance->GetCellAt(whereToGo));
	}

	--Unsorted::ScenarioInit;
	return KickOutResult::Succeeded;
}

// ============================================================================
// HELPER: Handle Weapons Factory Exit (dispatcher)
// ============================================================================

static KickOutResult HandleWeaponsFactoryExit(
	FakeBuildingClass* pBuilding,
	TechnoClass* pObject,
	AbstractType absType)
{
	BuildingTypeClass* pType = pBuilding->Type;
	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);

	// Check if this is a rubble-destroyed building needing barracks-style exit
	if (pTypeExt->RubbleDestroyed)
	{
		bool isFactoryProduct = (pType->Factory == pObject->GetTechnoType()->WhatAmI())
			&& pBuilding->Factory
			&& pBuilding->Factory->Object == pObject;

		if (!isFactoryProduct && absType == AbstractType::Infantry)
			return HandleBarracksExit(pBuilding, pObject);
	}

	// Naval factory path
	if (pType->Naval)
		return HandleNavalFactoryExit(pBuilding, pObject);

	// Land vehicle factory path
	return HandleLandVehicleFactoryExit(pBuilding, pObject);
}

// ============================================================================
// HELPER: Update Unit/Infantry Production State
// ============================================================================

static void UpdateGroundUnitProductionState(
	FakeBuildingClass* pBuilding,
	TechnoClass* pObject,
	HouseExtData* pHouseExt,
	AbstractType absType)
{
	BuildingTypeClass* pType = pBuilding->Type;

	// Skip production state update for hospital/armory
	if (pType->Hospital || pType->Armory)
		return;

	pBuilding->Owner->WhimpOnMoney(absType);

	if (absType == AbstractType::Unit)
	{
		UnitClass* pUnit = static_cast<UnitClass*>(pObject);

		if (pUnit->Type->Naval)
			pHouseExt->ProducingNavalUnitTypeIndex = -1;
		else
			pBuilding->Owner->ProducingUnitTypeIndex = -1;

		if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == pBuilding)
			pHouseExt->Factory_NavyType = nullptr;
		else if (!pUnit->Type->Naval && pHouseExt->Factory_VehicleType == pBuilding)
			pHouseExt->Factory_VehicleType = nullptr;
	}
	else if (absType == AbstractType::Infantry)
	{
		if (pBuilding == pHouseExt->Factory_InfantryType)
			pHouseExt->Factory_InfantryType = nullptr;

		pBuilding->Owner->ProducingInfantryTypeIndex = -1;
	}
}

// ============================================================================
// HELPER: Check Building Factory Link Availability
// ============================================================================

static bool CanExitFromBuilding(FakeBuildingClass* pBuilding)
{
	BuildingTypeClass* pType = pBuilding->Type;

	if (pType->Hospital || pType->Armory)
		return true;

	if (pType->WeaponsFactory)
	{
		auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);
		if (pTypeExt->CloningFacility)
			return pBuilding->HasFreeLink();
	}

	return pBuilding->HasFreeLink();
}

// ============================================================================
// HELPER: Handle Unit/Infantry Exit (main dispatcher)
// ============================================================================

static KickOutResult HandleGroundUnitExit(
	FakeBuildingClass* pBuilding,
	TechnoClass* pObject,
	HouseExtData* pHouseExt,
	AbstractType absType,
	const CellStruct& exitCell)
{
	BuildingTypeClass* pType = pBuilding->Type;

	// Check if we can even exit
	if (!pType->Hospital && !pType->Armory)
	{
		if (!pType->WeaponsFactory || BuildingTypeExtContainer::Instance.Find(pType)->CloningFacility)
		{
			if (!pBuilding->HasFreeLink())
				return KickOutResult::Busy;
		}
	}

	// Update production state
	UpdateGroundUnitProductionState(pBuilding, pObject, pHouseExt, absType);

	// -----------------------------------------------------------------------
	// Refinery / Weeder
	// -----------------------------------------------------------------------
	if (pType->Refinery || pType->Weeder)
		return HandleRefineryExit(pBuilding, pObject);

	// -----------------------------------------------------------------------
	// Weapons Factory
	// -----------------------------------------------------------------------
	if (pType->WeaponsFactory)
		return HandleWeaponsFactoryExit(pBuilding, pObject, absType);

	// -----------------------------------------------------------------------
	// Barracks / Infantry Production / Hospital / Armory / Cloning
	// -----------------------------------------------------------------------
	bool isBarracksStyle = (pType->Factory == AbstractType::InfantryType)
		|| pType->Hospital
		|| pType->Armory
		|| pType->Cloning;

	if (isBarracksStyle)
		return HandleBarracksExit(pBuilding, pObject);

	// -----------------------------------------------------------------------
	// Generic Unit Exit
	// -----------------------------------------------------------------------
	return HandleGenericUnitExit(pBuilding, pObject, exitCell);
}

// ============================================================================
// HELPER: Handle Building Placement Success
// ============================================================================

static void OnBuildingPlacementSuccess(
	FakeBuildingClass* pFactoryBuilding,
	BuildingClass* pBuilding,
	BaseNodeClass* pNode,
	HouseExtData* pHouseExt,
	const CellStruct& placeCell)
{
	if (pBuilding->SlaveManager)
		pBuilding->SlaveManager->Deploy2();

	if (pNode)
	{
		if (pFactoryBuilding == pHouseExt->Factory_BuildingType)
			pHouseExt->Factory_BuildingType = nullptr;

		if (pBuilding->Type->ArrayIndex == pFactoryBuilding->Owner->ProducingBuildingTypeIndex)
			pFactoryBuilding->Owner->ProducingBuildingTypeIndex = -1;
	}

	if (pBuilding->CurrentMission == Mission::None
		&& pBuilding->QueuedMission == Mission::Construction)
	{
		pBuilding->NextMission();
	}

	// Handle special wall types
	if (pBuilding->Type->FirestormWall)
	{
		FirewallFunctions::BuildLines(pBuilding, placeCell, pFactoryBuilding->Owner);
		MapClass::Instance->BuildingToFirestormWall(placeCell, pFactoryBuilding->Owner, pBuilding->Type);
	}
	else if (pBuilding->Type->ToOverlay && pBuilding->Type->ToOverlay->Wall)
	{
		MapClass::Instance->BuildingToWall(placeCell, pFactoryBuilding->Owner, pBuilding->Type);
	}

	// Handle wall tower placement
	if (RulesExtData::Instance()->WallTowers.Contains(pBuilding->Type))
	{
		int nodeIndex = pFactoryBuilding->Owner->Base.BaseNodes.index_of(pNode);

		for (int i = nodeIndex + 1; i < pFactoryBuilding->Owner->Base.BaseNodes.Count; ++i)
		{
			BaseNodeClass* pDefenseNode = &pFactoryBuilding->Owner->Base.BaseNodes.Items[i];

			if (pDefenseNode->BuildingTypeIndex >= 0)
			{
				BuildingTypeClass* pDefenseType = BuildingTypeClass::Array->Items[pDefenseNode->BuildingTypeIndex];

				if (pDefenseType->IsBaseDefense)
				{
					pDefenseNode->MapCoords = CellClass::Coord2Cell(pBuilding->Location);
					break;
				}
			}
		}
	}
}

// ============================================================================
// HELPER: Handle Building Placement Temporarily Blocked
// ============================================================================

static KickOutResult HandlePlacementTemporarilyBlocked(
	FakeBuildingClass* pBuilding,
	BaseNodeClass* pNode)
{
	int attempts = pBuilding->Owner->Base.FailedToPlaceNode(pNode);

	if (SessionClass::Instance->GameMode != GameMode::Campaign)
	{
		if (attempts > RulesClass::Instance->MaximumBuildingPlacementFailures)
		{
			if (pNode)
			{
				pBuilding->Owner->Base.BaseNodes.erase_at(
					pBuilding->Owner->Base.BaseNodes.index_of(pNode));
			}
		}
	}

	return KickOutResult::Busy;
}

// ============================================================================
// HELPER: Handle Building Placement Failed
// ============================================================================

static KickOutResult HandlePlacementFailed(
	FakeBuildingClass* pBuilding,
	BaseNodeClass* pNode,
	const CellStruct& placeCell)
{
	if (!pNode)
		return KickOutResult::Failed;

	int nodeIndex = pBuilding->Owner->Base.BaseNodes.index_of(pNode);
	BuildingTypeClass* pNodeType = BuildingTypeClass::Array->Items[pNode->BuildingTypeIndex];

	// Remove wall/gate nodes on failure
	if (pNodeType->Wall || pNodeType->Gate)
	{
		pBuilding->Owner->Base.BaseNodes.erase_at(nodeIndex);
		return KickOutResult::Failed;
	}

	// Clear cell from conflicting nodes
	for (int i = 0; i < pBuilding->Owner->Base.BaseNodes.Count; ++i)
	{
		BaseNodeClass* pOtherNode = &pBuilding->Owner->Base.BaseNodes.Items[i];

		if (pOtherNode->MapCoords == placeCell)
			pOtherNode->MapCoords = CellStruct::Empty;
	}

	return KickOutResult::Failed;
}

// ============================================================================
// HELPER: Try Extended Building Placement
// ============================================================================

static BuildingPlacementOutcome TryExtendedBuildingPlacement(
	FakeBuildingClass* pFactoryBuilding,
	BuildingClass* pBuilding,
	BuildingTypeClass* pBuildingType,
	HouseExtData* pHouseExt,
	const CellStruct& placeCell)
{
	if (placeCell == CellStruct::Empty || pBuildingType->PlaceAnywhere)
	{
		CoordStruct unlimboCoord = CellClass::Cell2Coord(placeCell);
		if (pBuilding->Unlimbo(unlimboCoord, DirType::North))
		{
			BuildingExtData::PlayConstructionYardAnim(pFactoryBuilding);
			return BuildingPlacementOutcome::Success;
		}
		return BuildingPlacementOutcome::Failed;
	}

	// Handle power-up buildings
	if (pBuildingType->PowersUpBuilding[0])
	{
		CellClass* pCell = MapClass::Instance->GetCellAt(placeCell);
		BuildingClass* pCellBuilding = pCell->GetBuilding();

		if (!pCellBuilding || !pCellBuilding->CanUpgrade(pBuildingType, pFactoryBuilding->Owner))
			return BuildingPlacementOutcome::Failed;

		CoordStruct unlimboCoord = CellClass::Cell2Coord(placeCell);
		if (pBuilding->Unlimbo(unlimboCoord, DirType::North))
		{
			BuildingExtData::PlayConstructionYardAnim(pFactoryBuilding);
			return BuildingPlacementOutcome::Success;
		}
		return BuildingPlacementOutcome::Failed;
	}

	// Check foundation
	bool noOccupy = true;
	bool canBuild = BuildingExtData::CheckBuildingFoundation(
		pBuildingType, placeCell, pFactoryBuilding->Owner, noOccupy);

	auto& place = (pBuildingType->BuildCat != BuildCat::Combat)
		? pHouseExt->Common
		: pHouseExt->Combat;

	if (!canBuild)
	{
		BuildingExtData::ClearPlacingBuildingData(&place);
		return BuildingPlacementOutcome::Failed;
	}

	if (!noOccupy)
	{
		if (placeCell != place.TopLeft || pBuildingType != place.Type)
		{
			place.Type = pBuildingType;
			place.DrawType = pBuildingType;
			place.TopLeft = placeCell;
		}

		if (!place.Timer.HasTimeLeft())
		{
			place.Timer.Start(40);

			if (!BuildingTypeExtData::CleanUpBuildingSpace(
				pBuildingType, placeCell, pFactoryBuilding->Owner))
			{
				return BuildingPlacementOutcome::TemporarilyBlocked;
			}
		}
		else
		{
			return BuildingPlacementOutcome::TemporarilyBlocked;
		}
	}

	BuildingExtData::ClearPlacingBuildingData(&place);

	CoordStruct unlimboCoord = CellClass::Cell2Coord(placeCell);
	if (pBuilding->Unlimbo(unlimboCoord, DirType::North))
	{
		BuildingExtData::PlayConstructionYardAnim(pFactoryBuilding);
		return BuildingPlacementOutcome::Success;
	}

	return BuildingPlacementOutcome::Failed;
}

// ============================================================================
// HELPER: Find Building Placement Cell
// ============================================================================

static CellStruct FindBuildingPlacementCell(
	FakeBuildingClass* pBuilding,
	BuildingTypeClass* pBuildingType,
	BaseNodeClass* pNode)
{
	CellStruct placeCell = CellStruct::Empty;

	if (pNode && pNode->MapCoords.IsValid())
	{
		CellStruct* pNodeCell = &pNode->MapCoords;

		if (pBuildingType->PowersUpBuilding[0] ||
			pBuilding->Owner->HasSpaceFor(pBuildingType, pNodeCell))
		{
			placeCell = *pNodeCell;
		}
		else
		{
			pBuilding->Owner->FindBuildLocation(
				&placeCell, pBuildingType,
				HouseClass::Func_placement_callback.asT(), -1);

			if (placeCell.IsValid())
				pNode->MapCoords = placeCell;
		}
	}
	else
	{
		if (pBuildingType->PowersUpBuilding[0])
			pBuilding->Owner->GetPoweups(&placeCell, pBuildingType);
		else
			pBuilding->Owner->FindBuildLocation(
				&placeCell, pBuildingType,
				HouseClass::Func_placement_callback.asT(), -1);

		if (pNode && placeCell.IsValid())
			pNode->MapCoords = placeCell;
	}

	return placeCell;
}

// ============================================================================
// HELPER: Handle Building Exit (main entry point)
// ============================================================================

static KickOutResult HandleBuildingExit(
	FakeBuildingClass* pFactoryBuilding,
	BuildingClass* pBuilding,
	HouseExtData* pHouseExt)
{
	// Only AI can place buildings
	if (pFactoryBuilding->Owner->IsControlledByHuman())
		return KickOutResult::Failed;

	pFactoryBuilding->Owner->WhimpOnMoney(AbstractType::Building);

	if (pHouseExt->Factory_BuildingType == pFactoryBuilding)
		pHouseExt->Factory_BuildingType = nullptr;

	pFactoryBuilding->Owner->ProducingBuildingTypeIndex = -1;

	BuildingTypeClass* pBuildingType = pBuilding->Type;
	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingType);

	BaseNodeClass* pNode = pFactoryBuilding->Owner->Base.NextBuildable(pBuildingType->ArrayIndex);

	// -----------------------------------------------------------------------
	// Find placement cell
	// -----------------------------------------------------------------------
	CellStruct placeCell = FindBuildingPlacementCell(pFactoryBuilding, pBuildingType, pNode);

	if (!placeCell.IsValid())
	{
		// Handle power-up building with no valid location
		if (pBuildingType->PowersUpBuilding[0])
		{
			if (pFactoryBuilding->Owner->Base.NextBuildable(-1) == pNode)
			{
				pFactoryBuilding->Owner->Base.BaseNodes.erase_at(
					pFactoryBuilding->Owner->Base.NextBuildableIdx(-1));
			}
		}
		return KickOutResult::Failed;
	}

	// -----------------------------------------------------------------------
	// AI Construction Yard restriction
	// -----------------------------------------------------------------------
	if (RulesExtData::Instance()->AIForbidConYard && pBuildingType->ConstructionYard)
	{
		if (pNode)
		{
			pNode->Placed = true;
			pNode->Attempts = 0;
		}
		return KickOutResult::Failed;
	}

	// -----------------------------------------------------------------------
	// Flush placement check
	// -----------------------------------------------------------------------
	CellStruct flushCell = placeCell;
	int flushResult = pBuilding->Type->FlushPlacement(&flushCell, pFactoryBuilding->Owner);

	// -----------------------------------------------------------------------
	// AI Wall node cleanup
	// -----------------------------------------------------------------------
	if (RulesExtData::Instance()->AICleanWallNode && pBuildingType->Wall)
	{
		CellClass* pCell = MapClass::Instance->GetCellAt(placeCell);
		bool hasProtectedNeighbor = false;

		for (int i = 0; i < 8; ++i)
		{
			BuildingClass* pAdjBuilding = pCell->GetNeighbourCell(static_cast<FacingType>(i))->GetBuilding();
			if (pAdjBuilding && pAdjBuilding->Type->ProtectWithWall)
			{
				hasProtectedNeighbor = true;
				break;
			}
		}

		if (!hasProtectedNeighbor)
			return HandlePlacementFailed(pFactoryBuilding, pNode, placeCell);
	}

	// -----------------------------------------------------------------------
	// Limbo Build
	// -----------------------------------------------------------------------
	if (pTypeExt->LimboBuild)
	{
		BuildingTypeExtData::CreateLimboBuilding(
			pBuilding, pBuildingType, pFactoryBuilding->Owner, pTypeExt->LimboBuildID);

		if (pNode)
		{
			pNode->Placed = true;
			pNode->Attempts = 0;

			if (pFactoryBuilding->Owner->ProducingBuildingTypeIndex == pBuildingType->ArrayIndex)
				pFactoryBuilding->Owner->ProducingBuildingTypeIndex = -1;
		}

		BuildingExtData::PlayConstructionYardAnim<true>(pFactoryBuilding);
		return KickOutResult::Succeeded;
	}

	// -----------------------------------------------------------------------
	// Extended Building Placing
	// -----------------------------------------------------------------------
	if (RulesExtData::Instance()->ExtendedBuildingPlacing)
	{
		BuildingPlacementOutcome outcome = TryExtendedBuildingPlacement(
			pFactoryBuilding, pBuilding, pBuildingType, pHouseExt, placeCell);

		switch (outcome)
		{
		case BuildingPlacementOutcome::Success:
			OnBuildingPlacementSuccess(pFactoryBuilding, pBuilding, pNode, pHouseExt, placeCell);
			return KickOutResult::Succeeded;

		case BuildingPlacementOutcome::TemporarilyBlocked:
			return HandlePlacementTemporarilyBlocked(pFactoryBuilding, pNode);

		case BuildingPlacementOutcome::Failed:
			return HandlePlacementFailed(pFactoryBuilding, pNode, placeCell);
		}
	}

	// -----------------------------------------------------------------------
	// Standard placement
	// -----------------------------------------------------------------------
	if (flushResult == 0)
	{
		CoordStruct placeCoord = CellClass::Cell2Coord(placeCell);

		if (pBuilding->Unlimbo(placeCoord, DirType::North))
		{
			OnBuildingPlacementSuccess(pFactoryBuilding, pBuilding, pNode, pHouseExt, placeCell);
			return KickOutResult::Succeeded;
		}
	}
	else if (flushResult == 1)
	{
		return HandlePlacementTemporarilyBlocked(pFactoryBuilding, pNode);
	}

	return HandlePlacementFailed(pFactoryBuilding, pNode, placeCell);
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

KickOutResult FakeBuildingClass::__ExitObject(TechnoClass* pObject, CellStruct exitCell)
{
	if (!pObject)
		return KickOutResult::Failed;

	pObject->IsInPlayfield = true;

	AbstractType absType = pObject->WhatAmI();
	HouseExtData* pHouseExt = HouseExtContainer::Instance.Find(this->Owner);

	// -----------------------------------------------------------------------
	// Dispatch based on object type
	// -----------------------------------------------------------------------

	switch (absType)
	{
	case AbstractType::Aircraft:
		return HandleAircraftExit(this, static_cast<AircraftClass*>(pObject), pHouseExt);

	case AbstractType::Unit:
	case AbstractType::Infantry:
		return HandleGroundUnitExit(this, pObject, pHouseExt, absType, exitCell);

	case AbstractType::Building:
		return HandleBuildingExit(this, static_cast<BuildingClass*>(pObject), pHouseExt);

	default:
		return KickOutResult::Failed;
	}
}

DEFINE_FUNCTION_JUMP(LJMP , 0x443C60 ,FakeBuildingClass::__ExitObject)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3FBC,FakeBuildingClass::__ExitObject)