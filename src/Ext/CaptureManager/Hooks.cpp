
#include "Body.h"
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x47257C, CaptureManagerClass_TeamChooseAction_Random, 0x6)
{
	GET(FootClass*, pFoot, EAX);

	if (auto pTeam = pFoot->Team)
	{
		if (auto nTeamDecision = pTeam->Type->MindControlDecision)
		{
			if (nTeamDecision > 5)
				nTeamDecision = ScenarioClass::Instance->Random.RandomRanged(1, 5);

			R->EAX(nTeamDecision);
			return 0x47258F;
		}
	}

	return 0x4725B0;
}

DEFINE_HOOK(0x4721E6, CaptureManagerClass_DrawLinkToVictim, 0x6) //C
{
	GET(CaptureManagerClass*, pThis, EDI);
	GET(TechnoClass*, pVictim, ECX);
	GET_STACK(int, nNodeCount, STACK_OFFS(0x30, 0x1C));

	const auto pAttacker = pThis->Owner;
	const auto pAttackerType = pAttacker->GetTechnoType();

	if (CaptureExt::AllowDrawLink(pAttackerType))
	{
		auto nVictimCoord = pVictim->Location;
		nVictimCoord.Z += pAttackerType->LeptonMindControlOffset;
		const auto nFLH = pAttacker->GetFLH(-1 - nNodeCount % 5, CoordStruct::Empty);
		CaptureExt::DrawLinkTo(nFLH, nVictimCoord, pAttacker->Owner->Color);
	}

	R->EBP(nNodeCount);
	return 0x472287;
}

DEFINE_HOOK(0x6FA726, TechnoClass_AI_Overload_AI_Replace, 0x6)
{
	GET(TechnoClass*, pOwner, ESI);

	if (auto pThis = pOwner->CaptureManager)
	{
		const auto pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

		//if (!pOwnerTypeExt) // we cant find type Ext for this , just return to original function !
		//{
		//	pThis->HandleOverload();
		//	return 0x6FA735;
		//}

		if (pThis->InfiniteMindControl)
		{
			if (pThis->OverloadPipState > 0)
				--pThis->OverloadPipState;

			if (pThis->OverloadDamageDelay <= 0)
			{
				auto const OverloadCount = pOwnerTypeExt->Overload_Count.GetElements(RulesGlobal->OverloadCount);

				if (OverloadCount.empty())
					return 0x6FA735;

				int nCurIdx = 0;
				const int nNodeCount = pThis->ControlNodes.Count;

				for (int i = 0; i < (int)(OverloadCount.size()); ++i)
				{
					if (nNodeCount > OverloadCount[i])
					{
						nCurIdx = i + 1;
					}
				}

				auto const nOverloadfr = pOwnerTypeExt->Overload_Frames.GetElements(RulesGlobal->OverloadFrames);
				pThis->OverloadDamageDelay = CaptureExt::FixIdx(nOverloadfr, nCurIdx);

				auto const nOverloadDmg = pOwnerTypeExt->Overload_Damage.GetElements(RulesGlobal->OverloadDamage);
				auto nDamage = CaptureExt::FixIdx(nOverloadDmg, nCurIdx);

				if (nDamage <= 0)
				{
					pThis->OverloadDeathSoundPlayed = false;
				}
				else
				{
					pThis->OverloadPipState = 10;
					pOwner->ReceiveDamage(&nDamage, 0, RulesGlobal->C4Warhead, 0, 0, 0, 0);

					if (!pThis->OverloadDeathSoundPlayed)
					{
						VocClass::PlayAt(pOwnerTypeExt->Overload_DeathSound.Get(RulesGlobal->MasterMindOverloadDeathSound), pOwner->Location, 0);
						pThis->OverloadDeathSoundPlayed = true;
					}

					if (auto const pParticle = pOwnerTypeExt->Overload_ParticleSys.Get(RulesGlobal->DefaultSparkSystem))
					{
						for (int i = pOwnerTypeExt->Overload_ParticleSysCount.Get(5); i > 0; --i)
						{
							auto const nRandomY = ScenarioGlobal->Random.RandomRanged(-200, 200);
							auto const nRamdomX = ScenarioGlobal->Random.RandomRanged(-200, 200);
							CoordStruct nParticleCoord { pOwner->Location.X + nRamdomX, nRandomY + pOwner->Location.Y, pOwner->Location.Z + 100 };
							GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
						}
					}

					if (nCurIdx > 0 && pOwner->IsAlive)
					{
						double const nBase = (nCurIdx != 1) ? 0.015 : 0.029999999;
						double const nCopied_base = (ScenarioGlobal->Random.RandomRanged(0, 100) < 50) ? -nBase : nBase;
						pOwner->RockingSidewaysPerFrame = static_cast<float>(nCopied_base);
					}
				}
			}
			else
			{
				--pThis->OverloadDamageDelay;
			}
		}
	}

	return 0x6FA735;
}

DEFINE_HOOK(0x471D40, CaptureManagerClass_CaptureUnit, 0x7)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureExt::CaptureUnit(pThis, pTechno));

	return 0x471D5A;
}

DEFINE_HOOK(0x471FF0, CaptureManagerClass_FreeUnit, 0x8)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureExt::FreeUnit(pThis, pTechno));

	return 0x472006;
}

DEFINE_HOOK(0x6FCB34, TechnoClass_CanFire_CanCapture, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);

	R->AL(CaptureExt::CanCapture(pThis->CaptureManager, pTarget));

	return 0x6FCB40;
}