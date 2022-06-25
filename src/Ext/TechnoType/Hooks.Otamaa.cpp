#include "Body.h"
#include <Ext/BuildingType/Body.h>
#include <Utilities/Macro.h>

#include <New/Type/ArmorTypeClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Others/DamageText.h>
#endif

DEFINE_HOOK(0x6B0C2C, SlaveManagerClass_FreeSlaves_Sound, 0xC)
{
	GET(InfantryClass*, pSlave, EDI);

	if (auto const pData = TechnoTypeExt::GetExtData(pSlave->Type))
	{
		if (pData->SlaveFreeSound_Enable.Get())
		{
			auto const nSound = pData->SlaveFreeSound.Get(RulesGlobal->SlavesFreeSound);
			if (nSound != -1)
			{
				auto nCoord = pSlave->GetCoords();
				VocClass::PlayAt(nSound, nCoord);
			}
		}

		return 0x6B0C65;
	}

	return 0x0;
}

/*
DEFINE_HOOK(0x4FB63A, HouseClass_PlaceObject_EVA_UnitReady, 0x5)
{
	GET(TechnoClass*, pProduct, ESI);

	if (pProduct && (pProduct->WhatAmI() != AbstractType::Building) && pProduct->GetOwningHouse() && pProduct->GetOwningHouse()->IsPlayer())
	{
		auto nSpeak = reinterpret_cast<const char*>(0x8249A0);

		if (auto const pTechnoTypeExt = TechnoTypeExt::GetExtData(pProduct->GetTechnoType()))
		{
			if (GeneralUtils::IsValidString(pTechnoTypeExt->Eva_Complete.data()))
			{
				nSpeak = pTechnoTypeExt->Eva_Complete.data();
			}
		}

		VoxClass::Play(nSpeak);

		return 0x4FB649;
	}

	return 0x0;
}

DEFINE_HOOK(0x4FB7CA, HouseClass_RegisterJustBuild_CreateSound_PlayerOnly, 0x9)
{
	GET(TechnoTypeClass*, pTech, ESI);
	GET_STACK(TechnoClass*, pTech_, STACK_OFFS(0x1C, -0x4));

	if (pTech && pTech_)
	{
		if (auto pTechnoTypeExt = TechnoTypeExt::GetExtData(pTech))
		{
			if (pTechnoTypeExt->VoiceCreate.Get() != -1)
				pTech_->QueueVoice(pTechnoTypeExt->VoiceCreate.Get());

			if (!pTechnoTypeExt->CreateSound_Enable.Get())
				return 0x4FB804;

			if (RulesExt::Global()->CreateSound_PlayerOnly.Get())
				return pTech_->GetOwningHouse() && pTech_->GetOwningHouse()->IsPlayer() ? 0x0 : 0x4FB804;

		}
	}

	return 0x0;
}

DEFINE_HOOK(0x6A8E25, SidebarClass_StripClass_AI_Building_EVA_ConstructionComplete, 0x5)
{
	GET(TechnoClass*, pTech, ESI);

	if (pTech && (pTech->WhatAmI() == AbstractType::Building) &&
		pTech->GetOwningHouse() &&
		pTech->GetOwningHouse()->IsPlayer())
	{
		auto nSpeak = reinterpret_cast<const char*>(0x83FA80);

		if (auto const pTechnoTypeExt = TechnoTypeExt::GetExtData(pTech->GetTechnoType()))
		{
			if (GeneralUtils::IsValidString(pTechnoTypeExt->Eva_Complete.data()))
			{
				nSpeak = pTechnoTypeExt->Eva_Complete.data();
			}
		}

		VoxClass::Play(nSpeak);

		return 0x6A8E34;
	}
	return 0x0;
}
*/

DEFINE_HOOK(0x443C0D, BuildingClass_AssignTarget_Jugger, 0x8)
{
	GET(BuildingTypeClass*, pThis, EAX);

	if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis)) {
		return (pThis->TickTank || pTypeExt->IsJuggernaut.Get() || pThis->Artillary) ? 0x443C21 : 0x443BB3;
	}

	return 0x0;
}

