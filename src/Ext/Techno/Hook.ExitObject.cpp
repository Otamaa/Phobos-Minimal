#include <CellStruct.h>
#include <CoordStruct.h>

#include <HouseClass.h>

#include <Ext/Building/Body.h>

COMPILETIMEEVAL CoordStruct CellToCoord(const CellStruct& cell, int z = 0, bool snap = true) {
	int offset = snap ? 128 : 0;
	return {
		(cell.X << 8) + offset,
		(cell.Y << 8) + offset,
		z
	};
}

// Helper: Convert world coordinates to cell coordinates
COMPILETIMEEVAL  CellStruct CoordToCell(const CoordStruct& coord) {
	return {
		static_cast<short>(coord.X / 256),
		static_cast<short>(coord.Y / 256)
	};
}

void RemoveNodeFromList(DynamicVectorClass<BaseNodeClass>* nodeList, int index) {
	nodeList->erase_at(index);
}

DirType Calculate_Exit_Direction(BuildingClass* pBuilding, const CoordStruct& targetCoord)
{
	CoordStruct buildingCenter = pBuilding->GetCoords();
	float angle = Math::atan2(double(targetCoord.Y - buildingCenter.Y), double(targetCoord.X - buildingCenter.X));
		  angle = (angle - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC;

	return static_cast<DirType>((((static_cast<int>(angle) >> 7) + 1) >> 1));
}

CellStruct Adjust_Exit_Cell(BuildingClass* pBuilding, const CellStruct& targetCell)
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

CoordStruct Apply_Barracks_Exit_Offset(BuildingClass* pBuilding,
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

bool Place_And_Configure_Unit(BuildingClass* pFactory ,
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

void Handle_Cloning_Vats(BuildingClass* pFactory, TechnoClass* infantry)
{
	if (pFactory->Type->Factory != AbstractType::Infantry || pFactory->Type->Cloning) {
		return;
	}

	TechnoTypeClass* infantryType = infantry->GetTechnoType();
	HouseClass* house = pFactory->Owner;

	// Duplicate infantry in all cloning vats
	for (int i = 0; i < house->CloningVats.Count; ++i) {

		BuildingClass* cloningVat = house->CloningVats.Items[i];
		TechnoClass* clone = (TechnoClass*)infantryType->CreateObject(cloningVat->Owner);

		cloningVat->KickOutUnit(clone, CellStruct::Empty);
	}
}

int Exit_Aircraft(AircraftClass* aircraft)
{
	// Mark aircraft as down (on ground)
	ObjectClass_isdown_markdown_set_z_5F6060(&aircraft->f.t.r.m.o, 0);

	++ScenarioInit;

	bool placed = false;
	DirType exitDirection = AircraftClass::Pose_Dir(aircraft);

	// Check if in radio contact or if ion storm is active
	if (RadioClass::In_Radio_Contact(&this->t.r, &aircraft->f.t.r) ||
		IonStormClass_53A130() && !aircraft->Type->AirportBound)
	{

		if (IonStormClass_53A130())
		{
			// Ion storm active - place at nearby location
			CellStruct nearbyCell = *TechnoClass::Nearby_Location(&aircraft->f.t, nullptr, this);
			CoordStruct exitCoord = CellToCoord(nearbyCell);

			placed = aircraft->f.Unlimbo(&aircraft->f.t.r.m.o,
																	 &exitCoord, exitDirection);
		}
		else
		{
			// Normal exit - place at docking coordinate
			CoordStruct dockCoord = *this->t.r.m.o.a.vftable->t.r.m.o.Docking_Coord(this, nullptr,
																					 aircraft, exitDirection);

			placed = aircraft->f.Unlimbo(&aircraft->f.t.r.m.o,
																	 &dockCoord, exitDirection);

			if (placed)
			{
				// Establish radio contact with building
				this->t.r.m.o.a.vftable->t.r.Transmit_Message__MSG__PTR(&this->t.r, RADIO_HELLO, &aircraft->f);
				this->t.r.m.o.a.vftable->t.r.Transmit_Message__MSG__PTR(&this->t.r, RADIO_TETHER, &aircraft->f);

				// Set aircraft position precisely
				CoordStruct finalDockCoord = *this->t.r.m.o.a.vftable->t.r.m.o.Docking_Coord(this, nullptr, aircraft);
				aircraft->f.t.r.m.o.a.vftable->t.r.m.o.Set_Coord(&aircraft->f.t, &finalDockCoord);

				aircraft->dock_technopointer6CC = &this->t;

				// Assign destination if not airport-bound
				TechnoClass* target = this->t.ArchiveTarget;
				if (target && !aircraft->Type->AirportBound)
				{
					aircraft->f.t.r.m.o.a.vftable->t.Assign_Destination(aircraft, target, true);
					aircraft->f.t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(aircraft, Mission::Move, 0);
				}
			}
		}
	}
	else
	{
		// Not in radio contact - must be non-airport-bound aircraft
		if (aircraft->Type->AirportBound)
		{
			--ScenarioInit;
			return 0; // Airport-bound aircraft need radio contact
		}

		// Calculate edge of map exit position
		CoordStruct buildingCenter;
		this->t.r.m.o.a.vftable->t.r.m.o.a.Center_Coord(this, &buildingCenter);

		CellStruct centerCell = CoordToCell(buildingCenter);

		int mapLocalX = MapClass_MapLocalSize.X;
		int mapLocalY = MapClass_MapLocalSize.Y;
		int mapWidth = MapClass_MapSize.Width;

		// Determine which map edge to exit from
		CellStruct exitCell;
		int diagonal1 = (mapLocalX + 1) + mapLocalY;
		int diagonal2 = (mapWidth - mapLocalX) + mapLocalY;

		if ((centerCell.X - diagonal1) - (centerCell.Y - diagonal2) <= MapClass_MapLocalSize.Width)
		{
			// Exit from one diagonal edge
			exitCell.X = diagonal1 - 1;
			exitCell.Y = diagonal2;
		}
		else
		{
			// Exit from opposite diagonal edge
			exitCell.X = MapClass_MapLocalSize.Width + diagonal1 - 1;
			exitCell.Y = diagonal2 - MapClass_MapLocalSize.Width;
		}

		// Add random offset along edge
		int randomOffset = Random2Class::operator()(&Scen->RandomNumber, 0, MapClass_MapLocalSize.Height);
		exitCell.X += randomOffset;
		exitCell.Y += randomOffset;

		CoordStruct exitCoord = CellToCoord(exitCell);

		placed = aircraft->f.Unlimbo(&aircraft->f.t.r.m.o,
																 &exitCoord, DIR_MIN);

		if (placed)
		{
			TechnoClass* target = this->t.ArchiveTarget;
			if (target)
			{
				aircraft->f.t.r.m.o.a.vftable->t.Assign_Destination(&aircraft->f.t.r.m.o, target);
				aircraft->f.t.r.m.o.a.vftable->t.r.m.Set_Mission__Bullet_Unlimbo_BulletMoveTo_YRPP(aircraft, 2);
			}
			else
			{
				// Find nearby location for non-targeted aircraft
				CellStruct nearbyCell = *TechnoClass::Nearby_Location(&aircraft->f.t, nullptr, this);

				if (nearbyCell == default_cell)
				{
					aircraft->f.t.r.m.o.a.vftable->t.Assign_Destination(&aircraft->f.t.r.m.o, nullptr);
					aircraft->f.t.r.m.o.a.vftable->t.r.m.Set_Mission__Bullet_Unlimbo_BulletMoveTo_YRPP(aircraft, 2);
				}
				else
				{
					CellClass* destCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &nearbyCell);
					aircraft->f.t.r.m.o.a.vftable->t.Assign_Destination(&aircraft->f.t.r.m.o, destCell, true);
					aircraft->f.t.r.m.o.a.vftable->t.r.m.Set_Mission__Bullet_Unlimbo_BulletMoveTo_YRPP(aircraft, 2);
				}
			}
		}
	}

	--ScenarioInit;

	return placed ? 2 : 0;
}

int Exit_Building(BuildingClass* building)
{
	// Player cannot use this function
	if (HouseClass::IsControlledByHuman(this->t.House)) {
		return 0;
	}

	HouseClass_whimp_on_money(this->t.House, RTTI_BUILDING);
	this->t.House->BuildStructure = -1;

	BuildingTypeClass* buildingType = building->Type;
	BaseNodeClass* baseNode = BaseClass::Next_Buildable(&this->t.House->Base.vtable, buildingType->Type);

	CoordStruct placementCoord = building_defaultcoord;

	// Determine placement location
	if (baseNode && (baseNode->Cell.X != 0 || baseNode->Cell.Y != 0))
	{
		// Has a predefined base location
		CellStruct targetCell = baseNode->Cell;

		if (buildingType->PowersUpBuilding[0] ||
			HouseClass::Base_Spacer(this->t.House, buildingType, &targetCell))
		{
			// Use the base node location directly
			placementCoord = CellToCoord(targetCell);
		}
		else
		{
			// Find a suitable build location
			CellStruct buildCell = *HouseClass::Find_Build_Location(this->t.House, nullptr,
																	 buildingType, placement_callback, -1);

			placementCoord = CellToCoord(buildCell);

			if (placementCoord == building_defaultcoord)
			{
				return 0; // No valid location found
			}

			// Update base node with found location
			baseNode->Cell = CoordToCell(placementCoord);
		}
	}
	else
	{
		// No base node - find location dynamically
		if (buildingType->PowersUpBuilding[0])
		{
			// Find power-up location
			CellStruct powerUpCell = *HouseClass_powerups_506B90(this->t.House, nullptr, buildingType);

			if (powerUpCell != default_cell)
			{
				placementCoord = CellToCoord(powerUpCell);
			}
		}
		else
		{
			// Find general build location
			CellStruct buildCell = *HouseClass::Find_Build_Location(this->t.House, nullptr,
																	 buildingType, placement_callback, -1);
			placementCoord = CellToCoord(buildCell);
		}

		// Update base node if exists
		if (baseNode && placementCoord != building_defaultcoord)
		{
			baseNode->Cell = CoordToCell(placementCoord);
		}
	}

	// Validate placement location
	if (placementCoord == building_defaultcoord)
	{
		// Failed to find location - handle power-up building special case
		if (buildingType->PowersUpBuilding[0])
		{
			if (BaseClass::Next_Buildable(&this->t.House->Base.vtable, -1) == baseNode)
			{
				int nodeIndex = BaseClass::Next_Buildable_Index(&this->t.House->Base, -1);
				RemoveNodeFromList(&this->t.House->Base.Nodes, nodeIndex);
			}
		}
		return 0;
	}

	// Try to flush any obstacles at placement site
	CellStruct placementCell = CoordToCell(placementCoord);

	int flushResult = BuildingTypeClass::Flush_For_Placement(
		building->t.r.m.o.a.vftable->t.r.m.o.Class_Of(&building->t.r.m.o),
		&placementCell, this->t.House);

	if (flushResult == 1)
	{
		// Temporary blockage - increment failure counter
		int failureCount = BaseNodeClass::Increase_Attempts(&this->t.House->Base, baseNode);

		if (Session.Type && failureCount > Rule->MaximumBuildingPlacementFailures)
		{
			// Too many failures - remove from build queue
			if (baseNode)
			{
				int nodeIndex = this->t.House->Base.Nodes.vftble->ID2(&this->t.House->Base.Nodes, baseNode);
				RemoveNodeFromList(&this->t.House->Base.Nodes, nodeIndex);
			}
		}
		return 1; // Temporary failure
	}

	if (flushResult == 2)
	{
		return 0; // Permanent blockage
	}

	// Try to place building
	if (!building->Unlimbo(&building->t.r.m.o, &placementCoord, DIR_MIN))
	{
		// Placement failed - update base node for walls/gates
		if (buildingType->IsWall || buildingType->IsGate)
		{
			if (baseNode)
			{
				int nodeIndex = this->t.House->Base.Nodes.vftble->ID2(&this->t.House->Base.Nodes, baseNode);
				RemoveNodeFromList(&this->t.House->Base.Nodes, nodeIndex);
			}
		}
		else
		{
			// For other buildings, clear the cell in base nodes
			HouseClass* house = this->t.House;
			CellStruct failedCell = CoordToCell(placementCoord);

			for (int i = 0; i < house->Base.Nodes.ActiveCount; ++i)
			{
				BaseNodeClass* node = house->Base.Nodes.Vector_Item[i];
				if (node->Cell.X == failedCell.X && node->Cell.Y == failedCell.Y)
				{
					node->Cell.X = 0;
					node->Cell.Y = 0;
				}
			}
		}
		return 0;
	}

	// Successfully placed building

	// Deploy slave miners if applicable
	SlaveManagerClass* slaveManager = building->t.__SlaveManager;
	if (slaveManager)
	{
		SlaveManagerClass::Deploy2(slaveManager);
	}

	// Clear build structure flag if this was the building being built
	if (baseNode && buildingType->Type == this->t.House->BuildStructure)
	{
		this->t.House->BuildStructure = -1;
	}

	// Commence construction if queued
	if (building->t.r.m.Mission == MISSION_NONE &&
		building->t.r.m.MissionQueue == MISSION_CONSTRUCTION)
	{
		building->t.r.m.o.a.vftable->t.r.m.Commence(&building->t.r.m);
	}

	// Handle special wall placement
	if (building->t.r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&building->t.r.m.o.a) == RTTI_BUILDING)
	{
		if (buildingType->FirestormWall)
		{
			CellStruct wallCell = CoordToCell(placementCoord);
			MapClass::Place_Firestorm_Wall_Building(&Map.sc.t.sb.p.r.d.m, &wallCell,
													this->t.House, buildingType);
		}
		else if (buildingType->ToOverlay && buildingType->ToOverlay->IsWall)
		{
			CellStruct wallCell = CoordToCell(placementCoord);
			MapClass::Place_Wall_Building(&Map.sc.t.sb.p.r.d.m, &wallCell,
										 this->t.House, buildingType);
		}
	}

	// Handle wall tower special logic
	if (buildingType == Rule->WallTower)
	{
		int nodeIndex = this->t.House->Base.Nodes.vftble->ID2(&this->t.House->Base.Nodes, baseNode);
		int nextIndex = nodeIndex + 1;

		// Find next base defense building in the list
		if (nextIndex < this->t.House->Base.Nodes.ActiveCount)
		{
			for (int i = nextIndex; i < this->t.House->Base.Nodes.ActiveCount; ++i)
			{
				BaseNodeClass* nextNode = this->t.House->Base.Nodes.Vector_Item[i];
				int buildingTypeIndex = nextNode->Type;

				if (buildingTypeIndex >= 0 && (*(&BuildingTypes + 1))[buildingTypeIndex]->IsBaseDefense)
				{
					// Update next defense building's cell to this tower's location
					nextNode->Cell = CoordToCell(building->t.r.m.o.Coord);
					break;
				}
			}
		}
	}

	return 2; // Successfully placed
}

int Exit_Naval_Vehicle(FootClass* ship)
{
	// Check if building is already in radio contact
	if (!RadioClass::Is_In_Radio_Contact(&this->t.r))
	{
		this->t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(this, MISSION_UNLOAD, 0);
	}

	// Get building center
	CoordStruct buildingCenter;
	this->t.r.m.o.a.vftable->t.r.m.o.a.Center_Coord(this, &buildingCenter);

	CellStruct exitCell = CoordToCell(buildingCenter);

	// If there's an archive target, calculate direction towards it
	TechnoClass* archiveTarget = this->t.ArchiveTarget;
	if (archiveTarget)
	{
		CoordStruct targetCoord;
		archiveTarget->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(archiveTarget, &targetCoord);

		CellStruct targetCell = CoordToCell(targetCoord);
		CellStruct buildingCell = *this->t.r.m.o.a.vftable->t.r.m.o.Coord_Cell(this);

		// Calculate direction angle
		float angle = FastMath::Atan2(buildingCell.Y - targetCell.Y, targetCell.X - buildingCell.X);
		angle = (angle - DEG90_AS_RAD) * BINARY_ANGLE_MAGIC;
		int dirIndex = ((static_cast<int>(angle) >> 12) + 1) >> 1) & 7;

		// Move exit cell in calculated direction until clear of building
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &exitCell);
		while (CellClass::Cell_Building(cell) == this)
		{
			exitCell.X += AdjacentCell[dirIndex].X;
			exitCell.Y += AdjacentCell[dirIndex].Y;
			cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &exitCell);
		}
	}

	// Validate exit cell
	CellClass* exitCellClass = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &exitCell);

	// Check if we need to find a better location
	if (!this->t.ArchiveTarget ||
		exitCellClass->Land != LAND_WATER ||
		CellClass::Cell_Techno(exitCellClass, nullptr, 0, 0) ||
		!MapClass::In_Radar(&Map.sc.t.sb.p.r.d.m, &exitCell, true))
	{

		// Find nearby valid water location
		CellStruct searchCell = CoordToCell(buildingCenter);
		TechnoTypeClass* shipType = ship->t.r.m.o.a.vftable->t.r.m.o.Techno_Type_Class(&ship->t);

		exitCell = *MapClass::Nearby_Location(&Map.sc.t.sb.p.r.d.m, nullptr, &searchCell,
											  shipType->Speed, -1, MZONE_NORMAL,
											  false, true, true, false, false, false, true,
											  nullptr, false, false);
	}

	// Get final exit coordinates
	CellClass* finalCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &exitCell);
	CoordStruct exitCoord = *finalCell->a.vftable->t.r.m.o.a.Center_Coord(finalCell);

	// Try to place ship
	if (!ship->Unlimbo(&ship->t.r.m.o, &exitCoord, DIR_E))
	{
		return 0; // Failed to place
	}

	// Configure ship after placement
	TechnoClass* target = this->t.ArchiveTarget;
	if (target)
	{
		ship->t.r.m.o.a.vftable->t.Assign_Destination(ship, target, true);
		ship->t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(ship, Mission::Move, 0);
	}

	// Update ship position
	ship->t.r.m.o.a.vftable->t.r.m.o.Mark(ship, MARK_UP);
	CoordStruct* cellCenter = CellClass::Cell_Coord(finalCell);
	ship->t.r.m.o.a.vftable->t.r.m.o.Set_Coord(&ship->t, cellCenter);
	ship->t.r.m.o.a.vftable->t.r.m.o.Mark(ship, MARK_DOWN);

	return 2; // Successfully exited
}

