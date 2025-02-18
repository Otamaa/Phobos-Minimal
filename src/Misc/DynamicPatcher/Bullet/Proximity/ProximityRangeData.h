#pragma once

#include <Utilities/TemplateDef.h>

//for Weapon
struct ProximityRangeData
{
	Valueable<int> Range;
	bool Random;
	int  MaxRange;
	int MinRange;

	ProximityRangeData(int range)
		: Range { range }
		, Random { false }
		, MinRange { 0 }
		, MaxRange { range }
	{ }

	ProximityRangeData()
		: Range { 0 }
		, Random { false }
		, MinRange { 0 }
		, MaxRange { 0 }
	{ }

	~ProximityRangeData() = default;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::LogInfo("Loading Element From ProximityRangeData ! ");  return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

	void Read(INI_EX& parser, const char* pSection, bool Allocate = false)
	{
		Nullable<Point2D> Buffer_randomRead;
		Range.Read(parser, pSection, "ProximityRange");
		Range = Range * 256;
		Buffer_randomRead.Read(parser, pSection, "Proximity.Random");

		if (Buffer_randomRead.isset())
		{
			Random = true;
			MaxRange = Buffer_randomRead.Get().X * 256;
			MinRange = Buffer_randomRead.Get().Y * 256;

			if (MinRange > MaxRange)
				std::swap(MinRange, MaxRange);

		}
	}

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Range)
			.Process(Random)
			.Process(MaxRange)
			.Process(MinRange)
			.Success()
			;
	}
};