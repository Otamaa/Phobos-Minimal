#include "InsigniaData.h"

bool InsigniaData::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool InsigniaData::Save(PhobosStreamWriter& stm) const
{
	return const_cast<InsigniaData*>(this)->Serialize(stm);
}

template <typename T>
bool InsigniaData::Serialize(T& stm)
{
	return stm
		.Process(this->Shapes)
		.Process(this->Frame)
		.Process(this->Frames)
		.Success();
}