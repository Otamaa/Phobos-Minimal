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

#include <Misc/Ares/Hooks/Header.h>
#include <New/Entity/FlyingStrings.h>

#include <AnimClass.h>
#include <HouseClass.h>
#include <RadarEventClass.h>
#include <TacticalClass.h>

HelperedVector<ShieldClass*> ShieldClass::Array;

ShieldClass::ShieldClass() : Techno { nullptr }
, HP { 0 }
, Timers { }
, IdleAnim { nullptr }
, Cloak { false }
, Online { true }
, Temporal { false }
, Available { true }
, Attached { false }
, AreAnimsHidden { false }
, SelfHealing_Warhead { 0.0 }
, SelfHealing_Rate_Warhead { -1 }
, Respawn_Warhead { 0.0 }
, Respawn_Rate_Warhead { -1 }
, LastBreakFrame { -1 }
, Type { nullptr }
{
	Array.push_back(this);
}

ShieldClass::ShieldClass(TechnoClass* pTechno, bool isAttached) : Techno { pTechno }
, CurTechnoType { nullptr }
, HP { 0 }
, Timers { }
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
	this->SetHP(this->Type->InitialStrength.Get(this->Type->Strength));
	this->CurTechnoType = pTechno->GetTechnoType();

	Array.push_back(this);
}

void ShieldClass::UpdateType()
{
	this->Type = TechnoExtContainer::Instance.Find(this->Techno)->CurrentShieldType;
}

template <typename T>
bool ShieldClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Techno, true)
		.Process(this->CurTechnoType, true)
		.Process(this->HP)
		.Process(this->Timers)
		.Process(this->IdleAnim, true)
		.Process(this->Cloak)
		.Process(this->Online)
		.Process(this->Temporal)
		.Process(this->Available)
		.Process(this->Attached)
		.Process(this->AreAnimsHidden)
		.Process(this->SelfHealing_Warhead)
		.Process(this->SelfHealing_Rate_Warhead)
		.Process(this->SelfHealing_RestartInCombat_Warhead)
		.Process(this->SelfHealing_RestartInCombatDelay_Warhead)
		.Process(this->Respawn_Warhead)
		.Process(this->Respawn_Rate_Warhead)
		.Process(this->LastBreakFrame)
		.Process(this->LastTechnoHealthRatio)
		.Process(this->Type, true)

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

Armor ShieldClass::GetOrInheritArmor() const
{
	const auto pShieldType = this->Type;

	if (pShieldType->InheritArmorFromTechno)
	{
		const auto pTechnoType = this->Techno->GetTechnoType();

		if (pShieldType->InheritArmor_Allowed.empty() || pShieldType->InheritArmor_Allowed.Contains(pTechnoType)
			&& (pShieldType->InheritArmor_Disallowed.empty() || !pShieldType->InheritArmor_Disallowed.Contains(pTechnoType)))
			return TechnoExtData::GetArmor(this->Techno);
	}

	return pShieldType->Armor.Get();
}

// Is used for DeploysInto/UndeploysInto
void ShieldClass::SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExtContainer::Instance.Find(pFrom);
	const auto pToExt = TechnoExtContainer::Instance.Find(pTo);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTo->GetTechnoType());

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
		pToExt->Shield->CreateAnim();
	}

	if (pFrom->WhatAmI() == AbstractType::Building && pFromExt->Shield)
		pFromExt->Shield = nullptr;
}

void ShieldClass::OnRemove() { KillAnim(); }

bool ShieldClass::TEventIsShieldBroken(ObjectClass* pAttached)
{
	if (const auto pThis = flag_cast_to<TechnoClass*>(pAttached))
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pThis);

		if (auto const pShield = pExt->GetShield())
		{
			return pShield->HP <= 0;
		}
	}

	return false;
}

