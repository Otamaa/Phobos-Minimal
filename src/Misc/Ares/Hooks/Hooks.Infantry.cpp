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
#include <Misc/AresData.h>
#include <Ares_TechnoExt.h>

DEFINE_OVERRIDE_HOOK(0x523932, InfantryTypeClass_CTOR_Initialize, 8)
{
	GET(InfantryTypeClass*, pItem, ESI)

	for (auto& nSeq : pItem->Sequence->Data)
	{
		nSeq.StartFrame = 0;
		nSeq.CountFrames = 0;
		nSeq.FacingMultiplier = 0;
		nSeq.Facing = DoTypeFacing(-1);
		nSeq.SoundCount = 0;
		nSeq.Sound1StartFrame = 0;
		nSeq.Sound1Index = -1;
		nSeq.Sound2StartFrame = 0;
		nSeq.Sound2Index = -1;
	}

	return 0x523970;
}

//
// skip old logic's way to determine the cursor
// Was 7
DEFINE_DISABLE_HOOK(0x51E5BB, InfantryClass_GetActionOnObject_MultiEngineerA_ares)//, 0x5, 51E5D9)
DEFINE_JUMP(LJMP, 0x51E5BB, 0x51E5D9);

DEFINE_OVERRIDE_HOOK(0x51F628, InfantryClass_Guard_Doggie, 0x5)
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

DEFINE_OVERRIDE_HOOK(0x51ABD7, InfantryClass_SetDestination_Doggie, 0x6)
{
	GET(InfantryClass* const, pThis, EBP);
	GET(AbstractClass* const, pTarget, EBX);

	// doggie cannot crawl; has to stand up and run
	const bool doggieStandUp = pTarget && pThis->Crawling && pThis->Type->Doggie;

	return doggieStandUp ? 0x51AC16 : 0;
}

