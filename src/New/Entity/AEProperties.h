#pragma once

#include <New/Entity/AEExtraRange.h>
#include <New/Entity/AEExtraCrit.h>
#include <New/Entity/AEArmorMults.h>
#include <New/Entity/AEFlags.h>

class TechnoClass;
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
		// ExtraRange / ExtraCrit / ArmorMultData are transient:
		// they hold pointers into AE type objects and are rebuilt
		// every Recalculate(). Serializing them is unnecessary
		// (and impossible now that they store pointers).
		// Just call Recalculate() after loading.
		return Stm
			.Process(this->Crate_FirepowerMultiplier)
			.Process(this->Crate_ArmorMultiplier)
			.Process(this->Crate_SpeedMultiplier)
			.Process(this->ROFMultiplier)
			.Process(this->ReceiveRelativeDamageMult)
			.Process(this->flags)

			.Success() && Stm.RegisterChange(this)
			;
	}
};
