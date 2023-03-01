#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>

#include <Utilities/EnumFunctions.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <New/Type/ArmorTypeClass.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>
#include <New/Entity/FlyingStrings.h>

WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;
void WarheadTypeExt::ExtData::Initialize()
{
	this->IsNukeWarhead = !std::strcmp(RulesExt::Global()->NukeWarheadName.data(), this->Get()->get_ID());
	this->Launchs.reserve(2);
}

//https://github.com/Phobos-developers/Phobos/issues/629
void WarheadTypeExt::ExtData::ApplyDamageMult(TechnoClass* pVictim, args_ReceiveDamage* pArgs)
{
	if (!pVictim)
		return;

	auto const pExt = TechnoExt::ExtMap.Find(pVictim);

	if (pExt->ReceiveDamageMultiplier.isset())
	{
		*pArgs->Damage = static_cast<int>(*pArgs->Damage * pExt->ReceiveDamageMultiplier.get());
		pExt->ReceiveDamageMultiplier.clear();
	}

	auto const& nAllyMod = AffectAlly_Damage_Mod;
	auto const& nOwnerMod = AffectOwner_Damage_Mod;
	auto const& nEnemyMod = AffectEnemies_Damage_Mod;

	if ((!nAllyMod.isset() && !nOwnerMod.isset() && !nEnemyMod.isset()))
		return;

	auto const pHouse = pArgs->SourceHouse ? pArgs->SourceHouse : pArgs->Attacker ? pArgs->Attacker->GetOwningHouse() : HouseExt::FindCivilianSide();
	auto const pVictimHouse = pVictim->GetOwningHouse();

	if (pHouse && pVictimHouse)
	{
		auto const pWH = Get();
		const int nDamage = *pArgs->Damage;

		if (pVictimHouse != pHouse)
		{
			if (pVictimHouse->IsAlliedWith(pHouse) && pWH->AffectsAllies && nAllyMod.isset())
			{
				*pArgs->Damage = static_cast<int>(nDamage * nAllyMod.Get());
			}
			else if (AffectsEnemies.Get() && nEnemyMod.isset())
			{
				*pArgs->Damage = static_cast<int>(nDamage * nEnemyMod.Get());
			}
		}
		else if (AffectsOwner.Get() && nOwnerMod.isset())
		{
			*pArgs->Damage = static_cast<int>(nDamage * nOwnerMod.Get());
		}
	}
}

void WarheadTypeExt::ExtData::ApplyRecalculateDistanceDamage(TechnoClass* pVictim, args_ReceiveDamage* pArgs)
{
	if (!this->RecalculateDistanceDamage.Get() || !pArgs->Attacker)
		return;

	if (!this->RecalculateDistanceDamage_IgnoreMaxDamage && *pArgs->Damage == RulesGlobal->MaxDamage)
		return;

	const auto pThisType = pVictim->GetTechnoType();
	const auto range = pArgs->Attacker->DistanceFrom(pVictim);
	const auto range_factor = range / (this->RecalculateDistanceDamage_Add_Factor.Get() * 256);
	const auto add = (this->RecalculateDistanceDamage_Add.Get() * range_factor);

	const auto multiply = pow((this->RecalculateDistanceDamage_Multiply.Get()), range_factor);

	auto nAddDamage = add * multiply;
	if (this->RecalculateDistanceDamage_ProcessVerses)
		nAddDamage *= GeneralUtils::GetWarheadVersusArmor(pArgs->WH, pThisType->Armor);

	auto const nEligibleAddDamage = std::clamp((int)nAddDamage,
		this->RecalculateDistanceDamage_Min.Get(), this->RecalculateDistanceDamage_Max.Get());

	*pArgs->Damage += nEligibleAddDamage;

	if (this->RecalculateDistanceDamage_Display || Phobos::Debug_DisplayDamageNumbers)
	{
		TechnoClass* pOwner = this->RecalculateDistanceDamage_Display_AtFirer ? pArgs->Attacker : pVictim;
		FlyingStrings::AddMoneyString(true, *pArgs->Damage, pOwner,
			AffectedHouse::All, pOwner->Location,
			this->RecalculateDistanceDamage_Display_Offset, ColorStruct::Empty);
	}

}

bool WarheadTypeExt::ExtData::CanAffectHouse(HouseClass* pOwnerHouse, HouseClass* pTargetHouse)
{
	if (pOwnerHouse && pTargetHouse)
	{
		if (pTargetHouse == pOwnerHouse)
			return this->AffectsOwner.Get(this->Get()->AffectsAllies);

		if (pOwnerHouse->IsAlliedWith(pTargetHouse) && pTargetHouse != pOwnerHouse)
			return this->Get()->AffectsAllies;

		return AffectsEnemies.Get();
	}

	return true;
}

