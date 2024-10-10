#pragma once

#include <Utilities/TemplateDefB.h>
#include <Utilities/SavegameDef.h>

#include "AEAttachParams.h"

class PhobosAttachEffectTypeClass;
// Container for AttachEffect attachment info parsed from INI.
class AEAttachInfoTypeClass
{
public:
	ValueableVector<PhobosAttachEffectTypeClass*> AttachTypes {};
	Valueable<bool> CumulativeRefreshAll { false };
	Valueable<bool> CumulativeRefreshAll_OnAttach { false };
	Valueable<bool> CumulativeRefreshSameSourceOnly { true };
	ValueableVector<PhobosAttachEffectTypeClass*> RemoveTypes {};
	std::vector<std::string> RemoveGroups {};
	ValueableVector<int> CumulativeRemoveMinCounts {};
	ValueableVector<int> CumulativeRemoveMaxCounts {};
	ValueableVector<int> DurationOverrides {};
	ValueableVector<int> Delays {};
	ValueableVector<int> InitialDelays {};
	NullableVector<int> RecreationDelays {};

public:

	void LoadFromINI(CCINIClass* pINI, const char* pSection);

	bool Load(PhobosStreamReader& stm, bool registerForChange) { return this->Serialize(stm); }
	bool Save(PhobosStreamWriter& stm) const {
		return const_cast<AEAttachInfoTypeClass*>(this)->Serialize(stm);
	}

	constexpr AEAttachParams GetAttachParams(unsigned int index, bool selfOwned) const
	{
		AEAttachParams info { };
		if (!this->DurationOverrides.empty())
			info.DurationOverride = this->DurationOverrides[this->DurationOverrides.size() > index ? index : this->DurationOverrides.size() - 1];
		if (selfOwned)
		{
			if (!this->Delays.empty())
				info.Delay = this->Delays[this->Delays.size() > index ? index : this->Delays.size() - 1];
			if (!this->InitialDelays.empty())
				info.InitialDelay = this->InitialDelays[this->InitialDelays.size() > index ? index : this->InitialDelays.size() - 1];
			if (!this->RecreationDelays.empty())
				info.RecreationDelay = this->RecreationDelays[this->RecreationDelays.size() > index ? index : this->RecreationDelays.size() - 1];
		}
		else
		{
			info.CumulativeRefreshAll = this->CumulativeRefreshAll;
			info.CumulativeRefreshAll_OnAttach = this->CumulativeRefreshAll_OnAttach;
			info.CumulativeRefreshSameSourceOnly = this->CumulativeRefreshSameSourceOnly;
		}
		return info;
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->AttachTypes)
			.Process(this->CumulativeRefreshAll)
			.Process(this->CumulativeRefreshAll_OnAttach)
			.Process(this->CumulativeRefreshSameSourceOnly)
			.Process(this->RemoveTypes)
			.Process(this->RemoveGroups)
			.Process(this->CumulativeRemoveMinCounts)
			.Process(this->CumulativeRemoveMaxCounts)
			.Process(this->DurationOverrides)
			.Process(this->Delays)
			.Process(this->InitialDelays)
			.Process(this->RecreationDelays)
			.Success();
	}
};