int ShieldClass::OnReceiveDamage(args_ReceiveDamage* args)
{
	if (!this->IsActive())
		return *args->Damage;

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(args->WH);

	if (!pWHExt || !this->HP || this->Temporal || *args->Damage == 0)
	{
		return *args->Damage;
	}

	if (*args->Damage < 0) {
		if (auto const pFoot = flag_cast_to<FootClass*, false>(this->Techno)) {
			if (auto const pParasite = pFoot->ParasiteEatingMe) {
					// Remove parasite.
					pParasite->ParasiteImUsing->SuppressionTimer.Start(50);
					pParasite->ParasiteImUsing->ExitUnit();
			}
		}
	}

	const bool IC = pWHExt->CanAffectInvulnerable(this->Techno);

	if(!IC || CanBePenetrated(args->WH) || TechnoExtData::IsTypeImmune(this->Techno, args->Attacker))
		return *args->Damage;

	const auto pSource = args->Attacker ? args->Attacker->Owner : args->SourceHouse;
	int nDamage = 0;
	int DamageToShield = 0;
	int DamageToShieldAfterMinMax = 0;
	int PassableDamageAnount = 0;
	const bool ShieldStillInfullHP = (this->Type->Strength - this->HP) == 0;

	if (pWHExt->CanTargetHouse(pSource, this->Techno) && !args->WH->Temporal)
	{
		if (*args->Damage > 0)
			nDamage = FakeWarheadTypeClass::ModifyDamage(*args->Damage, args->WH, this->Type->Armor, args->DistanceToEpicenter);
		else
			nDamage = -FakeWarheadTypeClass::ModifyDamage(-*args->Damage, args->WH, this->Type->Armor, args->DistanceToEpicenter);

		const bool affectsShield = pWHExt->Shield_AffectTypes.empty() || pWHExt->Shield_AffectTypes.Contains(this->Type);
		const double absorbPercent = affectsShield ? pWHExt->Shield_AbsorbPercent.Get(this->Type->AbsorbPercent) : this->Type->AbsorbPercent;
		const double passPercent = affectsShield ? pWHExt->Shield_PassPercent.Get(this->Type->PassPercent) : this->Type->PassPercent;
		int ShieldDamage = (int)((double)nDamage * absorbPercent);
		DamageToShield = ShieldDamage;
		// passthrough damage shouldn't be affected by shield armor
		PassableDamageAnount = (int)((double)*args->Damage * passPercent);
	}

	int nDamageResult = 0;
	bool IsShielRequreFeeback = true;
	DamageToShieldAfterMinMax = std::clamp(DamageToShield,
		int(pWHExt->Shield_ReceivedDamage_Minimum.Get(this->Type->ReceivedDamage_Minimum) * pWHExt->Shield_ReceivedDamage_MinMultiplier),
		int(pWHExt->Shield_ReceivedDamage_Maximum.Get(this->Type->ReceivedDamage_Maximum) * pWHExt->Shield_ReceivedDamage_MaxMultiplier));

	if (DamageToShieldAfterMinMax == 0)
	{
		auto nPassableDamageAnountCopy = PassableDamageAnount;
		if (Phobos::Debug_DisplayDamageNumbers && (nPassableDamageAnountCopy) != 0)
			FlyingStrings::DisplayDamageNumberString(nPassableDamageAnountCopy, DamageDisplayType::Shield, this->Techno->GetRenderCoords(), TechnoExtContainer::Instance.Find(this->Techno)->DamageNumberOffset);

		nDamageResult = PassableDamageAnount;
	}
	else if (DamageToShieldAfterMinMax > 0)
	{
		bool whModifiersApplied = this->Timers.SelfHealing_Warhead.InProgress();
		bool restart = whModifiersApplied ? this->SelfHealing_RestartInCombat_Warhead : this->Type->SelfHealing_RestartInCombat;

		if (restart)
		{
			int delay = whModifiersApplied ? this->SelfHealing_RestartInCombatDelay_Warhead : this->Type->SelfHealing_RestartInCombatDelay;

			if (delay > 0)
			{
				this->Timers.SelfHealing_CombatRestart.Start(this->Type->SelfHealing_RestartInCombatDelay);
				this->Timers.SelfHealing.Stop();
			}
			else
			{
				const int rate = whModifiersApplied ? this->SelfHealing_Rate_Warhead : this->Type->SelfHealing_Rate;
				this->Timers.SelfHealing.Start(rate); // when attacked, restart the timer
			}
		}

		if(pWHExt->Malicious && !pWHExt->Nonprovocative)
			this->ResponseAttack();

		if (pWHExt->DecloakDamagedTargets)
			this->Techno->Uncloak(false);

		const auto nHPCopy = this->HP;
		int residueDamage = DamageToShield - this->HP;

		//positive residual , mean that shield cant take all the damage
		//the shield will be broken
		if (residueDamage >= 0)
		{
			residueDamage = MaxImpl(0 ,int((double)(residueDamage) /
			//GeneralUtils::GetWarheadVersusArmor(args->WH , this->Type->Armor)
			pWHExt->GetVerses(this->Type->Armor).Verses)
			); //only absord percentage damage

			if (Phobos::Debug_DisplayDamageNumbers && (nHPCopy) != 0)
				FlyingStrings::DisplayDamageNumberString(nHPCopy, DamageDisplayType::Shield, this->Techno->GetRenderCoords(), TechnoExtContainer::Instance.Find(this->Techno)->DamageNumberOffset);

			this->BreakShield(pWHExt->Shield_BreakAnim, pWHExt->Shield_BreakWeapon.Get(nullptr));

			//rest of the damage will be passed to the techno
			nDamageResult = this->Type->AbsorbOverDamage ? PassableDamageAnount : residueDamage + PassableDamageAnount;
		}
		else //negative residual damage
			// that mean the damage can be sustained with the sield
		{
			if (Phobos::Debug_DisplayDamageNumbers && (-DamageToShield) != 0)
				FlyingStrings::DisplayDamageNumberString((-DamageToShield), DamageDisplayType::Shield, this->Techno->GetRenderCoords(), TechnoExtContainer::Instance.Find(this->Techno)->DamageNumberOffset);

			if (this->Type->HitFlash && pWHExt->Shield_HitFlash)
			{
				int size = this->Type->HitFlash_FixedSize.Get((DamageToShield * 2));
				SpotlightFlags flags = SpotlightFlags::NoColor;

				if (this->Type->HitFlash_Black)
				{
					flags = SpotlightFlags::NoColor;
				}
				else
				{
					if (!this->Type->HitFlash_Red)
						flags = SpotlightFlags::NoRed;
					if (!this->Type->HitFlash_Green)
						flags |= SpotlightFlags::NoGreen;
					if (!this->Type->HitFlash_Blue)
						flags |= SpotlightFlags::NoBlue;
				}

				MapClass::FlashbangWarheadAt(size, args->WH, this->Techno->Location, true, flags);
			}

			if(!pWHExt->Shield_SkipHitAnim)
				this->WeaponNullifyAnim(pWHExt->Shield_HitAnim);

			this->HP -= DamageToShield; //set the HP remaining after get hit
			UpdateIdleAnim();
			//absorb all the damage
			nDamageResult = 0;
		}
	}
	else if (DamageToShieldAfterMinMax < 0) //negative damage
	{
		// if the shield still in full HP
		// heal the shield user instead
		if (ShieldStillInfullHP) {
			if (FakeWarheadTypeClass::ModifyDamage(DamageToShieldAfterMinMax,
				 args->WH,
				 TechnoExtData::GetArmor(this->Techno),
				 args->DistanceToEpicenter) < 0
				 )
				IsShielRequreFeeback = !this->Type->PassthruNegativeDamage;
		} else { //otherwise we heal the shield
			if (this->Type->CanBeHealed) {
				auto nDamageCopy = DamageToShieldAfterMinMax;

				if (Phobos::Debug_DisplayDamageNumbers && DamageToShieldAfterMinMax != 0)
					FlyingStrings::DisplayDamageNumberString(DamageToShieldAfterMinMax, DamageDisplayType::Shield, this->Techno->GetRenderCoords(), TechnoExtContainer::Instance.Find(this->Techno)->DamageNumberOffset);

				this->HP = std::clamp(this->HP + (-nDamageCopy), 0, this->Type->Strength.Get());
				this->UpdateIdleAnim();
			}
		}

		nDamageResult = 0;
	}

	//replace damage
	if (!nDamageResult && this->HP == 0) {
		if (auto const pTag = this->Techno->AttachedTag)
			pTag->RaiseEvent((TriggerEvent)PhobosTriggerEvent::ShieldBroken, this->Techno,
				CellStruct::Empty, false, args->Attacker);//where is this? is this correct?
	}

	if (IsShielRequreFeeback)
	{
		*args->Damage = nDamageResult;
		TechnoExtContainer::Instance.Find(this->Techno)->SkipLowDamageCheck = true;
	}

	return nDamageResult;
}