DEFINE_OVERRIDE_HOOK(0x5200C1, InfantryClass_UpdatePanic_Doggie, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const auto pType = pThis->Type;

	if (!pType->Doggie) {
		return 0;
	}

	// if panicking badly, lay down on tiberium
	if (pThis->PanicDurationLeft >= RulesExt::Global()->DoggiePanicMax) {
		if (!pThis->Destination && !pThis->Locomotor.GetInterfacePtr()->Is_Moving())		{
			if (pThis->GetCell()->LandType == LandType::Tiberium) {
				// is on tiberium. just lay down
				pThis->PlayAnim(DoType::Down);
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

	if (!pType->Fearless) {
	 --pThis->PanicDurationLeft;
	}

	return 0x52025A;
}

// #1008047: the C4 did not work correctly in YR, because some ability checks were missing
DEFINE_OVERRIDE_HOOK(0x51C325, InfantryClass_IsCellOccupied_C4Ability, 0x6)
{
	GET(InfantryClass* const, pThis, EBP);

	return (pThis->Type->C4 || pThis->HasAbility(AbilityType::C4)) ?
		0x51C37D : 0x51C335;
}

DEFINE_OVERRIDE_HOOK(0x51A4D2, InfantryClass_UpdatePosition_C4Ability, 0x6)
{
	GET(InfantryClass* const, pThis, ESI);

	return (!pThis->Type->C4 && !pThis->HasAbility(AbilityType::C4)) ?
		0x51A7F4 : 0x51A4E6;
}

// do not prone in water
DEFINE_OVERRIDE_HOOK(0x5201CC, InfantryClass_UpdatePanic_ProneWater, 0x6)
{
	GET(InfantryClass* const, pThis, ESI);
	const auto pCell = pThis->GetCell();
	return (pCell->LandType == LandType::Beach || pCell->LandType == LandType::Water) &&
		!pCell->ContainsBridge() ? 0x5201DC : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x51F716, InfantryClass_Mi_Unload_Undeploy, 0x5)
{
	GET(InfantryTypeClass* const, pThisType, ECX);
	GET(InfantryClass* const, pThis, ESI);

	if (pThisType->UndeployDelay < 0)
		pThis->PlayAnim(DoType::Undeploy, true, false);

	R->EBX(1);
	return 0x51F7C9;
}

// should correct issue #743
DEFINE_OVERRIDE_HOOK(0x51D799, InfantryClass_PlayAnim_WaterSound, 0x7)
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

DEFINE_OVERRIDE_HOOK(0x520731, InfantryClass_UpdateFiringState_Heal, 0x5)
{
	GET(InfantryClass* const, pThis, EBP);

	const auto pTargetTechno = generic_cast<TechnoClass* const>(pThis->Target);

	if (!pTargetTechno || RulesClass::Instance->ConditionGreen <= pTargetTechno->GetHealthPercentage())
		pThis->SetTarget(nullptr);

	return 0x52094C;
}

// actual game code: if(auto B = specific_cast<BuildingClass *>(T)) { if(T->currentAmmo > 1) { return 1; } }
// if the object being queried doesn't have a weapon (Armory/Hospital), it'll return 1 anyway
DEFINE_DISABLE_HOOK(0x6FCFA4, TechnoClass_GetROF_BuildingHack_ares)//, 0x5, 6FCFC1)
DEFINE_JUMP(LJMP, 0x6FCFA4, 0x6FCFC1);

DEFINE_OVERRIDE_HOOK(0x51BCB2, InfantryClass_Update_Reload, 0x6)
{
	GET(InfantryClass* const, I, ESI);

	if (I->InLimbo) {
		return 0x51BDCF;
	}

	I->Reload();
	return 0x51BCC0;
}

DEFINE_DISABLE_HOOK(0x51F1D8, InfantryClass_ActionOnObject_IvanBombs_ares)//, 0x6, 51F1EA)
DEFINE_JUMP(LJMP, 0x51F1D8, 0x51F1EA);

DEFINE_OVERRIDE_HOOK(0x52070F, InfantryClass_UpdateFiringState_Uncloak, 0x5)
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
DEFINE_OVERRIDE_HOOK(0x51DF27, InfantryClass_Remove_Teleport, 0x6)
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

DEFINE_HOOK(0x5243E3, InfantryTypeClass_AllowDamageSparks, 0xB)
{
	GET(InfantryTypeClass*, pThis, ESI)
	GET(INIClass* const, pINI, EBP);

	pThis->DamageSparks = pINI->ReadBool(pThis->ID, "AllowDamageSparks", R->AL());

	return 0x5243EE;
}

DEFINE_OVERRIDE_HOOK(0x51E3B0, InfantryClass_GetActionOnObject_EMP, 0x7)
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

DEFINE_DISABLE_HOOK(0x5200D7, InfantryClass_UpdatePanic_DontReload_ares)//, 0x6, 52010B)
DEFINE_JUMP(LJMP, 0x5200D7, 0x52010B);

DEFINE_OVERRIDE_HOOK(0x51CE9A, InfantryClass_RandomAnim_IsCow, 5)
{
	GET(InfantryClass*, I, ESI);
	const auto pData = InfantryTypeExt::ExtMap.Find(I->Type);

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

DEFINE_OVERRIDE_HOOK(0x51F76D, InfantryClass_Unload, 5)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExt::ExtMap.Find(I->Type)->Is_Deso ? 0x51F77Du : 0x51F792u;
}

DEFINE_OVERRIDE_HOOK(0x52138c, InfantryClass_UpdateDeployment_Deso2, 6)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExt::ExtMap.Find(I->Type)->Is_Deso ? 0x52139A : 0x5214B9;
}

DEFINE_OVERRIDE_HOOK(0x5215f9, InfantryClass_UpdateDeployment_Deso1, 6)
{
	GET(InfantryClass*, I, ESI);
	return InfantryTypeExt::ExtMap.Find(I->Type)->Is_Deso ? 0x5216B6 : 0x52160D;
}

DEFINE_OVERRIDE_HOOK(0x629804, ParasiteClass_UpdateSquiddy, 9)
{
	GET(ParasiteClass*, pThis, ESI);
	R->EAX(pThis->Owner->GetWeapon(pThis->Owner->align_154->idxSlot_Parasite));
	return 0x62980D;
}

// #1283638: ivans cannot enter grinders; they get an attack cursor. if the
// grinder is rigged with a bomb, ivans can enter. this fix lets ivans enter
// allied grinders. pressing the force fire key brings back the old behavior.
DEFINE_OVERRIDE_HOOK(0x51EB48, InfantryClass_GetActionOnObject_IvanGrinder, 0xA)
{
	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);

	if (auto pTargetBld = abstract_cast<BuildingClass*>(pTarget)) {
		if (pTargetBld->Type->Grinding && pThis->Owner->IsAlliedWith_(pTargetBld)) {

			if (!InputManagerClass::Instance->IsForceFireKeyPressed()) {
				static constexpr BYTE return_grind[] = {
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