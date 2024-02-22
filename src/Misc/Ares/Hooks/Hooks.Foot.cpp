#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Techno/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Tiberium/Body.h>

#include"Header.h"

#include <New/Type/ArmorTypeClass.h>

DEFINE_OVERRIDE_HOOK(0x4DFE00, FootClass_GarrisonStructure_TakeVehicle, 6)
{
	GET(FootClass*, pThis, ECX);

	if (!TechnoExtContainer::Instance.Find(pThis)->TakeVehicleMode)
		return 0x0;

	R->EAX(TechnoExt_ExtData::FindAndTakeVehicle(pThis));
	return 0x4DFF3E;
}

DEFINE_OVERRIDE_HOOK(0x4D718C, FootClass_Put_InitialPayload, 6)
{
	GET(FootClass* const, pThis, ESI);

	if (pThis->WhatAmI() != AbstractType::Infantry)
	{
		TechnoExtContainer::Instance.Find(pThis)->CreateInitialPayload();
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D98C0, FootClass_Destroyed_PlayEvent, 0xA)
{
	enum { Skip = 0x4D9918 };
	GET(FootClass*, pThis, ECX);
	//GET_STACK(ObjectClass*, pKiller, 0x4);

	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->SupressEVALost
		|| pType->DontScore
		|| pType->Insignificant
		|| pType->Spawned
		|| !pThis->Owner
		|| !pThis->Owner->ControlledByCurrentPlayer()
	)
	{
		return Skip;
	}

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (RadarEventClass::Create(RadarEventType::UnitLost, pThis->GetMapCoords()))
		VoxClass::PlayIndex(pTypeExt->EVA_UnitLost, -1, -1);

	return Skip;
}

DEFINE_HOOK(0x4D5776, FootClass_ApproachTarget_Passive, 0x6)
{
	GET(FootClass* const, pThis, EBX);
	GET_STACK(bool, bSomething, 0x12);

	if (pThis->BunkerLinkedItem || pThis->ShouldLoseTargetNow || pThis->InOpenToppedTransport)
		R->AL(0);

	return (!bSomething)
		? 0x4D5796 : 0x4D57EA;
}

DEFINE_OVERRIDE_HOOK(0x4D9EE1, FootClass_CanBeSold_Dock, 0x6)
{
	GET(BuildingClass* const, pBld, EAX);
	GET(CoordStruct*, pBuffer, ECX);
	GET(TechnoClass* const, pDocker, ESI);
	R->EAX(pBld->GetDockCoords(pBuffer, pDocker));
	return 0x4D9EE7;
}

// replace Is_Moving_Now, because it doesn't check the
// current speed in case the unit is turning.
DEFINE_OVERRIDE_HOOK(0x4DBDD4, FootClass_IsCloakable_CloakStop, 0x6)
{
	GET(FootClass* const, pThis, ESI);
	R->AL(pThis->Locomotor.GetInterfacePtr()->Is_Moving());
	return 0x4DBDE3;
}

// support Occupier and VehicleThief on one type. if this is not done
// the Occupier handling will leave a dangling Destination pointer.
DEFINE_OVERRIDE_HOOK(0x4D9A83, FootClass_PointerGotInvalid_OccupierVehicleThief, 0x6)
{
	GET(InfantryClass* const, pInfantry, ESI);
	GET(InfantryTypeClass* const, pType, EAX);

	if (pType->VehicleThief
		&& pInfantry->Destination
		&& (pInfantry->Destination->AbstractFlags & AbstractFlags::Foot) )
	{
		return 0x4D9AB9;
	}

	return 0;
}

// update parasite coords along with the host
DEFINE_OVERRIDE_HOOK(0x4DB87E, FootClass_SetLocation_Parasite, 0x6)
{
	GET(FootClass* const, F, ESI);

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
				auto damage = RulesExtData::Instance()->HunterSeeker_Damage.Get();
				pTarget->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, pThis, true, true, pThis->Owner);
			}
		}
	}

	return 0;
}

// stops movement sound from being played while unit is being pulled by a magnetron (see terror drone)
DEFINE_OVERRIDE_HOOK(0x7101CF, FootClass_ImbueLocomotor, 0x7)
{
	GET(FootClass* const, pThis, ESI);
	pThis->Audio7.AudioEventHandleEndLooping();
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4DAA68, FootClass_Update_MoveSound, 0x6)
{
	GET(FootClass* const, pThis, ESI);

	const auto pType = pThis->GetTechnoType();

	if(pType->IdleRate && TechnoTypeExtContainer::Instance.Find(pType)->NoIdleSound) {
		if(!pThis->Locomotor->Is_Moving_Now()) {
			pThis->__MovingSoundDelay = 0;
			return 0x4DAB3C;
		}
	}

	if (pThis->__PlayingMovingSound) {
		return 0x4DAAEE;
	}

	if (pThis->LocomotorSource) {
		pThis->Audio7.AudioEventHandleEndLooping();
		return 0x4DAAEE;
	}

	return 0x4DAA70;
}

