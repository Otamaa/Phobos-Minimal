#include "Body.h"

#include <Ext/UnitType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>

#include <UnitClass.h>
#include <UnitTypeClass.h>

ASMJIT_PATCH(0x7394FF, UnitClass_TryToDeploy_CantDeployVoice, 0x8)
{
	GET(UnitClass* const, pThis, EBP);

	const auto pThisTechno = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	VoxClass::Play(GameStrings::EVA_CannotDeployHere());

	if (pThisTechno->VoiceCantDeploy.isset()) {
		//pThis->QueueVoice(pThisTechno->VoiceCantDeploy);
		VocClass::SafeImmedietelyPlayAt(pThisTechno->VoiceCantDeploy, &pThis->Location);
	}

	return 0x73950F;
}

ASMJIT_PATCH(0x74159F, UnitClass_ApproachTarget_GoAboveTarget, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	auto pType = pThis->Type;
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	R->AL(pType->BalloonHover || pTypeExt->CanGoAboveTarget);
	return R->Origin() + 0x6;
}

ASMJIT_PATCH(0x746A30, UnitClass_UpdateDisguise_DefaultMirageDisguises, 0x5)
{
	enum { Apply = 0x746A6C };

	GET(UnitClass*, pThis, ESI);

	const auto& disguises = UnitTypeExtContainer::Instance.Find(pThis->Type)
		->DefaultMirageDisguises.GetElements(RulesClass::Instance->DefaultMirageDisguises);

	TerrainTypeClass* pDisguiseAs = nullptr;

	if (disguises)
		pDisguiseAs = disguises[ScenarioClass::Instance->Random.RandomRanged(0, disguises.size() - 1)];
	else
		Debug::FatalErrorAndExit("DefaultDisguise Invalid for TechnoType %s", pThis->Type->ID);

	R->EAX(pDisguiseAs);
	return Apply;
}