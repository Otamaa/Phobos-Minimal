#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>

#include <WWKeyboardClass.h>

#include "Header.h"

#include <Locomotor/TeleportLocomotionClass.h>
#include <CaptureManagerClass.h>

ASMJIT_PATCH(0x51E5E1, InfantryClass_GetActionOnObject_MultiEngineerB, 7)
{
	GET(BuildingClass*, pBld, ECX);
	Action ret = TechnoExt_ExtData::GetEngineerEnterEnemyBuildingAction(pBld);

	// use a dedicated cursor
	if (ret == Action::Damage)
	{
		MouseCursorFuncs::SetMouseCursorAction(RulesExtData::Instance()->EngineerDamageCursor, Action::Damage, false);
	}

	// return our action
	R->EAX(ret);
	return 0;
}

ASMJIT_PATCH(0x519D9C, InfantryClass_UpdatePosition_MultiEngineer, 5)
{
	GET(InfantryClass*, pEngi, ESI);
	GET(BuildingClass*, pBld, EDI);

	// damage or capture
	Action action = TechnoExt_ExtData::GetEngineerEnterEnemyBuildingAction(pBld);

	if (action == Action::Damage)
	{
		int Damage = int(std::ceil(pBld->Type->Strength * RulesExtData::Instance()->EngineerDamage));
		pBld->ReceiveDamage(&Damage, 0, RulesClass::Global()->C4Warhead, pEngi, true, false, pEngi->Owner);
		return 0x51A010;
	}

	return 0x519EAA;
}

ASMJIT_PATCH(0x471C96, CaptureManagerClass_CanCapture, 0xA)
{
	// this is a complete rewrite, because it might be easier to change
	// this in a central place than spread all over the source code.
	enum
	{
		Allowed = 0x471D2E, // this can be captured
		Disallowed = 0x471D35 // can't be captured
	};

	GET(CaptureManagerClass*, pThis, ECX);
	GET(TechnoClass*, pTarget, ESI);
	TechnoClass* pCapturer = pThis->Owner;

	// target exists and doesn't belong to capturing player
	if (!pTarget || pTarget->Owner == pCapturer->Owner)
	{
		return Disallowed;
	}

	// generally not capturable
	if (TechnoExtData::IsPsionicsImmune(pTarget))
	{
		return Disallowed;
	}

	// disallow capturing bunkered units
	if (pTarget->BunkerLinkedItem)
	{
		return Disallowed;
	}

	// TODO: extend this for mind-control priorities
	if (pTarget->IsMindControlled() || pTarget->MindControlledByHouse)
	{
		return Disallowed;
	}

	// free slot? (move on if infinite or single slot which will be freed if used)
	if (!pThis->InfiniteMindControl && pThis->MaxControlNodes != 1 && pThis->ControlNodes.Count >= pThis->MaxControlNodes
		&& !TechnoTypeExtContainer::Instance.Find(pCapturer->GetTechnoType())->MultiMindControl_ReleaseVictim)
	{
		return Disallowed;
	}

	// currently disallowed
	auto mission = pTarget->CurrentMission;
	if (pTarget->IsIronCurtained() || mission == Mission::Selling || mission == Mission::Construction)
	{
		return Disallowed;
	}

	// driver killed. has no mind.
	if (TechnoExtContainer::Instance.Find(pTarget)->Is_DriverKilled)
	{
		return Disallowed;
	}

	// passed all tests
	return Allowed;
}

