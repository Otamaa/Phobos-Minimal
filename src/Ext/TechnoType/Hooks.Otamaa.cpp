#include "Body.h"
#include <Ext/BuildingType/Body.h>
#include <Utilities/Macro.h>

#include <New/Type/ArmorTypeClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES_
#include <Misc/DynamicPatcher/Others/DamageText.h>
#endif

#include <SlaveManagerClass.h>

DEFINE_HOOK(0x6B0C2C, SlaveManagerClass_FreeSlaves_Sound, 0x5) // C
{
	GET(TechnoClass*, pSlave, EDI);

	VocClass::PlayIndexAtPos
	(
		TechnoTypeExtContainer::Instance.Find(pSlave->GetTechnoType())->SlaveFreeSound.Get(RulesClass::Instance->SlavesFreeSound)
		, pSlave->Location
	);

	return 0x6B0C65;
}

DEFINE_HOOK(0x443C0D, BuildingClass_AssignTarget_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);

	if(pThis->TickTank || BuildingTypeExtContainer::Instance.Find(pThis)->IsJuggernaut || pThis->Artillary) {

		if(!pThis->UndeploysInto)
			return 0x443BB3;

		return 0x443C21;
	}

	return 0x443BB3;
}

DEFINE_HOOK(0x44A93D, BuildingClass_MI_DC_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);
	return (pThis->TickTank || BuildingTypeExtContainer::Instance.Find(pThis)->IsJuggernaut || pThis->Artillary)
		? 0x44A951 : 0x44A95E;
}

DEFINE_HOOK(0x739801, UnitClass_TryToDeploy_BarrelFacing_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);
	R->CL(pThis->TickTank || BuildingTypeExtContainer::Instance.Find(pThis)->IsJuggernaut);
	return 0x739807;
}

DEFINE_HOOK(0x6F6D94, TechnoClass_Unlimbo_BuildingFacing_Jugger, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (const auto pBuilding = cast_to<BuildingClass*, false>(pThis))
	{
		if (BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->IsJuggernaut)
		{
			pThis->PrimaryFacing.Set_Current(BuildingTypeExtData::DefaultJuggerFacing);
			R->Stack(0x30, BuildingTypeExtData::DefaultJuggerFacing);
			return 0x6F6DAF;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x449AF8, BuildingClass_MI_Construct_Facing_Jugger, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, EAX);

	if (BuildingTypeExtContainer::Instance.Find(pBldType)->IsJuggernaut)
	{
		pThis->PrimaryFacing.Set_Current(BuildingTypeExtData::DefaultJuggerFacing);
		return 0x449B15;
	}

	return 0x0;
}

DEFINE_HOOK(0x7365E6, UnitClass_AI_Rotation_AI_Replace, 0x7)
{
	GET(UnitClass*, pThis, ESI);

	const auto TypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	auto const nDisableEmp = pThis->EMPLockRemaining && TypeExt->FacingRotation_DisalbeOnEMP;
	auto const nDisableDeactivated = pThis->IsDeactivated() && TypeExt->FacingRotation_DisalbeOnDeactivated && !pThis->EMPLockRemaining;
	auto const nDisableDriverKilled = TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled && TypeExt->FacingRotation_DisableOnDriverKilled;

	if (TypeExt->FacingRotation_Disable.Get(nDisableEmp || nDisableDeactivated || nDisableDriverKilled))
		return 0x7365ED;

	pThis->UpdateRotation();

	return 0x7365ED;
}
