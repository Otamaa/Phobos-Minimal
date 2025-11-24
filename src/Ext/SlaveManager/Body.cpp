#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Utilities/Macro.h>

bool IsOwnerBuilding(TechnoClass* owner)
{
	return owner && owner->r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&owner->r.m.o.a) == RTTI_BUILDING;
}

bool IsOwnerUnit(TechnoClass* owner)
{
	return owner && owner->r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&owner->r.m.o.a) == RTTI_UNIT;
}

BuildingClass* TechnoAsBuilding(TechnoClass* techno)
{
	if (!techno) return nullptr;
	return techno->r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&techno->r.m.o.a) == RTTI_BUILDING
		? static_cast<BuildingClass*>(techno)
		: nullptr;
}

UnitClass* TechnoAsUnit(TechnoClass* techno)
{
	if (!techno) return nullptr;
	return techno->r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&techno->r.m.o.a) == RTTI_UNIT
		? static_cast<UnitClass*>(techno)
		: nullptr;
}

CellStruct GetPrimaryExitCell()
{
	BuildingClass* building = TechnoAsBuilding(Owner);
	if (!building)
	{
		CellStruct result;
		Owner->r.m.o.a.vftable->t.r.m.o.Coord_Cell(Owner, &result);
		return result;
	}

	return GetBuildingExitCell(building);
}

CellStruct GetSecondaryExitCell()
{
	if (!IsOwnerBuilding(Owner))
	{
		return default_cell_0;
	}

	CellStruct primaryCell = GetPrimaryExitCell();

	CellStruct secondaryCell;
	secondaryCell.X = primaryCell.X;
	secondaryCell.Y = primaryCell.Y - 1;

	CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &secondaryCell);

	// Check if cell belongs to this building
	if (CellClass::Cell_Building(cell) == TechnoAsBuilding(Owner))
	{
		return secondaryCell;
	}

	return default_cell_0;
}

CellStruct* Cell1(CellStruct* result)
{
	if (IsOwnerBuilding(Owner))
	{
		*result = GetPrimaryExitCell();
	}
	else
	{
		Owner->r.m.o.a.vftable->t.r.m.o.Coord_Cell(Owner, result);
	}

	return result;
}

CellStruct* Cell2(CellStruct* result)
{
	*result = GetSecondaryExitCell();
	return result;
}

bool Cell3(InfantryClass* infantry, CellClass* cell)
{
	CellStruct primaryCell = GetPrimaryExitCell();

	// Check primary cell
	if (cell == MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &primaryCell))
	{
		return IsInfantryInSlaveControl(infantry);
	}

	// Check secondary cell
	CellStruct secondaryCell = GetSecondaryExitCell();
	if (!IsAtLastOreLocation(secondaryCell))
	{
		if (cell == MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &secondaryCell))
		{
			return IsInfantryInSlaveControl(infantry);
		}
	}

	return false;
}

bool IsInfantryInSlaveControl(InfantryClass* infantry)
{
	for (int i = SlaveControls.ActiveCount - 1; i >= 0; --i)
	{
		if (SlaveControls.Vector_Item[i]->Slave == infantry)
		{
			return true;
		}
	}

	return false;
}

CellStruct GetBuildingExitCell(BuildingClass* building)
{
	int width = BuildingTypeClass::Width(building->Class) - 1;
	int height = BuildingTypeClass::Height(building->Class, 0) / 2;

	CellStruct buildingCell;
	building->t.r.m.o.a.vftable->t.r.m.o.Coord_Cell(&building->t, &buildingCell);

	CellStruct offset;
	offset.X = width;
	offset.Y = height;

	CellStruct result;
	Cell::operator+(&buildingCell, &result, &offset);
	return result;
}

CellStruct GetOwnerCell(TechnoClass* owner)
{
	if (IsOwnerBuilding(owner))
	{
		BuildingClass* building = TechnoAsBuilding(owner);
		if (building)
			return GetBuildingExitCell(building);
	}

	CellStruct result;
	owner->r.m.o.a.vftable->t.r.m.o.Coord_Cell(owner, &result);
	return result;
}

