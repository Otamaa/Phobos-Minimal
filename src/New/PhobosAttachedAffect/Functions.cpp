#include "Functions.h"

#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

#include <Misc/Ares/Hooks/Header.h>

#include <WeaponTypeClass.h>
#include <HouseClass.h>

int PhobosAEFunctions::GetAttachedEffectCumulativeCount(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n" , pTechno->GetThisClassName() , pTechno->get_ID());
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
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTechno->GetThisClassName(), pTechno->get_ID());
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

		if (pAttachEffectType->CumulativeAnimations.HasValue() && !pAttachEffectType->CumulativeAnimations.empty())
			attachEffect.KillAnim();
	}
}

#include <Ext/WeaponType/Body.h>

void PhobosAEFunctions::UpdateAttachEffects(TechnoClass* pTechno)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTechno->GetThisClassName(), pTechno->get_ID());
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	if (!pTechno->IsAlive || pExt->PhobosAE.begin() == pExt->PhobosAE.end())
		return;

	bool markForRedraw = false;
	const bool inTunnel = pExt->IsInTunnel || pExt->IsBurrowed;
	std::vector<WeaponTypeClass*> expireWeapons {};

	pExt->PhobosAE.remove_if([&](auto& it) {

		if (inTunnel)
			it.SetAnimationTunnelState(true);

		it.AI();

		if (it.HasExpired() || (it.IsActive() && !it.AllowedToBeActive()))
		{
			auto const pType = it.GetType();

			if (!markForRedraw && pType->HasTint())
				markForRedraw = true;

			PhobosAEFunctions::UpdateCumulativeAttachEffects(pTechno, it.GetType());

			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None) {
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, pType) < 1)
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			if (!it.AllowedToBeActive() && it.ResetIfRecreatable()) {
				return false;
			}

			return true;
		}

		return false;
	});

	AEProperties::Recalculate(pTechno);

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

bool PhobosAEFunctions::HasAttachedEffects(TechnoClass* pTechno, std::vector<PhobosAttachEffectTypeClass*>& attachEffectTypes, bool requireAll, bool ignoreSameSource,
		TechnoClass* pInvoker, AbstractClass* pSource, std::vector<int> const* minCounts, std::vector<int> const* maxCounts)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTechno->GetThisClassName(), pTechno->get_ID());
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
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTechno->GetThisClassName(), pTechno->get_ID());
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pNewType);
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	if (pExt->PhobosAE.begin() == pExt->PhobosAE.end())
		return;

	std::vector<WeaponTypeClass*> expireWeapons {};
	std::vector<PhobosAttachEffectTypeClass*> existingTypes {};

	bool markForRedraw = false;

	// Delete ones on old type and not on current.
	pExt->PhobosAE.remove_if([&](auto& it ) {
		auto const pType = it.GetType();
		const bool selfOwned = it.IsSelfOwned();

		if (selfOwned && !pTypeExt->AttachEffect_AttachTypes.Contains(pType)) {
			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None) {
				if (!pType->Cumulative
					|| !pType->ExpireWeapon_CumulativeOnlyOnce
					|| PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, pType) < 1)
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			markForRedraw |= pType->HasTint();
			return true;
		}

		if (selfOwned)
			existingTypes.push_back(pType);

		return false;
	});

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
		AEProperties::Recalculate(pTechno);

	if (markForRedraw && !hasTint)
		pTechno->MarkForRedraw();
}

void PhobosAEFunctions::ApplyRevengeWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pThis->GetThisClassName(), pThis->get_ID());
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	for (auto& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect.IsActive())
			continue;

		auto const pType = attachEffect.GetType();

		if (!pType->RevengeWeapon)
			continue;

		if (pWHExt->SuppressRevengeWeapons && (!pWHExt->SuppressRevengeWeapons_Types.empty() || pWHExt->SuppressRevengeWeapons_Types.Contains(pType->RevengeWeapon)))
			continue;

		if (EnumFunctions::CanTargetHouse(pType->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
			WeaponTypeExtData::DetonateAt(pType->RevengeWeapon, pSource->IsAlive ? pSource : nullptr, pThis, true, nullptr);
	}
}

void PhobosAEFunctions::ApplyExpireWeapon(std::vector<WeaponTypeClass*>& expireWeapons, std::set<PhobosAttachEffectTypeClass*>& cumulativeTypes, TechnoClass* pThis)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pThis->GetThisClassName(), pThis->get_ID());
	auto pTechExt = TechnoExtContainer::Instance.Find(pThis);

	for (auto const& attachEffect : pTechExt->PhobosAE) {
		auto const pType = attachEffect.GetType();
		if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Death) != ExpireWeaponCondition::None)
		{
			if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || !cumulativeTypes.contains(pType))
			{
				if (pType->Cumulative && pType->ExpireWeapon_CumulativeOnlyOnce)
					cumulativeTypes.insert(pType);

				expireWeapons.push_back(pType->ExpireWeapon);
			}
		}
	}
}

void PhobosAEFunctions::ApplyReflectDamage(TechnoClass* pThis , int* pDamage , TechnoClass* pAttacker , HouseClass* pAttacker_House, WarheadTypeClass* pWH)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pThis->GetThisClassName(), pThis->get_ID());
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pExt->AE.ReflectDamage && *pDamage > 0 && pAttacker && pAttacker->IsAlive) {
		for (auto& attachEffect : pExt->PhobosAE) {

			if (!attachEffect.IsActive())
				continue;

			auto const pType = attachEffect.GetType();

			if (!pType->ReflectDamage)
				continue;

			if (pType->ReflectDamage_Chance.isset() && abs(pType->ReflectDamage_Chance) < ScenarioClass::Instance->Random.RandomDouble())
				continue;

			if (pWHExt->SuppressReflectDamage && (pWHExt->SuppressReflectDamage_Types.Contains(pType) || pType->HasGroups(pWHExt->SuppressReflectDamage_Groups, false)))
				continue;

			int damage = pType->ReflectDamage_Override.Get(static_cast<int>(*pDamage * pType->ReflectDamage_Multiplier));
			auto const pReflectWH = pType->ReflectDamage_Warhead.Get(RulesClass::Instance->C4Warhead);

			if (EnumFunctions::CanTargetHouse(pType->ReflectDamage_AffectsHouses, pThis->Owner, pAttacker_House))
			{
				auto const pWHExtRef = WarheadTypeExtContainer::Instance.Find(pReflectWH);

				pWHExtRef->Reflected = true;

				if (pType->ReflectDamage_Warhead_Detonate)
					WarheadTypeExtData::DetonateAt(pReflectWH, pAttacker, pThis, damage, pThis->Owner);
				else if (pAttacker && pAttacker->IsAlive)
					pAttacker->ReceiveDamage(&damage, 0, pReflectWH, pThis, false, false, pThis->Owner);

				pWHExtRef->Reflected = false;
			}

			if (!pAttacker->IsAlive)
				break;
		}
	}
}
