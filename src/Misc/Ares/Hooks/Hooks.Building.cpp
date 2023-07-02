#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Utilities/Cast.h>

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

// #1156943: they check for type, and for the instance, yet
// the Log call uses the values as if nothing happened.
DEFINE_OVERRIDE_HOOK(0x4430E8, BuildingClass_Destroyed_SurvivourLog, 0x6)
{
	GET(BuildingClass* const, pThis, EDI);
	GET(InfantryClass* const, pInf, ESI);

	const auto pBldID = pThis ? pThis->Type->Name : GameStrings::NoneStr();
	const auto pInfID = pInf ? pInf->Type->Name : GameStrings::NoneStr();
	const auto pOwnedID = pThis && pThis->Owner && pThis->Owner->Type ? pThis->Owner->Type->ID : GameStrings::NoneStr();

	Debug::Log("[%x][%s - %s] Creating survivor type '%s' \n", pThis, pBldID, pOwnedID, pInfID);
	return 0x443109;
}

/* #183 - cloakable on Buildings and Aircraft */
DEFINE_OVERRIDE_HOOK(0x442CE0, BuildingClass_Init_Cloakable, 0x6)
{
	GET(BuildingClass*, Item, ESI);

	if (Item->Type->Cloakable) {
		Item->Cloakable = true;
	}

	return 0;
}

// if this is a radar, drop the new owner from the bitfield
DEFINE_OVERRIDE_HOOK(0x448D95, BuildingClass_ChangeOwnership_OldSpy2, 0x8)
{
	GET(HouseClass* const, newOwner, EDI);
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
	GET(CellClass* const, pCell, ESI);

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
	GET(BuildingClass* , pBld, ESI);

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
	GET(BuildingClass* const , pThis, ESI);

	if (pThis->Type->SensorArray)
	{
		R->EAX(pThis->Type->SensorsSight);
		return 0x45674B;
	}

	return 0x456703;
}

// #1156943: they check for type, and for the instance, yet
// the Log call uses the values as if nothing happened.
DEFINE_OVERRIDE_HOOK(0x4430E8, BuildingClass_Demolish_LogCrash, 0x6)
{
	GET(BuildingClass* const, pThis, EDI);
	GET(InfantryClass* const, pInf, ESI);

	R->EDX(pThis ? pThis->Type->Name : GameStrings::NoneStr());
	R->EAX(pInf ? pInf->Type->Name : GameStrings::NoneStr());

	return 0x4430FA;
}

// bugfix #231: DestroyAnims don't remap and cause reconnection errors
DEFINE_OVERRIDE_SKIP_HOOK(0x441D25, BuildingClass_Destroy, 0xA, 441D37);

DEFINE_OVERRIDE_HOOK(0x451E40, BuildingClass_DestroyNthAnim_Destroy, 0x7)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(int, AnimState, 0x4);

	if (AnimState == -2) {
		for (auto& pAnim : pThis->Anims) {
			if (pAnim) {
				pAnim->UnInit();
				pAnim = nullptr;
			}
		}
	}
	else
	{
		if (auto& pAnim = pThis->Anims[AnimState]) {
			pAnim->UnInit();
			pAnim = nullptr;
		}
	}

	return 0x451E93;
}

DEFINE_OVERRIDE_HOOK(0x451A28, BuildingClass_PlayAnim_Destroy, 0x7)
{
	//GET(BuildingClass* const , pThis , ESI);

	GET(AnimClass*, pAnim, ECX);
	pAnim->UnInit();
	return 0x451A2F;
}

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
	GET(BuildingClass* const , pThis, ECX);
	GET(BuildingTypeClass* const, pType, ESI);

	double amount = 0.0;
	if (pType->Storage > 0) {
		amount = pThis->Tiberium.GetStoragePercentage(pType->Storage);
	} else {
		amount = pThis->Owner->OwnedTiberium.GetStoragePercentage(pThis->Owner->TotalStorage);
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
	GET(BuildingTypeClass* const, pType, EDX);
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
	GET(TechnoTypeClass* const, pType, EAX);
	GET(WeaponTypeClass* const, pWeapon, EBP);

	return !pType->IsGattling && pWeapon->Report.Count ?
		0x44D4D4 : 0x44D51F;
}

// for yet unestablished reasons a unit might not be present.
// maybe something triggered the KickOutHospitalArmory
DEFINE_OVERRIDE_HOOK(0x44BB1B, BuildingClass_Mi_Repair_Promote, 0x6)
{
	//GET(BuildingClass*, pThis, EBP);
	GET(TechnoClass* const, pTrainee, EAX);
	return pTrainee ? 0 : 0x44BB3C;
}

// remember that this building ejected its survivors already
DEFINE_OVERRIDE_HOOK(0x44A8A2, BuildingClass_Mi_Selling_Crew, 0xA)
{
	GET(BuildingClass*, pThis, EBP);
	pThis->NoCrew = true;
	return 0;
}

// #1156943, #1156937: replace the engineer check, because they were smart
// enough to use the pointer right before checking whether it's null, and
// even if it isn't, they build a possible infinite loop.
DEFINE_OVERRIDE_HOOK(0x44A5F0, BuildingClass_Mi_Selling_EngineerFreeze, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
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
	++Unsorted::ScenarioInit;
	const auto ret = pUnit->Unlimbo(Crd, facing);
	--Unsorted::ScenarioInit;

	// should never happen, but if anything breaks, it's here
	if (!ret)
	{
		GameDelete<true,false>(pUnit);
		// do not keep the player alive if it couldn't be placed
		//pUnit->UnInit();
	}

	return ret ? 0x44A010u : 0x44A16Bu;
}