inline bool IsTimerExpired(const FrameTimerClass& timer)
{
	if (timer.Started == -1)
		return timer.Accumulated_DelayTime == 0;

	return (Frame - timer.Started) >= timer.Accumulated_DelayTime;
}

inline void StartTimer(FrameTimerClass& timer, int delay)
{
	timer.Started = Frame;
	timer.Accumulated_DelayTime = delay;
}

inline bool CellsEqual(const CellStruct& cell1, const CellStruct& cell2)
{
	return cell1.X == cell2.X && cell1.Y == cell2.Y;
}

inline bool IsAtLastOreLocation(const CellStruct& cell)
{
	return CellsEqual(cell, LastOreLocation);
}

void AssignMoveToCell(TechnoClass* techno, const CellStruct& targetCell)
{
	CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &targetCell);
	techno->r.m.o.a.vftable->t.Assign_Destination(&techno->r.m.o, cell, 1);
	techno->r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(techno, MISSION_MOVE, 0);
}

void RecallAllSlaves(SlaveManagerClass* manager)
{
	for (int i = manager->SlaveControls.ActiveCount - 1; i >= 0; --i)
	{
		SlaveManagerClass::SlaveControl* ctrl = manager->SlaveControls.Vector_Item[i];
		if (ctrl->State != SLAVECTRL_DEAD && ctrl->Slave)
		{
			ctrl->Slave->f.t.r.m.o.a.vftable->t.msub_70F850(ctrl->Slave);
		}
	}
}

void ResetSlaveControl(SlaveManagerClass::SlaveControl* ctrl, int respawnDelay)
{
	if (ctrl->Slave)
	{
		ctrl->Slave->f.t.__EnslavedBy = nullptr;
	}
	ctrl->Slave = nullptr;
	ctrl->State = SLAVECTRL_DEAD;
	StartTimer(ctrl->RespawnTimer, respawnDelay);
}

void HandleOreSearch(SlaveControl* ctrl)
{
	InfantryClass* slave = ctrl->Slave;
	CellStruct oreCell;

	// Find ore location
	slave->f.t.r.m.o.a.vftable->t.Goto_Tiberium(
		slave, &oreCell,
		Rule->SlaveMinerSlaveScan / 256, 0
	);

	if (IsAtLastOreLocation(oreCell))
	{
		// No ore found, return to base
		TechnoClass::Set_Archive(&slave->f.t, nullptr);
		CellStruct targetCell = GetOwnerCell(Owner);
		AssignMoveToCell(&slave->f.t, targetCell);
		ctrl->State = SLAVECTRL_UNLOAD_ORE;
	}
	else
	{
		// Move to ore location
		AssignMoveToCell(&slave->f.t, oreCell);
		ctrl->State = SLAVECTRL_MOVE_TO_ORE;
	}
}

void HandleMoveToOre(SlaveControl* ctrl)
{
	InfantryClass* slave = ctrl->Slave;
	CellClass* currentCell = slave->f.t.r.m.o.a.vftable->t.r.m.o.Coord_CellClass(slave);

	if (CellClass::Has_Tiberium(currentCell))
	{
		InfantryClass::Force_Harvest(slave);
		ctrl->State = SLAVECTRL_HARVEST;
	}
	else if (!slave->f.NavCom)
	{
		ctrl->State = SLAVECTRL_HANDLE_ORE;
	}
}

