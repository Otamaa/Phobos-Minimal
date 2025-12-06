#include "PhobosAttachEffectClass.h"

#include "PhobosAttachEffectTypeClass.h"
#include "Functions.h"
#include "AEAttachParams.h"
#include "AEAttachInfoTypeClass.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TEvent/Body.h>

PhobosAttachEffectClass::~PhobosAttachEffectClass()
{
	if (const auto& pTrail = this->LaserTrail) {

		const auto pTechnoExt = TechnoExtContainer::Instance.Find(this->Techno);
		const auto it = pTechnoExt->LaserTrails.find_if([pTrail](auto const& item) { return item.get() == pTrail; });

		if (it != pTechnoExt->LaserTrails.cend())
			pTechnoExt->LaserTrails.erase(it);

		this->LaserTrail = nullptr;
	}

}

void PhobosAttachEffectClass::Initialize(PhobosAttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse,
	TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", pTechno->GetThisClassName(), pTechno->get_ID());
	this->Duration = durationOverride != 0 ? durationOverride : pType->Duration;
	this->DurationOverride = durationOverride;
	this->Delay = delay;
	this->CurrentDelay = 0;
	this->InitialDelay = initialDelay;
	this->RecreationDelay = recreationDelay;
	this->Type = pType;
	this->Techno = pTechno;

	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pTechno);

	if (auto pWH = pType->Duration_ApplyVersus_Warhead)
	{
		const auto armor = TechnoExtData::GetTechnoArmor(pTechno, pType->Duration_ApplyVersus_Warhead);
		const auto verses = WarheadTypeExtContainer::Instance.Find(pWH)->GetVerses(armor);
		this->Duration = MaxImpl(static_cast<int>(this->Duration * verses.Verses), 0);
	}

	if (pInvoker) {
		auto pInvokerExt = TechnoExtContainer::Instance.Find(pInvoker);

		if(pType->Duration_ApplyFirepowerMult)
			this->Duration = static_cast<int>(this->Duration * pInvoker->FirepowerMultiplier * pInvokerExt->AE.FirepowerMultiplier);
	}

	if (pType->Duration_ApplyArmorMultOnTarget && this->Duration > 0) // count its own ArmorMultiplier as well
	{
		const auto _value = this->Duration / pTechno->ArmorMultiplier / pTechnoExt->AE.ArmorMultiplier / this->Type->ArmorMultiplier;
		this->Duration = MaxImpl(static_cast<int>(_value), 0);
	}

	this->InvokerHouse = pInvokerHouse;
	this->Invoker = pInvoker;
	this->Source = pSource;
	this->IsAnimHidden = false;
	this->IsUnderTemporal = false;
	this->IsOnline = true;
	this->IsCloaked = false;

	if (this->InitialDelay <= 0) {
		this->HasInitialized = true;
		if (auto pTag = pTechno->AttachedTag)
			pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::AttachedIsUnderAttachedEffect, pTechno, CellStruct::Empty);

	}

	this->HasInitialized = (initialDelay <= 0);
	this->NeedsDurationRefresh = false;
	this->HasCumulativeAnim = false;
	this->SelectedAnim = pType->Animation;

	const int laserTrailIdx = pType->LaserTrail_Type;

	if (laserTrailIdx != -1) {
		pTechnoExt->LaserTrails.emplace_back(
			std::move(std::make_unique<LaserTrailClass>(LaserTrailTypeClass::Array[laserTrailIdx].get(), pTechno->Owner->LaserColor)));
		this->LaserTrail = pTechnoExt->LaserTrails.back().get();
	}

}

void PhobosAttachEffectClass::InvalidatePointer(AbstractClass* ptr, bool removed)
{
	AnnounceInvalidPointer(this->Invoker, ptr, removed);
	AnnounceInvalidPointer(this->InvokerHouse, ptr);
}

void PhobosAttachEffectClass::InvalidateAnimPointer(AnimClass* ptr)
{
	if (this->Animation && (this->Animation.get() == ptr))
		this->Animation.release();
}