bool WarheadTypeExt::ExtData::CanDealDamage(TechnoClass* pTechno, bool Bypass, bool SkipVerses)
{
	if (pTechno)
	{
		if (pTechno->InLimbo
			|| !pTechno->IsOnMap
			|| !pTechno->IsAlive
			|| !pTechno->Health
			|| pTechno->IsSinking
			|| !pTechno->GetTechnoType()
			)
			return false;

		if (pTechno->GetTechnoType()->Immune)
			return false;

		if (auto const pBld = specific_cast<BuildingClass*>(pTechno))
		{
			auto const pBldExt = BuildingExt::ExtMap.Find(pBld);
			if (pBld->Type->InvisibleInGame || pBldExt->LimboID != -1)
				return false;
		}

		if (const auto pFoot = abstract_cast<FootClass*>(pTechno))
		{
			if (TechnoExt::IsChronoDelayDamageImmune(pFoot))
				return false;
		}

		if (pTechno->IsBeingWarpedOut())
			return false;

		if (!SkipVerses && EffectsRequireVerses.Get())
		{
			auto nArmor = pTechno->GetTechnoType()->Armor;
			const auto pExt = TechnoExt::ExtMap.Find(pTechno);

			if (pExt->CurrentShieldType && pExt->GetShield() && pExt->GetShield()->IsActive())
				nArmor = pExt->CurrentShieldType->Armor;

			return (fabs(GeneralUtils::GetWarheadVersusArmor(Get(), nArmor)) >= 0.001);
		}

		return true;
	}

	return Bypass;
}

bool WarheadTypeExt::ExtData::CanDealDamage(TechnoClass* pTechno, int damageIn, int distanceFromEpicenter, int& DamageResult, bool effectsRequireDamage)
{
	auto nArmor = pTechno->GetTechnoType()->Armor;

	if (auto pShield = TechnoExt::ExtMap.Find(pTechno)->GetShield())
		if (pShield->IsActive())
			nArmor = pShield->GetType()->Armor;

	if (damageIn > 0)
		DamageResult = MapClass::GetTotalDamage(damageIn, Get(), nArmor, distanceFromEpicenter);
	else
		DamageResult = -MapClass::GetTotalDamage(-damageIn, Get(), nArmor, distanceFromEpicenter);

	if (damageIn == 0)
	{
		return AllowZeroDamage;
	}
	else
	{
		if (EffectsRequireVerses)
		{
			if (MapClass::GetTotalDamage(RulesGlobal->MaxDamage, Get(), nArmor, 0) == 0.0)
			{
				return false;
			}

			if (effectsRequireDamage || EffectsRequireDamage)
			{
				return DamageResult != 0;
			}
		}
	}

	return true;

}

FullMapDetonateResult WarheadTypeExt::ExtData::EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner)
{
	if (!CanDealDamage(pTechno, false, !this->DetonateOnAllMapObjects_RequireVerses.Get()))
		return FullMapDetonateResult::TargetNotDamageable;

	auto const pType = pTechno->GetTechnoType();

	if (!EnumFunctions::IsTechnoEligibleB(pTechno, this->DetonateOnAllMapObjects_AffectTargets))
		return FullMapDetonateResult::TargetNotEligible;

	if (!EnumFunctions::CanTargetHouse(this->DetonateOnAllMapObjects_AffectHouses, pOwner, pTechno->Owner))
		return FullMapDetonateResult::TargetHouseNotEligible;

	if ((!this->DetonateOnAllMapObjects_AffectTypes.empty() &&
		!this->DetonateOnAllMapObjects_AffectTypes.Contains(pType)) ||
		this->DetonateOnAllMapObjects_IgnoreTypes.Contains(pType))
	{
		return FullMapDetonateResult::TargetRestricted;
	}

	return FullMapDetonateResult::TargetValid;
}

