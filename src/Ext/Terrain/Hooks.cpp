#include "Body.h"
#include <SpecificStructures.h>
#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>

/*
		to do :
		- idle anim
		- Re-Draw function , to show damaged part
*/
DEFINE_HOOK(0x71B98B, TerrainClass_TakeDamage_Add, 0x7)
{
	enum {PostMortemReturn = 0x71B994 , CheckNowDead = 0x71B9A7 , SetReturn = 0x71BB79};

	GET(DamageState, nState, EAX);
	GET(TerrainClass*, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, ReceiveDamageArgs, STACK_OFFS(0x3C, -0x4));

	R->EAX(nState);
	R->Stack(0x10, nState);

	auto pWarheadExt = WarheadTypeExt::ExtMap.Find(ReceiveDamageArgs.WH);

	// ignite this terrain object
	auto nDamage = ReceiveDamageArgs.Damage;
	if (!pThis->IsBurning && *nDamage > 0 && ReceiveDamageArgs.WH->Sparky)
	{
		bool spawn = pWarheadExt->Flammability.isset() ? (ScenarioClass::Instance->Random(0, 99) < abs(pWarheadExt->Flammability.Get())):true;

		if (spawn)
			pThis->Ignite();
	}

	//return handle !
	if (nState == DamageState::PostMortem)
		return PostMortemReturn;
	if (nState == DamageState::NowDead)
		return CheckNowDead;

	return SetReturn;
}

//this one on Very end of it
//let everything play first
DEFINE_HOOK(0x71BB2C, TerrainClass_TakeDamage_NowDead_Add_light, 0x6)
{
	GET(TerrainClass*, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, ReceiveDamageArgs, STACK_OFFS(0x3C, -0x4));

	if (auto const pTerrainExt = TerrainTypeExt::ExtMap.Find(pThis->Type))
	{
		auto const nCoords = pThis->GetCenterCoord();
		VocClass::PlayIndexAtPos(pTerrainExt->DestroySound.Get(-1), nCoords);

		if (auto const pAnimType = pTerrainExt->DestroyAnim.Get(nullptr))
		{
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoords))
			{
				if (AnimExt::SetAnimOwnerHouseKind(pAnim, ReceiveDamageArgs.SourceHouse, pThis->GetOwningHouse(), false))
					if (auto pAnimExt = AnimExt::GetExtData(pAnim))
						pAnimExt->Invoker = ReceiveDamageArgs.Attacker;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x71D09D, TerrainClass_UnLImbo_Light, 0x6)
{
	GET(TerrainClass*, pThis, ECX);
	TerrainExt::Unlimbo(pThis);
	return 0;
}

DEFINE_HOOK(0x71CA15, TerrainClass_Limbo_Light, 0x6)
{
	GET(bool, nLimboed, EAX);
	GET(TerrainClass*, pThis, EDI);

	if (nLimboed)
		TerrainExt::CleanUp(pThis);

	return 0;
}

/*
DEFINE_HOOK(0x71CC8C, TerrainClass_DrawIfVisible, 0x8)
{
	enum { Draw = 0x71CD8D , Skip = 0x71CC9A };

	GET(TerrainClass*, pThis, EDI);
	pThis->NeedsRedraw = false;

	bool bFogged = false;

	if (auto pCell = pThis->GetCell())
		bFogged = pCell->IsFogged();

	return pThis->InLimbo || bFogged ? Draw : Skip;
}
*/