void ShieldClass::ResponseAttack() const
{
	if (this->Techno->Owner != HouseClass::CurrentPlayer || this->Techno->GetTechnoType()->Insignificant)
		return;

	const auto pWhat = Techno->WhatAmI();
	if (pWhat == BuildingClass::AbsID)
	{
		this->Techno->Owner->BuildingUnderAttack(static_cast<BuildingClass*>(this->Techno));
	}
	else if (pWhat == UnitClass::AbsID)
	{
		const auto pUnit = static_cast<UnitClass*>(this->Techno);

		if (pUnit->Type->Harvester)
		{
			if (RadarEventClass::Create(
				RadarEventType::HarvesterAttacked,
				CellClass::Coord2Cell(pUnit->GetDestination(pUnit))))
			{
				VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack());
			}
		}
	}
}

void ShieldClass::WeaponNullifyAnim(AnimTypeClass* pHitAnim)
{
	if (this->AreAnimsHidden)
		return;

	const auto pAnimType = pHitAnim ? pHitAnim : this->Type->HitAnim;

	if (pAnimType)
	{
		auto nCoord = this->Techno->GetCenterCoords();
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, nCoord),
			Techno->GetOwningHouse(),
			nullptr,
			Techno
		);
	}
}

bool ShieldClass::CanBeTargeted(WeaponTypeClass* pWeapon) const
{
	if (!pWeapon)
		return false;

	if (this->CanBePenetrated(pWeapon->Warhead))
		return true;

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
	return (Math::abs( pWHExt->GetVerses(this->Type->Armor).Verses ) >= 0.001);
}

