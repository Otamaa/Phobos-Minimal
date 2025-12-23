#include <CellStruct.h>
#include <CoordStruct.h>

#include <HouseClass.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>

#include <Ext/Building/Body.h>

#include <Misc/Ares/Hooks/Header.h>

/**
 * BuildingClass::Exit_Object
 *
 * Address: 0x443C60
 *
 * Handles the exit/deployment of objects (units, infantry, aircraft, buildings)
 * from a building (factory, barracks, airport, etc.)
 *
 * @param pObject - The TechnoClass object to exit from this building
 * @param nUnused - Passed to Enter_Transport in some cases
 * @return int - 0 = failure, 1 = delayed/retry needed, 2 = success
 */
int BuildingClass::Exit_Object(TechnoClass* pObject, int nUnused)
{
	// Null check - 0x443C77
	if (!pObject)
	{
		return 0;
	}

	// Lock the object being exited - 0x443C81
	pObject->IsLocked = true;

	// Check if this is an aircraft - 0x443C8A
	AircraftClass* pAircraft = nullptr;
	if (pObject->WhatAmI() == AbstractType::Aircraft)
	{
		pAircraft = static_cast<AircraftClass*>(pObject);
	}

	// Main switch on object type - 0x443C9B
	AbstractType objectType = pObject->WhatAmI();

	switch (objectType)
	{
		//==========================================================================
		// CASE: AIRCRAFT (AbstractType::Aircraft = 2) - 0x443CB4
		//==========================================================================
	case AbstractType::Aircraft:
	{
		// Adjust house money and clear build queue - 0x443CBA
		HouseClass::AdjustThrottle(this->Owner, AbstractType::Aircraft);
		this->Owner->BuildAircraft = -1;

		// Check radio contact or ion storm conditions - 0x443CD4
		bool inRadioContact = RadioClass::In_Radio_Contact(&this->Radio, &pObject->Radio);
		bool ionStormActive = IonStorm::IsActive();

		if (inRadioContact || (ionStormActive && !pAircraft->Type->AirportBound))
		{
			// 0x443F54 - Set aircraft down and increment ScenarioInit
			ObjectClass::MarkDownAndSetZ(pAircraft, 0);
			++ScenarioInit;

			if (ionStormActive)
			{
				// 0x443F78 - Find nearby location during ion storm
				CellStruct nearbyCell;
				TechnoClass::Nearby_Location(pAircraft, &nearbyCell, this);

				CoordStruct unlimboCoord;
				unlimboCoord.X = (nearbyCell.X << 8) + 128;
				unlimboCoord.Y = (nearbyCell.Y << 8) + 128;
				unlimboCoord.Z = 0;

				DirType facing = AircraftClass::Pose_Dir(pAircraft);
				if (pAircraft->Unlimbo(&unlimboCoord, facing))
				{
					--ScenarioInit;
					return 2;
				}
			}
			else
			{
				// 0x443FE5 - Normal airport tethered exit
				DirType facing = AircraftClass::Pose_Dir(pAircraft);
				CoordStruct dockingCoord;
				this->Docking_Coord(&dockingCoord, pAircraft, facing);

				if (pAircraft->Unlimbo(&dockingCoord, facing))
				{
					// 0x444014 - Establish radio contact
					this->Transmit_Message(RadioMessage::Hello, pAircraft);
					this->Transmit_Message(RadioMessage::Tether, pAircraft);

					// 0x44402E - Set final position
					CoordStruct finalCoord;
					this->Docking_Coord(&finalCoord, pAircraft);
					pAircraft->Set_Coord(&finalCoord);

					// 0x44404D - Set dock reference
					pAircraft->DockBuilding = this;

					// 0x444053 - Check for move target
					TechnoClass* archiveTarget = this->ArchiveTarget;
					if (archiveTarget && !pAircraft->Type->AirportBound)
					{
						pAircraft->Assign_Destination(archiveTarget, true);
						pAircraft->Assign_Mission(Mission::Move, 0);
					}

					--ScenarioInit;
					return 2;
				}
			}

			// 0x444EDE - Unlimbo failed
			--ScenarioInit;
			return 0;
		}
		else
		{
			// 0x443D04 - Not in radio contact, spawn at map edge
			if (pAircraft->Type->AirportBound)
			{
				return 0;
			}

			// 0x443D18 - Calculate map edge spawn position
			int mapLocalY = MapClass::MapLocalSize.Y;
			int mapWidth = MapClass::MapSize.Width;
			int mapLocalX = MapClass::MapLocalSize.X;
			int mapLocalWidth = MapClass::MapLocalSize.Width;
			int mapLocalHeight = MapClass::MapLocalSize.Height;

			CellStruct spawnCell;
			CellStruct offsetCell;

			spawnCell.X = static_cast<short>(mapLocalY + mapLocalX + 1);
			spawnCell.Y = static_cast<short>(mapWidth - mapLocalX + mapLocalY);

			// Get building center
			CoordStruct centerCoord;
			this->Center_Coord(&centerCoord);
			int buildingCellX = centerCoord.X / 256;
			int buildingCellY = centerCoord.Y / 256;

			// 0x443DCC - Determine spawn edge
			if ((buildingCellX - spawnCell.X) - (buildingCellY - spawnCell.Y) > mapLocalWidth)
			{
				// 0x443DD9 - Spawn from different edge
				offsetCell.X = static_cast<short>(mapLocalWidth);
				offsetCell.Y = static_cast<short>(-mapLocalWidth);
				spawnCell.X = static_cast<short>(mapLocalWidth + spawnCell.X - 1);
				spawnCell.Y = static_cast<short>(spawnCell.Y - mapLocalWidth);
			}
			else
			{
				// 0x443E2B
				spawnCell.X--;
			}

			// 0x443E50 - Add random offset
			short randomOffset = Scen->RandomNumber(0, mapLocalHeight);
			spawnCell.X += randomOffset;
			spawnCell.Y += randomOffset;

			// 0x443E6E - Build spawn coordinate
			CoordStruct spawnCoord;
			spawnCoord.X = (spawnCell.X << 8) + 128;
			spawnCoord.Y = (spawnCell.Y << 8) + 128;
			spawnCoord.Z = 0;

			++ScenarioInit;

			// 0x443EA0 - Try to unlimbo
			if (pAircraft->Unlimbo(&spawnCoord, DirType::North))
			{
				TechnoClass* archiveTarget = this->ArchiveTarget;

				if (archiveTarget)
				{
					// 0x443EB8 - Has target
					pAircraft->Assign_Destination(archiveTarget, true);
					pAircraft->Assign_Mission(Mission::Move, 0);
				}
				else
				{
					// 0x443ED8 - Find nearby location
					CellStruct nearbyCell;
					TechnoClass::Nearby_Location(pAircraft, &nearbyCell, this);

					if (nearbyCell == CellStruct::Empty)
					{
						// 0x443F34
						pAircraft->Assign_Destination(nullptr, true);
						pAircraft->Assign_Mission(Mission::Move, 0);
					}
					else
					{
						// 0x443F05
						CellClass* pCell = Map[nearbyCell];
						pAircraft->Assign_Destination(pCell, true);
						pAircraft->Assign_Mission(Mission::Move, 0);
					}
				}

				--ScenarioInit;
				return 2;
			}

			--ScenarioInit;
			return 0;
		}
	}

	//==========================================================================
	// CASE: UNIT / INFANTRY (AbstractType::Unit = 1, AbstractType::Infantry = 15)
	// 0x444096
	//==========================================================================
	case AbstractType::Unit:
	case AbstractType::Infantry:
	{
		BuildingTypeClass* pType = this->Type;

		// 0x4440A2 - Check if building can handle this unit type
		if (!pType->Hospital && !pType->Armory && !pType->WeaponsFactory)
		{
			if (!RadioClass::Has_Free_Slots(&this->Radio))
			{
				return 1;
			}
		}

		// 0x4440D7 - Handle money adjustment
		pType = this->Type;
		if (!pType->Hospital && !pType->Armory)
		{
			// 0x4440F4
			HouseClass::AdjustThrottle(this->Owner, pObject->WhatAmI());

			if (pObject->WhatAmI() == AbstractType::Unit)
			{
				this->Owner->BuildUnit = -1;
			}
			if (pObject->WhatAmI() == AbstractType::Infantry)
			{
				this->Owner->BuildInfantry = -1;
			}
		}

		// 0x444137
		pType = this->Type;

		//----------------------------------------------------------------------
		// Refinery/Weeder harvester exit - 0x444145
		//----------------------------------------------------------------------
		if (pType->Refinery || pType->Weeder)
		{
			// 0x444DE4 - Handle refinery exit
			if (pObject->WhatAmI() != AbstractType::Unit)
			{
				// 0x444EF8 - Not a unit, scatter
				pObject->Scatter(&CoordStruct::Empty, true, false);
				return 0;
			}

			// 0x444DF4 - Unit (harvester) exit from refinery
			CoordStruct centerCoord;
			this->Center_Coord(&centerCoord);

			CellStruct exitCell;
			exitCell.X = (centerCoord.X / 256) + AdjacentCell[5].X;
			exitCell.Y = (centerCoord.Y / 256) + AdjacentCell[5].Y;

			++ScenarioInit;

			CoordStruct unlimboCoord;
			unlimboCoord.X = ((exitCell.X + AdjacentCell[4].X) << 8) + 128;
			unlimboCoord.Y = ((exitCell.Y + AdjacentCell[4].Y) << 8) + 128;
			unlimboCoord.Z = 0;

			if (pObject->Unlimbo(&unlimboCoord, DirType::SouthWest))
			{
				DirStruct facing;
				facing.Raw = 0x8000;
				FacingClass::Set(&pObject->PrimaryFacing, &facing);
				pObject->Assign_Mission(Mission::Harvest, 0);
			}

			--ScenarioInit;
			return 0;
		}

		//----------------------------------------------------------------------
		// Weapons Factory vehicle exit - 0x444159
		//----------------------------------------------------------------------
		if (pType->WeaponsFactory)
		{
			//------------------------------------------------------------------
			// Naval factory - 0x444167
			//------------------------------------------------------------------
			if (pType->Naval)
			{
				// 0x44418D - Naval exit logic
				if (!RadioClass::Is_In_Radio_Contact(&this->Radio))
				{
					this->Assign_Mission(Mission::Unload, 0);
				}

				// Get building center
				CoordStruct centerCoord;
				this->Center_Coord(&centerCoord);

				CellStruct targetCell;
				targetCell.X = centerCoord.X / 256;
				targetCell.Y = centerCoord.Y / 256;

				// 0x4441CC - Check archive target
				TechnoClass* archiveTarget = this->ArchiveTarget;
				if (archiveTarget)
				{
					// 0x4441D8 - Calculate direction to target
					CoordStruct targetCoord;
					archiveTarget->Center_Coord(&targetCoord);

					int targetCellX = targetCoord.X / 256;
					int targetCellY = targetCoord.Y / 256;

					CellStruct buildingCell;
					this->Coord_Cell(&buildingCell);

					// Calculate direction index
					double angle = FastMath::Atan2(
						static_cast<double>(buildingCell.Y - targetCellY),
						static_cast<double>(targetCellX - buildingCell.X)
					);
					angle -= DEG90_AS_RAD;
					int rawDir = static_cast<int>(angle * BINARY_ANGLE_MAGIC);
					int dirIndex = (((rawDir >> 12) + 1) >> 1) & 7;

					// 0x44428D - Walk out of building bounds
					CellClass* pCellClass = Map[targetCell];
					if (pCellClass->Cell_Building() == this)
					{
						const CellStruct* pAdjacentDir = &AdjacentCell[dirIndex & 7];
						do
						{
							targetCell.X += pAdjacentDir->X;
							targetCell.Y += pAdjacentDir->Y;
							pCellClass = Map[targetCell];
						}
						while (pCellClass->Cell_Building() == this);
					}
				}

				// 0x4442E5 - Validate target cell
				bool needNearbyLocation = true;
				if (archiveTarget)
				{
					CellClass* pTargetCell = Map[targetCell];
					if (pTargetCell->Land == LandType::Water)
					{
						Point2D checkPoint = { 0, 0 };
						if (!pTargetCell->Cell_Techno(&checkPoint, false, nullptr))
						{
							if (Map.In_Radar(&targetCell, true))
							{
								needNearbyLocation = false;
							}
						}
					}
				}

				// 0x44434B - Find nearby location if needed
				if (needNearbyLocation)
				{
					CoordStruct coord;
					this->Center_Coord(&coord);

					CellStruct searchCell;
					searchCell.X = coord.X / 256;
					searchCell.Y = coord.Y / 256;

					TechnoTypeClass* pTechnoType = pObject->Techno_Type_Class();
					int unusedResult = 0;

					targetCell = *Map.Nearby_Location(
						&searchCell,
						pTechnoType->SpeedType,
						-1,
						MovementZone::Normal,
						false,
						true,
						true,
						false,
						false,
						false,
						true,
						&unusedResult,
						false,
						false
					);
				}

				// 0x4443E7 - Get cell center and unlimbo
				CellClass* pTargetCellClass = Map[targetCell];
				CoordStruct cellCenterCoord;
				pTargetCellClass->Center_Coord(&cellCenterCoord);

				if (pObject->Unlimbo(&cellCenterCoord, DirType::North))
				{
					if (archiveTarget)
					{
						pObject->Assign_Destination(archiveTarget, true);
						pObject->Assign_Mission(Mission::Move, 0);
					}

					// 0x44443F - Update position
					pObject->Mark(MarkType::Up);
					CoordStruct finalCoord;
					Map[targetCell]->Cell_Coord(&finalCoord);
					pObject->Set_Coord(&finalCoord);
					pObject->Mark(MarkType::Down);

					return 2;
				}

				return 0;
			}

			//------------------------------------------------------------------
			// Land vehicle factory - 0x444492
			//------------------------------------------------------------------
			TechnoClass::Set_Archive(pObject, this->ArchiveTarget);

			// 0x4444A4 - Check if building is unloading (already processing)
			if (this->Get_Mission() == Mission::Unload)
			{
				// 0x4444B3 - Try to find alternate factory
				int buildingCount = this->Owner->Buildings.Count;

				for (int i = 0; i < buildingCount; ++i)
				{
					BuildingClass* pOtherBuilding = this->Owner->Buildings[i];

					if (pOtherBuilding->Type == this->Type && pOtherBuilding != this)
					{
						if (pOtherBuilding->Get_Mission() == Mission::Guard &&
							!pOtherBuilding->Factory)
						{
							// 0x44451F - Transfer factory and try alternate
							FactoryClass* pFactory = this->Factory;
							pOtherBuilding->Factory = pFactory;
							this->Factory = nullptr;

							int result = pOtherBuilding->Exit_Object(pObject, CellStruct::Empty);

							pOtherBuilding->Factory = nullptr;
							this->Factory = pFactory;
							return result;
						}
					}
				}

				return 1;
			}

			// 0x444565 - Normal factory exit
			++ScenarioInit;

			CoordStruct exitCoord;
			this->Exit_Coord(&exitCoord, 0);

			if (pObject->Unlimbo(&exitCoord, DirType::East))
			{
				// 0x44459A - Mark up, set position, mark down
				pObject->Mark(MarkType::Up);

				CoordStruct finalExitCoord;
				this->Exit_Coord(&finalExitCoord, 0);
				pObject->Set_Coord(&finalExitCoord);

				pObject->Mark(MarkType::Down);

				// 0x4445CF - Establish tether
				this->Transmit_Message(RadioMessage::Hello, pObject);
				this->Transmit_Message(RadioMessage::Tether, pObject);
				this->Assign_Mission(Mission::Unload, 0);

				--ScenarioInit;
				return 2;
			}

			--ScenarioInit;
			return 0;
		}

		//----------------------------------------------------------------------
		// Barracks / Infantry production - 0x4445FB
		//----------------------------------------------------------------------
		if (pType->ToBuild == AbstractType::InfantryType ||
			pType->Hospital ||
			pType->Armory ||
			pType->Cloning)
		{
			// 0x44498E - Infantry exit from barracks
			TechnoClass::Set_Archive(pObject, this->ArchiveTarget);

			CellStruct exitCell;
			this->Enter_Transport(&exitCell, pObject, CellStruct::Empty);

			if (exitCell == CellStruct::Empty)
			{
				return 0;
			}

			// 0x4449D9 - Handle cloning vats
			pType = this->Type;
			if (pType->ToBuild == AbstractType::InfantryType && !pType->Cloning)
			{
				TechnoTypeClass* pInfType = pObject->Techno_Type_Class();
				HouseClass* pHouse = this->Owner;

				for (int i = 0; i < pHouse->CloningVats.Count; ++i)
				{
					BuildingClass* pVat = pHouse->CloningVats[i];
					ObjectClass* pClone = pInfType->Create_One_Of(pVat->Owner);
					pVat->Exit_Object(static_cast<TechnoClass*>(pClone), CellStruct::Empty);
				}
			}

			// 0x444A53 - Calculate facing direction
			int destY = (exitCell.Y << 8) + 128;
			int destX = (exitCell.X << 8) + 128;

			CoordStruct centerCoord;
			this->Center_Coord(&centerCoord);

			double angle = FastMath::Atan2(
				static_cast<double>(centerCoord.Y - destY),
				static_cast<double>(destX - centerCoord.X)
			);
			angle -= DEG90_AS_RAD;
			int rawFacing = static_cast<int>(angle * BINARY_ANGLE_MAGIC);
			int facing = (((rawFacing >> 7) + 1) >> 1) & 0xFF;

			// 0x444AE5 - Get building cell and dimensions
			CellStruct buildingCell;
			this->Coord_Cell(&buildingCell);

			int buildingWidth = BuildingTypeClass::Width(this->Type);
			int buildingHeight = BuildingTypeClass::Height(this->Type, false);

			// 0x444B0A - Calculate intermediate cell
			CellStruct intermediateCell = exitCell;

			if (exitCell.X >= buildingCell.X + buildingWidth)
			{
				intermediateCell.X = exitCell.X - 1;
			}
			else if (exitCell.X < buildingCell.X)
			{
				intermediateCell.X = exitCell.X + 1;
			}

			if (exitCell.Y >= buildingCell.Y + buildingHeight)
			{
				intermediateCell.Y = exitCell.Y - 1;
			}
			else if (exitCell.Y < buildingCell.Y)
			{
				intermediateCell.Y = exitCell.Y + 1;
			}

			// 0x444B54 - Build unlimbo coordinates
			CoordStruct unlimboCoord;
			unlimboCoord.X = (intermediateCell.X << 8) + 128;
			unlimboCoord.Y = (intermediateCell.Y << 8) + 128;
			unlimboCoord.Z = 0;

			// 0x444B83 - Apply barracks exit offset (GDI)
			BuildingTypeClass* pBuildingType = this->Type;
			if (pBuildingType->GDIBarracks)
			{
				if (exitCell.X == buildingCell.X + 1 && exitCell.Y == buildingCell.Y + 2)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z = pBuildingType->ExitCoordinate.Z;
				}
			}

			// 0x444BD8 - Apply barracks exit offset (NOD)
			if (pBuildingType->NODBarracks)
			{
				if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 2)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z += pBuildingType->ExitCoordinate.Z;
				}
			}

			// 0x444C2C - Apply barracks exit offset (Yuri)
			if (pBuildingType->YuriBarracks)
			{
				if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 1)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z += pBuildingType->ExitCoordinate.Z;
				}
			}

			// 0x444C7C - Unlimbo
			++ScenarioInit;

			if (pObject->Unlimbo(&unlimboCoord, static_cast<DirType>(facing)))
			{
				// 0x444CA3 - Handle infantry door animation
				AnimClass* pDoorAnim = pObject->Anims[18];
				if (pDoorAnim)
				{
					TechnoTypeClass* pTechnoType = pObject->Techno_Type_Class();
					if (!pTechnoType->JumpJet && !pTechnoType->Teleporter)
					{
						if (pObject->Anims[18])
						{
							TechnoClass::Set_Archive(pObject, pObject->Anims[18]);
						}
						pObject->Assign_Mission(Mission::Move, 0);
						CellClass* pExitCell = Map[exitCell];
						pObject->Assign_Destination(pExitCell, true);
					}
				}

				// 0x444D11 - AI behavior
				if (!HouseClass::Is_Player_Control(this->Owner) || this->Type->Hospital)
				{
					pObject->Assign_Mission(Mission::GuardArea, 0);

					CellStruct whereToGo;
					HouseClass::Where_To_Go(this->Owner, &whereToGo, pObject);

					if (whereToGo == CellStruct::Empty || !this->Type->ToBuild)
					{
						TechnoClass::Set_Archive(pObject, nullptr);
					}
					else
					{
						CellClass* pDestCell = Map[whereToGo];
						TechnoClass::Set_Archive(pObject, pDestCell);
						FootClass::Queue_Navigation_List(static_cast<FootClass*>(pObject), pDestCell);
					}
				}

				// 0x444DBC - Establish radio contact
				if (this->Transmit_Message(RadioMessage::Hello, pObject) == RadioMessage::Roger)
				{
					this->Transmit_Message(RadioMessage::Unload, pObject);
				}

				--ScenarioInit;
				return 2;
			}

			--ScenarioInit;
			return 0;
		}

		//----------------------------------------------------------------------
		// Generic unit exit - 0x444638
		//----------------------------------------------------------------------
		{
			CellStruct exitCell;
			this->Enter_Transport(&exitCell, pObject, nUnused);

			if (exitCell == CellStruct::Empty)
			{
				return 0;
			}

			// Calculate facing
			int destY = (exitCell.Y << 8) + 128;
			int destX = (exitCell.X << 8) + 128;

			CoordStruct centerCoord;
			this->Center_Coord(&centerCoord);

			double angle = FastMath::Atan2(
				static_cast<double>(centerCoord.Y - destY),
				static_cast<double>(destX - centerCoord.X)
			);
			angle -= DEG90_AS_RAD;
			int rawFacing = static_cast<int>(angle * BINARY_ANGLE_MAGIC);
			int facing = (((rawFacing >> 7) + 1) >> 1) & 0xFF;

			// Get building dimensions
			CellStruct buildingCell;
			this->Coord_Cell(&buildingCell);

			int buildingWidth = BuildingTypeClass::Width(this->Type);
			int buildingHeight = BuildingTypeClass::Height(this->Type, false);

			// Calculate intermediate cell
			CellStruct intermediateCell = exitCell;

			if (exitCell.X >= buildingCell.X + buildingWidth)
			{
				intermediateCell.X = exitCell.X - 1;
			}
			else if (exitCell.X < buildingCell.X)
			{
				intermediateCell.X = exitCell.X + 1;
			}

			if (exitCell.Y >= buildingCell.Y + buildingHeight)
			{
				intermediateCell.Y = exitCell.Y - 1;
			}
			else if (exitCell.Y < buildingCell.Y)
			{
				intermediateCell.Y = exitCell.Y + 1;
			}

			// Build unlimbo coordinates
			CoordStruct unlimboCoord;
			unlimboCoord.X = (intermediateCell.X << 8) + 128;
			unlimboCoord.Y = (intermediateCell.Y << 8) + 128;
			unlimboCoord.Z = 0;

			// Apply barracks exit offsets
			BuildingTypeClass* pBuildingType = this->Type;

			if (pBuildingType->GDIBarracks)
			{
				if (exitCell.X == buildingCell.X + 1 && exitCell.Y == buildingCell.Y + 2)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z = pBuildingType->ExitCoordinate.Z;
				}
			}

			if (pBuildingType->NODBarracks)
			{
				if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 2)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z += pBuildingType->ExitCoordinate.Z;
				}
			}

			if (pBuildingType->YuriBarracks)
			{
				if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 1)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z += pBuildingType->ExitCoordinate.Z;
				}
			}

			++ScenarioInit;

			if (pObject->Unlimbo(&unlimboCoord, static_cast<DirType>(facing)))
			{
				pObject->Assign_Mission(Mission::Move, 0);
				CellClass* pExitCell = Map[exitCell];
				pObject->Assign_Destination(pExitCell, true);

				// AI behavior
				if (!HouseClass::Is_Player_Control(this->Owner))
				{
					pObject->Assign_Mission(Mission::GuardArea, 0);

					CellStruct whereToGo;
					HouseClass::Where_To_Go(this->Owner, &whereToGo, pObject);

					if (whereToGo == CellStruct::Empty || !this->Type->ToBuild)
					{
						TechnoClass::Set_Archive(pObject, nullptr);
					}
					else
					{
						CellClass* pDestCell = Map[whereToGo];
						TechnoClass::Set_Archive(pObject, pDestCell);
					}
				}

				--ScenarioInit;
				return 2;
			}

			--ScenarioInit;
			return 0;
		}
	}

	//==========================================================================
	// CASE: BUILDING (AbstractType::Building = 6) - 0x444F19
	// AI Construction Yard placing buildings
	//==========================================================================
	case AbstractType::Building:
	{
		// 0x444F1F - Only AI can place buildings this way
		if (HouseClass::Is_Player_Control(this->Owner))
		{
			return 0;
		}

		// 0x444F2C - Adjust house money
		HouseClass::AdjustThrottle(this->Owner, AbstractType::Building);
		this->Owner->BuildStructure = -1;

		BuildingClass* pBuilding = static_cast<BuildingClass*>(pObject);
		BuildingTypeClass* pBuildingType = pBuilding->Type;

		// 0x444F5C - Get next buildable node
		BaseNodeClass* pNode = BaseClass::Next_Buildable(
			&this->Owner->Base, pBuildingType->ArrayIndex
		);

		CoordStruct placeCoord = { 0, 0, 0 };

		//----------------------------------------------------------------------
		// Determine placement location - 0x444F83
		//----------------------------------------------------------------------
		if (pNode && (pNode->Cell.X != 0 || pNode->Cell.Y != 0))
		{
			// Node has valid cell
			CellStruct* pNodeCell = &pNode->Cell;

			// 0x444FA0 - Check if powerup building or spacer
			if (pBuildingType->PowersUpBuilding[0] ||
				HouseClass::Base_Spacer(this->Owner, pBuildingType, pNodeCell))
			{
				// 0x445068 - Use node cell directly
				placeCoord.X = (pNodeCell->X << 8) + 128;
				placeCoord.Y = (pNodeCell->Y << 8) + 128;
				placeCoord.Z = 0;
			}
			else
			{
				// 0x444FC7 - Find build location
				CellStruct foundCell;
				HouseClass::Find_Build_Location(
					this->Owner, &foundCell, pBuildingType, placement_callback, -1
				);

				placeCoord.X = (foundCell.X << 8) + 128;
				placeCoord.Y = (foundCell.Y << 8) + 128;
				placeCoord.Z = 0;

				// Check for invalid location
				if (placeCoord.X == building_defaultcoord.X &&
					placeCoord.Y == building_defaultcoord.Y &&
					placeCoord.Z == building_defaultcoord.Z)
				{
					return 0;
				}

				// Update node cell
				pNodeCell->X = placeCoord.X / 256;
				pNodeCell->Y = placeCoord.Y / 256;
			}
		}
		else
		{
			// 0x445099 - No valid node cell
			if (pBuildingType->PowersUpBuilding[0])
			{
				// 0x4450EF - Find powerup location
				CellStruct powerupCell;
				HouseClass::Find_Powerup_Location(this->Owner, &powerupCell, pBuildingType);

				if (powerupCell != CellStruct::Empty)
				{
					placeCoord.X = (powerupCell.X << 8) + 128;
					placeCoord.Y = (powerupCell.Y << 8) + 128;
					placeCoord.Z = 0;
				}
			}
			else
			{
				// 0x4450A7 - Find regular build location
				CellStruct foundCell;
				HouseClass::Find_Build_Location(
					this->Owner, &foundCell, pBuildingType, placement_callback, -1
				);

				placeCoord.X = (foundCell.X << 8) + 128;
				placeCoord.Y = (foundCell.Y << 8) + 128;
				placeCoord.Z = 0;
			}

			// 0x445151 - Update node if valid placement found
			if (pNode &&
				(placeCoord.X != building_defaultcoord.X ||
					placeCoord.Y != building_defaultcoord.Y ||
					placeCoord.Z != building_defaultcoord.Z))
			{
				pNode->Cell.X = placeCoord.X / 256;
				pNode->Cell.Y = placeCoord.Y / 256;
			}
		}

		//----------------------------------------------------------------------
		// Check if placement is valid - 0x4451AA
		//----------------------------------------------------------------------
		if (placeCoord.X == building_defaultcoord.X &&
			placeCoord.Y == building_defaultcoord.Y &&
			placeCoord.Z == building_defaultcoord.Z)
		{
			// 0x445614 - Invalid placement
			if (pBuildingType->PowersUpBuilding[0])
			{
				if (BaseClass::Next_Buildable(&this->Owner->Base, -1) == pNode)
				{
					int nodeIndex = BaseClass::Next_Buildable_Index(&this->Owner->Base, -1);
					RemoveNodeFromVector(&this->Owner->Base.Nodes, nodeIndex);
				}
			}
			return 0;
		}

		//----------------------------------------------------------------------
		// Try to place building - 0x4451CC
		//----------------------------------------------------------------------
		CellStruct placeCell;
		placeCell.X = placeCoord.X / 256;
		placeCell.Y = placeCoord.Y / 256;

		// 0x445208 - Check placement validity
		int flushResult = BuildingTypeClass::Flush_For_Placement(
			pBuilding->Class_Of(), &placeCell, this->Owner
		);

		// 0x445215 - Handle flush result
		if (flushResult == 0)
		{
			// 0x4452D7 - Clear to place, try unlimbo
			if (pBuilding->Unlimbo(&placeCoord, DirType::North))
			{
				// 0x4452F0 - Deploy slave manager
				if (pBuilding->SlaveManager)
				{
					SlaveManagerClass::Deploy(pBuilding->SlaveManager);
				}

				// 0x4452FF - Update build queue
				if (pNode)
				{
					HouseClass* pHouse = this->Owner;
					if (pBuilding->Type->ArrayIndex == pHouse->BuildStructure)
					{
						pHouse->BuildStructure = -1;
					}
				}

				// 0x445329 - Start construction if queued
				if (pBuilding->CurrentMission == Mission::None &&
					pBuilding->QueuedMission == Mission::Construction)
				{
					pBuilding->Commence();
				}

				// 0x445345 - Handle special building types
				if (pBuilding->WhatAmI() == AbstractType::Building)
				{
					BuildingTypeClass* pPlacedType = pBuilding->Type;

					if (pPlacedType->FirestormWall)
					{
						// 0x445365 - Firestorm wall
						Map.Place_Firestorm_Wall_Building(&placeCell, this->Owner, pPlacedType);
					}
					else if (pPlacedType->ToOverlay && pPlacedType->ToOverlay->IsWall)
					{
						// 0x4453B0 - Regular wall
						Map.Place_Wall_Building(&placeCell, this->Owner, pPlacedType);
					}
				}

				// 0x44540D - Handle wall tower (update defense positions)
				if (pBuilding->Type == RulesClass::Instance->WallTower)
				{
					int nodeIndex = this->Owner->Base.Nodes.ID(pNode);
					int nodeCount = this->Owner->Base.Nodes.Count;

					for (int i = nodeIndex + 1; i < nodeCount; ++i)
					{
						BaseNodeClass* pDefenseNode = this->Owner->Base.Nodes[i];
						if (pDefenseNode->Type >= 0)
						{
							BuildingTypeClass* pDefenseType = BuildingTypeClass::Array[pDefenseNode->Type];
							if (pDefenseType->IsBaseDefense)
							{
								pDefenseNode->Cell.X = pBuilding->Coord.X / 256;
								pDefenseNode->Cell.Y = pBuilding->Coord.Y / 256;
								break;
							}
						}
					}
				}

				return 2;
			}
			// Fall through to failure handling
		}
		else if (flushResult == 1)
		{
			// 0x445237 - Placement blocked, increment attempts
			int attempts = BaseNodeClass::Increase_Attempts(&this->Owner->Base, pNode);

			if (SessionClass::Instance->Type != GameType::Campaign)
			{
				if (attempts > RulesClass::Instance->MaximumBuildingPlacementFailures)
				{
					if (pNode)
					{
						int nodeIndex = this->Owner->Base.Nodes.ID(pNode);
						RemoveNodeFromVector(&this->Owner->Base.Nodes, nodeIndex);
					}
				}
			}

			return 1;
		}
		else if (flushResult != 2)
		{
			// Unknown result
			return 0;
		}

		//----------------------------------------------------------------------
		// Handle placement failure - 0x4454E6
		//----------------------------------------------------------------------
		if (!pNode)
		{
			return 0;
		}

		int nodeIndex = this->Owner->Base.Nodes.ID(pNode);
		BuildingTypeClass* pNodeType = BuildingTypeClass::Array[pNode->Type];

		// 0x445517 - For walls/gates, remove the node
		if (pNodeType->IsWall || pNodeType->IsGate)
		{
			// 0x4455B3
			RemoveNodeFromVector(&this->Owner->Base.Nodes, nodeIndex);
			return 0;
		}

		// 0x44552D - Clear conflicting cell assignments
		int targetCellX = placeCoord.X / 256;
		int targetCellY = placeCoord.Y / 256;

		for (int i = 0; i < this->Owner->Base.Nodes.Count; ++i)
		{
			BaseNodeClass* pOtherNode = this->Owner->Base.Nodes[i];
			if (pOtherNode->Cell.X == targetCellX && pOtherNode->Cell.Y == targetCellY)
			{
				pOtherNode->Cell.X = 0;
				pOtherNode->Cell.Y = 0;
			}
		}

		return 0;
	}

	//==========================================================================
	// DEFAULT - Unsupported object type - 0x445696
	//==========================================================================
	default:
		return 0;
	}
}