bool WarheadTypeExt::ExtData::CanTargetHouse(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (pHouse && pTarget)
	{
		return CanAffectHouse(pHouse, pTarget->GetOwningHouse());
	}

	return true;
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage, bool targetCell)
{
	if (!pThis)
		return;

	BulletTypeClass* pType = BulletTypeExt::GetDefaultBulletType();
	AbstractClass* pATarget = !targetCell ? static_cast<AbstractClass*>(pTarget) : pTarget->GetCell();

	if (BulletClass* pBullet = BulletTypeExt::ExtMap.Find(pType)->CreateBullet(pATarget, pOwner,
		damage, pThis, 0, 0, pThis->Bright))
	{
		BulletExt::DetonateAt(pBullet, pTarget, pOwner);
	}
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool targetCell)
{
	if (!pThis)
		return;

	BulletTypeClass* pType = BulletTypeExt::GetDefaultBulletType();
	AbstractClass* pTarget = !targetCell ? nullptr : Map[coords];

	if (pThis->NukeMaker)
	{
		if (!pTarget)
		{
			Debug::Log("WarheadTypeExt::DetonateAt , cannot execute when invalid Target is present , need to be avail ! \n");
			return;
		}
	}

	if (BulletClass* pBullet = BulletTypeExt::ExtMap.Find(pType)->CreateBullet(pTarget, pOwner,
		damage, pThis, 0, 0, pThis->Bright))
	{
		BulletExt::DetonateAt(pBullet, pTarget, pOwner, coords);
	}
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, AbstractClass* pTarget, const CoordStruct& coords, TechnoClass* pOwner, int damage)
{
	if (!pThis)
		return;

	BulletTypeClass* pType = BulletTypeExt::GetDefaultBulletType();

	if (pThis->NukeMaker)
	{
		if (!pTarget)
		{
			Debug::Log("WarheadTypeExt::DetonateAt , cannot execute when invalid Target is present , need to be avail ! \n");
			return;
		}
	}

	if (BulletClass* pBullet = BulletTypeExt::ExtMap.Find(pType)->CreateBullet(pTarget, pOwner,
		damage, pThis, 0, 0, pThis->Bright))
	{
		BulletExt::DetonateAt(pBullet, pTarget, pOwner, coords);
	}
}

// =============================
// load / save

void WarheadTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	// Miscs
	this->Reveal.Read(exINI, pSection, "Reveal");

	Nullable<bool> spySat;
	spySat.Read(exINI, pSection, GameStrings::SpySat());

	if (spySat.isset() && spySat.Get())
		this->Reveal = -1;

	this->BigGap.Read(exINI, pSection, "BigGap");
	this->TransactMoney.Read(exINI, pSection, "TransactMoney");
	this->SplashList.Read(exINI, pSection, GameStrings::SplashList());
	this->SplashList_PickRandom.Read(exINI, pSection, "SplashList.PickRandom");
	this->RemoveDisguise.Read(exINI, pSection, "RemoveDisguise");
	this->RemoveMindControl.Read(exINI, pSection, "RemoveMindControl");
	this->AnimList_PickRandom.Read(exINI, pSection, "AnimList.PickRandom");
	this->AnimList_ShowOnZeroDamage.Read(exINI, pSection, "AnimList.ShowOnZeroDamage");
	this->DecloakDamagedTargets.Read(exINI, pSection, "DecloakDamagedTargets");
	this->ShakeIsLocal.Read(exINI, pSection, "ShakeIsLocal");

	// Crits
	this->Crit_Chance.Read(exINI, pSection, "Crit.Chance");
	this->Crit_ApplyChancePerTarget.Read(exINI, pSection, "Crit.ApplyChancePerTarget");
	this->Crit_ExtraDamage.Read(exINI, pSection, "Crit.ExtraDamage");
	this->Crit_Warhead.Read(exINI, pSection, "Crit.Warhead");
	this->Crit_Affects.Read(exINI, pSection, "Crit.Affects");
	this->Crit_AffectsHouses.Read(exINI, pSection, "Crit.AffectsHouses");
	this->Crit_AnimList.Read(exINI, pSection, "Crit.AnimList");
	this->Crit_AnimList_PickRandom.Read(exINI, pSection, "Crit.AnimList.PickRandom");
	this->Crit_AnimOnAffectedTargets.Read(exINI, pSection, "Crit.AnimOnAffectedTargets");
	this->Crit_AffectBelowPercent.Read(exINI, pSection, "Crit.AffectBelowPercent");
	this->Crit_SuppressOnIntercept.Read(exINI, pSection, "Crit.SuppressWhenIntercepted");

	this->MindControl_Anim.Read(exINI, pSection, "MindControl.Anim");

	// Ares tags
	// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
	this->AffectsEnemies.Read(exINI, pSection, "AffectsEnemies");
	this->AffectsOwner.Read(exINI, pSection, "AffectsOwner");
	this->EffectsRequireDamage.Read(exINI, pSection, "EffectsRequireDamage");
	this->EffectsRequireVerses.Read(exINI, pSection, "EffectsRequireVerses");
	this->AllowZeroDamage.Read(exINI, pSection, "AllowZeroDamage");

	// Shields
	this->Shield_Penetrate.Read(exINI, pSection, "Shield.Penetrate");
	this->Shield_Break.Read(exINI, pSection, "Shield.Break");
	this->Shield_BreakAnim.Read(exINI, pSection, "Shield.BreakAnim");
	this->Shield_HitAnim.Read(exINI, pSection, "Shield.HitAnim");
	this->Shield_BreakWeapon.Read(exINI, pSection, "Shield.BreakWeapon", true);
	this->Shield_AbsorbPercent.Read(exINI, pSection, "Shield.AbsorbPercent");
	this->Shield_PassPercent.Read(exINI, pSection, "Shield.PassPercent");
	this->Shield_Respawn_Duration.Read(exINI, pSection, "Shield.Respawn.Duration");
	this->Shield_Respawn_Amount.Read(exINI, pSection, "Shield.Respawn.Amount");
	this->Shield_Respawn_Rate_InMinutes.Read(exINI, pSection, "Shield.Respawn.Rate");
	this->Shield_Respawn_Rate = (int)(this->Shield_Respawn_Rate_InMinutes * 900);
	this->Shield_Respawn_ResetTimer.Read(exINI, pSection, "Shield.Respawn.RestartTimer");
	this->Shield_SelfHealing_Duration.Read(exINI, pSection, "Shield.SelfHealing.Duration");
	this->Shield_SelfHealing_Amount.Read(exINI, pSection, "Shield.SelfHealing.Amount");
	this->Shield_SelfHealing_Rate_InMinutes.Read(exINI, pSection, "Shield.SelfHealing.Rate");
	this->Shield_SelfHealing_Rate = (int)(this->Shield_SelfHealing_Rate_InMinutes * 900);
	this->Shield_SelfHealing_ResetTimer.Read(exINI, pSection, "Shield.SelfHealing.RestartTimer");
	this->Shield_AttachTypes.Read(exINI, pSection, "Shield.AttachTypes");
	this->Shield_RemoveTypes.Read(exINI, pSection, "Shield.RemoveTypes");
	this->Shield_ReplaceOnly.Read(exINI, pSection, "Shield.ReplaceOnly");
	this->Shield_ReplaceNonRespawning.Read(exINI, pSection, "Shield.ReplaceNonRespawning");
	this->Shield_InheritStateOnReplace.Read(exINI, pSection, "Shield.InheritStateOnReplace");
	this->Shield_MinimumReplaceDelay.Read(exINI, pSection, "Shield.MinimumReplaceDelay");
	this->Shield_AffectTypes.Read(exINI, pSection, "Shield.AffectTypes");

	this->NotHuman_DeathSequence.Read(exINI, pSection, "NotHuman.DeathSequence");

	// Transact
	this->Transact.Read(exINI, pSection, "Transact");

	this->TransactMoney_Ally.Read(exINI, pSection, "TransactMoney.Ally");
	this->TransactMoney_Enemy.Read(exINI, pSection, "TransactMoney.Enemy");
	this->Transact_AffectsEnemies.Read(exINI, pSection, "TransactMoney.AffectsEnemies");
	this->Transact_AffectsAlly.Read(exINI, pSection, "TransactMoney.AffectsAlly");
	this->Transact_AffectsOwner.Read(exINI, pSection, "TransactMoney.AffectsOwner");

	this->Transact_SpreadAmongTargets.Read(exINI, pSection, "Transact.SpreadAmongTargets");
	this->Transact_Experience_Value.Read(exINI, pSection, "Transact.Experience.Value");
	this->Transact_Experience_Source_Flat.Read(exINI, pSection, "Transact.Experience.Source.Flat");
	this->Transact_Experience_Source_Percent.Read(exINI, pSection, "Transact.Experience.Source.Percent");
	this->Transact_Experience_Source_Percent_CalcFromTarget.Read(exINI, pSection, "Transact.Experience.Source.Percent.CalcFromTarget");
	this->Transact_Experience_Target_Flat.Read(exINI, pSection, "Transact.Experience.Target.Flat");
	this->Transact_Experience_Target_Percent.Read(exINI, pSection, "Transact.Experience.Target.Percent");
	this->Transact_Experience_Target_Percent_CalcFromSource.Read(exINI, pSection, "Transact.Experience.Target.Percent.CalcFromSource");
	this->Transact_Experience_IgnoreNotTrainable.Read(exINI, pSection, "Transact.Experience.IgnoreNotTrainable");

	this->TransactMoney_Display.Read(exINI, pSection, "TransactMoney.Display");
	this->TransactMoney_Display_Houses.Read(exINI, pSection, "TransactMoney.Display.Houses");
	this->TransactMoney_Display_AtFirer.Read(exINI, pSection, "TransactMoney.Display.AtFirer");
	this->TransactMoney_Display_Offset.Read(exINI, pSection, "TransactMoney.Display.Offset");

	this->StealMoney.Read(exINI, pSection, "StealMoney.Amount");
	this->Steal_Display_Houses.Read(exINI, pSection, "StealMoney.Display.Houses");
	this->Steal_Display.Read(exINI, pSection, "StealMoney.Display");
	this->Steal_Display_Offset.Read(exINI, pSection, "StealMoney.Display.Offset");

	this->NotHuman_DeathAnim.Read(exINI, pSection, "NotHuman.DeathAnim");
	this->AllowDamageOnSelf.Read(exINI, pSection, "AllowDamageOnSelf");
	this->Debris_Conventional.Read(exINI, pSection, "Debris.Conventional");

	this->GattlingStage.Read(exINI, pSection, "TargetGattlingStage");
	this->GattlingRateUp.Read(exINI, pSection, "TargetGattlingRateUp");
	this->ReloadAmmo.Read(exINI, pSection, "TargetReloadAmmo");

	this->MindControl_Threshold.Read(exINI, pSection, "MindControl.Threshold");
	this->MindControl_Threshold_Inverse.Read(exINI, pSection, "MindControl.Threshold.Inverse");
	this->MindControl_AlternateDamage.Read(exINI, pSection, "MindControl.AlternateDamage");
	this->MindControl_AlternateWarhead.Read(exINI, pSection, "MindControl.AlternateWarhead", true);
	this->MindControl_CanKill.Read(exINI, pSection, "MindControl.CanKill");

	this->DetonateOnAllMapObjects.Read(exINI, pSection, "DetonateOnAllMapObjects");
	this->DetonateOnAllMapObjects_RequireVerses.Read(exINI, pSection, "DetonateOnAllMapObjects.RequireVerses");
	this->DetonateOnAllMapObjects_AffectTargets.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectTargets");
	this->DetonateOnAllMapObjects_AffectHouses.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectHouses");
	this->DetonateOnAllMapObjects_AffectTypes.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectTypes");
	this->DetonateOnAllMapObjects_IgnoreTypes.Read(exINI, pSection, "DetonateOnAllMapObjects.IgnoreTypes");

	this->RevengeWeapon.Read(exINI, pSection, "RevengeWeapon", true);
	this->RevengeWeapon_GrantDuration.Read(exINI, pSection, "RevengeWeapon.GrantDuration");
	this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouses");
	this->RevengeWeapon_Cumulative.Read(exINI, pSection, "RevengeWeapon.Cumulative");
	this->RevengeWeapon_MaxCount.Read(exINI, pSection, "RevengeWeapon.MaxCount");

