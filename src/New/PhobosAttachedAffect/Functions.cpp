#include "Functions.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>


#include <WeaponTypeClass.h>
#include <HouseClass.h>
#include <TechnoTypeClass.h>

int PhobosAEFunctions::GetAttachedEffectCumulativeCount(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource, bool requireAnims)
{
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
	unsigned int foundCount = 0;

	for (auto const& attachEffect : pExt->PhobosAE) {
		if(!attachEffect)
			continue;

		if (attachEffect->GetType() == pAttachEffectType && attachEffect->IsActive() && (!requireAnims || !attachEffect->GetType()->HasAnim() || attachEffect->HasAnim())) {
			if (ignoreSameSource && pInvoker && pSource && attachEffect->IsFromSource(pInvoker, pSource))
				continue;

			foundCount++;
		}
	}

	return foundCount;
}

void PhobosAEFunctions::UpdateCumulativeAttachEffects(TechnoClass* pTarget, PhobosAttachEffectTypeClass* pAttachEffectType, bool createAnim)
{
	PhobosAttachEffectClass* pAELargestDuration = nullptr;
	PhobosAttachEffectClass* pAEWithAnim = nullptr;
	auto pExt = TechnoExtContainer::Instance.Find(pTarget);
	int duration = 0;
	int count = 0;

	for (auto const& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect || attachEffect->GetType() != pAttachEffectType)
			continue;

		if (attachEffect->HasCumulativeAnim)
		{
			pAEWithAnim = attachEffect.get();
		}
		else if (attachEffect->CanShowAnim())
		{
			int currentDuration = attachEffect->GetRemainingDuration();

			if (currentDuration < 0 || currentDuration > duration)
			{
				pAELargestDuration = attachEffect.get();
				duration = currentDuration;
			}
		}

		if (attachEffect->IsActive())
			count++;
	}

	if (pAEWithAnim)
		pAEWithAnim->UpdateCumulativeAnim(count);
	else if (pAELargestDuration){
		pAELargestDuration->HasCumulativeAnim = true;
		
		if (createAnim)
			pAELargestDuration->CreateAnim();
	}
}

#include <ExtraHeaders/StackVector.h>

void PhobosAEFunctions::UpdateAttachEffects(TechnoClass* pTechno)
{
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	if (pExt->PhobosAE.empty() || pTechno->IsImmobilized)
		return;

	auto const pThis = pTechno;
	bool inTunnel = pExt->IsInTunnel || pExt->IsBurrowed;
	bool markForRedraw = false;
	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons {};
	std::set<PhobosAttachEffectTypeClass*> HavecumulativeAnimTypes;
    bool altered = false;

	pExt->PhobosAE.remove_all_if([&](std::unique_ptr<PhobosAttachEffectClass>& attachEffect) {
		if(!attachEffect.get()) {
			altered = true;
		   return true;
		}

		if (!inTunnel)
			attachEffect->SetAnimationTunnelState(true);

		attachEffect->AI();

		if (attachEffect->NeedsRecalculateStat) {
			altered = true;
			attachEffect->NeedsRecalculateStat = false;
		}

 		bool hasExpired = attachEffect->HasExpired();
		bool shouldDiscard = attachEffect->IsActive() && attachEffect->ShouldBeDiscardedNow();

		if (hasExpired || shouldDiscard) {

			attachEffect->ShouldBeDiscarded = false;
			auto const pType = attachEffect->GetType();

			if (pType->HasTint() && !pTechno->InLimbo)
				markForRedraw = true;

			if (pType->Cumulative && pType->CumulativeAnimations.size() > 0 && !pTechno->InLimbo)
				HavecumulativeAnimTypes.insert(pType);

			if (pType->ExpireWeapon && ((hasExpired && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None)
				|| (shouldDiscard && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Discard) != ExpireWeaponCondition::None)))	{
					if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, pType) < 1) {
						PhobosAttachEffectClass::CumulateExpireWeapon(pType, pTechno, attachEffect->Invoker , expireWeapons);
					}
				}

			if (!(shouldDiscard && attachEffect->ResetIfRecreatable())){
				altered = true;
				return true;
			}
		}

		return false;
	});

	for(auto& cumType : HavecumulativeAnimTypes){
		PhobosAEFunctions::UpdateCumulativeAttachEffects(pTechno, cumType, false);
	}

	if(altered){
		AEProperties::Recalculate(pTechno);
		PhobosAEFunctions::UpdateAEAnimDrawingLogic(pTechno);
	}

	if (markForRedraw)
		pThis->MarkForRedraw();

	PhobosAttachEffectClass::DetonateExpireWeapon(expireWeapons, pTechno->Location);
}

