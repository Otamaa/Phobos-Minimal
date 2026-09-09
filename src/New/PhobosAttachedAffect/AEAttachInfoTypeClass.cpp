#include "AEAttachInfoTypeClass.h"

#include "PhobosAttachEffectTypeClass.h"

#include <Ext/Rules/Body.h>

// AEAttachInfoTypeClass
void AEAttachInfoTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	this->AttachTypes.Read(exINI, pSection, "AttachEffect.AttachTypes");
	this->CumulativeRefreshAll.Read(exINI, pSection, "AttachEffect.CumulativeRefreshAll");
	this->CumulativeRefreshAll_OnAttach.Read(exINI, pSection, "AttachEffect.CumulativeRefreshAll.OnAttach");
	this->CumulativeRefreshSameSourceOnly.Read(exINI, pSection, "AttachEffect.CumulativeRefreshSameSourceOnly");
	this->ReplaceLongerDuration.Read(exINI, pSection, "AttachEffect.ReplaceLongerDuration");
	this->RemoveTypes.Read(exINI, pSection, "AttachEffect.RemoveTypes");
	exINI.ParseList(this->RemoveGroups, pSection, "AttachEffect.RemoveGroups");
	this->CumulativeRemoveMinCounts.Read(exINI, pSection, "AttachEffect.CumulativeRemoveMinCounts");
	this->CumulativeRemoveMaxCounts.Read(exINI, pSection, "AttachEffect.CumulativeRemoveMaxCounts");
	this->CumulativeSourceMaxCount.Read(exINI, pSection, "AttachEffect.CumulativeSourceMaxCount");
	this->DurationOverrides.Read(exINI, pSection, "AttachEffect.DurationOverrides");
	this->Delays.Read(exINI, pSection, "AttachEffect.Delays");
	this->InitialDelays.Read(exINI, pSection, "AttachEffect.InitialDelays");
	this->RecreationDelays.Read(exINI, pSection, "AttachEffect.RecreationDelays");
}

AEAttachParams AEAttachInfoTypeClass::GetAttachParams(unsigned int index, bool hasDelay) const
{
	AEAttachParams info { };
	if (!this->DurationOverrides.empty())
		info.DurationOverride = this->DurationOverrides[this->DurationOverrides.size() > index ? index : this->DurationOverrides.size() - 1];
	if (hasDelay)
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
		info.CumulativeSourceMaxCount = this->CumulativeSourceMaxCount;
		info.CumulativeRefreshAll = this->CumulativeRefreshAll;
		info.CumulativeRefreshAll_OnAttach = this->CumulativeRefreshAll_OnAttach;
		info.CumulativeRefreshSameSourceOnly = this->CumulativeRefreshSameSourceOnly;
		info.ReplaceLongerDuration = this->ReplaceLongerDuration.Get(FakeRulesClass::Instance->AttachEffect_ReplaceLongerDuration);
		// set Delay to -1 so it can't be renew
		info.Delay = -1;
	}
	return info;
}

bool AEAttachInfoTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{ return this->Serialize(stm); }
bool AEAttachInfoTypeClass::Save(PhobosStreamWriter& stm) const
{ return const_cast<AEAttachInfoTypeClass*>(this)->Serialize(stm); }

template <typename T>
bool AEAttachInfoTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->AttachTypes)
		.Process(this->CumulativeRefreshAll)
		.Process(this->CumulativeRefreshAll_OnAttach)
		.Process(this->CumulativeRefreshSameSourceOnly)
		.Process(this->ReplaceLongerDuration)
		.Process(this->RemoveTypes)
		.Process(this->RemoveGroups)
		.Process(this->CumulativeRemoveMinCounts)
		.Process(this->CumulativeRemoveMaxCounts)
		.Process(this->CumulativeSourceMaxCount)
		.Process(this->ReplaceLongerDuration)
		.Process(this->DurationOverrides)
		.Process(this->Delays)
		.Process(this->InitialDelays)
		.Process(this->RecreationDelays)
		.Success();
}