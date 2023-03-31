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
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

DEFINE_HOOK(0x4D5776, FootClass_ApproachTarget_Passive, 0x6)
{
	GET(FootClass*, pThis, EBX);
	GET_STACK(bool, bSomething, 0x12);

	if (pThis->BunkerLinkedItem || pThis->ShouldLoseTargetNow || pThis->InOpenToppedTransport)
		R->AL(0);

	return (!bSomething)
		? 0x4D5796 : 0x4D57EA;
}

DEFINE_OVERRIDE_HOOK(0x4D9EE1, FootClass_CanBeSold_Dock, 0x6)
{
	GET(BuildingClass*, pBld, EAX);
	GET(CoordStruct*, pBuffer, ECX);
	GET(TechnoClass*, pDocker, ESI);
	R->EAX(pBld->GetDockCoords(pBuffer, pDocker));
	return 0x4D9EE7;
}

// replace Is_Moving_Now, because it doesn't check the
// current speed in case the unit is turning.
DEFINE_OVERRIDE_HOOK(0x4DBDD4, FootClass_IsCloakable_CloakStop, 0x6)
{
	GET(FootClass*, pThis, ESI);
	R->AL(pThis->Locomotor->Is_Moving());
	return 0x4DBDE3;
}

// support Occupier and VehicleThief on one type. if this is not done
// the Occupier handling will leave a dangling Destination pointer.
DEFINE_OVERRIDE_HOOK(0x4D9A83, FootClass_PointerGotInvalid_OccupierVehicleThief, 0x6)
{
	GET(InfantryClass*, pInfantry, ESI);
	GET(InfantryTypeClass*, pType, EAX);

	if (pType->VehicleThief 
		&& pInfantry->Destination 
		&& (pInfantry->Destination->AbstractFlags & AbstractFlags::Foot) )
	{
		return 0x4D9AB9;
	}

	return 0;
}

// respect the remove state when updating the parasite.
DEFINE_OVERRIDE_HOOK(0x4D99AA, FootClass_PointerGotInvalid_Parasite, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(AbstractClass*, ptr, EDI);
	GET(bool, remove, EBX);

	// pass the real remove state, instead of always true. this was unused
	// in the original game, but now propagates the real value.
	if (auto pParasiteOwner = pThis->ParasiteEatingMe) {
		if (pParasiteOwner->Health > 0) {
			pParasiteOwner->ParasiteImUsing->PointerExpired(ptr, remove);
		}
	}

	// only unset the parasite owner, if we are removed.
	// cloaking does not count any more.
	if (remove && pThis == ptr) {
		pThis->ParasiteEatingMe = nullptr;
	}

	return 0x4D99D3;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x4DB37C, FootClass_Remove_Airspace, 0x6, 4DB3A4)

// update parasite coords along with the host
DEFINE_OVERRIDE_HOOK(0x4DB87E, FootClass_SetLocation_Parasite, 0x6)
{
	GET(FootClass*, F, ESI);

	if (F->ParasiteEatingMe) {
		F->ParasiteEatingMe->SetLocation(F->Location);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D8D95, FootClass_UpdatePosition_HunterSeeker, 0xA)
{
	GET(FootClass* const, pThis, ESI);

	// ensure the target won't get away
	if (pThis->GetTechnoType()->HunterSeeker) {
		if (auto const pTarget = abstract_cast<TechnoClass*>(pThis->Target)) {

			const auto pWpS = pThis->GetWeapon(0);

			if(pWpS && pWpS->WeaponType)
			{
				auto damage = pWpS->WeaponType->Damage;
				pTarget->ReceiveDamage(&damage, 0, pWpS->WeaponType->Warhead, pThis, true, true, pThis->Owner);
			}
			else
			{
				auto damage = RulesExt::Global()->HunterSeeker_Damage.Get();
				pTarget->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, pThis, true, true, pThis->Owner);
			}
		}
	}

	return 0;
}

// stops movement sound from being played while unit is being pulled by a magnetron (see terror drone)
DEFINE_OVERRIDE_HOOK(0x7101CF, FootClass_ImbueLocomotor, 0x7)
{
	GET(FootClass*, pThis, ESI);
	pThis->Audio7.AudioEventHandleEndLooping();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4DAA68, FootClass_Update_MoveSound, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (pThis->__PlayingMovingSound) {
		return 0x4DAAEE;
	}

	if (pThis->LocomotorSource) {
		pThis->Audio7.AudioEventHandleEndLooping();
		return 0x4DAAEE;
	}

	return 0x4DAA70;
}

//#include <Ext/Techno/Body.h>
//
//DEFINE_HOOK(0x4DA56E, FootClass_Update_RadImmune, 0xA)
//{
//	enum { RetImmune = 0x4DA63B , Continue = 0x4DA593};
//
//	GET(FootClass*, pThis, ESI);
//
//	if (pThis->InLimbo)
//		return RetImmune;
//
//	if (pThis->IsInAir())
//		return RetImmune;
//
//	auto const pUnit = specific_cast<UnitClass*>(pThis);
//
//	if (pThis->GetTechnoType()->Immune ||
//		pThis->IsIronCurtained() ||
//		!pThis->IsInPlayfield ||
//		pThis->TemporalTargetingMe || (pUnit && pUnit->DeathFrameCounter > 0))
//	{
//		return RetImmune;
//	}
//
//	if (pThis->IsBeingWarpedOut() || TechnoExt::IsChronoDelayDamageImmune(pThis))
//		return RetImmune;
//
//	return TechnoExt::IsRadImmune(pThis) ? RetImmune : Continue;
//}