void HandleHarvest(SlaveControl* ctrl)
{
	InfantryClass* slave = ctrl->Slave;

	if (InfantryClass::Has_Tiberium_Load(slave))
	{
		// Full load, return to base
		CellClass* currentCell = slave->f.t.r.m.o.a.vftable->t.r.m.o.Coord_CellClass(slave);
		TechnoClass::Set_Archive(&slave->f.t, currentCell);

		CellStruct targetCell = GetOwnerCell(Owner);
		AssignMoveToCell(&slave->f.t, targetCell);
		ctrl->State = SLAVECTRL_UNLOAD_ORE;
	}
	else
	{
		CellClass* currentCell = slave->f.t.r.m.o.a.vftable->t.r.m.o.Coord_CellClass(slave);

		if (CellClass::Has_Tiberium(currentCell))
		{
			if (!InfantryClass::Is_Harvesting(slave))
			{
				ctrl->State = SLAVECTRL_HANDLE_ORE;
			}
		}
		else
		{
			InfantryClass::Force_Guard(slave);
			ctrl->State = SLAVECTRL_HANDLE_ORE;
		}
	}
}

bool ShouldResetPath(InfantryClass* slave)
{
	if (!slave->f.NavCom)
		return false;

	CellStruct targetCell = GetOwnerCell(Owner);

	CoordStruct navCoord;
	slave->f.NavCom->vftable->t.r.m.o.a.Center_Coord(slave->f.NavCom, &navCoord);

	CellStruct navCell;
	navCell.X = navCoord.X / 256;
	navCell.Y = navCoord.Y / 256;

	CellStruct delta;
	delta.X = navCell.X - targetCell.X;
	delta.Y = navCell.Y - targetCell.Y;

	return CellStruct::length(&delta) > Rule->ApproachTargetResetMultiplier;
}

void HandleUnloadOre(SlaveControl* ctrl)
{
	InfantryClass* slave = ctrl->Slave;
	CellStruct targetCell = GetOwnerCell(Owner);

	bool shouldReset = ShouldResetPath(slave);

	CellStruct slaveCell;
	slave->f.t.r.m.o.a.vftable->t.r.m.o.Coord_Cell(&slave->f.t, &slaveCell);

	if (CellsEqual(slaveCell, targetCell))
	{
		if (!slave->f.NavCom)
		{
			// Arrived at base, unload
			InfantryClass::Storage_AI(slave, Owner);
			slave->f.t.r.m.o.a.vftable->t.r.m.o.Limbo(&slave->f.t.r.m.o);

			ctrl->State = SLAVECTRL_RESPAWN;
			StartTimer(ctrl->RespawnTimer, ReloadRate);
			return;
		}
	}

	if (!slave->f.NavCom || shouldReset)
	{
		// Repath to base
		AssignMoveToCell(&slave->f.t, targetCell);
	}
}

void HandleRespawn(SlaveControl* ctrl)
{
	if (IsTimerExpired(ctrl->RespawnTimer))
	{
		InfantryClass* slave = ctrl->Slave;
		InfantryTypeClass* slaveType = slave->Class;

		slave->f.t.r.m.o.Strength = slaveType->tt.ot.MaxStrength;
		slave->f.t.r.m.o.__Strength2 = slaveType->tt.ot.MaxStrength;
		ctrl->State = SLAVECTRL_RESPAWNING;
	}
}

void HandleDead(SlaveControl* ctrl)
{
	if (IsTimerExpired(ctrl->RespawnTimer))
	{
		Create_Slave(this, ctrl);
	}
}

void Slave_AI()
{
	for (int i = 0; i < SlaveControls.ActiveCount; ++i)
	{
		SlaveControl* ctrl = SlaveControls.Vector_Item[i];
		InfantryClass* slave = ctrl->Slave;

		// Validate slave state
		if (!slave && ctrl->State != SLAVECTRL_DEAD)
		{
			ctrl->Slave = nullptr;
			ctrl->State = SLAVECTRL_DEAD;
			StartTimer(ctrl->RespawnTimer, RegenRate);
		}

		// Process state machine
		switch (ctrl->State)
		{
		case SLAVECTRL_HANDLE_ORE:
			HandleOreSearch(ctrl);
			break;

		case SLAVECTRL_MOVE_TO_ORE:
			HandleMoveToOre(ctrl);
			break;

		case SLAVECTRL_HARVEST:
			HandleHarvest(ctrl);
			break;

		case SLAVECTRL_UNLOAD_ORE:
			HandleUnloadOre(ctrl);
			break;

		case SLAVECTRL_RESPAWN:
			HandleRespawn(ctrl);
			break;

		case SLAVECTRL_DEAD:
			HandleDead(ctrl);
			break;
		}
	}
}

