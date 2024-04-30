#include "AresAttachEffectTypeClass.h"
#include <Utilities/Debug.h>

bool AresAttachEffectTypeClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return this->Serialize(Stm);
}

bool AresAttachEffectTypeClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<AresAttachEffectTypeClass*>(this)->Serialize(Stm);
}

void AresAttachEffectTypeClass::Read(INI_EX& exINI)
{
	if (!this->Owner){
		Debug::FatalErrorAndExit("AttahedAffectType Is Missing OwnerPointer!\n");
	}

	auto const pSection = this->Owner->ID;
	this->Duration.Read(exINI, pSection, "AttachEffect.Duration");
	this->Cumulative.Read(exINI, pSection, "AttachEffect.Cumulative");
	this->AnimType.Read(exINI, pSection, "AttachEffect.Animation");
	this->AnimResetOnReapply.Read(exINI, pSection, "AttachEffect.AnimResetOnReapply");
	this->TemporalHidesAnim.Read(exINI, pSection, "AttachEffect.TemporalHidesAnim");
	this->ForceDecloak.Read(exINI, pSection, "AttachEffect.ForceDecloak");
	this->DiscardOnEntry.Read(exINI, pSection, "AttachEffect.DiscardOnEntry");
	this->FirepowerMultiplier.Read(exINI, pSection, "AttachEffect.FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, pSection, "AttachEffect.ArmorMultiplier");
	this->SpeedMultiplier.Read(exINI, pSection, "AttachEffect.SpeedMultiplier");
	this->ROFMultiplier.Read(exINI, pSection, "AttachEffect.ROFMultiplier");
	this->ReceiveRelativeDamageMult.Read(exINI, pSection, "AttachEffect.ReceiveRelativeDamageMultiplier");
	this->Cloakable.Read(exINI, pSection, "AttachEffect.Cloakable");
	this->Delay.Read(exINI, pSection, "AttachEffect.Delay");
	this->InitialDelay.Read(exINI, pSection, "AttachEffect.InitialDelay");

	this->PenetratesIC.Read(exINI, pSection, "AttachEffect.PenetratesIronCurtain");
	this->DisableSelfHeal.Read(exINI, pSection, "AttachEffect.DisableSelfHeal");
	this->DisableWeapons.Read(exINI, pSection, "AttachEffect.DisableWeapons");
	this->Untrackable.Read(exINI, pSection, "AttachEffect.Untrackable");

	this->WeaponRange_Multiplier.Read(exINI, pSection, "WeaponRange.Multiplier");
	this->WeaponRange_ExtraRange.Read(exINI, pSection, "WeaponRange.ExtraRange");
	this->WeaponRange_AllowWeapons.Read(exINI, pSection, "WeaponRange.AllowWeapons");
	this->WeaponRange_DisallowWeapons.Read(exINI, pSection, "WeaponRange.DisallowWeapons");
}