#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/InfantryType/Body.h>

#include <Misc/AresData.h>

#include <numeric>
/* #183 - cloakable on Buildings and Aircraft */
DEFINE_OVERRIDE_HOOK(0x442CE0, BuildingClass_Init_Cloakable, 0x6)
{
	GET(BuildingClass*, Item, ESI);
	GET(BuildingTypeClass*, pType, EAX);

	if (pType->Cloakable)
	{
		Item->Cloakable = true;
	}

	return 0;
}

// if this is a radar, drop the new owner from the bitfield
DEFINE_OVERRIDE_HOOK(0x448D95, BuildingClass_ChangeOwnership_OldSpy2, 0x8)
{
	GET(HouseClass*, newOwner, EDI);
	GET(BuildingClass*, pThis, ESI);

	if (pThis->DisplayProductionTo.Contains(newOwner))
	{
		pThis->DisplayProductionTo.Remove(newOwner);
	}

	return 0x448DB9;
}

DEFINE_OVERRIDE_HOOK_AGAIN(0x4557BC, BuildingClass_SensorArray_BuildingRedraw, 0x6)
DEFINE_OVERRIDE_HOOK(0x455923, BuildingClass_SensorArray_BuildingRedraw, 0x6)
{
	GET(CellClass*, pCell, ESI);

	// mark detected buildings for redraw
	if (auto pBld = pCell->GetBuilding())
	{
		if (pBld->Owner != HouseClass::CurrentPlayer()
			&& pBld->VisualCharacter(VARIANT_FALSE, nullptr) != VisualType::Normal)
		{
			pBld->NeedsRedraw = true;
		}
	}

	return 0;
}

// capture and mind-control support: deactivate the array for the original
// owner, then activate it a few instructions later for the new owner.
DEFINE_OVERRIDE_HOOK(0x448B70, BuildingClass_ChangeOwnership_SensorArrayA, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->SensorArray)
	{
		pBld->SensorArrayDeactivate();
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x448C3E, BuildingClass_ChangeOwnership_SensorArrayB, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->SensorArray)
	{
		pBld->SensorArrayActivate();
	}

	return 0;
}

// remove sensor on destruction
DEFINE_OVERRIDE_HOOK(0x4416A2, BuildingClass_Destroy_SensorArray, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->Type->SensorArray)
	{
		pBld->SensorArrayDeactivate();
	}

	return 0;
}

// sensor arrays show SensorsSight instead of CloakRadiusInCells
DEFINE_OVERRIDE_HOOK(0x4566F9, BuildingClass_GetRangeOfRadial_SensorArray, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	auto pType = pThis->Type;

	if (pType->SensorArray)
	{
		R->EAX(pType->SensorsSight);
		return 0x45674B;
	}

	return 0x456703;
}

// #1156943: they check for type, and for the instance, yet
// the Log call uses the values as if nothing happened.
DEFINE_OVERRIDE_HOOK(0x4430E8, BuildingClass_Demolish_LogCrash, 0x6)
{
	GET(BuildingClass*, pThis, EDI);
	GET(InfantryClass*, pInf, ESI);

	R->EDX(pThis ? pThis->Type->Name : GameStrings::NoneStr());
	R->EAX(pInf ? pInf->Type->Name : GameStrings::NoneStr());

	return 0x4430FA;
}

// bugfix #231: DestroyAnims don't remap and cause reconnection errors
DEFINE_OVERRIDE_SKIP_HOOK(0x441D25, BuildingClass_Destroy, 0xA, 441D37);

//it seems crashing the game ?
//DEFINE_OVERRIDE_HOOK(0x451E40, BuildingClass_DestroyNthAnim_Destroy, 0x7)
//{
//	GET(BuildingClass*, pThis, ECX);
//	GET_STACK(int, AnimState, 0x4);
//
//	if (AnimState == -2) {
//		for (auto& pAnim : pThis->Anims) {
//			if (pAnim) {
//				pAnim->UnInit();
//				pAnim = nullptr;
//			}
//		}
//	}
//	else
//	{
//		if (auto& pAnim = pThis->Anims[AnimState]) {
//			pAnim->UnInit();
//			pAnim = nullptr;
//		}
//	}
//
//	return 0x451E93;
//}

