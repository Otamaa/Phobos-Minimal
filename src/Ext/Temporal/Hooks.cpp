#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>

#include <Misc/Ares/Hooks/Header.h>

#include <SpawnManagerClass.h>
#include <CaptureManagerClass.h>
#include <RadarEventClass.h>

// Also fix :
// https://bugs.launchpad.net/ares/+bug/1901825
ASMJIT_PATCH(0x71ADE0, TemporalClass_LetGo_Replace, 0x6)
{
	GET(TemporalClass*, pThis, ECX);

	if (auto pTarget = pThis->Target) {
		if (auto pBuilding = cast_to<BuildingClass*, false>(pTarget)) {
			auto pBldExt = BuildingExtContainer::Instance.Find(pBuilding);

			pBuilding->CashProductionTimer.Resume();

			for (size_t i = 0; i < std::size(pBuilding->Upgrades); ++i) {
				if (pBuilding->Upgrades[i]) {
					pBldExt->CashUpgradeTimers[i].Resume();
				}
			}
		}

		pTarget->BeingWarpedOut = 0;
		pTarget->TemporalTargetingMe = nullptr;
		pThis->Target = nullptr;
	}

	if (auto pPrevTemp = pThis->PrevTemporal)
	{
		if (pThis->NextTemporal == pThis)
			pThis->NextTemporal = nullptr;

		pPrevTemp->Detach();
	}

	if (auto pNextTemp = pThis->NextTemporal)
	{
		if (pNextTemp->PrevTemporal == pThis)
			pNextTemp->PrevTemporal = nullptr;

		pNextTemp->Detach();
	}

	pThis->PrevTemporal = nullptr;
	pThis->NextTemporal = nullptr;
	pThis->unknown_pointer_38 = nullptr;
	pThis->SourceSW = nullptr;

	if (auto pOwner = pThis->Owner)
		pOwner->EnterIdleMode(false, 1);

	return 0x71AE49;

}

ASMJIT_PATCH(0x71AB10, TemporalClass_GetWarpPerStep, 6)
{
	GET_STACK(int, nStep, 0x4);
	GET(TemporalClass*, pThis, ECX);
	R->EAX(TechnoExt_ExtData::GetWarpPerStep(pThis, nStep));
	return 0x71AB57;
}

ASMJIT_PATCH(0x71AC50, TemporalClass_LetItGo_ExpireEffect, 0x5)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto const pTarget = pThis->Target)
	{
		pTarget->Mark(MarkType::Redraw);

		if (auto nTotal = TechnoExt_ExtData::GetWarpPerStep(pThis, 0))
		{
			auto const pWeapon = pThis->Owner->GetWeapon(TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Warp)->WeaponType;

			if (auto const Warhead = pWeapon->Warhead)
			{
				auto const pTempOwner = pThis->Owner;
				auto const peWHext = WarheadTypeExtContainer::Instance.Find(Warhead);

				if (auto pExpireAnim = peWHext->TemporalExpiredAnim.Get())
				{
					auto nCoord = pTarget->GetCenterCoords();
					auto const pAnim = GameCreate<AnimClass>(pExpireAnim, nCoord);
					pAnim->ZAdjust = pTarget->GetZAdjustment() - 3;
					AnimExtData::SetAnimOwnerHouseKind(pAnim, pTempOwner->GetOwningHouse()
						, pTarget->GetOwningHouse(), pThis->Owner, false, false);
				}

				if (peWHext->TemporalExpiredApplyDamage.Get() && pThis->WarpRemaining > 0 && pTarget->IsAlive && !pTarget->IsSinking && !pTarget->IsCrashing) {
					auto const pTargetStreght = GET_TECHNOTYPE(pTarget)->Strength;
					auto damage = int((pTargetStreght * ((((double)pThis->WarpRemaining) / 10.0 / pTargetStreght)
							* (pWeapon->Damage * peWHext->TemporalDetachDamageFactor.Get()) / 100)));

					pTarget->ReceiveDamage(&damage, pTempOwner->DistanceFrom(pTarget), Warhead, pTempOwner, false, ScenarioClass::Instance->Random.RandomBool(), pTempOwner->Owner);
				}
			}
		}
	}

	return 0x71AC5D;
}

ASMJIT_PATCH(0x71AE50, TemporalClass_CanWarpTarget, 8)
{
	GET(TemporalClass*, pTemp, ECX);
	GET_STACK(TechnoClass*, pTarget, 0x4);
	R->EAX(TechnoExt_ExtData::Warpable(pTemp , pTarget));
	return 0x71AF19;
}