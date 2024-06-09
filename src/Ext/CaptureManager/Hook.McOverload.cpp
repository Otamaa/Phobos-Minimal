#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Misc/Ares/Hooks/Header.h>

DEFINE_HOOK(0x6FA726, TechnoClass_AI_MCOverload, 0x6)
{
	enum {
		SelfHeal = 0x6FA743, //continue ares check here
		DoNotSelfHeal = 0x6FA941,
		ReturnFunc = 0x6FAFFD,
		ContineCheckUpdateSelfHeal = 0x6FA75A,
		Continue = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	TechnoExtData::UpdateMCOverloadDamage(pThis);

	if(!pThis->IsAlive)
		return ReturnFunc;

	const auto pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

		// prevent crashing and sinking technos from self-healing
	if (pType->NoExtraSelfHealOrRepair || pThis->InLimbo || pThis->IsCrashing || pThis->IsSinking || TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled) {
		return DoNotSelfHeal;
	}

	const auto nUnit = specific_cast<UnitClass*>(pThis);
	if (nUnit && nUnit->DeathFrameCounter > 0) {
		return DoNotSelfHeal;
	}

	// this replaces the call to pThis->ShouldSelfHealOneStep()
	const auto nAmount = TechnoExt_ExtData::GetSelfHealAmount(pThis);
	bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
	if (nAmount > 0 || nAmount != 0) {
		pThis->Health += nAmount;
	}

	//this one take care of the visual stuffs
	//if the techno health back to normal
	TechnoExtData::ApplyGainedSelfHeal(pThis, wasDamaged);

	//handle everything
	return DoNotSelfHeal;
}