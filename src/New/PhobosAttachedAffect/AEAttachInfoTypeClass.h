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
	Valueable<int> CumulativeSourceMaxCount {};
	ValueableVector<int> DurationOverrides {};
	ValueableVector<int> Delays {};
	ValueableVector<int> InitialDelays {};
	NullableVector<int> RecreationDelays {};
	Nullable<bool> ReplaceLongerDuration {};
public:

	void LoadFromINI(CCINIClass* pINI, const char* pSection);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	AEAttachParams GetAttachParams(unsigned int index, bool selfOwned) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};