#pragma region Otamaa

	this->SquidSplash.Read(exINI, pSection, "Parasite.SplashAnims");
	this->TemporalExpiredAnim.Read(exINI, pSection, "Temporal.ExpiredAnim");
	this->TemporalDetachDamageFactor.Read(exINI, pSection, "Temporal.ExpiredDamageFactor");
	this->TemporalExpiredApplyDamage.Read(exINI, pSection, "Temporal.ExpiredApplyDamage");
	this->DebrisAnimTypes.Read(exINI, pSection, "DebrisAnims");
	this->Flammability.Read(exINI, pSection, "FlameChance");

	if (this->Launchs.empty())
	{
		for (size_t i = 0; ; ++i)
		{
			LauchSWData nData;
			if (!nData.Read(exINI, pSection, i))
				break;

			this->Launchs.push_back((nData));
		}
	}

	this->Parasite_DisableRocking.Read(exINI, pSection, "Parasite.DisableRocking");
	this->Parasite_GrappleAnim.Read(exINI, pSection, "Parasite.GrappleAnim");
	this->Parasite_ParticleSys.Read(exINI, pSection, "Parasite.ParticleSystem");
	this->Parasite_TreatInfantryAsVehicle.Read(exINI, pSection, "Parasite.TreatInfantryAsVehicle");
	this->Parasite_InvestationWP.Read(exINI, pSection, "Parasite.DamagingWeapon");
	this->Parasite_Damaging_Chance.Read(exINI, pSection, "Parasite.DamagingChance");

	auto ReadHitAnim = [this, &exINI, pSection](const char* pBaseKey, bool bAllocate = true)
	{
		char tempBuffer[2048];
		for (size_t i = 0; i < ArmorTypeClass::Array.size(); ++i)
		{
			Nullable<AnimTypeClass*> pAnimReaded;
			if (auto pArmor = ArmorTypeClass::Array[i].get())
			{
				_snprintf_s(tempBuffer, _TRUNCATE, "%s.%s", pBaseKey, pArmor->Name.data());
				pAnimReaded.Read(exINI, pSection, tempBuffer, bAllocate);
			}
			ArmorHitAnim[i] = pAnimReaded.Get(nullptr);
		}

		//remove invalid items!
		if (!ArmorHitAnim.empty())
		{
			//remove invalid items to keep memory clean !
			for (auto const& [nArmor, Anim] : ArmorHitAnim)
			{
				if (!Anim)
					ArmorHitAnim.erase(nArmor);
			}
		}
	};

	ReadHitAnim("HitAnim");

	this->IsNukeWarhead.Read(exINI, pSection, "IsNukeWarhead");
	this->Remover.Read(exINI, pSection, "Remover");
	this->Remover_Anim.Read(exINI, pSection, "Remover.Anim");
	this->PermaMC.Read(exINI, pSection, "MindControl.Permanent");
	this->Sound.Read(exINI, pSection, GameStrings::Sound());

	// Code disabled , require Ares 3.0 ++
	this->Converts.Read(exINI, pSection, "Converts");
	this->Converts_From.Read(exINI, pSection, "Converts.From");
	this->Converts_To.Read(exINI, pSection, "Converts.To");
	//

	this->DeadBodies.Read(exINI, pSection, "DeadBodies");
	this->AffectEnemies_Damage_Mod.Read(exINI, pSection, "AffectEnemies.DamageModifier");
	this->AffectOwner_Damage_Mod.Read(exINI, pSection, "AffectOwner.DamageModifier");
	this->AffectAlly_Damage_Mod.Read(exINI, pSection, "AffectAlly.DamageModifier");

	this->AttachTag.Read(pINI, pSection, "AttachTag");
	this->AttachTag_Imposed.Read(exINI, pSection, "AttachTag.Imposed");
	this->AttachTag_Types.Read(exINI, pSection, "AttachTag.Types");
	this->AttachTag_Ignore.Read(exINI, pSection, "AttachTag.Ignore");

	//this->DirectionalArmor.Read(exINI, pSection, "DirectionalArmor");
	//this->DirectionalArmor_FrontMultiplier.Read(exINI, pSection, "DirectionalArmor.FrontMultiplier");
	//this->DirectionalArmor_SideMultiplier.Read(exINI, pSection, "DirectionalArmor.SideMultiplier");
	//this->DirectionalArmor_BackMultiplier.Read(exINI, pSection, "DirectionalArmor.BackMultiplier");
	//this->DirectionalArmor_FrontField.Read(exINI, pSection, "DirectionalArmor.FrontField");
	//this->DirectionalArmor_BackField.Read(exINI, pSection, "DirectionalArmor.BackField");

	//this->DirectionalArmor_FrontField = Math::min(this->DirectionalArmor_FrontField.Get(), 1.0f);
	//this->DirectionalArmor_FrontField = Math::max(this->DirectionalArmor_FrontField.Get(), 0.0f);
	//this->DirectionalArmor_BackField = Math::min(this->DirectionalArmor_BackField.Get(), 1.0f);
	//this->DirectionalArmor_BackField = Math::max(this->DirectionalArmor_BackField.Get(), 0.0f);

	this->RecalculateDistanceDamage.Read(exINI, pSection, "RecalculateDistanceDamage");
	this->RecalculateDistanceDamage_IgnoreMaxDamage.Read(exINI, pSection, "RecalculateDistanceDamage.IgnoreMaxDamage");
	this->RecalculateDistanceDamage_Add.Read(exINI, pSection, "RecalculateDistanceDamage.Add");
	this->RecalculateDistanceDamage_Multiply.Read(exINI, pSection, "RecalculateDistanceDamage.Multiply");
	this->RecalculateDistanceDamage_Add_Factor.Read(exINI, pSection, "RecalculateDistanceDamage.Add.Factor");
	this->RecalculateDistanceDamage_Multiply_Factor.Read(exINI, pSection, "RecalculateDistanceDamage.Multiply.Factor");
	this->RecalculateDistanceDamage_Max.Read(exINI, pSection, "RecalculateDistanceDamage.Max");
	this->RecalculateDistanceDamage_Min.Read(exINI, pSection, "RecalculateDistanceDamage.Min");
	this->RecalculateDistanceDamage_Display.Read(exINI, pSection, "RecalculateDistanceDamage.Display");
	this->RecalculateDistanceDamage_Display_AtFirer.Read(exINI, pSection, "RecalculateDistanceDamage.Display.AtFirer");
	this->RecalculateDistanceDamage_Display_Offset.Read(exINI, pSection, "RecalculateDistanceDamage.Display.Offset");
	this->RecalculateDistanceDamage_ProcessVerses.Read(exINI, pSection, "RecalculateDistanceDamage.Add.ProcessVerses");