void PhobosAEFunctions::UpdateAEAnimDrawingLogic(TechnoClass* pTechno)
{
	for (auto const& attachEffect : TechnoExtContainer::Instance.Find(pTechno)->PhobosAE) {
		if(attachEffect)
		attachEffect->UpdateConditionalAnimDrawingLogic();
	}
}

bool PhobosAEFunctions::HasAttachedEffects(
	 TechnoClass* pTechno,
	 std::vector<PhobosAttachEffectTypeClass*>& attachEffectTypes,
	 bool requireAll,
	 bool ignoreSameSource,
	 TechnoClass* pInvoker,
	 AbstractClass* pSource,
	 std::vector<int> const* minCounts,
	 std::vector<int> const* maxCounts, 
	bool requireAnims
	) {

	const bool checkSource = ignoreSameSource && pInvoker && pSource;
	unsigned int foundCount = 0;
	unsigned int typeCounter = 1;
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	for (auto const& type : attachEffectTypes)
	{
		if (type->Cumulative)
		{
			const int cumulativeCount = PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, type, ignoreSameSource, pInvoker, pSource, requireAnims);
			bool matched = cumulativeCount > 0;
			const unsigned int minSize = minCounts ? minCounts->size() : 0;
			const unsigned int maxSize = maxCounts ? maxCounts->size() : 0;

			if (matched && minSize > 0)
			{
				if (cumulativeCount < minCounts->at(typeCounter - 1 >= minSize ? minSize - 1 : typeCounter - 1))
					matched = false;
			}

			if (matched && maxSize > 0)
			{
				if (cumulativeCount > maxCounts->at(typeCounter - 1 >= maxSize ? maxSize - 1 : typeCounter - 1))
					matched = false;
			}
			if (matched)
			{
				// Only need to find one match, can stop here.
				if (!requireAll)
					return true;

				foundCount++;
			}
		}
		else
		{
			for (auto const& attachEffect : pExt->PhobosAE)
			{
				if (attachEffect->GetType() == type && attachEffect->IsActive() 
					&& (!requireAnims || !type->HasAnim() || attachEffect->HasAnim()))
				{
					if (checkSource && attachEffect->IsFromSource(pInvoker, pSource))
						continue;

					// Only need to find one match, can stop here.
					if (!requireAll)
						return true;

					foundCount++;
					break;
				}
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

//bool PhobosAEFunctions::HasAttachedEffects(
//	TechnoClass* pTechno,
//	PhobosAttachEffectTypeClass* attachEffectType,
//	bool requireAll,
//	bool ignoreSameSource,
//	TechnoClass* pInvoker,
//	AbstractClass* pSource,
//	std::vector<int> const* minCounts,
//	std::vector<int> const* maxCounts,
//	bool requireAnims)
//{
//	std::vector<PhobosAttachEffectTypeClass*> _dummy {};
//	_dummy.push_back(attachEffectType);
//	return PhobosAEFunctions::HasAttachedEffects(pTechno, _dummy, requireAll, ignoreSameSource, pInvoker, pSource, minCounts, maxCounts, requireAnims);
//}

void PhobosAEFunctions::UpdateSelfOwnedAttachEffects(TechnoClass* pTechno, TechnoTypeClass* pNewType)
{
	auto const pThis = pTechno;
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pNewType);
	bool markForRedraw = false;
	bool altered = false;
	int removeCount = 0;

	if (!pExt->PhobosAE.empty()){

		std::vector<std::pair<WeaponTypeClass*, TechnoClass*>>  expireWeapons {};

		// Delete ones on old type and not on current.
		pExt->PhobosAE.remove_all_if([&](std::unique_ptr<PhobosAttachEffectClass>& it) {

			if(!it.get()) {
				removeCount++;
				altered = true;
				return true;
			}

			auto const attachEffect = it.get();
			auto const pType = attachEffect->GetType();
			bool selfOwned = attachEffect->IsSelfOwned();
			bool remove = selfOwned && !pTypeExt->PhobosAttachEffects.AttachTypes.Contains(pType);

			if (remove) {
				if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None) {
					if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, pType) < 1) {
						PhobosAttachEffectClass::CumulateExpireWeapon(pType, pTechno, it->Invoker , expireWeapons);
					}
				}

				markForRedraw |= pType->HasTint();
				altered = true;
				removeCount++;
				return true;
			}

			return false;
		});

		PhobosAttachEffectClass::DetonateExpireWeapon(expireWeapons, pTechno->Location);
	}

	// Add new ones.
	const int count = PhobosAttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, &pTypeExt->PhobosAttachEffects);
	if (!count && removeCount > 0) {
		if (altered)
			AEProperties::Recalculate(pTechno);

		PhobosAEFunctions::UpdateAEAnimDrawingLogic(pTechno);
		markForRedraw = true;
	}

	if (markForRedraw)
		pThis->MarkForRedraw();
}