DEFINE_OVERRIDE_HOOK(0x458E1E, BuildingClass_GetOccupyRangeBonus_Demacroize, 0xA)
{
	auto v1 = R->EDI<int>();
	if (v1 >= R->EAX<int>())
		v1 = R->EAX<int>();

	R->EAX(v1);
	return 0x458E2D;
}

// restore pip count for tiberium storage (building and house)
DEFINE_OVERRIDE_HOOK(0x44D755, BuildingClass_GetPipFillLevel_Tiberium, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	GET(BuildingTypeClass*, pType, ESI);

	double amount = 0.0;
	if (pType->Storage > 0)
	{
		float amounttotal = 0.0f;
		for (auto const& nTib : pThis->Tiberium.Tiberiums)
		{
			amounttotal += nTib;
		}

		amount = amounttotal / pType->Storage;
	}
	else
	{
		float amounttotal = 0.0f;
		for (auto const& nTib : pThis->Owner->OwnedTiberium.Tiberiums)
		{
			amounttotal += nTib;
		}

		amount = amounttotal / pThis->Owner->TotalStorage;
	}

	R->EAX(static_cast<int>(pType->GetPipMax() * amount));
	return 0x44D750;
}

// #814: force sidebar repaint for standard spy effects
DEFINE_OVERRIDE_HOOK_AGAIN(0x4574D2, BuildingClass_Infiltrate_Standard, 0x6)
DEFINE_OVERRIDE_HOOK(0x457533, BuildingClass_Infiltrate_Standard, 0x6)
{
	MouseClass::Instance->SidebarNeedsRepaint();
	return R->Origin() + 6;
}

// infantry exiting hospital get their focus reset, but not for armory
DEFINE_OVERRIDE_HOOK(0x444D26, BuildingClass_KickOutUnit_ArmoryExitBug, 0x6)
{
	GET(BuildingTypeClass*, pType, EDX);
	R->AL(pType->Hospital || pType->Armory);
	return 0x444D2C;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x4449DF, BuildingClass_KickOutUnit_PreventClone, 0x6, 444A53)

DEFINE_OVERRIDE_HOOK(0x44266B, BuildingClass_ReceiveDamage_Destroyed, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pKiller, EBP);
	pThis->Destroyed(pKiller);
	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4586D6, BuildingClass_KillOccupiers, 0x9)
{
	GET(TechnoClass*, pVictim, ECX);
	GET(TechnoClass*, pKiller, EBP);
	pKiller->RegisterDestruction(pVictim);
	return 0x4586DF;
}

// do not crash if the EMP cannon primary has no Report sound
DEFINE_OVERRIDE_HOOK(0x44D4CA, BuildingClass_Mi_Missile_NoReport, 0x9)
{
	GET(TechnoTypeClass*, pType, EAX);
	GET(WeaponTypeClass*, pWeapon, EBP);

	return !pType->IsGattling && pWeapon->Report.Count ?
		0x44D4D4 : 0x44D51F;
}

// for yet unestablished reasons a unit might not be present.
// maybe something triggered the KickOutHospitalArmory
DEFINE_OVERRIDE_HOOK(0x44BB1B, BuildingClass_Mi_Repair_Promote, 0x6)
{
	//GET(BuildingClass*, pThis, EBP);
	GET(TechnoClass*, pTrainee, EAX);
	return pTrainee ? 0 : 0x44BB3C;
}

// remember that this building ejected its survivors already
DEFINE_OVERRIDE_HOOK(0x44A8A2, BuildingClass_Mi_Selling_Crew, 0xA)
{
	GET(BuildingClass* const, pThis, EBP);
	pThis->NoCrew = true;
	return 0;
}

// #1156943, #1156937: replace the engineer check, because they were smart
// enough to use the pointer right before checking whether it's null, and
// even if it isn't, they build a possible infinite loop.
DEFINE_OVERRIDE_HOOK(0x44A5F0, BuildingClass_Mi_Selling_EngineerFreeze, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(InfantryTypeClass*, pType, ESI);
	LEA_STACK(bool*, pEngineerSpawned, 0x13);

	if (*pEngineerSpawned && pType && pType->Engineer)
	{
		// randomize until probability is below 0.01%
		// for only the Engineer tag being returned.
		for (int i = 9; i >= 0; --i)
		{
			pType = !i ? nullptr : pThis->GetCrew();

			if (!pType || !pType->Engineer)
			{
				break;
			}
		}
	}

	if (pType && pType->Engineer)
	{
		*pEngineerSpawned = true;
	}

	R->ESI(pType);
	return 0x44A628;
}

