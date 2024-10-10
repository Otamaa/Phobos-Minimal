#include "PhobosAttachEffectClass.h"

#include "PhobosAttachEffectTypeClass.h"
#include "Functions.h"
#include "AEAttachParams.h"
#include "AEAttachInfoTypeClass.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

void PhobosAttachEffectClass::Initialize(PhobosAttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse,
	TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTechno->GetThisClassName(), pTechno->get_ID());
	this->Duration = durationOverride != 0 ? durationOverride : pType->Duration;
	this->DurationOverride = durationOverride;
	this->Delay = delay;
	this->CurrentDelay = 0;
	this->InitialDelay = initialDelay;
	this->RecreationDelay = recreationDelay;
	this->Type = pType;
	this->Techno = pTechno;
	this->InvokerHouse = pInvokerHouse;
	this->Invoker = pInvoker;
	this->Source = pSource;
	this->IsAnimHidden = false;
	this->IsUnderTemporal = false;
	this->IsOnline = true;
	this->IsCloaked = false;
	this->HasInitialized = (initialDelay <= 0);
	this->NeedsDurationRefresh = false;
	this->HasCumulativeAnim = false;
	this->SelectedAnim = pType->Animation;

}

void PhobosAttachEffectClass::InvalidatePointer(AbstractClass* ptr, bool removed)
{
	if (!ptr)
		return;

	AnnounceInvalidPointer(this->Invoker, ptr, removed);
	AnnounceInvalidPointer(this->InvokerHouse, ptr);
}

// =============================
// actual logic

void PhobosAttachEffectClass::AI()
{
	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
		return;

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());

	if (this->InitialDelay > 0)
	{
		this->InitialDelay--;
		return;
	}

	if (!this->HasInitialized && this->InitialDelay == 0)
	{
		this->HasInitialized = true;

		if (this->Type->HasTint())
			this->Techno->MarkForRedraw();
	}

	if (this->CurrentDelay > 0)
	{
		this->CurrentDelay--;

		if (this->CurrentDelay == 0)
			this->NeedsDurationRefresh = true;

		return;
	}

	if (this->NeedsDurationRefresh)
	{
		if (AllowedToBeActive())
		{
			this->RefreshDuration();
			this->NeedsDurationRefresh = false;
		}

		return;
	}

	if (this->Duration > 0)
		this->Duration--;

	if (this->Duration == 0)
	{
		if (!this->IsSelfOwned() || this->Delay < 0)
			return;

		this->CurrentDelay = this->Delay;

		if (this->Delay > 0)
			this->KillAnim();
		else if (AllowedToBeActive())
			this->RefreshDuration();
		else
			this->NeedsDurationRefresh = true;

		return;
	}

	if (this->IsUnderTemporal)
		this->IsUnderTemporal = false;

	this->CloakCheck();
	this->OnlineCheck();

	if (!this->Animation && this->CanShowAnim())
		this->CreateAnim();

	this->AnimCheck();
}

void PhobosAttachEffectClass::AI_Temporal()
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());

	if (!this->IsUnderTemporal)
	{
		this->IsUnderTemporal = true;
		this->CloakCheck();

	if (!this->Animation && this->CanShowAnim())
		this->CreateAnim();

		if (this->Animation)
		{
			switch (this->Type->Animation_TemporalAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;
			case AttachedAnimFlag::Temporal:
				this->Animation->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->Animation->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->Animation->Pause();
				this->Animation->UnderTemporal = true;
				break;
			}

			this->AnimCheck();
		}
	}
}

void PhobosAttachEffectClass::AnimCheck()
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());

	if (!this->Type->Animation_HideIfAttachedWith.empty())
	{
		auto const pTechnoExt = TechnoExtContainer::Instance.Find(this->Techno);

		if (PhobosAEFunctions::HasAttachedEffects(this->Techno, this->Type->Animation_HideIfAttachedWith, false, false, nullptr, nullptr, nullptr, nullptr))
		{
			this->KillAnim();
			this->IsAnimHidden = true;
		}
		else
		{
			this->IsAnimHidden = false;

			if (!this->Animation && this->CanShowAnim())
				this->CreateAnim();
		}
	}
}

void PhobosAttachEffectClass::UpdateCumulativeAnim()
{
	if (!this->HasCumulativeAnim || !this->Animation)
		return;
	int count = PhobosAEFunctions::GetAttachedEffectCumulativeCount(this->Techno, this->Type);
	if (count < 1)
	{
		this->KillAnim();
		return;
	}
	auto const pAnimType = this->Type->GetCumulativeAnimation(count);
	if (this->Animation->Type != pAnimType)
		AnimExtData::ChangeAnimType(this->Animation, pAnimType, false, this->Type->CumulativeAnimations_RestartOnChange);
}

