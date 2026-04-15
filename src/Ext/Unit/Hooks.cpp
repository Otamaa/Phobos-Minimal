#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/UnitType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Cell/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <UnitClass.h>
#include <UnitTypeClass.h>

ASMJIT_PATCH(0x73844A, UnitClass_Destroyed_PlaceCrate, 0x8)
{
	GET(UnitClass*, pThis, ESI);
	GET(CellStruct, cell, EAX);

	const auto CrateType = &TechnoTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CrateType;
	PowerupEffects crate = CrateType->isset() ? (PowerupEffects)CrateType->Get() : (PowerupEffects)CrateTypeClass::Array.size();
	MapClass::Instance->Place_Crate(cell, crate);
	return 0x738457;
}

ASMJIT_PATCH(0x73ED40, UnitClass_Mission_Harvest_PathfindingFix, 0x7)
{
	GET(UnitClass*, pThis, EBP);
	LEA_STACK(CellStruct*, closeTo, STACK_OFFSET(0x64, -0x4C));
	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x64, -0x54));
	LEA_STACK(CellStruct*, outBuffer, STACK_OFFSET(0x64, -0x3C));

	if (pThis->Type->Teleporter)
		return 0x0;

	auto zone = MapClass::Instance->GetMovementZoneType(pThis->InlineMapCoords(), pThis->Type->MovementZone, pThis->OnBridge);
	R->EAX(MapClass::Instance->NearByLocation(*outBuffer, *cell, pThis->Type->SpeedType, zone, pThis->Type->MovementZone, false, 1, 1, false, false, false, true, *closeTo, false, false));

	return 0x73ED7A;
}

ASMJIT_PATCH(0x737BFB, UnitClass_Unlimbo_SmallVisceroid_DontMergeImmedietely, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pThisType, EAX);

	if (pThisType->SmallVisceroid)
	{
		TechnoExtContainer::Instance.Find(pThis)->MergePreventionTimer.Start(1000);
		return 0x737C38;
	}

	return pThisType->LargeVisceroid ? 0x737C38 : 0x737C0B;
}

ASMJIT_PATCH(0x73745C, UnitClass_ReceiveRadio_Parasited_WantRide, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	enum { negativemessage = 0x73746A, continueChecks = 0x737476 };

	if (pThis->IsBeingWarpedOut()
		|| (pThis->ParasiteEatingMe && pThis->ParasiteEatingMe->ParasiteImUsing->GrappleAnim))
		return negativemessage;

	return continueChecks;
}

ASMJIT_PATCH(0x7375B6, UnitClass_ReceiveRadio_Parasited_CanLoad, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	enum { staticmessage = 0x7375C4, continueChecks = 0x7375D0 };

	if (pThis->IsBeingWarpedOut()
		|| (pThis->ParasiteEatingMe && pThis->ParasiteEatingMe->ParasiteImUsing->GrappleAnim))
		return staticmessage;

	return continueChecks;
}

ASMJIT_PATCH(0x73730E, UnitClass_Visceroid_HealthCheckRestore, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (!pThis->Destination && !pThis->Locomotor.GetInterfacePtr()->Is_Moving())
	{
		if (pThis->IsRedHP() && (pThis->Type->TiberiumHeal || pThis->HasAbility(AbilityType::TiberiumHeal)))
		{

			if (pThis->GetCell()->LandType != LandType::Tiberium)
			{
				// search tiberium and abort current mission
				pThis->MoveToTiberium(pThis->Type->Sight, false);

				if (pThis->Destination)
				{
					if (pThis->ShouldLoseTargetNow)
						pThis->SetTarget(nullptr);

					pThis->unknown_int_6D4 = -1;
					pThis->QueueMission(Mission::Move, false);
					pThis->NextMission();
				}

			}
			else
			{
				pThis->unknown_int_6D4 = -1;
				pThis->QueueMission(pThis->IsArmed() ? Mission::Hunt : Mission::Area_Guard, false);
				pThis->NextMission();
				return 0x73741F;
			}
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x73666A, UnitClass_AI_Viscerid_ZeroStrength, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pType, EAX);
	return pType->Strength <= 0 || pThis->DeathFrameCounter > 0 ? 0x736685 : 0x0;
}

ASMJIT_PATCH(0x74097E, UnitClass_Mission_Guard_IncludeWeeder, 0x6)
{
	GET(BuildingTypeClass*, pBuilding, ECX);
	R->DL(pBuilding->Refinery || pBuilding->Weeder);
	return 0x740984;
}

ASMJIT_PATCH(0x73D0DB, UnitClass_DrawAt_Oregath_IncludeWeeder, 0x6)
{
	enum { Draw = 0x73D0E9, Skip = 0x73D298 };

	GET(UnitClass*, pUnit, ESI);

	return ((pUnit->Type->Harvester || pUnit->Type->Weeder) && !pUnit->IsHarvesting) ?
		Skip : Draw;
}

ASMJIT_PATCH(0x73D2A6, UnitClass_DrawAt_UnloadingClass_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pUnitType, EAX);

	R->CL(pUnitType->Harvester || pUnitType->Weeder);
	return 0x73D2AC;
}