#ifdef COMPILE_PORTED_DP_FEATURES_
	auto ReadHitTextData = [this, &exINI, pSection](const char* pBaseKey, bool bAllocate = true)
	{
		char tempBuffer[2048];
		for (size_t i = 0; i < ArmorTypeClass::Array.size(); ++i)
		{
			if (auto pArmor = ArmorTypeClass::Array[i].get())
			{
				_snprintf_s(tempBuffer, _TRUNCATE, "%s.%s.", pBaseKey, pArmor->Name.data());
				DamageTextPerArmor[i].Read(exINI, pSection, tempBuffer);
			}
		}
	};

	ReadHitTextData("DamageText");

#endif

#ifdef COMPILE_PORTED_DP_FEATURES
	this->PaintBallDuration.Read(exINI, pSection, "PaintBall.Duration");
	PaintBallData.Read(exINI, pSection);
#endif
#pragma endregion

	this->AttachedEffect.Read(exINI);
	this->DetonatesWeapons.Read(exINI, pSection, "DetonatesWeapons");
	this->LimboKill_IDs.Read(exINI, pSection, "LimboKill.IDs");
	this->LimboKill_Affected.Read(exINI, pSection, "LimboKill.Affected");
	this->InfDeathAnim.Read(exINI, pSection, "InfDeathAnim");
}

