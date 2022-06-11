#pragma once

#include <CoordStruct.h>
#include <GeneralStructures.h>

struct LocationMark
{
	CoordStruct Location;
	DirStruct Direction;

	LocationMark(CoordStruct location, DirStruct direction) :
		Location { location }
		, Direction { direction }
	{ }

	LocationMark() :
		Location { CoordStruct::Empty }
		, Direction {}
	{ }

	template <typename T>
	void Serialize(T & Stm)
	{
		Stm
			.Process(Location)
			.Process(Direction)
			;
	}
};
