#pragma once

#include <Utilities/TemplateDefB.h>

struct LaserTrailDataEntry
{
	int idxType;
	CoordStruct FLH;
	bool IsOnTurret;

	bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return this->Serialize(stm);
	}

	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<LaserTrailDataEntry*>(this)->Serialize(stm);
	}


	// For some Fcking unknown reason `emplace_back` doesnt knowh the default contructor for this
	LaserTrailDataEntry(int nIdx, const CoordStruct& nFlh, bool OnTur) :
		idxType { nIdx }
		, FLH { nFlh }
		, IsOnTurret { OnTur }
	{
	}

	LaserTrailDataEntry() :
		idxType { -1 }
		, FLH { 0,0,0 }
		, IsOnTurret { false }
	{
	}

	~LaserTrailDataEntry() = default;
	LaserTrailDataEntry(const LaserTrailDataEntry& other) = default;
	LaserTrailDataEntry& operator=(const LaserTrailDataEntry& other) = default;

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(idxType)
			.Process(FLH)
			.Process(IsOnTurret)
			.Success();
	}
};