template <typename T>
void WarheadTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Reveal)
		.Process(this->BigGap)
		.Process(this->TransactMoney)
		.Process(this->TransactMoney_Ally)
		.Process(this->TransactMoney_Enemy)
		.Process(this->Transact_AffectsEnemies)
		.Process(this->Transact_AffectsAlly)
		.Process(this->Transact_AffectsOwner)

		.Process(this->TransactMoney_Display)
		.Process(this->TransactMoney_Display_Houses)
		.Process(this->TransactMoney_Display_AtFirer)
		.Process(this->TransactMoney_Display_Offset)

		.Process(this->SplashList)
		.Process(this->SplashList_PickRandom)
		.Process(this->RemoveDisguise)
		.Process(this->RemoveMindControl)
		.Process(this->AnimList_PickRandom)
		.Process(this->AnimList_ShowOnZeroDamage)
		.Process(this->DecloakDamagedTargets)
		.Process(this->ShakeIsLocal)

		.Process(this->Crit_Chance)
		.Process(this->Crit_ApplyChancePerTarget)
		.Process(this->Crit_ExtraDamage)
		.Process(this->Crit_Warhead)
		.Process(this->Crit_Affects)
		.Process(this->Crit_AffectsHouses)
		.Process(this->Crit_AnimList)
		.Process(this->Crit_AnimList_PickRandom)
		.Process(this->Crit_AnimOnAffectedTargets)
		.Process(this->Crit_AffectBelowPercent)
		.Process(this->Crit_SuppressOnIntercept)

		.Process(this->MindControl_Anim)

		// Ares tags
		.Process(this->AffectsEnemies)
		.Process(this->AffectsOwner)
		.Process(this->EffectsRequireDamage)
		.Process(this->EffectsRequireVerses)
		.Process(this->AllowZeroDamage)

		.Process(this->Shield_Penetrate)
		.Process(this->Shield_Break)
		.Process(this->Shield_BreakAnim)
		.Process(this->Shield_HitAnim)
		.Process(this->Shield_BreakWeapon)
		.Process(this->Shield_AbsorbPercent)
		.Process(this->Shield_PassPercent)

		.Process(this->Shield_Respawn_Duration)
		.Process(this->Shield_Respawn_Amount)
		.Process(this->Shield_Respawn_Rate)
		.Process(this->Shield_Respawn_ResetTimer)
		.Process(this->Shield_SelfHealing_Duration)
		.Process(this->Shield_SelfHealing_Amount)
		.Process(this->Shield_SelfHealing_Rate)
		.Process(this->Shield_SelfHealing_ResetTimer)
		.Process(this->Shield_AttachTypes)
		.Process(this->Shield_RemoveTypes)
		.Process(this->Shield_ReplaceOnly)
		.Process(this->Shield_ReplaceNonRespawning)
		.Process(this->Shield_InheritStateOnReplace)
		.Process(this->Shield_MinimumReplaceDelay)
		.Process(this->Shield_AffectTypes)

		.Process(this->Transact)
		.Process(this->Transact_SpreadAmongTargets)
		.Process(this->Transact_Experience_Value)
		.Process(this->Transact_Experience_Source_Flat)
		.Process(this->Transact_Experience_Source_Percent)
		.Process(this->Transact_Experience_Source_Percent_CalcFromTarget)
		.Process(this->Transact_Experience_Target_Flat)
		.Process(this->Transact_Experience_Target_Percent)
		.Process(this->Transact_Experience_Target_Percent_CalcFromSource)
		.Process(this->Transact_Experience_IgnoreNotTrainable)

		.Process(this->NotHuman_DeathSequence)
		.Process(this->AllowDamageOnSelf)
		.Process(this->Debris_Conventional)

		.Process(GattlingStage)
		.Process(GattlingRateUp)
		.Process(ReloadAmmo)

		.Process(this->MindControl_Threshold)
		.Process(this->MindControl_Threshold_Inverse)
		.Process(this->MindControl_AlternateDamage)
		.Process(this->MindControl_AlternateWarhead)
		.Process(this->MindControl_CanKill)

		.Process(this->DetonateOnAllMapObjects)
		.Process(this->DetonateOnAllMapObjects_RequireVerses)
		.Process(this->DetonateOnAllMapObjects_AffectTargets)
		.Process(this->DetonateOnAllMapObjects_AffectHouses)
		.Process(this->DetonateOnAllMapObjects_AffectTypes)
		.Process(this->DetonateOnAllMapObjects_IgnoreTypes)

		.Process(this->RevengeWeapon)
		.Process(this->RevengeWeapon_GrantDuration)
		.Process(this->RevengeWeapon_AffectsHouses)
		.Process(this->RevengeWeapon_Cumulative)
		.Process(this->RevengeWeapon_MaxCount)

		.Process(this->WasDetonatedOnAllMapObjects)

		.Process(NotHuman_DeathAnim)
		.Process(IsNukeWarhead)
		.Process(Remover)
		.Process(Remover_Anim)
		.Process(ArmorHitAnim)
		.Process(DebrisAnimTypes)
		.Process(SquidSplash)
		.Process(TemporalExpiredAnim)
		.Process(TemporalExpiredApplyDamage)
		.Process(TemporalDetachDamageFactor)
		.Process(Parasite_DisableRocking)
		.Process(Parasite_GrappleAnim)
		.Process(Parasite_ParticleSys)
		.Process(Parasite_TreatInfantryAsVehicle)
		.Process(Parasite_InvestationWP)
		.Process(Parasite_Damaging_Chance)

		.Process(Flammability)
		.Process(Launchs)
		.Process(PermaMC)
		.Process(Sound)
		.Process(this->Converts)
		.Process(this->Converts_From)
		.Process(this->Converts_To)
		.Process(StealMoney)
		.Process(Steal_Display_Houses)
		.Process(Steal_Display)
		.Process(Steal_Display_Offset)
		.Process(DeadBodies)
		.Process(AffectEnemies_Damage_Mod)
		.Process(AffectOwner_Damage_Mod)
		.Process(AffectAlly_Damage_Mod)
		.Process(this->AttachTag)
		.Process(this->AttachTag_Types)
		.Process(this->AttachTag_Ignore)
		.Process(this->AttachTag_Imposed)
		.Process(this->DirectionalArmor)
		.Process(this->DirectionalArmor_FrontMultiplier)
		.Process(this->DirectionalArmor_SideMultiplier)
		.Process(this->DirectionalArmor_BackMultiplier)
		.Process(this->DirectionalArmor_FrontField)
		.Process(this->DirectionalArmor_BackField)

		.Process(this->RecalculateDistanceDamage)
		.Process(this->RecalculateDistanceDamage_IgnoreMaxDamage)
		.Process(this->RecalculateDistanceDamage_Add)
		.Process(this->RecalculateDistanceDamage_Multiply)
		.Process(this->RecalculateDistanceDamage_Add_Factor)
		.Process(this->RecalculateDistanceDamage_Multiply_Factor)
		.Process(this->RecalculateDistanceDamage_Max)
		.Process(this->RecalculateDistanceDamage_Min)
		.Process(this->RecalculateDistanceDamage_Display)
		.Process(this->RecalculateDistanceDamage_Display_AtFirer)
		.Process(this->RecalculateDistanceDamage_Display_Offset)
		.Process(this->RecalculateDistanceDamage_ProcessVerses)

		.Process(this->AttachedEffect)
		.Process(this->DetonatesWeapons)
		.Process(this->LimboKill_IDs)
		.Process(this->LimboKill_Affected)
		.Process(this->InfDeathAnim)