//==============================================================================
// Helper function to remove a node from a DynamicVectorClass
// Equivalent to the inline removal code at multiple locations
//==============================================================================
static void RemoveNodeFromVector(DynamicVectorClass<BaseNodeClass*>* pNodes, int index)
{
	int count = pNodes->Count;
	if (index >= count)
	{
		return;
	}

	int newCount = count - 1;
	pNodes->Count = newCount;

	// Shift remaining elements (each node is 16 bytes / 4 pointers)
	for (int i = index; i < newCount; ++i)
	{
		pNodes->Items[i] = pNodes->Items[i + 1];
	}
}

/**
 * BuildingClass::Exit_Object (a.k.a. KickOutUnit)
 *
 * Address: 0x443C60
 *
 * This version includes annotations for all known extension hooks
 * and their effects on the original game logic.
 */
int BuildingClass::Exit_Object(TechnoClass* pObject, int nUnused)
{
	// Null check - 0x443C77
	if (!pObject)
	{
		return 0;
	}

	// Lock the object being exited - 0x443C81
	pObject->IsLocked = true;

	// Check if this is an aircraft - 0x443C8A
	AircraftClass* pAircraft = nullptr;
	if (pObject->WhatAmI() == AbstractType::Aircraft)
	{
		pAircraft = static_cast<AircraftClass*>(pObject);
	}

	// Main switch on object type - 0x443C9B
	AbstractType objectType = pObject->WhatAmI();

	switch (objectType)
	{
		//==========================================================================
		// CASE: AIRCRAFT (AbstractType::Aircraft = 2) - 0x443CB4
		//==========================================================================
	case AbstractType::Aircraft:
	{
		// 0x443CBA - Adjust house money and clear build queue
		HouseClass::AdjustThrottle(this->Owner, AbstractType::Aircraft);

		//======================================================================
		// HOOK: 0x443CCA - BuildingClass_KickOutUnit_AircraftType_Phobos
		// Size: 0xA (10 bytes)
		// 
		// PURPOSE: Clears the HouseExt->Factory_AircraftType if this building
		//          was the aircraft factory.
		//
		// ORIGINAL CODE:
		//   mov dword ptr [edx+5658h], 0FFFFFFFFh  ; Owner->BuildAircraft = -1
		//
		// HOOK EFFECT:
		//   GET(FakeHouseClass*, pHouse, EDX);
		//   GET(BuildingClass*, pThis, ESI);
		//   auto pExt = pHouse->_GetExtData();
		//   if (pThis == pExt->Factory_AircraftType)
		//       pExt->Factory_AircraftType = nullptr;
		//======================================================================
		this->Owner->BuildAircraft = -1;
		// [EXTENSION POINT: Clear Factory_AircraftType in HouseExt if this == Factory_AircraftType]

		// 0x443CD4 - Check radio contact or ion storm conditions
		bool inRadioContact = RadioClass::In_Radio_Contact(&this->Radio, &pObject->Radio);
		bool ionStormActive = IonStorm::IsActive();

		if (inRadioContact || (ionStormActive && !pAircraft->Type->AirportBound))
		{
			// 0x443F54 - Set aircraft down and increment ScenarioInit
			ObjectClass::MarkDownAndSetZ(pAircraft, 0);
			++ScenarioInit;

			if (ionStormActive)
			{
				// 0x443F78 - Find nearby location during ion storm
				CellStruct nearbyCell;
				TechnoClass::Nearby_Location(pAircraft, &nearbyCell, this);

				CoordStruct unlimboCoord;
				unlimboCoord.X = (nearbyCell.X << 8) + 128;
				unlimboCoord.Y = (nearbyCell.Y << 8) + 128;
				unlimboCoord.Z = 0;

				DirType facing = AircraftClass::Pose_Dir(pAircraft);

				//==============================================================
				// HOOK: 0x443FD8 - BuildingClass_ExitObject_PoseDir_NotAirportBound  
				// Size: 0x8 (8 bytes)
				//
				// PURPOSE: Sets aircraft facing and DockedTo when NOT airport bound
				//          (spawning during ion storm at random location)
				//
				// ORIGINAL CODE:
				//   call dword ptr [esi+0D8h]  ; Unlimbo
				//   test al, al
				//   jz loc_444EDE
				//
				// HOOK EFFECT:
				//   if (Unlimbo succeeded) {
				//       pAir->DockedTo = pThis;
				//       DirStruct dir { GetPoseDir(pAir, pThis) << 13 };
				//       if (RulesExtData::Instance()->ExpandAircraftMission)
				//           pAir->PrimaryFacing.Set_Current(dir);
				//       pAir->SecondaryFacing.Set_Current(dir);
				//   }
				//==============================================================
				if (pAircraft->Unlimbo(&unlimboCoord, facing))
				{
					// [EXTENSION POINT: Set DockedTo and facing via BuildingExtData::GetPoseDir]
					--ScenarioInit;
					return 2;
				}
			}
			else
			{
				// 0x443FE5 - Normal airport tethered exit
				DirType facing = AircraftClass::Pose_Dir(pAircraft);
				CoordStruct dockingCoord;
				this->Docking_Coord(&dockingCoord, pAircraft, facing);

				if (pAircraft->Unlimbo(&dockingCoord, facing))
				{
					//==========================================================
					// HOOK: 0x444014 - BuildingClass_ExitObject_PoseDir_AirportBound
					// Size: 0x5 (5 bytes)
					//
					// PURPOSE: Handles airport-bound aircraft exit with proper
					//          radio contact and facing direction
					//
					// ORIGINAL CODE:
					//   push ebp           ; FootClass*
					//   push 2             ; RadioMessage::Hello  
					//   mov ecx, esi       ; this
					//   call [edx+278h]    ; Transmit_Message
					//
					// HOOK EFFECT:
					//   pThis->SendCommand(RadioCommand::RequestLink, pAir);
					//   pThis->SendCommand(RadioCommand::RequestTether, pAir);
					//   pAir->SetLocation(pThis->GetDockCoords(pAir));
					//   pAir->DockedTo = pThis;
					//   FacingType result = BuildingExtData::GetPoseDir(pAir, pThis);
					//   DirStruct dir { result };
					//   if (RulesExtData::Instance()->ExpandAircraftMission)
					//       pAir->PrimaryFacing.Set_Current(dir);
					//   pAir->SecondaryFacing.Set_Current(dir);
					//   return 0x444053;  // Skip original radio/tether code
					//==========================================================

					// Original code (replaced by hook):
					this->Transmit_Message(RadioMessage::Hello, pAircraft);
					this->Transmit_Message(RadioMessage::Tether, pAircraft);
					CoordStruct finalCoord;
					this->Docking_Coord(&finalCoord, pAircraft);
					pAircraft->Set_Coord(&finalCoord);

					// 0x44404D
					pAircraft->DockBuilding = this;

					// 0x444053 - Check for move target (hook returns here)
					TechnoClass* archiveTarget = this->ArchiveTarget;
					if (archiveTarget && !pAircraft->Type->AirportBound)
					{
						pAircraft->Assign_Destination(archiveTarget, true);
						pAircraft->Assign_Mission(Mission::Move, 0);
					}

					--ScenarioInit;
					return 2;
				}
			}

			--ScenarioInit;
			return 0;
		}
		else
		{
			// 0x443D04 - Not in radio contact, spawn at map edge
			if (pAircraft->Type->AirportBound)
			{
				return 0;
			}

			// ... map edge spawn logic (0x443D18 - 0x443EA0) ...
			// [Code omitted for brevity - see previous version]

			// 0x443EA0 - Try to unlimbo at map edge
			CoordStruct spawnCoord;
			// ... calculate spawnCoord ...

			++ScenarioInit;

			if (pAircraft->Unlimbo(&spawnCoord, DirType::North))
			{
				// ... assign destination ...
				--ScenarioInit;
				return 2;
			}

			--ScenarioInit;
			return 0;
		}
	}

	//==========================================================================
	// CASE: UNIT / INFANTRY (AbstractType::Unit = 1, AbstractType::Infantry = 15)
	// 0x444096
	//==========================================================================
	case AbstractType::Unit:
	case AbstractType::Infantry:
	{
		BuildingTypeClass* pType = this->Type;

		// 0x4440A2 - Check if building can handle this unit type
		//==================================================================
		// HOOK: 0x4440B0 - BuildingClass_KickOutUnit_CloningFacility
		// Size: 0x6 (6 bytes)
		//
		// PURPOSE: Allows CloningFacility buildings to bypass WeaponsFactory check
		//
		// ORIGINAL CODE:
		//   cmp cl, cl           ; test WeaponsFactory
		//   jnz short loc_4440D7
		//
		// HOOK EFFECT:
		//   if (!pFactoryType->WeaponsFactory 
		//       || BuildingTypeExtContainer::Instance.Find(pFactoryType)->CloningFacility)
		//       return CheckFreeLinks;  // 0x4440BA
		//   return ContinueIn;  // 0x4440D7
		//==================================================================
		if (!pType->Hospital && !pType->Armory && !pType->WeaponsFactory)
		{
			// [EXTENSION POINT: Also check CloningFacility flag]
			if (!RadioClass::Has_Free_Slots(&this->Radio))
			{
				return 1;
			}
		}

		// 0x4440D7 - Handle money adjustment
		pType = this->Type;
		if (!pType->Hospital && !pType->Armory)
		{
			// 0x4440F4
			HouseClass::AdjustThrottle(this->Owner, pObject->WhatAmI());

			// 0x44410E
			if (pObject->WhatAmI() == AbstractType::Unit)
			{
				//==============================================================
				// HOOK: 0x444113 - BuildingClass_ExitObject_NavalProductionFix1
				// Size: 0x6 (6 bytes)
				//
				// PURPOSE: Separates naval and land unit production indices
				//
				// ORIGINAL CODE:
				//   mov ecx, [esi+21Ch]                    ; Owner
				//   mov [ecx+5650h], ebp                   ; BuildUnit = -1
				//
				// HOOK EFFECT:
				//   if (pObject->Type->Naval)
				//       HouseExtContainer::Instance.Find(pThis->Owner)->ProducingNavalUnitTypeIndex = -1;
				//   else
				//       pThis->Owner->ProducingUnitTypeIndex = -1;
				//==============================================================
				this->Owner->BuildUnit = -1;
				// [EXTENSION POINT: Check Naval flag, set ProducingNavalUnitTypeIndex if naval]

				//==============================================================
				// HOOK: 0x444119 - BuildingClass_KickOutUnit_UnitType_Phobos
				// Size: 0x6 (6 bytes)
				//
				// PURPOSE: Clears Factory_NavyType or Factory_VehicleType in HouseExt
				//
				// HOOK EFFECT:
				//   auto pHouseExt = HouseExtContainer::Instance.Find(pFactory->Owner);
				//   if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == pFactory)
				//       pHouseExt->Factory_NavyType = nullptr;
				//   else if (!pUnit->Type->Naval && pHouseExt->Factory_VehicleType == pFactory)
				//       pHouseExt->Factory_VehicleType = nullptr;
				//==============================================================
				// [EXTENSION POINT: Clear Factory_NavyType or Factory_VehicleType]
			}

			// 0x44411F
			if (pObject->WhatAmI() == AbstractType::Infantry)
			{
				//==============================================================
				// HOOK: 0x444131 - BuildingClass_KickOutUnit_InfantryType_Phobos
				// Size: 0x6 (6 bytes)
				//
				// PURPOSE: Clears Factory_InfantryType in HouseExt
				//
				// HOOK EFFECT:
				//   auto pExt = pHouse->_GetExtData();
				//   if (pThis == pExt->Factory_InfantryType)
				//       pExt->Factory_InfantryType = nullptr;
				//==============================================================
				this->Owner->BuildInfantry = -1;
				// [EXTENSION POINT: Clear Factory_InfantryType if this == Factory_InfantryType]
			}
		}

		// 0x444137
		pType = this->Type;

		//----------------------------------------------------------------------
		// Refinery/Weeder check - 0x444145
		//----------------------------------------------------------------------
		if (pType->Refinery || pType->Weeder)
		{
			// ... refinery exit logic at 0x444DE4 ...
			// [Code identical to previous version]
		}

		//----------------------------------------------------------------------
		// Weapons Factory check - 0x444159
		//----------------------------------------------------------------------
		//======================================================================
		// HOOK: 0x444159 - BuildingClass_KickoutUnit_WeaponFactory_Rubble
		// Size: 0x6 (6 bytes)
		//
		// PURPOSE: Handles RubbleDestroyed buildings specially - allows infantry
		//          to exit even if not matching the factory type
		//
		// ORIGINAL CODE:
		//   cmp cl, cl           ; test WeaponsFactory
		//   jz loc_4445FB        ; jump if not weapons factory
		//
		// HOOK EFFECT:
		//   if (!pThis->Type->WeaponsFactory)
		//       return 0x4445FB;  // not a weapon factory
		//   
		//   auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
		//   if (pExt->RubbleDestroyed) {
		//       if (pThis->Type->Factory == pObj->GetTechnoType()->WhatAmI() 
		//           && pThis->Factory && pThis->Factory->Object == pObj)
		//           return 0x444167;  // continue check
		//       if (pObj->WhatAmI() == AbstractType::Infantry)
		//           return 0x4445FB;  // just eject infantry
		//   }
		//   return 0x444167;  // continue check
		//======================================================================
		if (pType->WeaponsFactory)
		{
			// [EXTENSION POINT: Check RubbleDestroyed and handle infantry ejection]

			//------------------------------------------------------------------
			// Naval factory - 0x444167
			//------------------------------------------------------------------
			if (pType->Naval)
			{
				// ... naval exit logic (0x44418D - 0x444480) ...
				// [Code identical to previous version]
			}

			//------------------------------------------------------------------
			// Land vehicle factory - 0x444492
			//------------------------------------------------------------------
			TechnoClass::Set_Archive(pObject, this->ArchiveTarget);

			//==================================================================
			// HOOK: 0x4444A0 - BuildingClass_KickOutUnit_NoKickOutInConstruction
			// Size: 0xA (10 bytes)
			//
			// PURPOSE: Prevents kicking out units while building is under construction
			//
			// ORIGINAL CODE:
			//   mov edx, [esi]
			//   mov ecx, esi
			//   call dword ptr [edx+184h]    ; Get_Mission
			//   cmp eax, 10h                 ; Mission::Unload
			//   jnz loc_444565
			//
			// HOOK EFFECT:
			//   const auto mission = pThis->GetCurrentMission();
			//   return (mission == Mission::Unload || mission == Mission::Construction) 
			//       ? ThisIsNotOK : ThisIsOK;
			//   // ThisIsNotOK = 0x4444B3 (try alternate factory)
			//   // ThisIsOK = 0x444565 (normal exit)
			//==================================================================

			// 0x4444A4 - Check if building is unloading (already processing)
			if (this->Get_Mission() == Mission::Unload)
			{
				// [EXTENSION POINT: Also block if mission == Construction]

				//==============================================================
				// HOOK: 0x4444B3 - BuildingClass_KickOutUnit_NoAlternateKickout
				// Size: 0x6 (6 bytes)
				//
				// PURPOSE: Prevents alternate kickout for CloningFacility or
				//          buildings with Factory == None
				//
				// ORIGINAL CODE:
				//   mov eax, [esi+21Ch]       ; Owner
				//   xor ebx, ebx
				//   mov eax, [eax+78h]        ; Buildings.Count
				//
				// HOOK EFFECT:
				//   return pThis->Type->Factory == AbstractType::None
				//       || pThis->_GetTypeExtData()->CloningFacility.Get()
				//       ? 0x4452C5 : 0x0;  // Skip alternate factory search
				//==============================================================
				// [EXTENSION POINT: Skip alternate factory for CloningFacility]

				// 0x4444CA - Try to find alternate factory
				int buildingCount = this->Owner->Buildings.Count;

				for (int i = 0; i < buildingCount; ++i)
				{
					BuildingClass* pOtherBuilding = this->Owner->Buildings[i];

					//==========================================================
					// HOOK: 0x4444E2 - BuildingClass_KickOutUnit_FindAlternateKickout
					// Size: 0x6 (6 bytes)
					//
					// PURPOSE: Enhanced alternate factory matching with Naval
					//          and TechnoType compatibility checks
					//
					// ORIGINAL CODE:
					//   cmp ecx, edx          ; Compare Type pointers
					//   jnz short loc_444508
					//
					// HOOK EFFECT:
					//   if (Src != Tst
					//       && Tst->GetCurrentMission() == Mission::Guard
					//       && Tst->Type->Factory == Src->Type->Factory
					//       && Tst->Type->Naval == Src->Type->Naval
					//       && TechnoTypeExtData::CanBeBuiltAt(Production->GetTechnoType(), Tst->Type)
					//       && !Tst->Factory)
					//       return 0x44451F;  // Use this alternate
					//   return 0x444508;  // Continue search
					//==========================================================
					if (pOtherBuilding->Type == this->Type && pOtherBuilding != this)
					{
						// [EXTENSION POINT: Also check Factory type, Naval flag, CanBeBuiltAt]
						if (pOtherBuilding->Get_Mission() == Mission::Guard &&
							!pOtherBuilding->Factory)
						{
							// 0x44451F - Transfer factory and try alternate
							FactoryClass* pFactory = this->Factory;
							pOtherBuilding->Factory = pFactory;
							this->Factory = nullptr;

							int result = pOtherBuilding->Exit_Object(pObject, CellStruct::Empty);

							pOtherBuilding->Factory = nullptr;
							this->Factory = pFactory;
							return result;
						}
					}
				}

				return 1;
			}

			// 0x444565 - Normal factory exit
			++ScenarioInit;

			CoordStruct exitCoord;
			this->Exit_Coord(&exitCoord, 0);

			if (pObject->Unlimbo(&exitCoord, DirType::East))
			{
				pObject->Mark(MarkType::Up);
				CoordStruct finalExitCoord;
				this->Exit_Coord(&finalExitCoord, 0);
				pObject->Set_Coord(&finalExitCoord);
				pObject->Mark(MarkType::Down);

				this->Transmit_Message(RadioMessage::Hello, pObject);
				this->Transmit_Message(RadioMessage::Tether, pObject);
				this->Assign_Mission(Mission::Unload, 0);

				//==============================================================
				// HOOK: 0x4445F6 - BuildingClass_KickOutUnit_Clone_NonNavalUnit
				// Size: 0x5 (5 bytes)
				//
				// PURPOSE: Handles cloning for non-naval units exiting factory
				//
				// LOCATION: Just before "jmp loc_444971"
				//
				// HOOK EFFECT:
				//   --Unsorted::ScenarioInit;  // turn it off
				//   TechnoExt_ExtData::KickOutClones(Factory, Production);
				//   ++Unsorted::ScenarioInit;  // turn it back on
				//   return 0x444971;  // Success path
				//==============================================================
				// [EXTENSION POINT: KickOutClones for non-naval units]

				--ScenarioInit;
				return 2;
			}

			--ScenarioInit;
			return 0;
		}

		//----------------------------------------------------------------------
		// Barracks / Infantry production - 0x4445FB
		// (ToBuild == InfantryType || Hospital || Armory || Cloning)
		//----------------------------------------------------------------------
		if (pType->ToBuild == AbstractType::InfantryType ||
			pType->Hospital ||
			pType->Armory ||
			pType->Cloning)
		{
			TechnoClass::Set_Archive(pObject, this->ArchiveTarget);

			CellStruct exitCell;
			this->Enter_Transport(&exitCell, pObject, CellStruct::Empty);

			if (exitCell == CellStruct::Empty)
			{
				return 0;
			}

			//==================================================================
			// HOOK: 0x4449DF - BuildingClass_KickOutUnit_PreventClone  
			// Size: LJMP (5 bytes)
			// DEFINE_JUMP(LJMP, 0x4449DF, 0x444A53);
			//
			// PURPOSE: Skips the original cloning vats logic entirely
			//          (handled by extension code instead)
			//
			// ORIGINAL CODE at 0x4449D9:
			//   mov eax, [esi+520h]          ; Type
			//   cmp [eax+0EB8h], ebx         ; ToBuild == 0?
			//   jnz short loc_444A53
			//   ... cloning vat logic ...
			//
			// HOOK EFFECT:
			//   Unconditional jump to 0x444A53, bypassing all original
			//   cloning vat iteration code
			//==================================================================

			// Original cloning code (SKIPPED by hook):
			/*
			pType = this->Type;
			if (pType->ToBuild == AbstractType::InfantryType && !pType->Cloning) {
				TechnoTypeClass* pInfType = pObject->Techno_Type_Class();
				HouseClass* pHouse = this->Owner;

				for (int i = 0; i < pHouse->CloningVats.Count; ++i) {
					BuildingClass* pVat = pHouse->CloningVats[i];
					ObjectClass* pClone = pInfType->Create_One_Of(pVat->Owner);
					pVat->Exit_Object(static_cast<TechnoClass*>(pClone), CellStruct::Empty);
				}
			}
			*/
			// [EXTENSION: Cloning handled by KickOutClones at 0x444DBC instead]

			// 0x444A53 - Calculate facing direction
			int destY = (exitCell.Y << 8) + 128;
			int destX = (exitCell.X << 8) + 128;

			CoordStruct centerCoord;
			this->Center_Coord(&centerCoord);

			double angle = FastMath::Atan2(
				static_cast<double>(centerCoord.Y - destY),
				static_cast<double>(destX - centerCoord.X)
			);
			angle -= DEG90_AS_RAD;
			int rawFacing = static_cast<int>(angle * BINARY_ANGLE_MAGIC);
			int facing = (((rawFacing >> 7) + 1) >> 1) & 0xFF;

			// 0x444AE5 - Get building cell and dimensions
			CellStruct buildingCell;
			this->Coord_Cell(&buildingCell);

			int buildingWidth = BuildingTypeClass::Width(this->Type);
			int buildingHeight = BuildingTypeClass::Height(this->Type, false);

			// Calculate intermediate cell (0x444B0A - 0x444B54)
			CellStruct intermediateCell = exitCell;
			// ... calculation logic ...

			// Build unlimbo coordinates (0x444B54 - 0x444B83)
			CoordStruct unlimboCoord;
			unlimboCoord.X = (intermediateCell.X << 8) + 128;
			unlimboCoord.Y = (intermediateCell.Y << 8) + 128;
			unlimboCoord.Z = 0;

			//==================================================================
			// HOOK: 0x444B83 - BuildingClass_ExitObject_BarracksExitCell
			// Size: 0x7 (7 bytes)
			//
			// PURPOSE: Allows custom BarracksExitCell configuration via
			//          BuildingTypeExt instead of hardcoded GDI/NOD/Yuri checks
			//
			// ORIGINAL CODE:
			//   cmp byte ptr [ecx+16E4h], 0  ; GDIBarracks check
			//   jz short loc_444BD8
			//
			// HOOK EFFECT:
			//   if (pThis->_GetTypeExtData()->BarracksExitCell.isset()) {
			//       auto exitCoords = pThis->Type->ExitCoord;
			//       resultCoords = CoordStruct { 
			//           xCoord + exitCoords.X, 
			//           yCoord + exitCoords.Y, 
			//           exitCoords.Z 
			//       };
			//       return SkipGameCode;  // 0x444C7C
			//   }
			//   return 0;  // Continue with original GDI/NOD/Yuri checks
			//==================================================================

			// Original barracks offset code (may be skipped by hook):
			BuildingTypeClass* pBuildingType = this->Type;
			// [EXTENSION POINT: Check BarracksExitCell.isset() first]

			if (pBuildingType->GDIBarracks)
			{
				if (exitCell.X == buildingCell.X + 1 && exitCell.Y == buildingCell.Y + 2)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z = pBuildingType->ExitCoordinate.Z;
				}
			}

			// 0x444BD8 - NODBarracks
			if (pBuildingType->NODBarracks)
			{
				if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 2)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z += pBuildingType->ExitCoordinate.Z;
				}
			}

			// 0x444C2C - YuriBarracks
			if (pBuildingType->YuriBarracks)
			{
				if (exitCell.X == buildingCell.X + 2 && exitCell.Y == buildingCell.Y + 1)
				{
					unlimboCoord.X += pBuildingType->ExitCoordinate.X;
					unlimboCoord.Y += pBuildingType->ExitCoordinate.Y;
					unlimboCoord.Z += pBuildingType->ExitCoordinate.Z;
				}
			}

			// 0x444C7C - Unlimbo (BarracksExitCell hook returns here)
			++ScenarioInit;

			if (pObject->Unlimbo(&unlimboCoord, static_cast<DirType>(facing)))
			{
				// 0x444CA3 - Handle infantry door animation
				// ... animation logic ...

				// 0x444D11 - AI behavior
				if (!HouseClass::Is_Player_Control(this->Owner) || this->Type->Hospital)
				{
					//==========================================================
					// HOOK: 0x444D26 - BuildingClass_KickOutUnit_ArmoryExitBug
					// Size: 0x6 (6 bytes)
					//
					// PURPOSE: Fixes bug where Armory units don't get focus reset
					//          (Hospital units should, but Armory should too)
					//
					// ORIGINAL CODE:
					//   mov al, [edx+16C1h]   ; Hospital flag
					//   test al, al
					//   jz loc_444DBC
					//
					// HOOK EFFECT:
					//   R->AL(pType->Hospital || pType->Armory);
					//   // Now both Hospital AND Armory skip the archive reset
					//==========================================================
					// [EXTENSION POINT: Check Hospital || Armory instead of just Hospital]

					pObject->Assign_Mission(Mission::GuardArea, 0);

					CellStruct whereToGo;
					HouseClass::Where_To_Go(this->Owner, &whereToGo, pObject);

					if (whereToGo == CellStruct::Empty || !this->Type->ToBuild)
					{
						TechnoClass::Set_Archive(pObject, nullptr);
					}
					else
					{
						CellClass* pDestCell = Map[whereToGo];
						TechnoClass::Set_Archive(pObject, pDestCell);
						FootClass::Queue_Navigation_List(static_cast<FootClass*>(pObject), pDestCell);
					}
				}

				//==============================================================
				// HOOK: 0x444DBC - BuildingClass_KickOutUnit_Infantry
				// Size: 0x5 (5 bytes)
				//
				// PURPOSE: Handles infantry cloning when exiting barracks
				//
				// ORIGINAL CODE:
				//   mov eax, [esi]
				//   push edi               ; pObject
				//   push 2                 ; RadioMessage::Hello
				//
				// HOOK EFFECT:
				//   --Unsorted::ScenarioInit;
				//   TechnoExt_ExtData::KickOutClones(Factory, Production);
				//   ++Unsorted::ScenarioInit;
				//   return 0;  // Continue to radio contact code
				//==============================================================
				// [EXTENSION POINT: KickOutClones for infantry]

				//==============================================================
				// HOOK: 0x444DC9 - BuildingClass_KickOutUnit_Barracks
				// Size: 0x9 (9 bytes)
				//
				// PURPOSE: Enhanced barracks exit handling with scatter fallback
				//
				// ORIGINAL CODE:
				//   cmp eax, 1             ; RadioMessage::Roger
				//   jnz loc_444971
				//
				// HOOK EFFECT:
				//   if (respond == RadioCommand::AnswerPositive) {
				//       pThis->SendCommand(RadioCommand::RequestUnload, pProduct);
				//       if (auto pDest = pProduct->ArchiveTarget) {
				//           pProduct->SetDestination(pDest, true);
				//           return 0x444971;
				//       }
				//       pProduct->Scatter(CoordStruct::Empty, true, false);
				//   }
				//   return 0x444971;
				//==============================================================

				// Original radio contact code:
				if (this->Transmit_Message(RadioMessage::Hello, pObject) == RadioMessage::Roger)
				{
					this->Transmit_Message(RadioMessage::Unload, pObject);
					// [EXTENSION POINT: Also SetDestination or Scatter]
				}

				--ScenarioInit;
				return 2;
			}

			--ScenarioInit;
			return 0;
		}

		//----------------------------------------------------------------------
		// Generic unit exit - 0x444638
		//----------------------------------------------------------------------
		{
			// ... similar structure to barracks, shares the 0x444B83 hook area ...
			// [Code similar to barracks section]
		}

		// Naval unit exit also has:
		//==================================================================
		// HOOK: 0x44441A - BuildingClass_KickOutUnit_Clone_NavalUnit
		// Size: 0x6 (6 bytes)
		//
		// PURPOSE: Handles cloning for naval units exiting shipyard
		//
		// HOOK EFFECT:
		//   TechnoExt_ExtData::KickOutClones(Factory, Production);
		//   return 0;  // Continue normal execution
		//==================================================================
	}

	//==========================================================================
	// CASE: BUILDING (AbstractType::Building = 6) - 0x444F19
	//==========================================================================
	case AbstractType::Building:
	{
		if (HouseClass::Is_Player_Control(this->Owner))
		{
			return 0;
		}

		HouseClass::AdjustThrottle(this->Owner, AbstractType::Building);

		//======================================================================
		// HOOK: 0x44531F - BuildingClass_KickOutUnit_BuildingType_Phobos
		// Size: 0xA (10 bytes)
		// NOTE: This is at 0x44531F, AFTER successful unlimbo at 0x4452D7
		//
		// PURPOSE: Clears Factory_BuildingType in HouseExt
		//
		// HOOK EFFECT:
		//   auto pExt = pHouse->_GetExtData();
		//   if (pThis == pExt->Factory_BuildingType)
		//       pExt->Factory_BuildingType = nullptr;
		//======================================================================
		this->Owner->BuildStructure = -1;

		BuildingClass* pBuilding = static_cast<BuildingClass*>(pObject);
		BuildingTypeClass* pBuildingType = pBuilding->Type;

		BaseNodeClass* pNode = BaseClass::Next_Buildable(&this->Owner->Base, pBuildingType->ArrayIndex);

		CoordStruct placeCoord = { 0, 0, 0 };

		// ... placement location logic (0x444F83 - 0x4451AA) ...

		// 0x4451CC - Check placement validity
		CellStruct placeCell;
		placeCell.X = placeCoord.X / 256;
		placeCell.Y = placeCoord.Y / 256;

		//======================================================================
		// HOOK: 0x4451F8 - BuildingClass_KickOutUnit_CleanUpAIBuildingSpace
		// Size: 0x6 (6 bytes)
		//
		// PURPOSE: Major hook for AI building placement with many features:
		//   - AIForbidConYard: Prevents AI from placing construction yards
		//   - AICleanWallNode: Cleans up invalid wall placement nodes
		//   - LimboBuild: Creates limbo buildings instead of real ones
		//   - ExtendedBuildingPlacing: Enhanced placement validation
		//   - CleanUpBuildingSpace: Clears space for AI buildings
		//
		// ORIGINAL CODE:
		//   mov eax, [esi+21Ch]           ; Owner
		//   mov edx, [edi]                ; pBuilding vtable
		//   push eax                      ; house
		//   push ecx                      ; cell
		//   mov ecx, edi                  ; this = pBuilding
		//   call [edx+88h]                ; Class_Of
		//
		// HOOK EFFECT: See full hook code for details - handles:
		//   CanBuild = 0x4452F0
		//   TemporarilyCanNotBuild = 0x445237
		//   CanNotBuild = 0x4454E6
		//   BuildSucceeded = 0x4454D4
		//   BuildFailed = 0x445696
		//======================================================================

		// 0x445208 - Original: Flush_For_Placement call
		int flushResult = BuildingTypeClass::Flush_For_Placement(
			pBuilding->Class_Of(), &placeCell, this->Owner
		);
		// [EXTENSION POINT: ExtendedBuildingPlacing, LimboBuild, etc.]

		// 0x445215 - Handle flush result
		if (flushResult == 0)
		{
			// 0x4452D7 - Try unlimbo
			if (pBuilding->Unlimbo(&placeCoord, DirType::North))
			{
				// ... slave manager, build queue logic ...

				// 0x445345 - Handle special building types
				if (pBuilding->WhatAmI() == AbstractType::Building)
				{
					BuildingTypeClass* pPlacedType = pBuilding->Type;

					//==========================================================
					// HOOK: 0x445355 - BuildingClass_KickOutUnit_Firewall
					// Size: 0x6 (6 bytes)
					//
					// PURPOSE: Custom firewall building logic
					//
					// ORIGINAL CODE:
					//   mov ecx, [edi+520h]       ; Type
					//   cmp byte ptr [ecx+16C0h], 0 ; FirestormWall check
					//
					// HOOK EFFECT:
					//   FirewallFunctions::BuildLines(B, CenterPos, Factory->Owner);
					//   return 0;  // Continue with original logic
					//==========================================================

					if (pPlacedType->FirestormWall)
					{
						// [EXTENSION POINT: FirewallFunctions::BuildLines]
						Map.Place_Firestorm_Wall_Building(&placeCell, this->Owner, pPlacedType);
					}
					else if (pPlacedType->ToOverlay && pPlacedType->ToOverlay->IsWall)
					{
						Map.Place_Wall_Building(&placeCell, this->Owner, pPlacedType);
					}
				}

				//==============================================================
				// HOOK: 0x44540D - BuildingClass_ExitObject_WallTowers
				// Size: 0x5 (5 bytes)
				//
				// PURPOSE: Extended wall tower detection via RulesExt
				//
				// ORIGINAL CODE:
				//   mov eax, RulesClass Rule
				//   mov edx, [edi+520h]           ; pBuilding->Type
				//   cmp edx, [eax+87Ch]           ; Rule->WallTower
				//
				// HOOK EFFECT:
				//   R->EDX(pThis->Type);
				//   const auto& Nvec = RulesExtData::Instance()->WallTowers;
				//   return Nvec.Contains(pThis->Type) ? 0x445424 : 0x4454D4;
				//   // Uses configurable WallTowers list instead of single type
				//==============================================================

				// Original: Single WallTower check
				if (pBuilding->Type == RulesClass::Instance->WallTower)
				{
					// [EXTENSION POINT: Check WallTowers vector instead]
					// ... update defense node positions ...
				}

				return 2;
			}
		}
		else if (flushResult == 1)
		{
			// 0x445237 - Blocked
			// ... attempt tracking logic ...
			return 1;
		}

		// ... failure handling ...
		return 0;
	}

	default:
		return 0;
	}
}