// Added more conditions , especially for AI better to set is as Hunt
DEFINE_OVERRIDE_HOOK(0x446E9F, BuildingClass_Place_FreeUnit_Mission, 0x6)
{
	GET(UnitClass* const, pFreeUnit, EDI);
	Mission nMissions;

	if(!pFreeUnit->Owner){
		nMissions = Mission::Sleep;
	}else {
		if (pFreeUnit->Type->Harvester ||
			pFreeUnit->Type->Weeder ||
			pFreeUnit->Type->ResourceGatherer)
		{ nMissions = Mission::Harvest; }
		else {
			nMissions = !pFreeUnit->Owner->IsControlledByHuman()
				? Mission::Hunt : Mission::Area_Guard;
		}
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
	GET(BuildingClass* const, pThis, EBP);
	R->AL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x4467DC;
}

DEFINE_OVERRIDE_HOOK(0x454BF7, BuildingClass_UpdatePowered_NeedsEngineer, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->CL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x454BFD;
}

DEFINE_OVERRIDE_HOOK(0x451A54, BuildingClass_PlayAnim_NeedsEngineer, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->CL(pThis->Type->Powered || (pThis->Type->NeedsEngineer && !pThis->HasEngineer));
	return 0x451A5A;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x441163, BuildingClass_Put_DontSpawnSpotlight, 0x6, 441196)
DEFINE_OVERRIDE_SKIP_HOOK(0x451132, BuildingClass_ProcessAnims_SuperWeaponsB, 0x6, 451145)
DEFINE_OVERRIDE_SKIP_HOOK(0x44656D, BuildingClass_Place_SuperWeaponAnimsB, 0x6, 446580)

// EMP'd power plants don't produce power
DEFINE_OVERRIDE_HOOK(0x44E855, BuildingClass_PowerProduced_EMP, 0x6)
{
	GET(BuildingClass* const, pBld, ESI);
	return ((pBld->EMPLockRemaining > 0) ? 0x44E873 : 0);
}

// removing hardcoded references to GAWALL and NAWALL as part of #709
DEFINE_OVERRIDE_HOOK(0x440709, BuildingClass_Unlimbo_RemoveHarcodedWall, 0x6)
{
	GET(CellClass* const , Cell, EDI);
	const int idxOverlay = Cell->OverlayTypeIndex;
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
	GET(BuildingClass* const, pBld, ECX);

	// add the EMP check to the limbo check
	return (pBld->InLimbo || pBld->IsUnderEMP()) ?
		0x44583E : 0x4456F3;
}

//void AddPassengers(std::vector<TechnoClass*>& colle , TechnoClass* Vic)
//{
//	for(auto nPass = Vic->Passengers.GetFirstPassenger();
//		nPass;
//		nPass = (FootClass*)nPass->NextObject)
//	{
//		if (TechnoTypeExt::ExtMap.Find(nPass->GetTechnoType())->CanBeReversed) {
//			colle.push_back(nPass);
//		}
//
//		AddPassengers(colle, nPass);
//	}
//}

// https://bugs.launchpad.net/ares/+bug/1925359
//bool ReverseEngineer(BuildingClass* pBuilding , TechnoClass* Victim) {
//	auto pReverseData = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
//
//	if (!pReverseData->ReverseEngineersVictims || !pBuilding->Owner) {
//		return false;
//	}
//
//	std::vector<TechnoClass*> Victims;
//
//	if(TechnoTypeExt::ExtMap.Find(Victim->GetTechnoType())->CanBeReversed)
//		Victims.push_back(Victim);
//
//	AddPassengers(Victims,Victim);
//	HouseClass* Owner = pBuilding->Owner;
//	auto& nVec = ReverseEngineeredTechnoType(Owner);
//
//	for(auto nColle : Victims) {
//
//		const auto VictimType = nColle->GetTechnoType();
//		const auto pVictimTypeExt = TechnoTypeExt::ExtMap.Find(VictimType);
//		const auto pVictimAs = pVictimTypeExt->ReversedAs.Get(VictimType);
//
//		if(std::find_if(std::begin(nVec), std::end(nVec), [&](TechnoTypeClass* pTech) { return pTech == pVictimAs; }) == std::end(nVec)) {
//			if (!(AresData::PrereqValidate(Owner, pVictimAs, false, true) == CanBuildResult::Buildable)) {
//
//				nVec.push_back(pVictimAs);
//
//				if (AresData::RequirementsMet(Owner, pVictimAs) >= 2) {
//
//					Owner->RecheckTechTree = true;
//
//					if (nColle->Owner && nColle->Owner->ControlledByPlayer()) {
//						VoxClass::Play(nColle->WhatAmI() == InfantryClass::AbsID ? "EVA_ReverseEngineeredInfantry" : "EVA_ReverseEngineeredVehicle");
//						VoxClass::Play(GameStrings::EVA_NewTechAcquired());
//					}
//
//					if (auto FirstTag = pBuilding->AttachedTag) {
//						FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, pBuilding, CellStruct::Empty, false, nColle);
//
//						if (auto pSecondTag = pBuilding->AttachedTag)
//							FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, pBuilding, CellStruct::Empty, false, nullptr);
//					}
//				}
//			}
//		}
//	}
//
//	return true;
//}

//https://bugs.launchpad.net/ares/+bug/1925359
void AddPassengers(BuildingClass* const Grinder, TechnoClass* Vic)
{
	for (auto nPass = Vic->Passengers.GetFirstPassenger();
		nPass;
		nPass = (FootClass*)nPass->NextObject)
	{
		const auto pType = nPass->GetTechnoType();

		if (AresData::ReverseEngineer(Grinder, pType)) {
			if (nPass->Owner && nPass->Owner->ControlledByPlayer()) {
				VoxClass::Play(nPass->WhatAmI() == InfantryClass::AbsID ? "EVA_ReverseEngineeredInfantry" : "EVA_ReverseEngineeredVehicle");
				VoxClass::Play(GameStrings::EVA_NewTechAcquired());
			}
		}

		if (const auto FirstTag = Grinder->AttachedTag)
		{
			FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, Grinder, CellStruct::Empty, false, nPass);

			if (auto pSecondTag = Grinder->AttachedTag)
			{
				pSecondTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, Grinder, CellStruct::Empty, false, nullptr);
			}
		}

		AddPassengers(Grinder, nPass);
	}
}

DEFINE_OVERRIDE_HOOK(0x73A1BC, UnitClass_UpdatePosition_EnteredGrinder, 0x7)
{
	GET(UnitClass* const, Vehicle, EBP);
	GET(BuildingClass* const, Grinder, EBX);

	//ReverseEngineer(Grinder, Vehicle);

	// TODO : bring  ReverseEngineer in later
	if (AresData::ReverseEngineer(Grinder, Vehicle->Type))
	{
		if (Vehicle->Owner && Vehicle->Owner->ControlledByPlayer())
		{
			VoxClass::Play("EVA_ReverseEngineeredVehicle");
			VoxClass::Play(GameStrings::EVA_NewTechAcquired());
		}
	}

	if (const auto FirstTag = Grinder->AttachedTag)
	{
		FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, Grinder, CellStruct::Empty, false, Vehicle);

		if (auto pSecondTag = Grinder->AttachedTag)
		{
			pSecondTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, Grinder, CellStruct::Empty, false, nullptr);
		}
	}

	// https://bugs.launchpad.net/ares/+bug/1925359
	AddPassengers(Grinder, Vehicle);

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
	GET(InfantryClass* const, Infantry, ESI);
	GET(BuildingClass* const, Grinder, EBX);

	//ReverseEngineer(Grinder, Infantry);

	// TODO : bring  ReverseEngineer in later
	if (AresData::ReverseEngineer(Grinder, Infantry->Type))
	{
		if (Infantry->Owner->ControlledByPlayer())
		{
			VoxClass::Play("EVA_ReverseEngineeredInfantry");
			VoxClass::Play(GameStrings::EVA_NewTechAcquired());
		}
	}

	//Ares 3.0 Added
	if (const auto FirstTag = Grinder->AttachedTag)
	{
		//80
		FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, Grinder, CellStruct::Empty, false, Infantry);

		//79
		if (const auto pSecondTag = Grinder->AttachedTag)
			pSecondTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, Grinder, CellStruct::Empty, false, nullptr);
	}

	return 0;
}