bool CanStartFindingOre()
{
	if (!IsOwnerBuilding(Owner))
		return false;

	MissionType mission = Owner->r.m.Mission;
	return mission != MISSION_DECONSTRUCTION &&
		mission != MISSION_CONSTRUCTION;
}

void HandleScanState()
{
	// Check if unit is deployed
	if (IsOwnerUnit(Owner))
	{
		UnitClass* unit = TechnoAsUnit(Owner);
		if (unit && unit->f.IsDeploying)
		{
			State = SLAVEMAN_MOVE;
			LastScanFrame = 0x7FFFFFFF;
			return;
		}
	}

	// Find ore location
	CellStruct oreCell;
	Owner->r.m.o.a.vftable->t.Goto_Tiberium(
		Owner, &oreCell,
		Rule->SlaveMinerLongScan / 256, 1
	);

	if (IsAtLastOreLocation(oreCell))
	{
		State = SLAVEMAN_0;
		LastScanFrame = Frame;
		return;
	}

	// Calculate deployment location
	CellStruct deployCell;
	Where_To_Deploy(&deployCell, oreCell);

	if (IsAtLastOreLocation(deployCell))
	{
		State = SLAVEMAN_0;
		LastScanFrame = Frame;
	}
	else
	{
		AssignMoveToCell(Owner, deployCell);
		State = SLAVEMAN_MOVE;
		LastScanFrame = 0x7FFFFFFF;
	}
}

void HandleMoveState()
{
	UnitClass* unit = TechnoAsUnit(Owner);
	if (!unit)
	{
		State = SLAVEMAN_0;
		LastScanFrame = Frame;
		return;
	}

	if (!unit->f.NavCom)
	{
		if (UnitClass::Try_To_Deploy(unit))
		{
			State = SLAVEMAN_4;
			LastScanFrame = 0x7FFFFFFF;
		}
		else
		{
			State = SLAVEMAN_DEPLOY;
			StartTimer(LogicDelayTimer, 30);
			LastScanFrame = 0x7FFFFFFF;
		}
	}
}

void HandleDeployState()
{
	UnitClass* unit = TechnoAsUnit(Owner);
	if (!unit)
	{
		State = SLAVEMAN_0;
		LastScanFrame = Frame;
		return;
	}

	if (UnitClass::Try_To_Deploy(unit))
	{
		State = SLAVEMAN_4;
		LastScanFrame = 0x7FFFFFFF;
	}
}

bool IsBuildingFullyDeployed()
{
	BuildingClass* building = TechnoAsBuilding(Owner);
	if (!building)
		return false;

	return building->t.r.m.o.a.TargetBitfield != 0;
}

void HandleDeployedState()
{
	if (IsOwnerBuilding(Owner))
	{
		if (IsBuildingFullyDeployed())
		{
			State = SLAVEMAN_FIND_ORE;
			LastScanFrame = 0x7FFFFFFF;
		}
	}
	else if (IsOwnerUnit(Owner))
	{
		UnitClass* unit = TechnoAsUnit(Owner);
		if (unit && !unit->f.IsDeploying && Owner->r.m.Mission == MISSION_GUARD)
		{
			State = SLAVEMAN_MOVE;
			LastScanFrame = 0x7FFFFFFF;
		}
	}
}

