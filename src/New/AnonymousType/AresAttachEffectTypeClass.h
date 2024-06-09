#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/TemplateDef.h>

class AresAttachEffectTypeClass
{
public:

	const AbstractTypeClass* Owner { nullptr };

	Valueable<int> Duration { 0 };
	Valueable<bool> Cumulative { false };
	Valueable<bool> ForceDecloak { false };
	Valueable<bool> DiscardOnEntry { false };
	Valueable<bool> PenetratesIC { false };

	//#1573, #1623 animations on units
	Valueable<AnimTypeClass*> AnimType { nullptr };
	Valueable<bool> AnimResetOnReapply { false };
	Valueable<bool> TemporalHidesAnim { false };

	//#255, crate stat modifiers on weapons
	Valueable<double> FirepowerMultiplier { 1.0 };
	Valueable<double> ArmorMultiplier { 1.0 };
	Valueable<double> SpeedMultiplier { 1.0 };
	Valueable<double> ROFMultiplier { 1.0 };
	Valueable<double> ReceiveRelativeDamageMult { 1.0 };
	Valueable<bool> Cloakable { false };

	//#1623-only tags
	Valueable<int> Delay { 0 };
	Valueable<int> InitialDelay { 0 };

	Valueable<bool> DisableWeapons { false };
	Valueable<bool> DisableSelfHeal { false };
	Valueable<bool> Untrackable { false };

	Valueable<double> WeaponRange_Multiplier { 1.0 };
	Valueable<double> WeaponRange_ExtraRange { 0.0 };
	ValueableVector<WeaponTypeClass*> WeaponRange_AllowWeapons {};
	ValueableVector<WeaponTypeClass*> WeaponRange_DisallowWeapons {};

	Valueable<bool> ROFMultiplier_ApplyOnCurrentTimer { false };

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	bool Save(PhobosStreamWriter& Stm) const;

	AresAttachEffectTypeClass() = default;
	AresAttachEffectTypeClass(const AbstractTypeClass* pOwner) { Owner = pOwner;  }
	~AresAttachEffectTypeClass() noexcept = default;

	AresAttachEffectTypeClass(const AresAttachEffectTypeClass& other) = default;
	AresAttachEffectTypeClass& operator=(const AresAttachEffectTypeClass& other) = default;

	void Read(INI_EX& exINI);

private:

	template <typename T>
	bool Serialize(T& Stm)
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
			.Process(this->ReceiveRelativeDamageMult)
			.Process(this->Cloakable)
			.Process(this->Delay)
			.Process(this->InitialDelay)
			.Process(this->DisableWeapons)
			.Process(this->PenetratesIC)
			.Process(this->DisableSelfHeal)
			.Process(this->Untrackable)
			.Process(this->WeaponRange_Multiplier)
			.Process(this->WeaponRange_ExtraRange)
			.Process(this->WeaponRange_AllowWeapons)
			.Process(this->WeaponRange_DisallowWeapons)
			.Process(this->ROFMultiplier_ApplyOnCurrentTimer)
			.Success()
			&& Stm.RegisterChange(this); // announce this type
	}
};