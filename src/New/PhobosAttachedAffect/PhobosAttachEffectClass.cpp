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
#include <Ext/Bullet/Body.h>
#include <Ext/Infantry/Body.h>

#include <ExtraHeaders/StackVector.h>

HelperedVector<PhobosAttachEffectClass*> PhobosAttachEffectClass::Array;

PhobosAttachEffectClass::PhobosAttachEffectClass()
{
	PhobosAttachEffectClass::Array.push_back(this);
}

PhobosAttachEffectClass::~PhobosAttachEffectClass()
{
	if (const auto& pTrail = this->LaserTrail) {

		const auto pTechnoExt = TechnoExtContainer::Instance.Find(this->Techno);
		const auto it = pTechnoExt->LaserTrails.find_if([pTrail](auto const& item) { return item.get() == pTrail; });

		if (it != pTechnoExt->LaserTrails.cend())
			pTechnoExt->LaserTrails.erase(it);

		this->LaserTrail = nullptr;
	}

	PhobosAttachEffectClass::Array.remove(this);
}

void PhobosAttachEffectClass::Initialize(PhobosAttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse,
	TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
{
	if (!pTechno->IsAlive)
		return;

	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", pTechno->GetThisClassName(), pTechno->get_ID());
	this->PeriodicWeaponTimer = pType->PeriodicWeapon_FiringDelay;
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

	if (pInvoker && pInvoker->IsAlive) {
		const int dur = static_cast<int>(TechnoExtData::ApplyDamageMult(pTechno, this->Duration, !pType->Duration_ApplyFirepowerMult));
		this->Duration = dur;
	}

	if (pType->Duration_ApplyArmorMultOnTarget && this->Duration > 0) // count its own ArmorMultiplier as well
	{
		const int dur = static_cast<int>(TechnoExtData::GetArmorMult(pTechno, this->Duration, nullptr, false));
		this->Duration = MaxImpl(dur, 0);
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

void PhobosAttachEffectClass::InvalidatePointer(AbstractClass* ptr, bool removed, AbstractType type)
{
	switch(type)
	{
	case AbstractType::House:
		AnnounceInvalidPointer(this->InvokerHouse, ptr);
		break;

	case AbstractType::Infantry:
	case AbstractType::Unit:
	case AbstractType::Aircraft:
	case AbstractType::Building:
		AnnounceInvalidPointer(this->Invoker, ptr, removed);
		break;
	default: break;
	}
}

void PhobosAttachEffectClass::InvalidateAnimPointer(AnimClass* ptr)
{
	if (this->Animation && (this->Animation.get() == ptr))
		this->Animation.release();
}

void PhobosAttachEffectClass::HandleEvent(TechnoClass* pTarget)
{
	if (const auto pTag = pTarget->AttachedTag)
		pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::AttachedIsUnderAttachedEffect, pTarget, CellStruct::Empty);
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

	auto pType = this->Type;

	if (!this->HasInitialized)
	{
		this->HasInitialized = true;
		auto const pExt = TechnoExtContainer::Instance.Find(this->Techno);
		auto const pTechno = this->Techno;

		if (pType->ROFMultiplier != 1.0 && pType->ROFMultiplier > 0.0 && pType->ROFMultiplier_ApplyOnCurrentTimer)
		{
			double ROFModifier = pType->ROFMultiplier;

			pTechno->RearmTimer.Start(static_cast<int>(pTechno->RearmTimer.GetTimeLeft() * ROFModifier));

			if (!pExt->ChargeTurretTimer.HasStarted() && pExt->LastRearmWasFullDelay)
				pTechno->ROF = static_cast<int>(pTechno->ROF * ROFModifier);
		}

		bool decloak = false;
		AEProperties::RecalculateSingle(this->Techno, this, &decloak, nullptr, true);

		if(decloak)
			this->Techno->Uncloak(true);

		HandleEvent(pTechno);
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

			bool decloak = false;
			AEProperties::RecalculateSingle(this->Techno, this, &decloak, nullptr, true);

			if(decloak)
				this->Techno->Uncloak(true);

			this->NeedsDurationRefresh = false;
			HandleEvent(this->Techno);
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

		if (this->Delay > 0) {
			this->KillAnim();
			this->NeedsRecalculateStat = true;
		}
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
	this->AnimCheck();

	// --- Periodic Weapon Logic ---
	if (pType->PeriodicWeapon && pType->PeriodicWeapon_FiringDelay > 0 && pType->PeriodicWeapon_Range.Get() > 0) {

		this->PeriodicWeaponTimer--;

		if (this->PeriodicWeaponTimer <= 0) {
			this->PeriodicWeaponTimer = pType->PeriodicWeapon_FiringDelay;
			this->FirePeriodicWeapon();
		}
	}
}

void PhobosAttachEffectClass::AI_Temporal()
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", this->Techno->GetThisClassName(), this->Techno->get_ID());

	if (!this->IsUnderTemporal)
	{
		this->IsUnderTemporal = true;

		this->CloakCheck();
		this->AnimCheck();

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

void PhobosAttachEffectClass::FirePeriodicWeapon()
{
	auto const pType = this->Type;
	auto const pWeapon = pType->PeriodicWeapon;

	if (!pWeapon)
		return;

	auto const pTechno = this->Techno;

	if (!pTechno || pTechno->InLimbo || pTechno->IsImmobilized)
		return;

	// 确定开火者
	TechnoClass* pFirer = pType->PeriodicWeapon_UseInvokerAsOwner ? this->Invoker : pTechno;

	if (!pFirer)
		pFirer = pTechno;

	if (!pFirer->IsAlive)
		return;

	HouseClass* pFirerHouse = pType->PeriodicWeapon_UseInvokerAsOwner
		? (this->InvokerHouse ? this->InvokerHouse : pTechno->Owner)
		: pTechno->Owner;

	if (!pFirerHouse)
		pFirerHouse = pTechno->Owner;

	auto const pWH = pWeapon->Warhead;
	const int searchRange = pType->PeriodicWeapon_Range.Get();  // ← 只定义一次，int类型
	const auto firePos = pTechno->Location;

	// === 索敌 ===
	std::vector<TechnoClass*> validTargets;

	for (auto const pTarget : *TechnoClass::Array)
	{
		if (!pTarget || pTarget == pTechno || pTarget->InLimbo)
			continue;

		// 1) 距离检查
		const int dist = pTarget->DistanceFrom(pTechno);
		if (dist > searchRange)
			continue;

		// 2) AffectsHouse 检查
		if (!EnumFunctions::CanTargetHouse(pType->PeriodicWeapon_AffectsHouse, pFirerHouse, pTarget->Owner))
			continue;

		// 3) AffectTypes / IgnoreTypes 检查
		auto const pTargetType = pTarget->GetTechnoType();

		if (pType->PeriodicWeapon_AffectTypes.size() > 0)
		{
			bool found = false;
			for (auto const& pAllowed : pType->PeriodicWeapon_AffectTypes)
			{
				if (pTargetType == pAllowed) { found = true; break; }
			}
			if (!found) continue;
		}

		if (pType->PeriodicWeapon_IgnoreTypes.size() > 0)
		{
			bool ignored = false;
			for (auto const& pIgnored : pType->PeriodicWeapon_IgnoreTypes)
			{
				if (pTargetType == pIgnored) { ignored = true; break; }
			}
			if (ignored) continue;
		}

		// 4) 弹头护甲比率检查
		auto armor = TechnoExtData::GetTechnoArmor(pTarget, pWH);
		const double versus = GeneralUtils::GetWarheadVersusArmor(pWH, armor);
		if (versus < 0.001)
			continue;

		// 5) 存活检查
		if (!pTarget->IsAlive || pTarget->Health <= 0)
			continue;

		validTargets.push_back(pTarget);
	}

	if (validTargets.empty())
		return;

	// === 开火（内联，不走 LaunchPeriodicBullet）===
	if (pType->PeriodicWeapon_FireAll)
	{
		for (auto const pTarget : validTargets)
		{
			auto const pBulletType = pWeapon->Projectile;
			if (!pBulletType) continue;

			BulletClass* pBullet = pBulletType->CreateBullet(
				pTarget, pFirer, pWeapon->Damage, pWeapon->Warhead,
				static_cast<int>(pWeapon->Speed), pWeapon->Bright);

			if (!pBullet) continue;

			pBullet->Owner = pFirer;
			auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);
			pBulletExt->Owner = pFirerHouse;
			BulletExtData::SimulatedFiringUnlimbo(pBullet, pFirerHouse, pWeapon, firePos, false, {});
			BulletExtData::SimulatedFiringEffects(pBullet, pFirerHouse, pFirer, true, true);
		}
	}
	else
	{
		TechnoClass* pClosest = nullptr;
		double closestDistSq = DBL_MAX;

		for (auto const pTarget : validTargets)
		{
			const int d = pTarget->DistanceFrom(pTechno);
			if (d < closestDistSq)
			{
				closestDistSq = d;
				pClosest = pTarget;
			}
		}

		if (pClosest)
		{
			auto const pBulletType = pWeapon->Projectile;
			if (pBulletType)
			{
				BulletClass* pBullet = pBulletType->CreateBullet(
					pClosest, pFirer, pWeapon->Damage, pWeapon->Warhead,
					pWeapon->Speed, pWeapon->Bright);

				if (pBullet)
				{
					pBullet->Owner = pFirer;
					auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);
					pBulletExt->Owner = pFirerHouse;
					BulletExtData::SimulatedFiringUnlimbo(pBullet, pFirerHouse, pWeapon, firePos, false, {});
					BulletExtData::SimulatedFiringEffects(pBullet, pFirerHouse, pFirer, true, true);
				}
			}
		}
	}
}

void PhobosAttachEffectClass::AnimCheck()
{
	if (!this->Type->Animation_HideIfAttachedWith.empty())
	{
		if (PhobosAEFunctions::HasAttachedEffects(this->Techno, this->Type->Animation_HideIfAttachedWith, false, false, nullptr, nullptr, nullptr, nullptr))
		{
			this->KillAnim();
			this->IsAnimHidden = true;
			return;
		}
	}

	if (this->Animation && this->Type->Animation_DrawOffsets.size() > 0)
	{
		auto const pAnimExt = AnimExtContainer::Instance.Find(this->Animation);
		//auto const pTechnoExt = TechnoExtContainer::Instance.Find(this->Techno);
		pAnimExt->AEDrawOffset = Point2D::Empty;

		for (auto& drawOffset : this->Type->Animation_DrawOffsets)
		{
			if (drawOffset.RequiredTypes.size() < 1 || PhobosAEFunctions::HasAttachedEffects(this->Techno, drawOffset.RequiredTypes, false, false, nullptr, nullptr, nullptr, nullptr))
				pAnimExt->AEDrawOffset += drawOffset.Offset;
		}
	}

	this->IsAnimHidden = false;

	if (!this->Animation && this->CanShowAnim())
		this->CreateAnim();
}

void PhobosAttachEffectClass::DiscardOnFire()
{ 
	if (GeneralUtils::Contains<DiscardCondition>(
		this->GetType()->DiscardOn,
		DiscardCondition::Firing))
		this->ShouldBeDiscarded = true;
}

void PhobosAttachEffectClass::UpdateCumulativeAnim(int count)
{
	if (!this->Animation)
		return;

	if (count < 1) {
		this->KillAnim();
		return;
	}

	auto const pAnimType = this->Type->GetCumulativeAnimation(count);
	if (this->Animation->Type != pAnimType)
		AnimExtData::ChangeAnimType(this->Animation, pAnimType, false, this->Type->CumulativeAnimations_RestartOnChange);
}

bool PhobosAttachEffectClass::CanShowAnim() const
{
	const auto pType = this->Type;

	return pType->Animation || (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
		&& (this->IsOnline || pType->Animation_OfflineAction != AttachedAnimFlag::Hides)
		&& (!this->IsUnderTemporal || pType->Animation_TemporalAction != AttachedAnimFlag::Hides)
		&& !this->IsAnimHidden && !this->IsInTunnel;
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

	if (isActive != this->LastActiveStat) {
		bool decloak = false;
		AEProperties::RecalculateSingle(this->Techno, this, &decloak, nullptr, true);

		if(decloak)
			this->Techno->Uncloak(true);

		this->LastActiveStat = isActive;
	}

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
	auto const pType = this->Type;
	AnimTypeClass* pAnimType = nullptr;

	if (pType->Cumulative && !this->Type->CumulativeAnimations.empty())
	{
		if (!this->HasCumulativeAnim)
			return;

		const int count = PhobosAEFunctions::GetAttachedEffectCumulativeCount(this->Techno, this->Type);
		//Debug::LogInfo("AE[%s] cumulativeAnim [%d] from size[%d] ", this->Type->Name.data(), count , this->Type->CumulativeAnimations.size());
		pAnimType = this->Type->GetCumulativeAnimation(count);
	}
	else
	{
		pAnimType = this->SelectedAnim;
	}

	if(!pAnimType)
		return;

	if (this->IsCloaked && AnimTypeExtContainer::Instance.Find(pAnimType)->DetachOnCloak)
		return;

	//force set the anim , if it already has anim the Handle<T> will kill the previous anim
	this->Animation.reset(GameCreate<AnimClass>(pAnimType, this->Techno->Location));
	this->Animation->SetOwnerObject(this->Techno);
	this->Animation->Owner = this->Type->Animation_UseInvokerAsOwner ? InvokerHouse : this->Techno->Owner;
	this->Animation->RemainingIterations = 0xFFu;
	auto pAnimExt = ((FakeAnimClass*)this->Animation.get())->_GetExtData();
	if (this->Type->Animation_UseInvokerAsOwner) {
		pAnimExt->Invoker = Invoker;
	}
}

void PhobosAttachEffectClass::KillAnim()
{
	if (this->Animation) {
		if (!Phobos::Otamaa::ExeTerminated)
			this->Animation.reset();
		else
			this->Animation.detachptr();
	}

}

void PhobosAttachEffectClass::SetAnimationTunnelState(bool visible)
{
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

	if (this->Invoker && this->Invoker->IsAlive) {
		const int dur = static_cast<int>(TechnoExtData::ApplyDamageMult(this->Invoker, this->Duration, !this->Type->Duration_ApplyFirepowerMult));
		this->Duration = dur;
	}

	// count its own ArmorMultiplier as well
	if (this->Type->Duration_ApplyArmorMultOnTarget && this->Duration > 0) {
		const int dur = static_cast<int>(TechnoExtData::GetArmorMult(this->Techno, this->Duration, nullptr, false));
		this->Duration = MaxImpl(dur, 0);
	}

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
	this->NeedsDurationRefresh = true;

	return true;
}

bool PhobosAttachEffectClass::ShouldBeDiscardedNow()
{
	if (this->LastDiscardCheckFrame == Unsorted::CurrentFrame())
		return this->LastDiscardCheckValue;

	this->LastDiscardCheckFrame = Unsorted::CurrentFrame();

	const auto pType = this->Type;
	const auto pTechno = this->Techno;
	const auto discardOn = pType->DiscardOn;

	// caches the answer for this frame and returns it — single exit shape
	const auto finish = [this](bool value)
		{
			this->LastDiscardCheckValue = value;
			return value;
		};

	const auto has = [discardOn](DiscardCondition cond)
		{
			return (discardOn & cond) != DiscardCondition::None;
		};

	if (this->ShouldBeDiscarded)
		return finish(true);

	// ------------------------------------------------------------------
	// health ratio gates — independent of the DiscardOn flag set
	// ------------------------------------------------------------------
	{
		if (pType->DiscardOn_AbovePercent.isset()
			&& pTechno->GetHealthRatio() >= pType->DiscardOn_AbovePercent.Fetch())
		{
			return finish(true);
		}

		if (pType->DiscardOn_BelowPercent.isset()
			&& pTechno->GetHealthRatio() <= pType->DiscardOn_BelowPercent.Fetch())
		{
			return finish(true);
		}

		// everything below is flag driven — bail out early instead of indenting
		if (discardOn == DiscardCondition::None)
			return finish(false);
	}

	// ------------------------------------------------------------------
	// movement / harvesting / stationary
	// ------------------------------------------------------------------
	if (const auto pFoot = flag_cast_to<FootClass*, false>(pTechno)) {
		const bool isMoving = pType->DiscardOn_MoveBasedOnDestination.Get(
				FakeRulesClass::Instance()->DiscardOn_ConsiderHoverAsMoving)
			? pFoot->Locomotor->Is_Moving()
			: pFoot->Locomotor->Is_Really_Moving_Now();

		// BUG: when the unit *is* moving but DiscardOn lacks `Move`, the original
		// fell into the else-branch and still evaluated the stationary/harvesting
		// chain below. Preserved verbatim (flattening keeps this identical).
		if (isMoving && has(DiscardCondition::Move))
			return finish(true);

		if (pType->DiscardOn_ConsiderHarvestingAsStationary.Get(
			FakeRulesClass::Instance()->DiscardOn_ConsiderHarvestingAsStationary)) {
			// harvesting counts as standing still — no separate probe needed
			if (has(DiscardCondition::Stationary))
				return finish(true);
		} else {
			bool isHarvestingNow = false;

			if (const auto pUnit = cast_to<UnitClass*>(pFoot))
				isHarvestingNow = pUnit->IsHarvesting;
			else if (const auto pInf = cast_to<InfantryClass*>(pFoot))
				isHarvestingNow = (pInf->SequenceAnim == DoType::Shovel);

			if (isHarvestingNow && has(DiscardCondition::Harvesting))
				return finish(true);

			// intermediate state: about to start harvesting, but the checks above
			// do not report it yet — treat as "not discardable" rather than
			// letting the Stationary test fire.
			// BUGFIX: original dereferenced GetCell() unconditionally.
			const auto pFootCell = pFoot->GetCell();

			if (pFoot->CurrentMission == Mission::Harvest
				&& pFootCell && pFootCell->LandType == LandType::Tiberium) {
				return finish(false);
			}

			if (has(DiscardCondition::Stationary))
				return finish(true);
		}
	}

	// ------------------------------------------------------------------
	// drain
	// ------------------------------------------------------------------
	if (has(DiscardCondition::Drain) && pTechno->DrainingMe)
		return finish(true);

	// ------------------------------------------------------------------
	// ammo window — both bounds optional, -1 means "unbounded on that side"
	// ------------------------------------------------------------------
	if (has(DiscardCondition::Ammo)) {
		const auto& nMin = pType->DiscardOn_Ammo_Min;
		const auto& nMax = pType->DiscardOn_Ammo_Max;
		const int ammo = pTechno->Ammo;

		if ((nMin < 0 || ammo >= nMin) && (nMax < 0 || ammo <= nMax)) {
			this->LastDiscardCheckValue = true;
			return true;
		}
	}

	// ------------------------------------------------------------------
	// health window
	// ------------------------------------------------------------------
	// DIFF: dropped the `GetTechnoType()` guard — the result was never used and
	// the pointer is non-null for any live techno.
	if (has(DiscardCondition::Health)) {
		const auto& nMin = pType->DiscardOn_Health_Min;
		const auto& nMax = pType->DiscardOn_Health_Max;

		const bool hasMin = nMin.isset();
		const bool hasMax = nMax.isset();

		if (hasMin || hasMax) {
			const double hp = pTechno->GetHealthPercentage();

			// SUSPECT: exclusive lower bound above zero, inclusive at zero.
			// Preserved verbatim.
			// DIFF: an unset bound is now "no bound" instead of the literal
			// 0.0 / 1.0 clamp. Identical for hp in [0,1]; only differs if
			// GetHealthPercentage() ever returns > 1.0 (over-heal), where the
			// old 1.0 default would have silently failed the max test.
			const bool minOk = !hasMin || (hp > 0.0 ? hp > nMin.Fetch() : hp >= nMin.Fetch());
			const bool maxOk = !hasMax || hp <= nMax.Fetch();

			if (minOk && maxOk)
				return finish(true);
		}
	}

	// ------------------------------------------------------------------
	// land type
	// ------------------------------------------------------------------
	if (has(DiscardCondition::LandType) && pType->DiscardOn_LandTypes != LandTypeFlags::None) {
		if (const auto pCell = pTechno->GetCell()) {
			if (IsLandTypeInFlags(pType->DiscardOn_LandTypes, pCell->LandType))
				return finish(true);
		}
	}

	// ------------------------------------------------------------------
	// Mission
	// ------------------------------------------------------------------
	if (has(DiscardCondition::Mission)) {
		auto const& missions = pTechno->Owner->IsControlledByHuman()

			? pType->DiscardOn_Missions
			: (pType->DiscardOn_AIMissions.HasValue()
				? static_cast<ValueableVector<Mission>&>(pType->DiscardOn_AIMissions)
				: pType->DiscardOn_Missions);

		if (missions.size() > 0 && missions.Contains(pTechno->CurrentMission)) {
			return finish(true);
		}
	}

	// ------------------------------------------------------------------
	// Sequence
	// ------------------------------------------------------------------
	if (has(DiscardCondition::Sequence)) {
		if (auto const pInf = cast_to<InfantryClass*, false>(pTechno)) {
			if (pType->DiscardOn_Sequences.size() > 0) {
				if (pType->DiscardOn_Sequences_Immediate.Get(FakeRulesClass::Instance->DiscardOn_Sequences_Immediate)) {
					if (pType->DiscardOn_Sequences.Contains(pInf->SequenceAnim)) {
						return finish(true);
					}
				} else {
					if (this->LastSequenceCheck != pInf->SequenceAnim &&
						pType->DiscardOn_Sequences.Contains(this->LastSequenceCheck)) {
						return finish(true);
					}

					this->LastSequenceCheck = pInf->SequenceAnim;
				}
			}
		}
		else {
			this->LastSequenceCheck = DoType::None;
		}
	}

	// ------------------------------------------------------------------
	// range to current target
	// ------------------------------------------------------------------
	if (pTechno->Target) {
		const bool inRange = has(DiscardCondition::InRange);
		const bool outOfRange = has(DiscardCondition::OutOfRange);

		if (inRange || outOfRange) {
			// SUSPECT: if neither the override nor a weapon resolves, distance stays
			// -1, which makes the OutOfRange test (dist >= -1) always fire.
			// Preserved verbatim.
			int distance = -1;

			if (pType->DiscardOn_RangeOverride.isset()) {
				distance = pType->DiscardOn_RangeOverride.Fetch();
			} else {
				const int weaponIndex = pTechno->SelectWeapon(pTechno->Target);

				// BUGFIX: original dereferenced GetWeapon() without a null check.
				if (const auto pWeaponStruct = pTechno->GetWeapon(weaponIndex)) {
					if (const auto pWeapon = pWeaponStruct->WeaponType)
						distance = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pTechno);
				}
			}

			const int distanceFromTgt = pTechno->DistanceFrom(pTechno->Target);

			if ((inRange && distanceFromTgt <= distance)
				|| (outOfRange && distanceFromTgt >= distance)) {
				return finish(true);
			}
		}
	}

	// ------------------------------------------------------------------
	// selling / undeploying
	// ------------------------------------------------------------------
	if (const auto pBuilding = cast_to<BuildingClass*, true>(pTechno)) {
		if (pBuilding->CurrentMission == Mission::Selling) {
			const auto condition = pBuilding->ArchiveTarget
				? DiscardCondition::Undeploying
				: DiscardCondition::Selling;

			if (has(condition))
				return finish(true);
		}
	}

	return finish(false);
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
	bool decloak = false;
	double ROFModifier = 1.0;
	bool selfOwned = pTarget == pSource;
	std::set<PhobosAttachEffectTypeClass*> cumulativeAnimTypes;

	for (size_t i = 0; i < types.size(); i++)
	{
		auto const pType = types[i];
		auto const params = attachEffectInfo->GetAttachParams(i, selfOwned);

		if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(pType, pTarget, pTargetExt->PhobosAE, pInvokerHouse, pInvoker, pSource, params))
		{
			attachedCount++;

			if (params.InitialDelay <= 0)
			{
				AEProperties::RecalculateSingle(pTarget, pAE, &decloak, &markForRedraw, false);

				if (pType->ROFMultiplier > 0.0 && pType->ROFMultiplier_ApplyOnCurrentTimer)
					ROFModifier *= pType->ROFMultiplier;

				if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
					cumulativeAnimTypes.insert(pType);
			}
		}
	}

	if (ROFModifier != 1.0)
	{
		if (!pTargetExt->ChargeTurretTimer.HasStarted() && pTargetExt->LastRearmWasFullDelay)
			pTarget->ROF = static_cast<int>(pTarget->ROF * ROFModifier);
	}

	if (attachedCount > 0){

		if (markForRedraw) {
			pTarget->MarkForRedraw();
			pTargetExt->Tints.Update();
		}

		if (decloak && pTarget->CloakState == CloakState::Cloaked)
			pTarget->Uncloak(true);
	}

	for (auto const cumType : cumulativeAnimTypes) {
		PhobosAEFunctions::UpdateCumulativeAttachEffects(pTarget, cumType , false);
	}

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

	const auto pTargetTechnoType = GET_TECHNOTYPE(pTarget);

	if (pTarget->IsIronCurtained()) {
		const bool penetrates = pTarget->ProtectType == ProtectTypes::ForceShield
			? pType->PenetratesForceShield.Get(pType->PenetratesIronCurtain) : pType->PenetratesIronCurtain;

		if (!penetrates)
			return nullptr;
	}

	if (!pType->AffectTypes.empty() && !pType->AffectTypes.Contains(pTargetTechnoType) || pType->IgnoreTypes.Contains(pTargetTechnoType))
		return nullptr;

	if (!EnumFunctions::IsTechnoEligible(pTarget, pType->AffectTargets, true))
		return nullptr;

	if (pType->AffectAbovePercent.isset() && pTarget->GetHealthRatio() < pType->AffectAbovePercent.Fetch())
		return nullptr;

	if (pType->AffectBelowPercent.isset() && pTarget->GetHealthRatio() > pType->AffectBelowPercent.Fetch())
		return nullptr;

	int currentTypeCount = 0;
	int currentSourceCount = 0;
	const bool cumulative = pType->Cumulative && checkCumulative;
	PhobosAttachEffectClass* match = nullptr;
	StackVector<PhobosAttachEffectClass* , 256> cumulativeMatches;

	for (auto const& aePtr : targetAEs)
	{
		auto const attachEffect = aePtr.get();

		if (attachEffect->GetType() == pType)
		{
			currentTypeCount++;

			if (!cumulative) {
				attachEffect->RefreshDuration(attachParams.DurationOverride);

				if (auto pTag = pTarget->AttachedTag)
					pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::AttachedIsUnderAttachedEffect, pTarget, CellStruct::Empty);

				return nullptr;
			} else
			{
				if (attachEffect->IsFromSource(pInvoker, pSource))
					currentSourceCount++;

				if (!attachParams.CumulativeRefreshSameSourceOnly || attachEffect->IsFromSource(pInvoker, pSource)) {
					cumulativeMatches->push_back(attachEffect);

					if (!match || attachEffect->Duration < match->Duration)
						match = attachEffect;
				}
			}
		}
	}

	if (cumulative)
	{
		if ((pType->Cumulative_MaxCount >= 0 && currentTypeCount >= pType->Cumulative_MaxCount)
			|| (attachParams.CumulativeSourceMaxCount >= 0 && currentSourceCount >= attachParams.CumulativeSourceMaxCount))
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
	std::set<PhobosAttachEffectTypeClass*> cumulativeAnimTypes;

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
				cumulativeAnimTypes.insert(pType);

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

	for (auto const Cumtype : cumulativeAnimTypes){
		PhobosAEFunctions::UpdateCumulativeAttachEffects(pTarget, Cumtype, true);
	}

	PhobosAttachEffectClass::DetonateExpireWeapon(expireWeapons, pTarget->Location);

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

void PhobosAttachEffectClass::DetonateExpireWeapon(std::vector<std::pair<WeaponTypeClass*, TechnoClass*>>& expireContainer, CoordStruct& designation)
{
	for (auto& [pWeapon, pTarget] : expireContainer) {
		WeaponTypeExtData::DetonateAt5(pWeapon, designation, pTarget, pTarget, pWeapon->Damage, false, pTarget->Owner);
	}
}

void PhobosAttachEffectClass::TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget)
{
	//Debug::LogInfo(__FUNCTION__" Executed [%s - %s]", pTarget->GetThisClassName(), pTarget->get_ID());
	const auto pSourceExt = TechnoExtContainer::Instance.Find(pSource);
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
	bool markForRedraw = false;
	int transferCount = 0;

	for (auto it = pSourceExt->PhobosAE.begin(); it != pSourceExt->PhobosAE.end(); )
	{
		auto const attachEffect = it->get();
		if (!attachEffect)
		{
			it = pSourceExt->PhobosAE.erase(it);
			continue;
		}

		if (attachEffect->IsSelfOwned())
		{
			++it;
			continue;
		}

		auto const type = attachEffect->GetType();
		auto pTargetType = GET_TECHNOTYPE(pTarget);

		const bool isValid = EnumFunctions::IsTechnoEligible(pTarget, type->AffectTargets, true)
			&& (type->AffectTypes.empty() || type->AffectTypes.Contains(pTargetType))
			&& !type->IgnoreTypes.Contains(pTargetType);

		if (!isValid)
		{
			it = pSourceExt->PhobosAE.erase(it);
			continue;
		}


		int currentTypeCount = 0;
		PhobosAttachEffectClass* match = nullptr;
		PhobosAttachEffectClass* sourceMatch = nullptr;
		const bool cumulative = type->Cumulative;

		for (auto const& aePtr : pTargetExt->PhobosAE)
		{
			if (!aePtr)
				continue;

			auto const targetAttachEffect = aePtr.get();

			if (targetAttachEffect->GetType() == type)
			{
				currentTypeCount++;
				if (!cumulative)
				{
					match = targetAttachEffect;
					break;
				}
				else if (targetAttachEffect->IsFromSource(attachEffect->Invoker, attachEffect->Source))
				{
					if (!match || targetAttachEffect->Duration < match->Duration)
						sourceMatch = targetAttachEffect;
				}
			}
		}

		if (match)
		{
			if (!cumulative || (type->Cumulative_MaxCount >= 0 && currentTypeCount >= type->Cumulative_MaxCount))
				match->Duration = MaxImpl(match->Duration, attachEffect->Duration);

		}
		else
		{
			AEAttachParams info {};
			info.DurationOverride = attachEffect->DurationOverride;

			if (auto const pAE = PhobosAttachEffectClass::CreateAndAttach(type, pTarget, pTargetExt->PhobosAE, attachEffect->InvokerHouse, attachEffect->Invoker, attachEffect->Source, info, false))
				pAE->Duration = attachEffect->Duration;
		}

		if (type->HasTint())
			markForRedraw = true;

		transferCount++;
		it = pSourceExt->PhobosAE.erase(it);
	}

	if (transferCount > 0) {

		AEProperties::Recalculate(pTarget);

		if (markForRedraw)
			pTarget->MarkForRedraw();
	}
}

#pragma endregion

// =============================
// load / save

template <typename T>
bool PhobosAttachEffectClass::Serialize(T& Stm)
{
	return Stm
	.Process(Duration)
	.Process(DurationOverride)
	.Process(Delay)
	.Process(CurrentDelay)
	.Process(InitialDelay)
	.Process(RecreationDelay)
	.Process(LastDiscardCheckFrame)
	.Process(PeriodicWeaponTimer)
	.Process(Type)
	.Process(Techno)
	.Process(InvokerHouse)
	.Process(Invoker)
	.Process(Source)
	.Process(SelectedAnim)
	.Process(LaserTrail)
	.Process(Animation)
	.Process(IsAnimHidden)
	.Process(IsInTunnel)
	.Process(IsUnderTemporal)
	.Process(IsOnline)
	.Process(IsCloaked)
	.Process(HasInitialized)
	.Process(NeedsDurationRefresh)
	.Process(LastDiscardCheckValue)

	.Process(LastActiveStat)
	.Process(NeedsRecalculateStat)
	.Process(ShouldBeDiscarded)
	.Process(HasCumulativeAnim)

	.Process(LastSequenceCheck)

	.Success() && Stm.RegisterChange(this)
		;
}

bool PhobosAttachEffectClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool PhobosAttachEffectClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<PhobosAttachEffectClass*>(this)->Serialize(Stm);
}