// =============================
// actual logic

void PhobosAttachEffectClass::AI()
{
	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
		return;

	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());

	if (this->InitialDelay > 0)
	{
		this->InitialDelay--;
		return;
	}

	if (!this->HasInitialized)
	{
		this->HasInitialized = true;
		auto const pExt = TechnoExtContainer::Instance.Find(this->Techno);
		auto const pTechno = this->Techno;

		if (this->Type->ROFMultiplier != 1.0 && this->Type->ROFMultiplier > 0.0 && this->Type->ROFMultiplier_ApplyOnCurrentTimer)
		{
			double ROFModifier = this->Type->ROFMultiplier;

			pTechno->RearmTimer.Start(static_cast<int>(pTechno->RearmTimer.GetTimeLeft() * ROFModifier));

			if (!pExt->ChargeTurretTimer.HasStarted() && pExt->LastRearmWasFullDelay)
				pTechno->ROF = static_cast<int>(pTechno->ROF * ROFModifier);
		}

		if (this->Type->HasTint())
			pTechno->MarkForRedraw();
	}

	if (this->CurrentDelay > 0)
	{
		if (!this->ShouldBeDiscardedNow()) {
			this->CurrentDelay--;

			if (this->CurrentDelay == 0)
				this->NeedsDurationRefresh = true;
		}

		return;
	}

	if (this->NeedsDurationRefresh)
	{
		if (!this->ShouldBeDiscardedNow())
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
		else if (this->ShouldBeDiscardedNow())
			this->RefreshDuration();
		else
			this->NeedsDurationRefresh = true;

		if (auto pTag = this->Techno->AttachedTag)
			pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::AttachedIsUnderAttachedEffect, this->Techno, CellStruct::Empty);

		return;
	}

	if (this->IsUnderTemporal)
		this->IsUnderTemporal = false;

	this->CloakCheck();
	this->OnlineCheck();

	if (!this->Animation && this->CanShowAnim())
		this->CreateAnim();

	this->UpdateAnimLogic();

	if (auto pTag = this->Techno->AttachedTag)
		pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::AttachedIsUnderAttachedEffect, this->Techno, CellStruct::Empty);
}

void PhobosAttachEffectClass::AI_Temporal()
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());

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

			this->UpdateAnimLogic();
		}
	}
}

void PhobosAttachEffectClass::UpdateAnimLogic()
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());

	if (!this->Type->Animation_HideIfAttachedWith.empty())
	{
		//auto const pTechnoExt = TechnoExtContainer::Instance.Find(this->Techno);

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

	if (this->Animation && this->Type->Animation_DrawOffsets.size() > 0) {
		auto const pAnimExt = AnimExtContainer::Instance.Find(this->Animation);
		//auto const pTechnoExt = TechnoExtContainer::Instance.Find(this->Techno);
		pAnimExt->AEDrawOffset = Point2D::Empty;

		for (auto& drawOffset : this->Type->Animation_DrawOffsets) {
			if (drawOffset.RequiredTypes.size() < 1 || PhobosAEFunctions::HasAttachedEffects(this->Techno, drawOffset.RequiredTypes, false, false, nullptr, nullptr, nullptr, nullptr, true))
				pAnimExt->AEDrawOffset += drawOffset.Offset;
		}
	}
}

void PhobosAttachEffectClass::UpdateCumulativeAnim(int count)
{
	if (!this->HasCumulativeAnim || !this->Animation)
		return;

	if (count < 1) {
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
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());

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
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());
	const auto cloakState = this->Techno->CloakState;

	this->IsCloaked = cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking;

	if (this->IsCloaked && this->Animation && AnimTypeExtContainer::Instance.Find(this->Animation->Type)->DetachOnCloak)
		this->KillAnim();
}

