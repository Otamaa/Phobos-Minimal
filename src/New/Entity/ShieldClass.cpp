#include "ShieldClass.h"

#include <Ext/Rules/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TEvent/Body.h>

#include <Utilities/GeneralUtils.h>

#include <AnimClass.h>
#include <HouseClass.h>
#include <RadarEventClass.h>
#include <TacticalClass.h>


ShieldClass::ShieldClass() : Techno { nullptr }
, HP { 0 }
, Timers_SelfHealing { }
, Timers_SelfHealing_Warhead { }
, Timers_Respawn { }
, Timers_Respawn_Warhead { }
, AreAnimsHidden { false }
{
}

ShieldClass::ShieldClass(TechnoClass* pTechno, bool isAttached) : Techno { pTechno }
, CurTechnoType { nullptr }
, HP { 0 }
, Timers_SelfHealing { }
, Timers_SelfHealing_Warhead { }
, Timers_Respawn { }
, Timers_Respawn_Warhead { }
, IdleAnim { nullptr }
, Cloak { false }
, Online { true }
, Temporal { false }
, Available { true }
, Attached { isAttached }
, AreAnimsHidden { false }
, SelfHealing_Warhead { 0.0 }
, SelfHealing_Rate_Warhead { -1 }
, Respawn_Warhead { 0.0 }
, Respawn_Rate_Warhead { -1 }
, LastBreakFrame { -1 }
, Type { nullptr }
{
	this->UpdateType();
	SetHP(this->Type->InitialStrength.Get(this->Type->Strength));
	this->CurTechnoType = pTechno->GetTechnoType();
}

void ShieldClass::UpdateType()
{
	this->Type = TechnoExt::ExtMap.Find(this->Techno)->CurrentShieldType;
}

template <typename T>
bool ShieldClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Techno)
		.Process(this->CurTechnoType)
		.Process(this->HP)
		.Process(this->Timers_SelfHealing)
		.Process(this->Timers_SelfHealing_Warhead)
		.Process(this->Timers_Respawn)
		.Process(this->Timers_Respawn_Warhead)
		.Process(this->IdleAnim)
		.Process(this->Cloak)
		.Process(this->Online)
		.Process(this->Temporal)
		.Process(this->Available)
		.Process(this->Attached)
		.Process(this->AreAnimsHidden)
		.Process(this->SelfHealing_Warhead)
		.Process(this->SelfHealing_Rate_Warhead)
		.Process(this->Respawn_Warhead)
		.Process(this->Respawn_Rate_Warhead)
		.Process(this->LastBreakFrame)
		.Process(this->LastTechnoHealthRatio)
		.Process(this->Type)

		.Success();
}

bool ShieldClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool ShieldClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<ShieldClass*>(this)->Serialize(Stm);
}

// Is used for DeploysInto/UndeploysInto
void ShieldClass::SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
	const auto pToExt = TechnoExt::ExtMap.Find(pTo);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTo->GetTechnoType());

	if (!pToExt || !pFromExt || !pFromExt->Shield)
		return;

	if (pTypeExt->ShieldType && pFromExt->CurrentShieldType != pTypeExt->ShieldType)
	{
		if(pToExt->Shield) {
			const auto nFromPrecentage = int(pFromExt->Shield->GetHealthRatio() * pToExt->Shield->Type->Strength);
			pToExt->Shield->SetHP((int)nFromPrecentage);

			if (pToExt->Shield->GetHP() == 0)
				pToExt->Shield->SetRespawn(pTypeExt->ShieldType->Respawn_Rate, pTypeExt->ShieldType->Respawn, pTypeExt->ShieldType->Respawn_Rate, true);

		}
	}
	else
	{
		pToExt->CurrentShieldType = pFromExt->CurrentShieldType;
		pToExt->Shield.reset(pFromExt->Shield.release());
		pToExt->Shield->KillAnim();
		pToExt->Shield->Techno = pTo;
		pToExt->Shield->CurTechnoType = pFromExt->Shield->CurTechnoType;
		pToExt->Shield->CreateAnim();
		pFromExt->Shield = nullptr;
	}
}

void ShieldClass::OnRemove() { KillAnim(); }

bool ShieldClass::TEventIsShieldBroken(ObjectClass* pAttached)
{
	if (const auto pThis = generic_cast<TechnoClass*>(pAttached))
	{
		const auto pExt = TechnoExt::ExtMap.Find(pThis);

		if (auto const pShield = pExt->GetShield())
		{
			return pShield->HP <= 0;
		}
	}

	return false;
}