//==============================================================================
// SUMMARY OF ALL HOOKS IN THIS FUNCTION
//==============================================================================
/*

ADDRESS     SIZE  NAME                                            EFFECT
---------   ----  --------------------------------------------    ------------------------------------------
0x443CCA    0xA   KickOutUnit_AircraftType_Phobos                 Clear HouseExt->Factory_AircraftType
0x443FD8    0x8   ExitObject_PoseDir_NotAirportBound              Set facing for non-airport aircraft
0x444014    0x5   ExitObject_PoseDir_AirportBound                 Set facing for airport aircraft
0x444113    0x6   ExitObject_NavalProductionFix1                  Separate naval/land unit indices
0x444119    0x6   KickOutUnit_UnitType_Phobos                     Clear Factory_NavyType/Factory_VehicleType
0x444131    0x6   KickOutUnit_InfantryType_Phobos                 Clear Factory_InfantryType
0x444159    0x6   KickoutUnit_WeaponFactory_Rubble                RubbleDestroyed handling
0x4440B0    0x6   KickOutUnit_CloningFacility                     CloningFacility bypass
0x4444A0    0xA   KickOutUnit_NoKickOutInConstruction             Block during construction
0x4444B3    0x6   KickOutUnit_NoAlternateKickout                  Skip alternate for CloningFacility
0x4444E2    0x6   KickOutUnit_FindAlternateKickout                Enhanced alternate factory matching
0x44441A    0x6   KickOutUnit_Clone_NavalUnit                     Naval unit cloning
0x4445F6    0x5   KickOutUnit_Clone_NonNavalUnit                  Non-naval unit cloning
0x4449DF    LJMP  KickOutUnit_PreventClone                        Skip original clone logic
0x444B83    0x7   ExitObject_BarracksExitCell                     Custom barracks exit cell
0x444D26    0x6   KickOutUnit_ArmoryExitBug                       Fix armory focus reset
0x444DBC    0x5   KickOutUnit_Infantry                            Infantry cloning
0x444DC9    0x9   KickOutUnit_Barracks                            Enhanced barracks exit
0x44531F    0xA   KickOutUnit_BuildingType_Phobos                 Clear Factory_BuildingType
0x445355    0x6   KickOutUnit_Firewall                            Custom firewall logic
0x44540D    0x5   ExitObject_WallTowers                           Extended wall tower list
0x4451F8    0x6   KickOutUnit_CleanUpAIBuildingSpace              AI building placement overhaul

*/