DEFINE_HOOK(0x44A93D, BuildingClass_MI_DC_Jugger, 0x8)
{
	GET(BuildingTypeClass*, pThis, EAX);

	if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis)) {
		return (pThis->TickTank || pTypeExt->IsJuggernaut.Get() || pThis->Artillary) ? 0x44A951 : 0x44A95E;
	}

	return 0x0;
}

DEFINE_HOOK(0x739801, UnitClass_TryToDeploy_BarrelFacing_Jugger, 0x8)
{
	GET(BuildingTypeClass*, pThis, EAX);

	if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis))
	{
		R->CL(pThis->TickTank || pTypeExt->IsJuggernaut.Get());
		return 0x739807;
	}

	return 0x0;
}

//there is 2 hooks that get weird optimization behaviour
//so disable  it for them
//#pragma optimize( "", off )
DEFINE_HOOK(0x6F6D9E, TechnoClass_Unlimbo_BuildingFacing_Jugger, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	//GET_STACK(int, nDir, STACK_OFFS(0x28, -0x8));

	//nDir = TranslateFixedPoint(16, 16, nDir, 0);
	//nDir = nDir << 8;
	if (auto pBuilding = specific_cast<BuildingClass*>(pThis)){
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		//auto Allow = [](BuildingTypeClass const * pType) {
		//	if(pType->Wall || pType->LaserFence || pType->LaserFencePost || pType->FirestormWall)
		//		return false;
		//	return true;
	//	};

		if (pTypeExt) {
			if (pTypeExt->IsJuggernaut.Get()) {
				R->ECX(&BuildingTypeExt::DefaultJuggerFacing);
			} //else if (Allow(pBuildingType)) {
			//	const auto nDirOut = CreateDir(pBuildingType->StartFacing);
			//	R->ECX(&nDirOut);
		//	}
		}
	}

	return 0x0;
}
//#pragma optimize("", on)
//#pragma optimize( "", off )
DEFINE_HOOK(0x449B04, TechnoClass_MI_Construct_Facing_Jugger, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type)) {
		if (pTypeExt->IsJuggernaut.Get()) {
			R->EDX(&BuildingTypeExt::DefaultJuggerFacing);
		}
	}

	return 0x0;
}
//#pragma optimize("", on)

static void __fastcall UnitClass_RotationAI_(UnitClass* pThis, void* _)
{
	if (auto TypeExt = TechnoTypeExt::GetExtData(pThis->Type)) {
		auto const nDisableEmp = pThis->EMPLockRemaining && TypeExt->FacingRotation_DisalbeOnEMP.Get();
		auto const nDisableDeactivated = pThis->Deactivated && TypeExt->FacingRotation_DisalbeOnDeactivated.Get() && !pThis->EMPLockRemaining;

		if ((nDisableEmp || nDisableDeactivated || TypeExt->FacingRotation_Disable.Get()))
			return;
	}

	pThis->UpdateRotation();
}

DEFINE_POINTER_CALL(0x7365E8, UnitClass_RotationAI_);

DEFINE_HOOK(0x5F53E5, ObjectClass_ReceiveDamage_HitAnim, 0x8)
{
	GET(ObjectClass*, pThis, ESI);

	/*
		switch (pThis->WhatAmI())
		{
		case AbstractType::Unit:
		case AbstractType::Aircraft:
		case AbstractType::Terrain:
		case AbstractType::Overlay:
		case AbstractType::Infantry:
		case AbstractType::Building:
		case AbstractType::VeinholeMonster:
			break;
		default:
			return 0x0;
			break;
		}
	*/
	GET_STACK(TechnoClass*, pAttacker, STACK_OFFS(0x24, -0x10));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0x24, -0xC));
	GET_STACK(bool, bIgnoreDefense, STACK_OFFS(0x24, -0x14));
	GET_STACK(HouseClass*, pAttackerHouse, STACK_OFFS(0x24, -0x1C));
#ifdef COMPILE_PORTED_DP_FEATURES_
	GET(int*, pDamage, EDI);