ASMJIT_PATCH(0x51DF38, InfantryClass_Remove, 0xA)
{
	GET(InfantryClass*, pThis, ESI);

	if (auto pGarrison = std::exchange(TechnoExtContainer::Instance.Find(pThis)->GarrisonedIn , nullptr)) {
		if (!pGarrison->Occupants.Remove<true>(pThis)) {
			Debug::LogInfo("Infantry {} was garrisoned in building {}, but building didn't find it. WTF?",
				pThis->Type->ID, pGarrison->Type->ID);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x51DFFD, InfantryClass_Put, 5)
{
	GET(InfantryClass*, pThis, EDI);
	TechnoExtContainer::Instance.Find(pThis)->GarrisonedIn = nullptr;
	return 0;
}

ASMJIT_PATCH(0x517D51, InfantryClass_Init_Academy, 6)
{
	GET(InfantryClass*, pThis, ESI);

	if (pThis->Owner)
	{
		HouseExtData::ApplyAcademy(pThis->Owner, pThis, AbstractType::Infantry);
	}

	return 0;
}

ASMJIT_PATCH(0x51E7BF, InfantryClass_GetActionOnObject_CanCapture, 6)
{
	enum
	{
		Capture = 0x51E84B,  // the game will return an Enter cursor no questions asked
		DontCapture = 0x51E85A, // the game will assume this is not a VehicleThief and will check for other cursors normally
		Select = 0x51E7EF, // select target instead of ordering this
		DontMindMe = 0, // the game will check if this is a VehicleThief
	};

	GET(InfantryClass*, pSelected, EDI);
	GET(ObjectClass*, pTarget, ESI);

	TechnoClass* pTechnoTarget = flag_cast_to<TechnoClass*>(pTarget);

	if (!pTechnoTarget)
		return DontCapture;

	const auto pSelectedType = pSelected->Type;
	if (!pSelectedType->VehicleThief
		&& !TechnoTypeExtContainer::Instance.Find(pSelectedType)->CanDrive.Get(RulesExtData::Instance()->CanDrive))
		return DontCapture;

	if (pTechnoTarget->GetTechnoType()->IsTrain)
		return Select;

	//const auto nResult = (AresHijackActionResult)AresData::TechnoExt_GetActionHijack(pSelected, pTechnoTarget);
	const auto nResult = TechnoExt_ExtData::GetActionHijack(pSelected, pTechnoTarget);
	if (nResult == AresHijackActionResult::None)
		return DontCapture;

	if (nResult == AresHijackActionResult::Drive && InputManagerClass::Instance->IsForceFireKeyPressed())
		return DontCapture;

	MouseCursorFuncs::SetMouseCursorAction(92, Action::Capture, false);
	return Capture;
}

// the hijacker is close to the target. capture.
ASMJIT_PATCH(0x5203F7, InfantryClass_UpdateVehicleThief_Hijack, 5)
{
	enum { GoOn = 0x5206A1, Stop = 0x520473 };

	GET(InfantryClass*, pThis, ESI);
	GET(FootClass*, pTarget, EDI);
	//TechnoExtData* pExt = TechnoExtContainer::Instance.Find(pThis);

	bool finalize = TechnoExt_ExtData::PerformActionHijack(pThis, pTarget);
	if (finalize)
	{
		// manually deinitialize this infantry
		if (pThis->IsAlive)
			pThis->UnInit();
	}
	return finalize ? Stop : GoOn;
}

// change all the special things infantry do, like vehicle thief, infiltration,
// bridge repair, enter transports or bio reactors, ...
ASMJIT_PATCH(0x519675, InfantryClass_UpdatePosition_BeforeInfantrySpecific, 0xA)
{
	// called after FootClass:UpdatePosition has been called and before
	// all specific infantry handling takes place.
	enum
	{
		Return = 0x51AA01, // skip the original logic
		Destroy = 0x51A010, // uninits this infantry and returns
		Handle = 0 // resume the original function
	} DoWhat = Handle;

	GET(InfantryClass*, pThis, ESI);

	if (pThis)
	{
		// steal vehicles / reclaim KillDriver'd units using CanDrive
		if (pThis->CurrentMission == Mission::Capture)
		{
			if (TechnoClass* pDest = flag_cast_to<TechnoClass*>(pThis->Destination))
			{
				// this is the possible target we stand on
				CellClass* pCell = pThis->GetCell();
				TechnoClass* pTarget = pCell->GetUnit(pThis->OnBridge);
				if (!pTarget)
				{
					pTarget = pCell->GetAircraft(pThis->OnBridge);
					if (!pTarget)
					{
						pTarget = pCell->GetBuilding();
						if (pTarget && !pTarget->IsStrange())
						{
							return 0;
						}
					}
				}

				// reached its destination?
				if (pTarget && pTarget == pDest)
				{
					// reached the target. capture.
					DoWhat = TechnoExt_ExtData::PerformActionHijack(pThis, pTarget) ? Destroy : Return;
				}
			}
		}
	}

	return DoWhat;
}

// update the vehicle thief's destination. needed to follow a
// target without the requirement to also enable Thief=yes.
ASMJIT_PATCH(0x5202F9, InfantryClass_UpdateVehicleThief_Check, 6)
{
	GET(InfantryClass*, pThis, ESI);

	// good old WW checks for Thief. idiots.
	if (!pThis->Type->VehicleThief)
	{
		// also allow for drivers, because vehicles may still drive around. usually they are not.
		if (!TechnoTypeExtContainer::Instance.Find(pThis->Type)->CanDrive)
		{
			return 0x5206A1;
		}
	}

	return 0x52030D;
}

//
// skip old logic's way to determine the cursor
// Was 7
//InfantryClass_GetActionOnObject_MultiEngineerA
DEFINE_JUMP(LJMP, 0x51E5BB, 0x51E5D9);

ASMJIT_PATCH(0x51F628, InfantryClass_Guard_Doggie, 0x5)
{
	GET(InfantryClass*, pThis, ESI);
	GET(int, res, EAX);

	if (res != -1) {
		return 0x51F634;
	}

	// doggie sit down on tiberium handling
	if (pThis->Type->Doggie && !pThis->Crawling && !pThis->Target && !pThis->Destination) {

		const auto& nPrimaryFacing = pThis->PrimaryFacing;

		if (!nPrimaryFacing.Is_Rotating() && pThis->GetCell()->LandType == LandType::Tiberium) {
			if (nPrimaryFacing.Current().GetDir() == DirType::East) {
				// correct facing, sit down
				pThis->PlayAnim(DoType::Down);
			} else {
				// turn to correct facing
				pThis->Locomotor.GetInterfacePtr()->Do_Turn(DirStruct{ 3u , DirType::East });
			}
		}
	}

	return 0x51F62D;
}

ASMJIT_PATCH(0x51ABD7, InfantryClass_SetDestination_Doggie, 0x6)
{
	GET(InfantryClass* const, pThis, EBP);
	GET(AbstractClass* const, pTarget, EBX);

	// doggie cannot crawl; has to stand up and run
	const bool doggieStandUp = pTarget && pThis->Crawling && pThis->Type->Doggie;

	return doggieStandUp ? 0x51AC16 : 0;
}

ASMJIT_PATCH(0x5200C1, InfantryClass_UpdatePanic_Doggie, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const auto pType = pThis->Type;

	if(!pType->Doggie) {
		return 0;
	}

	// if panicking badly, lay down on tiberium
	if (pThis->PanicDurationLeft >= RulesExtData::Instance()->DoggiePanicMax) {
		if (!pThis->Destination && !pThis->Locomotor.GetInterfacePtr()->Is_Moving())		{
			if (pThis->GetCell()->LandType == LandType::Tiberium) {
				// is on tiberium. just lay down
				//if(!pType->Fraidycat)
					pThis->PlayAnim(DoType::Down);
				//else {
					//return 0x5201EC;
				//}
			} else if (!pThis->InLimbo) {
					// search tiberium and abort current mission
					pThis->MoveToTiberium(16, false);
				if (pThis->Destination) {
					pThis->SetTarget(nullptr);
					pThis->QueueMission(Mission::Move, false);
					pThis->NextMission();
				}
			}
		}
	}

	if (!(pType->Fearless || pThis->HasAbility(AbilityType::Fearless))) {
	 	--pThis->PanicDurationLeft;
	}

	return 0x52025A;
}

// #1008047: the C4 did not work correctly in YR, because some ability checks were missing
ASMJIT_PATCH(0x51C325, InfantryClass_IsCellOccupied_C4Ability, 0x6)
{
	GET(InfantryClass* const, pThis, EBP);

	return (pThis->Type->C4 || pThis->HasAbility(AbilityType::C4)) ?
		0x51C37D : 0x51C335;
}

ASMJIT_PATCH(0x51A4D2, InfantryClass_UpdatePosition_C4Ability, 0x6)
{
	GET(InfantryClass* const, pThis, ESI);

	return (!pThis->Type->C4 && !pThis->HasAbility(AbilityType::C4)) ?
		0x51A7F4 : 0x51A4E6;
}

// do not prone in water
ASMJIT_PATCH(0x5201CC, InfantryClass_UpdatePanic_ProneWater, 0x6)
{
	GET(InfantryClass* const, pThis, ESI);
	const auto pCell = pThis->GetCell();
	return (pCell->LandType == LandType::Beach || pCell->LandType == LandType::Water) &&
		!pCell->ContainsBridge() ? 0x5201DC : 0x0;
}

ASMJIT_PATCH(0x51F716, InfantryClass_Mi_Unload_Undeploy, 0x5)
{
	GET(InfantryTypeClass* const, pThisType, ECX);
	GET(InfantryClass* const, pThis, ESI);

	if (pThisType->UndeployDelay < 0)
		pThis->PlayAnim(DoType::Undeploy, true, false);

	R->EBX(1);
	return 0x51F7C9;
}

// should correct issue #743
ASMJIT_PATCH(0x51D799, InfantryClass_PlayAnim_WaterSound, 0x7)
{
	enum
	{
		Play = 0x51D7A6,
		SkipPlay = 0x51D8BF
	};

	GET(InfantryClass* const, I, ESI);

	return (I->Transporter || I->Type->MovementZone != MovementZone::AmphibiousDestroyer)
		? SkipPlay : Play ;
}

ASMJIT_PATCH(0x520731, InfantryClass_UpdateFiringState_Heal, 0x5)
{
	GET(InfantryClass* const, pThis, EBP);

	const auto pTargetTechno = flag_cast_to<TechnoClass* const>(pThis->Target);

	if (!pTargetTechno || RulesClass::Instance->ConditionGreen <= pTargetTechno->GetHealthPercentage())
		pThis->SetTarget(nullptr);

	return 0x52094C;
}

// actual game code: if(auto B = specific_cast<BuildingClass *>(T)) { if(T->currentAmmo > 1) { return 1; } }
// if the object being queried doesn't have a weapon (Armory/Hospital), it'll return 1 anyway
//echnoClass_GetROF_BuildingHack
DEFINE_JUMP(LJMP, 0x6FCFA4, 0x6FCFC1);

ASMJIT_PATCH(0x51BCB2, InfantryClass_Update_Reload, 0x6)
{
	GET(InfantryClass* const, I, ESI);

	if (I->InLimbo) {
		return 0x51BDCF;
	}

	I->Reload();
	return 0x51BCC0;
}

//InfantryClass_ActionOnObject_IvanBombs
DEFINE_JUMP(LJMP, 0x51F1D8, 0x51F1EA);

ASMJIT_PATCH(0x52070F, InfantryClass_UpdateFiringState_Uncloak, 0x5)
{
	GET(InfantryClass* const, pThis, EBP);
	GET_STACK(int, idxWeapon, STACK_OFFS(0x34, 0x24));

	//const auto pWeapon = pThis->GetWeapon(idxWeapon);

	//if (pWeapon && pWeapon->WeaponType->DecloakToFire) {
	//	pThis->Uncloak(false);
	//}else
	if (pThis->IsCloseEnough(pThis->Target, idxWeapon)) {
		pThis->Uncloak(false);
	}

	return 0x52094C;
}

// issues 896173 and 1433804: the teleport locomotor keeps a copy of the last
// coordinates, and unmarks the occupation bits of that place instead of the
// ones the unit was added to after putting it back on the map. that left the
// actual cell blocked. this fix resets the last coords, so the actual position
// is unmarked.
ASMJIT_PATCH(0x51DF27, InfantryClass_Remove_Teleport, 0x6)
{
	GET(InfantryClass* const, pThis, ECX);

	if (pThis->Type->Teleporter) {
		const auto pLoco = pThis->Locomotor.GetInterfacePtr();

		if (VTable::Get(pLoco) == TeleportLocomotionClass::ILoco_vtable) {
			static_cast<TeleportLocomotionClass*>(pLoco)->LastCoords = CoordStruct::Empty;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x5243DD, InfantryTypeClass_AllowDamageSparks, 0x6)
{
	GET(InfantryTypeClass*, pThis, ESI)
	GET(INIClass* const, pINI, EBP);

	pThis->DamageSparks = pINI->ReadBool(pThis->ID, "AllowDamageSparks", R->AL());

	return 0x5243EE;
}

ASMJIT_PATCH(0x51E3B0, InfantryClass_GetActionOnObject_EMP, 0x7)
{
	GET(InfantryClass* const, pInfantry, ECX);
	GET_STACK(TechnoClass* const, pTarget, 0x4);

	// infantry should really not be able to deploy then EMP'd.
	if ((pInfantry == pTarget) && pInfantry->Type->Deployer && pInfantry->IsUnderEMP()) {
		R->EAX(Action::NoDeploy);
		return 0x51F187;
	}

	return 0;
}

//DEFINE_SKIP_HOOK(0x5200D7, InfantryClass_UpdatePanic_DontReload ,0x6, 52010B)
DEFINE_JUMP(LJMP, 0x5200D7, 0x52010B);

ASMJIT_PATCH(0x51CE9A, InfantryClass_RandomAnim_IsCow, 5)
{
	GET(InfantryClass*, I, ESI);
	const auto pData = InfantryTypeExtContainer::Instance.Find(I->Type);

	// don't play idle when paralyzed
	if (I->IsUnderEMP())
	{
		R->BL(false);
		return 0x51CECD;
	}

	R->EDI(R->EAX()); // argh
	R->BL(pData->Is_Cow); // aaaargh! again!
	return pData->Is_Cow ? 0x51CEAEu : 0x51CECDu;
}

ASMJIT_PATCH(0x51F76D, InfantryClass_Unload, 5)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExtContainer::Instance.Find(I->Type)->Is_Deso ? 0x51F77Du : 0x51F792u;
}

ASMJIT_PATCH(0x52138c, InfantryClass_UpdateDeployment_Deso2, 6)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExtContainer::Instance.Find(I->Type)->Is_Deso ? 0x52139A : 0x5214B9;
}

ASMJIT_PATCH(0x5215f9, InfantryClass_UpdateDeployment_Deso1, 6)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExtContainer::Instance.Find(I->Type)->Is_Deso ? 0x5216B6 : 0x52160D;
}

ASMJIT_PATCH(0x629804, ParasiteClass_UpdateSquiddy, 9)
{
	GET(ParasiteClass*, pThis, ESI);
	R->EAX(pThis->Owner->GetWeapon(TechnoExtContainer::Instance.Find(pThis->Owner)->idxSlot_Parasite));
	return 0x62980D;
}

// #1283638: ivans cannot enter grinders; they get an attack cursor. if the
// grinder is rigged with a bomb, ivans can enter. this fix lets ivans enter
// allied grinders. pressing the force fire key brings back the old behavior.
ASMJIT_PATCH(0x51EB48, InfantryClass_GetActionOnObject_IvanGrinder, 0xA)
{
	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);

	if (auto pTargetBld = cast_to<BuildingClass*>(pTarget)) {
		if (pTargetBld->Type->Grinding && pThis->Owner->IsAlliedWith(pTargetBld)) {

			if (!InputManagerClass::Instance->IsForceFireKeyPressed()) {
				static COMPILETIMEEVAL BYTE return_grind[] = {
					0x5F, 0x5E, 0x5D, // pop edi, esi and ebp
					0xB8, 0x0B, 0x00, 0x00, 0x00, // eax = Action::Repair (not Action::Eaten)
					0x5B, 0x83, 0xC4, 0x28, // esp += 0x28
					0xC2, 0x08, 0x00 // retn 8
				};

				return reinterpret_cast<DWORD>(return_grind);
			}
		}
	}

	return 0;
}