bool NOINLINE IsSabotagable(BuildingClass const* const pThis)
{
	auto const pType = pThis->Type;
	auto const pExt = BuildingTypeExt::ExtMap.Find(pType);
	auto const civ_occupiable = pType->CanBeOccupied && pType->TechLevel == -1;
	auto const default_sabotabable = pType->CanC4 && !civ_occupiable;

	return !pExt->ImmuneToSaboteurs.Get(!default_sabotabable);
}

Action NOINLINE GetiInfiltrateActionResult(InfantryClass* pInf , BuildingClass* pBuilding)
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

	if (bIsSaboteur && IsSabotagable(pBuilding))
		return Action::NoMove;

	return IsAgent || bIsSaboteur || !pBldType->Capturable ? Action::None : Action::Enter;
}

DEFINE_OVERRIDE_HOOK(0x7004AD, TechnoClass_GetActionOnObject_Saboteur, 0x6)
{
	// this is known to be InfantryClass, and Infiltrate is yes
	GET(InfantryClass* const, pThis, ESI);
	GET(ObjectClass* const, pObject, EDI);

	bool infiltratable = false;
	if (const auto pBldObject = specific_cast<BuildingClass*>(pObject))
	{
		infiltratable = GetiInfiltrateActionResult(pThis, pBldObject) != Action::None;
	}

	return infiltratable ? 0x700531u : 0x700536u;
}

DEFINE_OVERRIDE_HOOK(0x51EE6B, InfantryClass_GetActionOnObject_Saboteur, 6)
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
			const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBldObject->Type);

			switch (GetiInfiltrateActionResult(pThis , pBldObject))
			{
			case Action::Move:
			{
				AresData::SetMouseCursorAction(pTypeExt->Cursor_Spy, Action::Capture, 0);
				break;
			}
			case Action::NoMove:
				AresData::SetMouseCursorAction(pTypeExt->Cursor_Sabotage, Action::Capture, 0);
				break;
			case Action::None:
				return Notinfiltratable;
			}

			return infiltratable;
		}
	}

	return Notinfiltratable;
}

DEFINE_OVERRIDE_HOOK(0x51E635, InfantryClass_GetActionOnObject_EngineerOverFriendlyBuilding, 5)
{
	enum
	{
		DontRepair = 0x51E63A,
		DoRepair = 0x51E659,
		SkipAll = 0x51E458,
	};

	GET(BuildingClass* const, pTarget, ESI);
	GET(InfantryClass* const, pThis, EDI);

	const auto pData = BuildingTypeExt::ExtMap.Find(pTarget->Type);

	if ((pData->RubbleIntact || pData->RubbleIntactRemove) && pTarget->Owner->IsAlliedWith_(pThis))
	{
		AresData::SetMouseCursorAction(90u, Action::GRepair, false);
		R->EAX(Action::GRepair);
		return SkipAll;
	}

	return ((R->EAX<DWORD>() & 0x4000) != 0) ? DontRepair : DoRepair;
}

DEFINE_OVERRIDE_HOOK(0x51FA82, InfantryClass_GetActionOnCell_EngineerRepairable, 6)
{
	GET(BuildingTypeClass* const , pBuildingType, EBP);
	R->AL(BuildingTypeExt::ExtMap.Find(pBuildingType)
		->EngineerRepairable.Get(pBuildingType->Repairable));
	return 0x51FA88;
}

DEFINE_OVERRIDE_HOOK(0x51E4ED, InfantryClass_GetActionOnObject_EngineerRepairable, 6)
{
	GET(BuildingClass* const, pBuilding, ESI);
	R->CL(BuildingTypeExt::ExtMap.Find(pBuilding->Type)
		->EngineerRepairable.Get(pBuilding->Type->Repairable));
	return 0x51E4F3;
}

// placement linking
DEFINE_OVERRIDE_HOOK(0x6D5455, TacticalClass_DrawPlacement_IsLInkable, 6)
{
	GET(BuildingTypeClass* const, pType, EAX);
	return BuildingTypeExt::IsLinkable(pType) ?
		0x6D545Fu : 0x6D54A9u;
}

// placement linking
DEFINE_OVERRIDE_HOOK(0x6D5A5C, TacticalClass_DrawPlacement_FireWall_IsLInkable, 6)
{
	GET(BuildingTypeClass* const, pType, EDX);
	return BuildingTypeExt::IsLinkable(pType) ?
		0x6D5A66u : 0x6D5A75u;
}

DEFINE_OVERRIDE_HOOK(0x51B2CB, InfantryClass_SetTarget_Saboteur, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(ObjectClass* const, pTarget, EDI);

	if (const auto pBldObject = specific_cast<BuildingClass*>(pTarget))
	{
		const auto nResult = GetiInfiltrateActionResult(pThis, pBldObject);

		if (nResult == Action::Move || nResult == Action::NoMove || nResult == Action::Enter)
			pThis->SetDestination(pTarget, true);
	}

	return 0x51B33F;
}

bool ApplyC4ToBuilding(InfantryClass* const pThis, BuildingClass* const pBuilding, const bool IsSaboteur)
{
	const auto pInfext = InfantryTypeExt::ExtMap.Find(pThis->Type);

	if (pBuilding->IsIronCurtained() || pBuilding->IsBeingWarpedOut()
		|| pBuilding->GetCurrentMission() == Mission::Selling)
	{
		pThis->AbortMotion();
		pThis->Uncloak(false);
		const int Rof = pInfext->C4ROF.Get(pThis->GetROF(1));
		pThis->ReloadTimer.Start(Rof);
		if (!IsSaboteur)
		{
			pThis->Scatter(pBuilding->GetCoords(), true, true);
		}
		return false;
	}
	else
	if (pBuilding->IsGoingToBlow)
	{
		const int Rof = pInfext->C4ROF.Get(pThis->GetROF(1));
		pThis->ReloadTimer.Start(Rof);
		if (!IsSaboteur)
		{
			pThis->AbortMotion();
			//need to set target ?
			pThis->SetDestination(nullptr, true);
			pThis->Scatter(pBuilding->GetCoords(), true, true);
		}
		return false;
	}

	// sabotage
	pBuilding->IsGoingToBlow = true;
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

	//auto pBldExt = BuildingExt::ExtMap.Find(pBuilding);
	//if (pInfext->C4Damage.isset())
	//{
	//	pBldExt->C4Damage = pInfext->C4Damage;
	//}
	//
	//pBldExt->C4Warhead = pInfext->C4Warhead.Get(RulesClass::Instance->C4Warhead);
	//pBldExt->C4Owner = pThis->GetOwningHouse();
	pBuilding->Flash(duration / 2);
	pBuilding->GoingToBlowTimer.Start(duration);

	if (!IsSaboteur)
	{
		pThis->SetDestination(nullptr, true);
		pThis->Scatter(pBuilding->GetCoords(), true, true);
	}

	return true;
}

