#pragma once
#include <Utilities/TemplateDef.h>

struct LocationMark
{
	CoordStruct Location;
	DirStruct Direction;

	LocationMark(CoordStruct location, DirStruct direction) :
		Location { Location }
		, Direction { Direction }
	{ }

	LocationMark() :
		Location { 0,0,0 }
		, Direction { }
	{ }

	__forceinline operator bool() const
	{
		return (Location != CoordStruct::Empty);
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From LocationMark ! \n");
		return Stm
			.Process(Location)
			.Process(Direction)
			.Success()
			;
	}
};
