#include "AresAttachEffectTypeClass.h"

bool AresAttachEffectTypeClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(this->Owner)
		.Process(this->Duration)
		.Process(this->Cumulative)
		.Process(this->ForceDecloak)
		.Process(this->DiscardOnEntry)
		.Process(this->AnimType)
		.Process(this->AnimResetOnReapply)
		.Process(this->TemporalHidesAnim)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
		.Process(this->SpeedMultiplier)
		.Process(this->ROFMultiplier)
		.Process(this->Cloakable)
		.Process(this->Delay)
		.Success()
		&& Stm.RegisterChange(this); // announce this type
}

bool AresAttachEffectTypeClass::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(this->Owner)
		.Process(this->Duration)
		.Process(this->Cumulative)
		.Process(this->ForceDecloak)
		.Process(this->DiscardOnEntry)
		.Process(this->AnimType)
		.Process(this->AnimResetOnReapply)
		.Process(this->TemporalHidesAnim)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
		.Process(this->SpeedMultiplier)
		.Process(this->ROFMultiplier)
		.Process(this->Cloakable)
		.Process(this->Delay)
		.Success()
		&& Stm.RegisterChange(this);
}

void AresAttachEffectTypeClass::Read(INI_EX& exINI)
{
	auto const pSection = this->Owner->ID;
	this->Duration.Read(exINI, pSection, "AttachEffect.Duration");
	this->Cumulative.Read(exINI, pSection, "AttachEffect.Cumulative");
	this->AnimType.Read(exINI, pSection, "AttachEffect.Animation", true);
	this->AnimResetOnReapply.Read(exINI, pSection, "AttachEffect.AnimResetOnReapply");
	this->TemporalHidesAnim.Read(exINI, pSection, "AttachEffect.TemporalHidesAnim");
	this->ForceDecloak.Read(exINI, pSection, "AttachEffect.ForceDecloak");
	this->DiscardOnEntry.Read(exINI, pSection, "AttachEffect.DiscardOnEntry");
	this->FirepowerMultiplier.Read(exINI, pSection, "AttachEffect.FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, pSection, "AttachEffect.ArmorMultiplier");
	this->SpeedMultiplier.Read(exINI, pSection, "AttachEffect.SpeedMultiplier");
	this->ROFMultiplier.Read(exINI, pSection, "AttachEffect.ROFMultiplier");
	this->Cloakable.Read(exINI, pSection, "AttachEffect.Cloakable");
	this->Delay.Read(exINI, pSection, "AttachEffect.Delay");
	this->InitialDelay.Read(exINI, pSection, "AttachEffect.InitialDelay");

	this->PenetratesIC.Read(exINI, pSection, "AttachEffect.PenetratesIronCurtain");
}