void ShieldClass::OnReceiveDamage(args_ReceiveDamage* args)
{
	if (!IsActive())
		return;

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);

	if (!pWHExt || !this->HP || this->Temporal || *args->Damage == 0 ||
		this->Techno->IsIronCurtained())
	{
		return;
	}

	if(CanBePenetrated(args->WH))
		return;

	const auto pSource = args->Attacker ? args->Attacker->Owner : args->SourceHouse;
	int nDamage = 0;
	int DamageToShield = 0;
	int PassableDamageAnount = 0;
	const bool ShieldStillInfullHP = (this->Type->Strength - this->HP) == 0;

	if (pWHExt->CanTargetHouse(pSource, this->Techno) && !args->WH->Temporal)
	{
		if (*args->Damage > 0)
			nDamage = MapClass::GetTotalDamage(*args->Damage, args->WH, this->Type->Armor, args->DistanceToEpicenter);
		else
			nDamage = -MapClass::GetTotalDamage(-*args->Damage, args->WH, this->Type->Armor, args->DistanceToEpicenter);

		const bool affectsShield = pWHExt->Shield_AffectTypes.empty() || pWHExt->Shield_AffectTypes.Contains(this->Type);
		const double absorbPercent = affectsShield ? pWHExt->Shield_AbsorbPercent.Get(this->Type->AbsorbPercent) : this->Type->AbsorbPercent;
		const double passPercent = affectsShield ? pWHExt->Shield_PassPercent.Get(this->Type->PassPercent) : this->Type->PassPercent;

		DamageToShield = (int)((double)nDamage * absorbPercent);
		// passthrough damage shouldn't be affected by shield armor
		PassableDamageAnount = (int)((double)*args->Damage * passPercent);
	}

	int nDamageResult = 0;
	bool IsShielRequreFeeback = true;

	if (DamageToShield == 0)
	{
		auto nPassableDamageAnountCopy = PassableDamageAnount;
		if (Phobos::Debug_DisplayDamageNumbers && (nPassableDamageAnountCopy) != 0)
			TechnoExt::DisplayDamageNumberString(this->Techno, (nPassableDamageAnountCopy), true, args->WH);

		nDamageResult = PassableDamageAnount;
	}
	else if (DamageToShield > 0)
	{
		const int rate = this->Timers_SelfHealing_Warhead.InProgress() ? this->SelfHealing_Rate_Warhead : this->Type->SelfHealing_Rate;

		this->Timers_SelfHealing.Start(rate); // when attacked, restart the timer
		this->ResponseAttack();

		if (pWHExt->DecloakDamagedTargets)
			this->Techno->Uncloak(false);

		const auto nHPCopy = this->HP;
		int residueDamage = DamageToShield - this->HP;
		//positive residual , mean that shield cant take all the damage 
		//the shield will be broken
		if (residueDamage >= 0)
		{
			residueDamage = int((double)(residueDamage) /
			//GeneralUtils::GetWarheadVersusArmor(args->WH , this->Type->Armor)
			pWHExt->GetVerses(this->Type->Armor).Verses
			); //only absord percentage damage

			if (Phobos::Debug_DisplayDamageNumbers && (nHPCopy) != 0)
				TechnoExt::DisplayDamageNumberString(this->Techno, (nHPCopy), true, args->WH);

			this->BreakShield(pWHExt->Shield_BreakAnim.Get(nullptr), pWHExt->Shield_BreakWeapon.Get(nullptr));

			//rest of the damage will be passed to the techno
			nDamageResult = this->Type->AbsorbOverDamage ? PassableDamageAnount : residueDamage + PassableDamageAnount;
		}
		else //negative residual damage
			// that mean the damage can be sustained with the sield
		{
			auto nResidualCopy = residueDamage;
			if (Phobos::Debug_DisplayDamageNumbers && (-nResidualCopy)  != 0)
				TechnoExt::DisplayDamageNumberString(this->Techno, (-nResidualCopy), true, args->WH);

			this->WeaponNullifyAnim(pWHExt->Shield_HitAnim.Get(nullptr));
			this->HP = MaxImpl(this->HP + residueDamage, 0);

			UpdateIdleAnim();

			//absorb all the damage
			nDamageResult = 0;
		}
	}
	else if (DamageToShield < 0) //negative damage
	{
		// if the shield still in full HP 
		// heal the shield user instead
		if (ShieldStillInfullHP) {
			if (MapClass::GetTotalDamage(DamageToShield, args->WH, this->CurTechnoType->Armor, args->DistanceToEpicenter) < 0)
				IsShielRequreFeeback = !this->Type->PassthruNegativeDamage;
		} else { //otherwise we heal the shield
			if (this->Type->CanBeHealed) {
				auto nDamageCopy = DamageToShield;

				if (Phobos::Debug_DisplayDamageNumbers && DamageToShield != 0)
					TechnoExt::DisplayDamageNumberString(this->Techno, DamageToShield, true, args->WH);

				this->HP = std::clamp(this->HP + (-nDamageCopy), 0, this->Type->Strength.Get());
				this->UpdateIdleAnim();
			}
		}

		nDamageResult = 0;
	}

	//replace damage
	if (nDamageResult > 0) {
		if (auto const pTag = this->Techno->AttachedTag)
			pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::ShieldBroken, this->Techno,
				CellStruct::Empty, false, args->Attacker);//where is this? is this correct?	
	}

	if (IsShielRequreFeeback)
	{
		*args->Damage = nDamageResult;
		TechnoExt::ExtMap.Find(this->Techno)->SkipLowDamageCheck = true;
	}

}