int Exit_Ground_Vehicle(FootClass* unit)
{
	TechnoClass::Set_Archive(&unit->t, this->t.ArchiveTarget);

	// Get exit cell from building
	CellStruct exitCell = *this->t.r.m.o.a.vftable->Enter_Transport_4D4(this, nullptr, unit, default_cell);

	if (exitCell == default_cell)
	{
		return 0; // No valid exit
	}

	// Convert cell to coordinates (centered)
	CoordStruct exitCoord = CellToCoord(exitCell);

	// Calculate exit direction
	DirType exitDirection = Calculate_Exit_Direction(exitCoord);

	// Adjust exit cell based on building dimensions
	CellStruct adjustedCell = Adjust_Exit_Cell(exitCell);

	// Convert adjusted cell to coordinates (centered)
	CoordStruct adjustedCoord = CellToCoord(adjustedCell);

	// Apply faction-specific barracks offsets (also used for some vehicle factories)
	CellStruct buildingCell = *this->t.r.m.o.a.vftable->t.r.m.o.Coord_Cell(this);
	adjustedCoord = Apply_Barracks_Exit_Offset(adjustedCoord, exitCell, buildingCell);

	++ScenarioInit;

	bool placed = unit->Unlimbo(&unit->t.r.m.o, &adjustedCoord, exitDirection);

	--ScenarioInit;

	if (!placed)
	{
		return 0; // Failed to place
	}

	// Assign move mission to exit destination
	unit->t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(unit, Mission::Move, 0);
	CellClass* destCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &exitCell);
	unit->t.r.m.o.a.vftable->t.Assign_Destination(&unit->t.r.m.o, destCell, true);

	// Handle AI unit behavior
	if (!HouseClass::IsControlledByHuman(this->t.House))
	{
		unit->t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(unit, Mission::GuardArea, 0);

		CellStruct rallyPoint = *HouseClass::Where_To_Go(this->t.House, nullptr, unit);

		if (rallyPoint == default_cell || !this->Type->Factory)
		{
			TechnoClass::Set_Archive(&unit->t, nullptr);
		}
		else
		{
			CellClass* rallyCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &rallyPoint);
			TechnoClass::Set_Archive(&unit->t, rallyCell);
		}
	}

	return 2; // Successfully exited
}

