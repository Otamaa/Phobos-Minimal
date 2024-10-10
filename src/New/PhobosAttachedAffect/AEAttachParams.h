#pragma once

#include <Utilities/SavegameDef.h>

// Container for AttachEffect attachment for an individual effect passed to AE attach function.
struct AEAttachParams
{
	int DurationOverride { 0 };
	int Delay { 0 };
	int InitialDelay { 0 };
	int RecreationDelay { -1 };
	bool CumulativeRefreshAll { false };
	bool CumulativeRefreshAll_OnAttach { false };
	bool CumulativeRefreshSameSourceOnly { true };

public :

	bool Load(PhobosStreamReader& stm, bool registerForChange) { return this->Serialize(stm); }
	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<AEAttachParams*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->DurationOverride)
			.Process(this->Delay)
			.Process(this->InitialDelay)
			.Process(this->RecreationDelay)
			.Process(this->CumulativeRefreshAll)
			.Process(this->CumulativeRefreshAll_OnAttach)
			.Process(this->CumulativeRefreshSameSourceOnly)

			.Success();
	}
};