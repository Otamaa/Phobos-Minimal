#include "PhobosAttachEffectClass.h"

#include "PhobosAttachEffectTypeClass.h"
#include "Functions.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>

void PhobosAttachEffectClass::Initialize(PhobosAttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse,
	TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
{
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
	this->IsFirstCumulativeInstance = false;

	if(!pType->Animation.empty()) {
		if (pType->Animation.size() == 1 || !pType->AnimRandomPick)
			this->SelectedAnim = pType->Animation[0];
		else if (pType->AnimRandomPick && pType->Animation.size() > 1)
			this->SelectedAnim = pType->Animation[ScenarioClass::Instance()->Random.RandomFromMax(pType->Animation.size() - 1)];
	}
}

void PhobosAttachEffectClass::InvalidatePointer(AbstractClass* ptr, bool removed)
{
	auto const absType = ptr->WhatAmI();

	if (absType == AbstractType::Anim)
	{
		if (ptr == this->Animation.get())
			this->Animation.clear();
	}
	else if ((ptr->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
	{
		AnnounceInvalidPointer(this->Invoker, ptr);
	}
	else if (absType == AbstractType::House)
	{
		AnnounceInvalidPointer(this->InvokerHouse, ptr);
	}
}

// =============================
// actual logic

void PhobosAttachEffectClass::AI()
{
	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
		return;

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

	if (!this->Animation && !this->IsUnderTemporal && this->IsOnline && !this->IsCloaked && !this->IsAnimHidden)
		this->CreateAnim();
}

void PhobosAttachEffectClass::AI_Temporal()
{
	if (!this->IsUnderTemporal)
	{
		this->IsUnderTemporal = true;

		this->CloakCheck();

		if (!this->Animation && this->Type->Animation_TemporalAction != AttachedAnimFlag::Hides && this->IsOnline && !this->IsCloaked && !this->IsAnimHidden)
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
		}
	}
}

void PhobosAttachEffectClass::OnlineCheck()
{
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
	const auto cloakState = this->Techno->CloakState;

	if (cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking)
	{
		this->IsCloaked = true;
		this->KillAnim();
	}
	else
	{
		this->IsCloaked = false;
	}
}

void PhobosAttachEffectClass::CreateAnim()
{
	if (!this->Type)
		return;

	AnimTypeClass* pAnimType = nullptr;

	if (this->Type->Cumulative && this->Type->CumulativeAnimations.HasValue())
	{
		if (!this->IsFirstCumulativeInstance || this->Type->CumulativeAnimations.empty())
			return;

		const int count = PhobosAEFunctions::GetAttachedEffectCumulativeCount(this->Techno, this->Type);
		//Debug::Log("AE[%s] cumulativeAnim [%d] from size[%d] \n", this->Type->Name.data(), count , this->Type->CumulativeAnimations.size());
		pAnimType = this->Type->GetCumulativeAnimation(count);
	}
	else
	{
		pAnimType = this->SelectedAnim;
	}

	if (!this->Animation && pAnimType)
	{
		if (auto const pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location))
		{
			pAnim->SetOwnerObject(this->Techno);
			pAnim->Owner = this->Type->Animation_UseInvokerAsOwner ? InvokerHouse : this->Techno->Owner;
			pAnim->RemainingIterations = 0xFFu;
			this->Animation.reset(pAnim);

			if (this->Type->Animation_UseInvokerAsOwner)
			{
				auto const pAnimExt = AnimExtContainer::Instance.Find(pAnim);
				pAnimExt->Invoker = Invoker;
			}
		}
	}
}

void PhobosAttachEffectClass::KillAnim()
{
	if (this->Animation)
	{
		this->Animation.clear();
	}
}

void PhobosAttachEffectClass::SetAnimationVisibility(bool visible)
{
	if (!this->IsAnimHidden && !visible)
		this->KillAnim();

	this->IsAnimHidden = !visible;
}

void PhobosAttachEffectClass::RefreshDuration(int durationOverride)
{
	if (durationOverride)
		this->Duration = durationOverride;
	else
		this->Duration = this->DurationOverride ? this->DurationOverride : this->Type->Duration;

	if (this->Type->Animation_ResetOnReapply)
	{
		this->KillAnim();
		this->CreateAnim();
	}
}

bool PhobosAttachEffectClass::ResetIfRecreatable()
{
	if (!this->IsSelfOwned() || this->RecreationDelay < 0)
		return false;

	this->KillAnim();
	this->Duration = 0;
	this->CurrentDelay = this->RecreationDelay;

	return true;
}

bool PhobosAttachEffectClass::AllowedToBeActive() const
{
	if (auto const pFoot = abstract_cast<FootClass*>(this->Techno))
	{
		bool isMoving = pFoot->Locomotor->Is_Moving();

		if (isMoving && (Type->DiscardOn & DiscardCondition::Move) != DiscardCondition::None)
			return false;

		if (!isMoving && (Type->DiscardOn & DiscardCondition::Stationary) != DiscardCondition::None)
			return false;
	}

	if (this->Techno->DrainingMe && (Type->DiscardOn & DiscardCondition::Drain) != DiscardCondition::None)
		return false;

	return true;
}

void PhobosAttachEffectClass::ExpireWeapon() const
{
	TechnoExtData::FireWeaponAtSelf(this->Techno, this->Type->ExpireWeapon.Get());
}

#pragma region StaticFunctions_AttachDetachTransfer

/// <summary>
/// Creates and attaches AttachEffect of a given type to a techno.
/// </summary>
/// <param name="pType">AttachEffect type.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="pInvokerHouse">House that invoked the attachment.</param>
/// <param name="pInvoker">Techno that invoked the attachment.</param>
/// <param name="pSource">Source object for the attachment e.g a Warhead or Techno.</param>
/// <param name="durationOverride">Override for AttachEffect duration.</param>
/// <param name="delay">Override for AttachEffect delay.</param>
/// <param name="initialDelay">Override for AttachEffect initial delay.</param>
/// <param name="recreationDelay">Override for AttachEffect recreation delay.</param>
/// <returns>True if successful, false if not.</returns>
bool PhobosAttachEffectClass::Attach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
	AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
{
	if (!pType || !pTarget)
		return false;

	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

	if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(pType, pTarget, pTargetExt->PhobosAE, pInvokerHouse, pInvoker, pSource, durationOverride, delay, initialDelay, recreationDelay))
	{
		if (initialDelay <= 0)
		{
			AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pTarget)->AeData, pTarget);

			if (pType->HasTint())
				pTarget->MarkForRedraw();
		}

		return true;
	}

	return false;
}