int Exit_Harvester(FootClass* harvester)
{
	// Get building center and calculate exit position (adjacent cell to the south)
	CoordStruct buildingCenter;
	this->t.r.m.o.a.vftable->t.r.m.o.a.Center_Coord(this, &buildingCenter);

	CellStruct exitCell = CoordToCell(buildingCenter);
	exitCell.X += AdjacentCell[5].X;
	exitCell.Y += AdjacentCell[5].Y;

	// Calculate final exit position (one cell further south)
	exitCell.X += AdjacentCell[4].X;
	exitCell.Y += AdjacentCell[4].Y;

	CoordStruct exitCoord = CellToCoord(exitCell);

	++ScenarioInit;

	bool placed = harvester->Unlimbo(&harvester->t.r.m.o, &exitCoord, 160);

	--ScenarioInit;

	if (!placed)
	{
		return 0; // Failed to place
	}

	// Set harvester facing south
	DirStruct facing;
	facing.un.Facing = 0x8000;
	FacingClass::Set(&harvester->t.PrimaryFacing, &facing);

	// Assign harvest mission
	harvester->t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(harvester, MISSION_HARVEST, 0);

	return 2; // Successfully exited
}

int Exit_Infantry_Type(FootClass* unit)
{
	TechnoClass::Set_Archive(&unit->t, this->t.ArchiveTarget);

	// Get exit cell from building
	CellStruct exitCell = *this->t.r.m.o.a.vftable->Enter_Transport_4D4(this, nullptr, &unit->t, default_cell);

	if (exitCell == default_cell)
	{
		return 0; // No valid exit
	}

	// Handle cloning vats if applicable
	Handle_Cloning_Vats(&unit->t);

	// Convert cell to coordinates (centered)
	CoordStruct exitCoord = CellToCoord(exitCell);

	// Calculate exit direction
	DirType exitDirection = Calculate_Exit_Direction(exitCoord);

	// Adjust exit cell based on building dimensions
	CellStruct adjustedCell = Adjust_Exit_Cell(exitCell);

	// Convert adjusted cell to coordinates (centered)
	CoordStruct adjustedCoord = CellToCoord(adjustedCell);

	// Apply faction-specific barracks offsets
	CellStruct buildingCell = *this->t.r.m.o.a.vftable->t.r.m.o.Coord_Cell(this);
	adjustedCoord = Apply_Barracks_Exit_Offset(adjustedCoord, exitCell, buildingCell);

	// Try to place unit at exit
	if (Place_And_Configure_Unit(unit, adjustedCoord, exitDirection, exitCell))
	{
		return 2; // Successfully exited
	}

	return 0; // Failed to place
}

