#include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <CaptureManagerClass.h>

static bool CanAttackMindControlled(TechnoClass* pControlled, TechnoClass* pRetaliator)
{
	const auto pMind = pControlled->MindControlledBy;

	if (!pMind || pRetaliator->Berzerk)
		return true;

	const auto pManager = pMind->CaptureManager;

	if (!pManager)
		return true;

	const auto pHome = pManager->GetOriginalOwner(pControlled);
	const auto pHouse = pRetaliator->Owner;

	if (!pHome || !pHouse || !pHouse->IsAlliedWith(pHome))
		return true;

	return TechnoExtContainer::Instance.Find(pControlled)->BeControlledThreatFrame <= Unsorted::CurrentFrame();
}

ASMJIT_PATCH(0x7089E8, TechnoClass_AllowedToRetaliate_AttackMindControlledDelay, 0x6)
{
	enum { CannotRetaliate = 0x708B17 };

	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pAttacker, EBP);

	return CanAttackMindControlled(pAttacker, pThis) ? 0 : CannotRetaliate;
}

ASMJIT_PATCH(0x6F88BF, TechnoClass_CanAutoTargetObject_AttackMindControlledDelay, 0x6)
{
	enum { CannotSelect = 0x6F894F };

	GET(TechnoClass* const, pThis, EDI);
	GET(ObjectClass* const, pTarget, ESI);

	if (const auto pTechno = flag_cast_to<TechnoClass* , false>(pTarget)) {
		if (!CanAttackMindControlled(pTechno, pThis))
			return CannotSelect;
	}

	return 0;
}