DEFINE_HOOK(0x51A521, InfantryClass_UpdatePosition_ApplyC4, 0xA)
{
	enum { RetFail = 0x51A59D, RetSucceeded = 0x51A65D };

	GET(InfantryClass* const, pThis, ESI);
	GET(BuildingClass* const, pBuilding, EDI);

	if (!ApplyC4ToBuilding(pThis, pBuilding, false))
		return RetFail;

	return RetSucceeded;
}

struct AresBldExtStuffs
{
	static int GetFirstSuperWeaponIndex(BuildingClass* pThis)
	{
		const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
		const auto count = pExt->GetSuperWeaponCount();
		for (auto i = 0; i < count; ++i)
		{
			const auto idxSW = pExt->GetSuperWeaponIndex(i, pThis->Owner);
			if (idxSW != -1)
			{
				return idxSW;
			}
		}
		return -1;
	}

	static void UpdateDisplayTo(BuildingClass* pThis)
	{
		if (pThis->Type->Radar)
		{
			auto pHouse = pThis->Owner;
			pHouse->RadarVisibleTo.Clear();

			pHouse->RadarVisibleTo.data |= RadarPresist(pHouse).data;

			for (auto pBld : pHouse->Buildings)
			{
				if (!pBld->InLimbo)
				{
					if (BuildingTypeExt::ExtMap.Find(pBld->Type)->SpyEffect_RevealRadar)
					{
						pHouse->RadarVisibleTo.data |= pBld->DisplayProductionTo.data;
					}
				}
			}
			MapClass::Instance->RedrawSidebar(2);
		}
	}