void ShieldClass::ResponseAttack() const
{
	if (this->Techno->Owner != HouseClass::CurrentPlayer || this->Techno->GetTechnoType()->Insignificant)
		return;

	const auto pWhat = GetVtableAddr(Techno);
	if (pWhat == BuildingClass::vtable)
	{
		this->Techno->Owner->BuildingUnderAttack(static_cast<BuildingClass*>(this->Techno));
	}
	else if (pWhat == UnitClass::vtable)
	{
		const auto pUnit = static_cast<UnitClass*>(this->Techno);

		if (pUnit->Type->Harvester)
		{
			if (RadarEventClass::Create(
				RadarEventType::HarvesterAttacked,
				CellClass::Coord2Cell(pUnit->GetDestination(pUnit))))
			{
				VoxClass::Play(Eva_OreMinerUnderAttack);
			}
		}
	}
}

void ShieldClass::WeaponNullifyAnim(AnimTypeClass* pHitAnim)
{
	if (this->AreAnimsHidden)
		return;

	const auto pAnimType = pHitAnim ? pHitAnim : this->Type->HitAnim.Get(nullptr);

	if (pAnimType)
	{
		auto nCoord = this->Techno->GetCenterCoords();
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord))
		{
			AnimExt::SetAnimOwnerHouseKind(pAnim, Techno->GetOwningHouse(), nullptr, Techno);
		}
	}
}

bool ShieldClass::CanBeTargeted(WeaponTypeClass* pWeapon) const
{
	if (!pWeapon)
		return false;

	if (this->CanBePenetrated(pWeapon->Warhead))
		return true;

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
	return (std::abs( pWHExt->GetVerses(this->Type->Armor).Verses ) >= 0.001);
}

bool ShieldClass::CanBePenetrated(WarheadTypeClass* pWarhead) const
{
	if (!pWarhead)
		return false;

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	const auto affectedTypes = pWHExt->Shield_Penetrate_Types.GetElements(pWHExt->Shield_AffectTypes);

	if (!affectedTypes.empty() && !affectedTypes.contains(this->Type))
		return false;

	if (pWarhead->Psychedelic)
		return !this->Type->ImmuneToPsychedelic;

	return pWHExt->Shield_Penetrate;
}

void ShieldClass::OnTemporalUpdate(TemporalClass* pTemporal)
{
	if (!this->Temporal)
	{
		this->Temporal = true;

		const auto timer = (this->HP <= 0) ? &this->Timers_Respawn : &this->Timers_SelfHealing;
		timer->Pause();

		this->CloakCheck();

		if (this->IdleAnim)
		{
			switch (this->Type->IdleAnim_TemporalAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;

			case AttachedAnimFlag::Temporal:
				this->IdleAnim->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->IdleAnim->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->IdleAnim->Pause();
				this->IdleAnim->UnderTemporal = true;
				break;
			}
		}
	}
}

