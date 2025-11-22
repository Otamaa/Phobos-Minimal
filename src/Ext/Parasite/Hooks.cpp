#include "Body.h"
#include <TechnoClass.h>
#include <HouseClass.h>
#include <AnimClass.h>
#include <ScenarioClass.h>
#include <SpecificStructures.h>
#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TerrainType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

//TODO :
// Parasite Heal
// Parasite Able To target friendly
// Parasite not removed when heal
// Parasite Gain victim control instead of damaging

#include <TerrainClass.h>
#include <InfantryClass.h>

ASMJIT_PATCH(0x629E90, FootClass_WakeAnim_OnlyWater, 0x6)
{
	GET(FootClass*, pThis, ECX);

	return pThis->GetCell()->Tile_Is_Water() ? 0x0 : 0x629FC6;
}

ASMJIT_PATCH(0x62A915, ParasiteClass_CanInfect_Parasiteable, 0xA)
{
	enum { continuecheck = 0x62A929, returnfalse = 0x62A976, continuecheckB = 0x62A933 };
	GET(ParasiteClass*, pThis, EDI);
	GET(FootClass* const, pVictim, ESI);

	if (pThis->Owner->Transporter)
		return returnfalse;

	if (TechnoExtData::IsParasiteImmune(pVictim))
		return returnfalse;

	if (pVictim->IsIronCurtained())
		return returnfalse;

	if (pVictim->IsBeingWarpedOut())
		return returnfalse;

	if (TechnoExtData::IsChronoDelayDamageImmune(pVictim))
		return returnfalse;

	return !pVictim->BunkerLinkedItem ? continuecheckB : returnfalse;
}