int FakeBuildingClass::__ExitObject(TechnoClass* object, int exitFlags)
{
	if (!object)
	{
		return 0;
	}

	// Lock the building during exit operations
	this->t.IsLocked = true;

	// Determine object type
	RTTIType objectType = object->r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&object->r.m.o.a);
	AircraftClass* aircraft = nullptr;

	if (objectType == RTTI_AIRCRAFT)
	{
		aircraft = static_cast<AircraftClass*>(object);
	}

	// Route to appropriate exit handler based on object type
	switch (objectType)
	{
	case RTTI_UNIT:
	case RTTI_INFANTRY:
	{
		BuildingTypeClass* buildingType = this->Type;

		// Check if building can exit units
		if (!buildingType->Hospital &&
			!buildingType->Armory &&
			!buildingType->WeaponsFactory &&
			!RadioClass::Has_Free_Slots(&this->t.r))
		{
			return 1; // Building is full
		}

		// Handle production buildings
		if (!buildingType->Hospital && !buildingType->Armory)
		{
			// Deduct money for production
			HouseClass_whimp_on_money(this->t.House, objectType);

			if (objectType == RTTI_UNIT)
			{
				this->t.House->BuildUnit = -1;
			}
			if (objectType == RTTI_INFANTRY)
			{
				this->t.House->BuildInfantry = -1;
			}
		}

		// Check for special building types
		if (buildingType->Refinery || buildingType->Weeder)
		{
			// Harvester exit
			if (objectType != RTTI_UNIT)
			{
				object->r.m.o.a.vftable->t.r.m.o.Scatter(object, &building_defaultcoord, true, false);
				return 0;
			}
			return Exit_Harvester(static_cast<FootClass*>(object));
		}

		if (!buildingType->WeaponsFactory)
		{
			// Infantry/healing facility exit
			if (buildingType->Factory == RTTI_INFANTRYTYPE ||
				buildingType->Hospital ||
				buildingType->Armory ||
				buildingType->Cloning)
			{
				return Exit_Infantry_Type(static_cast<FootClass*>(object));
			}

			// Non-infantry unit exit (regular ground vehicles)
			return Exit_Ground_Vehicle(static_cast<FootClass*>(object));
		}

		// War factory exit
		if (!buildingType->tt.IsNaval)
		{
			// Ground vehicle factory
			TechnoClass::Set_Archive(&object->t, this->t.ArchiveTarget);

			// Check if unloading and try to delegate to another factory
			if (this->t.r.m.o.a.vftable->t.r.m.o.Get_Mission(&this->t.r.m) == MISSION_UNLOAD)
			{
				HouseClass* house = this->t.House;

				for (int i = 0; i < house->__Buildings.ActiveCount; ++i)
				{
					BuildingClass* otherFactory = house->__Buildings.Vector_Item[i];

					if (otherFactory->Type == this->Type && otherFactory != this)
					{
						if (otherFactory->t.r.m.o.a.vftable->t.r.m.o.Get_Mission(otherFactory) == MISSION_GUARD &&
							!otherFactory->Factory)
						{

							// Transfer factory to other building temporarily
							FactoryClass* tempFactory = this->Factory;
							otherFactory->Factory = tempFactory;
							this->Factory = nullptr;

							int result = otherFactory->Exit_Object(object, default_cell);

							// Restore factory ownership
							otherFactory->Factory = nullptr;
							this->Factory = tempFactory;

							return result;
						}
					}
				}
				return 1; // No available factory found
			}

			// Standard ground vehicle exit through war factory
			++ScenarioInit;

			CoordStruct exitCoord = *this->t.r.m.o.a.vftable->t.r.m.o.Exit_Coord(this, nullptr, 0);
			bool placed = object->r.m.o.a.vftable->t.r.m.o.Unlimbo(&object->r.m.o, &exitCoord, DIR_E);

			if (placed)
			{
				object->r.m.o.a.vftable->t.r.m.o.Mark(object, MARK_UP);
				CoordStruct* finalCoord = this->t.r.m.o.a.vftable->t.r.m.o.Exit_Coord(this, nullptr, 0);
				object->r.m.o.a.vftable->t.r.m.o.Set_Coord(&object->r.m.o, finalCoord);
				object->r.m.o.a.vftable->t.r.m.o.Mark(object, MARK_DOWN);

				this->t.r.m.o.a.vftable->t.r.Transmit_Message__MSG__PTR(&this->t.r, RADIO_HELLO, object);
				this->t.r.m.o.a.vftable->t.r.Transmit_Message__MSG__PTR(&this->t.r, RADIO_TETHER, object);
				this->t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(&this->t, MISSION_UNLOAD, 0);

				--ScenarioInit;
				return 2;
			}

			--ScenarioInit;
			return 0;
		}

		// Naval yard exit
		return Exit_Naval_Vehicle(static_cast<FootClass*>(object));
	}

	case RTTI_AIRCRAFT:
	{
		HouseClass_whimp_on_money(this->t.House, RTTI_AIRCRAFT);
		this->t.House->BuildAircraft = -1;

		return Exit_Aircraft(aircraft);
	}

	case RTTI_BUILDING:
	{
		return Exit_Building(static_cast<BuildingClass*>(object));
	}

	default:
		return 0;
	}
}