bool ShouldRelocateForOre()
{
	// Find short range ore
	CellStruct nearOre;
	Owner->r.m.o.a.vftable->t.Goto_Tiberium(
		Owner, &nearOre,
		Rule->SlaveMinerShortScan / 256
	);

	if (!IsAtLastOreLocation(nearOre))
		return false;

	// Find long range ore
	CellStruct farOre;
	Owner->r.m.o.a.vftable->t.Goto_Tiberium(
		Owner, &farOre,
		Rule->SlaveMinerLongScan / 256
	);

	CellStruct deployCell;
	Where_To_Deploy(&deployCell, farOre);

	// Calculate distances
	CellStruct ownerCell;
	Owner->r.m.o.a.vftable->t.r.m.o.Coord_Cell(Owner, &ownerCell);

	CellStruct deltaToOre;
	deltaToOre.X = deployCell.X - farOre.X;
	deltaToOre.Y = deployCell.Y - farOre.Y;
	int distToOre = FastMath::Sqrt(deltaToOre.X * deltaToOre.X + deltaToOre.Y * deltaToOre.Y);

	CellStruct deltaToCurrent;
	deltaToCurrent.X = ownerCell.X - farOre.X;
	deltaToCurrent.Y = ownerCell.Y - farOre.Y;
	int distToCurrent = FastMath::Sqrt(
		deltaToCurrent.X * deltaToCurrent.X +
		deltaToCurrent.Y * deltaToCurrent.Y
	);

	return (distToOre + Rule->SlaveMinerScanCorrection / 256) < distToCurrent;
}

void HandleFindOreState()
{
	if (IsOwnerUnit(Owner))
	{
		State = SLAVEMAN_0;
		LastScanFrame = Frame;
		return;
	}

	if (ShouldRelocateForOre())
	{
		Owner->deploy4F8 = 1;
		CellClass* currentCell = Owner->r.m.o.a.vftable->t.r.m.o.Coord_CellClass(Owner);
		TechnoClass::Set_Archive(Owner, currentCell);
		Owner->r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(
			Owner, MISSION_DECONSTRUCTION, 0
		);
		State = SLAVEMAN_UNDEPLOY;
		LastScanFrame = 0x7FFFFFFF;
	}
	else
	{
		Send_Slaves();
	}
}

void HandleUndeployState()
{
	if (IsOwnerUnit(Owner))
	{
		RecallAllSlaves(this);
		State = SLAVEMAN_SCAN;
		LastScanFrame = 0x7FFFFFFF;
	}
}

void Owner_AI()
{
	switch (State)
	{
	case SLAVEMAN_0:
		if (Owner && CanStartFindingOre())
		{
			State = SLAVEMAN_FIND_ORE;
			LastScanFrame = 0x7FFFFFFF;
		}
		break;

	case SLAVEMAN_SCAN:
		HandleScanState();
		break;

	case SLAVEMAN_MOVE:
		HandleMoveState();
		break;

	case SLAVEMAN_DEPLOY:
		HandleDeployState();
		break;

	case SLAVEMAN_4:
		HandleDeployedState();
		break;

	case SLAVEMAN_FIND_ORE:
		HandleFindOreState();
		break;

	case SLAVEMAN_UNDEPLOY:
		HandleUndeployState();
		break;
	}
}

void Send_Slaves()
{
	for (int i = SlaveControls.ActiveCount - 1; i >= 0; --i)
	{
		SlaveControl* ctrl = SlaveControls.Vector_Item[i];
		InfantryClass* slave = ctrl->Slave;

		if (ctrl->State != SLAVECTRL_RESPAWNING)
			continue;

		CellStruct spawnCell = GetOwnerCell(Owner);

		if (IsAtLastOreLocation(spawnCell))
			continue;

		// Get spawn location
		CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &spawnCell);
		CoordStruct spawnCoord;
		cell->a.vftable->t.r.m.o.a.Center_Coord(cell, &spawnCoord);

		CoordStruct freeSpot;
		DisplayClass::Closest_Free_Spot(&Map.sc.t.sb.p.r.d, &freeSpot, &spawnCoord, 0);

		// Check if valid spawn location
		if (freeSpot.X == dword_B0B5D8 && freeSpot.Y == dword_B0B5DC && freeSpot.Z == dword_B0B5E0)
			continue;
		if (freeSpot.X == dword_B0B618 && freeSpot.Y == dword_B0B61C && freeSpot.Z == dword_B0B620)
			continue;

		// Spawn slave
		if (slave->f.t.r.m.o.a.vftable->t.r.m.o.Unlimbo(&slave->f.t.r.m.o, &freeSpot, 0))
		{
			CoordStruct ownerCoord;
			Owner->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(Owner, &ownerCoord);
			slave->f.t.r.m.o.a.vftable->t.r.m.o.Scatter(slave, &ownerCoord);
			ctrl->State = SLAVECTRL_HANDLE_ORE;
		}
	}
}

