#include "Body.h"
#include <Ext/BuildingType/Body.h>
#include <Utilities/Macro.h>

#include <New/Type/ArmorTypeClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Others/DamageText.h>
#endif

DEFINE_HOOK(0x6B0C2C, SlaveManagerClass_FreeSlaves_Sound, 0x5) // C
{
	GET(InfantryClass*, pSlave, EDI);

	auto const pData = TechnoTypeExt::ExtMap.Find(pSlave->Type);
	auto const nSound = pData->SlaveFreeSound.Get(RulesClass::Instance->SlavesFreeSound);

	if (nSound != -1) {
		VocClass::PlayAt(nSound, pSlave->Location);
	}

	return 0x6B0C65;

}

DEFINE_HOOK(0x443C0D, BuildingClass_AssignTarget_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);
	return (pThis->TickTank || BuildingTypeExt::ExtMap.Find(pThis)->IsJuggernaut.Get() || pThis->Artillary)
		? 0x443C21 : 0x443BB3;
}

DEFINE_HOOK(0x44A93D, BuildingClass_MI_DC_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);
	return (pThis->TickTank || BuildingTypeExt::ExtMap.Find(pThis)->IsJuggernaut.Get() || pThis->Artillary)
		? 0x44A951 : 0x44A95E;
}

DEFINE_HOOK(0x739801, UnitClass_TryToDeploy_BarrelFacing_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);
	R->CL(pThis->TickTank || BuildingTypeExt::ExtMap.Find(pThis)->IsJuggernaut.Get());
	return 0x739807;
}

DEFINE_HOOK(0x6F6D9E, TechnoClass_Unlimbo_BuildingFacing_Jugger, 0x7)
{
	GET(TechnoClass*, pThis, ESI);

	if (const auto pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		if (BuildingTypeExt::ExtMap.Find(pBuilding->Type)->IsJuggernaut.Get())
		{
			R->ECX(&BuildingTypeExt::DefaultJuggerFacing);
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x449B04, TechnoClass_MI_Construct_Facing_Jugger, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (BuildingTypeExt::ExtMap.Find(pThis->Type)->IsJuggernaut.Get())
	{
		R->EDX(&BuildingTypeExt::DefaultJuggerFacing);
	}

	return 0x0;
}

//#ifdef ENABLE_NEWHOOKS
 DEFINE_HOOK(0x7365E6, UnitClass_AI_Rotation_AI_Replace, 0x7) //was 5
 {
 	GET(UnitClass*, pThis, ESI);

 	const auto TypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
 	{
 		auto const nDisableEmp = pThis->EMPLockRemaining && TypeExt->FacingRotation_DisalbeOnEMP.Get();
 		auto const nDisableDeactivated = pThis->IsDeactivated() && TypeExt->FacingRotation_DisalbeOnDeactivated.Get() && !pThis->EMPLockRemaining;

 #ifdef COMPILE_DP_FEATURES
 		auto const bDriverKilled = (*(bool*)((char*)pThis->align_154 + 0x9C));
 		auto const nDisableDriverKilled = bDriverKilled && TypeExt->FacingRotation_DisableOnDriverKilled.Get();

 		if ((nDisableEmp || nDisableDeactivated || nDisableDriverKilled || TypeExt->FacingRotation_Disable.Get()))
 #else
 		if ((nDisableEmp || nDisableDeactivated || TypeExt->FacingRotation_Disable.Get()))
 #endif
 			return 0x7365ED;
 	}

 	pThis->UpdateRotation();

 	return 0x7365ED;
 }