	static bool InfiltratedBy(BuildingClass* EnteredBuilding, HouseClass* Enterer)
	{
		auto EnteredType = EnteredBuilding->Type;
		auto Owner = EnteredBuilding->Owner;
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(EnteredBuilding->Type);
		//auto pEntererExt = HouseExt::ExtMap.Find(Enterer);
		//auto pEnteredExt = HouseExt::ExtMap.Find(Owner);

		if (!pTypeExt->SpyEffect_Custom) {
			return false;
		}

		bool raiseEva = false;
		const bool IsOwnerControlledByPlayer = Owner->ControlledByPlayer();
		const bool IsEntererControlledByPlayer = Enterer->ControlledByPlayer();

		if (IsEntererControlledByPlayer || IsOwnerControlledByPlayer)
		{
			CellStruct xy = CellClass::Coord2Cell(EnteredBuilding->GetCoords());
			if (RadarEventClass::Create(RadarEventType::BuildingInfiltrated, xy))
			{
				raiseEva = true;
			}
		}

		const bool evaForOwner = IsOwnerControlledByPlayer && raiseEva;
		const bool evaForEnterer = IsEntererControlledByPlayer && raiseEva;
		bool effectApplied = false;

		if (pTypeExt->SpyEffect_ResetRadar)
		{
			Owner->ReshroudMap();
			if (!Owner->SpySatActive && evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_RadarSabotaged);
			}
			if (!Owner->SpySatActive && evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfRadarSabotaged);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_PowerOutageDuration > 0)
		{
			Owner->CreatePowerOutage(pTypeExt->SpyEffect_PowerOutageDuration);
			if (evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_PowerSabotaged);
			}
			if (evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				VoxClass::Play(GameStrings::EVA_EnemyBasePoweredDown);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_StolenTechIndex > -1)
		{
			StolenTechnoType(Enterer).set(static_cast<size_t>(pTypeExt->SpyEffect_StolenTechIndex));
			Enterer->RecheckTechTree = true;
			if (evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_TechnologyStolen);
			}
			if (evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				VoxClass::Play(GameStrings::EVA_NewTechAcquired);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_UnReverseEngineer)
		{
			Debug::Log("Undoing all Reverse Engineering achieved by house %ls\n", Owner->UIName);
			ReverseEngineeredTechnoType(Owner).clear();
			Owner->RecheckTechTree = true;

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_ResetSW)
		{
			bool somethingReset = false;
			const auto buildingSWCount = pTypeExt->GetSuperWeaponCount();
			for (auto i = 0; i < buildingSWCount; ++i)
			{
				if (auto pSuper = Owner->Supers.GetItemOrDefault(i))
				{
					pSuper->Reset();
					somethingReset = true;
				}
			}

			for (int i = 0; i < EnteredType->Upgrades; ++i)
			{
				if (auto Upgrade = EnteredBuilding->Upgrades[i])
				{
					auto UpgradeExt = BuildingTypeExt::ExtMap.Find(Upgrade);
					const auto upgradeSWCount = UpgradeExt->GetSuperWeaponCount();

					for (auto j = 0; j < upgradeSWCount; ++j)
					{
						int swIdx = UpgradeExt->GetSuperWeaponIndex(j, Owner);
						if (swIdx != -1)
						{
							Owner->Supers.Items[swIdx]->Reset();
							somethingReset = true;
						}
					}
				}
			}

			if (somethingReset)
			{
				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}
				effectApplied = true;
			}
		}

		// Did you mean for not launching for real or not, Morton?
		auto launchTheSWHere = [EnteredBuilding](int const idx, HouseClass* const pHouse, bool realLaunch = false)
		{
			if (const auto pSuper = pHouse->Supers.GetItem(idx))
			{
				if (!realLaunch || (pSuper->Granted && pSuper->IsCharged && !pSuper->IsOnHold))
				{
					const int oldstart = pSuper->RechargeTimer.StartTime;
					const int oldleft = pSuper->RechargeTimer.TimeLeft;
					pSuper->SetReadiness(true);
					pSuper->Launch(CellClass::Coord2Cell(EnteredBuilding->GetCenterCoords()), pHouse->IsCurrentPlayer());
					pSuper->Reset();
					pSuper->RechargeTimer.StartTime = oldstart;
					pSuper->RechargeTimer.TimeLeft = oldleft;
				}
			}
		};

		auto justGrantTheSW = [](int const idx, HouseClass* const pHouse)
		{
			if (const auto pSuper = pHouse->Supers.GetItem(idx))
			{
				if (pSuper->Granted)
					pSuper->SetCharge(100);
				else
				{
					pSuper->Grant(true, false, false);
					if (pHouse->IsCurrentPlayer())
						SidebarClass::Instance->AddCameo(AbstractType::Special, idx);
				}
				SidebarClass::Instance->RepaintSidebar(1);
			}
		};

		if (pTypeExt->SpyEffect_VictimSuperWeapon.isset() && !Owner->IsNeutral())
		{
			launchTheSWHere(pTypeExt->SpyEffect_VictimSuperWeapon.Get(), Owner, pTypeExt->SpyEffect_VictimSW_RealLaunch.Get());

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_InfiltratorSuperWeapon.isset())
		{
			const int swidx = pTypeExt->SpyEffect_InfiltratorSuperWeapon.Get();

			if (pTypeExt->SpyEffect_InfiltratorSW_JustGrant.Get())
				justGrantTheSW(swidx, Enterer);
			else
				launchTheSWHere(swidx, Enterer);

			if (evaForOwner || evaForEnterer) {
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (auto pSuperType = pTypeExt->SpyEffect_SuperWeapon)
		{
			const auto nIdx = pSuperType->ArrayIndex;
			const auto pSuper = Enterer->Supers.GetItem(nIdx);
			const bool Onetime = !pTypeExt->SpyEffect_SuperWeaponPermanent;
			bool CanLauch = true;

			if (!pSuperType->IsPowered || Enterer->PowerDrain == 0 || Enterer->PowerOutput >= Enterer->PowerDrain)
				CanLauch = false;

			const bool IsCurrentPlayer = Enterer->IsCurrentPlayer();

			if (pSuper->Grant(Onetime, IsCurrentPlayer, CanLauch))
			{
				if (pTypeExt->SpyEffect_SuperWeaponPermanent)
					pSuper->CanHold = false;

				if (IsCurrentPlayer)
				{
					SidebarClass::Instance->AddCameo(AbstractType::Special, nIdx);
					const auto nTab = SidebarClass::GetObjectTabIdx(AbstractType::Special, nIdx, false);
					SidebarClass::Instance->RepaintSidebar(nTab);
				}

				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}

				effectApplied = true;
			}
		}

		if (pTypeExt->SpyEffect_SabotageDelay > 0)
		{
			const int nDelay = int(pTypeExt->SpyEffect_SabotageDelay * 900.0);

			if (nDelay >= 0 && !EnteredBuilding->IsGoingToBlow)
			{
				EnteredBuilding->IsGoingToBlow = true;
				EnteredBuilding->GoingToBlowTimer.Start(nDelay);
				EnteredBuilding->Flash(nDelay / 2);

				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}

				effectApplied = true;
			}
		}

		{
			int bounty = 0;
			int available = Owner->Available_Money();
			if (pTypeExt->SpyEffect_StolenMoneyAmount > 0)
			{
				bounty = pTypeExt->SpyEffect_StolenMoneyAmount;
			}
			else if (pTypeExt->SpyEffect_StolenMoneyPercentage > 0)
			{
				bounty = available * pTypeExt->SpyEffect_StolenMoneyPercentage;
			}

			if (bounty > 0)
			{
				bounty = std::min(bounty, available);
				Owner->TakeMoney(bounty);
				Enterer->GiveMoney(bounty);
				if (evaForOwner)
				{
					VoxClass::Play(GameStrings::EVA_CashStolen);
				}
				if (evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfCashStolen);
				}
				effectApplied = true;
			}
		}

		{
			bool promotionStolen = false;
			if (pTypeExt->SpyEffect_GainVeterancy)
			{
				switch (EnteredType->Factory)
				{
				case UnitTypeClass::AbsID:
					Enterer->WarFactoryInfiltrated = true;
					promotionStolen = true;
					break;
				case InfantryTypeClass::AbsID:
					Enterer->BarracksInfiltrated = true;
					promotionStolen = true;
					break;
				case AircraftTypeClass::AbsID:
					Is_AirfieldSpied(Enterer) = true;
					promotionStolen = true;
					break;
				case BuildingTypeClass::AbsID:
					Is_BuildingProductionSpied(Enterer) = true;
					promotionStolen = true;
					break;
				default:
					break;
				}
			}
			else
			{
				if (pTypeExt->SpyEffect_AircraftVeterancy)
				{
					Is_AirfieldSpied(Enterer) = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_InfantryVeterancy)
				{
					Enterer->BarracksInfiltrated = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_NavalVeterancy)
				{
					Is_NavalYardSpied(Enterer) = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_VehicleVeterancy)
				{
					Enterer->WarFactoryInfiltrated = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_BuildingVeterancy)
				{
					Is_BuildingProductionSpied(Enterer) = true;
					promotionStolen = true;
				}
			}

			if (promotionStolen)
			{
				Enterer->RecheckTechTree = true;
				if (IsEntererControlledByPlayer) {
					MouseClass::Instance->SidebarNeedsRepaint();
				}

				if (evaForOwner) {
					VoxClass::Play(GameStrings::EVA_TechnologyStolen);
				}

				if (evaForEnterer) {
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
					VoxClass::Play(GameStrings::EVA_NewTechAcquired);
				}
				effectApplied = true;
			}
		}

		/*	RA1-Style Spying, as requested in issue #633
			This sets the respective bit to inform the game that a particular house has spied this building.
			Knowing that, the game will reveal the current production in this building to the players who have spied it.
			In practice, this means: If a player who has spied a factory clicks on that factory,
			he will see the cameo of whatever is being built in the factory.

			Addition 04.03.10: People complained about it not being optional. Now it is.
		*/
		if (pTypeExt->SpyEffect_RevealProduction)
		{
			EnteredBuilding->DisplayProductionTo.Add(Enterer);
			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_RevealRadar)
		{
			/*	Remember the new persisting radar spy effect on the victim house itself, because
				destroying the building would destroy the spy reveal info in the ExtData, too.
				2013-08-12 AlexB
			*/
			if (pTypeExt->SpyEffect_RevealRadarPersist)
			{
				RadarPresist(Owner).Add(Enterer);
			}

			EnteredBuilding->DisplayProductionTo.Add(Enterer);
			AresBldExtStuffs::UpdateDisplayTo(EnteredBuilding);

			if (evaForOwner || evaForEnterer) {
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			MapClass::Instance->Map_AI();
			MapClass::Instance->RedrawSidebar(2);
			effectApplied = true;
		}

		if (effectApplied) {
			EnteredBuilding->UpdatePlacement(PlacementType::Redraw);
		}

		return true;
	}

	//static DWORD GetFirewallFlags(BuildingClass* pThis)
	//{
	//	auto pCell = MapClass::Instance->GetCellAt(pThis->Location);
	//	DWORD flags = 0;
	//	for (size_t direction = 0; direction < 8; direction += 2)
	//	{
	//		auto pNeighbour = pCell->GetNeighbourCell(direction);
	//		if (auto pBld = pNeighbour->GetBuilding())
	//		{
	//			if (pBld->Type->FirestormWall && pBld->Owner == pThis->Owner && !pBld->InLimbo && pBld->IsAlive)
	//			{
	//				flags |= 1 << (direction >> 1);
	//			}
	//		}
	//	}
	//	return flags;
	//}

	//static void UpdateFirewall(BuildingClass* pThis , bool const changedState)
	//{
	//	auto const active = pThis->Owner->FirestormActive;
	//
	//	if (!changedState)
	//	{
	//		// update only the idle anim
	//		auto& Anim = pThis->GetAnim(BuildingAnimSlot::SpecialTwo);
	//
	//		// (0b0101 || 0b1010) == part of a straight line
	//		auto const connections = pThis->FirestormWallFrame & 0xF;
	//		if (active && Unsorted::CurrentFrame & 7 && !Anim
	//			&& connections != 0b0101 && connections != 0b1010
	//			&& (ScenarioClass::Instance->Random.Random() & 0xF) == 0)
	//		{
	//			if (AnimTypeClass* pType = RulesExt::Global()->FirestormIdleAnim)
	//			{
	//				auto const crd = pThis->GetCoords() - CoordStruct { 740, 740, 0 };
	//				Anim = GameCreate<AnimClass>(pType, crd, 0, 1, 0x604, -10);
	//				Anim->IsBuildingAnim = true;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		// update the frame, cell passability and active anim
	//		auto const idxFrame = AresBldExtStuffs::GetFirewallFlags(pThis)
	//			+ (active ? 32u : 0u);
	//
	//		if (pThis->FirestormWallFrame != idxFrame)
	//		{
	//			pThis->FirestormWallFrame = idxFrame;
	//			pThis->GetCell()->Setup(0xFFFFFFFF);
	//			pThis->UpdatePlacement(PlacementType::Redraw);
	//		}
	//
	//		auto& Anim = pThis->GetAnim(BuildingAnimSlot::Special);
	//
	//		auto const connections = idxFrame & 0xF;
	//		if (active && connections != 0b0101 && connections != 0b1010 && !Anim)
	//		{
	//			if (auto const& pType = RulesExt::Global()->FirestormActiveAnim)
	//			{
	//				auto const crd = pThis->GetCoords() - CoordStruct { 128, 128, 0 };
	//				Anim = GameCreate<AnimClass>(pType, crd, 1, 0, 0x600, -10);
	//				Anim->IsFogged = pThis->IsFogged;
	//				Anim->IsBuildingAnim = true;
	//			}
	//		}
	//		else if (Anim)
	//		{
	//			Anim->UnInit();
	//		}
	//	}
	//
	//	if (active)
	//	{
	//		AresBldExtStuffs::ImmolateVictims(pThis);
	//	}
	//}

	//static void UpdateFirewallLinks(BuildingClass* pThis)
	//{
	//	if (pThis->Type->FirestormWall)
	//	{
	//		// update this
	//		if (!pThis->InLimbo && pThis->IsAlive)
	//		{
	//			AresBldExtStuffs::UpdateFirewall(pThis , true);
	//		}
	//
	//		// and all surrounding buildings
	//		auto const pCell = MapClass::Instance->GetCellAt(pThis->Location);
	//		for (auto i = 0u; i < 8; i += 2)
	//		{
	//			auto const pNeighbour = pCell->GetNeighbourCell(i);
	//			if (auto const pBld = pNeighbour->GetBuilding())
	//			{
	//				AresBldExtStuffs::UpdateFirewall(pBld, true);
	//			}
	//		}
	//	}
	//}

	//static void ImmolateVictims(BuildingClass* pThis)
	//{
	//	auto const pCell = pThis->GetCell();
	//	for (NextObject object(pCell->FirstObject); object; ++object)
	//	{
	//		if (auto pFoot = abstract_cast<FootClass*>(*object))
	//		{
	//			if (!pFoot->GetType()->IgnoresFirestorm)
	//			{
	//				AresBldExtStuffs::ImmolateVictim(pThis , pFoot , true);
	//			}
	//		}
	//	}
	//}

	//static bool ImmolateVictim(BuildingClass* pThis ,ObjectClass* const pVictim, bool const destroy)
	//{
	//	if (pVictim && pVictim->Health > 0)
	//	{
	//		const auto pRulesExt = RulesExt::Global();
	//
	//		if (destroy)
	//		{
	//			auto const pWarhead = pRulesExt->FirestormWarhead.Get(RulesClass::Instance->C4Warhead);
	//
	//			auto damage = pVictim->Health;
	//			pVictim->ReceiveDamage(&damage, 0, pWarhead, nullptr, true, true,
	//				pThis->Owner);
	//		}
	//
	//		const auto pType = ((pVictim->GetHeight() < 100)
	//			? pRulesExt->FirestormGroundAnim
	//			: pRulesExt->FirestormAirAnim).Get();
	//
	//		if (pType)
	//		{
	//			auto const crd = pVictim->GetCoords();
	//			GameCreate<AnimClass>(pType, crd, 0, 1, 0x600, -10, false);
	//		}
	//
	//		return true;
	//	}
	//
	//	return false;
	//}

};