void AI()
{
	if (!IsTimerExpired(LogicDelayTimer))
		return;

	StartTimer(LogicDelayTimer, 10);

	if (Owner)
	{
		Slave_AI();
		Owner_AI();
	}
}

CellStruct* Where_To_Deploy(CellStruct* result, CellStruct targetCell)
{
	int width = 1;
	int height = 1;

	// Get owner dimensions
	if (IsOwnerBuilding(Owner))
	{
		BuildingClass* building = TechnoAsBuilding(Owner);
		if (building)
		{
			width = BuildingTypeClass::Width(building->Class);
			height = BuildingTypeClass::Height(building->Class, 0);
		}
	}
	else
	{
		TechnoTypeClass* technoType = Owner->r.m.o.a.vftable->t.r.m.o.Techno_Type_Class(Owner);
		if (technoType && technoType->DeploysInto)
		{
			width = BuildingTypeClass::Width(technoType->DeploysInto);
			height = BuildingTypeClass::Height(technoType->DeploysInto, 0);
		}
	}

	// Find nearby location
	CellStruct ownerCell;
	Owner->r.m.o.a.vftable->t.r.m.o.Coord_Cell(Owner, &ownerCell);

	int zone = MapClass_zone_56D230(&Map.sc.t.sb.p.r.d.m, &ownerCell, MZONE_NORMAL, 0);

	CellStruct deployCell;
	MapClass::Nearby_Location(
		&Map.sc.t.sb.p.r.d.m, &deployCell, &targetCell,
		SPEED_TRACK, zone, MZONE_NORMAL, 0,
		width, height, 1, 0, 0, 0, &targetCell, 0, 1
	);

	if (IsAtLastOreLocation(deployCell))
	{
		*result = LastOreLocation;
		return result;
	}

	// Adjust for larger buildings
	if (width > 2)
		deployCell.X += 1;
	if (height > 2)
		deployCell.Y += 1;

	*result = deployCell;
	return result;
}

bool To_Wake_Up()
{
	if (!Owner || State != SLAVEMAN_0)
		return false;

	if (!Owner->House->IsHuman)
		return true;

	CellClass* ownerCell = Owner->r.m.o.a.vftable->t.r.m.o.Coord_CellClass(Owner);
	if (ownerCell->Land == LAND_TIBERIUM)
		return true;

	if (LastScanFrame + Rule->SlaveMinerKickFrameDelay >= Frame)
		return false;

	CellStruct nearOre;
	Owner->r.m.o.a.vftable->t.Goto_Tiberium(
		Owner, &nearOre,
		Rule->SlaveMinerShortScan / 256
	);

	return !IsAtLastOreLocation(nearOre);
}

void Release_A_Slave(int index)
{
	SlaveControl* ctrl = SlaveControls.Vector_Item[index];
	ResetSlaveControl(ctrl, RegenRate);
}

void Recall_Slaves()
{
	RecallAllSlaves(this);
}

void Lose_Slave(InfantryClass* slave)
{
	for (int i = SlaveControls.ActiveCount - 1; i >= 0; --i)
	{
		SlaveControl* ctrl = SlaveControls.Vector_Item[i];
		if (ctrl->Slave == slave)
		{
			ResetSlaveControl(ctrl, RegenRate);
			return;
		}
	}
}