ASMJIT_PATCH(0x73B0C5, UnitClass_Render_nullptrradio, 0x6)
{
	GET(TechnoClass*, pContact, EAX);
	return !pContact ? 0x73B124 : 0x0;
}

//this one dextroy special anim : 741C32
ASMJIT_PATCH(0x73E005, UnitClass_Mission_Unload_PlayBuildingProductionAnim_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->AL(pType->Harvester || (pType->Weeder && TechnoTypeExtContainer::Instance.Find(pType)->Weeder_TriggerPreProductionBuildingAnim));
	return 0x73E00B;
}

ASMJIT_PATCH(0x741C32, UnitClass_AssignDestination_DestroyBuildingProductionAnim_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->DL(pType->Harvester || (pType->Weeder && TechnoTypeExtContainer::Instance.Find(pType)->Weeder_TriggerPreProductionBuildingAnim));
	return 0x741C38;
}

ASMJIT_PATCH(0x736823, UnitClass_AI_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, EAX);
	R->CL(pType->Harvester || pType->Weeder);
	return 0x736829;
}

ASMJIT_PATCH(0x7368C6, UnitClass_Update_WeederMissionMove2, 0x6)
{
	GET(BuildingTypeClass*, pBuildingType, EDX);
	R->CL(pBuildingType->Refinery || pBuildingType->Weeder);
	return 0x7368CC;
}

ASMJIT_PATCH(0x73D4DA, UnitClass_Harvest_VeinsStorageAmount, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(FakeCellClass*, pCell, EBP);

	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	if (storage->m_values.empty())
		storage->m_values.resize(TiberiumClass::Array->Count);

	double amount = 1.0;

	if (pThis->Type->Weeder)
	{
		pCell->RemoveWeed();
		TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.IncreaseAmount(RulesExtData::Instance()->Veins_PerCellAmount, 0);
		return 0x73D502;
	}

	int tibType = pCell->_GetTiberiumType();
	double cur = storage->GetAmounts();

	if (((double)pThis->Type->Storage - cur) <= 1.0)
	{
		amount = (double)pThis->Type->Storage - cur;
	}

	int reduced = pCell->ReduceTiberium((int)amount);

	if (reduced > 0)
	{
		storage->IncreaseAmount((float)amount, tibType);
		return 0x73D5BE;
	}
	return 0x73D623;
}

ASMJIT_PATCH(0x746AFF, UnitClass_Disguise_Update_MoveToClear, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	return pThis->Disguise && pThis->Disguise->WhatAmI() == UnitClass::AbsID ? 0x746A9C : 0;
}

ASMJIT_PATCH(0x7463DC, UnitClass_SetOwningHouse_FixArgs, 0x5)
{
	GET(UnitClass* const, pThis, EDI);
	GET(HouseClass* const, pNewOwner, EBX);
	GET_STACK(bool const, bAnnounce, 0xC + 0x8);

	R->EAX(pThis->FootClass::SetOwningHouse(pNewOwner, bAnnounce));
	return 0x7463E6;
}

ASMJIT_PATCH(0x741554, UnitClass_ApproachTarget_CrushRange, 0x6)
{
	enum { Crush = 0x741599, ContinueCheck = 0x741562 };
	GET(UnitClass* const, pThis, ESI);
	GET(int const, range, EAX);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	return range >= pTypeExt->CrushRange.GetOrDefault(pThis, RulesClass::Instance->Crush) ?
		Crush : ContinueCheck;
}

ASMJIT_PATCH(0x7439AD, UnitClass_ShouldCrush_CrushRange, 0x6)
{
	enum { DoNotCrush = 0x743A39, ContinueCheck = 0x7439B9 };
	GET(UnitClass* const, pThis, ESI);
	GET(int const, range, EAX);
	GET(RulesClass* const, pRules, ECX);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	return range <= pTypeExt->CrushRange.GetOrDefault(pThis, pRules->Crush) ?
		ContinueCheck : DoNotCrush;
}

// make unit able to deploy fire without ejecting it passengers
ASMJIT_PATCH(0x73D6EC, UnitClass_Unload_NoManualEject, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);
	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualEject.Get() ? 0x73DCD3 : 0x0;
}

