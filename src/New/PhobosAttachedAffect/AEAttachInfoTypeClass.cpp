#include "AEAttachInfoTypeClass.h"

#include "PhobosAttachEffectTypeClass.h"

// AEAttachInfoTypeClass
void AEAttachInfoTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);
	this->AttachTypes.Read(exINI, pSection, "AttachEffect.AttachTypes");
	this->CumulativeRefreshAll.Read(exINI, pSection, "AttachEffect.CumulativeRefreshAll");
	this->CumulativeRefreshAll_OnAttach.Read(exINI, pSection, "AttachEffect.CumulativeRefreshAll.OnAttach");
	this->CumulativeRefreshSameSourceOnly.Read(exINI, pSection, "AttachEffect.CumulativeRefreshSameSourceOnly");
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
		.Process(this->RemoveTypes)
		.Process(this->RemoveGroups)
		.Process(this->CumulativeRemoveMinCounts)
		.Process(this->CumulativeRemoveMaxCounts)
		.Process(this->CumulativeSourceMaxCount)
		.Process(this->DurationOverrides)
		.Process(this->Delays)
		.Process(this->InitialDelays)
		.Process(this->RecreationDelays)
		.Success();
}