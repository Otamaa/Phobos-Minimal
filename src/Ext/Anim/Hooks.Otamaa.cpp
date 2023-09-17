#include "Body.h"
#include <Utilities/Macro.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/AnimHelpers.h>
#include <Utilities/Helpers.h>
#include <SmudgeClass.h>
#include <SmudgeTypeClass.h>

#include <Memory.h>

void ApplyVeinsDamage(AnimClass* pThis ,int VeinDamage , WarheadTypeClass* VeinWarhead)
{
	//causing game to lock up atm
	if (pThis->Type->IsVeins && VeinWarhead)
	{
		auto coord = pThis->GetCoords();
		auto pCoorCell = MapClass::Instance->GetCellAt(coord);
		auto pFirst = pCoorCell->FirstObject;

		if (!pFirst
			|| pFirst->GetHeight() > 0
			|| pCoorCell->OverlayTypeIndex != 12
			|| pCoorCell->OverlayData < 0x30u
			|| pCoorCell->Level
			)
		{
			pThis->__ToDelete_197 = true; // wut
		}

		if ((Unsorted::CurrentFrame & 1) == 0 && pFirst)
		{
			for (auto pFist_ = pFirst; pFist_; pFist_->NextObject)
			{
				if (auto pTechno = generic_cast<TechnoClass*>(pFist_))
				{

					if (!pTechno->IsAlive || pTechno->Health <= 0 || pTechno->InLimbo)
						continue;

					const auto pType = pTechno->GetTechnoType();
					if ( (!RulesExt::Global()->VeinsDamagingWeightTreshold.isset() || pType->Weight >= RulesExt::Global()->VeinsDamagingWeightTreshold)
						&& !pType->ImmuneToVeins
						&& !pTechno->HasAbility(AbilityType::VeinProof)
						&& pTechno->GetHeight() <= 5
						)
					{
						int dmg = VeinDamage;
						pFist_->ReceiveDamage(&dmg, 0, VeinWarhead, nullptr, false, false, nullptr);
						Debug::Log("VeinAnim[%x] Damaging[%s - %x]\n", pThis, pTechno->get_ID(), pTechno);
					}
				}
			}
		}
	}
}

//DEFINE_HOOK(0x4243BC, AnimClass_AI_Veins, 0x6)
//{
//	GET(AnimClass*, pThis, ESI);
//	ApplyVeinsDamage(pThis, RulesClass::Instance->VeinDamage, RulesExt::Global()->Veinhole_Warhead);
//	return 0x0;
//}

DEFINE_HOOK(0x685078, Generate_OreTwinkle_Anims, 0x7)
{
	GET(CellClass* const, location, ESI);

	if (location->GetContainedTiberiumValue() > 0)
	{
		auto const pTibExt = TiberiumExt::ExtMap.Find(
			TiberiumClass::Array->GetItemOrDefault(location->GetContainedTiberiumIndex())
		);

		if(!pTibExt)
			return 0x0;

		if (!ScenarioClass::Instance->Random.RandomFromMax(pTibExt->GetTwinkleChance() - 1)) {
			if (auto pAnimtype = pTibExt->GetTwinkleAnim()) {
				GameCreate<AnimClass>(pAnimtype, location->GetCoords(), 1);
			}
		}
	}

	return 0x6850E5;
}

DEFINE_HOOK(0x423CC1, AnimClass_AI_HasExtras_Expired, 0x6)
{
	enum { SkipGameCode = 0x423EFD };

	GET(AnimClass* const, pThis, ESI);
	GET8(bool, LandIsWater, BL);
	GET8(bool, EligibleHeight, AL);

	//overriden instruction !
	R->Stack(STACK_OFFS(0x8C, 0x78), R->AL());

	return AnimExt::OnExpired(pThis, LandIsWater, EligibleHeight) ?
		SkipGameCode : 0x0 ;
}

//crash and corrup ESI pointer around
DEFINE_HOOK(0x424FE8, AnimClass_Middle_SpawnParticle, 0x6) //was C
{
	GET(AnimClass*, pThis, ESI);

	return AnimExt::OnMiddle(pThis) ? 0x42504D : 0x0;
}

DEFINE_HOOK(0x42504D, AnimClass_Middle_SpawnCreater, 0xA) //was 4
{
	GET(AnimClass*, pThis, ESI);
	GET_STACK(CellClass*, pCell, STACK_OFFS(0x30, 0x14));
	GET(int, nX, EBP);
	GET_STACK(int, nY, STACK_OFFS(0x30, 0x20));

	return AnimExt::OnMiddle_SpawnParticle(pThis, pCell, { nX ,nY }) ?
		0x42513F : 0x0 ;
}

DEFINE_HOOK(0x42264D, AnimClass_Init, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	GET_BASE(CoordStruct*, pCoord, 0xC);

	AnimExt::OnInit(pThis, pCoord);

	return 0x0;
}

//TODO :
//DEFINE_HOOK(0x424785, AnimClass_OnLoop, 6)
//{
//	GET(AnimClass*, pThis, ESI);
//
//	return 0;
//}

#ifdef ENABLE_NEWHOOKS
TODO : retest for desync


DEFINE_HOOK(0x4242BA, AnimClass_AI_SetCoord, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (auto const pExt = AnimExt::ExtMap.Find(pThis))
	{
		if (pExt->Something)
		{
			auto nCoord = pThis->GetCoords() + pExt->Something;
			pThis->SetLocation(nCoord);
		}
	}
	return 0x0;
}

DEFINE_HOOK(0x422CC6, AnimClass_DrawIT_SpecialDraw, 0xA)
{
	GET(AnimClass* const, pThis, ESI);

	if (auto const pTypeExt = AnimTypeExt::ExtMap.TryFind(pThis->Type))
	{
		R->AL(pTypeExt->SpecialDraw.Get());
		return 0x422CD0;
	}

	return 0x0;
}

#endif