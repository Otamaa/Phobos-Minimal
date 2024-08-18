#include "PhobosAttachEffectClass.h"

#include "PhobosAttachEffectTypeClass.h"
#include "Functions.h"

#include <Ext/Anim/Body.h>
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
	this->IsFirstCumulativeInstance = false;
	this->SelectedAnim = pType->Animation;

}

void PhobosAttachEffectClass::InvalidatePointer(AbstractClass* ptr, bool removed)
{
	if (!ptr)
		return;

	//if(this->Techno)
	//	Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	//else
	//	Debug::Log(__FUNCTION__" Executed Without Techno \n");

	//if (this->Animation && ptr == this->Animation)
	//	this->Animation.release();

	//if (removed && ptr == static_cast<void*>(this->Techno))
	//	this->Techno = nullptr;

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

	if (!this->Animation && !this->IsUnderTemporal && this->IsOnline && !this->IsCloaked && !this->IsInTunnel && !this->IsAnimHidden)
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

	if (!this->Animation && this->Type->Animation_TemporalAction != AttachedAnimFlag::Hides && this->IsOnline && !this->IsInTunnel && !this->IsAnimHidden)
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

			if (!this->Animation && (!this->IsUnderTemporal || this->Type->Animation_TemporalAction != AttachedAnimFlag::Hides))
				this->CreateAnim();
		}
	}
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
#include <Ext/AnimType/Body.h>

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
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (this->Type->DiscardOn_AbovePercent.isset() && this->Techno->GetHealthPercentage() >= this->Type->DiscardOn_AbovePercent.Get())
		return false;

	if (this->Type->DiscardOn_BelowPercent.isset() && this->Techno->GetHealthPercentage() <= this->Type->DiscardOn_BelowPercent.Get())
		return false;

	if (auto const pFoot = abstract_cast<FootClass*>(this->Techno))
	{
		bool isMoving = pFoot->Locomotor->Is_Moving();

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

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
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

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
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

		if (!durationOverrides.empty())
			durationOverride = durationOverrides[durationOverrides.size() > i ? i : durationOverrides.size() - 1];

		if (delays && !delays->empty())
			delay = (*delays)[delays->size() > i ? i : delays->size() - 1];

		if (initialDelays && !initialDelays->empty())
			initialDelay = (*initialDelays)[initialDelays->size() > i ? i : initialDelays->size() - 1];

		if (recreationDelays && !recreationDelays->empty())
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
PhobosAttachEffectClass* PhobosAttachEffectClass::CreateAndAttach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, HelperedVector<PhobosAttachEffectClass>& targetAEs,
	HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
{
	if (!pType || !pTarget)
		return nullptr;

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());

	if (pType->AffectAbovePercent.isset() && pTarget->GetHealthPercentage() < pType->AffectAbovePercent.Get())
		return nullptr;

	if (pType->AffectBelowPercent.isset() && pTarget->GetHealthPercentage() > pType->AffectBelowPercent.Get())
		return nullptr;

	if (pTarget->IsIronCurtained())
	{
		const bool penetrates = pTarget->ProtectType == ProtectTypes::ForceShield ? pType->PenetratesForceShield.Get(pType->PenetratesIronCurtain) : pType->PenetratesIronCurtain;

		if (!penetrates)
			return nullptr;
	}

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

	if (!pType->Cumulative && currentTypeCount > 0 && match) {
		match->Techno = pTarget;
		match->RefreshDuration(durationOverride);
	}
	else
	{
		targetAEs.emplace_back();
		targetAEs.back().Initialize(pType, pTarget, pInvokerHouse, pInvoker, pSource, durationOverride, delay, initialDelay, recreationDelay);
		return &targetAEs.back();
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

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
	const int detachedCount = PhobosAttachEffectClass::RemoveAllOfType(pType, pTarget, minCount, maxCount);

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
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
	if (types.size() < 1 || !pTarget)
		return 0;

	int detachedCount = 0;
	bool markForRedraw = false;
	size_t index = 0, minSize = minCounts.size(), maxSize = maxCounts.size();

	for (auto& pType : types)
	{
		const int minCount = minSize > 0 ? (index < minSize ? minCounts.operator[](index) : minCounts.back()) : -1;
		const int maxCount = maxSize > 0 ? (index < maxSize ? maxCounts.operator[](index) : minCounts.back()) : -1;
		const int count = PhobosAttachEffectClass::RemoveAllOfType(pType, pTarget, minCount, maxCount);

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

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
	int detachedCount = 0;
	bool markForRedraw = false;

	for (auto& pType : types)
	{
		const int count = PhobosAttachEffectClass::RemoveAllOfTypeAndSource(pType, pTarget, pTarget, minCount, maxCount);

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

	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	std::vector<PhobosAttachEffectTypeClass*> types {};

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
		if (maxCount > 0 && detachedCount >= maxCount)
			return false;

		if (pType == it.Type) {
			detachedCount++;

			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Remove) != ExpireWeaponCondition::None) {
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType) < 2)
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			if (it.ResetIfRecreatable()) {
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

		WeaponTypeExtData::DetonateAt(pWeapon, coords, _pTarget, pWeapon->Damage ,false, pOwner);
	}

	return detachedCount;
}

int PhobosAttachEffectClass::RemoveAllOfTypeAndSource(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, AbstractClass* pSource, int minCount, int maxCount)
{
	if (!pType || !pTarget || !pSource)
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
		if (maxCount > 0 && detachedCount >= maxCount)
			return false;

		if (pType == it.Type)
		{
			detachedCount++;

			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Remove) != ExpireWeaponCondition::None)
			{
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType) < 2)
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			if (it.ResetIfRecreatable())
			{
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

		WeaponTypeExtData::DetonateAt(pWeapon, coords, _pTarget, pWeapon->Damage, false, pOwner);
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
	//Debug::Log(__FUNCTION__" Executed [%s - %s]\n", pTarget->GetThisClassName(), pTarget->get_ID());
	const auto pSourceExt = TechnoExtContainer::Instance.Find(pSource);
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

	if (pSourceExt->PhobosAE.begin() == pSourceExt->PhobosAE.end())
		return;

	pSourceExt->PhobosAE.remove_if([&](auto& it) {
		if (it.IsSelfOwned())
			return false;

		auto const type = it.GetType();
		int currentTypeCount = 0;
		PhobosAttachEffectClass* match = nullptr;
		PhobosAttachEffectClass* sourceMatch = nullptr;

		for (auto& aePtr : pTargetExt->PhobosAE) {
			if (aePtr.GetType() == type) {
				currentTypeCount++;
				match = &aePtr;

				if (aePtr.Source == it.Source && aePtr.Invoker == it.Invoker)
					sourceMatch = &aePtr;
			}
		}

		if (type->Cumulative && type->Cumulative_MaxCount >= 0 && currentTypeCount >= type->Cumulative_MaxCount && sourceMatch) {
			sourceMatch->Duration = MaxImpl(sourceMatch->Duration, it.Duration);
		}
		else if (!type->Cumulative && currentTypeCount > 0 && match) {
			match->Duration = MaxImpl(sourceMatch->Duration, it.Duration);
		} else {
			if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(
				type,
				pTarget,
				pTargetExt->PhobosAE,
				it.InvokerHouse,
				it.Invoker,
				it.Source,
				it.DurationOverride)
				)
			{
				pAE->Duration = it.Duration;
			}
		}

		return true;
	});
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