bool ShieldClass::CanBePenetrated(WarheadTypeClass* pWarhead) const
{
	if (!pWarhead)
		return false;

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
	const auto affectedTypes = pWHExt->Shield_Penetrate_Types.GetElements(pWHExt->Shield_AffectTypes);

	if (!affectedTypes.empty() && !affectedTypes.contains(this->Type))
		return false;

	if (!pWHExt->Shield_Penetrate_Armor_Types.empty()) {
		Armor shieldArmror = this->GetOrInheritArmor();
		if(!pWHExt->Shield_Penetrate_Armor_Types.Contains(ArmorTypeClass::Array[(int)shieldArmror].get()))
			return false;
	}

	if (pWarhead->Psychedelic)
		return !this->Type->ImmuneToPsychedelic;

	return pWHExt->Shield_Penetrate;
}

void ShieldClass::OnTemporalUpdate(TemporalClass* pTemporal)
{
	if (!this->Temporal)
	{
		this->Temporal = true;

		const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
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

	auto const loc = this->Techno->InlineMapCoords();
	if (!loc.IsValid())
		return;

	if (this->Techno->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(this->Techno))->LimboID != -1)
			return;
	}

	if (this->Techno->Health <= 0 || !this->Techno->IsAlive || this->Techno->IsSinking)
	{
		TechnoExtContainer::Instance.Find(this->Techno)->Shield = nullptr;
		return;
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

	const auto selfHealingCheck = this->SelfHealEnabledByCheck();
	auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;

	if (selfHealingCheck == SelfHealingStatus::Offline) {
		timer->Pause();
	}
	else if(selfHealingCheck == SelfHealingStatus::Online) {
		timer->Resume();
		this->RespawnShield();
		this->SelfHealing();
	}

	double ratio = this->Techno->GetHealthPercentage();
	if (!this->AreAnimsHidden)
	{
		if (GeneralUtils::HasHealthRatioThresholdChanged(LastTechnoHealthRatio, ratio))
			this->UpdateIdleAnim();

		if (!this->Temporal && this->Online && (this->HP > 0 && this->Techno->Health > 0))
			this->CreateAnim();
	}

	if (this->Timers.Respawn_Warhead.Completed())
		this->Timers.Respawn_Warhead.Stop();

	if (this->Timers.SelfHealing_Warhead.Completed())
		this->Timers.SelfHealing_Warhead.Stop();

	this->LastTechnoHealthRatio = ratio;
}