void PhobosAttachEffectClass::TransferCumulativeAnim(PhobosAttachEffectClass* pSource)
{
	if (!pSource || !pSource->Animation)
		return;

	this->KillAnim();
	this->Animation.swap(pSource->Animation);
	this->HasCumulativeAnim = true;
	pSource->HasCumulativeAnim = false;
}

bool PhobosAttachEffectClass::CanShowAnim() const
{
	return (!this->IsUnderTemporal || this->Type->Animation_TemporalAction != AttachedAnimFlag::Hides)
		&& (this->IsOnline || this->Type->Animation_OfflineAction != AttachedAnimFlag::Hides)
		&& !this->IsCloaked && !this->IsInTunnel && !this->IsAnimHidden;
}

void PhobosAttachEffectClass::OnlineCheck()
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());

	if (!this->Type->Powered)
		return;

	auto pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && this->Techno->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(this->Techno);
		isActive = pBuilding->IsPowerOnline();
	}

	this->IsOnline = isActive;

	if (!this->Animation)
		return;

	if (!isActive)
	{
		switch (this->Type->Animation_OfflineAction)
		{
		case AttachedAnimFlag::Hides:
			this->KillAnim();
			break;

		case AttachedAnimFlag::Temporal:
			this->Animation->UnderTemporal = true;
			break;

		case AttachedAnimFlag::Paused:
			this->Animation->Pause();
			break;

		case AttachedAnimFlag::PausedTemporal:
			this->Animation->Pause();
			this->Animation->UnderTemporal = true;
			break;
		}
	}
	else
	{
		this->Animation->UnderTemporal = false;
		this->Animation->Unpause();
	}
}

void PhobosAttachEffectClass::CloakCheck()
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	const auto cloakState = this->Techno->CloakState;

	this->IsCloaked = cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking;

	if (this->IsCloaked && this->Animation && AnimTypeExtContainer::Instance.Find(this->Animation->Type)->DetachOnCloak)
		this->KillAnim();
}

void PhobosAttachEffectClass::CreateAnim()
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (!this->Type)
		return;

	AnimTypeClass* pAnimType = nullptr;

	if (this->Type->Cumulative && this->Type->CumulativeAnimations.HasValue())
	{
		if (!this->HasCumulativeAnim || this->Type->CumulativeAnimations.empty())
			return;

		const int count = PhobosAEFunctions::GetAttachedEffectCumulativeCount(this->Techno, this->Type);
		//Debug::Log("AE[%s] cumulativeAnim [%d] from size[%d] \n", this->Type->Name.data(), count , this->Type->CumulativeAnimations.size());
		pAnimType = this->Type->GetCumulativeAnimation(count);
	}
	else
	{
		pAnimType = this->SelectedAnim;
	}

	if (!pAnimType)
		return;

	if (this->IsCloaked && AnimTypeExtContainer::Instance.Find(pAnimType)->DetachOnCloak)
		return;

	if (!this->Animation) {
		this->Animation.reset(GameCreate<AnimClass>(pAnimType, this->Techno->Location));
		this->Animation->SetOwnerObject(this->Techno);
		this->Animation->Owner = this->Type->Animation_UseInvokerAsOwner ? InvokerHouse : this->Techno->Owner;
		this->Animation->RemainingIterations = 0xFFu;
		if (this->Type->Animation_UseInvokerAsOwner) {
				AnimExtContainer::Instance.Find(this->Animation)->Invoker = Invoker;
		}
	}
}

void PhobosAttachEffectClass::KillAnim()
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (this->Animation) {
		this->Animation.clear();
	}
}

void PhobosAttachEffectClass::SetAnimationTunnelState(bool visible)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (!this->IsInTunnel && !visible)
		this->KillAnim();

	this->IsInTunnel = !visible;
}

void PhobosAttachEffectClass::RefreshDuration(int durationOverride)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (durationOverride)
		this->Duration = durationOverride;
	else
		this->Duration = this->DurationOverride ? this->DurationOverride : this->Type->Duration;

	if (this->Type->Animation_ResetOnReapply)
	{
		this->KillAnim();
		if (this->CanShowAnim())
			this->CreateAnim();
	}
}

bool PhobosAttachEffectClass::ResetIfRecreatable()
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (!this->IsSelfOwned() || this->RecreationDelay < 0)
		return false;

	this->KillAnim();
	this->Duration = 0;
	this->CurrentDelay = this->RecreationDelay;

	return true;
}