void ShieldClass::OnUpdate()
{
	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
	{
		return;
	}

	if (this->Techno->Location == CoordStruct::Empty)
		return;

	if (Is_Building(this->Techno))
	{
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(this->Techno))->LimboID != -1)
			return;
	}

	if (this->Techno->Health <= 0 || !this->Techno->IsAlive || this->Techno->IsSinking)
	{
		if (auto const pTechnoExt = TechnoExt::ExtMap.Find(this->Techno))
		{
			pTechnoExt->Shield = nullptr;
			return;
		}
	}

	if (this->ConvertCheck())
		return;

	this->UpdateType();
	this->CloakCheck();

	if (!this->Available)
		return;

	this->TemporalCheck();
	if (this->Temporal)
		return;

	this->OnlineCheck();
	this->RespawnShield();
	this->SelfHealing();

	double ratio = this->Techno->GetHealthPercentage();
	if (!this->AreAnimsHidden)
	{
		if (GeneralUtils::HasHealthRatioThresholdChanged(LastTechnoHealthRatio, ratio))
			UpdateIdleAnim();

		if (!this->Cloak && !this->Temporal && this->Online && (this->HP > 0 && this->Techno->Health > 0))
			this->CreateAnim();
	}

	if (this->Timers_Respawn_Warhead.Completed())
		this->Timers_Respawn_Warhead.Stop();

	if (this->Timers_SelfHealing_Warhead.Completed())
		this->Timers_SelfHealing_Warhead.Stop();

	this->LastTechnoHealthRatio = ratio;
}

// The animation is automatically destroyed when the associated unit receives the isCloak statute.
// Therefore, we must zero out the invalid pointer
void ShieldClass::CloakCheck()
{
	const auto cloakState = this->Techno->CloakState;
	this->Cloak = cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking;

	if (this->Cloak)
		KillAnim();
}

void ShieldClass::SetAnimationVisibility(bool visible)
{
	if (!this->AreAnimsHidden && !visible)
		this->KillAnim();

	this->AreAnimsHidden = !visible;
}

void ShieldClass::OnlineCheck()
{
	if (!this->Type->Powered)
		return;

	const auto timer = (this->HP <= 0) ? &this->Timers_Respawn : &this->Timers_SelfHealing;

	auto pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && Is_Building(this->Techno))
	{
		auto const pBuilding = static_cast<BuildingClass const*>(this->Techno);
		isActive = pBuilding->IsPowerOnline();
	}

	if (!isActive)
	{
		this->Online = false;
		timer->Pause();

		if (this->IdleAnim)
		{
			switch (this->Type->IdleAnim_OfflineAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;

			case AttachedAnimFlag::Temporal:
				this->IdleAnim->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->IdleAnim->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->IdleAnim->Pause();
				this->IdleAnim->UnderTemporal = true;
				break;
			}
		}
	}
	else
	{
		this->Online = true;
		timer->Resume();

		if (this->IdleAnim)
		{
			this->IdleAnim->UnderTemporal = false;
			this->IdleAnim->Unpause();
		}
	}
}

void ShieldClass::TemporalCheck()
{
	if (!this->Temporal)
		return;

	this->Temporal = false;

	const auto timer = (this->HP <= 0) ? &this->Timers_Respawn : &this->Timers_SelfHealing;
	timer->Resume();

	if (this->IdleAnim)
	{
		this->IdleAnim->UnderTemporal = false;
		this->IdleAnim->Unpause();
	}
}

// Is used for DeploysInto/UndeploysInto and DeploysInto/UndeploysInto
bool ShieldClass::ConvertCheck()
{
	const auto newID = this->Techno->GetTechnoType();

	if (this->CurTechnoType == newID)
		return false;

	const auto pTechnoExt = TechnoExt::ExtMap.Find(this->Techno);
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(this->Techno->GetTechnoType());
	const auto pOldType = this->Type;
	bool allowTransfer = this->Type->AllowTransfer.Get(Attached);

	// Update shield type.
	if (!allowTransfer && !pTechnoTypeExt->ShieldType->Strength)
	{
		this->KillAnim();
		pTechnoExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(DEFAULT_STR2);
		pTechnoExt->Shield = nullptr;

		return true;
	}
	else if (pTechnoTypeExt->ShieldType->Strength)
	{
		pTechnoExt->CurrentShieldType = pTechnoTypeExt->ShieldType;
	}

	const auto pNewType = pTechnoExt->CurrentShieldType;

	// Update shield properties.
	if (pNewType->Strength && this->Available)
	{
		bool isDamaged = this->Techno->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
		double healthRatio = this->GetHealthRatio();

		if (pOldType->GetIdleAnimType(isDamaged, healthRatio) != pNewType->GetIdleAnimType(isDamaged, healthRatio))
			this->KillAnim();

		this->HP = (int)round(
			(double)this->HP /
			(double)pOldType->Strength *
			(double)pNewType->Strength
		);
	}
	else
	{
		const auto timer = (this->HP <= 0) ? &this->Timers_Respawn : &this->Timers_SelfHealing;
		if (pNewType->Strength && !this->Available)
		{ // Resume this shield when became Available
			timer->Resume();
			this->Available = true;
		}
		else if (this->Available)
		{ // Pause this shield when became unAvailable
			timer->Pause();
			this->Available = false;
			this->KillAnim();
		}
	}

	this->CurTechnoType = newID;

	return false;
}

