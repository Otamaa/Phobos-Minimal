#include "AEAttachParams.h"

#include <Utilities/SavegameDef.h>

bool AEAttachParams::Load(PhobosStreamReader& stm, bool registerForChange) 
{ return this->Serialize(stm); }

bool AEAttachParams::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AEAttachParams*>(this)->Serialize(stm);
}

template <typename T>
bool AEAttachParams::Serialize(T& stm)
{
	return stm
		.Process(this->DurationOverride)
		.Process(this->Delay)
		.Process(this->InitialDelay)
		.Process(this->RecreationDelay)
		.Process(this->CumulativeSourceMaxCount)
		.Process(this->CumulativeRefreshAll)
		.Process(this->CumulativeRefreshAll_OnAttach)
		.Process(this->CumulativeRefreshSameSourceOnly)

		.Success();
}