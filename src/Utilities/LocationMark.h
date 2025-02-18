#pragma once
#include <Utilities/TemplateDef.h>

struct OffsetData
{
	CoordStruct Offset;
	bool IsOnWorld;
	int Dir;
	bool IsOnturret;
};

struct LocationMark
{
	CoordStruct Location;
	DirStruct Direction;

	COMPILETIMEEVAL LocationMark(CoordStruct location, DirStruct direction) :
		Location { location }
		, Direction { direction }
	{ }

	COMPILETIMEEVAL LocationMark(DirStruct direction , CoordStruct location) :
		Location { location }
		, Direction { direction }
	{ }

	COMPILETIMEEVAL LocationMark() :
		Location { 0,0,0 }
		, Direction { }
	{ }

	COMPILETIMEEVAL LocationMark(const LocationMark& other) = default;
	COMPILETIMEEVAL LocationMark& operator=(const LocationMark& other) = default;
	COMPILETIMEEVAL ~LocationMark() = default;

	COMPILETIMEEVAL OPTIONALINLINE bool IsValid() const
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
		Debug::LogInfo("Processing Element From LocationMark ! ");
		return Stm
			.Process(Location)
			.Process(Direction)
			.Success()
			;
	}
};
