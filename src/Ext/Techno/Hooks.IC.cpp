#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <BuildingClass.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

using PreApplyIC = std::function<bool(TechnoClass*)>;

class FakeWrapperClass final {
public:

	static int __fastcall __IronTintTimer(TechnoClass* pThis, discard_t, int intensity)
	{
		// Calculate remaining time on iron tint timer
		const int remainingTime = pThis->IronTintTimer.GetTimeLeft();

		// Calculate stage-specific multiplier (v5 in original)
		int multiplier = 0;

		switch (pThis->IronTintStage)
		{
		case 1:  // Fade in: (12 - time) * 256 / 6
			multiplier = ((12 - remainingTime) << 8) / 6;
			break;

		case 2:  // Sustain
		case 8:  // Sustain (alternate)
			multiplier = 512;
			break;

		case 3:  // Complex fade: (461 * time + 1020) / 10
		{
			int temp = 461 * remainingTime + 1020;
			// Original uses magic multiply 0x66666667 for /10
			multiplier = temp / 10;
			break;
		}

		case 4:  // Fade: (1024 - 77 * time) / 8
			multiplier = (1024 - 77 * remainingTime) / 8;
			break;

		case 5:  // Fade: (77 * time + 816) / 16
			multiplier = (77 * remainingTime + 816) / 16;
			break;

		case 6:  // Fixed value
			multiplier = 51;
			break;

		case 7:  // Fade out: (3072 - 461 * time) / 6
			multiplier = (3072 - 461 * remainingTime) / 6;
			break;

		case 9:  // Quick fade: (time + 20) * 256 / 20
			multiplier = ((remainingTime + 20) << 8) / 20;
			break;

		default:  // No tinting active
			return intensity;
		}

		// Apply multiplier and clamp
		int result = (intensity * multiplier) >> 8;
		if (result > 2000)
			result = 2000;

		const int extraIntensity = static_cast<int>(
			(pThis->ProtectType == ProtectTypes::ForceShield ? RulesExtData::Instance()->ForceShield_ExtraTintIntensity
				: RulesExtData::Instance()->IronCurtain_ExtraTintIntensity) * 1000
		);

		return result + extraIntensity;
	}

	static int __fastcall __AirstrikeTintTimer(TechnoClass* pThis, discard_t, int intensity)
	{
		// Calculate remaining time on airstrike tint timer
		int remainingTime = pThis->AirstrikeTintTimer.GetTimeLeft();

		// Calculate stage-specific multiplier
		int multiplier;

		switch (pThis->AirstrikeTintStage)
		{
		case 1:  // Fade in (shared with case 7)
		case 7:  // Fade out variant
			multiplier = ((12 - remainingTime) << 8) / 6;
			break;

		case 2:  // Sustain
		case 8:  // Sustain (alternate)
			multiplier = 512;
			break;

		case 3:  // Quick fade (shared with case 9)
		case 9:  // Quick fade variant
			multiplier = ((remainingTime + 20) << 8) / 20;
			break;

		case 4:  // Fade: (128 - time) * 256 / 64
			multiplier = ((128 - remainingTime) << 8) / 64;
			break;

		case 5:  // Fade: (time + 64) * 256 / 64
			multiplier = ((remainingTime + 64) << 8) / 64;
			break;

		case 6:  // Fixed value
			multiplier = 256;
			break;

		default:  // No tinting active
			return intensity;
		}

		// Apply multiplier and clamp
		int result = (intensity * multiplier) >> 8;
		return (result > 2000) ? 2000 : result;
	}

	static IronCurtainFlag GetFlag(TechnoClass* pThis, TechnoTypeExtData* pTypeExt, bool forceshield)
	{
		const auto what = pThis->WhatAmI();
		bool isOrganic = pTypeExt->This()->Organic || what == InfantryClass::AbsID;
		const IronCurtainFlag defaultaffect = (!isOrganic ? IronCurtainFlag::Invulnerable : (forceshield ? &RulesExtData::Instance()->ForceShield_EffectOnOrganics : &RulesExtData::Instance()->IronCurtain_EffectOnOrganics)->Get());
		const IronCurtainFlag affect = (forceshield ? &pTypeExt->ForceShield_Effect : &pTypeExt->IronCurtain_Effect)->Get(defaultaffect);
		return EnumFunctions::GetICFlagResult(affect);
	}