void Idle()
{
	State = SLAVEMAN_0;
	LastScanFrame = Frame;
	RecallAllSlaves(this);
}

BOOL Found_More_Ore()
{
	CellStruct oreCell;
	Owner->r.m.o.a.vftable->t.Goto_Tiberium(
		Owner, &oreCell,
		Rule->SlaveMinerShortScan / 256
	);

	return !IsAtLastOreLocation(oreCell);
}

CellStruct* Find_More_Ore(CellStruct* result)
{
	Owner->r.m.o.a.vftable->t.Goto_Tiberium(
		Owner, result,
		Rule->SlaveMinerLongScan / 256
	);
	return result;
}

HouseClass* FindCivilianHouse()
{
	int civilianSide = Side_To_Index("Civilian");

	for (int i = 0; i < Houses.ActiveCount; ++i)
	{
		if (Houses.Vector_Item[i]->Class->Side == civilianSide)
		{
			return Houses.Vector_Item[i];
		}
	}

	return nullptr;
}

void FreeSingleSlave(InfantryClass* slave, TechnoClass* killer, HouseClass* newOwner, HouseClass* civilianHouse)
{
	if (!slave || !GameActive)
		return;

	slave->f.t.__EnslavedBy = nullptr;

	if (slave->f.t.r.m.o.IsInLimbo)
	{
		slave->f.t.r.m.o.a.vftable->t.r.m.o.__BeingManipulatedBy_Record_The_Kill1(slave, killer);
		slave->f.t.r.m.o.a.vftable->t.r.m.o.Remove_This_deletethis(slave);
	}
	else
	{
		HouseClass* targetHouse = nullptr;

		if (killer)
			targetHouse = killer->House;
		else if (newOwner)
			targetHouse = newOwner;
		else if (civilianHouse)
			targetHouse = civilianHouse;

		if (targetHouse)
		{
			slave->f.t.r.m.o.a.vftable->t.Captured(slave, targetHouse, 1);
			slave->f.t.r.m.o.a.vftable->t.msub_70F850(slave);
			slave->f.t.r.m.o.a.vftable->t.Chear(slave, 1);
		}
		else
		{
			// Kill the slave if no valid house
			int damage = slave->f.t.r.m.o.Strength;
			slave->f.t.r.m.o.a.vftable->t.r.m.o.Take_Damage(
				&slave->f.t.r.m.o, &damage, 0,
				Rule->C4Warhead, 0, 0, 0, 0
			);
		}
	}
}

void Free_Slaves(TechnoClass* killer, HouseClass* newOwner)
{
	if (!Owner)
		return;

	HouseClass* civilianHouse = FindCivilianHouse();
	InfantryClass* firstFreedSlave = nullptr;

	for (int i = SlaveControls.ActiveCount - 1; i >= 0; --i)
	{
		InfantryClass* slave = SlaveControls.Vector_Item[i]->Slave;

		if (slave)
		{
			FreeSingleSlave(slave, killer, newOwner, civilianHouse);

			if (!firstFreedSlave)
				firstFreedSlave = slave;
		}
	}

	// Play sound effect
	if (GameActive && firstFreedSlave && !ScenarioInit && Rule->SlavesFreeSound != -1)
	{
		CoordStruct soundPos = firstFreedSlave->f.t.r.m.o.Coord;
		VocClass::Play_Ranged(Rule->SlavesFreeSound, &soundPos, 0);
	}

	Owner = nullptr;
}

