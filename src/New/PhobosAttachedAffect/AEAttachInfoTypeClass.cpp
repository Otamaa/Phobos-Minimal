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