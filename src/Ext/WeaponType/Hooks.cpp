#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x772A0A, WeaponTypeClass_SetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x772A29;
}

DEFINE_HOOK(0x773087, WeaponTypeClass_GetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x7730A3;
}

#pragma region Otamaa
//Ares 3.0p1 completely change the function , no reference to this anymore !
//DEFINE_HOOK(0x71AB47, TemporalClass_GetHelperDamage_AfterAres, 0x5)
//{
//	GET(WeaponStruct* const, Weapon, EAX);
//	GET(TemporalClass*, pTemp, ESI);
//
//	if (auto const TemporalWeapon = Weapon->WeaponType) {
//		TemporalExt::ExtMap.Find(pTemp)->Weapon = TemporalWeapon;
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x71AC50, TemporalClass_LetItGo_ExpireEffect, 0x5)
//{
//	GET(TemporalClass* const, pThis, ESI);
//
//	if (auto const pTarget = pThis->Target) {
//		pTarget->UpdatePlacement(PlacementType::Redraw);
//
//		auto nTotal = pThis->GetWarpPerStep();
//		if (nTotal) {
//			auto pWeapon = TemporalExt::ExtMap.Find(pThis)->Weapon;
//			if (auto const Warhead = pWeapon->Warhead) {
//
//				auto const pTempOwner = pThis->Owner;
//				auto const peWHext = WarheadTypeExt::ExtMap.Find(Warhead);
//
//				if (auto pExpireAnim = peWHext->TemporalExpiredAnim.Get()) {
//
//					auto nCoord = pTarget->GetCenterCoord();
//
//					if (auto const pAnim = GameCreate<AnimClass>(pExpireAnim, nCoord)) {
//						pAnim->ZAdjust = pTarget->GetZAdjustment() - 3;
//						AnimExt::SetAnimOwnerHouseKind(pAnim, pTempOwner->GetOwningHouse()
//							, pTarget->GetOwningHouse(), pThis->Owner, false) ;
//					}
//				}
//
//				if (peWHext->TemporalExpiredApplyDamage.Get())
//				{
//					auto const pTargetStreght = pTarget->GetTechnoType()->Strength;
//
//					if (pThis->WarpRemaining > 0) {
//
//						auto damage = Game::F2I((pTargetStreght * ((1.0 - pThis->WarpRemaining / 10.0 / pTargetStreght)
//							* (pWeapon->Damage * peWHext->TemporalDetachDamageFactor.Get()) / 100)));
//
//						if (pTarget->IsAlive && !pTarget->IsSinking && !pTarget->IsCrashing)
//							pTarget->ReceiveDamage(&damage, pTempOwner->DistanceFrom(pTarget), Warhead, pTempOwner, false, static_cast<bool>(ScenarioClass::Instance->Random(0, 1)), pTempOwner->Owner);
//					}
//				}
//			}
//		}
//	}
//
//	return 0x71AC5D;
//}

DEFINE_HOOK(0x71A9EE, TemporalClass_Update_RemoveBuildingTarget, 0x9)
{
	GET(TemporalClass* const, pThis, ESI);

	BuildingClass* BuildingTargetResult = nullptr;
	auto const pTarget = pThis->Target;

	{
		if (pTarget->IsSelected)
			pTarget->Deselect();

		if (auto const BuildingTarget= specific_cast<BuildingClass*>(pTarget))
		{
			BuildingTargetResult = BuildingTarget;

			if (BuildingTarget->BunkerLinkedItem)
				BuildingTarget->UnloadBunker();

			if (BuildingTarget->Type->Helipad
				&& BuildingTarget->RadioLinks.Items
				&& BuildingTarget->RadioLinks.IsAllocated
				&& BuildingTarget->RadioLinks.IsInitialized
				)
			{
				for (auto i = 0; i < BuildingTarget->RadioLinks.Capacity; ++i)
				{
					if (auto const pAir = specific_cast<AircraftClass*>(BuildingTarget->RadioLinks[i]))
					{
						if (pAir->IsAlive && !pAir->InLimbo)
						{
							if (pAir->IsInAir() && pAir->Type->Crashable) {
								pAir->Crash(pThis->Owner);
							} else {
								//Ask plane to fly
								BuildingTarget->SendCommand(RadioCommand::AnswerLeave, pAir);
								pAir->DockedTo = nullptr;
							}
						}
					}
				}
			}
		}
	}

	R->ECX(BuildingTargetResult);
	return 0x71AA1D;
}
#pragma endregion