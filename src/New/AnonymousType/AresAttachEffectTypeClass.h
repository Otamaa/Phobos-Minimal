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
	Valueable<bool> Cloakable { false };

	//#1623-only tags
	Valueable<int> Delay { 0 };
	Valueable<int> InitialDelay { 0 };


	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	bool Save(PhobosStreamWriter& Stm) const;

	AresAttachEffectTypeClass(const AbstractTypeClass* pOwner) :
		Owner { pOwner }
	{ }

	~AresAttachEffectTypeClass() noexcept = default;

	AresAttachEffectTypeClass(const AresAttachEffectTypeClass& other) = default;
	AresAttachEffectTypeClass& operator=(const AresAttachEffectTypeClass& other) = default;

	void Read(INI_EX& exINI);
};