#include <Ext/AnimType/Body.h>

// The animation is automatically destroyed when the associated unit receives the isCloak statute.
// Therefore, we must zero out the invalid pointer
void ShieldClass::CloakCheck()
{
	this->Cloak = this->Techno->IsInCloakState();

	if (this->Cloak && this->IdleAnim && AnimTypeExtContainer::Instance.Find(this->IdleAnim->Type)->DetachOnCloak)
		KillAnim();
}

void ShieldClass::OnlineCheck()
{
	if (!this->Type->Powered)
		return;

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;

	auto pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && this->Techno->WhatAmI() == BuildingClass::AbsID)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(this->Techno);
		isActive = pBuilding->IsPowerOnline();
	}

	if (!isActive)
	{
		if (this->Online){
			this->UpdateTint();
			this->Online = false;
		}
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
		if (!this->Online) {
			this->UpdateTint();
			this->Online = true;
		}

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

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
	timer->Resume();

	if (this->IdleAnim)
	{
		this->IdleAnim->UnderTemporal = false;
		this->IdleAnim->Unpause();
	}
}

void ShieldClass::UpdateTint()
{
	if (this->Type->Tint_Color.isset() || this->Type->Tint_Intensity != 0.0)
		this->Techno->MarkForRedraw();
}

// Is used for DeploysInto/UndeploysInto and DeploysInto/UndeploysInto
bool ShieldClass::ConvertCheck()
{
	const auto newID = this->Techno->GetTechnoType();

	if (this->CurTechnoType == newID)
		return false;

	const auto pTechnoExt = TechnoExtContainer::Instance.Find(this->Techno);
	const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(this->Techno->GetTechnoType());
	const auto pOldType = this->Type;
	bool allowTransfer = this->Type->AllowTransfer.Get(Attached);

	// Update shield type.
	if (!allowTransfer && !pTechnoTypeExt->ShieldType->Strength)
	{
		this->KillAnim();
		pTechnoExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(DEFAULT_STR2);
		pTechnoExt->Shield = nullptr;
		this->UpdateTint();

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
		bool isDamaged = this->Techno->GetHealthPercentage() <= this->Type->GetConditionYellow();
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
		const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
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
	this->UpdateTint();

	return false;
}

SelfHealingStatus ShieldClass::SelfHealEnabledByCheck()
{
	if (!this->Type->SelfHealing_EnabledBy.empty())
	{
		for (auto& pBuilding : this->Techno->Owner->Buildings)
		{
			if (!this->Type->SelfHealing_EnabledBy.Contains(pBuilding->Type))
				continue;

			const bool isActive = !(pBuilding->Deactivated || pBuilding->IsUnderEMP()) && pBuilding->IsPowerOnline();

			if (isActive)
			{
				return SelfHealingStatus::Online;
			}
		}

		return SelfHealingStatus::Offline;
	}

	return SelfHealingStatus::Online;
}

void ShieldClass::SelfHealing()
{
	const auto pType = this->Type;
	auto timer = &this->Timers.SelfHealing;
	auto timerWHModifier = &this->Timers.SelfHealing_Warhead;
	auto timerSelfHealCombat = &this->Timers.SelfHealing_CombatRestart;

	if (timerSelfHealCombat->InProgress()) {
		return;
	}
	else if (timerSelfHealCombat->Completed())
	{
		const int rate = timerWHModifier->InProgress() ? this->SelfHealing_Rate_Warhead : this->Type->SelfHealing_Rate;
		timer->Start(rate);
		timerSelfHealCombat->Stop();
	}

	if (timerWHModifier->Completed() && timer->InProgress())
	{
		double mult = this->SelfHealing_Rate_Warhead > 0 ? Type->SelfHealing_Rate / this->SelfHealing_Rate_Warhead : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}

	const double amount = timerWHModifier->InProgress() ? this->SelfHealing_Warhead : pType->SelfHealing;
	const int rate = timerWHModifier->InProgress() ? this->SelfHealing_Rate_Warhead : pType->SelfHealing_Rate;
	const auto percentageAmount = this->GetPercentageAmount(amount);

	if (percentageAmount != 0)
	{
		if ((this->HP < this->Type->Strength || percentageAmount < 0) && timer->StartTime == -1)
			timer->Start(rate);

		if (this->HP > 0 && timer->Completed())
		{
			timer->Start(rate);
			this->HP += percentageAmount;

			this->UpdateIdleAnim();

			if (this->HP > pType->Strength)
			{
				this->HP = pType->Strength;
				timer->Stop();
			}
			else if (this->HP <= 0)
			{
				this->BreakShield();
			}
		}
	}
}

void ShieldClass::InvalidateAnimPointer(AnimClass* ptr)
{
	if (this->IdleAnim && this->IdleAnim.get() == ptr)
		this->IdleAnim.release();
}

void ShieldClass::BreakShield(AnimTypeClass* pBreakAnim, WeaponTypeClass* pBreakWeapon)
{
	this->HP = 0;

	if (this->Type->Respawn)
		this->Timers.Respawn.Start(this->Timers.Respawn_Warhead.InProgress() ? Respawn_Rate_Warhead : this->Type->Respawn_Rate);

	this->Timers.SelfHealing.Stop();

	this->KillAnim();

	if (!this->AreAnimsHidden)
	{
		const auto pAnimType = pBreakAnim ? pBreakAnim : this->Type->BreakAnim;

		if (pAnimType)
		{
			auto const pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location);
			pAnim->SetOwnerObject(this->Techno);
			AnimExtData::SetAnimOwnerHouseKind(pAnim, Techno->GetOwningHouse(), nullptr, Techno);
		}
	}

	this->LastBreakFrame = Unsorted::CurrentFrame;
	this->UpdateTint();

	if (const auto pWeaponType = pBreakWeapon ? pBreakWeapon : this->Type->BreakWeapon)
	{
		AbstractClass* const pTarget = this->Type->BreakWeapon_TargetSelf.Get() ? static_cast<AbstractClass*>(this->Techno) : this->Techno->GetCell();
		WeaponTypeExtData::DetonateAt(pWeaponType, pTarget, this->Techno, true, nullptr);
	}
}