	static DamageState TechnoClass_Kill(TechnoClass* pThis, TechnoTypeExtData* pTypeExt, HouseClass* pSource, bool forceshield, bool isOrganic)
	{
		const auto killWH = (forceshield ? &pTypeExt->ForceShield_KillWarhead : &pTypeExt->IronCurtain_KillWarhead);
		const auto killWH_org = forceshield ? &RulesExtData::Instance()->ForceShield_KillOrganicsWarhead : &RulesExtData::Instance()->IronCurtain_KillOrganicsWarhead;
		auto killWH_result = killWH->Get(!isOrganic ? RulesClass::Instance->C4Warhead : killWH_org->Get());
		return pThis->ReceiveDamage
		(
			&pThis->Health,
			0,
			killWH_result,
			nullptr,
			true,
			false,
			pSource
		);
	}

	static DamageState DecideAction(TechnoClass* pThis, int nDur, HouseClass* pSource, bool bIsFC, PreApplyIC func_pre = nullptr)
	{
		const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

		switch (GetFlag(pThis, pTypeExt, bIsFC))
		{
		case IronCurtainFlag::Ignore:
		{
			return (DamageState::Unaffected);
		}
		case IronCurtainFlag::Kill:
		{
			const auto what = pThis->WhatAmI();
			bool isOrganic = pTypeExt->This()->Organic || what == InfantryClass::AbsID;
			return TechnoClass_Kill(pThis, pTypeExt, pSource, bIsFC, isOrganic);
		}
		case IronCurtainFlag::Invulnerable:
		{
			if (func_pre)
			{
				if (func_pre(pThis))
					return DamageState::Unaffected;
			}

			return TechnoClass_ApplyIC(pThis, discard_t(), nDur, pSource ,bIsFC);
		}
		default:
			const auto what = pThis->WhatAmI();
			bool isOrganic = pTypeExt->This()->Organic || what == InfantryClass::AbsID;

			if (!pTypeExt->This()->Organic || what != InfantryClass::AbsID)
			{
				if (bIsFC && what != BuildingClass::AbsID)
				{
					return (DamageState::Unaffected);
				}
				else
				{
					if (func_pre)
					{
						if (func_pre(pThis))
							return DamageState::Unaffected;
					}

					return TechnoClass_ApplyIC(pThis, discard_t(), nDur, pSource, bIsFC);
				}

			}

			return TechnoClass_Kill(pThis, pTypeExt, pSource, bIsFC, isOrganic);
		}

		return DamageState::Unaffected;
	}

	static DamageState __fastcall FootClass_ApplyIC(FootClass* pThis, discard_t , int duration, HouseClass* pSource, bool force)
	{
		return DecideAction(pThis, duration, pSource, force, [](TechnoClass* pThis)-> bool
		{
			auto pFoot = static_cast<FootClass*>(pThis);

			if (auto pPara = pFoot->ParasiteEatingMe)
			{
				pFoot->ParasiteImUsing->SuppressionTimer.Start(50);
				pFoot->ParasiteImUsing->ExitUnit();
			}

			pFoot->ParalysisTimer.Stop();
			return false;
		});
	}

	static DamageState __fastcall TechnoClass_ApplyIC(TechnoClass* pThis, discard_t, int duration, HouseClass* pSource, bool force)
	{
		const auto pData = GET_TECHNOTYPEEXT(pThis);
		const auto modifier = (force ? pData->ForceShield_Modifier : pData->IronCurtain_Modifier).Get();

		pThis->IronCurtainTimer.Start(int(duration * modifier));
		pThis->IronTintStage = 0;
		pThis->ProtectType = force ? ProtectTypes::ForceShield : ProtectTypes::IronCurtain;

		return DamageState::Unaffected;
	}