void ShieldClass::SelfHealing()
{
	const auto pType = this->Type;
	const auto timer = &this->Timers_SelfHealing;
	const auto timerWH = &this->Timers_SelfHealing_Warhead;

	if (timerWH->Completed() && timer->InProgress())
	{
		int passedTime = Unsorted::CurrentFrame - timer->StartTime;
		int timeLeft = pType->SelfHealing_Rate - passedTime;
		timer->TimeLeft = timeLeft <= 0 ? 0 : timeLeft;
	}

	const double amount = timerWH->InProgress() ? this->SelfHealing_Warhead : pType->SelfHealing;
	const int rate = timerWH->InProgress() ? this->SelfHealing_Rate_Warhead : pType->SelfHealing_Rate;
	const auto percentageAmount = this->GetPercentageAmount(amount);

	if (percentageAmount != 0)
	{
		if ((this->HP < this->Type->Strength || percentageAmount < 0) && timer->StartTime == -1)
			timer->Start(rate);

		if (this->HP > 0 && timer->Completed())
		{
			timer->Start(rate);
			this->HP += percentageAmount;

			UpdateIdleAnim();

			if (this->HP > pType->Strength)
			{
				this->HP = pType->Strength;
				timer->Stop();
			}
			else if (this->HP <= 0)
			{
				BreakShield();
			}
		}
	}
}

int ShieldClass::GetPercentageAmount(double iStatus) const
{
	if (iStatus == 0)
		return 0;

	if (iStatus >= -1.0 && iStatus <= 1.0)
		return (int)round(this->Type->Strength * iStatus);

	return (int)trunc(iStatus);
}

void ShieldClass::InvalidatePointer(void* ptr, bool bDetach)
{
	AnnounceInvalidPointer(this->IdleAnim, ptr);
}

void ShieldClass::BreakShield(AnimTypeClass* pBreakAnim, WeaponTypeClass* pBreakWeapon)
{
	this->HP = 0;

	if (this->Type->Respawn)
		this->Timers_Respawn.Start(Timers_Respawn_Warhead.InProgress() ? Respawn_Rate_Warhead : this->Type->Respawn_Rate);

	this->Timers_SelfHealing.Stop();

	this->KillAnim();

	if (!this->AreAnimsHidden)
	{
		const auto pAnimType = pBreakAnim ? pBreakAnim : this->Type->BreakAnim.Get(nullptr);

		if (pAnimType)
		{
			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location))
			{
				pAnim->SetOwnerObject(this->Techno);
				AnimExt::SetAnimOwnerHouseKind(pAnim, Techno->GetOwningHouse(), nullptr, Techno);
			}
		}
	}

	this->LastBreakFrame = Unsorted::CurrentFrame;

	if (const auto pWeaponType = pBreakWeapon ? pBreakWeapon : this->Type->BreakWeapon.Get(nullptr))
	{
		AbstractClass* const pTarget = this->Type->BreakWeapon_TargetSelf.Get() ? static_cast<AbstractClass*>(this->Techno) : this->Techno->GetCell();
		WeaponTypeExt::DetonateAt(pWeaponType, pTarget, this->Techno, true);
	}
}

void ShieldClass::RespawnShield()
{
	const auto timer = &this->Timers_Respawn;
	const auto timerWH = &this->Timers_Respawn_Warhead;

	if (this->HP <= 0 && timer->Completed())
	{
		timer->Stop();
		double amount = timerWH->InProgress() ? Respawn_Warhead : this->Type->Respawn;
		this->HP = this->GetPercentageAmount(amount);
	}
	else if (timerWH->Expired() && timer->InProgress())
	{
		int passedTime = Unsorted::CurrentFrame - timer->StartTime;
		int timeLeft = Type->Respawn_Rate - passedTime;
		timer->TimeLeft = timeLeft <= 0 ? 0 : timeLeft;
	}
}

