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
		Debug::FatalErrorAndExit("AttahedAffectType Is Missing OwnerPointer!");
	}

	const std::string initial = "AttachEffect.";

	auto const pSection = this->Owner->ID;
	this->Duration.Read(exINI, pSection, (initial + "Duration").c_str());
	// cumulative no : will override the existing duration
	// cumulative yes : will add new AE object onto the techno vector
	this->Cumulative.Read(exINI, pSection, (initial + "Cumulative").c_str());
	this->AnimType.Read(exINI, pSection, (initial + "Animation").c_str(), true);
	this->AnimResetOnReapply.Read(exINI, pSection, (initial + "AnimResetOnReapply").c_str());
	this->TemporalHidesAnim.Read(exINI, pSection, (initial + "TemporalHidesAnim").c_str());
	this->ForceDecloak.Read(exINI, pSection, (initial + "ForceDecloak").c_str());
	this->DiscardOnEntry.Read(exINI, pSection, (initial + "DiscardOnEntry").c_str());
	this->FirepowerMultiplier.Read(exINI, pSection, (initial + "FirepowerMultiplier").c_str());
	this->ArmorMultiplier.Read(exINI, pSection, (initial + "ArmorMultiplier").c_str());
	this->SpeedMultiplier.Read(exINI, pSection, (initial + "SpeedMultiplier").c_str());
	this->ROFMultiplier.Read(exINI, pSection, (initial + "ROFMultiplier").c_str());
	this->ReceiveRelativeDamageMult.Read(exINI, pSection, (initial + "ReceiveRelativeDamageMultiplier").c_str());
	this->Cloakable.Read(exINI, pSection, (initial + "Cloakable").c_str());
	this->Delay.Read(exINI, pSection, (initial + "Delay").c_str());
	this->InitialDelay.Read(exINI, pSection, (initial + "InitialDelay").c_str());

	this->PenetratesIC.Read(exINI, pSection, (initial + "PenetratesIronCurtain").c_str());
	this->DisableSelfHeal.Read(exINI, pSection, (initial + "DisableSelfHeal").c_str());
	this->DisableWeapons.Read(exINI, pSection, (initial + "DisableWeapons").c_str());
	this->Untrackable.Read(exINI, pSection, (initial + "Untrackable").c_str());

	this->WeaponRange_Multiplier.Read(exINI, pSection, (initial + "WeaponRange.Multiplier").c_str());
	this->WeaponRange_ExtraRange.Read(exINI, pSection, (initial + "WeaponRange.ExtraRange").c_str());
	this->WeaponRange_AllowWeapons.Read(exINI, pSection, (initial + "WeaponRange.AllowWeapons").c_str());
	this->WeaponRange_DisallowWeapons.Read(exINI, pSection, (initial + "WeaponRange.DisallowWeapons").c_str());

	this->ROFMultiplier_ApplyOnCurrentTimer.Read(exINI, pSection, (initial + "ROFMultiplier.ApplyOnCurrentTimer").c_str());

	this->DisableRadar.Read(exINI, pSection, (initial + "DisableRadar").c_str());
	this->DisableSpySat.Read(exINI, pSection, (initial + "DisableSpySat").c_str());

	this->Unkillable.Read(exINI, pSection, (initial + "Unkillable").c_str());
}