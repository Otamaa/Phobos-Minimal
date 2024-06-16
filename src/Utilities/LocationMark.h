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

	constexpr LocationMark(CoordStruct location, DirStruct direction) :
		Location { location }
		, Direction { direction }
	{ }

	constexpr LocationMark(DirStruct direction , CoordStruct location) :
		Location { location }
		, Direction { direction }
	{ }

	constexpr LocationMark() :
		Location { 0,0,0 }
		, Direction { }
	{ }

	constexpr LocationMark(const LocationMark& other) = default;
	constexpr LocationMark& operator=(const LocationMark& other) = default;
	constexpr ~LocationMark() = default;

	constexpr inline bool IsValid() const
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
