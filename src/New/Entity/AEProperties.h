#pragma once

#include <New/Entity/AEExtraRange.h>
#include <New/Entity/AEExtraCrit.h>
#include <New/Entity/AEArmorMults.h>
#include <New/Entity/AEFlags.h>

class TechnoClass;
class PhobosAttachEffectClass;
struct AEProperties
{
	// Transient — rebuilt every Recalculate(), not serialized.
	AEExtraRange ExtraRange {};
	AEExtraCrit ExtraCrit {};
	AEArmorMults ArmorMultData {};

	// Persistent — crate multipliers survive save/load.
	double Crate_FirepowerMultiplier { 1.0 };
	double Crate_ArmorMultiplier { 1.0 };
	double Crate_SpeedMultiplier { 1.0 };
	double ROFMultiplier { 1.0 };
	double ReceiveRelativeDamageMult { 1.0 };

	AEFlags flags;

public:

	static void Recalculate(TechnoClass* pTechno);
	// ActuallyNeedRecalc is used to override function recalc handler , it will send signal to the outside variable to do reset , just after everything is done
	// so it wont repeatedly calling the mark redraw function
	static void RecalculateSingle(TechnoClass* pTechno, PhobosAttachEffectClass* pAE, bool* forceDecloakResult, bool* ActuallyNeedRecalc, bool recalc);

public:

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AEProperties*>(this)->Serialize(Stm);
	}

protected:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Crate_FirepowerMultiplier)
			.Process(this->Crate_ArmorMultiplier)
			.Process(this->Crate_SpeedMultiplier)
			.Process(this->ROFMultiplier)
			.Process(this->ReceiveRelativeDamageMult)
			.Process(this->flags)
			.Process(this->ExtraRange)
			.Process(this->ExtraCrit)
			.Process(this->ArmorMultData)
			.Success() && Stm.RegisterChange(this)
			;
	}
};