void Harvest()
{
	UnitClass* unit = TechnoAsUnit(Owner);

	// If unit is moving to target
	if (unit && unit->f.NavCom)
	{
		// Get navigation target
		CoordStruct navCoord;
		unit->f.NavCom->vftable->t.r.m.o.a.Center_Coord(unit->f.NavCom, &navCoord);

		CellStruct navCell;
		navCell.X = navCoord.X / 256;
		navCell.Y = navCoord.Y / 256;

		CellStruct deployCell;
		Where_To_Deploy(&deployCell, navCell);

		if (IsAtLastOreLocation(deployCell))
		{
			// No valid ore location
			State = SLAVEMAN_0;
			LastScanFrame = Frame;
			Owner->r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(
				Owner, MISSION_GUARD, 0
			);
		}
		else
		{
			// Continue moving
			AssignMoveToCell(Owner, deployCell);
			State = SLAVEMAN_MOVE;
			LastScanFrame = 0x7FFFFFFF;
			RecallAllSlaves(this);
		}
	}
	// If building with archive target
	else if (IsOwnerBuilding(Owner) && Owner->ArchiveTarget)
	{
		CoordStruct targetCoord;
		Owner->ArchiveTarget->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(
			Owner->ArchiveTarget, &targetCoord
		);

		CellStruct targetCell;
		targetCell.X = targetCoord.X / 256;
		targetCell.Y = targetCoord.Y / 256;

		CellStruct deployCell;
		Where_To_Deploy(&deployCell, targetCell);

		if (!IsAtLastOreLocation(deployCell))
		{
			CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &deployCell);
			TechnoClass::Set_Archive(Owner, cell);

			State = SLAVEMAN_UNDEPLOY;
			LastScanFrame = 0x7FFFFFFF;
			RecallAllSlaves(this);
		}
	}
	else
	{
		// Default to guard
		Owner->r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(
			Owner, MISSION_GUARD, 0
		);
		State = SLAVEMAN_0;
		LastScanFrame = Frame;
	}
}

void Guard()
{
	if (State == SLAVEMAN_0)
	{
		State = SLAVEMAN_SCAN;
		LastScanFrame = 0x7FFFFFFF;
		RecallAllSlaves(this);
	}
}

void Deploy1()
{
	if (State == SLAVEMAN_0)
	{
		State = SLAVEMAN_4;
		LastScanFrame = 0x7FFFFFFF;
		RecallAllSlaves(this);
	}
}

void Deploy2()
{
	if (State == SLAVEMAN_0)
	{
		State = SLAVEMAN_4;
		LastScanFrame = 0x7FFFFFFF;
		RecallAllSlaves(this);
	}
}

void Enslave(TechnoClass* newOwner)
{
	// Clear old relationship
	if (Owner)
	{
		Owner->__SlaveManager = nullptr;
	}

	// Free existing slaves of new owner
	if (newOwner->__SlaveManager)
	{
		newOwner->__SlaveManager->Free_Slaves(nullptr, nullptr);

		if (newOwner->__SlaveManager)
		{
			newOwner->__SlaveManager->a.vftable->t.r.m.o.a.SDTOR(
				newOwner->__SlaveManager, 1
			);
		}
	}

	// Establish new relationship
	Owner = newOwner;
	newOwner->__SlaveManager = this;

	// Update all slaves
	for (int i = SlaveControls.ActiveCount - 1; i >= 0; --i)
	{
		InfantryClass* slave = SlaveControls.Vector_Item[i]->Slave;
		if (slave)
		{
			slave->f.t.__EnslavedBy = newOwner;
		}
	}
}

void Create_Slave(SlaveControl* ctrl)
{
	HouseClass* house = Owner->r.m.o.a.vftable->t.r.m.o.a.Owner__Owning_House(Owner);

	InfantryClass* slave = SlaveType->tt.ot.at.a.vftable->ot.Create_One_Of(
		&SlaveType->tt.ot, house
	);

	if (slave)
	{
		ctrl->Slave = slave;
		slave->f.t.r.m.o.a.vftable->t.r.m.o.Limbo(&slave->f.t.r.m.o);
		slave->f.t.__EnslavedBy = Owner;

		ctrl->State = SLAVECTRL_RESPAWNING;
		StartTimer(ctrl->RespawnTimer, 0);
	}
}