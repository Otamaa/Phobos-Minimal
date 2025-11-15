#include "Functions.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

#include <Misc/Ares/Hooks/Header.h>

#include <WeaponTypeClass.h>
#include <HouseClass.h>
#include <TechnoTypeClass.h>

int PhobosAEFunctions::GetAttachedEffectCumulativeCount(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource)
{
	if (!pAttachEffectType->Cumulative)
		return 0;

	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
	unsigned int foundCount = 0;

	for (auto const& attachEffect : pExt->PhobosAE) {
		if(!attachEffect)
			continue;

		if (attachEffect->GetType() == pAttachEffectType && attachEffect->IsActive()) {
			if (ignoreSameSource && pInvoker && pSource && attachEffect->IsFromSource(pInvoker, pSource))
				continue;

			foundCount++;
		}
	}

	return foundCount;
}

void PhobosAEFunctions::UpdateCumulativeAttachEffects(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pAttachEffectType, PhobosAttachEffectClass* pRemoved)
{
	PhobosAttachEffectClass* pAELargestDuration = nullptr;
	PhobosAttachEffectClass* pAEWithAnim = nullptr;
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
	int duration = 0;
	int count = 0;

	for (auto const& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect || attachEffect->GetType() != pAttachEffectType)
			continue;

		count++;

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
	}

	if (pAEWithAnim)
	{
		pAEWithAnim->UpdateCumulativeAnim(count);

		if (pRemoved == pAEWithAnim)
		{
			pAEWithAnim->HasCumulativeAnim = false;

			if (pAELargestDuration)
				pAELargestDuration->TransferCumulativeAnim(pAEWithAnim);
		}
	}
}

#include <ExtraHeaders/StackVector.h>

void PhobosAEFunctions::UpdateAttachEffects(TechnoClass* pTechno)
{
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	if (pExt->PhobosAE.empty())
		return;

	auto const pThis = pTechno;
	bool inTunnel = pExt->IsInTunnel || pExt->IsBurrowed;
	bool markForRedraw = false;
	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons {};
    bool altered = false;

	pExt->PhobosAE.remove_all_if([&](std::unique_ptr<PhobosAttachEffectClass>& attachEffect) {
		if(!attachEffect.get()) {
			altered = true;
		   return true;
		}

		if (!inTunnel)
			attachEffect->SetAnimationTunnelState(true);

		attachEffect->AI();
 		bool hasExpired = attachEffect->HasExpired();
		bool shouldDiscard = attachEffect->IsActive() && attachEffect->ShouldBeDiscardedNow();

		if (hasExpired || shouldDiscard) {

			attachEffect->ShouldBeDiscarded = false;
			auto const pType = attachEffect->GetType();

			if (pType->HasTint())
				markForRedraw = true;

			if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
				PhobosAEFunctions::UpdateCumulativeAttachEffects(pTechno , attachEffect->GetType(), attachEffect.get());

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

	if(altered){
		AEProperties::Recalculate(pTechno);
		AEProperties::UpdateAEAnimLogic(pTechno);
	}

	if (markForRedraw)
		pThis->MarkForRedraw();

	PhobosAttachEffectClass::DetonateExpireWeapon(expireWeapons);
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
	unsigned int foundCount = 0;
	unsigned int typeCounter = 1;
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	for (auto const& type : attachEffectTypes)
	{
		for (auto const& attachEffect : pExt->PhobosAE)
		{
			if(!attachEffect)
				continue;

			if (attachEffect->Type == type
				&& attachEffect->IsActive()
				&& (!requireAnims || !attachEffect->Type->HasAnim() || attachEffect->HasAnim())
			) {
				if (ignoreSameSource && pInvoker && pSource && attachEffect->IsFromSource(pInvoker, pSource))
					continue;

				unsigned int minSize = minCounts ? minCounts->size() : 0;
				unsigned int maxSize = maxCounts ? maxCounts->size() : 0;

				if (type->Cumulative && (minSize > 0 || maxSize > 0)) {

					int cumulativeCount = PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTechno, type, ignoreSameSource, pInvoker, pSource);

					if (minSize > 0 && (cumulativeCount < minCounts->operator[](typeCounter - 1 >= minSize ? minSize - 1 : typeCounter - 1))) {
						continue;
					}

					if (maxSize > 0 && (cumulativeCount > maxCounts->operator[](typeCounter - 1 >= maxSize ? maxSize - 1 : typeCounter - 1))) {
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

	if (!pExt->PhobosAE.empty()){

		std::vector<std::pair<WeaponTypeClass*, TechnoClass*>>  expireWeapons {};

		// Delete ones on old type and not on current.
		pExt->PhobosAE.remove_all_if([&](std::unique_ptr<PhobosAttachEffectClass>& it) {

			if(!it.get()) {
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
				return true;
			}

			return false;
		});

		PhobosAttachEffectClass::DetonateExpireWeapon(expireWeapons);
	}

	// Add new ones.
	int count = PhobosAttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, &pTypeExt->PhobosAttachEffects);

	if (altered && !count){
		AEProperties::Recalculate(pTechno);
		AEProperties::UpdateAEAnimLogic(pTechno);
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

	if (pExt->AE.flags.ReflectDamage && *pDamage > 0 && pAttacker && pAttacker->IsAlive) {
		for (auto& attachEffect : pExt->PhobosAE) {

			if (!attachEffect || !attachEffect->IsActive())
				continue;

			auto const pType = attachEffect->GetType();

			if (!pType->ReflectDamage)
				continue;

			if (pType->ReflectDamage_Chance.isset() && Math::abs(pType->ReflectDamage_Chance.Get()) < ScenarioClass::Instance->Random.RandomDouble())
				continue;

			if (pWHExt->SuppressReflectDamage && (pWHExt->SuppressReflectDamage_Types.Contains(pType) || pType->HasGroups(pWHExt->SuppressReflectDamage_Groups, false)))
				continue;

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

			if (!pAttacker->IsAlive)
				break;
		}
	}
}
