
#include <Ext/Anim/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Utilities/Macro.h>
#include <Ext/House/Body.h>

#include <MapClass.h>
#include <Kamikaze.h>

#include <New/Entity/FlyingStrings.h>

#include <Commands/ShowTechnoNames.h>

#include <Locomotor/TunnelLocomotionClass.h>

#include <InfantryClass.h>
#include <VeinholeMonsterClass.h>

#define ENABLE_THESE

ASMJIT_PATCH(0x55B4E1, LogicClass_Update_Veinhole, 0x5)
{
	UpdateAllVeinholes();
	return 0;
}

void UpdateWebbed(FootClass* pThis)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pExt->IsWebbed)
		return;

	if (auto pInf = cast_to<InfantryClass*, false>(pThis)){
		if (pInf->ParalysisTimer.Completed()) {

			pExt->IsWebbed = false;

			if (pExt->WebbedAnim) {
				pExt->WebbedAnim.reset();
			}

			TechnoExtData::RestoreLastTargetAndMissionAfterWebbed(pInf);
		}
	}
}

#include <New/PhobosAttachedAffect/Functions.h>

ASMJIT_PATCH(0x703789, TechnoClass_Cloak_BeforeDetach, 0x6)        // TechnoClass_Do_Cloak
{
	GET(TechnoClass*, pThis, ESI);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	pExt->UpdateMindControlAnim();
	pExt->IsDetachingForCloak = true;

	return 0;
}ASMJIT_PATCH_AGAIN(0x6FBBC3, TechnoClass_Cloak_BeforeDetach, 0x5)  // TechnoClass_Cloaking_AI


ASMJIT_PATCH(0x703799, TechnoClass_Cloak_AfterDetach, 0xA)        // TechnoClass_Do_Cloak
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtContainer::Instance.Find(pThis)->IsDetachingForCloak = false;
	return 0;
}ASMJIT_PATCH_AGAIN(0x6FBBCE, TechnoClass_Cloak_AfterDetach, 0x7)  // TechnoClass_Cloaking_AI


ASMJIT_PATCH(0x6FB9D7, TechnoClass_Cloak_RestoreMCAnim, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pExt = TechnoExtContainer::Instance.Find(pThis))
		pExt->UpdateMindControlAnim();

	return 0;
}

ASMJIT_PATCH(0x51B389, FootClass_TunnelAI_Enter, 0x6) // Inf
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtContainer::Instance.Find(pThis)->UpdateOnTunnelEnter();
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x735A26, FootClass_TunnelAI_Enter, 0x6) // Unit

bool Spawned_Check_Destruction(AircraftClass* aircraft)
{
	if (aircraft->SpawnOwner == nullptr
		|| !aircraft->SpawnOwner->IsAlive
		|| aircraft->SpawnOwner->IsCrashing
		|| aircraft->SpawnOwner->IsSinking
		)
	{
		return false;
	}

	/**
	 *  If our TarCom is null, our original target has died.
	 *  Try targeting something else that is nearby,
	 *  unless we've already decided to head back to the spawner.
	 */
	if (aircraft->Target == nullptr && aircraft->Destination != aircraft->SpawnOwner)
	{
		CoordStruct loc = aircraft->GetCoords();
		aircraft->TargetAndEstimateDamage(&loc, ThreatType::Area);
	}

	/**
	 *  If our TarCom is still null or we're run out of ammo, return to
	 *  whoever spawned us. Once we're close enough, we should be erased from the map.
	 */
	if (aircraft->Target == nullptr || aircraft->Ammo == 0)
	{

		if (aircraft->Destination != aircraft->SpawnOwner)
		{
			aircraft->SetDestination(aircraft->SpawnOwner, true);
			aircraft->ForceMission(Mission::Move);
			aircraft->NextMission();
		}

		CoordStruct myloc = aircraft->GetCoords();
		CoordStruct spawnerloc = aircraft->GetCoords();
		if (myloc.DistanceFrom(spawnerloc) < Unsorted::LeptonsPerCell)
			return true;
	}

	return false;
}

DEFINE_FUNCTION_JUMP(CALL , 0x414DA3  , FakeAircraftClass::_FootClass_Update_Wrapper);