void PhobosAttachEffectClass::CreateAnim()
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (!this->Type)
		return;

	AnimTypeClass* pAnimType = nullptr;

	if (this->Type->Cumulative && this->Type->CumulativeAnimations.HasValue())
	{
		if (!this->HasCumulativeAnim || this->Type->CumulativeAnimations.empty())
			return;

		const int count = PhobosAEFunctions::GetAttachedEffectCumulativeCount(this->Techno, this->Type);
		//Debug::LogInfo("AE[%s] cumulativeAnim [%d] from size[%d] ", this->Type->Name.data(), count , this->Type->CumulativeAnimations.size());
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
		auto pAnimExt = ((FakeAnimClass*)this->Animation.get())->_GetExtData();
		if (this->Type->Animation_UseInvokerAsOwner) {
			pAnimExt->Invoker = Invoker;
		}

		AEProperties::UpdateAEAnimLogic(this->Techno);
	}
}

void PhobosAttachEffectClass::KillAnim()
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (this->Animation) {
		this->Animation.detachptr();
		AEProperties::UpdateAEAnimLogic(this->Techno);
	}

}

void PhobosAttachEffectClass::SetAnimationTunnelState(bool visible)
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (!this->IsInTunnel && !visible)
		this->KillAnim();

	this->IsInTunnel = !visible;
}

void PhobosAttachEffectClass::RefreshDuration(int durationOverride)
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (durationOverride)
		this->Duration = durationOverride;
	else
		this->Duration = this->DurationOverride ? this->DurationOverride : this->Type->Duration;


	if (auto pWH = this->Type->Duration_ApplyVersus_Warhead)
	{
		const auto armor = TechnoExtData::GetTechnoArmor(this->Techno, this->Type->Duration_ApplyVersus_Warhead);
		const auto verses = WarheadTypeExtContainer::Instance.Find(pWH)->GetVerses(armor);
		this->Duration = MaxImpl(static_cast<int>(this->Duration * verses.Verses), 0);
	}

	if (this->Invoker)
	{
		auto pInvokerExt = TechnoExtContainer::Instance.Find(this->Invoker);

		if (this->Type->Duration_ApplyFirepowerMult)
			this->Duration = static_cast<int>(this->Duration * this->Invoker->FirepowerMultiplier * pInvokerExt->AE.FirepowerMultiplier);
	}

	if (this->Type->Duration_ApplyArmorMultOnTarget && this->Duration > 0) // count its own ArmorMultiplier as well
		this->Duration = MaxImpl(static_cast<int>(this->Duration / this->Techno->ArmorMultiplier / TechnoExtContainer::Instance.Find(this->Techno)->AE.ArmorMultiplier / this->Type->ArmorMultiplier), 0);

	if (this->Type->Animation_ResetOnReapply)
	{
		this->KillAnim();
		if (this->CanShowAnim())
			this->CreateAnim();
	}
}

bool PhobosAttachEffectClass::ResetIfRecreatable()
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (!this->IsSelfOwned() || this->RecreationDelay < 0)
		return false;

	this->KillAnim();
	this->Duration = 0;
	this->CurrentDelay = this->RecreationDelay;

	return true;
}