void ShieldClass::SetRespawn(int duration, double amount, int rate, bool resetTimer)
{
	const auto timer = &this->Timers_Respawn;
	const auto timerWH = &this->Timers_Respawn_Warhead;

	this->Respawn_Warhead = amount > 0 ? amount : Type->Respawn;
	this->Respawn_Rate_Warhead = rate >= 0 ? rate : Type->Respawn_Rate;

	timerWH->Start(duration);

	if (this->HP <= 0 && Respawn_Rate_Warhead >= 0 && (resetTimer || timer->Expired()))
	{
		timer->Start(Respawn_Rate_Warhead);
	}
	else if (timer->InProgress())
	{
		int passedTime = Unsorted::CurrentFrame - timer->StartTime;
		int timeLeft = Respawn_Rate_Warhead - passedTime;
		timer->TimeLeft = timeLeft <= 0 ? 0 : timeLeft;
	}
}

void ShieldClass::SetSelfHealing(int duration, double amount, int rate, bool resetTimer)
{
	auto timer = &this->Timers_SelfHealing;
	auto timerWH = &this->Timers_SelfHealing_Warhead;

	this->SelfHealing_Warhead = amount;
	this->SelfHealing_Rate_Warhead = rate >= 0 ? rate : Type->SelfHealing_Rate;

	timerWH->Start(duration);

	if (this->HP < this->Type->Strength && (resetTimer || timer->Expired()))
	{
		timer->Start(this->SelfHealing_Rate_Warhead);
	}
	else if (timer->InProgress())
	{
		int passedTime = Unsorted::CurrentFrame - timer->StartTime;
		int timeLeft = SelfHealing_Rate_Warhead - passedTime;
		timer->TimeLeft = timeLeft <= 0 ? 0 : timeLeft;
	}
}

void ShieldClass::CreateAnim()
{
	auto idleAnimType = this->GetIdleAnimType();

	if (!this->IdleAnim && idleAnimType)
	{
		if (auto const pAnim = GameCreate<AnimClass>(idleAnimType, this->Techno->Location))
		{
			pAnim->SetOwnerObject(this->Techno);
			pAnim->Owner = this->Techno->Owner;
			pAnim->RemainingIterations = 0xFFu;
			this->IdleAnim = pAnim;
		}
	}
}

void ShieldClass::KillAnim()
{
	if (this->IdleAnim)
	{
		if (this->IdleAnim->Type) //this anim doesnt have type pointer , just detach it 
		{
			//GameDelete<true,false>(this->IdleAnim);
			this->IdleAnim->TimeToDie = true;
			this->IdleAnim->UnInit();
		}

		this->IdleAnim = nullptr;
	}
}

void ShieldClass::UpdateIdleAnim()
{
	if (this->IdleAnim && this->IdleAnim->Type != this->GetIdleAnimType())
	{
		this->KillAnim();
		this->CreateAnim();
	}
}

AnimTypeClass* ShieldClass::GetIdleAnimType() const
{
	if (!this->Type || !this->Techno)
		return nullptr;

	bool isDamaged = this->Techno->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

	return this->Type->GetIdleAnimType(isDamaged, this->GetHealthRatio());
}

void ShieldClass::DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	if (this->HP > 0 || this->Type->Respawn)
	{
		if (Is_Building(this->Techno))
			this->DrawShieldBar_Building(iLength, pLocation, pBound);
		else
			this->DrawShieldBar_Other(iLength, pLocation, pBound);
	}
}

void ShieldClass::DrawShieldBar_Building(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	CoordStruct vCoords = { 0, 0, 0 };
	this->Techno->GetTechnoType()->Dimension2(&vCoords);
	Point2D vPos2 = { 0, 0 };
	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
	TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);

	Point2D vLoc = *pLocation;
	vLoc.X -= 5;
	vLoc.Y -= 3;

	Point2D vPos = { 0, 0 };

	const int iTotal = DrawShieldBar_PipAmount(iLength);
	int frame = this->DrawShieldBar_Pip(true);

	if (iTotal > 0)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iTotal, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	if (iTotal < iLength)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iLength - iTotal, deltaX = 4 * iTotal, deltaY = -2 * iTotal;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;
			int emptyFrame = this->Type->Pips_Building_Empty.Get(RulesExt::Global()->Pips_Shield_Building_Empty.Get(0));

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				emptyFrame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
}

