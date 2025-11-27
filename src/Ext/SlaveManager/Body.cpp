#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Utilities/Macro.h>

// Helper functions - these work on parameters, not members
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

// ========================================================================
// MAIN WORKER FUNCTIONS - These now take SlaveManagerClass* parameter
// ========================================================================

CellStruct GetPrimaryExitCell(SlaveManagerClass* manager)
{
	BuildingClass* building = TechnoAsBuilding(manager->Owner);
	if (!building)
	{
		CellStruct result;
		manager->Owner->r.m.o.a.vftable->t.r.m.o.Coord_Cell(manager->Owner, &result);
		return result;
	}

	return GetBuildingExitCell(building);
}

CellStruct GetSecondaryExitCell(SlaveManagerClass* manager)
{
	if (!IsOwnerBuilding(manager->Owner))
	{
		return CellStruct {}; // Return zero-initialized cell
	}

	CellStruct primaryCell = GetPrimaryExitCell(manager);

	CellStruct secondaryCell;
	secondaryCell.X = primaryCell.X;
	secondaryCell.Y = primaryCell.Y - 1;

	CellClass* cell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &secondaryCell);

	if (CellClass::Cell_Building(cell) == TechnoAsBuilding(manager->Owner))
	{
		return secondaryCell;
	}

	return CellStruct {}; // Return zero-initialized cell
}

bool IsAtLastOreLocation(SlaveManagerClass* manager, const CellStruct& cell)
{
	// Assuming LastOreLocation is a static or global sentinel value
	static const CellStruct LastOreLocation = { -1, -1 }; // Adjust as needed
	return CellsEqual(cell, LastOreLocation);
}

bool IsInfantryInSlaveControl(SlaveManagerClass* manager, InfantryClass* infantry)
{
	for (int i = manager->SlaveControls.ActiveCount - 1; i >= 0; --i)
	{
		if (manager->SlaveControls.Vector_Item[i]->Slave == infantry)
		{
			return true;
		}
	}

	return false;
}

void HandleOreSearch(SlaveManagerClass* manager, SlaveManagerClass::SlaveControl* ctrl)
{
	InfantryClass* slave = ctrl->Slave;
	CellStruct oreCell;

	slave->f.t.r.m.o.a.vftable->t.Goto_Tiberium(
		slave, &oreCell,
		Rule->SlaveMinerSlaveScan / 256, 0
	);

	if (IsAtLastOreLocation(manager, oreCell))
	{
		TechnoClass::Set_Archive(&slave->f.t, nullptr);
		CellStruct targetCell = GetOwnerCell(manager->Owner);
		AssignMoveToCell(&slave->f.t, targetCell);
		ctrl->State = SLAVECTRL_UNLOAD_ORE;
	}
	else
	{
		AssignMoveToCell(&slave->f.t, oreCell);
		ctrl->State = SLAVECTRL_MOVE_TO_ORE;
	}
}

void HandleMoveToOre(SlaveManagerClass* manager, SlaveManagerClass::SlaveControl* ctrl)
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

void HandleHarvest(SlaveManagerClass* manager, SlaveManagerClass::SlaveControl* ctrl)
{
	InfantryClass* slave = ctrl->Slave;

	if (InfantryClass::Has_Tiberium_Load(slave))
	{
		CellClass* currentCell = slave->f.t.r.m.o.a.vftable->t.r.m.o.Coord_CellClass(slave);
		TechnoClass::Set_Archive(&slave->f.t, currentCell);

		CellStruct targetCell = GetOwnerCell(manager->Owner);
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

bool ShouldResetPath(SlaveManagerClass* manager, InfantryClass* slave)
{
	if (!slave->f.NavCom)
		return false;

	CellStruct targetCell = GetOwnerCell(manager->Owner);

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

void HandleUnloadOre(SlaveManagerClass* manager, SlaveManagerClass::SlaveControl* ctrl)
{
	InfantryClass* slave = ctrl->Slave;
	CellStruct targetCell = GetOwnerCell(manager->Owner);

	bool shouldReset = ShouldResetPath(manager, slave);

	CellStruct slaveCell;
	slave->f.t.r.m.o.a.vftable->t.r.m.o.Coord_Cell(&slave->f.t, &slaveCell);

	if (CellsEqual(slaveCell, targetCell))
	{
		if (!slave->f.NavCom)
		{
			InfantryClass::Storage_AI(slave, manager->Owner);
			slave->f.t.r.m.o.a.vftable->t.r.m.o.Limbo(&slave->f.t.r.m.o);

			ctrl->State = SLAVECTRL_RESPAWN;
			StartTimer(ctrl->RespawnTimer, manager->ReloadRate);
			return;
		}
	}

	if (!slave->f.NavCom || shouldReset)
	{
		AssignMoveToCell(&slave->f.t, targetCell);
	}
}

void HandleRespawn(SlaveManagerClass* manager, SlaveManagerClass::SlaveControl* ctrl)
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

void HandleDead(SlaveManagerClass* manager, SlaveManagerClass::SlaveControl* ctrl)
{
	if (IsTimerExpired(ctrl->RespawnTimer))
	{
		Create_Slave(manager, ctrl);
	}
}

void Slave_AI(SlaveManagerClass* manager)
{
	for (int i = 0; i < manager->SlaveControls.ActiveCount; ++i)
	{
		SlaveManagerClass::SlaveControl* ctrl = manager->SlaveControls.Vector_Item[i];
		InfantryClass* slave = ctrl->Slave;

		if (!slave && ctrl->State != SLAVECTRL_DEAD)
		{
			ctrl->Slave = nullptr;
			ctrl->State = SLAVECTRL_DEAD;
			StartTimer(ctrl->RespawnTimer, manager->RegenRate);
		}

		switch (ctrl->State)
		{
		case SLAVECTRL_HANDLE_ORE:
			HandleOreSearch(manager, ctrl);
			break;

		case SLAVECTRL_MOVE_TO_ORE:
			HandleMoveToOre(manager, ctrl);
			break;

		case SLAVECTRL_HARVEST:
			HandleHarvest(manager, ctrl);
			break;

		case SLAVECTRL_UNLOAD_ORE:
			HandleUnloadOre(manager, ctrl);
			break;

		case SLAVECTRL_RESPAWN:
			HandleRespawn(manager, ctrl);
			break;

		case SLAVECTRL_DEAD:
			HandleDead(manager, ctrl);
			break;
		}
	}
}

// Continue with remaining functions following the same pattern...
// (Add manager parameter to ALL remaining functions)

void Create_Slave(SlaveManagerClass* manager, SlaveManagerClass::SlaveControl* ctrl)
{
	HouseClass* house = manager->Owner->r.m.o.a.vftable->t.r.m.o.a.Owner__Owning_House(manager->Owner);

	InfantryClass* slave = manager->SlaveType->tt.ot.at.a.vftable->ot.Create_One_Of(
		&manager->SlaveType->tt.ot, house
	);

	if (slave)
	{
		ctrl->Slave = slave;
		slave->f.t.r.m.o.a.vftable->t.r.m.o.Limbo(&slave->f.t.r.m.o);
		slave->f.t.__EnslavedBy = manager->Owner;

		ctrl->State = SLAVECTRL_RESPAWNING;
		StartTimer(ctrl->RespawnTimer, 0);
	}
}