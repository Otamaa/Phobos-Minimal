#include "LaserTrailDataEntry.h"

#include <Utilities/SavegameDef.h>

bool LaserTrailDataEntry::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool LaserTrailDataEntry::Save(PhobosStreamWriter& stm) const
{
	return const_cast<LaserTrailDataEntry*>(this)->Serialize(stm);
}

// For some Fcking unknown reason `emplace_back` doesnt knowh the default contructor for this
LaserTrailDataEntry::LaserTrailDataEntry(int nIdx, const CoordStruct& nFlh, bool OnTur) :
	idxType { nIdx }
	, FLH { nFlh }
	, IsOnTurret { OnTur }
{}

LaserTrailDataEntry::LaserTrailDataEntry() :
	idxType { -1 }
	, FLH { 0,0,0 }
	, IsOnTurret { false }
{}

template <typename T>
bool LaserTrailDataEntry::Serialize(T& stm)
{
	return stm
		.Process(idxType)
		.Process(FLH)
		.Process(IsOnTurret)
		.Success();
}