bool _retTrue(bool& check) {
	check = true;
	return true;
}
bool PhobosAttachEffectClass::ShouldBeDiscardedNow()
{
	if (this->LastDiscardCheckFrame == Unsorted::CurrentFrame)
		return this->LastDiscardCheckValue;

	this->LastDiscardCheckFrame = Unsorted::CurrentFrame;

	if (this->ShouldBeDiscarded)
		return _retTrue(this->LastDiscardCheckValue);

	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());
	if (this->Type->DiscardOn_AbovePercent.isset() && this->Techno->GetHealthPercentage() >= this->Type->DiscardOn_AbovePercent.Get())
		return _retTrue(this->LastDiscardCheckValue);

	if (this->Type->DiscardOn_BelowPercent.isset() && this->Techno->GetHealthPercentage() <= this->Type->DiscardOn_BelowPercent.Get())
		return _retTrue(this->LastDiscardCheckValue);

	if(this->Type->DiscardOn != DiscardCondition::None){

		if (auto const pFoot = flag_cast_to<FootClass*, false>(this->Techno))
		{
			bool isMoving = pFoot->Locomotor->Is_Really_Moving_Now();

			if (isMoving && (this->Type->DiscardOn & DiscardCondition::Move) != DiscardCondition::None)
				return _retTrue(this->LastDiscardCheckValue);

			if (!isMoving && (this->Type->DiscardOn & DiscardCondition::Stationary) != DiscardCondition::None)
				return _retTrue(this->LastDiscardCheckValue);
		}

		if (this->Techno->DrainingMe && (this->Type->DiscardOn & DiscardCondition::Drain) != DiscardCondition::None)
			return _retTrue(this->LastDiscardCheckValue);

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
						distance = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, this->Techno);
				}

				const int distanceFromTgt = this->Techno->DistanceFrom(this->Techno->Target);

				if ((inRange && distanceFromTgt <= distance) || (outOfRange && distanceFromTgt >= distance))
					return _retTrue(this->LastDiscardCheckValue);
			}
		}

	}

	this->LastDiscardCheckValue = false;
	return false;
}

#pragma region StaticFunctions_AttachDetachTransfer

int PhobosAttachEffectClass::Attach(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
	AbstractClass* pSource, AEAttachInfoTypeClass* attachEffectInfo)
{
	auto const& types = attachEffectInfo->AttachTypes;

	if (types.size() < 1 || !pTarget)
		return 0;

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

	if (ROFModifier != 1.0)
	{
		if (!pTargetExt->ChargeTurretTimer.HasStarted() && pTargetExt->LastRearmWasFullDelay)
			pTarget->ROF = static_cast<int>(pTarget->ROF * ROFModifier);
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

#include <ExtraHeaders/StackVector.h>

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

PhobosAttachEffectClass* PhobosAttachEffectClass::CreateAndAttach(
	PhobosAttachEffectTypeClass* pType,
	TechnoClass* pTarget,
	HelperedVector<std::unique_ptr<PhobosAttachEffectClass>>& targetAEs,
	HouseClass* pInvokerHouse,
	TechnoClass* pInvoker,
	AbstractClass* pSource,
	AEAttachParams const& attachParams,
	bool checkCumulative)
{
	if (!pType || !pTarget)
		return nullptr;

	const auto pTargetTechnoType = pTarget->GetTechnoType();

	if (!pType->AffectTypes.empty() && !pType->AffectTypes.Contains(pTargetTechnoType))
		return nullptr;

	if (pType->IgnoreTypes.Contains(pTargetTechnoType))
		return nullptr;

	if (!EnumFunctions::IsTechnoEligible(pTarget, pType->AffectTargets, true))
		return nullptr;

	if (pType->AffectAbovePercent.isset() && pTarget->GetHealthPercentage() < pType->AffectAbovePercent)
		return nullptr;

	if (pType->AffectBelowPercent.isset() && pTarget->GetHealthPercentage() > pType->AffectBelowPercent)
		return nullptr;

	if (pTarget->IsIronCurtained()) {
		const bool penetrates = pTarget->ProtectType == ProtectTypes::ForceShield
			? pType->PenetratesForceShield.Get(pType->PenetratesIronCurtain) : pType->PenetratesIronCurtain;

		if (!penetrates)
			return nullptr;
	}

	int currentTypeCount = 0;
	const bool cumulative = pType->Cumulative && checkCumulative;
	PhobosAttachEffectClass* match = nullptr;
	StackVector<PhobosAttachEffectClass* , 256> cumulativeMatches;

	for (auto const& aePtr : targetAEs)
	{
		auto const attachEffect = aePtr.get();

		if (attachEffect->GetType() == pType)
		{
			currentTypeCount++;
			match = attachEffect;
			if(!cumulative)
				break;

			if (!attachParams.CumulativeRefreshSameSourceOnly || (attachEffect->Source == pSource && attachEffect->Invoker == pInvoker))
				cumulativeMatches->push_back(attachEffect);

			if (!match || attachEffect->Duration < match->Duration)
				attachEffect->RefreshDuration(attachParams.DurationOverride);

			if (auto pTag = pTarget->AttachedTag)
				pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::AttachedIsUnderAttachedEffect, pTarget, CellStruct::Empty);

			return nullptr;
		}
	}

	if (cumulative)
	{
		if (pType->Cumulative_MaxCount >= 0 && currentTypeCount >= pType->Cumulative_MaxCount)
		{
			if (attachParams.CumulativeRefreshAll)
			{
				for (auto const& ae : cumulativeMatches.container())
				{
					ae->RefreshDuration(attachParams.DurationOverride);
				}
			}
			else if(match) {
				match->RefreshDuration(attachParams.DurationOverride);
			}

			if (auto pTag = pTarget->AttachedTag)
				pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::AttachedIsUnderAttachedEffect, pTarget, CellStruct::Empty);

			return nullptr;
		}
		else if (attachParams.CumulativeRefreshAll && attachParams.CumulativeRefreshAll_OnAttach)
		{
			for (auto const& ae : cumulativeMatches.container())
			{
				ae->RefreshDuration(attachParams.DurationOverride);
			}
		}
	}

	targetAEs.emplace_back((std::make_unique<PhobosAttachEffectClass>()));
	auto const pAE = targetAEs.back().get();
	pAE->Initialize(pType, pTarget, pInvokerHouse, pInvoker, pSource, attachParams.DurationOverride, attachParams.Delay, attachParams.InitialDelay, attachParams.RecreationDelay);
	if (!currentTypeCount && cumulative && pType->CumulativeAnimations.size() > 0)
		pAE->HasCumulativeAnim = true;

	AEProperties::UpdateAEAnimLogic(pTarget);
	return pAE;
}