// prevent invisible mcvs, which shouldn't happen any more as the sell/move
// hack is fixed. thus this one is a double unnecessity
DEFINE_OVERRIDE_HOOK(0x449FF8, BuildingClass_Mi_Selling_PutMcv, 7)
{
	GET(UnitClass* const, pUnit, EBX);
	GET(DirType, facing, EAX);
	REF_STACK(CoordStruct const, Crd, STACK_OFFS(0xD0, 0xB8));

	// set the override for putting, not just for creation as WW did
	++Unsorted::IKnowWhatImDoing;
	auto const ret = pUnit->Unlimbo(Crd, facing);
	--Unsorted::IKnowWhatImDoing;

	// should never happen, but if anything breaks, it's here
	if (!ret)
	{
		// do not keep the player alive if it couldn't be placed
		pUnit->UnInit();
	}

	return ret ? 0x44A010u : 0x44A16Bu;
}

// Added more conditions , especially for AI better to set is as Hunt
DEFINE_OVERRIDE_HOOK(0x446E9F, BuildingClass_Place_FreeUnit_Mission, 0x6)
{
	GET(UnitClass*, pFreeUnit, EDI);

	Mission nMissions = Mission::None;
	if ((pFreeUnit->Type->Harvester || pFreeUnit->Type->Weeder)
		&& pFreeUnit->Type->ResourceGatherer)
	{
		nMissions = Mission::Harvest;
	}
	else
	{
		nMissions = (pFreeUnit->Owner && !pFreeUnit->Owner->IsControlledByHuman())
			? Mission::Hunt : Mission::Area_Guard;
	}

	pFreeUnit->QueueMission(nMissions, false);

	return 0x446EAD;
}

// also consider NeedsEngineer when activating animations
// if the status changes, animations might start to play that aren't
// supposed to play because the building requires an Engineer which
// didn't capture the building yet.
DEFINE_OVERRIDE_HOOK(0x4467D6, BuildingClass_Place_NeedsEngineer, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	R->AL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x4467DC;
}

DEFINE_OVERRIDE_HOOK(0x454BF7, BuildingClass_UpdatePowered_NeedsEngineer, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	R->CL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x454BFD;
}

DEFINE_OVERRIDE_HOOK(0x451A54, BuildingClass_PlayAnim_NeedsEngineer, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	R->CL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x451A5A;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x441163, BuildingClass_Put_DontSpawnSpotlight, 0x6, 441196)
DEFINE_OVERRIDE_SKIP_HOOK(0x451132, BuildingClass_ProcessAnims_SuperWeaponsB, 0x6, 451145)
DEFINE_OVERRIDE_SKIP_HOOK(0x44656D, BuildingClass_Place_SuperWeaponAnimsB, 0x6, 446580)

DEFINE_OVERRIDE_HOOK(0x451A28, BuildingClass_PlayAnim_Destroy, 0x7)
{
	GET(AnimClass*, pAnim, ECX);
	pAnim->UnInit();
	return 0x451A2F;
}

// EMP'd power plants don't produce power
DEFINE_OVERRIDE_HOOK(0x44E855, BuildingClass_PowerProduced_EMP, 0x6)
{
	GET(BuildingClass*, pBld, ESI);
	return ((pBld->EMPLockRemaining > 0) ? 0x44E873 : 0);
}

// removing hardcoded references to GAWALL and NAWALL as part of #709
DEFINE_OVERRIDE_HOOK(0x440709, BuildingClass_Unlimbo_RemoveHarcodedWall, 0x6)
{
	GET(CellClass*, Cell, EDI);
	int idxOverlay = Cell->OverlayTypeIndex;
	return idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall ? 0x44071A : 0x440725;
}