bool PhobosAttachEffectClass::AllowedToBeActive() const
{
	if (this->ShouldBeDiscarded)
		return false;

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (this->Type->DiscardOn_AbovePercent.isset() && this->Techno->GetHealthPercentage() >= this->Type->DiscardOn_AbovePercent.Get())
		return false;

	if (this->Type->DiscardOn_BelowPercent.isset() && this->Techno->GetHealthPercentage() <= this->Type->DiscardOn_BelowPercent.Get())
		return false;

	if (auto const pFoot = abstract_cast<FootClass*>(this->Techno))
	{
		bool isMoving = pFoot->Locomotor->Is_Really_Moving_Now();

		if (isMoving && (this->Type->DiscardOn & DiscardCondition::Move) != DiscardCondition::None)
			return false;

		if (!isMoving && (this->Type->DiscardOn & DiscardCondition::Stationary) != DiscardCondition::None)
			return false;
	}

	if (this->Techno->DrainingMe && (this->Type->DiscardOn & DiscardCondition::Drain) != DiscardCondition::None)
		return false;

	if (this->Techno->Target)
	{
		bool inRange = (this->Type->DiscardOn & DiscardCondition::InRange) != DiscardCondition::None;
		bool outOfRange = (this->Type->DiscardOn & DiscardCondition::OutOfRange) != DiscardCondition::None;

		if (inRange || outOfRange)
		{
			int distance = -1;

			if (this->Type->DiscardOn_RangeOverride.isset())
			{
				distance = this->Type->DiscardOn_RangeOverride.Get();
			}
			else
			{
				int weaponIndex = this->Techno->SelectWeapon(this->Techno->Target);
				auto const pWeapon = this->Techno->GetWeapon(weaponIndex)->WeaponType;

				if (pWeapon)
					distance = pWeapon->Range;
			}

			const int distanceFromTgt = this->Techno->DistanceFrom(this->Techno->Target);

			if ((inRange && distanceFromTgt <= distance) || (outOfRange && distanceFromTgt >= distance))
				return false;
		}
	}

	return true;
}

#pragma region StaticFunctions_AttachDetachTransfer

int PhobosAttachEffectClass::Attach(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
	AbstractClass* pSource, AEAttachInfoTypeClass* attachEffectInfo)
{
	auto const& types = attachEffectInfo->AttachTypes;

	if (types.size() < 1 || !pTarget)
		return false;

	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	int attachedCount = 0;
	bool markForRedraw = false;
	double ROFModifier = 1.0;
	bool selfOwned = pTarget == pSource;

	for (size_t i = 0; i < types.size(); i++)
	{
		auto const pType = types[i];
		auto const params = attachEffectInfo->GetAttachParams(i, selfOwned);

		if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(pType, pTarget, pTargetExt->PhobosAE, pInvokerHouse, pInvoker, pSource, params))
		{
			attachedCount++;

			if (params.InitialDelay <= 0)
			{
				if (pType->ROFMultiplier > 0.0 && pType->ROFMultiplier_ApplyOnCurrentTimer)
					ROFModifier *= pType->ROFMultiplier;

				if (pType->HasTint())
					markForRedraw = true;

				if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
					PhobosAEFunctions::UpdateCumulativeAttachEffects(pTarget, pType , nullptr);
			}
		}
	}

	if (attachedCount > 0)
		AEProperties::Recalculate(pTarget);

	if (markForRedraw)
		pTarget->MarkForRedraw();

	return attachedCount;
}

int PhobosAttachEffectClass::Detach(TechnoClass* pTarget, AEAttachInfoTypeClass* attachEffectInfo)
{
	if (attachEffectInfo->RemoveTypes.size() < 1 || !pTarget)
		return 0;

	return DetachTypes(pTarget, attachEffectInfo, attachEffectInfo->RemoveTypes);
}

int PhobosAttachEffectClass::DetachByGroups(TechnoClass* pTarget, AEAttachInfoTypeClass* attachEffectInfo)
{
	auto const& groups = attachEffectInfo->RemoveGroups;

	if (groups.size() < 1 || !pTarget)
		return 0;

	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	std::vector<PhobosAttachEffectTypeClass*> types;

	for (auto const& attachEffect : pTargetExt->PhobosAE)
	{
		if(!attachEffect)
			continue;

		auto const pType = attachEffect->Type;

		if (pType->HasGroups(groups, false))
			types.push_back(pType);
	}

	return DetachTypes(pTarget, attachEffectInfo, types);
}