int PhobosAttachEffectClass::DetachTypes(TechnoClass* pTarget, AEAttachInfoTypeClass* attachEffectInfo, std::vector<PhobosAttachEffectTypeClass*> const& types)
{
	//auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
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

	if (detachedCount > 0) {
		AEProperties::Recalculate(pTarget);
		AEProperties::UpdateAEAnimLogic(pTarget);
	}

	if (markForRedraw)
		pTarget->MarkForRedraw();

	return detachedCount;
}

int PhobosAttachEffectClass::RemoveAllOfType(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount)
{
	if (!pType || !pTarget)
		return 0;

	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", pTarget->GetThisClassName(), pTarget->get_ID());
	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	int detachedCount = 0;
	int stackCount = -1;

	if (pType->Cumulative)
		stackCount = PhobosAEFunctions::GetAttachedEffectCumulativeCount(pTarget, pType);

	if (minCount > 0 && stackCount > -1 && pType->Cumulative && minCount > stackCount)
		return 0;

	if (pTargetExt->PhobosAE.begin() == pTargetExt->PhobosAE.end())
		return 0;

	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons {};

	for (auto it = pTargetExt->PhobosAE.begin(); it != pTargetExt->PhobosAE.end(); )
	{
		if (maxCount > 0 && detachedCount >= maxCount)
			break;

		auto const attachEffect = it->get();

		if (pType == attachEffect->Type)
		{
			detachedCount++;

			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Remove) != ExpireWeaponCondition::None) {
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || stackCount == 1) {
					PhobosAttachEffectClass::CumulateExpireWeapon(pType, pTarget, it->get()->Invoker , expireWeapons);
				}
			}

			if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
				PhobosAEFunctions::UpdateCumulativeAttachEffects(pTarget, pType, nullptr);

			if (attachEffect->ResetIfRecreatable()) {
				++it;
				continue;
			}

			it = pTargetExt->PhobosAE.erase(it);

			if (!pType->Cumulative)
				break;

			stackCount--;
		}
		else
		{
			++it;
		}
	}

	PhobosAttachEffectClass::DetonateExpireWeapon(expireWeapons);

	return detachedCount;
}