DEFINE_OVERRIDE_HOOK(0x45E416, BuildingTypeClass_CTOR_Initialize, 0x6)
{
	GET(BuildingTypeClass*, pThis, ESI);

	pThis->BuildingAnimFrame[3].dwUnknown = 0;
	pThis->BuildingAnimFrame[3].FrameCount = 1;
	pThis->BuildingAnimFrame[3].FrameDuration = 0;
	pThis->double_1728 = 1.0;
	pThis->VoxelBarrelOffsetToPitchPivotPoint = CoordStruct::Empty;
	pThis->VoxelBarrelOffsetToRotatePivotPoint = CoordStruct::Empty;
	pThis->VoxelBarrelOffsetToBuildingPivotPoint = CoordStruct::Empty;
	pThis->VoxelBarrelOffsetToBarrelEnd = CoordStruct::Empty;
	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4456E5, BuildingClass_UpdateConstructionOptions_ExcludeDisabled, 0x6)
{
	GET(BuildingClass*, pBld, ECX);

	// add the EMP check to the limbo check
	return (pBld->InLimbo || pBld->IsUnderEMP()) ?
		0x44583E : 0x4456F3;
}

DEFINE_OVERRIDE_HOOK(0x4368C9, BuildingLightClass_Update_Trigger, 0x5)
{
	GET(TechnoClass*, pTechno, EAX);

	if (pTechno->AttachedTag)
	{
		pTechno->AttachedTag->RaiseEvent(TriggerEvent::EnemyInSpotlight, pTechno, CellStruct::Empty, 0, 0);
	}

	if (pTechno->IsAlive)
	{
		if (pTechno->AttachedTag)
		{
			//66
			pTechno->AttachedTag->RaiseEvent((TriggerEvent)AresNewTriggerEvents::EnemyInSpotlightNow, pTechno, CellStruct::Empty, 0, 0);
		}
	}

	return 0x4368D9;
}