/// <summary>
/// Creates and attaches AttachEffects of given types to a techno.
/// </summary>
/// <param name="types">List of AttachEffect types.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="pInvokerHouse">House that invoked the attachment.</param>
/// <param name="pInvoker">Techno that invoked the attachment.</param>
/// <param name="pSource">Source object for the attachment e.g a Warhead or Techno.</param>
/// <param name="durationOverrides">Overrides for AttachEffect duration.</param>
/// <param name="delays">Overrides for AttachEffect delay.</param>
/// <param name="initialDelays">Overrides for AttachEffect initial delay.</param>
/// <param name="recreationDelays">Overrides for AttachEffect recreation delay.</param>
/// <returns>Number of AttachEffect instances created and attached.</returns>
int PhobosAttachEffectClass::Attach(std::vector<PhobosAttachEffectTypeClass*> const& types, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
	AbstractClass* pSource, std::vector<int>& durationOverrides, std::vector<int> const* delays, std::vector<int> const* initialDelays, std::vector<int> const* recreationDelays)
{
	if (types.size() < 1 || !pTarget || !pTarget->IsAlive)
		return false;

	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

	int attachedCount = 0;
	bool markForRedraw = false;
	double ROFModifier = 1.0;

	for (size_t i = 0; i < types.size(); i++)
	{
		auto const pType = types[i];
		int durationOverride = 0;
		int delay = 0;
		int initialDelay = 0;
		int recreationDelay = -1;

		if (durationOverrides.size() > 0)
			durationOverride = durationOverrides[durationOverrides.size() > i ? i : durationOverrides.size() - 1];

		if (delays && delays->size() > 0)
			delay = (*delays)[delays->size() > i ? i : delays->size() - 1];

		if (initialDelays && initialDelays->size() > 0)
			initialDelay = (*initialDelays)[initialDelays->size() > i ? i : initialDelays->size() - 1];

		if (recreationDelays && recreationDelays->size() > 0)
			recreationDelay = (*recreationDelays)[recreationDelays->size() > i ? i : recreationDelays->size() - 1];

		if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(pType, pTarget, pTargetExt->PhobosAE, pInvokerHouse, pInvoker, pSource, durationOverride, delay, initialDelay, recreationDelay))
		{
			attachedCount++;

			if (initialDelay <= 0)
			{
				if (pType->HasTint())
					markForRedraw = true;

				if (pType->Cumulative)
					PhobosAEFunctions::UpdateCumulativeAttachEffects(pTarget, pType);
			}
		}
	}


	if (attachedCount > 0)
		AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pTarget)->AeData, pTarget);

	if (markForRedraw)
		pTarget->MarkForRedraw();

	return attachedCount;
}