void ShieldClass::RespawnShield()
{
	auto timer = &this->Timers.Respawn;
	auto timerWH = &this->Timers.Respawn_Warhead;

	if (this->HP <= 0 && timer->Completed())
	{
		timer->Stop();
		double amount = timerWH->InProgress() ? Respawn_Warhead : this->Type->Respawn;
		this->HP = this->GetPercentageAmount(amount);
		this->UpdateTint();
	}
	else if (timerWH->Completed() && timer->InProgress())
	{
		double mult = this->Respawn_Rate_Warhead > 0 ? Type->Respawn_Rate / this->Respawn_Rate_Warhead : 1.0;
		timer->TimeLeft = static_cast<int>(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::SetRespawn(int duration, double amount, int rate, bool resetTimer)
{
	auto timer = &this->Timers.Respawn;
	auto timerWH = &this->Timers.Respawn_Warhead;
	const bool modifierTimerInProgress = timerWH->InProgress();

	this->Respawn_Warhead = amount;
	this->Respawn_Rate_Warhead = rate >= 0 ? rate : Type->Respawn_Rate;

	timerWH->Start(duration);

	if (this->HP <= 0 && Respawn_Rate_Warhead >= 0 && resetTimer)
	{
		timer->Start(Respawn_Rate_Warhead);
	}
	else if (timer->InProgress() && !modifierTimerInProgress && this->Respawn_Rate_Warhead != Type->Respawn_Rate)
	{
		double mult = Type->Respawn_Rate > 0 ? ((double)this->Respawn_Rate_Warhead / (double)Type->Respawn_Rate.Get()) : 1.0;
		timer->TimeLeft = int(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::SetSelfHealing(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer)
{
	auto timer = &this->Timers.SelfHealing;
	auto timerWH = &this->Timers.SelfHealing_Warhead;
	const bool modifierTimerInProgress = timerWH->InProgress();

	this->SelfHealing_Warhead = amount;
	this->SelfHealing_Rate_Warhead = rate >= 0 ? rate : Type->SelfHealing_Rate;
	this->SelfHealing_RestartInCombat_Warhead = restartInCombat;
	this->SelfHealing_RestartInCombatDelay_Warhead = restartInCombatDelay >= 0 ? restartInCombatDelay : Type->SelfHealing_RestartInCombatDelay;

	timerWH->Start(duration);

	if (resetTimer)
	{
		timer->Start(this->SelfHealing_Rate_Warhead);
	}
	else if (timer->InProgress() && !modifierTimerInProgress && this->SelfHealing_Rate_Warhead != Type->SelfHealing_Rate)
	{
		double mult = Type->SelfHealing_Rate > 0 ? ((double)this->SelfHealing_Rate_Warhead / (double)Type->SelfHealing_Rate.Get()) : 1.0;
		timer->TimeLeft = int(timer->GetTimeLeft() * mult);
	}
}

void ShieldClass::CreateAnim()
{
	auto idleAnimType = this->GetIdleAnimType();

		if (this->Cloak && (!idleAnimType || AnimTypeExtContainer::Instance.Find(idleAnimType)->DetachOnCloak))
			return;

	if (!this->IdleAnim && idleAnimType)
	{
		auto const pAnim = GameCreate<AnimClass>(idleAnimType, this->Techno->Location);
		pAnim->RemainingIterations = 0xFFu;
		AnimExtData::SetAnimOwnerHouseKind(pAnim, this->Techno->Owner, nullptr, this->Techno, false, false);
		pAnim->SetOwnerObject(this->Techno);
		auto pAnimExt = ((FakeAnimClass*)pAnim)->_GetExtData();
		pAnimExt->IsShieldIdleAnim = true;
		this->IdleAnim.reset(pAnim);
	}
}

void ShieldClass::KillAnim()
{
	IdleAnim.reset(nullptr);
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

	bool isDamaged = this->Techno->GetHealthPercentage() <= this->Type->GetConditionYellow();

	return this->Type->GetIdleAnimType(isDamaged, this->GetHealthRatio());
}

void ShieldClass::DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	if (this->HP >= 0 || this->Type->Respawn)
	{
		if (this->HP <= 0 && this->Type->Pips_HideIfNoStrength)
			return;

		if (this->Techno->WhatAmI() == BuildingClass::AbsID)
			this->DrawShieldBar_Building(iLength, pLocation, pBound);
		else
			this->DrawShieldBar_Other(iLength, pLocation, pBound);
	}
}

void ShieldClass::DrawShieldBar_Building(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	if (this->HP == 0 && this->Type->Pips_HideIfNoStrength)
		return;

	static constexpr Point2D selectBracketPositionOffset { -6 , -3 };
	static constexpr Point2D locationOffset {-5 , -3};

	Point2D selectBracketPosition = TechnoExtData::GetBuildingSelectBracketPosition(
		this->Techno,
		BuildingSelectBracketPosition::Top,
		selectBracketPositionOffset
	);

	Point2D vLoc = *pLocation + locationOffset;
	Point2D position = { 0, 0 };

	const int iTotal = DrawShieldBar_PipAmount(iLength);
	int frame = this->DrawShieldBar_Pip(true);

	if (iTotal > 0)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iTotal, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			position = selectBracketPosition;
			position.X -= deltaX + 6;
			position.Y -= deltaY + 3;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				frame, &position, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	if (iTotal < iLength)
	{
		int emptyFrame = this->Type->Pips_Building_Empty.Get(RulesExtData::Instance()->Pips_Shield_Building_Empty.Get(0));

		int frameIdx, deltaX, deltaY;
		for (frameIdx = iLength - iTotal, deltaX = 4 * iTotal, deltaY = -2 * iTotal;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			position = selectBracketPosition;
			position.X -= deltaX + 6;
			position.Y -= deltaY + 3;


			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
				emptyFrame, &position, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
}

void ShieldClass::DrawShieldBar_Other(int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	if (this->HP == 0 && this->Type->Pips_HideIfNoStrength)
		return;

	const auto pipBoard = this->Type->Pips_Background_SHP
		.Get(RulesExtData::Instance()->Pips_Shield_Background_SHP.Get(FileSystem::PIPBRD_SHP()));

	if (!pipBoard)
		return;

	auto position = TechnoExtData::GetFootSelectBracketPosition(Techno, Anchor(HorizontalPosition::Left, VerticalPosition::Top));
	position.X -= 1;
	position.Y += this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->Type->BracketDelta - 3;
	int	frame = (iLength == 8) ? (pipBoard->Frames > 2 ? 3 : 1) : pipBoard->Frames > 2 ? 2 : 0;

	if (this->Techno->IsSelected)
	{
		position.X += iLength + 1 + (iLength == 8 ? iLength + 1 : 0);
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pipBoard,
			frame, &position, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		position.X -= iLength + 1 + (iLength == 8 ? iLength + 1 : 0);
	}

	const int iTotal = DrawShieldBar_PipAmount(iLength);

	frame = this->DrawShieldBar_Pip(false);

	for (int i = 0; i < iTotal; ++i)
	{
		position.X += 2;
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			frame, &position, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

int ShieldClass::DrawShieldBar_Pip(const bool isBuilding)
{
	const auto strength = this->Type->Strength;
	const double condYellow = this->Type->GetConditionYellow();
	const double condRed = this->Type->GetConditionRed();

	const auto& pips_Shield = isBuilding ? this->Type->Pips_Building : this->Type->Pips;
	const auto& pips_Global = isBuilding ? RulesExtData::Instance()->Pips_Shield_Building : RulesExtData::Instance()->Pips_Shield;
	const auto& shieldPip = pips_Shield->X != -1 ? pips_Shield : pips_Global;

	if (this->HP > condYellow * strength && shieldPip->X != -1)
		return shieldPip->X;
	else if (this->HP > condRed * strength && (shieldPip->Y != -1 || shieldPip->X != -1))
		return shieldPip->Y == -1 ? shieldPip->X : shieldPip->Y;
	else if (shieldPip->Z != -1 || shieldPip->X != -1)
		return shieldPip->Z == -1 ? shieldPip->X : shieldPip->Z;

	return isBuilding ? 5 : 16;
}