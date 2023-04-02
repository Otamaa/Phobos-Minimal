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

#include <Conversions.h>

DEFINE_OVERRIDE_HOOK(0x73D219, UnitClass_Draw_OreGatherAnim, 0x6)
{
	GET(TechnoClass*, pTechno, ECX);

	// disabled ore gatherers should not appear working.
	return (pTechno->IsWarpingIn() || pTechno->IsUnderEMP()) ?
	 0x73D28E : 0x73D223;
}

DEFINE_HOOK(0x7461C5, UnitClass_BallooonHoverExplode_OverrideCheck, 0x6)
{
	GET(UnitClass*, pThis, EDI);
	GET(UnitTypeClass*, pType, EAX);

	R->CL(pType->BalloonHover || pType->Explodes || pThis->HasAbility(AbilityType::Explodes));
	return 0x7461CB;
}

DEFINE_OVERRIDE_HOOK(0x73F7DD, UnitClass_IsCellOccupied_Bib, 0x8)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(UnitClass*, pThis, EBX);

	return pThis && pBuilding->Owner->IsAlliedWith(pThis) ? 0x0 : 0x73F823;
}

// #1171643: keep the last passenger if this is a gunner, not just
// when it has multiple turrets. gattling and charge turret is no
// longer affected by this.
DEFINE_OVERRIDE_HOOK(0x73D81E, UnitClass_Mi_Unload_LastPassenger, 0x5)
{
	GET(UnitClass*, pThis, ECX);
	R->EAX(pThis->GetTechnoType()->Gunner);
	return 0x73D823;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x7388EB, UnitClass_ActionOnObject_IvanBombs, 0x6, 7388FD)

DEFINE_OVERRIDE_HOOK(0x744745, UnitClass_RegisterDestruction_Trigger, 0x5)
{
	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pAttacker, EDI);

	if (pThis->IsAlive && pAttacker)
	{
		if (auto pTag = pThis->AttachedTag) {
			pTag->RaiseEvent((TriggerEvent)AresNewTriggerEvents::DestroyedByHouse, pThis, CellStruct::Empty, false, pAttacker->GetOwningHouse());
		}
	}

	return 0x0;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x73C733, UnitClass_DrawSHP_SkipTurretedShadow, 7, 73C7AC)

DEFINE_OVERRIDE_HOOK(0x741206, UnitClass_CanFire, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	auto Type = pThis->Type;

	if (!Type->TurretCount || Type->IsGattling) {
		return 0x741229;
	}

	auto W = pThis->GetWeapon(pThis->SelectWeapon(nullptr));
	return (W->WeaponType && W->WeaponType->Warhead->Temporal)
		? 0x741210u
		: 0x741229u
		;
}

DEFINE_OVERRIDE_HOOK(0x741113, UnitClass_CanFire_Heal, 0xA)
{
	GET(ObjectClass*, pTarget, EDI);

	return RulesClass::Instance->ConditionGreen > pTarget->GetHealthPercentage() ? 
		 0x741121 : 0x74113A;
}

DEFINE_OVERRIDE_HOOK(0x73C613, UnitClass_DrawSHP_FacingsA, 0x7)
{
	GET(UnitClass*, pThis, EBP);

	unsigned int ret = 0;

	if(pThis->Type->Facings > 0) {
		auto highest = Conversions::Int2Highest(pThis->Type->Facings);

		// 2^highest is the frame count, 3 means 8 frames
		if (highest >= 3 && !pThis->IsDisguised()) {
			ret = pThis->PrimaryFacing.Current().GetValue(highest, 1u << (highest - 3));
		}
	}

	R->EBX(ret);
	return 0x73C64B;
}

DEFINE_OVERRIDE_HOOK(0x73CD01, UnitClass_DrawSHP_FacingsB, 0x5)
{
	GET(UnitClass*, pThis, EBP);
	GET(UnitTypeClass*, pType, ECX);
	GET(int, facing, EAX);

	R->ECX(pThis);
	R->EAX(facing + pType->WalkFrames * pType->Facings);

	return 0x73CD06;
}

DEFINE_OVERRIDE_HOOK(0x739ADA, UnitClass_SimpleDeploy_Height, 0xA)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Deployed)
		return 0x739CBF;

	if (!pThis->InAir && pThis->Type->DeployToLand && pThis->GetHeight() > 0)
		pThis->InAir = 1;

	R->EAX(true);
	return 0x739B14;
}

DEFINE_OVERRIDE_HOOK(0x736E8E, UnitClass_UpdateFiringState_Heal, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pTargetTechno = generic_cast<TechnoClass*>(pThis->Target);

	if (!pTargetTechno || pTargetTechno->GetHealthPercentage() <= RulesClass::Instance()->ConditionGreen)
		pThis->SetTarget(nullptr);

	return 0x737063;
}

DEFINE_OVERRIDE_HOOK(0x7440BD, UnitClass_Remove, 0x6)
{
	GET(UnitClass*, U, ESI);

	if (auto Bld = specific_cast<BuildingClass*>(U->BunkerLinkedItem)) {
		Bld->ClearBunker();
	}

	return 0;
}