DEFINE_OVERRIDE_HOOK(0x4D7524, FootClass_ActionOnObject_Allow, 9)
{
	//overwrote the ja, need to replicate it
	GET(Action, CursorIndex, EBP);

	if (CursorIndex == Action::None || CursorIndex > Action::Airstrike) {
		return CursorIndex == Action(127) || CursorIndex == Action(126) ? 0x4D769F : 0x4D7CC0;
	}

	return 0x4D752D;
}

DEFINE_OVERRIDE_HOOK(0x4D9920, FootClass_SelectAutoTarget_Cloaked, 9)
{
	GET(FootClass* const, pThis, ECX);

	if (pThis->Owner->IsControlledByHuman()
		&& pThis->GetCurrentMission() == Mission::Guard)
	{
		auto const pType = pThis->GetTechnoType();
		auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

		auto allowAquire = true;

		if (!pExt->CanPassiveAcquire_Guard)
		{
			// we are in guard mode
			allowAquire = false;
		}
		else if (!pExt->CanPassiveAcquire_Cloak)
		{
			// passive acquire is disallowed when guarding and cloakable
			if (pThis->IsCloakable() || pThis->HasAbility(AbilityType::Cloak))
			{
				allowAquire = false;
			}
		}

		if (!allowAquire)
		{
			R->EAX(static_cast<TechnoClass*>(nullptr));
			return 0x4D995C;
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D9EBD, FootClass_CanBeSold_SellUnit, 6)
{
	GET(BuildingClass*, pBld, EAX);
	GET(TechnoClass*, pDocker, ESI);

	const auto nUnitRepair = BuildingTypeExtContainer::Instance.Find(pBld->Type)->UnitSell.Get(pBld->Type->UnitRepair);
	const auto nSellable = TechnoTypeExtContainer::Instance.Find(pDocker->GetTechnoType())->Unsellable.Get(RulesExtData::Instance()->Units_UnSellable);

	if (!nUnitRepair || !nSellable)
	{
		R->CL(false);
	}
	else
	{
		R->CL(true);
	}

	return 0x4D9EC9;
}

// move to the next hva frame, even if this unit isn't moving
DEFINE_OVERRIDE_HOOK(0x4DA8B2, FootClass_Update_AnimRate, 6)
{
	GET(FootClass*, pThis, ESI);
	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	enum { Undecided = 0u, NoChange = 0x4DAA01u, Advance = 0x4DA9FBu };

	// any of these prevents the animation to advance to the next frame
	if (pThis->IsBeingWarpedOut() || pThis->IsWarpingIn() || pThis->IsAttackedByLocomotor)
	{
		return NoChange;
	}

	// animate unit whenever in air
	if (pExt->AirRate && pThis->GetHeight() > 0)
	{
		return (Unsorted::CurrentFrame % pExt->AirRate) ? NoChange : Advance;
	}

	return Undecided;
}

 //rotation when crashing made optional
DEFINE_OVERRIDE_HOOK(0x4DECAE, FootClass_Crash_Spin, 5)
{
	GET(FootClass*, pThis, ESI);
	return TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->CrashSpin ? 0u : 0x4DED4Bu;
}

DEFINE_OVERRIDE_HOOK(0x518744, InfantryClass_ReceiveDamage_ElectricDeath, 6)
{
	AnimTypeClass* El = RulesExtData::Instance()->ElectricDeath;

	if (!El) {
		El = AnimTypeClass::Array->Items[0];
	}

	R->EDX(El);
	return 0x51874D;
}

DEFINE_OVERRIDE_HOOK(0x4D85E4, FootClass_UpdatePosition_TiberiumDamage, 9)
{
	GET(FootClass*, pThis, ESI);

	if (!pThis->IsAlive)
		return 0x0;

	int damage = 0;
	WarheadTypeClass* pWarhead = nullptr;
	int transmogrify = RulesClass::Instance->TiberiumTransmogrify;

	if (RulesExtData::Instance()->Tiberium_DamageEnabled && pThis->GetHeight() <= RulesClass::Instance->HoverHeight)
	{
		TechnoTypeClass* pType = pThis->GetTechnoType();
		TechnoTypeExtData* pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// default is: infantry can be damaged, others cannot
		const bool enabled = (pThis->WhatAmI() != InfantryClass::AbsID);

		if (!pExt->TiberiumProof.Get(enabled) && !pThis->HasAbility(AbilityType::TiberiumProof))
		{
			if (pThis->Health > 0)
			{
				if (auto pTiberium = TiberiumClass::Array->GetItemOrDefault(pThis->GetCell()->GetContainedTiberiumIndex()))
				{
					auto pTibExt = TiberiumExtExtContainer::Instance.Find(pTiberium);

					pWarhead = pTibExt->GetWarhead();
					damage = pTibExt->GetDamage();

					transmogrify = pExt->TiberiumTransmogrify.Get(transmogrify);
				}
			}
		}
	}

	if (damage != 0 && pWarhead)
	{
		CoordStruct crd = pThis->GetCoords();

		if (pThis->ReceiveDamage(&damage, 0, pWarhead, nullptr, false, false, nullptr) == DamageState::NowDead)
		{
			TechnoExt_ExtData::SpawnVisceroid(crd, RulesClass::Instance->SmallVisceroid, transmogrify, ScenarioClass::Instance->TiberiumDeathToVisceroid , HouseExtData::FindNeutral());
			return 0x4D8F29;
		}
	}

	return 0;
}