ASMJIT_PATCH(0x739450, UnitClass_Deploy_LocationFix, 0x7)
{
	GET(UnitClass* const, pThis, EBP);
	const auto deploysInto = pThis->Type->DeploysInto;
	CellStruct mapCoords = pThis->InlineMapCoords();
	R->Stack(STACK_OFFSET(0x28, -0x10), mapCoords.Pack());

	const short width = deploysInto->GetFoundationWidth();
	const short height = deploysInto->GetFoundationHeight(false);

	if (width > 2)
		mapCoords.X -= static_cast<short>(std::ceil(width / 2.0) - 1);
	if (height > 2)
		mapCoords.Y -= static_cast<short>(std::ceil(height / 2.0) - 1);

	R->Stack(STACK_OFFSET(0x28, -0x14), mapCoords.Pack());

	return 0x7394BE;
}

ASMJIT_PATCH(0x73AED4, UnitClass_PCP_DamageSelf_C4WarheadAnimCheck, 0x7)
{
	GET(UnitClass* const, pThis, EBP);
	GET(AnimClass*, pAllocatedMem, ESI);
	GET(LandType const, nLand, EBX);

	CoordStruct nLoc = pThis->Location;
	if (auto const pC4AnimType = MapClass::SelectDamageAnimation(pThis->Health, RulesClass::Instance->C4Warhead, nLand, nLoc))
	{
		pAllocatedMem->AnimClass::AnimClass(pC4AnimType, nLoc, 0, 1, 0x2600, -15, false);
		AnimExtData::SetAnimOwnerHouseKind(pAllocatedMem, nullptr, pThis->GetOwningHouse(), true);
	}
	else
	{
		GameDelete<false, false>(pAllocatedMem);
	}

	return 0x73AF4C;
}

ASMJIT_PATCH(0x7394FF, UnitClass_TryToDeploy_CantDeployVoice, 0x8)
{
	GET(UnitClass* const, pThis, EBP);

	const auto pThisTechno = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	VoxClass::Play(GameStrings::EVA_CannotDeployHere());

	if (pThisTechno->VoiceCantDeploy.isset()) {
		//pThis->QueueVoice(pThisTechno->VoiceCantDeploy);
		VocClass::SafeImmedietelyPlayAt(pThisTechno->VoiceCantDeploy, &pThis->Location);
	}

	return 0x73950F;
}

ASMJIT_PATCH(0x74159F, UnitClass_ApproachTarget_GoAboveTarget, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	auto pType = pThis->Type;
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	R->AL(pType->BalloonHover || pTypeExt->CanGoAboveTarget);
	return R->Origin() + 0x6;
}

ASMJIT_PATCH(0x746A30, UnitClass_UpdateDisguise_DefaultMirageDisguises, 0x5)
{
	enum { Apply = 0x746A6C };

	GET(UnitClass*, pThis, ESI);

	const auto& disguises = UnitTypeExtContainer::Instance.Find(pThis->Type)
		->DefaultMirageDisguises.GetElements(RulesClass::Instance->DefaultMirageDisguises);

	TerrainTypeClass* pDisguiseAs = nullptr;

	if (disguises)
		pDisguiseAs = disguises[ScenarioClass::Instance->Random.RandomRanged(0, disguises.size() - 1)];
	else
		Debug::FatalErrorAndExit("DefaultDisguise Invalid for TechnoType %s", pThis->Type->ID);

	R->EAX(pDisguiseAs);
	return Apply;
}

int FakeUnitClass::_Mission_Attack()
{
	if (this->BunkerLinkedItem && this->Target)
	{
		auto err = this->GetFireError(this->Target, this->SelectWeapon(this->Target), false);

		if (err == FireError::CANT || err == FireError::RANGE)
		{

			this->SetTarget(nullptr);
			this->EnterIdleMode(false, 1u);

			auto control = this->GetCurrentMissionControl();
			double rate = control->Rate * 900.0;
			return (int)rate + ScenarioClass::Instance->Random.RandomRanged(0, 2);
		}
	}

	return FootClass::Mission_Attack();
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7447A0, FakeUnitClass::_Mission_Attack);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5E80, FakeUnitClass::_Mission_Attack);

ASMJIT_PATCH(0x738703, UnitClass_Explode_ExplodeAnim, 0x5)
{
	GET(AnimTypeClass*, pExplType, EDI);
	GET(UnitClass*, pThis, ESI);

	if (pExplType)
	{

		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExplType, pThis->Location, 0, 1, AnimFlag::AnimFlag_600, 0, false),
			pThis->GetOwningHouse(),
			nullptr,
			false
		);
	}

	return 0x738748;
}