/// <summary>
/// Creates and attaches a single AttachEffect instance of specified type on techno.
/// </summary>
/// <param name="pType">AttachEffect type.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="targetAEs">Target's AttachEffect vector</param>
/// <param name="pInvokerHouse">House that invoked the attachment.</param>
/// <param name="pInvoker">Techno that invoked the attachment.</param>
/// <param name="pSource">Source object for the attachment e.g a Warhead or Techno.</param>
/// <param name="durationOverride">Override for AttachEffect duration.</param>
/// <param name="delay">Override for AttachEffect delay.</param>
/// <param name="initialDelay">Override for AttachEffect initial delay.</param>
/// <param name="recreationDelay">Override for AttachEffect recreation delay.</param>
/// <returns>The created and attached AttachEffect if successful, nullptr if not.</returns>
PhobosAttachEffectClass* PhobosAttachEffectClass::CreateAndAttach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, std::vector<PhobosAttachEffectClass>& targetAEs,
	HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
{
	if (!pType || !pTarget)
		return nullptr;

	if (!pType->PenetratesIronCurtain && pTarget->IsIronCurtained())
		return nullptr;

	int currentTypeCount = 0;
	PhobosAttachEffectClass* match = nullptr;
	PhobosAttachEffectClass* sourceMatch = nullptr;

	for (auto& aePtr : targetAEs)
	{
		if (aePtr.GetType() == pType)
		{
			currentTypeCount++;
			match = &aePtr;

			if (aePtr.Source == pSource && aePtr.Invoker == pInvoker)
				sourceMatch = &aePtr;
		}
	}

	if (pType->Cumulative && pType->Cumulative_MaxCount >= 0 && currentTypeCount >= pType->Cumulative_MaxCount)
	{
		if (sourceMatch)
			sourceMatch->RefreshDuration(durationOverride);

		return nullptr;
	}

	if (!pType->Cumulative && currentTypeCount > 0 && match)
		match->RefreshDuration(durationOverride);
	else
	{
		targetAEs.emplace_back();
		auto ptr = &targetAEs.back();
		ptr->Initialize(pType, pTarget, pInvokerHouse, pInvoker, pSource, durationOverride, delay, initialDelay, recreationDelay);
		return ptr;
	}

	return nullptr;
}

/// <summary>
/// Remove AttachEffects of given type from techno.
/// </summary>
/// <param name="types">AttachEffect type.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="minCount">Minimum instance count needed for cumulative type to be removed.</param>
/// <param name="maxCount">Maximum instance count of cumulative type to be removed.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int PhobosAttachEffectClass::Detach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount)
{
	if (!pType || !pTarget)
		return 0;

	int detachedCount = PhobosAttachEffectClass::RemoveAllOfType(pType, pTarget, minCount, maxCount);

	if (detachedCount > 0)
	{
		AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pTarget)->AeData, pTarget);

		if (pType->HasTint())
			pTarget->MarkForRedraw();
	}

	return detachedCount;
}