// Fixing this wont change anything unless i change 
// everything that reference this function , for fuck sake
//DEFINE_OVERRIDE_HOOK(0x4440378, BuildingClass_Update_FirestormWall, 6)
//{
//	GET(BuildingClass* const, pThis, ESI);
//
//	if(Is_FirestromWall(pThis->Type))
//		AresBldExtStuffs::UpdateFirewall(pThis , false);
//
//	return 0;
//}

/* #633 - spy building infiltration */
// wrapper around the entire function
DEFINE_OVERRIDE_HOOK(0x4571E0, BuildingClass_Infiltrate, 5)
{
	GET(BuildingClass*, EnteredBuilding, ECX);
	GET_STACK(HouseClass*, Enterer, 0x4);

	return (AresBldExtStuffs::InfiltratedBy(EnteredBuilding, Enterer))
		? 0x4575A2
		: 0
		;
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

	const auto nResult = GetiInfiltrateActionResult(pThis, pBuilding);

	if (nResult == Action::Move) // this one will Infiltrate instead
	{
		const auto pHouse = pThis->Owner;
		if (!pThis->Type->Agent || pHouse->IsAlliedWith_(pBuilding))
			return SkipInfiltrate;

		pBuilding->Infiltrate(pHouse);
		return InfiltrateSucceded;
	}
	else
		if (nResult == Action::NoMove)
		{
			if (!ApplyC4ToBuilding(pThis, pBuilding, true))
				return SkipInfiltrate;

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
	GET(UnitClass* const, pUnit, ESI);
	GET(DirStruct* const, nCurrentFacing, EAX);

	const auto nDecidedFacing = Get::UnloadFacing(pUnit);

	if (*nCurrentFacing == nDecidedFacing)
		return 0x73771B;

	pUnit->Locomotor.GetInterfacePtr()->Do_Turn(nDecidedFacing);

	return 0x73770C;
}

DEFINE_OVERRIDE_HOOK(0x73DF66, UnitClass_Mi_Unload_DockUnload_Facing, 5)
{
	GET(UnitClass* const , pUnit, ESI);
	GET(DirStruct* const , nCurrentFacing, EAX);

	const auto nDecidedFacing = Get::UnloadFacing(pUnit);

	if (*nCurrentFacing == nDecidedFacing || pUnit->IsRotating)
		return 0x73DFBD;

	pUnit->Locomotor.GetInterfacePtr()->Do_Turn(nDecidedFacing);

	return 0x73DFB0;
}

DEFINE_OVERRIDE_HOOK(0x43CA80, BuildingClass_ReceivedRadioCommand_DockUnloadCell, 7)
{
	GET(CellStruct* const, pCell, EAX);
	GET(BuildingClass* const, pThis, ESI);

	const auto nBuff = Get::UnloadCell(pThis);
	R->DX(pCell->X + nBuff.X);
	R->AX(pCell->Y + nBuff.Y);

	return 0x43CA8D;
}

DEFINE_OVERRIDE_HOOK(0x73E013, UnitClass_Mi_Unload_DockUnloadCell1, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(Get::BuildingUnload(pThis));
	return 0x73E05F;
}

DEFINE_OVERRIDE_HOOK(0x73E17F, UnitClass_Mi_Unload_DockUnloadCell2, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(Get::BuildingUnload(pThis));
	return 0x73E1CB;
}

DEFINE_OVERRIDE_HOOK(0x73E2BF, UnitClass_Mi_Unload_DockUnloadCell3, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(Get::BuildingUnload(pThis));
	return 0x73E30B;
}

DEFINE_OVERRIDE_HOOK(0x741BDB, UnitClass_SetDestination_DockUnloadCell, 7)
{
	GET(UnitClass* const, pThis, EBP);
	R->EAX(Get::BuildingUnload(pThis));
	return 0x741C28;
}
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

	if (Is_BuildingType(pItem))
	{
		const auto pBuilding = static_cast<BuildingTypeClass* const>(pItem);
		if (!BuildingTypeExt::ExtMap.Find(pBuilding)->PowersUp_Buildings.empty())
		{
			if (nAresREsult == CanBuildResult::Buildable)
			{
				R->EAX(BuildingTypeExt::CheckBuildLimit(pThis, pBuilding, includeInProduction));
				return 0x4F8361;
			}
		}
	}

	R->EAX(nAresREsult);
	return 0x4F8361;
}

