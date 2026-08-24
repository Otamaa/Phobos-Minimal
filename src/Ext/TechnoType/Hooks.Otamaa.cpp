#include "Body.h"
#include <Ext/BuildingType/Body.h>
#include <Utilities/Macro.h>

#include <New/Type/ArmorTypeClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>

#include <SlaveManagerClass.h>
#include <VocClass.h>
#include <VoxClass.h>

ASMJIT_PATCH(0x6B0C2C, SlaveManagerClass_FreeSlaves_Sound, 0x5) // C
{
	GET(TechnoClass*, pSlave, EDI);

	VocClass::SafeImmedietelyPlayAt
	(
		GET_TECHNOTYPEEXT(pSlave)->SlaveFreeSound.Get(RulesClass::Instance->SlavesFreeSound)
		, &pSlave->Location
	);

	return 0x6B0C65;
}

ASMJIT_PATCH(0x443C0D, BuildingClass_AssignTarget_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);

	if(pThis->TickTank || BuildingTypeExtContainer::Instance.Find(pThis)->IsJuggernaut || pThis->Artillary) {

		if(!pThis->UndeploysInto)
			return 0x443BB3;

		return 0x443C21;
	}

	return 0x443BB3;
}

ASMJIT_PATCH(0x44A93D, BuildingClass_Mission_Selling_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);
	return (pThis->TickTank || BuildingTypeExtContainer::Instance.Find(pThis)->IsJuggernaut || pThis->Artillary)
		? 0x44A951 : 0x44A95E;
}

ASMJIT_PATCH(0x739801, UnitClass_TryToDeploy_BarrelFacing_Jugger, 0x6) //8
{
	GET(BuildingTypeClass*, pThis, EAX);
	R->CL(pThis->TickTank || BuildingTypeExtContainer::Instance.Find(pThis)->IsJuggernaut);
	return 0x739807;
}


ASMJIT_PATCH(0x7365E6, UnitClass_AI_Rotation_AI_Replace, 0x7)
{
	GET(UnitClass*, pThis, ESI);

	const auto TypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	auto const nDisableEmp = pThis->EMPLockRemaining && TypeExt->FacingRotation_DisalbeOnEMP;
	auto const nDisableDeactivated = (pThis->IsDeactivated())&& TypeExt->FacingRotation_DisalbeOnDeactivated && !pThis->EMPLockRemaining;
	auto const nDisableDriverKilled = TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled && TypeExt->FacingRotation_DisableOnDriverKilled;

	if (TypeExt->FacingRotation_Disable.Get(nDisableEmp || nDisableDeactivated || nDisableDriverKilled))
		return 0x7365ED;

	pThis->UpdateRotation();

	return 0x7365ED;
}