void PhobosAEFunctions::ApplyRevengeWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH)
{
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	for (auto& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect || !attachEffect->IsActive())
			continue;

		auto const pType = attachEffect->GetType();

		if (!pType->RevengeWeapon)
			continue;

		if (pWHExt->SuppressRevengeWeapons || (!pWHExt->SuppressRevengeWeapons_Types.empty() && pWHExt->SuppressRevengeWeapons_Types.Contains(pType->RevengeWeapon)))
			continue;

		if (pType->RevengeWeapon_UseInvokerAsOwner)
		{
			auto const pInvoker = attachEffect->GetInvoker();

			if (pInvoker && EnumFunctions::CanTargetHouse(pType->RevengeWeapon_AffectsHouses, pInvoker->Owner, pSource->Owner))
			{
				WeaponTypeExtData::DetonateAt1(pType->RevengeWeapon, pSource->IsAlive ? pSource : nullptr, pInvoker, true, nullptr);
			}
			else if (EnumFunctions::CanTargetHouse(pType->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
			{
				WeaponTypeExtData::DetonateAt1(pType->RevengeWeapon, pSource->IsAlive ? pSource : nullptr, pThis, true, nullptr);
			}
		}
	}
}

void PhobosAEFunctions::ApplyReflectDamage(TechnoClass* pThis , int* pDamage , TechnoClass* pAttacker , HouseClass* pAttacker_House, WarheadTypeClass* pWH)
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", pThis->GetThisClassName(), pThis->get_ID());
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if ((pExt->AE.flags.ReflectDamage && *pDamage > 0 && pAttacker && pAttacker->IsAlive) || pExt->AE.flags.HasOnDamageDiscardables) {
		for (auto& attachEffect : pExt->PhobosAE) {

			if (!attachEffect || !attachEffect->IsActive())
				continue;

			auto const pType = attachEffect->GetType();

			if((pExt->AE.flags.ReflectDamage && *pDamage > 0 && pAttacker && pAttacker->IsAlive)){
				if (pType->ReflectDamage &&
					!(pType->ReflectDamage_Chance.isset() && 
						Math::abs(pType->ReflectDamage_Chance.Fetch()) < ScenarioClass::Instance->Random.RandomDouble())
					&& !(pWHExt->SuppressReflectDamage && (pWHExt->SuppressReflectDamage_Types.Contains(pType) || pType->HasGroups(pWHExt->SuppressReflectDamage_Groups, false)))
					) {

						int damage = pType->ReflectDamage_Override.Get(static_cast<int>(*pDamage * pType->ReflectDamage_Multiplier));
							auto const pReflectWH = pType->ReflectDamage_Warhead.Get(RulesClass::Instance->C4Warhead);
						auto const pWHExtRef = WarheadTypeExtContainer::Instance.Find(pReflectWH);

							if (pType->ReflectDamage_UseInvokerAsOwner) {

								auto const pInvoker = attachEffect->GetInvoker();

								if (pInvoker && EnumFunctions::CanTargetHouse(pType->ReflectDamage_AffectsHouses, pInvoker->Owner, pAttacker_House))

								{
									pWHExtRef->Reflected = true;

									if (pType->ReflectDamage_Warhead_Detonate)
										WarheadTypeExtData::DetonateAt(pReflectWH, pAttacker, pInvoker, damage, pInvoker->Owner);
									else
										pAttacker->ReceiveDamage(&damage, 0, pWH, pInvoker, false, false, pInvoker->Owner);

									pWHExtRef->Reflected = false;
								}
							}
							else  if (EnumFunctions::CanTargetHouse(pType->ReflectDamage_AffectsHouses, pThis->Owner, pAttacker_House))
							{

								pWHExtRef->Reflected = true;

								if (pType->ReflectDamage_Warhead_Detonate)
									WarheadTypeExtData::DetonateAt(pReflectWH, pAttacker, pThis, damage, pThis->Owner);
								else if (pAttacker && pAttacker->IsAlive)
									pAttacker->ReceiveDamage(&damage, 0, pReflectWH, pThis, false, false, pThis->Owner);

								pWHExtRef->Reflected = false;
							}
				}
			}
	
			if (pExt->AE.flags.HasOnDamageDiscardables 
				&& (pType->DiscardOn & DiscardCondition::ReceivedDamage) != DiscardCondition::None
				&& EnumFunctions::CanTargetHouse(pType->DiscardOn_ReceivedDamage_AffectsHouse, pThis->Owner, pAttacker_House))
			{
				attachEffect->ReceivedDamageCount++;

				if (attachEffect->ReceivedDamageCount >= pType->DiscardOn_ReceivedDamage_Count)
					attachEffect->ShouldBeDiscarded = true;
			}
		}
	}
}
