 #include "Body.h"
#include <SpecificStructures.h>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TEvent/Body.h>
#include <Locomotor/Cast.h>
#include <Ext/Script/Body.h>
#include <Ext/Anim/Body.h>

#include <RadarEventClass.h>


// namespace EvaluateObjectTemp
// {
// 	WeaponTypeClass* PickedWeapon = nullptr;
// }
//
// ASMJIT_PATCH(0x6F7E24, TechnoClass_EvaluateObject_SetContext, 0x6)
// {
// 	GET(WeaponTypeClass*, pWeapon, EBP);
//
// 	EvaluateObjectTemp::PickedWeapon = pWeapon;
//
// 	return 0;
// }

//TODO : evaluate this
//interesting mechanic to replace negative damage always remove parasite
//https://github.com/Phobos-developers/Phobos/pull/1126
static void applyRemoveParasite(TechnoClass* pThis, args_ReceiveDamage* args)
{
	if (ScriptExtData::IsUnitAvailable(pThis, false))
	{
		if (const auto pFoot = flag_cast_to<FootClass*, false>(pThis))
		{
			// Ignore other cases that aren't useful for this logic
			if (pFoot->ParasiteEatingMe)
			{
				const auto pWHExt = WarheadTypeExtContainer::Instance.Find(args->WH);
				auto parasyte = pFoot->ParasiteEatingMe;

				if (!pWHExt->CanRemoveParasytes.isset() || !pWHExt->CanTargetHouse(parasyte->Owner, pThis))
					return;

				if (pWHExt->CanRemoveParasytes.Get())
				{
					if (pWHExt->CanRemoveParasytes_ReportSound.isset())
						VocClass::SafeImmedietelyPlayAt(pWHExt->CanRemoveParasytes_ReportSound.Get(), &parasyte->GetCoords() , nullptr);

					// Kill the parasyte
					CoordStruct coord = TechnoExtData::PassengerKickOutLocation(pThis, parasyte, 10);

					if (!pWHExt->CanRemoveParasytes_KickOut.Get() || coord == CoordStruct::Empty)
					{
						//Debug::LogInfo(__FUNCTION__);
						TechnoExtData::HandleRemove(parasyte, args->Attacker, false, false);
					}
					else
					{
						// Kick the parasyte outside
						pFoot->ParasiteEatingMe = nullptr;

						if (!parasyte->Unlimbo(coord, parasyte->PrimaryFacing.Current().GetDir()))
						{
							//Debug::LogInfo(__FUNCTION__);
							TechnoExtData::HandleRemove(parasyte, nullptr, false, false);
							return;
						}

						parasyte->Target = nullptr;
						int paralysisCountdown = pWHExt->CanRemoveParasytes_KickOut_Paralysis.Get() < 0 ? 15 : pWHExt->CanRemoveParasytes_KickOut_Paralysis.Get();

						if (paralysisCountdown > 0)
						{
							parasyte->ParalysisTimer.Start(paralysisCountdown);
							parasyte->DiskLaserTimer.Start(paralysisCountdown);
						}

						if (pWHExt->CanRemoveParasytes_KickOut_Anim.isset() && pWHExt->CanRemoveParasytes_KickOut_Anim)
						{
							auto const pAnim = GameCreate<AnimClass>(pWHExt->CanRemoveParasytes_KickOut_Anim.Get(), parasyte->GetCoords());
							AnimExtData::SetAnimOwnerHouseKind(pAnim, args->SourceHouse ? args->SourceHouse : parasyte->Owner, pThis->Owner, parasyte, false, false);
							pAnim->SetOwnerObject(parasyte);
						}
					}
				}
			}
		}
	}
}
#undef REPLACE_ARMOR

#include <Ext/Super/Body.h>
#include <New/PhobosAttachedAffect/Functions.h>

ASMJIT_PATCH(0x6F6AC4, TechnoClass_Limbo_AfterRadioClassRemove, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pThis->Owner && pThis->Owner->CountOwnedAndPresent(pTypeExt->AttachedToObject) <= 0 && !pTypeExt->Linked_SW.empty())
		pThis->Owner->UpdateSuperWeaponsOwned();

	if (const auto pShieldData = pExt->GetShield())
		pShieldData->OnRemove();

	bool markForRedraw = false;
	bool altered = false;

	// Do not remove attached effects from undeploying buildings.
	if (auto const pBuilding = cast_to<BuildingClass*, false>(pThis)) {
		if ((pBuilding->Type->UndeploysInto && pBuilding->CurrentMission == Mission::Selling && pBuilding->MissionStatus == 2)) {
			return 0;
		}
	}

	pExt->PhobosAE.remove_all_if([&](auto& it){

		if(!it)
			return true;

		if ((it->GetType()->DiscardOn & DiscardCondition::Entry) != DiscardCondition::None) {
			altered = true;

			if (it->GetType()->HasTint())
				markForRedraw = true;

			if (it->ResetIfRecreatable()) {
				return false;
			}

			return true;
		}

		return false;
	});

	if (altered)
		AEProperties::Recalculate(pThis);

	if (markForRedraw)
		pThis->MarkForRedraw();

	return 0;
}