/// <summary>
/// Remove all AttachEffects matching given types from techno.
/// </summary>
/// <param name="types">List of AttachEffect types.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="minCounts">Minimum instance counts needed for cumulative types to be removed.</param>
/// <param name="maxCounts">Maximum instance counts of cumulative types to be removed.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int PhobosAttachEffectClass::Detach(std::vector<PhobosAttachEffectTypeClass*> const& types, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts)
{
	if (types.size() < 1 || !pTarget)
		return 0;

	int detachedCount = 0;
	bool markForRedraw = false;
	size_t index = 0, minSize = minCounts.size(), maxSize = maxCounts.size();

	for (auto const pType : types)
	{
		int minCount = minSize > 0 ? (index < minSize ? minCounts.at(index) : minCounts.at(minSize - 1)) : -1;
		int maxCount = maxSize > 0 ? (index < maxSize ? maxCounts.at(index) : maxCounts.at(maxSize - 1)) : -1;

		int count = PhobosAttachEffectClass::RemoveAllOfType(pType, pTarget, minCount, maxCount);

		if (count && pType->HasTint())
			markForRedraw = true;

		if (count && pType->Cumulative)
			PhobosAEFunctions::UpdateCumulativeAttachEffects(pTarget, pType);

		detachedCount += count;
		index++;
	}

	if (detachedCount > 0)
		AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pTarget)->AeData, pTarget);

	if (markForRedraw)
		pTarget->MarkForRedraw();

	return detachedCount;
}

int PhobosAttachEffectClass::Detach(std::vector<PhobosAttachEffectTypeClass*> const& types, TechnoClass* pTarget, int minCount, int maxCount, bool recalc)
{
	if (types.size() < 1 || !pTarget)
		return 0;

	int detachedCount = 0;
	bool markForRedraw = false;

	for (auto const pType : types)
	{
		int count = PhobosAttachEffectClass::RemoveAllOfTypeAndSource(pType, pTarget, pTarget, minCount, maxCount);

		if (count && pType->HasTint())
			markForRedraw = true;

		if (count && pType->Cumulative)
			PhobosAEFunctions::UpdateCumulativeAttachEffects(pTarget, pType);

		detachedCount += count;
	}

	if (recalc && detachedCount > 0)
		AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pTarget)->AeData, pTarget);

	if (markForRedraw)
		pTarget->MarkForRedraw();

	return detachedCount;
}

/// <summary>
/// Remove all AttachEffects matching given groups from techno.
/// </summary>
/// <param name="groups">List of group ID's.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="minCounts">Minimum instance counts needed for cumulative types to be removed.</param>
/// <param name="maxCounts">Maximum instance counts of cumulative types to be removed.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int PhobosAttachEffectClass::DetachByGroups(std::vector<std::string> const& groups, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts)
{
	if (groups.size() < 1 || !pTarget)
		return 0;

	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	std::vector<PhobosAttachEffectTypeClass*> types;

	for (auto const& attachEffect : pTargetExt->PhobosAE)
	{
		if (attachEffect.Type->HasGroups(groups, false))
			types.push_back(attachEffect.Type);
	}

	return Detach(types, pTarget, minCounts, maxCounts);
}

/// <summary>
/// Remove all AttachEffects of given type from a techno.
/// </summary>
/// <param name="pType">Type of AttachEffect to remove.</param>
/// <param name="targetAEs">Target techno.</param>
/// <param name="minCount">Minimum instance count needed for cumulative type to be removed.</param>
/// <param name="maxCount">Maximum instance count of cumulative type to be removed.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int PhobosAttachEffectClass::RemoveAllOfType(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount)
{
	if (!pType || !pTarget)
		return 0;

	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	int detachedCount = 0;
	int stackCount = -1;

	if (pType->Cumulative)
		stackCount = PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType);

	if (minCount > 0 && stackCount > -1 && pType->Cumulative && minCount > stackCount)
		return 0;

	auto const targetAEs = &pTargetExt->PhobosAE;

	for (auto it = targetAEs->begin(); it != targetAEs->end(); )
	{
		if (maxCount > 0 && detachedCount >= maxCount)
			break;

		if (pType == (it)->Type)
		{
			detachedCount++;

			if (pType->ExpireWeapon.isset() && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Remove) != ExpireWeaponCondition::None)
			{
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType) < 2)
					(it)->ExpireWeapon();
			}

			if ((it)->ResetIfRecreatable())
			{
				++it;
				continue;
			}

			it = targetAEs->erase(it);
		}
		else
		{
			++it;
		}
	}

	return detachedCount;
}