#ifdef COMPILE_PORTED_DP_FEATURES_
		.Process(DamageTextPerArmor)

#endif
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(PaintBallDuration)
#endif
		;
#ifdef COMPILE_PORTED_DP_FEATURES
	PaintBallData.Serialize(Stm);
#endif

}

void WarheadTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WarheadTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void WarheadTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WarheadTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WarheadTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool WarheadTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

WarheadTypeExt::ExtContainer::ExtContainer() : Container("WarheadTypeClass") { }
WarheadTypeExt::ExtContainer::~ExtContainer() = default;

//void WarheadTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks

DEFINE_HOOK(0x75D1A9, WarheadTypeClass_CTOR, 0x7)
{
	GET(WarheadTypeClass*, pItem, EBP);

#ifndef ENABLE_NEWEXT
	WarheadTypeExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);
#endif

	return 0;
}

DEFINE_HOOK(0x75E5C8, WarheadTypeClass_SDDTOR, 0x6)
{
	GET(WarheadTypeClass*, pItem, ESI);

	WarheadTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x75E2C0, WarheadTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x75E0C0, WarheadTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(WarheadTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WarheadTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x75E2AE, WarheadTypeClass_Load_Suffix, 0x7)
{
	WarheadTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x75E39C, WarheadTypeClass_Save_Suffix, 0x5)
{
	WarheadTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x75DEAF, WarheadTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x75DEA0, WarheadTypeClass_LoadFromINI, 0x5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

//#ifdef ENABLE_NEWEXT
//DEFINE_JUMP(LJMP, 0x75D798, 0x75D7AC)
//DEFINE_JUMP(LJMP, 0x75D7B2, 0x75D7B8)
//DEFINE_JUMP(LJMP, 0x75DFAE, 0x75DFBC)
//