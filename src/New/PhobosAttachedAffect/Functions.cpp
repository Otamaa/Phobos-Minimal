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

#include <Ext/WeaponType/Body.h>

void PhobosAEFunctions::UpdateAttachEffects(TechnoClass* pTechno)
{
	bool markForRedraw = false;
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
	bool inTunnel = pExt->IsInTunnel || pExt->IsBurrowed;
	bool technoIsDead = !pTechno->IsAlive;
	std::vector<WeaponTypeClass*> expireWeapons;

	for (auto it = pExt->PhobosAE.begin(); it != pExt->PhobosAE.end(); )
	{
		if (technoIsDead)
			break;

		{
			if (inTunnel)
				it->SetAnimationTunnelState(true);

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

				if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None) {
					if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, pType) < 1)
						expireWeapons.push_back(pType->ExpireWeapon);
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

		auto coords = pTechno->GetCoords();
		auto const pOwner = pTechno->Owner;

		for (auto const& pWeapon : expireWeapons) {

			TechnoClass* pTarget = pTechno;
			if(!pTechno->IsAlive)
				pTarget = nullptr;

			WeaponTypeExtData::DetonateAt(pWeapon, coords, pTarget, false, pOwner);
		}
	}
}

bool PhobosAEFunctions::HasAttachedEffects(TechnoClass* pTechno, std::vector<PhobosAttachEffectTypeClass*>& attachEffectTypes, bool requireAll, bool ignoreSameSource,
		TechnoClass* pInvoker, AbstractClass* pSource, std::vector<int> const* minCounts, std::vector<int> const* maxCounts)
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


				if (type->Cumulative && (minCounts || maxCounts))
				{
					int cumulativeCount = PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, type, ignoreSameSource, pInvoker, pSource);

					unsigned int minSize = minCounts ? minCounts->size() : 0;
					unsigned int maxSize = maxCounts ? maxCounts->size() : 0;

					if (minSize > 0)
					{
						if (cumulativeCount < minCounts->operator[](typeCounter - 1 >= minSize ? minSize - 1 : typeCounter - 1))
							continue;
					}
					if (maxSize > 0)
					{
						if (cumulativeCount > maxCounts->operator[](typeCounter - 1 >= maxSize ? maxSize - 1 : typeCounter - 1))
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

#include <Ext/TechnoType/Body.h>
#include <TechnoTypeClass.h>
#include <Ext/WeaponType/Body.h>

void PhobosAEFunctions::UpdateSelfOwnedAttachEffects(TechnoClass* pTechno, TechnoTypeClass* pNewType) {

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pNewType);
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
	std::vector<WeaponTypeClass*> expireWeapons;
	std::vector<PhobosAttachEffectTypeClass*> existingTypes;

	bool markForRedraw = false;

	// Delete ones on old type and not on current.
	for (auto it = pExt->PhobosAE.begin(); it != pExt->PhobosAE.end(); )
	{
		auto const pType = it->GetType();
		bool selfOwned = it->IsSelfOwned();
		bool remove = selfOwned && !pTypeExt->AttachEffect_AttachTypes.Contains(pType);

		if (remove)
		{
			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None) {
				if (!pType->Cumulative
					|| !pType->ExpireWeapon_CumulativeOnlyOnce
					|| PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno , pType) < 1)
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			markForRedraw |= pType->HasTint();
			it = pExt->PhobosAE.erase(it);
		}
		else
		{
			if (selfOwned)
				existingTypes.push_back(pType);

			it++;
		}
	}

	auto const coords = pTechno->GetCoords();
	auto const pOwner = pTechno->Owner;

	for (auto const& pWeapon : expireWeapons) {
		WeaponTypeExtData::DetonateAt(pWeapon, coords, pTechno , pWeapon->Damage,true, pOwner);

		if (!pTechno->IsAlive)
			return;
	}

	bool attached = false;
	bool hasTint = false;

	// Add new ones.
	for (size_t i = 0; i < pTypeExt->AttachEffect_AttachTypes.size(); i++)
	{
		auto const pAEType = pTypeExt->AttachEffect_AttachTypes[i];

		// Skip ones that are already there.
		if (std::count(existingTypes.begin(), existingTypes.end(), pAEType))
			continue;

		int durationOverride = 0;
		int delay = 0;
		int initialDelay = 0;
		int recreationDelay = -1;

		PhobosAttachEffectClass::SetValuesHelper(i, pTypeExt->AttachEffect_DurationOverrides, pTypeExt->AttachEffect_Delays, pTypeExt->AttachEffect_InitialDelays, pTypeExt->AttachEffect_RecreationDelays, durationOverride, delay, initialDelay, recreationDelay);
		bool wasAttached = PhobosAttachEffectClass::Attach(pAEType, pTechno, pOwner, pTechno, pTechno, durationOverride, delay, initialDelay, recreationDelay);

		attached |= initialDelay <= 0 && wasAttached;
		hasTint |= wasAttached && pAEType->HasTint();
	}

	if (!attached)
		AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pTechno)->AeData, pTechno);

	if (markForRedraw && !hasTint)
		pTechno->MarkForRedraw();
}
