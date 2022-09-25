#include "Hooks.OtamaaBugFix.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>

//TODO : retest for desync
DEFINE_HOOK(0x4242F4, AnimClass_Trail_Override, 0x6) // was 4
{
	GET(AnimClass*, pAnim, EDI);
	GET(AnimClass*, pThis, ESI);

	auto nCoord = pThis->GetCenterCoord();
	GameConstruct(pAnim, pThis->Type->TrailerAnim, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	{
		if (const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
		{
			const auto pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
			const auto pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTech, false);
		}
	}

	return 0x424322;
}


DEFINE_HOOK(0x47257C, CaptureManagerClass_TeamChooseAction_Random, 0x6)
{
	GET(FootClass*, pFoot, EAX);

	if (auto pTeam = pFoot->Team)
	{
		if (auto nTeamDecision = pTeam->Type->MindControlDecision)
		{
			if (nTeamDecision > 5)
				nTeamDecision = ScenarioClass::Instance->Random.RandomRanged(1, 5);

			R->EAX(nTeamDecision);
			return 0x47258F;
		}
	}

	return 0x4725B0;
}

//#ifdef ENABLE_NEWHOOKS
//TODO : retest for desync
DEFINE_HOOK(0x442A2A, BuildingClass_ReceiveDamage_RotateVsAircraft, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(RulesClass*, pRules, ECX);

	if (pThis && pThis->Type) {
		if (auto const pStructureExt = BuildingTypeExt::ExtMap.Find(pThis->Type)) {
			R->AL(pStructureExt->PlayerReturnFire.Get(pRules->PlayerReturnFire));
			return 0x442A30;
		}
	}

	return 0x0;
}
//#endif

DEFINE_HOOK_AGAIN(0x4426DB, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK_AGAIN(0x702777, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK(0x70272E, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
{
	enum
	{
		BuildingClass_TakeDamage_DamageSound = 0x4426DB,
		BuildingClass_TakeDamage_DamageSound_Handled_ret = 0x44270B,

		TechnoClass_TakeDamage_Building_DamageSound_01 = 0x702777,
		TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret = 0x7027AE,

		TechnoClass_TakeDamage_Building_DamageSound_02 = 0x70272E,
		TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret = 0x702765,

		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		if (pExt && pExt->DisableDamageSound.Get())
		{
			switch (R->Origin())
			{
			case BuildingClass_TakeDamage_DamageSound:
				return BuildingClass_TakeDamage_DamageSound_Handled_ret;
			case TechnoClass_TakeDamage_Building_DamageSound_01:
				return TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret;
			case TechnoClass_TakeDamage_Building_DamageSound_02:
				return TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret;
			}
		}
	}

	return Nothing;
}


DEFINE_HOOK(0x4FB63A, HouseClass_PlaceObject_EVA_UnitReady, 0x5)
{
	GET(TechnoClass*, pProduct, ESI);

	auto nSpeak = reinterpret_cast<const char*>(0x8249A0);

	if (auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pProduct->GetTechnoType())) {
		if (CRT::strlen(pTechnoTypeExt->Eva_Complete.data())) {
			if (INIClass::IsBlank(pTechnoTypeExt->Eva_Complete.data()) ||
			 !CRT::strcmpi(DEFAULT_STR, pTechnoTypeExt->Eva_Complete.data()) ||
			 !CRT::strcmpi(DEFAULT_STR2, pTechnoTypeExt->Eva_Complete.data()))
			{
				return 0x4FB649;
			}
			else
			{
				nSpeak = pTechnoTypeExt->Eva_Complete.data();
			}
		}
	}

	VoxClass::Play(nSpeak);

	return 0x4FB649;
}

DEFINE_HOOK(0x4FB7CA, HouseClass_RegisterJustBuild_CreateSound_PlayerOnly, 0x6) //9
{
	GET(HouseClass*, pThis, EDI);
	GET(TechnoTypeClass*, pTech, ESI);
	GET_STACK(TechnoClass*, pTech_, STACK_OFFS(0x1C, -0x4));

	if (pTech && pTech_)
	{
		if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTech))
		{
			if (pTechnoTypeExt->VoiceCreate.Get() != -1)
				pTech_->QueueVoice(pTechnoTypeExt->VoiceCreate.Get());

			if (!pTechnoTypeExt->CreateSound_Enable.Get())
				return 0x4FB804;

			if (RulesExt::Global()->CreateSound_PlayerOnly.Get())
				return pThis->IsCurrentPlayer() ? 0x0 : 0x4FB804;

		}
	}

	return 0x0;
}

DEFINE_HOOK(0x6A8E25, SidebarClass_StripClass_AI_Building_EVA_ConstructionComplete, 0x5)
{
	GET(TechnoClass*, pTech, ESI);

	if (pTech && (pTech->WhatAmI() == AbstractType::Building) &&
		pTech->GetOwningHouse() &&
		pTech->GetOwningHouse()->IsCurrentPlayer())
	{
		auto nSpeak = reinterpret_cast<const char*>(0x83FA80); //EVA_ConstructionComplete
		if (auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTech->GetTechnoType())) {
			if (CRT::strlen(pTechnoTypeExt->Eva_Complete.data())) {
				if (INIClass::IsBlank(pTechnoTypeExt->Eva_Complete.data()) ||
				 !CRT::strcmpi(DEFAULT_STR, pTechnoTypeExt->Eva_Complete.data()) ||
				 !CRT::strcmpi(DEFAULT_STR2, pTechnoTypeExt->Eva_Complete.data()))
				{
					return 0x6A8E34;
				}
				else
				{
					nSpeak = pTechnoTypeExt->Eva_Complete.data();
				}
			}
		}

		VoxClass::Play(nSpeak);
	}

	return 0x6A8E34;
}

