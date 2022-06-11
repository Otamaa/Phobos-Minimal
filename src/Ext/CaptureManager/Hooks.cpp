
#include "Body.h"
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

static void __stdcall DrawALinkTo(CoordStruct nFrom, CoordStruct nTo, ColorStruct color)
{
	JMP_STD(0x704E40);
}

DEFINE_HOOK(0x4721E6, CaptureManagerClass_DrawLinkToVictim, 0xC)
{
	GET(CaptureManagerClass*, pThis, EDI);
	GET(TechnoClass*, pVictim, ECX);
	GET_STACK(int, nNodeCount, STACK_OFFS(0x30, 0x1C));

	auto pAttacker = pThis->Owner;
	auto pAttackerType = pAttacker->GetTechnoType();

	auto const Allow = [pAttackerType]()
	{
		if (const auto pExt = TechnoTypeExt::ExtMap.Find(pAttackerType))
			return pExt->Draw_MindControlLink.Get();

		return true;
	};

	if (Allow())
	{
		auto nVictimCoord = pVictim->Location;
		nVictimCoord.Z += pAttackerType->LeptonMindControlOffset;
		auto nFLH = pAttacker->GetFLH(-1 - nNodeCount % 5, CoordStruct::Empty);
		DrawALinkTo(nFLH, nVictimCoord, pAttacker->Owner->Color);
	}

	R->EBP(nNodeCount);
	return 0x472287;
}

void __fastcall CaptureManagerClass_Overload_AI(CaptureManagerClass* pThis, void* _)
{
	auto pOwner = pThis->Owner;
	auto pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

	if (!pOwnerTypeExt) // we cant find type Ext for this , just return to original function !
	{
		pThis->HandleOverload();
		return;
	}

	if (pThis->InfiniteMindControl)
	{
		if (pThis->OverloadPipState > 0)
			--pThis->OverloadPipState;

		if (pThis->OverloadDamageDelay <= 0)
		{
			auto const OverloadCount = pOwnerTypeExt->Overload_Count.GetElements(RulesGlobal->OverloadCount);

			if (OverloadCount.empty())
				return;

			int nCurIdx = 0;
			int nNodeCount = pThis->ControlNodes.Count;

			for (int i = 0; i < (int)(OverloadCount.size()); ++i)
			{
				if (nNodeCount > OverloadCount[i])
					nCurIdx = i + 1;
			}

			// nCuridx is following Overload .size , so it may causing problem if other vector .size not match Overload .size
			// fixed it !
			constexpr auto FixIdx = [](const Iterator<int>& iter, int nInput) {
				return iter.empty() ? 0 : iter[nInput > (int)iter.size() ? (int)iter.size() : nInput];
			};

			auto const nOverloadfr = pOwnerTypeExt->Overload_Frames.GetElements(RulesGlobal->OverloadFrames);
			pThis->OverloadDamageDelay = FixIdx(nOverloadfr, nCurIdx);

			auto const nOverloadDmg = pOwnerTypeExt->Overload_Damage.GetElements(RulesGlobal->OverloadDamage);
			auto nDamage = FixIdx(nOverloadDmg, nCurIdx);

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
					pOwner->RockingSidewaysPerFrame = (float)nCopied_base;
				}
			}
		}
		else
		{
			--pThis->OverloadDamageDelay;
		}
	}
}

DEFINE_POINTER_CALL(0x6FA730, &CaptureManagerClass_Overload_AI);