PhobosAttachEffectClass* PhobosAttachEffectClass::CreateAndAttach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, HelperedVector<std::unique_ptr<PhobosAttachEffectClass>>& targetAEs,
	HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, AEAttachParams const& attachParams)
{
	if (!pType || !pTarget)
		return nullptr;

	if (pTarget->IsIronCurtained()) {
		const bool penetrates = pTarget->ProtectType == ProtectTypes::ForceShield
			? pType->PenetratesForceShield.Get(pType->PenetratesIronCurtain) : pType->PenetratesIronCurtain;

		if (!penetrates)
			return nullptr;
	}

	int currentTypeCount = 0;
	PhobosAttachEffectClass* match = nullptr;
	std::vector<PhobosAttachEffectClass*> cumulativeMatches;

	for (auto const& aePtr : targetAEs)
	{
		auto const attachEffect = aePtr.get();

		if (attachEffect->GetType() == pType)
		{
			currentTypeCount++;
			match = attachEffect;

			if (pType->Cumulative && (!attachParams.CumulativeRefreshSameSourceOnly || (attachEffect->Source == pSource && attachEffect->Invoker == pInvoker)))
				cumulativeMatches.push_back(attachEffect);
		}
	}

	if (pType->Cumulative)
	{
		if (pType->Cumulative_MaxCount >= 0 && currentTypeCount >= pType->Cumulative_MaxCount)
		{
			if (attachParams.CumulativeRefreshAll)
			{
				for (auto const& ae : cumulativeMatches)
				{
					ae->RefreshDuration(attachParams.DurationOverride);
				}
			}
			else
			{
				if (cumulativeMatches.size() > 0)
				{
					PhobosAttachEffectClass* best = nullptr;

					for (auto const& ae : cumulativeMatches)
					{
						if (!best || ae->Duration < best->Duration)
							best = ae;
					}

					best->RefreshDuration(attachParams.DurationOverride);
				}
			}

			return nullptr;
		}
		else if (attachParams.CumulativeRefreshAll && attachParams.CumulativeRefreshAll_OnAttach)
		{
			for (auto const& ae : cumulativeMatches)
			{
				ae->RefreshDuration(attachParams.DurationOverride);
			}
		}
	}

	if (!pType->Cumulative && currentTypeCount > 0 && match)
	{
		match->RefreshDuration(attachParams.DurationOverride);
	}
	else
	{
		targetAEs.emplace_back(std::move(std::make_unique<PhobosAttachEffectClass>()));
		auto const pAE = targetAEs.back().get();
		pAE->Initialize(pType, pTarget, pInvokerHouse, pInvoker, pSource, attachParams.DurationOverride, attachParams.Delay, attachParams.InitialDelay, attachParams.RecreationDelay);
		if (!currentTypeCount && pType->Cumulative && pType->CumulativeAnimations.size() > 0)
			pAE->HasCumulativeAnim = true;

		return pAE;
	}

	return nullptr;
}

int PhobosAttachEffectClass::DetachTypes(TechnoClass* pTarget, AEAttachInfoTypeClass* attachEffectInfo, std::vector<PhobosAttachEffectTypeClass*> const& types)
{
	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	int detachedCount = 0;
	bool markForRedraw = false;
	auto const& minCounts = attachEffectInfo->CumulativeRemoveMinCounts;
	auto const& maxCounts = attachEffectInfo->CumulativeRemoveMaxCounts;
	size_t index = 0, minSize = minCounts.size(), maxSize = maxCounts.size();

	for (auto const pType : types)
	{
		int minCount = minSize > 0 ? (index < minSize ? minCounts.operator[](index) : minCounts.operator[](minSize - 1)) : -1;
		int maxCount = maxSize > 0 ? (index < maxSize ? maxCounts.operator[](index) : maxCounts.operator[](maxSize - 1)) : -1;

		int count = PhobosAttachEffectClass::RemoveAllOfType(pType, pTarget, minCount, maxCount);

		if (count && pType->HasTint())
			markForRedraw = true;

		detachedCount += count;
		index++;
	}

	if (detachedCount > 0)
		AEProperties::Recalculate(pTarget);

	if (markForRedraw)
		pTarget->MarkForRedraw();

	return detachedCount;
}