DEFINE_HOOK(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->JumpJet && !pThis->Target && !pThis->Type->TurretSpins)
	{
		pThis->SecondaryFacing.Set_Desired(pThis->PrimaryFacing.Current());
		pThis->__IsTurretTurning_49C = pThis->PrimaryFacing.Is_Rotating();
		return 0x736C09;
	}

	return 0;
}

//size
//#ifdef ENABLE_NEWHOOKS
//Crash
//DEFINE_HOOK(0x444014, AircraftClass_ExitObject_DisableRadioContact, 0x5)
//{
//	enum { SkipAllSet = 0x444053, Nothing = 0x0 };
//
//	//GET(BuildingClass*, pBuilding, ESI);
//	GET(AircraftClass*, pProduct, EBP);
//
//	if (pProduct) {
//		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pProduct->Type);
//		if (pTypeExt && !pProduct->Type->AirportBound && pTypeExt->NoAirportBound_DisableRadioContact.Get()) {
//			return SkipAllSet;
//		}
//	}
//
//	return Nothing;
//}

//#endif
//
//DEFINE_HOOK(0x415302, AircraftClass_MissionUnload_IsDropship, 0x8)
//{
//	GET(AircraftClass*, pThis, ESI);
//
//	if (pThis->Destination) {
//		if (pThis->Type->IsDropship) {
//			CellStruct nCell = CellStruct::Empty;
//			if (pThis->SelectNavalTargeting(pThis->Destination) != 11) {
//				if (auto pTech = pThis->Destination->AsTechno()) {
//					auto nCoord = pTech->GetCoords();
//					nCell = CellClass::Coord2Cell(nCoord);
//
//					if (nCell != CellStruct::Empty) {
//						if (auto pCell = Map[nCell]) {
//							for (auto pOccupy = pCell->FirstObject;
//								pOccupy;
//								pOccupy = pOccupy->NextObject) {
//								if (pOccupy->WhatAmI() == AbstractType::Building) {
//									auto pGoodLZ = pThis->GoodLandingZone();
//									pThis->SetDestination(pGoodLZ, true);
//								}
//								else {
//									nCoord = pThis->GetCoords();
//									pOccupy->Scatter(nCoord, true, true);
//								}
//							}
//						}
//					}
//				}
//			}
//		} else {
//			return 0x41531B;
//		}
//	}
//
//	return 0x41530C;
//}


//DEFINE_HOOK(0x4CD8C9, FlyLocomotionClass_Movement_AI_DisableTSExp, 0x9)
//{
//	GET(FootClass*, pFoot, EDX);
//	auto const& pTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType());
//	return pTypeExt->Disable_C4WarheadExp.Get() ? 0x4CD9C0 : 0x0;
//}


static Iterator<AnimTypeClass*> GetDeathBodies(InfantryTypeClass* pThisType)
{
	Iterator<AnimTypeClass*> Iter;

	if (pThisType->DeadBodies.Count > 0)
		Iter = pThisType->DeadBodies;

	if (!pThisType->NotHuman)
		Iter = RulesGlobal->DeadBodies;

	return Iter;
}
//
//TODO : retest for desync
DEFINE_HOOK(0x520BE5, InfantryClass_DoingAI_DeadBodies, 0x6)
{
	GET(InfantryClass* const, pThis, ESI);
	GET(InfantryTypeClass* const, pThisType, ECX);

	auto const Iter = GetDeathBodies(pThisType);

	if (!Iter.empty())
	{
		if (AnimTypeClass* pSelected = Iter.at(ScenarioGlobal->Random.RandomFromMax(Iter.size() - 1)))
		{
			if (const auto pAnim = GameCreate<AnimClass>(pSelected, pThis->GetCenterCoord(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
			}
		}
	}

	return 0x520CA9;
}

DEFINE_HOOK(0x5F54A8, ObjectClass_ReceiveDamage_ConditionYellow, 0x6)
{
	enum
	{
		ContinueCheck = 0x5F54C4
		, ResultHalf = 0x5F54B8
	};

	GET(int, nOldStr, EDX);
	GET(int, nCurStr, EBP);
	GET(int, nDamage, ECX);

	const auto curstr = Game::F2I(nCurStr * RulesGlobal->ConditionYellow);
	return (nOldStr <= curstr || !((nOldStr - nDamage) < curstr)) ? ContinueCheck : ResultHalf;
}