#endif
	if (pWarhead)
	{
		auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		auto const pTechno = generic_cast<TechnoClass*>(pThis);
		auto const pType = pThis->GetType();
		bool bImmune_pt2 = false;
		bool const bImmune_pt1 =
			(pThis->IsIronCurtained() && !bIgnoreDefense) ||
			(pType->Immune && !bIgnoreDefense) || pThis->InLimbo
			;

		if (pTechno)
		{
			const auto pExt = TechnoExt::GetExtData(pTechno);
			bImmune_pt2 = (pExt && pExt->Shield.get() && pExt->Shield.get()->IsActive())
				|| pTechno->TemporalTargetingMe
				|| (pTechno->ForceShielded && !bIgnoreDefense)
				|| pTechno->BeingWarpedOut
				|| pTechno->IsSinking
				;

		}

		if (!bImmune_pt1 && !bImmune_pt2)
		{
			auto const nArmor = pType->Armor;
			auto const pArmor = ArmorTypeClass::FindFromIndex((int)nArmor);

			if (pWarheadExt && pArmor)
			{
#ifdef COMPILE_PORTED_DP_FEATURES_
				TechnoClass_ReceiveDamage2_DamageText(pTechno, pDamage, pWarheadExt->DamageTextPerArmor[(int)nArmor]);
#endif

				if ((!pWarheadExt->ArmorHitAnim.empty()))
				{
					AnimTypeClass* pAnimTypeDecided = pWarheadExt->ArmorHitAnim.get_or_default((int)nArmor);

					if (!pAnimTypeDecided && pArmor->DefaultTo != -1)
					{
							//Holy shit !
						for (auto pDefArmor = ArmorTypeClass::Array[pArmor->DefaultTo].get();
							pDefArmor && pDefArmor->DefaultTo != -1;
							pDefArmor = ArmorTypeClass::Array[pDefArmor->DefaultTo].get()) {
							pAnimTypeDecided = pWarheadExt->ArmorHitAnim.get_or_default(pDefArmor->DefaultTo);
							if (pAnimTypeDecided)
								break;
						}
					}

					if (pAnimTypeDecided)
					{
						CoordStruct nBuffer { 0, 0 , 0 };
						if (pTechno)
						{
							if (auto const pTechnoTypeExt = TechnoTypeExt::GetExtData(pTechno->GetTechnoType()))
							{
								if (!pTechnoTypeExt->HitCoordOffset.empty())
								{
									if ((pTechnoTypeExt->HitCoordOffset.size() > 1) && pTechnoTypeExt->HitCoordOffset_Random.Get())
										nBuffer = pTechnoTypeExt->HitCoordOffset[ScenarioGlobal->Random(0, pTechnoTypeExt->HitCoordOffset.size() - 1)];
									else
										nBuffer = pTechnoTypeExt->HitCoordOffset[0];
								}
							}

							auto const nCoord = pThis->GetCenterCoord() + nBuffer;
							if (auto pAnimPlayed = GameCreate<AnimClass>(pAnimTypeDecided, nCoord))
							{
								if (AnimExt::SetAnimOwnerHouseKind(pAnimPlayed, pAttacker ? pAttacker->GetOwningHouse() : pAttackerHouse, pThis->GetOwningHouse(), false))
									if (auto const pAnimExt = AnimExt::GetExtData(pAnimPlayed))
										pAnimExt->Invoker = pAttacker;
							}
						}
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x662720, RocketLocomotionClass_ILocomotion_Process_Raise, 0x6)
{
	enum { Handled = 0x6624C8, Skip = 0x0 };

	GET(RocketLocomotionClass*, pThis, ESI);
	if (auto pAir = specific_cast<AircraftClass*>(pThis->Owner))
	{
		if (const auto pExt = TechnoTypeExt::GetExtData(pAir->Type))
		{
			if (pExt->IsCustomMissile.Get() && !pExt->CustomMissileRaise.Get(pAir))
			{
				return Handled;
			}
		}
	}

	return Skip;
}

DEFINE_HOOK(0x6634F6, RocketLocomotionClass_ILocomotion_DrawMatrix_CustomMissile, 0x6)
{
	enum { Handled = 0x66351B, Skip = 0x0 };

	GET(AircraftTypeClass*, pType, ECX);
	const auto pExt = TechnoTypeExt::GetExtData(pType);

	if (pExt && pExt->IsCustomMissile.Get())
	{
		R->EAX(&pExt->CustomMissileData);
		return Handled;
	}

	return Skip;
}
