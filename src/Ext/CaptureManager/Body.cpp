#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>

CaptureExt::ExtContainer CaptureExt::ExtMap;

void CaptureExt::ExtData::InitializeConstants() { }

CaptureExt::ExtData* CaptureExt::GetExtData(CaptureExt::base_type* pThis)
{
	return ExtMap.Find(pThis);
}

bool CaptureExt::AllowDrawLink(TechnoTypeClass* pType)
{
	if (const auto pExt = TechnoTypeExt::ExtMap.Find(pType))
		return pExt->Draw_MindControlLink.Get();

	return true;
}

void __fastcall CaptureExt::Overload_AI(CaptureManagerClass* pThis, void* _)
{
	const auto pOwner = pThis->Owner;
	const auto pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

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
			const int nNodeCount = pThis->ControlNodes.Count;

			/* Vanilla code !
			int MaxVal = 0;
			if (nNodeCount > *OverloadCount.begin()) {

				do {
					if (nCurIdx >= static_cast<int>((OverloadCount.size())) - 1) {
						break;
					}

					MaxVal = *(OverloadCount.begin()+1);
					++nCurIdx;
				} while (nNodeCount > MaxVal);
			}
			*/

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

bool CaptureExt::CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget)
{
	if (pManager->MaxControlNodes == 1)
		return pManager->CanCapture(pTarget);

	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType());
	if (pTechnoTypeExt && pTechnoTypeExt->MultiMindControl_ReleaseVictim)
	{
		// I hate Ares' completely rewritten things - secsome
		pManager->MaxControlNodes += 1;
		bool result = pManager->CanCapture(pTarget);
		pManager->MaxControlNodes -= 1;
		return result;
	}

	return pManager->CanCapture(pTarget);
}

bool CaptureExt::FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bSilent)
{
	if (pTarget)
	{
		for (int i = pManager->ControlNodes.Count - 1; i >= 0; --i)
		{
			const auto pNode = pManager->ControlNodes[i];
			if (pTarget == pNode->Unit)
			{
				if (pTarget->MindControlRingAnim)
				{
					pTarget->MindControlRingAnim->UnInit();
					pTarget->MindControlRingAnim = nullptr;
				}

				if (!bSilent)
				{
					int nSound = pTarget->GetTechnoType()->MindClearedSound;

					if (nSound == -1)
						nSound = RulesClass::Instance->MindClearedSound;
					if (nSound != -1)
						VocClass::PlayIndexAtPos(nSound, pTarget->GetCoords());
				}

				// Fix : Player defeated should not get this unit.
				auto const pOriginOwner = pNode->OriginalOwner->Defeated ?
					HouseExt::FindNeutral() : pNode->OriginalOwner;

				pTarget->SetOwningHouse(pOriginOwner);
				pManager->DecideUnitFate(pTarget);
				pTarget->MindControlledBy = nullptr;

				if (pNode)
					GameDelete(pNode);

				pManager->ControlNodes.RemoveItem(i);

				return true;
			}
		}
	}

	return false;
}

bool CaptureExt::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget,
	bool bRemoveFirst, AnimTypeClass* pControlledAnimType)
{
	if (CaptureExt::CanCapture(pManager, pTarget))
	{
		if (pManager->MaxControlNodes <= 0)
			return false;

		if (!pManager->InfiniteMindControl)
		{
			if (pManager->MaxControlNodes == 1 && pManager->ControlNodes.Count == 1)
				CaptureExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
			else if (pManager->ControlNodes.Count == pManager->MaxControlNodes)
				if (bRemoveFirst)
					CaptureExt::FreeUnit(pManager, pManager->ControlNodes[0]->Unit);
		}

		if (auto pControlNode = GameCreate<ControlNode>(pTarget, pTarget->Owner, RulesClass::Instance->MindControlAttackLineFrames))
		{
			pManager->ControlNodes.AddItem(pControlNode);

			if (pTarget->SetOwningHouse(pManager->Owner->Owner))
			{
				pTarget->MindControlledBy = pManager->Owner;

				pManager->DecideUnitFate(pTarget);

				auto const pBld = specific_cast<BuildingClass*>(pTarget);
				auto const pType = pTarget->GetTechnoType();
				CoordStruct location = pTarget->GetCoords();

				if (pBld)
					location.Z += pBld->Type->Height * Unsorted::LevelHeight;
				else
					location.Z += pType->MindControlRingOffset;

				if (auto const pAnimType = pControlledAnimType)
				{
					if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location))
					{
						pTarget->MindControlRingAnim = pAnim;
						pAnim->SetOwnerObject(pTarget);

						if (pBld)
							pAnim->ZAdjust = -1024;
					}
				}

//#ifdef ENABLE_NEWHOOKS
				if (abstract_cast<FootClass*>(pTarget) && pTarget->Target && pManager->Owner->IsHumanControlled) {
					pTarget->SetTarget(nullptr);
					pTarget->Override_Mission(Mission::Guard, nullptr,nullptr);
					pTarget->SetDestination(pTarget->GetCell(), true);
				}
//#endif
				return true;
			}
		}
	}

	return false;
}

bool CaptureExt::CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTechno, AnimTypeClass* pControlledAnimType)
{
	if (pTechno)
	{
		const auto pTarget = pTechno->AbsDerivateID & AbstractFlags::Techno ? pTechno : nullptr;

		bool bRemoveFirst = false;
		if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pManager->Owner->GetTechnoType()))
			bRemoveFirst = pTechnoTypeExt->MultiMindControl_ReleaseVictim;

		return CaptureExt::CaptureUnit(pManager, pTarget, bRemoveFirst, pControlledAnimType);
	}

	return false;
}

void CaptureExt::DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot)
{
	// to be implemented (if needed). - secsome
}

// =============================
// load / save

template <typename T>
void CaptureExt::ExtData::Serialize(T& Stm) { }

void CaptureExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<CaptureManagerClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void CaptureExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<CaptureManagerClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool CaptureExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool CaptureExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}
// =============================
// container

CaptureExt::ExtContainer::ExtContainer() : Container("CaptureManagerClass") { };
CaptureExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks


#ifndef ENABLE_NEWHOOKS
DEFINE_HOOK(0x471887, CaptureManagerClass_CTOR, 0x6)
{
	GET(CaptureManagerClass* const, pItem, ESI);
#ifdef ENABLE_NEWHOOKS
	CaptureExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	CaptureExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x4729EF, CaptureManagerClass_DTOR, 0x7)
{
	GET(CaptureManagerClass* const, pItem, ESI);
	CaptureExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x4728E0, CaptureManagerClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x472720, CaptureManagerClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(CaptureManagerClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	CaptureExt::ExtMap.PrepareStream(pThis, pStm);
	return 0;
}

DEFINE_HOOK(0x4728CA, CaptureManagerClass_Load_Suffix, 0x7)
{
	CaptureExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x472958, CaptureManagerClass_Save_Suffix, 0x7)
{
	CaptureExt::ExtMap.SaveStatic();
	return 0;
}
#endif