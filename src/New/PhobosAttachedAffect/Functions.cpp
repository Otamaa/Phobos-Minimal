#include "Functions.h"

#include <Ext/Techno/Body.h>
#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

#include <Misc/Ares/Hooks/Header.h>

int PhobosAEFunctions::GetAttachedEffectCumulativeCount(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource)
{
	if (!pAttachEffectType->Cumulative)
		return 0;

	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	unsigned int foundCount = 0;

	for (auto const& attachEffect : pExt->PhobosAE)
	{
		if (attachEffect.GetType() == pAttachEffectType && attachEffect.IsActive())
		{
			if (ignoreSameSource && pInvoker && pSource && attachEffect.IsFromSource(pInvoker, pSource))
				continue;

			foundCount++;
		}
	}

	return foundCount;
}

// Updates state of AttachEffects of same cumulative type on techno, (which one is first active instance existing, if any), kills animations if needed.
void PhobosAEFunctions::UpdateCumulativeAttachEffects(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType)
{
	if (!pAttachEffectType || !pAttachEffectType->Cumulative)
		return;

	bool foundFirst = false;
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	for (auto& attachEffect : pExt->PhobosAE)
	{
		if (attachEffect.GetType() != pAttachEffectType || !attachEffect.IsActive())
			continue;

		if (!foundFirst)
		{
			foundFirst = true;
			attachEffect.IsFirstCumulativeInstance = true;
		}
		else
		{
			attachEffect.IsFirstCumulativeInstance = false;
		}

		if (pAttachEffectType->CumulativeAnimations.HasValue())
			attachEffect.KillAnim();
	}
}

void PhobosAEFunctions::UpdateAttachEffects(TechnoClass* pTechno)
{
	bool markForRedraw = false;
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
	bool technoIsDead = !pTechno->IsAlive;

	for (auto it = pExt->PhobosAE.begin(); it != pExt->PhobosAE.end(); )
	{
		if (technoIsDead)
			break;

		{
			if (!pExt->IsInTunnel && !pExt->IsBurrowed)
				it->SetAnimationVisibility(true);

			it->AI();

			if (technoIsDead = !pTechno->IsAlive)
				break;

			if (it->HasExpired() || (it->IsActive() && !it->AllowedToBeActive()))
			{
				Debug::Log("Erasing [%s] from [%s]\n", it->GetType()->Name.data(), pTechno->get_ID());

				auto const pType = it->GetType();

				if (pType->HasTint())
					markForRedraw = true;

				PhobosAEFunctions::UpdateCumulativeAttachEffects(pTechno, it->GetType());

				if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None)
				{
					if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, pType) < 1)
						it->ExpireWeapon();
				}

				if (technoIsDead = !pTechno->IsAlive)
					break;

				if (!it->AllowedToBeActive() && it->ResetIfRecreatable())
				{
					++it;
					continue;
				}

				it = pExt->PhobosAE.erase(it);

			}
			else
			{
				++it;
			}
		}
	}

	if (!technoIsDead)
	{
		AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pTechno)->AeData, pTechno);

		if (markForRedraw)
			pTechno->MarkForRedraw();
	}
}

bool PhobosAEFunctions::HasAttachedEffects(TechnoClass* pTechno, std::vector<PhobosAttachEffectTypeClass*>& attachEffectTypes, bool requireAll, bool ignoreSameSource,
		TechnoClass* pInvoker, AbstractClass* pSource, std::vector<int> const& minCounts, std::vector<int> const& maxCounts)
{
	unsigned int foundCount = 0;
	unsigned int typeCounter = 1;
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	for (auto const& type : attachEffectTypes)
	{
		for (auto const& attachEffect : pExt->PhobosAE)
		{
			if (attachEffect.GetType() == type && attachEffect.IsActive())
			{
				if (ignoreSameSource && pInvoker && pSource && attachEffect.IsFromSource(pInvoker, pSource))
					continue;

				unsigned int minSize = minCounts.size();
				unsigned int maxSize = maxCounts.size();

				if (type->Cumulative && (minSize > 0 || maxSize > 0))
				{
					int cumulativeCount = PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, type, ignoreSameSource, pInvoker, pSource);

					if (minSize > 0)
					{
						if (cumulativeCount < minCounts.at(typeCounter - 1 >= minSize ? minSize - 1 : typeCounter - 1))
							continue;
					}
					if (maxSize > 0)
					{
						if (cumulativeCount > maxCounts.at(typeCounter - 1 >= maxSize ? maxSize - 1 : typeCounter - 1))
							continue;
					}
				}

				// Only need to find one match, can stop here.
				if (!requireAll)
					return true;

				foundCount++;
				break;
			}
		}

		// One of the required types was not found, can stop here.
		if (requireAll && foundCount < typeCounter)
			return false;

		typeCounter++;
	}

	if (requireAll && foundCount == attachEffectTypes.size())
		return true;

	return false;
}