	static DamageState __fastcall AircraftClass_IronCurtain(AircraftClass* pThis, discard_t, int nDur, HouseClass* pSource, bool bIsFC)
	{
		return DecideAction(pThis, nDur, pSource, bIsFC, [](TechnoClass* pThis)-> bool
		{
			auto pAir = static_cast<AircraftClass*>(pThis);

			if (auto pPara = pAir->ParasiteEatingMe)
			{
				pAir->ParasiteImUsing->SuppressionTimer.Start(50);
				pAir->ParasiteImUsing->ExitUnit();
			}

			pAir->ParalysisTimer.Stop();
			return false;
		});
	}

	static DamageState __fastcall UnitClass_IronCurtain(UnitClass* pThis, discard_t, int nDur, HouseClass* pSource, bool bIsFC)
	{
		return DecideAction(pThis, nDur, pSource, bIsFC, [](TechnoClass* pThis)-> bool
		 {
		 auto pUnit = static_cast<UnitClass*>(pThis);

		 if (auto pPara = pUnit->ParasiteEatingMe)
		 {
			 pUnit->ParasiteImUsing->SuppressionTimer.Start(50);
			 pUnit->ParasiteImUsing->ExitUnit();
		 }

		 pUnit->ParalysisTimer.Stop();
		 return false;
		});
	}

	static DamageState __fastcall BuildingClass_IronCurtain(BuildingClass* pThis, discard_t, int duration, HouseClass* pSource, bool force)
	{
		return DecideAction(pThis, duration, pSource, force, [](TechnoClass* pThis)-> bool
		 {
			 auto pBld = static_cast<BuildingClass*>(pThis);

			 if (pBld->IsGoingToBlow)
			 {
				 pBld->IsGoingToBlow = false;
				 pBld->C4AppliedBy = nullptr;
				 pBld->GoingToBlowTimer.Stop();
			 }
			 return false;
		});
	}

	static DamageState __fastcall InfantryClass_IronCurtain(InfantryClass* pThis, discard_t, int nDur, HouseClass* pSource, bool bIsFC)
	{
		return DecideAction(pThis, nDur, pSource, bIsFC, [](TechnoClass* pThis)-> bool
		{
			 auto pInf = static_cast<InfantryClass*>(pThis);

			 if (pInf->Type->Engineer && pInf->TemporalTargetingMe && pInf->Destination)
			 {
				 if (auto const pCell = pInf->GetCell())
				 {
					 if (auto const pBld = pCell->GetBuilding())
					 {
						 if (pInf->Destination == pBld && pBld->Type->BridgeRepairHut)
						 {
							 return true;
						 }
					 }
				 }
			 }
			 return false;
		});
	}
};

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB1AC, FakeWrapperClass::InfantryClass_IronCurtain);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8DE8, FakeWrapperClass::FootClass_ApplyIC);
DEFINE_FUNCTION_JUMP(LJMP, 0x4DEAE0, FakeWrapperClass::FootClass_ApplyIC);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4AB4, FakeWrapperClass::TechnoClass_ApplyIC);
DEFINE_FUNCTION_JUMP(LJMP, 0x70E2B0, FakeWrapperClass::TechnoClass_ApplyIC);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E23F8, FakeWrapperClass::AircraftClass_IronCurtain);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5DC4, FakeWrapperClass::UnitClass_IronCurtain);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4010, FakeWrapperClass::BuildingClass_IronCurtain);
DEFINE_FUNCTION_JUMP(LJMP, 0x457C90, FakeWrapperClass::BuildingClass_IronCurtain);
DEFINE_FUNCTION_JUMP(LJMP, 0x70E380, FakeWrapperClass::__IronTintTimer);
DEFINE_FUNCTION_JUMP(LJMP, 0x70E4B0, FakeWrapperClass::__AirstrikeTintTimer);