int PhobosAttachEffectClass::RemoveAllOfType(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount)
{
	if (!pType || !pTarget)
		return 0;

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	int detachedCount = 0;
	int stackCount = -1;

	if (pType->Cumulative)
		stackCount = PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType);

	if (minCount > 0 && stackCount > -1 && pType->Cumulative && minCount > stackCount)
		return 0;

	if (pTargetExt->PhobosAE.begin() == pTargetExt->PhobosAE.end())
		return 0;

	std::vector<WeaponTypeClass*> expireWeapons {};
	pTargetExt->PhobosAE.remove_if([&](auto& it) {

		if(!it)
			return true;

		if (maxCount > 0 && detachedCount >= maxCount)
			return false;

		if (pType == it->Type) {
			detachedCount++;

			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Remove) != ExpireWeaponCondition::None) {
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType) < 2)
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			if (pType->Cumulative && !pType->CumulativeAnimations.empty())
				PhobosAEFunctions::UpdateCumulativeAttachEffects(pTarget, pType, nullptr);

			if (it->ResetIfRecreatable()) {
				return false;
			}

			return true;
		}

		return false;
	});

	auto const coords = pTarget->GetCoords();
	auto const pOwner = pTarget->Owner;
	auto _pTarget = pTarget;

	for (auto const& pWeapon : expireWeapons) {
		if (_pTarget && !_pTarget->IsAlive)
			_pTarget = nullptr;

		WeaponTypeExtData::DetonateAt(pWeapon, coords, _pTarget, _pTarget, pWeapon->Damage ,false, pOwner);
	}

	return detachedCount;
}

void PhobosAttachEffectClass::TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget)
{
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
	const auto pSourceExt = TechnoExtContainer::Instance.Find(pSource);
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

	std::vector<std::unique_ptr<PhobosAttachEffectClass>>::iterator it;

	for (it = pSourceExt->PhobosAE.begin(); it != pSourceExt->PhobosAE.end(); )
	{
		auto const attachEffect = it->get();
		if(!attachEffect) {
			it = pSourceExt->PhobosAE.erase(it);
			continue;
		}

		if (attachEffect->IsSelfOwned()) {
			++it;
			continue;
		}

		auto const type = attachEffect->GetType();
		int currentTypeCount = 0;
		PhobosAttachEffectClass* match = nullptr;
		PhobosAttachEffectClass* sourceMatch = nullptr;

		for (auto const& aePtr : pTargetExt->PhobosAE)
		{
			if(!aePtr)
				continue;

			auto const targetAttachEffect = aePtr.get();

			if (targetAttachEffect->GetType() == type)
			{
				currentTypeCount++;
				match = targetAttachEffect;

				if (targetAttachEffect->Source == attachEffect->Source && targetAttachEffect->Invoker == attachEffect->Invoker)
					sourceMatch = targetAttachEffect;
			}
		}

		if (type->Cumulative && type->Cumulative_MaxCount >= 0 && currentTypeCount >= type->Cumulative_MaxCount && sourceMatch)
		{
			sourceMatch->Duration = MaxImpl(sourceMatch->Duration, attachEffect->Duration);
		}
		else if (!type->Cumulative && currentTypeCount > 0 && match)
		{
			match->Duration = MaxImpl(match->Duration, attachEffect->Duration);
		}
		else
		{
			AEAttachParams info {};
			info.DurationOverride = attachEffect->DurationOverride;

			if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(type, pTarget, pTargetExt->PhobosAE, attachEffect->InvokerHouse, attachEffect->Invoker, attachEffect->Source, info))
				pAE->Duration = attachEffect->Duration;
		}

		it = pSourceExt->PhobosAE.erase(it);
	}
}

#pragma endregion

// =============================
// load / save

template <typename T>
bool PhobosAttachEffectClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Duration)
		.Process(this->DurationOverride)
		.Process(this->Delay)
		.Process(this->CurrentDelay)
		.Process(this->InitialDelay)
		.Process(this->RecreationDelay)
		.Process(this->Type, true)
		.Process(this->Techno, true)
		.Process(this->InvokerHouse, true)
		.Process(this->Invoker, true)
		.Process(this->Source, true)
		.Process(this->Animation, true)
		.Process(this->IsAnimHidden)
		.Process(this->IsUnderTemporal)
		.Process(this->IsOnline)
		.Process(this->IsCloaked)
		.Process(this->HasInitialized)
		.Process(this->SelectedAnim)
		.Process(this->NeedsDurationRefresh)
		.Process(this->ShouldBeDiscarded)
		.Success() && Stm.RegisterChange(this);
}

bool PhobosAttachEffectClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool PhobosAttachEffectClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<PhobosAttachEffectClass*>(this)->Serialize(Stm);
}