//#pragma region HealingWeapons
//
//#pragma region TechnoClass__Evaluate_Object
//
//double __fastcall HealthRatio_Wrapper(TechnoClass* pTechno, void* _)
//{
//	double result = pTechno->GetHealthPercentage();
//	if (result >= 1.0)
//	{
//		const auto pExt = TechnoExtContainer::Instance.Find(pTechno);
//		if (const auto pShieldData = pExt->Shield.get())
//		{
//			if (pShieldData->IsActive())
//			{
//				const auto pWeapon = EvaluateObjectTemp::PickedWeapon;
//				if (!pShieldData->CanBePenetrated(pWeapon ? pWeapon->Warhead : nullptr))
//					result = pExt->Shield->GetHealthRatio();
//			}
//		}
//	}
//
//	EvaluateObjectTemp::PickedWeapon = nullptr;
//	return result;
//}
//
//DEFINE_FUNCTION_JUMP(CALL, 0x6F7F51, GET_OFFSET(HealthRatio_Wrapper));
//
//#include <Ext/BuildingType/Body.h>
//#pragma endregion TechnoClass__Evaluate_Object
//
//class AresScheme
//{
//	static OPTIONALINLINE ObjectClass* LinkedObj = nullptr;
//public:
//
//	static void __cdecl Prefix(TechnoClass* pThis, ObjectClass* pObj, int nWeaponIndex)
//	{
//		if (LinkedObj)
//			return;
//
//		if (const auto pTechno = abstract_cast<TechnoClass*>(pObj))
//		{
//			const auto pExt = TechnoExtContainer::Instance.Find(pTechno);
//
//			if (const auto pShieldData = pExt->Shield.get())
//			{
//				if (pShieldData->IsActive())
//				{
//					const auto pWeapon = pThis->GetWeapon(nWeaponIndex < 0 ? pThis->SelectWeapon(pObj) : nWeaponIndex);
//
//					if (pWeapon && pWeapon->WeaponType && !pShieldData->CanBePenetrated(pWeapon->WeaponType->Warhead))
//					{
//						if (pExt->Shield->GetHealthRatio() < 1.0)
//						{
//							LinkedObj = pObj;
//							--LinkedObj->Health;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	static void __cdecl Prefix(InfantryClass* pThis, ObjectClass* pObj, int nWeaponIndex, bool IsEngiiner)
//	{
//		if (LinkedObj)
//			return;
//
//		if (IsEngiiner && CanApplyEngineerActions(pThis, pObj))
//			return;
//
//		if (const auto pTechno = abstract_cast<TechnoClass*>(pObj))
//		{
//			const auto pExt = TechnoExtContainer::Instance.Find(pTechno);
//
//			if (const auto pShieldData = pExt->Shield.get())
//			{
//				if (pShieldData->IsActive())
//				{
//					const auto pWeapon = pThis->GetWeapon(nWeaponIndex < 0 ? pThis->SelectWeapon(pObj) : nWeaponIndex);
//
//					if (pWeapon && pWeapon->WeaponType && !pShieldData->CanBePenetrated(pWeapon->WeaponType->Warhead))
//					{
//						if (pExt->Shield->GetHealthRatio() < 1.0)
//						{
//							LinkedObj = pObj;
//							--LinkedObj->Health;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	static void __cdecl Suffix()
//	{
//		if (LinkedObj)
//		{
//			++LinkedObj->Health;
//			LinkedObj = nullptr;
//		}
//	}
//private:
//	static bool CanApplyEngineerActions(InfantryClass* pThis, ObjectClass* pTarget)
//	{
//		if (const auto pBuilding = specific_cast<BuildingClass*>(pTarget))
//		{
//			const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
//
//			if (HouseClass::CurrentPlayer->IsAlliedWith(pBuilding))
//			{
//				return pExt->EngineerRepairable.Get(pBuilding->Type->Repairable);
//			}
//			else
//			{
//				return pBuilding->Type->Capturable && (!pBuilding->Owner->Type->MultiplayPassive || !pBuilding->Type->CanBeOccupied || pBuilding->IsBeingWarpedOut());
//			}
//		}
//
//		return false;
//	}
//};
//
//#pragma region UnitClass_GetFireError_Heal
//
//FireError __fastcall UnitClass__GetFireError_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
//{
//	AresScheme::Prefix(pThis, pObj, nWeaponIndex);
//	auto const result = pThis->UnitClass::GetFireError(pObj, nWeaponIndex, ignoreRange);
//	AresScheme::Suffix();
//	return result;
//}
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6030, GET_OFFSET(UnitClass__GetFireError_Wrapper));
//#pragma endregion UnitClass_GetFireError_Heal
//
//#pragma region InfantryClass_GetFireError_Heal
//
//FireError __fastcall InfantryClass__GetFireError_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
//{
//	AresScheme::Prefix(pThis, pObj, nWeaponIndex);
//	auto const result = pThis->InfantryClass::GetFireError(pObj, nWeaponIndex, ignoreRange);
//	AresScheme::Suffix();
//	return result;
//}
//
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB418, GET_OFFSET(InfantryClass__GetFireError_Wrapper));
//#pragma endregion InfantryClass_GetFireError_Heal
//
//#pragma region UnitClass__WhatAction
//
//Action __fastcall UnitClass__WhatAction_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
//{
//	AresScheme::Prefix(pThis, pObj, -1);
//	auto const result = pThis->UnitClass::MouseOverObject(pObj, ignoreForce);
//	AresScheme::Suffix();
//	return result;
//}
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5CE4, GET_OFFSET(UnitClass__WhatAction_Wrapper));
//#pragma endregion UnitClass__WhatAction
//
//#pragma region InfantryClass__WhatAction
//Action __fastcall InfantryClass__WhatAction_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
//{
//	AresScheme::Prefix(pThis, pObj, -1, pThis->Type->Engineer);
//	auto const result = pThis->InfantryClass::MouseOverObject(pObj, ignoreForce);
//	AresScheme::Suffix();
//	return result;
//}
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB0CC, GET_OFFSET(InfantryClass__WhatAction_Wrapper));
//#pragma endregion InfantryClass__WhatAction
//#pragma endregion HealingWeapons