DEFINE_OVERRIDE_HOOK(0x73A1BC, UnitClass_UpdatePosition_EnteredGrinder, 0x7)
{
	GET(UnitClass*, Vehicle, EBP);
	GET(BuildingClass*, Grinder, EBX);

	// TODO : bring  ReverseEngineer in later
	if (AresData::ReverseEngineer(Grinder, Vehicle->Type))
	{
		if (Vehicle->Owner->ControlledByPlayer())
		{
			VoxClass::Play("EVA_ReverseEngineeredVehicle");
			VoxClass::Play("EVA_NewTechnologyAcquired");
		}
	}

	if (auto const pTag = Grinder->AttachedTag)
	{
		pTag->RaiseEvent((TriggerEvent)AresNewTriggerEvents::ReverseEngineerType, Grinder, CellStruct::Empty, false, Vehicle);

		if (auto const pTag2 = Grinder->AttachedTag)
		{
			pTag2->RaiseEvent((TriggerEvent)AresNewTriggerEvents::ReverseEngineerAnything, Grinder, CellStruct::Empty, false, nullptr);
		}
	}

	// #368: refund hijackers
	if (Vehicle->HijackerInfantryType != -1)
	{
		if (InfantryTypeClass* Hijacker =
			InfantryTypeClass::Array->GetItem(Vehicle->HijackerInfantryType))
		{
			Grinder->Owner->GiveMoney(Hijacker->GetRefund(Vehicle->Owner, 0));
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5198AD, InfantryClass_UpdatePosition_EnteredGrinder, 0x6)
{
	GET(InfantryClass*, Infantry, ESI);
	GET(BuildingClass*, Grinder, EBX);

	// TODO : bring  ReverseEngineer in later
	if (AresData::ReverseEngineer(Grinder, Infantry->Type))
	{
		if (Infantry->Owner->ControlledByPlayer())
		{
			VoxClass::Play("EVA_ReverseEngineeredInfantry");
			VoxClass::Play("EVA_NewTechnologyAcquired");
		}
	}

	//Ares 3.0 Added
	if (auto FirstTag = Grinder->AttachedTag)
	{
		//80
		FirstTag->RaiseEvent((TriggerEvent)AresNewTriggerEvents::ReverseEngineerType, Grinder, CellStruct::Empty, false, Infantry);

		//79
		if (auto pSecondTag = Grinder->AttachedTag)
			FirstTag->RaiseEvent((TriggerEvent)AresNewTriggerEvents::ReverseEngineerAnything, Grinder, CellStruct::Empty, false, nullptr);
	}

	return 0;
}

// TODO : bring all here , the real code already complete and tested , for now these should do the trick
// Func arguments is actualli reverse
/*
Action GetiInfiltrateActionResult(BuildingClass* pBuilding, InfantryClass* pInf)
{
	auto const pInfType = pInf->Type;
	auto const pBldType = pBuilding->Type;

	if ((pInfType->Thief || pInf->HasAbility(AbilityType::C4)) && pBldType->CanC4)
		return Action::Self_Deploy;

	const bool IsAgent = pInfType->Agent;
	if (IsAgent && pBldType->Spyable)
	{
		auto pBldOwner = pBuilding->GetOwningHouse();
		auto pInfOwner = pInf->GetOwningHouse();

		if (!pBldOwner || (pBldOwner != pInfOwner && !pBldOwner->IsAlliedWith(pInfOwner)))
			return Action::Move;
	}

	auto const bIsSaboteur = TechnoTypeExt::ExtMap.Find(pInfType)->Saboteur.Get();

	if (bIsSaboteur && BuildingTypeExt::IsSabotagable(pBldType))
		return Action::NoMove;

	return IsAgent || bIsSaboteur || !pBldType->Capturable ? Action::None : Action::Enter;
}*/

DEFINE_OVERRIDE_HOOK(0x7004AD, TechnoClass_GetActionOnObject_Saboteur, 0x6)
{
	// this is known to be InfantryClass, and Infiltrate is yes
	GET(InfantryClass* const, pThis, ESI);
	GET(ObjectClass* const, pObject, EDI);

	bool infiltratable = false;
	if (const auto pBldObject = specific_cast<BuildingClass*>(pObject))
	{
		infiltratable = AresData::GetInfActionOverObject(pThis, pBldObject) != Action::None;
	}

	return infiltratable ? 0x700531u : 0x700536u;
}

/* TODO : require MouseCursorType Set Here
DEFINE_HOOK(51EE6B, InfantryClass_GetActionOnObject_Saboteur, 6)
{
	enum
	{
		infiltratable = 0x51EEEDu
		, Notinfiltratable = 0x51F04E
	};

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);

	if (auto pBldObject = abstract_cast<BuildingClass*>(pObject))
	{
		if (!pThis->Owner->IsAlliedWith(pBldObject))
		{
			switch (GetiInfiltrateActionResult(pBldObject, pThis))
			{
			case Action::Move:
				//MouseCursor::SetAction(*(*(pInf[328] + 3620) + 576), 9, 0);
				break;
			case Action::NoMove:
				//MouseCursor::SetAction(93, 9, 0);
				break;
			case Action::None:
				return Notinfiltratable;
			}

			return infiltratable;
		}
	}

	return Notinfiltratable;
}*/

DEFINE_OVERRIDE_HOOK(0x51B2CB, InfantryClass_SetTarget_Saboteur, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(ObjectClass*, pTarget, EDI);

	if (const auto pBldObject = specific_cast<BuildingClass*>(pTarget))
	{
		const auto nResult = AresData::GetInfActionOverObject(pThis, pBldObject);

		if (nResult == Action::Move || nResult == Action::NoMove || nResult == Action::Enter)
			pThis->SetDestination(pTarget, true);
	}

	return 0x51B33F;
}

DEFINE_OVERRIDE_HOOK(0x519FF8, InfantryClass_UpdatePosition_Saboteur, 6)
{
	enum
	{
		SkipInfiltrate = 0x51A03E,
		Infiltrate_Vanilla = 0x51A002,
		InfiltrateSucceded = 0x51A010,
	};

	GET(InfantryClass* const, pThis, ESI);
	GET(BuildingClass* const, pBuilding, EDI);

	const auto nResult = AresData::GetInfActionOverObject(pThis, pBuilding);

	if (nResult == Action::Move) // this one will Infiltrate instead
	{
		auto const pHouse = pThis->Owner;
		if (!pThis->Type->Agent || pHouse->IsAlliedWith(pBuilding))
			return SkipInfiltrate;

		pBuilding->Infiltrate(pHouse);
		BuildingExt::HandleInfiltrate(pBuilding, pHouse);
		return InfiltrateSucceded;
	}
	else
	if (nResult == Action::NoMove)
	{
		const auto pInfext = InfantryTypeExt::ExtMap.Find(pThis->Type);

		if (pBuilding->IsIronCurtained() || pBuilding->IsBeingWarpedOut()
			|| pBuilding->GetCurrentMission() == Mission::Selling)
		{
			pThis->AbortMotion();
			pThis->Uncloak(false);
			const int Rof = pInfext->C4ROF.Get(pThis->GetROF(1));
			pThis->ReloadTimer.Start(Rof);
			return SkipInfiltrate;
		}
		else if (pBuilding->C4Applied)
		{
			const int Rof = pInfext->C4ROF.Get(pThis->GetROF(1));
			pThis->ReloadTimer.Start(Rof);
			return SkipInfiltrate;
		}

		// sabotage
		pBuilding->C4Applied = true;
		pBuilding->C4AppliedBy = pThis;

		const auto pData = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		const auto delay = pInfext->C4Delay.Get(RulesClass::Instance->C4Delay);

		auto duration = (int)(delay * 900.0);

		// modify good durations only
		if (duration > 0)
		{
			duration = (int)(duration * pData->C4_Modifier);
			if (duration <= 0)
				duration = 1;
		}

		pBuilding->Flash(duration / 2);
		pBuilding->C4Timer.Start(duration);

		if (auto const pTag = pBuilding->AttachedTag)
		{
			pTag->RaiseEvent(TriggerEvent::EnteredBy, pThis, CellStruct::Empty, false, nullptr);
		}

		return InfiltrateSucceded;
	}

	return SkipInfiltrate;

}

/*	Hook pack for DockUnload , put them onto one hook file for easy diagnostic later on -Otamaa */

namespace Get
{
	DirStruct UnloadFacing(UnitClass* pThis)
	{
		DirStruct nResult;
		nResult.Raw = 0x4000;

		if (pThis->HasAnyLink())
		{
			if (const auto pBld = specific_cast<BuildingClass*>(pThis->RadioLinks.Items[0]))
			{
				auto const pBldExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
				if (pBldExt->DockUnload_Facing.isset())
					nResult.Raw = ((size_t)pBldExt->DockUnload_Facing.Get()) << 11;
			}
		}

		return nResult;
	}

	CellStruct UnloadCell(BuildingClass* pThis)
	{
		return BuildingTypeExt::ExtMap.Find(pThis->Type)->DockUnload_Cell;
	}

	BuildingClass* BuildingUnload(UnitClass* pThis)
	{
		if (const auto pBld = specific_cast<BuildingClass*>(pThis->RadioLinks.Items[0]))
		{
			const auto pBldCells = pBld->InlineMapCoords();
			const auto pThisCells = pThis->InlineMapCoords();

			if ((pBldCells + UnloadCell(pBld)) == pThisCells)
			{
				return pBld;
			}
		}

		return nullptr;
	}
}

DEFINE_OVERRIDE_HOOK(0x7376D9, UnitClass_ReceivedRadioCommand_DockUnload_Facing, 5)
{
	GET(UnitClass*, pUnit, ESI);
	GET(DirStruct*, nCurrentFacing, EAX);

	auto nDecidedFacing = Get::UnloadFacing(pUnit);

	if (*nCurrentFacing == nDecidedFacing)
		return 0x73771B;

	pUnit->Locomotor.get()->Do_Turn(nDecidedFacing);

	return 0x73770C;
}

DEFINE_OVERRIDE_HOOK(0x73DF66, UnitClass_Mi_Unload_DockUnload_Facing, 5)
{
	GET(UnitClass*, pUnit, ESI);
	GET(DirStruct*, nCurrentFacing, EAX);

	auto nDecidedFacing = Get::UnloadFacing(pUnit);

	if (*nCurrentFacing == nDecidedFacing || pUnit->IsRotating)
		return 0x73DFBD;

	pUnit->Locomotor.get()->Do_Turn(nDecidedFacing);

	return 0x73DFB0;
}

DEFINE_OVERRIDE_HOOK(0x43CA80, BuildingClass_ReceivedRadioCommand_DockUnloadCell, 7)
{
	GET(CellStruct*, pCell, EAX);
	GET(BuildingClass*, pThis, ESI);

	auto nBuff = Get::UnloadCell(pThis);
	R->DX(pCell->X + nBuff.X);
	R->AX(pCell->Y + nBuff.Y);

	return 0x43CA8D;
}

DEFINE_OVERRIDE_HOOK(0x73E013, UnitClass_Mi_Unload_DockUnloadCell1, 6)
{
	GET(UnitClass*, pThis, ESI);
	R->EAX(Get::BuildingUnload(pThis));
	return 0x73E05F;
}

DEFINE_OVERRIDE_HOOK(0x73E17F, UnitClass_Mi_Unload_DockUnloadCell2, 6)
{
	GET(UnitClass*, pThis, ESI);
	R->EAX(Get::BuildingUnload(pThis));
	return 0x73E1CB;
}

DEFINE_OVERRIDE_HOOK(0x73E2BF, UnitClass_Mi_Unload_DockUnloadCell3, 6)
{
	GET(UnitClass*, pThis, ESI);
	R->EAX(Get::BuildingUnload(pThis));
	return 0x73E30B;
}

DEFINE_OVERRIDE_HOOK(0x741BDB, UnitClass_SetDestination_DockUnloadCell, 7)
{
	GET(UnitClass*, pThis, EBP);
	R->EAX(Get::BuildingUnload(pThis));
	return 0x741C28;
}

#include <Misc/AresData.h>
#include <Utilities/Cast.h>

DEFINE_OVERRIDE_HOOK(0x4F7870, HouseClass_CanBuild, 7)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
/* return
	 1 - cameo shown
	 0 - cameo not shown
	-1 - cameo greyed out
 */

	GET(HouseClass* const, pThis, ECX);
	GET_STACK(TechnoTypeClass* const, pItem, 0x4);
	GET_STACK(bool const, buildLimitOnly, 0x8);
	GET_STACK(bool const, includeInProduction, 0xC);

	const auto nAresREsult = AresData::PrereqValidate(pThis, pItem, buildLimitOnly, includeInProduction);
	
	if (const auto pBuilding = type_cast<BuildingTypeClass*, true>(pItem)) {
		if (!BuildingTypeExt::ExtMap.Find(pBuilding)->PowersUp_Buildings.empty())
		{  if (nAresREsult == CanBuildResult::Buildable) {	
				R->EAX(BuildingTypeExt::CheckBuildLimit(pThis, pBuilding, includeInProduction));
				return 0x4F8361;
			}
		}
	}

	R->EAX(nAresREsult);
	return 0x4F8361;
}

#define Is_FirestromWall(techno) \
(*(bool*)((char*)GetAresBuildingTypeExt(techno) + 0x5D))

#define Is_Passable(techno) \
(*(bool*)((char*)GetAresBuildingTypeExt(techno) + 0x5E))

DEFINE_OVERRIDE_HOOK(0x73F7B0, UnitClass_IsCellOccupied, 6)
{
	GET(BuildingClass* const, pBld, ESI);

	enum
	{
		Impassable = 0x73FCD0, // return Move_No
		Ignore = 0x73FA87, // check next object
		NoDecision = 0x73F7D3, // check other
		CheckFirestormActive = 0x73F7BA // check if the object owner has FirestromActive flag
	};

	if (Is_Passable(pBld->Type))
	{
		return Ignore;
	}

	if (Is_FirestromWall(pBld->Type)) {
		return CheckFirestormActive;
	}

	return NoDecision;
}

DEFINE_OVERRIDE_HOOK(0x7413FF, UnitClass_Fire_Ammo, 7)
{
	GET(UnitClass*, pThis, ESI);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x20, 0x8));
	const auto pWP = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWP->Ammo > 0)
		pThis->StartReloading();

	return 0x741406;
}

// the game specifically hides tiberium building pips. allow them, but
// take care they don't show up for the original game
DEFINE_OVERRIDE_HOOK(0x709B4E, TechnoClass_DrawPipscale_SkipSkipTiberium, 6)
{
	GET(TechnoClass*, pThis, EBP);

	bool showTiberium = true;
	if (Is_Building(pThis))
	{
		const auto pBld = static_cast<BuildingClass*>(pThis);
		if ((pBld->Type->Refinery || pBld->Type->ResourceDestination) && pBld->Type->Storage > 0)
		{
			// show only if this refinery uses storage. otherwise, the original
			// refineries would show an unused tiberium pip scale
			showTiberium = TechnoTypeExt::ExtMap.Find(pBld->Type)->Refinery_UseStorage;
		}
	}

	return showTiberium ? 0x709B6E : 0x70A980;
}