void PhobosAttachEffectClass::CumulateExpireWeapon(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, TechnoClass* pInvoker, std::vector<std::pair<WeaponTypeClass*, TechnoClass*>>& expireContainer)
{
	if (pType->ExpireWeapon_UseInvokerAsOwner && pInvoker)
	{
		expireContainer.emplace_back(pType->ExpireWeapon, pInvoker);
	}
	else
	{
		expireContainer.emplace_back(pType->ExpireWeapon, pTarget);
	}
}

void PhobosAttachEffectClass::DetonateExpireWeapon(std::vector<std::pair<WeaponTypeClass*, TechnoClass*>>& expireContainer)
{
	for (auto const& [pWeapon, pTarget] : expireContainer) {
		WeaponTypeExtData::DetonateAt5(pWeapon, pTarget->GetCoords(), pTarget, pTarget, pWeapon->Damage, false, pTarget->Owner);
	}
}

void PhobosAttachEffectClass::TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget)
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", pTarget->GetThisClassName(), pTarget->get_ID());
	const auto pSourceExt = TechnoExtContainer::Instance.Find(pSource);
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	int transferCount = 0;

	for (auto it = pSourceExt->PhobosAE.begin(); it != pSourceExt->PhobosAE.end(); )
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

		const bool isValid = EnumFunctions::IsTechnoEligible(pTarget, type->AffectTargets, true)
		&& (type->AffectTypes.empty() || type->AffectTypes.Contains(pTarget->GetTechnoType()))
		&& !type->IgnoreTypes.Contains(pTarget->GetTechnoType());

		if (!isValid) {
			it = pSourceExt->PhobosAE.erase(it);
			continue;
		}


		int currentTypeCount = 0;
		PhobosAttachEffectClass* match = nullptr;
		PhobosAttachEffectClass* sourceMatch = nullptr;
		const bool cumulative = type->Cumulative;

		for (auto const& aePtr : pTargetExt->PhobosAE)
		{
			if(!aePtr)
				continue;

			auto const targetAttachEffect = aePtr.get();

			if (targetAttachEffect->GetType() == type)
			{
				currentTypeCount++;
				if (!cumulative) {
					match = targetAttachEffect;
					break;
				} else if (targetAttachEffect->Source == attachEffect->Source && targetAttachEffect->Invoker == attachEffect->Invoker) {
					if (!match || targetAttachEffect->Duration < match->Duration)
						sourceMatch = targetAttachEffect;
				}
			}
		}

		if (match)
		{
			if (!cumulative || (type->Cumulative_MaxCount >= 0 && currentTypeCount >= type->Cumulative_MaxCount))
				match->Duration = MaxImpl(match->Duration, attachEffect->Duration);

		} else {
			AEAttachParams info {};
			info.DurationOverride = attachEffect->DurationOverride;

			if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(type, pTarget, pTargetExt->PhobosAE, attachEffect->InvokerHouse, attachEffect->Invoker, attachEffect->Source, info , false))
				pAE->Duration = attachEffect->Duration;
		}

		transferCount++;
		it = pSourceExt->PhobosAE.erase(it);
	}

	if (transferCount) {
		AEProperties::UpdateAEAnimLogic(pSource);
		AEProperties::UpdateAEAnimLogic(pTarget);
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
		.Process(this->IsInTunnel)
		.Process(this->IsUnderTemporal)
		.Process(this->IsOnline)
		.Process(this->IsCloaked)
		.Process(this->HasInitialized)
		.Process(this->SelectedAnim)
		.Process(this->NeedsDurationRefresh)
		.Process(this->HasCumulativeAnim)
		.Process(this->ShouldBeDiscarded)
		.Process(this->LastDiscardCheckFrame)
		.Process(this->LastDiscardCheckValue)
		.Process(this->LaserTrail)
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