DEFINE_OVERRIDE_HOOK(0x6F64CB, TechnoClass_DrawHealthBar_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	return Is_FirestromWall(pThis->Type) ? 0x6F6832u : 0u;
}

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

	if (Is_FirestromWall(pBld->Type))
	{
		return CheckFirestormActive;
	}

	return NoDecision;
}

// the game specifically hides tiberium building pips. allow them, but
// take care they don't show up for the original game
DEFINE_OVERRIDE_HOOK(0x709B4E, TechnoClass_DrawPipscale_SkipSkipTiberium, 6)
{
	GET(TechnoClass* const, pThis, EBP);

	bool showTiberium = true;
	if (const auto pBld = specific_cast<BuildingClass*>(pThis))
	{
		if ((pBld->Type->Refinery || pBld->Type->ResourceDestination) && pBld->Type->Storage > 0)
		{
			// show only if this refinery uses storage. otherwise, the original
			// refineries would show an unused tiberium pip scale
			showTiberium = TechnoTypeExt::ExtMap.Find(pBld->Type)->Refinery_UseStorage;
		}
	}

	return showTiberium ? 0x709B6E : 0x70A980;
}

DEFINE_OVERRIDE_HOOK(0x44F7A0, BuildingClass_UpdateDisplayTo, 6)
{
	GET(BuildingClass*, B, ECX);
	AresData::BuildingExt_UpdateDisplayTo(B);
	return 0x44F813;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
DEFINE_OVERRIDE_HOOK(0x44161C, BuildingClass_Destroy_OldSpy1, 6)
{
	GET(BuildingClass*, B, ESI);
	B->DisplayProductionTo.Clear();
	AresData::BuildingExt_UpdateDisplayTo(B);
	return 0x4416A2;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
DEFINE_OVERRIDE_HOOK(0x448312, BuildingClass_ChangeOwnership_OldSpy1, 0xA)
{
	GET(HouseClass*, newOwner, EBX);
	GET(BuildingClass*, B, ESI);

	if (B->DisplayProductionTo.Contains(newOwner)) {
		B->DisplayProductionTo.Remove(newOwner);
		AresData::BuildingExt_UpdateDisplayTo(B);
	}

	return 0x4483A0;
}

DEFINE_OVERRIDE_HOOK(0x455DA0, BuildingClass_IsFactory_CloningFacility, 6)
{
	GET(BuildingClass*, pThis, ECX);
	return BuildingTypeExt::ExtMap.Find(pThis->Type)->Cloning_Facility.Get()
		? 0x455DCD : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4444B3, BuildingClass_KickOutUnit_NoAlternateKickout, 6)
{
	GET(BuildingClass*, pThis, ESI);
	return pThis->Type->Factory == AbstractType::None
		|| BuildingTypeExt::ExtMap.Find(pThis->Type)->Cloning_Facility.Get()
		? 0x4452C5 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x446366, BuildingClass_Place_Academy, 6)
{
	GET(BuildingClass*, pThis, EBP);

	if (Is_Academy(pThis->Type) && pThis->Owner)
	{
		AresData::UpdateAcademy(pThis->Owner, pThis, true);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x445905, BuildingClass_Remove_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->IsOnMap && Is_Academy(pThis->Type) && pThis->Owner)
	{
		AresData::UpdateAcademy(pThis->Owner, pThis, false);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x448AB2, BuildingClass_ChangeOwnership_Remove_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->IsOnMap && Is_Academy(pThis->Type) && pThis->Owner)
	{
		AresData::UpdateAcademy(pThis->Owner, pThis, false);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4491D5, BuildingClass_ChangeOwnership_Add_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if (Is_Academy(pThis->Type) && pThis->Owner)
	{
		AresData::UpdateAcademy(pThis->Owner, pThis, true);
	}

	return 0;
}

void KickOutHospitalArmory(BuildingClass* pThis)
{
	if (pThis->Type->Hospital || pThis->Type->Armory)
	{
		if (FootClass* Passenger = pThis->Passengers.RemoveFirstPassenger())
		{
			pThis->KickOutUnit(Passenger, CellStruct::Empty);
		}
	}
}

DEFINE_OVERRIDE_HOOK(0x44D8A1, BuildingClass_UnloadPassengers_Unload, 6)
{
	GET(BuildingClass*, B, EBP);
	KickOutHospitalArmory(B);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x447113, BuildingClass_Sell_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);

	// #754 - evict Hospital/Armory contents
	KickOutHospitalArmory(pThis);
	AresData::CPrismRemoveFromNetwork(&PrimsForwardingPtr(pThis), true);
	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x446AAF, BuildingClass_Place_SkipFreeUnits, 6)
{
	// allow free units and non-separate aircraft to be created
	// only once.
	GET(BuildingClass*, pBld, EBP);

	// skip handling free units
	if (FreeUnitDone(pBld))
		return 0x446FB6;

	FreeUnitDone(pBld) = true;
	return 0;
}

// #665: Raidable Buildings - prevent raided buildings from being sold while raided
DEFINE_OVERRIDE_HOOK(0x4494D2, BuildingClass_IsSellable, 6)
{
	enum { Sellable = 0x449532, Unsellable = 0x449536, Undecided = 0 };
	GET(BuildingClass*, pThis, ESI);

	// enemy shouldn't be able to sell "borrowed" buildings
	return Is_CurrentlyRaided(pThis) ? Unsellable : Undecided;
}

DEFINE_OVERRIDE_HOOK(0x449518, BuildingClass_IsSellable_FirestormWall, 6)
{
	enum { CheckHouseFireWallActive = 0x449522, ReturnFalse = 0x449536 };
	//GET(BuildingClass*, pThis, ESI);

	GET(BuildingTypeClass*, pType, ECX);
	return Is_FirestromWall(pType) ? CheckHouseFireWallActive : ReturnFalse;
}

DEFINE_OVERRIDE_HOOK(0x44E550, BuildingClass_Mi_Open_GateDown, 6)
{
	GET(BuildingClass*, pThis, ESI);
	R->ECX(BuildingTypeExt::ExtMap.Find(pThis->Type)->GateDownSound
		.Get(RulesClass::Instance->GateDown));
	return 0x44E556;
}

DEFINE_OVERRIDE_HOOK(0x44E61E, BuildingClass_Mi_Open_GateUp, 6)
{
	GET(DWORD, offset, ESI);
	const auto pThis = reinterpret_cast<BuildingClass*>(offset - 0x9C);
	R->ECX(BuildingTypeExt::ExtMap.Find(pThis->Type)->GateUpSound
		.Get(RulesClass::Instance->GateUp));
	return 0x44E624;
}

DEFINE_OVERRIDE_HOOK(0x4509B4, BuildingClass_UpdateRepair_Funds, 7)
{
	GET(BuildingClass*, pThis, ESI);
	return !pThis->Owner->IsControlledByHuman_() || RulesExt::Global()->RepairStopOnInsufficientFunds
		? 0x0 : 0x4509BB;
}

DEFINE_OVERRIDE_HOOK(0x4521C8, BuildingClass_Disable_Temporal_Factories, 6)
{
	GET(BuildingClass*, pThis, ECX);

	auto const pType = pThis->Type;
	if (pType->Factory != AbstractType::None)
	{
		pThis->Owner->Update_FactoriesQueues(
		pType->Factory, pType->Naval, BuildCat::DontCare);
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4566B0, BuildingClass_GetRangeOfRadial_Radius, 6)
{
	enum
	{
		SetVal = 0x45674E
		, Nothing = 0x0
	};

	GET(BuildingClass*, pThis, ECX);
	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (!pExt->RadialIndicatorRadius.isset())
		return Nothing;

	R->EAX(pExt->RadialIndicatorRadius.Get());
	return SetVal;
}

DEFINE_HOOK(0x456768 , BuildingClass_DrawRadialIndicator_Always , 0x6)
{
	GET(BuildingClass* , pThis ,ESI);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	return pExt->AlwayDrawRadialIndicator.Get(pThis->HasPower) ?
		0x456776  : 0x456962;
}

DEFINE_OVERRIDE_HOOK(0x4581CD, BuildingClass_UnloadOccupants_AllOccupantsHaveLeft, 6)
{
	GET(BuildingClass*, pBld, ESI);
	AresData::EvalRaidStatus(pBld);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x458729, BuildingClass_KillOccupiers_AllOccupantsKilled, 6)
{
	GET(BuildingClass*, pBld, ESI);
	AresData::EvalRaidStatus(pBld);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4586CA, BuildingClass_KillOccupiers_EachOccupierKilled, 6)
{
	GET(BuildingClass*, pBld, ESI);
	//GET(TechnoClass*, pKiller, EBP);
	//GET(int, idxOccupant, EDI);
	AresData::EvalRaidStatus(pBld);
	//return 0x4586F0;
	return 0;
}

void KickOutOfRubble(BuildingClass* pBld)
{
	DynamicVectorClass<std::pair<FootClass*, bool>> list;

	// iterate over all cells and remove all infantry

	auto const location = MapClass::Instance->GetCellAt(pBld->Location)->MapCoords;
	// get the number of non-end-marker cells and a pointer to the cell data
	for (auto i = pBld->Type->FoundationData; *i != CellStruct{0x7FFF, 0x7FFF}; ++i)
	{
		// remove every techno that resides on this cell
		for (NextObject obj(MapClass::Instance->GetCellAt(location + *i)->
			GetContent()); obj; ++obj)
		{
			if (auto const pFoot = abstract_cast<FootClass*>(*obj))
			{
				if (pFoot->Limbo())
				{
					list.AddItem(std::make_pair(pFoot, pFoot->IsSelected));
				}
			}
		}
	}

	// this part kicks out all units we found in the rubble
	for (auto const& [pFoot, bIsSelected] : list)
	{
		if (pBld->KickOutUnit(pFoot, location) == KickOutResult::Succeeded)
		{
			if (bIsSelected)
			{
				pFoot->Select();
			}
		}
		else
		{
			pFoot->UnInit();
		}
	}
}

DEFINE_OVERRIDE_HOOK(0x441f2c ,BuildingClass_Destroy_KickOutOfRubble, 5)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->RubbleDestroyed || pTypeExt->RubbleIntact)
		KickOutOfRubble(pThis);

	return 0x0;
}

//there is 2 missing elemts here 
// it is seems not readed from ini but only stored on BuildingTypeExt::ExtData 
//void UpdateBuildupFrames(BuildingTypeClass* pThis)
//{
//	if (auto const pShp = pThis->Buildup)
//	{
//		auto const frames = pThis->Gate ?
//			pThis->GateStages + 1 : pShp->Frames / 2;
//
//		auto const duration = (frames < 1) ?
//			1 : static_cast<int>(BuildingTypeExt::ExtMap.Find(pThis)->BuildupTime.Get(
//				RulesClass::Instance->BuildupTime) * 900.0 / frames);
//
//		pThis->BuildingAnimFrame[0].dwUnknown = 0;
//		pThis->BuildingAnimFrame[0].FrameCount = frames;
//		pThis->BuildingAnimFrame[0].FrameDuration = duration;
//	}
//}
//
//DEFINE_OVERRIDE_HOOK(0x465A48, BuildingTypeClass_GetBuildup_BuildupTime, 5)
//{
//	GET(BuildingTypeClass* const, pThis, ESI);
//	UpdateBuildupFrames(pThis);
//	return 0x465AAE;
//}
//
//DEFINE_OVERRIDE_HOOK(0x45EAA5, BuildingTypeClass_LoadArt_BuildupTime, 6)
//{
//	GET(BuildingTypeClass* const, pThis, ESI);
//	UpdateBuildupFrames(pThis);
//	return 0x45EB3A;
//}
//
//DEFINE_OVERRIDE_HOOK(0x45F2B4, BuildingTypeClass_Load2DArt_BuildupTime, 5)
//{
//	GET(BuildingTypeClass* const, pThis, EBP);
//	UpdateBuildupFrames(pThis);
//	return 0x45F310;
//}

DEFINE_OVERRIDE_HOOK(0x459C03, BuildingClass_CanBeSelectedNow_MassSelectable, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->MassSelectable.Get(pThis->Type->IsUndeployable()))
	{
		return 0x459C14;
	}

	R->EAX(false);
	return 0x459C12;
}
