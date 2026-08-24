#pragma once

#include <CoordStruct.h>

class PhobosStreamReader;
class PhobosStreamWriter;
struct LaserTrailDataEntry
{
	int idxType {};
	CoordStruct FLH {};
	bool IsOnTurret {};

public:

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

public:

	// For some Fcking unknown reason `emplace_back` doesnt knowh the default contructor for this
	LaserTrailDataEntry(int nIdx, const CoordStruct& nFlh, bool OnTur);
	LaserTrailDataEntry();
	~LaserTrailDataEntry() = default;
	LaserTrailDataEntry(const LaserTrailDataEntry& other) = default;
	LaserTrailDataEntry& operator=(const LaserTrailDataEntry& other) = default;

private:
	template <typename T>
	bool Serialize(T& stm);
};