void ShieldClass::DrawShieldBar_Other(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	const auto pipBoard = this->Type->Pips_Background_SHP.Get(RulesExt::Global()->Pips_Shield_Background_SHP.Get(FileSystem::PIPBRD_SHP()));

	if (!pipBoard)
		return;

	Point2D vPos = { 0,0 };
	Point2D vLoc = *pLocation;
	int frame, XOffset, YOffset;
	YOffset = this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->Type->BracketDelta;
	vLoc.Y -= 5;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 11;
		vPos.Y = vLoc.Y - 25 + YOffset;
		frame = pipBoard->Frames > 2 ? 3 : 1;
		XOffset = -5;
		YOffset -= 24;
	}
	else
	{
		vPos.X = vLoc.X + 1;
		vPos.Y = vLoc.Y - 26 + YOffset;
		frame = pipBoard->Frames > 2 ? 2 : 0;
		XOffset = -15;
		YOffset -= 25;
	}

	if (this->Techno->IsSelected)
	{
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pipBoard,
			frame, &vPos, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	const int iTotal = DrawShieldBar_PipAmount(iLength);

	frame = this->DrawShieldBar_Pip(false);

	for (int i = 0; i < iTotal; ++i)
	{
		vPos.X = vLoc.X + XOffset + 2 * i;
		vPos.Y = vLoc.Y + YOffset;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

int ShieldClass::DrawShieldBar_Pip(const bool isBuilding)
{
	const auto strength = this->Type->Strength;
	const auto& pips_Shield = isBuilding ? this->Type->Pips_Building : this->Type->Pips;
	const auto& pips_Global = isBuilding ? RulesExt::Global()->Pips_Shield_Building : RulesExt::Global()->Pips_Shield;
	const auto& shieldPip = pips_Shield->X != -1 ? pips_Shield : pips_Global;

	if (this->HP > RulesClass::Instance->ConditionYellow * strength && shieldPip->X != -1)
		return shieldPip->X;
	else if (this->HP > RulesClass::Instance->ConditionRed * strength && (shieldPip->Y != -1 || shieldPip->X != -1))
		return shieldPip->Y == -1 ? shieldPip->X : shieldPip->Y;
	else if (shieldPip->Z != -1 || shieldPip->X != -1)
		return shieldPip->Z == -1 ? shieldPip->X : shieldPip->Z;

	return isBuilding ? 5 : 16;
}

int ShieldClass::DrawShieldBar_PipAmount(int iLength)
{
	return this->IsActive()
		? std::clamp((int)std::round(this->GetHealthRatio() * iLength), 1, iLength)
		: 0;
}

double ShieldClass::GetHealthRatio() const
{
	return static_cast<double>(this->HP) / this->Type->Strength;
}

int ShieldClass::GetHP() const
{
	return this->HP;
}

void ShieldClass::SetHP(int amount)
{
	this->HP = amount;
	if (this->HP > this->Type->Strength)
		this->HP = this->Type->Strength;
}

Armor ShieldClass::GetArmor() const
{
	return this->Type->Armor;
}

ShieldTypeClass* ShieldClass::GetType() const
{
	return this->Type;
}

int ShieldClass::GetFramesSinceLastBroken() const
{
	return Unsorted::CurrentFrame - this->LastBreakFrame;
}

bool ShieldClass::IsActive() const
{
	return
		this->Available &&
		this->HP > 0 &&
		this->Online;
}

bool ShieldClass::IsAvailable() const
{
	return this->Available;
}

bool ShieldClass::IsBrokenAndNonRespawning() const
{
	return this->HP <= 0 && !this->Type->Respawn;
}

bool ShieldClass::IsGreenSP()
{
	return RulesClass::Instance->ConditionYellow * Type->Strength.Get() < HP;
}

bool ShieldClass::IsYellowSP()
{
	return RulesClass::Instance->ConditionRed * Type->Strength.Get() < HP && HP <= RulesClass::Instance->ConditionYellow * Type->Strength.Get();
}

bool ShieldClass::IsRedSP()
{
	return HP <= RulesClass::Instance->ConditionRed * Type->Strength.Get();
}