int PhobosAttachEffectClass::RemoveAllOfTypeAndSource(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, AbstractClass* pSource, int minCount, int maxCount)
{

	if (!pType || !pTarget || !pSource)
		return 0;

	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	int detachedCount = 0;
	int stackCount = -1;

	if (pType->Cumulative)
		stackCount = PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType);

	if (minCount > 0 && stackCount > -1 && pType->Cumulative && minCount > stackCount)
		return 0;

	auto const targetAEs = &pTargetExt->PhobosAE;

	for (auto it = targetAEs->begin(); it != targetAEs->end(); )
	{
		if (maxCount > 0 && detachedCount >= maxCount)
			break;

		if (pType == (it)->Type && pTarget == pSource)
		{
			detachedCount++;

			if (pType->ExpireWeapon.isset() && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Remove) != ExpireWeaponCondition::None)
			{
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType) < 2)
					(it)->ExpireWeapon();
			}

			if ((it)->ResetIfRecreatable())
			{
				++it;
				continue;
			}

			it = targetAEs->erase(it);
		}
		else
		{
			++it;
		}
	}

	return detachedCount;
}

/// <summary>
/// Transfer AttachEffects from one techno to another.
/// </summary>
/// <param name="pSource">Source techno.</param>
/// <param name="pTarget">Target techno.</param>
void PhobosAttachEffectClass::TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget)
{
	const auto pSourceExt = TechnoExtContainer::Instance.Find(pSource);
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

	for (auto it = pSourceExt->PhobosAE.begin(); it != pSourceExt->PhobosAE.end(); )
	{
		if ((it)->IsSelfOwned())
		{
			++it;
			continue;
		}

		auto const type = (it)->GetType();
		int currentTypeCount = 0;
		PhobosAttachEffectClass* match = nullptr;
		PhobosAttachEffectClass* sourceMatch = nullptr;

		for (auto& aePtr : pTargetExt->PhobosAE)
		{
			if (aePtr.GetType() == type)
			{
				currentTypeCount++;
				match = &aePtr;

				if (aePtr.Source == (it)->Source && aePtr.Invoker == (it)->Invoker)
					sourceMatch = &aePtr;
			}
		}

		if (type->Cumulative && type->Cumulative_MaxCount >= 0 && currentTypeCount >= type->Cumulative_MaxCount && sourceMatch)
		{
			sourceMatch->Duration = MaxImpl(sourceMatch->Duration, (it)->Duration);
		}
		else if (!type->Cumulative && currentTypeCount > 0 && match)
		{
			match->Duration = MaxImpl(sourceMatch->Duration, (it)->Duration);
		}
		else
		{
			if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(
				type,
				pTarget,
				pTargetExt->PhobosAE,
				(it)->InvokerHouse,
				(it)->Invoker,
				(it)->Source,
				(it)->DurationOverride)
				)
			{
				pAE->Duration = (it)->Duration;
			}
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
		.Process(this->Type)
		.Process(this->Techno)
		.Process(this->InvokerHouse)
		.Process(this->Invoker)
		.Process(this->Source)
		.Process(this->Animation)
		.Process(this->IsAnimHidden)
		.Process(this->IsUnderTemporal)
		.Process(this->IsOnline)
		.Process(this->IsCloaked)
		.Process(this->HasInitialized)
		.Process(this->SelectedAnim)
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

