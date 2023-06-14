#include "Meteor.h"

/**
 *  Creates a meteor shower around the current mouse cell.
 *
 *  @author: CCHyper
 */
const char* MeteorShowerCommandClass::Get_Name() const
{
	return "MeteorShower";
}

const char* MeteorShowerCommandClass::Get_UI_Name() const
{
	return "Meteor Shower";
}

const char* MeteorShowerCommandClass::Get_Category() const
{
	return CATEGORY_DEVELOPER;
}

const char* MeteorShowerCommandClass::Get_Description() const
{
	return "Creates a meteor shower around the current mouse cell.";
}




/**
 *  Sends a meteor at the current mouse cell.
 *
 *  @author: CCHyper
 */
const char* MeteorImpactCommandClass::Get_Name() const
{
	return "MeteorImpact";
}

const char* MeteorImpactCommandClass::Get_UI_Name() const
{
	return "Meteor Impact";
}

const char* MeteorImpactCommandClass::Get_Category() const
{
	return CATEGORY_DEVELOPER;
}

const char* MeteorImpactCommandClass::Get_Description() const
{
	return "Sends a meteor at the current mouse cell.";
}

bool MeteorImpactCommandClass::Process()
{
	if (!Session.Singleplayer_Game())
	{
		return false;
	}

	Coordinate mouse_coord = Get_Coord_Under_Mouse();
	mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

	const CellClass* cellptr = &Map[mouse_coord];
	if (!cellptr)
	{
		return false;
	}

	/**
	 *  Pick a random a random meteor object.
	 */
	const VoxelAnimTypeClass* voxelanimtypeptr = 
	if (!voxelanimtypeptr)
	{
		return false;
	}

	new VoxelAnimClass(voxelanimtypeptr, mouse_coord);

	return true;
}