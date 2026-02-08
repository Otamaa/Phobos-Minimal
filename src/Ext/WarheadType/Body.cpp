#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/LightningStorm.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.h>

#include <New/Entity/FlyingStrings.h>
#include <New/Type/ArmorTypeClass.h>

#include <IonBlastClass.h>
#include <InfantryClass.h>
#include <TerrainClass.h>

#include <Phobos.SaveGame.h>

#pragma region defines
PhobosMap<IonBlastClass*, WarheadTypeExtData*> WarheadTypeExtData::IonBlastExt;

#pragma endregion
int __fastcall FakeWarheadTypeClass::ModifyDamageA(int damage, FakeWarheadTypeClass* pWH, Armor armor, int distance)
{
	int res = 0;

	if (damage == 0
		|| ScenarioClass::Instance->SpecialFlags.StructEd.Inert
		|| !pWH
		)
	{
		return res;
	}

	const auto pExt = pWH->_GetExtData();

	if (damage > 0 || pExt->ApplyModifiersOnNegativeDamage)
	{
		if (pExt->ApplyMindamage)
			damage = MaxImpl(pExt->MinDamage >= 0 ? pExt->MinDamage : RulesClass::Instance->MinDamage, damage);

		const double dDamage = (double)damage;
		const float fDamage = (float)damage;
		const double dCellSpreadRadius = pWH->CellSpread * Unsorted::d_LeptonsPerCell;
		const int cellSpreadRadius = int(dCellSpreadRadius);

		const float Atmax = float(dDamage * pWH->PercentAtMax);
		const auto vsData = pWH->GetVersesData(armor);

		if (Atmax != dDamage && cellSpreadRadius)
		{
			res = int((fDamage - Atmax) * (double)(cellSpreadRadius - distance) / (double)cellSpreadRadius + Atmax);
		}
		else
		{
			res = damage;
		}

		if (!pExt->ApplyModifiersOnNegativeDamage)
			res = int(double(res <= 0 ? 0 : res) * vsData->Verses);
		else
			res = int(res * vsData->Verses);

		/**
		 *	Allow damage to drop to zero only if the distance would have
		 *	reduced damage to less than 1/4 full damage. Otherwise, ensure
		 *	that at least one damage point is done.
		 */
		if (pExt->ApplyMindamage && distance < 4)
			damage = MaxImpl(damage, pExt->MinDamage >= 0 ? pExt->MinDamage : RulesClass::Instance->MinDamage);

		damage = MinImpl(damage, RulesClass::Instance->MaxDamage);

	}
	else
	{
		res = distance >= 8 ? 0 : damage;
	}

	return res;
}
void WarheadTypeExtData::InitializeConstant()
{
	this->AttachedEffect.Owner = this->This();
	this->EvaluateArmor(this->This());
}

//need to be initialize when first ini loading done
//since the CTOR is only creating the pointer , rules not yet loaded , cant evaluate there
void WarheadTypeExtData::Initialize()
{
	if (IS_SAME_STR_(RulesExtData::Instance()->NukeWarheadName.data(), this->This()->ID))
	{
		IsNukeWarhead = true;
		PreImpactAnim = AnimTypeClass::Find(GameStrings::NUKEBALL());
		NukeFlashDuration = 30;
	}
}

bool WarheadTypeExtData::IsVeterancyInThreshold(TechnoClass* pTarget) const {
	if (!this->VeterancyCheck)
		return true;

	return EnumFunctions::CanTargetVeterancy(this->AffectsVeterancy, pTarget);
}

bool WarheadTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = This();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	for (size_t i = Verses.size(); i < ArmorTypeClass::Array.size(); ++i)
	{
		auto& pArmor = ArmorTypeClass::Array[i];
		const int nDefaultIdx = pArmor->DefaultTo;
		Verses.push_back((nDefaultIdx == -1 || nDefaultIdx > (int)i)
				? pArmor->DefaultVersesValue
				: Verses[nDefaultIdx]
		);
	}

	if (parseFailAddr)
	{
		return false;
	}

	// writing custom verses parser just because
	if (exINI.ReadString(pSection, GameStrings::Verses()) > 0)
	{
		int idx = 0;
		char* context = nullptr;
		for (char* cur = strtok_s(exINI.value(), Phobos::readDelims, &context);
			cur;
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			this->Verses[idx].Parse_NoCheck(cur);

			if (++idx > 10)
			{
				break;
			}
		}
	}

	ArmorTypeClass::LoadForWarhead(pINI, pThis);

	//this will break targeting , so use it with caution !
	pThis->IsOrganic = exINI.ReadBool(pSection, "IsOrganic", &pThis->IsOrganic);

	// Miscs
	this->Reveal.Read(exINI, pSection, "Reveal");

	bool spySat;
	if (detail::read(spySat, exINI, pSection, GameStrings::SpySat()) && spySat)
		this->Reveal = -1;

	this->BigGap.Read(exINI, pSection, "BigGap");

	this->CreateGap.Read(exINI, pSection, "CreateGap");
	this->TransactMoney.Read(exINI, pSection, "TransactMoney");
	this->SplashList.Read(exINI, pSection, GameStrings::SplashList());
	this->SplashList_PickRandom.Read(exINI, pSection, "SplashList.PickRandom");
	this->SplashList_CreateAll.Read(exINI, pSection, "SplashList.CreateAll");
	this->SplashList_CreationInterval.Read(exINI, pSection, "SplashList.CreationInterval");
	this->SplashList_ScatterMin.Read(exINI, pSection, "SplashList.ScatterMin");
	this->SplashList_ScatterMax.Read(exINI, pSection, "SplashList.ScatterMax");

	this->RemoveDisguise.Read(exINI, pSection, "RemoveDisguise");
	this->RemoveMindControl.Read(exINI, pSection, "RemoveMindControl");
	this->AnimList_PickRandom.Read(exINI, pSection, "AnimList.PickRandom");
	this->AnimList_CreateAll.Read(exINI, pSection, "AnimList.CreateAll");
	this->AnimList_CreationInterval.Read(exINI, pSection, "AnimList.CreationInterval");
	this->AnimList_ScatterMin.Read(exINI, pSection, "AnimList.ScatterMin");
	this->AnimList_ScatterMax.Read(exINI, pSection, "AnimList.ScatterMax");
	this->AnimList_ShowOnZeroDamage.Read(exINI, pSection, "AnimList.ShowOnZeroDamage");
	this->DecloakDamagedTargets.Read(exINI, pSection, "DecloakDamagedTargets");
	this->ShakeIsLocal.Read(exINI, pSection, "ShakeIsLocal");
	this->Shake_UseAlternativeCalculation.Read(exINI, pSection, "Shake.UseAlternativeCalculation");

	// Crits
	this->Crit_Chance.Read(exINI, pSection, "Crit.Chance");
	this->Crit_ApplyChancePerTarget.Read(exINI, pSection, "Crit.ApplyChancePerTarget");
	this->Crit_ExtraDamage.Read(exINI, pSection, "Crit.ExtraDamage");
	this->Crit_ExtraDamage_ApplyFirepowerMult.Read(exINI, pSection, "Crit.ExtraDamage.ApplyFirepowerMult");
	this->Crit_Warhead.Read(exINI, pSection, "Crit.Warhead", true);
	this->Crit_Warhead_FullDetonation.Read(exINI, pSection, "Crit.Warhead.FullDetonation");
	this->Crit_Affects.Read(exINI, pSection, "Crit.Affects");
	this->Crit_Affects.Read(exINI, pSection, "Crit.AffectsTarget");
	this->Crit_AffectsHouses.Read(exINI, pSection, "Crit.AffectsHouses");
	this->Crit_AffectsHouses.Read(exINI, pSection, "Crit.AffectsHouse");
	this->Crit_AnimList.Read(exINI, pSection, "Crit.AnimList");
	this->Crit_AnimList_PickRandom.Read(exINI, pSection, "Crit.AnimList.PickRandom");
	this->Crit_AnimList_CreateAll.Read(exINI, pSection, "Crit.AnimList.CreateAll");
	this->Crit_ActiveChanceAnims.Read(exINI, pSection, "Crit.ActiveChanceAnims");
	this->Crit_AnimOnAffectedTargets.Read(exINI, pSection, "Crit.AnimOnAffectedTargets");
	this->Crit_AffectBelowPercent.Read(exINI, pSection, "Crit.AffectBelowPercent");
	this->Crit_AffectBelowPercent.Read(exINI, pSection, "Crit.AffectsBelowPercent");
	this->Crit_AffectAbovePercent.Read(exINI, pSection, "Crit.AffectAbovePercent");
	this->Crit_AffectAbovePercent.Read(exINI, pSection, "Crit.AffectsAbovePercent");
	this->Crit_SuppressWhenIntercepted.Read(exINI, pSection, "Crit.SuppressWhenIntercepted");

	this->MindControl_Anim.Read(exINI, pSection, "MindControl.Anim");

	// Ares tags
	// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
	this->AffectsEnemies.Read(exINI, pSection, "AffectsEnemies");
	this->AffectsOwner.Read(exINI, pSection, "AffectsOwner");
	this->EffectsRequireDamage.Read(exINI, pSection, "EffectsRequireDamage");
	this->EffectsRequireVerses.Read(exINI, pSection, "EffectsRequireVerses");
	this->AllowZeroDamage.Read(exINI, pSection, "AllowZeroDamage");

	// Shields

	this->Shield_Penetrate.Read(exINI, pSection, "PenetratesShield");
	this->Shield_Penetrate.Read(exINI, pSection, "Shield.Penetrate");

	this->Shield_RemoveAll.Read(exINI, pSection, "Shield.RemoveAll");

	this->Shield_Break.Read(exINI, pSection, "BreaksShield");
	this->Shield_Break.Read(exINI, pSection, "Shield.Break");

	this->Shield_BreakAnim.Read(exINI, pSection, "Shield.BreakAnim");
	this->Shield_HitAnim.Read(exINI, pSection, "Shield.HitAnim");
	this->Shield_BreakWeapon.Read(exINI, pSection, "Shield.BreakWeapon", true);
	this->Shield_AbsorbPercent.Read(exINI, pSection, "Shield.AbsorbPercent");
	this->Shield_PassPercent.Read(exINI, pSection, "Shield.PassPercent");
	this->Shield_ReceivedDamage_Minimum.Read(exINI, pSection, "Shield.ReceivedDamage.Minimum");
	this->Shield_ReceivedDamage_Maximum.Read(exINI, pSection, "Shield.ReceivedDamage.Maximum");
	this->Shield_ReceivedDamage_MinMultiplier.Read(exINI, pSection, "Shield.ReceivedDamage.MinMultiplier");
	this->Shield_ReceivedDamage_MaxMultiplier.Read(exINI, pSection, "Shield.ReceivedDamage.MaxMultiplier");
	this->Shield_Respawn_Duration.Read(exINI, pSection, "Shield.Respawn.Duration");
	this->Shield_Respawn_Amount.Read(exINI, pSection, "Shield.Respawn.Amount");
	this->Shield_Respawn_Rate_InMinutes.Read(exINI, pSection, "Shield.Respawn.Rate");
	this->Shield_Respawn_Rate = (int)(this->Shield_Respawn_Rate_InMinutes * 900);
	this->Shield_Respawn_RestartInCombat.Read(exINI, pSection, "Shield.Respawn.RestartInCombat");
	this->Shield_Respawn_RestartInCombatDelay.Read(exINI, pSection, "Shield.Respawn.RestartInCombatDelay");
	this->Shield_Respawn_RestartTimer.Read(exINI, pSection, "Shield.Respawn.RestartTimer");
	this->Shield_Respawn_Anim.Read(exINI, pSection, "Shield.Respawn.Anim");
	this->Shield_Respawn_Weapon.Read(exINI, pSection, "Shield.Respawn.Weapon");
	this->Shield_SelfHealing_Duration.Read(exINI, pSection, "Shield.SelfHealing.Duration");
	this->Shield_SelfHealing_Amount.Read(exINI, pSection, "Shield.SelfHealing.Amount");
	this->Shield_SelfHealing_Rate_InMinutes.Read(exINI, pSection, "Shield.SelfHealing.Rate");
	this->Shield_SelfHealing_Rate = (int)(this->Shield_SelfHealing_Rate_InMinutes * 900);
	this->Shield_SelfHealing_RestartInCombat.Read(exINI, pSection, "Shield.SelfHealing.RestartInCombat");
	this->Shield_SelfHealing_RestartInCombatDelay.Read(exINI, pSection, "Shield.SelfHealing.RestartInCombatDelay");
	this->Shield_SelfHealing_RestartTimer.Read(exINI, pSection, "Shield.SelfHealing.RestartTimer");
	this->Shield_AttachTypes.Read(exINI, pSection, "Shield.AttachTypes");
	this->Shield_RemoveTypes.Read(exINI, pSection, "Shield.RemoveTypes");
	this->Shield_ReplaceOnly.Read(exINI, pSection, "Shield.ReplaceOnly");
	this->Shield_ReplaceNonRespawning.Read(exINI, pSection, "Shield.ReplaceNonRespawning");
	this->Shield_InheritStateOnReplace.Read(exINI, pSection, "Shield.InheritStateOnReplace");
	this->Shield_MinimumReplaceDelay.Read(exINI, pSection, "Shield.MinimumReplaceDelay");
	this->Shield_AffectTypes.Read(exINI, pSection, "Shield.AffectTypes");

	this->Shield_Penetrate_Types.Read(exINI, pSection, "Shield.Penetrate.Types");
	this->Shield_Penetrate_Types_Disallowed_Types.Read(exINI, pSection, "Shield.Penetrate.Disallow.Types");
	this->Shield_Penetrate_Armor_Types.Read(exINI,pSection, "Shield.Penetrates.ArmorTypes");
	this->Shield_Break_Types.Read(exINI, pSection, "Shield.Break.Types");
	this->Shield_Respawn_Types.Read(exINI, pSection, "Shield.Respawn.Types");
	this->Shield_SelfHealing_Types.Read(exINI, pSection, "Shield.SelfHealing.Types");

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
	this->DebrisTypes_Limit.Read(exINI, pSection, "DebrisTypes.Limit");
	this->DebrisMinimums.Read(exINI, pSection, "DebrisMinimums");

	this->GattlingStage.Read(exINI, pSection, "TargetGattlingStage");
	this->GattlingRateUp.Read(exINI, pSection, "TargetGattlingRateUp");
	this->ReloadAmmo.Read(exINI, pSection, "TargetReloadAmmo");

	this->MindControl_UseTreshold.Read(exINI, pSection, "MindControl.UseThreshold");
	this->MindControl_Threshold.Read(exINI, pSection, "MindControl.Threshold");
	this->MindControl_Threshold_Inverse.Read(exINI, pSection, "MindControl.Threshold.Inverse");
	this->MindControl_AlternateDamage.Read(exINI, pSection, "MindControl.AlternateDamage");
	this->MindControl_AlternateWarhead.Read(exINI, pSection, "MindControl.AlternateWarhead", true);
	this->MindControl_CanKill.Read(exINI, pSection, "MindControl.CanKill");

	this->DetonateOnAllMapObjects.Read(exINI, pSection, "DetonateOnAllMapObjects");
	this->DetonateOnAllMapObjects_Full.Read(exINI, pSection, "DetonateOnAllMapObjects.Full");
	this->DetonateOnAllMapObjects_RequireVerses.Read(exINI, pSection, "DetonateOnAllMapObjects.RequireVerses");
	this->DetonateOnAllMapObjects_AffectTargets.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectTargets");
	this->DetonateOnAllMapObjects_AffectHouses.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectHouses");
	this->DetonateOnAllMapObjects_AffectTypes.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectTypes");
	this->DetonateOnAllMapObjects_IgnoreTypes.Read(exINI, pSection, "DetonateOnAllMapObjects.IgnoreTypes");

	this->RevengeWeapon.Read(exINI, pSection, "RevengeWeapon", true);
	this->RevengeWeapon_GrantDuration.Read(exINI, pSection, "RevengeWeapon.GrantDuration");
	this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouses");
	this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouse");
	this->RevengeWeapon_Cumulative.Read(exINI, pSection, "RevengeWeapon.Cumulative");
	this->RevengeWeapon_MaxCount.Read(exINI, pSection, "RevengeWeapon.MaxCount");

#pragma region Otamaa

	this->SquidSplash.Read(exINI, pSection, "Parasite.SplashAnims");
	this->TemporalExpiredAnim.Read(exINI, pSection, "Temporal.ExpiredAnim");
	this->TemporalDetachDamageFactor.Read(exINI, pSection, "Temporal.ExpiredDamageFactor");
	this->TemporalExpiredApplyDamage.Read(exINI, pSection, "Temporal.ExpiredApplyDamage");
	this->DebrisAnimTypes.Read(exINI, pSection, "DebrisAnims");
	this->Flammability.Read(exINI, pSection, "FlameChance");

	this->Parasite_DisableRocking.Read(exINI, pSection, "Parasite.DisableRocking");
	this->Parasite_GrappleAnim.Read(exINI, pSection, "Parasite.GrappleAnim");
	this->Parasite_ParticleSys.Read(exINI, pSection, "Parasite.ParticleSystem");
	this->Parasite_TreatInfantryAsVehicle.Read(exINI, pSection, "Parasite.TreatInfantryAsVehicle");
	this->Parasite_InvestationWP.Read(exINI, pSection, "Parasite.DamagingWeapon");
	this->Parasite_Damaging_Chance.Read(exINI, pSection, "Parasite.DamagingChance");

	std::vector<AnimTypeClass*> hitAnim {};
	this->ArmorHitAnim.clear();
	hitAnim.reserve(ArmorTypeClass::Array.size());

	for (auto const& ArmorType : ArmorTypeClass::Array)
	{
		AnimTypeClass* pAnimReaded = nullptr;
		detail::read(pAnimReaded, exINI, pSection, ArmorType->HitAnim_Tag.c_str(), true);
		hitAnim.push_back(pAnimReaded);
	}

	for (size_t i = 0; i < hitAnim.size(); ++i)
	{
		if (!hitAnim[i] && ArmorTypeClass::Array[i]->DefaultTo != -1)
		{
			for (auto pDefArmor = ArmorTypeClass::Array[ArmorTypeClass::Array[i]->DefaultTo].get();
				pDefArmor != ArmorTypeClass::Array[ArmorTypeClass::Array.size()].get();
				pDefArmor = ArmorTypeClass::Array[pDefArmor->DefaultTo].get())
			{

				if (auto pFallback = hitAnim[ArmorTypeClass::Array[i]->DefaultTo])
					hitAnim[i] = pFallback;

				if (pDefArmor->DefaultTo == -1)
					break;
			}
		}
	}

	for (size_t a = 0; a < hitAnim.size(); ++a)
	{
		if (hitAnim[a])
		{
			this->ArmorHitAnim.emplace_unchecked(ArmorTypeClass::Array[a].get(), hitAnim[a]);
		}
	}

	this->IsNukeWarhead.Read(exINI, pSection, "IsNukeWarhead");
	this->PreImpactAnim.Read(exINI, pSection, "PreImpactAnim", true);
	this->NukeFlashDuration.Read(exINI, pSection, "NukeFlash.Duration");

	this->Remover.Read(exINI, pSection, "Remover");
	this->Remover_Anim.Read(exINI, pSection, "Remover.Anim");
	this->PermaMC.Read(exINI, pSection, "MindControl.Permanent");
	this->Sound.Read(exINI, pSection, GameStrings::Sound());


	TechnoTypeConvertData::Parse(Phobos::Otamaa::CompatibilityMode, this->ConvertsPair, exINI, pSection, "ConvertsPair");
	this->Convert_SucceededAnim.Read(exINI, pSection, "ConvertsAnim");

	this->AffectEnemies_Damage_Mod.Read(exINI, pSection, "AffectEnemies.DamageModifier");
	this->AffectOwner_Damage_Mod.Read(exINI, pSection, "AffectOwner.DamageModifier");
	this->AffectAlly_Damage_Mod.Read(exINI, pSection, "AffectAlly.DamageModifier");

	this->DamageOwnerMultiplier.Read(exINI, pSection, "DamageOwnerMultiplier");
	this->DamageAlliesMultiplier.Read(exINI, pSection, "DamageAlliesMultiplier");
	this->DamageEnemiesMultiplier.Read(exINI, pSection, "DamageEnemiesMultiplier");
	this->DamageOwnerMultiplier_Berzerk.Read(exINI, pSection, "DamageOwnerMultiplier.Berzerk");
	this->DamageAlliesMultiplier_Berzerk.Read(exINI, pSection, "DamageAlliesMultiplier.Berzerk");
	this->DamageEnemiesMultiplier_Berzerk.Read(exINI, pSection, "DamageEnemiesMultiplier.Berzerk");
	this->AttachTag.Read(pINI, pSection, "AttachTag");
	this->AttachTag_Imposed.Read(exINI, pSection, "AttachTag.Imposed");
	this->AttachTag_Types.Read(exINI, pSection, "AttachTag.Types");
	this->AttachTag_Ignore.Read(exINI, pSection, "AttachTag.Ignore");

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

	this->Berzerk_dur.Read(exINI, pSection, "Berzerk.Duration");
	this->Berzerk_cap.Read(exINI, pSection, "Berzerk.Cap");
	this->Berzerk_dealDamage.Read(exINI, pSection, "Berzerk.DealDamage");
	this->IC_Flash.Read(exINI, pSection, "IronCurtain.Flash");

	this->PreventScatter.Read(exINI, pSection, "PreventScatter");

	this->DieSound_Override.Read(exINI, pSection, "DieSound.Override");
	this->VoiceSound_Override.Read(exINI, pSection, "VoiceSound.Override");

	this->SuppressDeathWeapon_Vehicles.Read(exINI, pSection, "DeathWeapon.SuppressVehicles");
	this->SuppressDeathWeapon_Infantry.Read(exINI, pSection, "DeathWeapon.SuppressInfantry");
	this->SuppressDeathWeapon.Read(exINI, pSection, "DeathWeapon.Suppress");
	this->SuppressDeathWeapon_Exclude.Read(exINI, pSection, "DeathWeapon.SuppressExclude");
	this->SuppressDeathWeapon_Chance.Read(exINI, pSection, "DeathWeapon.SuppressChance");

	this->DeployedDamage.Read(exINI, pSection, "Damage.Deployed");
	this->Temporal_WarpAway.Read(exINI, pSection, "Temporal.WarpAway");
	this->Supress_LostEva.Read(exINI, pSection, "UnitLost.Suppress");
	this->Temporal_HealthFactor.Read(exINI, pSection, "Temporal.HealthFactor");

	this->PaintBallDuration.Read(exINI, pSection, "PaintBall.Duration");
	this->PaintBallData.Read(exINI, pSection);
#pragma endregion

#ifndef _Handle_Old_
	this->AttachedEffect.Read(exINI);
#else
	int _AE_Dur { 0 };
	this->AttachEffect_AttachTypes.clear();
	if (detail::read(_AE_Dur, exINI, pSection, "AttachEffect.Duration") && _AE_Dur != 0)
	{
		auto& back = this->AttachEffect_AttachTypes.emplace_back(PhobosAttachEffectTypeClass::FindOrAllocate(pSection));
		back->Duration = _AE_Dur;
		back->Cumulative.Read(exINI, pSection, "AttachEffect.Cumulative");
		back->Animation.Read(exINI, pSection, "AttachEffect.Animation", true);
		back->Animation_ResetOnReapply.Read(exINI, pSection, "AttachEffect.AnimResetOnReapply");

		bool AE_TemporalHidesAnim {};
		if (detail::read(AE_TemporalHidesAnim, exINI, pSection, "AttachEffect.TemporalHidesAnim") && AE_TemporalHidesAnim)
			back->Animation_TemporalAction = AttachedAnimFlag::Hides;

		back->ForceDecloak.Read(exINI, pSection, "AttachEffect.ForceDecloak");

		bool AE_DiscardOnEntry {};
		if (detail::read(AE_DiscardOnEntry, exINI, pSection, "AttachEffect.DiscardOnEntry") && AE_DiscardOnEntry)
			back->DiscardOn = DiscardCondition::Entry;

		back->FirepowerMultiplier.Read(exINI, pSection, "AttachEffect.FirepowerMultiplier");
		back->ArmorMultiplier.Read(exINI, pSection, "AttachEffect.ArmorMultiplier");
		back->SpeedMultiplier.Read(exINI, pSection, "AttachEffect.SpeedMultiplier");
		back->ROFMultiplier.Read(exINI, pSection, "AttachEffect.ROFMultiplier");
		back->ReceiveRelativeDamageMult.Read(exINI, pSection, "AttachEffect.ReceiveRelativeDamageMultiplier");
		back->Cloakable.Read(exINI, pSection, "AttachEffect.Cloakable");

		back->PenetratesIronCurtain.Read(exINI, pSection, "AttachEffect.PenetratesIronCurtain");
		back->DisableSelfHeal.Read(exINI, pSection, "AttachEffect.DisableSelfHeal");
		back->DisableWeapons.Read(exINI, pSection, "AttachEffect.DisableWeapons");
		back->Untrackable.Read(exINI, pSection, "AttachEffect.Untrackable");

		back->WeaponRange_Multiplier.Read(exINI, pSection, "AttachEffect.WeaponRange.Multiplier");
		back->WeaponRange_ExtraRange.Read(exINI, pSection, "AttachEffect.WeaponRange.ExtraRange");
		back->WeaponRange_AllowWeapons.Read(exINI, pSection, "AttachEffect.WeaponRange.AllowWeapons");
		back->WeaponRange_DisallowWeapons.Read(exINI, pSection, "AttachEffect.WeaponRange.DisallowWeapons");

		back->ROFMultiplier_ApplyOnCurrentTimer.Read(exINI, pSection, "AttachEffect.ROFMultiplier.ApplyOnCurrentTimer");
	}
#endif

	this->DetonatesWeapons.Read(exINI, pSection, "DetonatesWeapons");
	this->LimboKill_IDs.Read(exINI, pSection, "LimboKill.IDs");
	this->LimboKill_Affected.Read(exINI, pSection, "LimboKill.Affected");
	this->LimboKill_Affected.Read(exINI, pSection, "LimboKill.AffectsHouse");
	this->InfDeathAnim.Read(exINI, pSection, "InfDeathAnim");
	this->Culling_BelowHP.Read(exINI, pSection, "Culling.%sBelowHealth");
	this->Culling_Chance.Read(exINI, pSection, "Culling.%sChance");
	this->Culling_Target.Read(exINI, pSection, "Culling.Target");

	this->RelativeDamage.Read(exINI, pSection, "RelativeDamage");
	this->RelativeDamage_AirCraft.Read(exINI, pSection, "RelativeDamage.Aircraft");
	this->RelativeDamage_Unit.Read(exINI, pSection, "RelativeDamage.Vehicles");
	this->RelativeDamage_Infantry.Read(exINI, pSection, "RelativeDamage.Infantry");
	this->RelativeDamage_Building.Read(exINI, pSection, "RelativeDamage.Buildings");
	this->RelativeDamage_Terrain.Read(exINI, pSection, "RelativeDamage.Terrain");

	this->Sonar_Duration.Read(exINI, pSection, "Sonar.Duration");
	this->DisableWeapons_Duration.Read(exINI, pSection, "DisableWeapons.Duration");
	this->Flash_Duration.Read(exINI, pSection, "Flash.Duration");
	this->ImmunityType.Read(exINI, pSection, "ImmunityType");
	this->Malicious.Read(exINI, pSection, "Malicious");
	this->PreImpact_Moves.Read(exINI, pSection, "PreImpactAnim.Moves");

	LauchSWData::ReadVector(this->Launchs, exINI , pSection, Phobos::Otamaa::CompatibilityMode);

	this->Conventional_IgnoreUnits.Read(exINI, pSection, "Conventional.IgnoreUnits");

	this->InflictLocomotor.Read(exINI, pSection, "InflictLocomotor");
	this->RemoveInflictedLocomotor.Read(exINI, pSection, "RemoveInflictedLocomotor");
	this->Rocker_AmplitudeMultiplier.Read(exINI, pSection, "Rocker.AmplitudeMultiplier");
	this->Rocker_AmplitudeOverride.Read(exINI, pSection, "Rocker.AmplitudeOverride");
	this->NukePayload_LinkedSW.Read(exINI, pSection, "NukeMaker.LinkedSW");
	this->IC_Duration.Read(exINI, pSection, "IronCurtain.Duration");
	this->IC_Cap.Read(exINI, pSection, "IronCurtain.Cap");

#pragma region Ion
	this->Ion.Read(exINI, pSection, "IonCannon");

	int Ion_ripple;
	if (detail::read(Ion_ripple, exINI, pSection, "Ripple.Radius"))
		this->Ripple_Radius = Ion_ripple;
	else
		this->Ripple_Radius.Read(exINI, pSection, "IonCannon.Ripple");

	this->Ion_Beam.Read(exINI, pSection, "IonCannon.Beam");
	this->Ion_Blast.Read(exINI, pSection, "IonCannon.Blast");
	this->Ion_AllowWater.Read(exINI, pSection, "IonCannon.AllowWater");
	this->Ion_Rocking.Read(exINI, pSection, "IonCannon.Rock");
	this->Ion_WH.Read(exINI, pSection, "IonCannon.Warhead");
	this->Ion_Damage.Read(exINI, pSection, "IonCannon.Damage");

	this->DetonateParticleSystem.Read(exINI, pSection, "DetonateParticleSystems");
	this->BridgeAbsoluteDestroyer.Read(exINI, pSection, "BridgeAbsoluteDestroyer");
	this->CellSpread_MaxAffect.Read(exINI, pSection, "CellSpread.MaxAffect");
	this->DamageAirThreshold.Read(exINI, pSection, "DamageAirThreshold");

	this->EMP_Duration.Read(exINI, pSection, "EMP.Duration");
	this->EMP_Cap.Read(exINI, pSection, "EMP.Cap");
	this->EMP_Sparkles.Read(exINI, pSection, "EMP.Sparkles");

	this->CanRemoveParasytes.Read(exINI, pSection, "CanRemoveParasytes");
	this->CanRemoveParasytes_KickOut.Read(exINI, pSection, "CanRemoveParasytes.KickOut");
	this->CanRemoveParasytes_KickOut_Paralysis.Read(exINI, pSection, "CanRemoveParasytes.KickOut.Paralysis");
	this->CanRemoveParasytes_ReportSound.Read(exINI, pSection, "CanRemoveParasytes.ReportSound");
	this->CanRemoveParasytes_KickOut_Anim.Read(exINI, pSection, "CanRemoveParasytes.KickOut.Anim");

	this->Webby.Read(exINI, pSection, "Webby");
	this->Webby_Anims.Read(exINI, pSection, "Webby.Anims");
	this->Webby_Duration.Read(exINI, pSection, "Webby.Duration");
	this->Webby_Cap.Read(exINI, pSection, "Webby.Cap");
	this->Webby_Duration_Variation.Read(exINI, pSection, "Webby.DurationVariation");

	this->SelfHealing_CombatDelay.Read(exINI, pSection, "VictimSelfHealing.%sCombatDelay");
	this->ApplyModifiersOnNegativeDamage.Read(exINI, pSection, "ApplyModifiersOnNegativeDamage");

	this->KillDriver.Read(exINI, pSection, "KillDriver");
	this->KillDriver_KillBelowPercent.Read(exINI, pSection, "KillDriver.KillBelowPercent");
	this->KillDriver_Owner.Read(exINI, pSection, "KillDriver.Owner");
	this->KillDriver_ResetVeterancy.Read(exINI, pSection, "KillDriver.RemoveVeterancy");
	this->KillDriver_Chance.Read(exINI, pSection, "KillDriver.Chance");

	this->CombatLightDetailLevel.Read(exINI, pSection, "CombatLightDetailLevel");
	this->CombatLightChance.Read(exINI, pSection, "CombatLightChance");
	this->Particle_AlphaImageIsLightFlash.Read(exINI, pSection, "Particle.AlphaImageIsLightFlash");

	this->Nonprovocative.Read(exINI, pSection, "Nonprovocative");

	// AttachEffect
	this->PhobosAttachEffects.LoadFromINI(pINI, pSection);
	this->Shield_HitFlash.Read(exINI, pSection, "Shield.HitFlash");
	this->Shield_SkipHitAnim.Read(exINI, pSection, "Shield.SkipHitAnim");
	this->CombatAlert_Suppress.Read(exINI, pSection, "CombatAlert.Suppress");

	this->AffectsGround.Read(exINI, pSection, "AffectsGround");
	this->AffectsInAir.Read(exINI, pSection, "AffectsAir");
	this->CellSpread_Cylinder.Read(exINI, pSection, "CellSpread.Cylinder");

	this->DamageSourceHealthMultiplier.Read(exINI, pSection, "DamageSourceHealthMultiplier");
	this->DamageTargetHealthMultiplier.Read(exINI, pSection, "DamageTargetHealthMultiplier");

	ValueableVector<InfantryTypeClass*> InfDeathAnims_List {};

	InfDeathAnims_List.Read(exINI, pSection, "InfDeathAnim.LinkedList");

	if (!InfDeathAnims_List.empty())
	{
		this->InfDeathAnims.reserve(InfDeathAnims_List.size());
		for (size_t i = 0; i < InfDeathAnims_List.size(); ++i)
		{
			detail::read(this->InfDeathAnims[InfDeathAnims_List[i]], exINI, pSection, (std::string("InfDeathAnim") + std::to_string(i)).c_str());
		}
	}

#pragma endregion

	//if (this->InflictLocomotor && pThis->Locomotor == _GUID())
	//{
	//	Debug::LogInfo("[Developer warning][%s] InflictLocomotor is specified but Locomotor is not set!", pSection);
	//	this->InflictLocomotor = false;
	//}
	//
	//if ((this->InflictLocomotor || this->RemoveInflictedLocomotor) && pThis->IsLocomotor)
	//{
	//	Debug::LogInfo("[Developer warning][%s] InflictLocomotor=yes/RemoveInflictedLocomotor=yes can't be specified while IsLocomotor is set!", pSection);
	//	this->InflictLocomotor = this->RemoveInflictedLocomotor = false;
	//}
	//
	//if (this->InflictLocomotor && this->RemoveInflictedLocomotor)
	//{
	//	Debug::LogInfo("[Developer warning][%s] InflictLocomotor=yes and RemoveInflictedLocomotor=yes can't be set simultaneously!", pSection);
	//	this->InflictLocomotor = this->RemoveInflictedLocomotor = false;
	//}

	this->SpawnsCrate_Types.clear();
	this->SpawnsCrate_Weights.clear();

	for (size_t i = 0; ; i++)
	{

		std::string base("SpawnsCrate");
		std::string base_Num = base + std::to_string(i);

		NullableIdx<CrateTypeClass> Idx;
		Idx.Read(exINI, pSection, (base_Num + ".Type").c_str());

		if (i == 0 && !Idx.isset())
			Idx.Read(exINI, pSection, (base + ".Type").c_str());

		if (!Idx.isset() || Idx == -1)
			break;

		Nullable<int> weight;
		weight.Read(exINI, pSection, (base_Num + ".Weight").c_str());

		if (i == 0 && !weight.isset())
			weight.Read(exINI, pSection, (base + ".Weight").c_str());

		if (!weight.isset())
			weight = 1;

		this->SpawnsCrate_Types.emplace_back(Idx);
		this->SpawnsCrate_Weights.emplace_back(weight);
	}

	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");
	this->PenetratesForceShield.Read(exINI, pSection, "PenetratesForceShield");


	this->SuppressRevengeWeapons.Read(exINI, pSection, "SuppressRevengeWeapons");
	this->SuppressRevengeWeapons_Types.Read(exINI, pSection, "SuppressRevengeWeapons.Types");
	this->SuppressReflectDamage.Read(exINI, pSection, "SuppressReflectDamage");
	this->SuppressReflectDamage_Types.Read(exINI, pSection, "SuppressReflectDamage.Types");
	this->SuppressReflectDamage_Groups.Read(exINI, pSection, "SuppressReflectDamage.Groups");

	this->RemoveParasites.Read(exINI, pSection, "RemoveParasites");
	this->CLIsBlack.Read(exINI, pSection, "CLIsBlack");
	this->ApplyMindamage.Read(exINI, pSection, "ApplyMinDamage");
	this->MinDamage.Read(exINI, pSection, "MinDamage");

	this->KillWeapon.Read(exINI, pSection, "KillWeapon");
	this->KillWeapon_OnFirer.Read(exINI, pSection, "KillWeapon.OnFirer");
	this->KillWeapon_AffectsHouses.Read(exINI, pSection, "KillWeapon.AffectsHouses");
	this->KillWeapon_AffectsHouses.Read(exINI, pSection, "KillWeapon.AffectsHouse");
	this->KillWeapon_OnFirer_AffectsHouses.Read(exINI, pSection, "KillWeapon.OnFirer.AffectsHouses");
	this->KillWeapon_OnFirer_AffectsHouses.Read(exINI, pSection, "KillWeapon.OnFirer.AffectsHouse");
	this->KillWeapon_Affects.Read(exINI, pSection, "KillWeapon.Affects");
	this->KillWeapon_Affects.Read(exINI, pSection, "KillWeapon.AffectsTarget");
	this->KillWeapon_OnFirer_Affects.Read(exINI, pSection, "KillWeapon.OnFirer.Affects");
	this->KillWeapon_OnFirer_Affects.Read(exINI, pSection, "KillWeapon.OnFirer.AffectsTarget");

	this->MindControl_ThreatDelay.Read(exINI, pSection, "MindControl.ThreatDelay");
	this->MergeBuildingDamage.Read(exINI, pSection, "MergeBuildingDamage");

	this->BuildingSell.Read(exINI, pSection, "BuildingSell");
	this->BuildingSell_IgnoreUnsellable.Read(exINI, pSection, "BuildingSell.IgnoreUnsellable");
	this->BuildingUndeploy.Read(exINI, pSection, "BuildingUndeploy");
	this->BuildingUndeploy_Leave.Read(exINI, pSection, "BuildingUndeploy.Leave");

	this->ScorchChance.Read(exINI, pSection, "ScorchChance");
	this->CraterChance.Read(exINI, pSection, "CraterChance");
	this->CellAnimChance.Read(exINI, pSection, "CellAnimChance");

	this->ScorchPercentAtMax.Read(exINI, pSection, "ScorchPercentAtMax");
	this->CraterPercentAtMax.Read(exINI, pSection, "CraterPercentAtMax");
	this->CellAnimPercentAtMax.Read(exINI, pSection, "CellAnimPercentAtMax");

	this->CellAnim.Read(exINI, pSection, "CellAnim");
	this->ElectricAssaultLevel.Read(exINI, pSection, "ElectricAssaultLevel");
	this->AirstrikeTargets.Read(exINI, pSection, "AirstrikeTargets");
	this->CanKill.Read(exINI, pSection, "CanKill");

	this->AffectsBelowPercent.Read(exINI, pSection, "AffectsBelowPercent");
	this->AffectsAbovePercent.Read(exINI, pSection, "AffectsAbovePercent");
	this->AffectsVeterancy.Read(exINI, pSection, "AffectsVeterancy");
	this->AffectsNeutral.Read(exINI, pSection, "AffectsNeutral");

	this->ElectricAssault_Requireverses.Read(exINI, pSection, "ElectricAssault.Requireverses");

	this->PenetratesTransport_Level.Read(exINI, pSection, "PenetratesTransport.Level");
	this->PenetratesTransport_PassThrough.Read(exINI, pSection, "PenetratesTransport.PassThrough");
	this->PenetratesTransport_FatalRate.Read(exINI, pSection, "PenetratesTransport.FatalRate");
	this->PenetratesTransport_DamageMultiplier.Read(exINI, pSection, "PenetratesTransport.DamageMultiplier");
	this->PenetratesTransport_DamageAll.Read(exINI, pSection, "PenetratesTransport.DamageAll");
	this->PenetratesTransport_CleanSound.Read(exINI, pSection, "PenetratesTransport.CleanSound");

	this->FakeEngineer_CanRepairBridges.Read(exINI, pSection, "FakeEngineer.CanRepairBridges");
	this->FakeEngineer_CanDestroyBridges.Read(exINI, pSection, "FakeEngineer.CanDestroyBridges");
	this->FakeEngineer_CanCaptureBuildings.Read(exINI, pSection, "FakeEngineer.CanCaptureBuildings");
	this->FakeEngineer_BombDisarm.Read(exINI, pSection, "FakeEngineer.BombDisarm");

	this->UnlimboDetonate.Read(exINI, pSection, "UnlimboDetonate");
	this->UnlimboDetonate_Force.Read(exINI, pSection, "UnlimboDetonate.ForceLocation");
	this->UnlimboDetonate_KeepTarget.Read(exINI, pSection, "UnlimboDetonate.KeepTarget");
	this->UnlimboDetonate_KeepSelected.Read(exINI, pSection, "UnlimboDetonate.KeepSelected");

	this->ReverseEngineer.Read(exINI, pSection, "ReverseEngineer");

	this->Block_BasedOnWarhead.Read(exINI, pSection, "Block.BasedOnWarhead");
	this->Block_AllowOverride.Read(exINI, pSection, "Block.AllowOverride");
	this->Block_IgnoreChanceModifier.Read(exINI, pSection, "Block.IgnoreChanceModifier");
	this->Block_ChanceMultiplier.Read(exINI, pSection, "Block.ChanceMultiplier");
	this->Block_ExtraChance.Read(exINI, pSection, "Block.ExtraChance");
	this->ImmuneToBlock.Read(exINI, pSection, "ImmuneToBlock");
	this->AffectsUnderground.Read(exINI, pSection, "AffectsUnderground");
	this->PlayAnimUnderground.Read(exINI, pSection, "PlayAnimUnderground");
	this->PlayAnimAboveSurface.Read(exINI, pSection, "PlayAnimAboveSurface");

	if (!this->BlockType)
		this->BlockType = std::make_unique<BlockTypeClass>();

	this->BlockType->LoadFromINI(pINI, pSection);
	this->AnimZAdjust.Read(exINI, pSection, "AnimZAdjust");
	this->ApplyPerTargetEffectsOnDetonate.Read(exINI, pSection, "ApplyPerTargetEffectsOnDetonate");

	// Return warhead
	this->ReturnWarhead.Read(exINI, pSection, "ReturnWarhead");
	this->ReturnWarhead_Damage.Read(exINI, pSection, "ReturnWarhead.Damage");
	this->ReturnWarhead_Chance.Read(exINI, pSection, "ReturnWarhead.Chance");
	this->ReturnWarhead_ApplyChancePerTarget.Read(exINI, pSection, "ReturnWarhead.ApplyChancePerTarget");
	this->ReturnWarhead_FullDetonation.Read(exINI, pSection, "ReturnWarhead.FullDetonation");
	this->ReturnWarhead_AffectsTarget.Read(exINI, pSection, "ReturnWarhead.AffectsTarget");
	this->ReturnWarhead_AffectsHouse.Read(exINI, pSection, "ReturnWarhead.AffectsHouse");

	this->VeterancyCheck = this->AffectsVeterancy != AffectedVeterancy::All;

	this->IsCellSpreadWH =
		this->RemoveDisguise ||
		this->RemoveMindControl ||
		//this->Crit_Chance ||
		this->Shield_Break ||
		!this->ConvertsPair.empty() ||
		this->Shield_Respawn_Duration > 0 ||
		this->Shield_SelfHealing_Duration > 0 ||
		!this->Shield_AttachTypes.empty() ||
		!this->Shield_RemoveTypes.empty() ||
		this->Shield_RemoveAll ||
		this->Transact ||
		this->PermaMC ||
		this->GattlingStage > 0 ||
		this->GattlingRateUp != 0 ||
		this->AttachTag ||
		//this->DirectionalArmor ||
		this->ReloadAmmo != 0
		|| (this->RevengeWeapon && this->RevengeWeapon_GrantDuration > 0)
		|| !this->LimboKill_IDs.empty()
		|| (this->PaintBallData.Color != ColorStruct::Empty)
		//|| this->InflictLocomotor
		//|| this->RemoveInflictedLocomotor
		|| this->PenetratesTransport_Level > 0
		|| this->IC_Duration != 0

		|| !this->PhobosAttachEffects.AttachTypes.empty()
		|| !this->PhobosAttachEffects.RemoveTypes.empty()
		|| !this->PhobosAttachEffects.RemoveGroups.empty()
		|| this->BuildingSell
		|| this->BuildingUndeploy
		|| this->ReverseEngineer
		|| this->ReturnWarhead
		;

	this->IsFakeEngineer =
		this->FakeEngineer_CanRepairBridges ||
		this->FakeEngineer_CanDestroyBridges ||
		this->FakeEngineer_CanCaptureBuildings ||
		this->FakeEngineer_BombDisarm;

	return true;
}

//https://github.com/Phobos-developers/Phobos/issues/629
void WarheadTypeExtData::ApplyDamageMult(TechnoClass* pVictim, TechnoClass* pSource, HouseClass* pSourceHouse,  int* pDamage) const
{
	//auto const pExt = TechnoExtContainer::Instance.Find(pVictim);

	// AffectsAbove/BelowPercent & AffectsNeutral can ignore IgnoreDefenses like AffectsAllies/Enmies/Owner
	// They should be checked here to cover all cases that directly use ReceiveDamage to deal damage
	if (!this->IsHealthInThreshold(pVictim) || !this->IsVeterancyInThreshold(pVictim) || (!this->AffectsNeutral && pVictim->Owner->IsNeutral())) {
		*pDamage = 0;
		return;
	}

	// if (pExt->ReceiveDamageMultiplier.isset())
	// {
	// 	*pArgs->Damage = static_cast<int>(*pArgs->Damage * pExt->ReceiveDamageMultiplier.get());
	// 	pExt->ReceiveDamageMultiplier.clear();
	// }

	auto const& nAllyMod = AffectAlly_Damage_Mod;
	auto const& nOwnerMod = AffectOwner_Damage_Mod;
	auto const& nEnemyMod = AffectEnemies_Damage_Mod;
	auto const pVictimHouse = pVictim->Owner;

	if ((nAllyMod.isset() && nOwnerMod.isset() && nEnemyMod.isset()))
	{
		auto const pHouse = pSourceHouse ? pSourceHouse :pSource ? pSource->Owner : HouseExtData::FindFirstCivilianHouse();

		if (pHouse && pVictimHouse)
		{
			auto const pWH = This();
			const int nDamage = *pDamage;

			if (pVictimHouse != pHouse)
			{
				if (pVictimHouse->IsAlliedWith(pHouse) && pWH->AffectsAllies && nAllyMod.isset())
				{
					*pDamage = static_cast<int>(nDamage * nAllyMod.Get());
				}
				else if (AffectsEnemies.Get() && nEnemyMod.isset())
				{
					*pDamage = static_cast<int>(nDamage * nEnemyMod.Get());
				}
			}
			else if (AffectsOwner.Get() && nOwnerMod.isset())
			{
				*pDamage = static_cast<int>(nDamage * nOwnerMod.Get());
			}
		}
	}

	//Calculate Damage Multiplier
	//this abomination is always active
	const auto pRulesExt = RulesExtData::Instance();
	double multiplier = 1.0;

	if (pSource && pSource->Berzerk)
	{
		if (!pSourceHouse || !pVictimHouse || !pSourceHouse->IsAlliedWith(pVictimHouse))
			multiplier = this->DamageEnemiesMultiplier_Berzerk.Get(pRulesExt->DamageEnemiesMultiplier_Berzerk.Get(pRulesExt->DamageEnemiesMultiplier));
		else if (pSourceHouse != pVictimHouse)
			multiplier = this->DamageAlliesMultiplier_Berzerk.Get(pRulesExt->DamageAlliesMultiplier_Berzerk.Get(!this->AffectsEnemies ? pRulesExt->DamageAlliesMultiplier_NotAffectsEnemies.Get(pRulesExt->DamageAlliesMultiplier) : pRulesExt->DamageAlliesMultiplier));
		else
			multiplier = this->DamageOwnerMultiplier_Berzerk.Get(pRulesExt->DamageOwnerMultiplier_Berzerk.Get(!this->AffectsEnemies ? pRulesExt->DamageOwnerMultiplier_NotAffectsEnemies.Get(pRulesExt->DamageOwnerMultiplier) : pRulesExt->DamageOwnerMultiplier));
	}
	else
	{
		if (!pSourceHouse || !pVictimHouse || !pSourceHouse->IsAlliedWith(pVictimHouse))
			multiplier = this->DamageEnemiesMultiplier.Get(pRulesExt->DamageEnemiesMultiplier);
		else if (pSourceHouse != pVictimHouse)
			multiplier = this->DamageAlliesMultiplier.Get(!this->AffectsEnemies ? pRulesExt->DamageAlliesMultiplier_NotAffectsEnemies.Get(pRulesExt->DamageAlliesMultiplier) : pRulesExt->DamageAlliesMultiplier);
		else
			multiplier = this->DamageOwnerMultiplier.Get(!this->AffectsEnemies ? pRulesExt->DamageOwnerMultiplier_NotAffectsEnemies.Get(pRulesExt->DamageOwnerMultiplier) : pRulesExt->DamageOwnerMultiplier);
	}

	if (multiplier != 1.0) {
		const auto sgnDamage = *pDamage > 0 ? 1 : -1;
		const auto calculateDamage = static_cast<int>(*pDamage * multiplier);
		*pDamage = calculateDamage ? calculateDamage : sgnDamage;
	}
}

void WarheadTypeExtData::EvaluateArmor(WarheadTypeClass* OwnerObject)
{
	//const auto nArmorArrSize = ArmorTypeClass::Array.size();
	Verses.resize(Unsorted::ArmorNameArray.size());

	//for (size_t i = 0; i < nArmorArrSize; ++i)
	//{
	//	const auto& pArmor = ArmorTypeClass::Array[i];
	//	if(i < this->Verses.size()){
	//		if (pArmor->DefaultTo != -1) {
	//			this->Verses[i] = ArmorTypeClass::Array[(size_t)pArmor->DefaultTo]->DefaultVersesValue;
	//		} else { //evaluate armor with not valid default index
	//			this->Verses[i] = pArmor->DefaultVersesValue;
	//		}
	//	}
	//	else
	//	{
	//		this->Verses.push_back(pArmor->DefaultVersesValue);
	//	}
	//}
}

void WarheadTypeExtData::ApplyRecalculateDistanceDamage(ObjectClass* pVictim, args_ReceiveDamage* pArgs) const
{
	if ((pVictim->AbstractFlags & AbstractFlags::Techno) == AbstractFlags::None)
		return;

	const auto pVictimTechno = static_cast<TechnoClass*>(pVictim);

	if (!this->RecalculateDistanceDamage.Get() || !pArgs->Attacker)
		return;

	if (!this->RecalculateDistanceDamage_IgnoreMaxDamage && *pArgs->Damage == RulesClass::Instance->MaxDamage)
		return;

	const auto range = pArgs->Attacker->DistanceFrom(pVictim);
	const auto range_factor = range / (this->RecalculateDistanceDamage_Add_Factor.Get() * 256);
	const auto add = (this->RecalculateDistanceDamage_Add.Get() * range_factor);

	const auto multiply = Math::pow((this->RecalculateDistanceDamage_Multiply.Get()), range_factor);

	auto nAddDamage = add * multiply;
	if (this->RecalculateDistanceDamage_ProcessVerses)
	{
		nAddDamage *=
			// GeneralUtils::GetWarheadVersusArmor(this->Get() , pThisType->Armor)
			this->GetVerses(TechnoExtData::GetTechnoArmor(pVictimTechno, This())).Verses
			;
	}

	auto const nEligibleAddDamage = std::clamp((int)nAddDamage,
		this->RecalculateDistanceDamage_Min.Get(), this->RecalculateDistanceDamage_Max.Get());

	*pArgs->Damage += nEligibleAddDamage;

	if (this->RecalculateDistanceDamage_Display)
	{
		TechnoClass* pOwner = this->RecalculateDistanceDamage_Display_AtFirer ? pArgs->Attacker : pVictimTechno;
		FlyingStrings::Instance.AddNumberString(*pArgs->Damage, pOwner->Owner,
			AffectedHouse::All, Drawing::DefaultColors[(int)DefaultColorList::Yellow], pOwner->Location,
			this->RecalculateDistanceDamage_Display_Offset, true, L"");
	}
}

bool WarheadTypeExtData::CanAffectHouse(HouseClass* pOwnerHouse, HouseClass* pTargetHouse) const
{
	if (pOwnerHouse && pTargetHouse)
	{
		if (!this->AffectsNeutral && pTargetHouse->IsNeutral())
			return false;

		const bool affect_ally = This()->AffectsAllies;

		if (pTargetHouse == pOwnerHouse) {
			return this->AffectsOwner.Get(affect_ally);
		}

		const bool isAlly = pOwnerHouse->IsAlliedWith(pTargetHouse);
		return isAlly ? affect_ally : this->AffectsEnemies.Get();
	}

	return true;
}

bool WarheadTypeExtData::CanDealDamage(TechnoClass* pTechno, bool Bypass, bool SkipVerses , bool CheckImmune, bool checkLimbo) const
{
	if (pTechno)
	{
		if (checkLimbo && pTechno->InLimbo)
			return false;

		if (!pTechno->IsAlive
			|| !pTechno->Health
			|| pTechno->IsSinking
			|| pTechno->IsCrashing
			)
			return false;

		const auto pType = GET_TECHNOTYPE(pTechno);

		if (CheckImmune && pType->Immune)
			return false;

		if (!IsVeterancyInThreshold(pTechno))
			return false;
	
		if (auto const pBld = cast_to<BuildingClass*, false>(pTechno))
		{
			auto const pBldExt = BuildingExtContainer::Instance.Find(pBld);

			if (this->LimboKill_IDs.empty() && pBldExt->LimboID >= 0)
			{
				return false;
			}

			if (pBld->Type->InvisibleInGame)
				return false;
		}

		if (const auto pFoot = flag_cast_to<FootClass*, false>(pTechno))
		{
			if (TechnoExtData::IsChronoDelayDamageImmune(pFoot))
				return false;

			if (pFoot->WhatAmI() == UnitClass::AbsID)
			{
				if (static_cast<const UnitClass*>(pFoot)->DeathFrameCounter > 0)
				{
					return false;
				}
			}
		}

		if (pTechno->IsBeingWarpedOut())
			return false;

		if (!SkipVerses && EffectsRequireVerses.Get())
		{
			return (Math::abs(this->GetVerses(TechnoExtData::GetTechnoArmor(pTechno, This())).Verses) >= 0.001);
		}

		return true;
	}

	return Bypass;
}

bool WarheadTypeExtData::CanAffectInvulnerable(TechnoClass* pTarget) const {

	if (!pTarget || !pTarget->IsIronCurtained())
		return true;

	return pTarget->ProtectType == ProtectTypes::ForceShield ?
		this->PenetratesForceShield.Get(this->PenetratesIronCurtain) : this->PenetratesIronCurtain;
}

bool WarheadTypeExtData::CanDealDamage(TechnoClass* pTechno, int damageIn, int distanceFromEpicenter, int& DamageResult, bool effectsRequireDamage) const
{
	auto nArmor = TechnoExtData::GetTechnoArmor(pTechno, This());

	if (damageIn > 0)
		DamageResult = FakeWarheadTypeClass::ModifyDamage(damageIn, This(), nArmor, distanceFromEpicenter);
	else
		DamageResult = -FakeWarheadTypeClass::ModifyDamage(-damageIn, This(), nArmor, distanceFromEpicenter);

	if (damageIn == 0)
	{
		return AllowZeroDamage;
	}
	else
	{
		if (EffectsRequireVerses)
		{
			if (FakeWarheadTypeClass::ModifyDamage(RulesClass::Instance->MaxDamage, This(), nArmor, 0) == 0)
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

FullMapDetonateResult WarheadTypeExtData::EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner) const
{
	if (!EnumFunctions::IsTechnoEligibleB(pTechno, this->DetonateOnAllMapObjects_AffectTargets))
		return FullMapDetonateResult::TargetNotEligible;

	if (!this->CanDealDamage(pTechno, false, !this->DetonateOnAllMapObjects_RequireVerses.Get()))
		return FullMapDetonateResult::TargetNotDamageable;

	auto const pType = GET_TECHNOTYPE(pTechno);

	if (!EnumFunctions::CanTargetHouse(this->DetonateOnAllMapObjects_AffectHouses, pOwner, pTechno->Owner))
		return FullMapDetonateResult::TargetHouseNotEligible;

	if (!this->IsVeterancyInThreshold(pTechno))
		return FullMapDetonateResult::TargetRestricted;
	
	if (!this->DetonateOnAllMapObjects_AffectTypes.empty()
		&& !this->DetonateOnAllMapObjects_AffectTypes.Contains(pType))
		return FullMapDetonateResult::TargetRestricted;

	if (!this->DetonateOnAllMapObjects_IgnoreTypes.empty() &&
		this->DetonateOnAllMapObjects_IgnoreTypes.Contains(pType))
		return FullMapDetonateResult::TargetRestricted;

	return FullMapDetonateResult::TargetValid;
}

bool WarheadTypeExtData::CanTargetHouse(HouseClass* pHouse, TechnoClass* pTarget) const
{
	if (pHouse && pTarget)
	{
		return CanAffectHouse(pHouse, pTarget->GetOwningHouse());
	}

	return true;
}

void WarheadTypeExtData::applyWebby(TechnoClass* pTarget, HouseClass* pKillerHouse, TechnoClass* pKillerTech) const
{
	if (!this->Webby || this->Webby_Duration == 0)
		return;

	if (auto pInf = cast_to<InfantryClass*>(pTarget))
	{
		if (TechnoExtData::IsWebImmune(pInf))
			return;

		const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pInf->Type);
		const auto pAnim = pTechnoTypeExt->Webby_Anims.GetElements(this->Webby_Anims);

		if (pAnim.empty())
			return;

		const auto durVar = pTechnoTypeExt->Webby_Duration_Variation.Get(this->Webby_Duration_Variation);

		// duration modifier
		int duration = this->Webby_Duration;
		int durMin = this->Webby_Duration - durVar;
		int durMax = this->Webby_Duration + durVar;
		int getDur = ScenarioClass::Instance->Random.RandomRanged(durMin, durMax);

		if (getDur > 0)
			getDur = int(getDur * pTechnoTypeExt->Webby_Modifier);

		// get the values
		auto pExt = TechnoExtContainer::Instance.Find(pInf);
		int oldValue = (!pExt->IsWebbed || pInf->ParalysisTimer.Expired() ? 0 : pInf->ParalysisTimer.GetTimeLeft());
		int newValue = Helpers::Alex::getCappedDuration(oldValue, duration, this->Webby_Cap);

		// update according to oldval
		if (oldValue <= 0)
		{
			// start effect?
			if (newValue > 0)
			{
				if (pInf->Locomotor->Is_Moving())
					pInf->Locomotor->Stop_Moving();

				pExt->IsWebbed = true;
				pInf->ParalysisTimer.Start(newValue);
				AnimTypeClass* pAnimType = pAnim[ScenarioClass::Instance->Random.RandomFromMax(pAnim.size() - 1)];
				pExt->WebbedAnim.reset(GameCreate<AnimClass>(pAnimType, pInf->Location, 0, 1, 0x600, 0, false));
				pExt->WebbedAnim->SetOwnerObject(pInf);
				AnimExtData::SetAnimOwnerHouseKind(pExt->WebbedAnim, pKillerHouse, pInf->Owner, pKillerTech, false, false);
				TechnoExtData::StoreLastTargetAndMissionAfterWebbed(pInf);
			}
		}
		else
		{
			// is already on.
			if (newValue > 0)
			{

				// set new length and reset the anim ownership
				pInf->ParalysisTimer.Start(newValue);

				if (!pExt->WebbedAnim)
				{
					AnimTypeClass* pAnimType = pAnim[ScenarioClass::Instance->Random.RandomFromMax(pAnim.size() - 1)];
					pExt->WebbedAnim.reset(GameCreate<AnimClass>(pAnimType, pInf->Location, 0, 1, 0x600, 0, false));
					pExt->WebbedAnim->SetOwnerObject(pInf);
					AnimExtData::SetAnimOwnerHouseKind(pExt->WebbedAnim, pKillerHouse, pInf->Owner, pKillerTech, false, false);
				}
				else
				{
					AnimExtData::SetAnimOwnerHouseKind(pExt->WebbedAnim, pKillerHouse, pInf->Owner, pKillerTech, false, false);
				}
			}
			else //turn off
			{
				if (pExt->IsWebbed)
				{
					pExt->IsWebbed = false;
					pInf->ParalysisTimer.Stop();
					TechnoExtData::RestoreLastTargetAndMissionAfterWebbed(pInf);
				}

				if (pExt->WebbedAnim)
				{
					pExt->WebbedAnim.reset();
				}
			}
		}
	}
}

AnimTypeClass* __fastcall WarheadTypeExtData::SelectCombatAnim(int damage, WarheadTypeClass* pWarhead, LandType land, CoordStruct& coord)
{
	if (pWarhead) {

		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
		pWHExt->Splashed = false;

		//allowing zero damage to pass ,..
		//hopefully it wont do any harm to these thing , ...

		if ((damage == 0 && pWHExt->AnimList_ShowOnZeroDamage) || damage) {

			if (damage < 0)
				damage = -damage;

			if (land == LandType::Water
				&& pWarhead->Conventional
				&& !MapClass::Instance->GetCellAt(coord)->ContainsBridge()
				&& coord.Z < (MapClass::Instance->GetCellFloorHeight(coord) + Unsorted::CellHeight)
				) {
				pWHExt->Splashed = true;

				if (const auto Vec = pWHExt->SplashList.GetElements(RulesClass::Instance->SplashList)) {

					size_t idx = pWHExt->SplashList_PickRandom ?
						ScenarioClass::Instance->Random.RandomFromMax(Vec.size() - 1) :
						MinImpl(Vec.size() * 35 - 1, (size_t)damage) / 35;

					return (Vec[idx < Vec.size() ? idx : 0]);
				}

				return (nullptr);
			}

			if (auto const pSuper = SW_LightningStorm::CurrentLightningStorm) {
				auto const pData = SWTypeExtContainer::Instance.Find(pSuper->Type);

				if (pData->GetNewSWType()->GetWarhead(pData) == pWarhead) {
					if (auto const pAnimType = pData->Weather_BoltExplosion.Get(
						RulesClass::Instance->WeatherConBoltExplosion)) {
						return (pAnimType);
					}
				}
			}

			if (pWHExt->CritActive && !pWHExt->Crit_AnimList.empty() && !pWHExt->Crit_AnimOnAffectedTargets) {
				const size_t idx = pWHExt->Crit_AnimList_PickRandom.Get(pWHExt->AnimList_PickRandom.Get(pWarhead->EMEffect)) ?
					ScenarioClass::Instance->Random.RandomFromMax(pWHExt->Crit_AnimList.size() - 1) :
					(MinImpl(pWHExt->Crit_AnimList.size() * 25 - 1, (size_t)damage) / 25);

				return (pWHExt->Crit_AnimList[idx < pWHExt->Crit_AnimList.size() ? idx : 0]);
			}

			if (!pWarhead->AnimList.Empty()) {
				const size_t idx = pWHExt->AnimList_PickRandom.Get(pWarhead->EMEffect) ?
					ScenarioClass::Instance->Random.RandomFromMax(pWarhead->AnimList.Count - 1) :
					MinImpl(pWarhead->AnimList.Count * 25 - 1, damage) / 25;

				return pWarhead->AnimList.Items[idx < pWarhead->AnimList.size() ? idx : 0];
			}
		}
	}

	return nullptr;
}

bool WarheadTypeExtData::applyCulling(TechnoClass* pSource, ObjectClass* pTarget) const
{
	auto const pThis = This();

	if (!pThis->Culling || !pSource)
		return false;

	if (auto const pTargetTechno = flag_cast_to<TechnoClass*, false>(pTarget))
	{
		if (TechnoExtData::IsCullingImmune(pTargetTechno))
			return false;

		if (this->Culling_Target.isset() && !EnumFunctions::IsTechnoEligible(pTargetTechno, this->Culling_Target.Get(), false))
			return false;
	}

	const auto nCullingHP = Culling_BelowHP.Get(pSource);

	if (nCullingHP <= 0)
	{
		if ((int)pTarget->GetHealthStatus() > -nCullingHP)
			return false;
	}
	else
	{
		if (static_cast<int>(pTarget->GetHealthPercentage() * 100.0) > nCullingHP)
			return false;
	}

	const auto nChance = Culling_Chance.Get(pSource);
	return  nChance < 0 || ScenarioClass::Instance->Random.RandomFromMax(99) < nChance;
}

static COMPILETIMEEVAL int GetRelativeValue(ObjectClass* pTarget, WarheadTypeExtData* pWHExt)
{
	auto const pWhat = pTarget->WhatAmI();
	bool IsTechno = false;
	int relative = 0;

	switch (pWhat)
	{
	case UnitClass::AbsID:
	{
		const auto pUnit = static_cast<UnitClass*>(pTarget);

		if (pUnit->Type->ConsideredAircraft)
			relative = pWHExt->RelativeDamage_AirCraft;
		else if (pUnit->Type->Organic)
			relative = pWHExt->RelativeDamage_Infantry;
		else
			relative = pWHExt->RelativeDamage_Unit;

		IsTechno = true;
		break;
	}
	case AircraftClass::AbsID:
	{
		relative = pWHExt->RelativeDamage_AirCraft;
		IsTechno = true;
		break;
	}
	case BuildingClass::AbsID:
		relative = pWHExt->RelativeDamage_Building;
		IsTechno = true;
		break;
	case InfantryClass::AbsID:
		relative = pWHExt->RelativeDamage_Infantry;
		IsTechno = true;
		break;
	case TerrainClass::AbsID:
		return pWHExt->RelativeDamage_Terrain;
	}

	if (IsTechno && relative > 0)
	{
		return int(relative * TechnoExtContainer::Instance.Find((TechnoClass*)pTarget)->AE.ReceiveRelativeDamageMult);
	}

	return relative;
}

void WarheadTypeExtData::applyRelativeDamage(ObjectClass* pTarget, args_ReceiveDamage* pArgs) const
{
	if (!this->RelativeDamage)
		return;

	auto nRelativeVal = GetRelativeValue(pTarget, const_cast<WarheadTypeExtData*>(this));

	if (nRelativeVal != 0)
	{
		nRelativeVal *= nRelativeVal < 0 ? pTarget->Health / -100 : pTarget->GetType()->Strength / 100;
	}

	if (*pArgs->Damage < 0)
		nRelativeVal = -nRelativeVal;

	*pArgs->Damage = nRelativeVal;
}

bool WarheadTypeExtData::GoBerzerkFor(FootClass* pVictim, int* damage) const
{
	int nDur = this->Berzerk_dur.Get(*damage);
	auto const pType = GET_TECHNOTYPE(pVictim);

	if (nDur != 0)
	{
		if (nDur > 0)
		{
			if (auto pData = TechnoTypeExtContainer::Instance.Find(pType))
			{
				nDur = static_cast<int>(nDur * pData->Berzerk_Modifier.Get());
			}
		}

		//Default way game modify duration
		nDur = FakeWarheadTypeClass::ModifyDamage(nDur, This(),
					TechnoExtData::GetTechnoArmor(pVictim, This()), 0);

		const int oldValue = (!pVictim->Berzerk ? 0 : pVictim->BerzerkDurationLeft);
		const int newValue = Helpers::Alex::getCappedDuration(oldValue, nDur, this->Berzerk_cap.Get());

		//auto const underBrzBefore = (oldValue > 0);
		//auto const underBrzAfter = (pVictim->Berzerk && pVictim->BerzerkDurationLeft > 0);
		//auto const newlyUnderBrz = !underBrzBefore && underBrzAfter;
		if (oldValue == newValue)
			return this->Berzerk_dealDamage.Get();

		if (oldValue <= 0)
		{
			if (newValue > 0) {
				pVictim->GoBerzerkFor(newValue);
			}
		}
		else
		{

			if (newValue > 0) {
				pVictim->GoBerzerkFor(newValue);
			} else {

				auto const nLeft = pVictim->BerzerkDurationLeft - newValue;
				if (nLeft <= 0)
				{
					pVictim->BerzerkDurationLeft = 0;
					pVictim->Berzerk = false;
					pVictim->SetTarget(nullptr);
					TechnoExtData::SetMissionAfterBerzerk(pVictim);
				} else {
					pVictim->BerzerkDurationLeft -= nLeft;
				}
			}
		}

		return this->Berzerk_dealDamage.Get();
	}

	return false; //default
}

AnimTypeClass* WarheadTypeExtData::GetArmorHitAnim(int Armor)
{
	for (auto begin = this->ArmorHitAnim.begin(); begin != this->ArmorHitAnim.end(); ++begin) {
		if (begin->first == ArmorTypeClass::Array[Armor].get()) {
			return begin->second;
		}
	}

	return nullptr;
}

void WarheadTypeExtData::DetonateAt(
	WarheadTypeClass* pThis,
	ObjectClass* pTarget,
	TechnoClass* pOwner,
	int damage,
	bool targetCell,
	HouseClass* pFiringHouse
)
{
	if (targetCell && !pTarget)
	{
		Debug::LogInfo("WarheadTypeExtData::Detonate asking for targetCell but pTarget is nullptr ! ");
		return;
	}

	AbstractClass* pATarget = !targetCell ? static_cast<AbstractClass*>(pTarget) : pTarget->GetCell();
	WarheadTypeExtData::DetonateAt(pThis, pATarget, CoordStruct::Empty, pOwner, damage, pFiringHouse);
}

void WarheadTypeExtData::DetonateAt(
	WarheadTypeClass* pThis,
	const CoordStruct coords,
	TechnoClass* pOwner,
	int damage,
	bool targetCell,
	HouseClass* pFiringHouse
)
{
	if (targetCell && !coords.IsValid())
	{
		Debug::LogInfo("WarheadTypeExtData::Detonate asking for targetCell but coords is invalid ! ");
		return;
	}

	AbstractClass* pTarget = !targetCell ? nullptr : coords.IsValid() ? MapClass::Instance->GetCellAt(coords) : nullptr;
	WarheadTypeExtData::DetonateAt(pThis, pTarget, coords, pOwner, damage, pFiringHouse);
}

#include <Ext/Scenario/Body.h>

void WarheadTypeExtData::DetonateAt(
	WarheadTypeClass* pThis,
	AbstractClass* pTarget,
	const CoordStruct coords,
	TechnoClass* pOwner,
	int damage,
	HouseClass* pFiringHouse
)
{
	//BulletTypeClass* pType = BulletTypeExtData::GetDefaultBulletType();

	//if(!pType)
	//	Debug::FatalError("Uneable to Fetch %s BulletType ! " , DEFAULT_STR2);

	//if (pThis->NukeMaker)
	//{
	//	if (!pTarget)
	//	{
	//		Debug::LogInfo("WarheadTypeExtData::DetonateAt , cannot execute when invalid Target is present , need to be avail ! ");
	//		return;
	//	}
	//}
	//
	//if (!pOwner && Phobos::Otamaa::IsAdmin) {
	//	Debug::LogInfo("WarheadTypeExtData::DetonateAt[%s] delivering damage from unknown source [%x] !", pThis->get_ID(), pOwner);
	//}

	ScenarioExtData::DetonateMasterBullet(coords,
		pOwner,
		damage,
		pFiringHouse,
		pTarget,
		pThis->Bright,
		nullptr,
		pThis
	);

	//if (BulletClass* pBullet = BulletTypeExtContainer::Instance.Find(pType)->CreateBullet(pTarget, pOwner,
	//	damage, pThis, 0, 0, pThis->Bright, true))
	//{
	//	pBullet->MoveTo(coords, VelocityClass::Empty);
	//
	//	//something like 0x6FF08B
	//	const auto pCellCoord = MapClass::Instance->GetCellAt(coords);
	//	if (pBullet->Type->Inviso && pCellCoord->ContainsBridge())
	//		pBullet->OnBridge = true;
	//
	//	BulletExtData::DetonateAt(pBullet, pTarget, pOwner, coords, pFiringHouse);
	//}
}

void WarheadTypeExtData::CreateIonBlast(WarheadTypeClass* pThis, const CoordStruct& coords)
{
	const auto pExt = WarheadTypeExtContainer::Instance.Find(pThis);

	if (pExt->Ion || pExt->Ripple_Radius.isset() && pExt->Ripple_Radius > 0)
	{
		auto pIon = GameCreate<IonBlastClass>(coords);
		pIon->DisableIonBeam = !pExt->Ion;
		IonBlastExt.insert(pIon, pExt);
	}
}

#include <Misc/Ares/Hooks/Header.h>

void WarheadTypeExtData::applyEMP(WarheadTypeClass* pWH, const CoordStruct& coords, TechnoClass* source)
{
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->EMP_Duration)
		AresEMPulse::CreateEMPulse(pWH, coords, source);

}

void WarheadTypeExtData::ApplyPenetratesTransport(TechnoClass* pTarget, TechnoClass* pInvoker, HouseClass* pInvokerHouse, const CoordStruct& coords, int damage) const
{
	auto& passengers = pTarget->Passengers;
	auto passenger = passengers.GetFirstPassenger();

	if (!passenger)
		return;

	const auto pTargetType = GET_TECHNOTYPE(pTarget);
	const auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);

	if (this->PenetratesTransport_Level <= pTargetTypeExt->PenetratesTransport_Level.Get(RulesExtData::Instance()->PenetratesTransport_Level))
		return;

	const double passThrough = this->PenetratesTransport_PassThrough * pTargetTypeExt->PenetratesTransport_PassThroughMultiplier;

	if (passThrough < 1.0 && ScenarioClass::Instance->Random.RandomDouble() > passThrough)
		return;

	const double fatalRate = this->PenetratesTransport_FatalRate * pTargetTypeExt->PenetratesTransport_FatalRateMultiplier;
	const bool fatal = fatalRate > 0.0 && ScenarioClass::Instance->Random.RandomDouble() <= fatalRate;
	const auto pTargetFoot = flag_cast_to<FootClass*, false>(pTarget);
	const int distance = static_cast<int>(coords.DistanceFrom(pTarget->GetCoords()));
	const auto pWH = This();
	bool gunnerRemoved = false;

	if (this->PenetratesTransport_DamageAll)
	{
		bool isFirst = true;

		if (fatal)
		{
			while (passenger)
			{
				const auto nextPassenger = flag_cast_to<FootClass*>(passenger->NextObject);

				if (this->PenetratesTransport_Level > 
					GET_TECHNOTYPEEXT(passenger)->PenetratesTransport_Level.Get(RulesExtData::Instance()->PenetratesTransport_Level))
				{
					if (passenger->ReceiveDamage(&passenger->Health, distance, pWH, pInvoker, false, true, pInvokerHouse) == DamageState::NowDead && isFirst && pTargetType->Gunner && pTargetFoot)
					{
						pTargetFoot->RemoveGunner(passenger);
						gunnerRemoved = true;
					}
				}

				passenger = nextPassenger;
				isFirst = false;
			}
		}
		else
		{
			const int adjustedDamage = static_cast<int>(std::ceil(damage * this->PenetratesTransport_DamageMultiplier * pTargetTypeExt->PenetratesTransport_DamageMultiplier));

			while (passenger)
			{
				const auto nextPassenger = flag_cast_to<FootClass*>(passenger->NextObject);

				if (this->PenetratesTransport_Level > 
					GET_TECHNOTYPEEXT(passenger)->PenetratesTransport_Level.Get(RulesExtData::Instance()->PenetratesTransport_Level))
				{
					int applyDamage = adjustedDamage;

					if (passenger->ReceiveDamage(&applyDamage, distance, pWH, pInvoker, false, true, pInvokerHouse) == DamageState::NowDead && isFirst && pTargetType->Gunner && pTargetFoot)
					{
						pTargetFoot->RemoveGunner(passenger);
						gunnerRemoved = true;
					}
				}

				passenger = nextPassenger;
				isFirst = false;
			}
		}
	}
	else
	{
		int poorBastardIdx = ScenarioClass::Instance->Random(0, passengers.NumPassengers - 1);
		const bool isFirst = poorBastardIdx == 0;

		while (poorBastardIdx > 0 && flag_cast_to<FootClass*>(passenger->NextObject))
		{
			passenger = static_cast<FootClass*>(passenger->NextObject);
			--poorBastardIdx;
		}

		if (this->PenetratesTransport_Level <= 
			GET_TECHNOTYPEEXT(passenger)->PenetratesTransport_Level.Get(RulesExtData::Instance()->PenetratesTransport_Level))
			return;

		if (fatal)
		{
			if (passenger->ReceiveDamage(&passenger->Health, distance, pWH, pInvoker, false, true, pInvokerHouse) == DamageState::NowDead && isFirst && pTargetType->Gunner && pTargetFoot)
			{
				pTargetFoot->RemoveGunner(passenger);
				gunnerRemoved = true;
			}
		}
		else
		{
			int adjustedDamage = static_cast<int>(std::ceil(damage * this->PenetratesTransport_DamageMultiplier * pTargetTypeExt->PenetratesTransport_DamageMultiplier));

			if (passenger->ReceiveDamage(&adjustedDamage, distance, pWH, pInvoker, false, true, pInvokerHouse) == DamageState::NowDead && isFirst && pTargetType->Gunner && pTargetFoot)
			{
				pTargetFoot->RemoveGunner(passenger);
				gunnerRemoved = true;
			}
		}
	}

	passenger = passengers.GetFirstPassenger();

	if (passenger)
	{
		if (gunnerRemoved)
			pTargetFoot->ReceiveGunner(passenger);
	}
	else
	{
		VocClass::SafeImmedietelyPlayAt(this->PenetratesTransport_CleanSound, coords);
	}
}

// =============================
// load / save

template <typename T>
void WarheadTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Reveal)
		.Process(this->BigGap)
		.Process(this->CreateGap)
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
		.Process(this->SplashList_CreateAll)
		.Process(this->SplashList_CreationInterval)
		.Process(this->SplashList_ScatterMin)
		.Process(this->SplashList_ScatterMax)
		.Process(this->RemoveDisguise)
		.Process(this->RemoveMindControl)
		.Process(this->AnimList_PickRandom)
		.Process(this->AnimList_CreateAll)
		.Process(this->AnimList_CreationInterval)
		.Process(this->AnimList_ScatterMin)
		.Process(this->AnimList_ScatterMax)
		.Process(this->AnimList_ShowOnZeroDamage)
		.Process(this->DecloakDamagedTargets)
		.Process(this->ShakeIsLocal)
		.Process(this->Shake_UseAlternativeCalculation)

		.Process(this->Crit_Chance)
		.Process(this->Crit_ApplyChancePerTarget)
		.Process(this->Crit_ExtraDamage)
		.Process(this->Crit_ExtraDamage_ApplyFirepowerMult)
		.Process(this->Crit_Warhead)
		.Process(this->Crit_Warhead_FullDetonation)
		.Process(this->Crit_Affects)
		.Process(this->Crit_AffectsHouses)
		.Process(this->Crit_AnimList)
		.Process(this->Crit_AnimList_PickRandom)
		.Process(this->Crit_AnimList_CreateAll)
		.Process(this->Crit_ActiveChanceAnims)
		.Process(this->Crit_AnimOnAffectedTargets)
		.Process(this->Crit_AffectBelowPercent)
		.Process(this->Crit_AffectAbovePercent)
		.Process(this->Crit_SuppressWhenIntercepted)
		.Process(this->CritActive)
		.Process(this->CritRandomBuffer)
		.Process(this->CritCurrentChance)
		.Process(this->ReturnWarhead_RandomBuffer)
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
		.Process(this->Shield_ReceivedDamage_Minimum)
		.Process(this->Shield_ReceivedDamage_Maximum)
		.Process(this->Shield_ReceivedDamage_MinMultiplier)
		.Process(this->Shield_ReceivedDamage_MaxMultiplier)
		.Process(this->Shield_Respawn_Duration)
		.Process(this->Shield_Respawn_Amount)
		.Process(this->Shield_Respawn_Rate)
		.Process(this->Shield_Respawn_Rate_InMinutes)
		.Process(this->Shield_Respawn_RestartInCombat)
		.Process(this->Shield_Respawn_RestartInCombatDelay)
		.Process(this->Shield_Respawn_RestartTimer)
		.Process(this->Shield_Respawn_Anim)
		.Process(this->Shield_Respawn_Weapon)
		.Process(this->Shield_SelfHealing_Duration)
		.Process(this->Shield_SelfHealing_Amount)
		.Process(this->Shield_SelfHealing_Rate)
		.Process(this->Shield_SelfHealing_Rate_InMinutes)
		.Process(this->Shield_SelfHealing_RestartInCombat)
		.Process(this->Shield_SelfHealing_RestartInCombatDelay)
		.Process(this->Shield_SelfHealing_RestartTimer)
		.Process(this->Shield_AttachTypes)
		.Process(this->Shield_RemoveTypes)
		.Process(this->Shield_ReplaceOnly)
		.Process(this->Shield_ReplaceNonRespawning)
		.Process(this->Shield_InheritStateOnReplace)
		.Process(this->Shield_MinimumReplaceDelay)
		.Process(this->Shield_AffectTypes)
		.Process(this->Shield_Penetrate_Types)
		.Process(this->Shield_Penetrate_Types_Disallowed_Types)
		.Process(this->Shield_Penetrate_Armor_Types)
		.Process(this->Shield_Break_Types)
		.Process(this->Shield_Respawn_Types)
		.Process(this->Shield_SelfHealing_Types)
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
		.Process(this->DebrisTypes_Limit)
		.Process(this->DebrisMinimums)

		.Process(this->GattlingStage)
		.Process(this->GattlingRateUp)
		.Process(this->ReloadAmmo)

		.Process(this->MindControl_UseTreshold)
		.Process(this->MindControl_Threshold)
		.Process(this->MindControl_Threshold_Inverse)
		.Process(this->MindControl_AlternateDamage)
		.Process(this->MindControl_AlternateWarhead)
		.Process(this->MindControl_CanKill)

		.Process(this->DetonateOnAllMapObjects)
		.Process(this->DetonateOnAllMapObjects_Full)
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
		.Process(this->Splashed)
		.Process(this->RemainingAnimCreationInterval)

		.Process(this->NotHuman_DeathAnim)
		.Process(this->IsNukeWarhead)
		.Process(this->PreImpactAnim)
		.Process(this->NukeFlashDuration)
		.Process(this->Remover)
		.Process(this->Remover_Anim)
		.Process(this->ArmorHitAnim)
		.Process(this->DebrisAnimTypes)
		.Process(this->SquidSplash)
		.Process(this->TemporalExpiredAnim)
		.Process(this->TemporalExpiredApplyDamage)
		.Process(this->TemporalDetachDamageFactor)
		.Process(this->Parasite_DisableRocking)
		.Process(this->Parasite_GrappleAnim)
		.Process(this->Parasite_ParticleSys)
		.Process(this->Parasite_TreatInfantryAsVehicle)
		.Process(this->Parasite_InvestationWP)
		.Process(this->Parasite_Damaging_Chance)

		.Process(this->Flammability)
		.Process(this->Launchs)
		.Process(this->PermaMC)
		.Process(this->Sound)
		.Process(this->ConvertsPair)
		.Process(this->Convert_SucceededAnim)
		.Process(this->StealMoney)
		.Process(this->Steal_Display_Houses)
		.Process(this->Steal_Display)
		.Process(this->Steal_Display_Offset)
		.Process(this->AffectEnemies_Damage_Mod)
		.Process(this->AffectOwner_Damage_Mod)
		.Process(this->AffectAlly_Damage_Mod)

		.Process(this->DamageOwnerMultiplier)
		.Process(this->DamageAlliesMultiplier)
		.Process(this->DamageEnemiesMultiplier)
		.Process(this->DamageOwnerMultiplier_Berzerk)
		.Process(this->DamageAlliesMultiplier_Berzerk)
		.Process(this->DamageEnemiesMultiplier_Berzerk)

		.Process(this->AttachTag)
		.Process(this->AttachTag_Types)
		.Process(this->AttachTag_Ignore)
		.Process(this->AttachTag_Imposed)

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

		.Process(this->Culling_BelowHP)
		.Process(this->Culling_Chance)
		.Process(this->Culling_Target)

		.Process(this->RelativeDamage)
		.Process(this->RelativeDamage_AirCraft)
		.Process(this->RelativeDamage_Unit)
		.Process(this->RelativeDamage_Infantry)
		.Process(this->RelativeDamage_Building)
		.Process(this->RelativeDamage_Terrain)
		.Process(this->Verses)

		.Process(this->Berzerk_dur)
		.Process(this->Berzerk_cap)
		.Process(this->Berzerk_dealDamage)
		.Process(this->IC_Flash)

		.Process(this->PreventScatter)
		.Process(this->DieSound_Override)
		.Process(this->VoiceSound_Override)

		.Process(this->SuppressDeathWeapon_Vehicles)
		.Process(this->SuppressDeathWeapon_Infantry)
		.Process(this->SuppressDeathWeapon)
		.Process(this->SuppressDeathWeapon_Exclude)
		.Process(this->SuppressDeathWeapon_Chance)

		.Process(this->DeployedDamage)
		.Process(this->Temporal_WarpAway)
		.Process(this->Supress_LostEva)
		.Process(this->Temporal_HealthFactor)
		.Process(this->InfDeathAnims)
		.Process(this->Sonar_Duration)
		.Process(this->DisableWeapons_Duration)
		.Process(this->Flash_Duration)
		.Process(this->ImmunityType)
		.Process(this->Malicious)
		.Process(this->PreImpact_Moves)
		.Process(this->Conventional_IgnoreUnits)

		.Process(this->InflictLocomotor)
		.Process(this->RemoveInflictedLocomotor)
		.Process(this->Rocker_AmplitudeMultiplier)
		.Process(this->Rocker_AmplitudeOverride)
		.Process(this->NukePayload_LinkedSW)
		.Process(this->IC_Duration)
		.Process(this->IC_Cap)

#pragma region Ion
		.Process(this->Ion)
		.Process(this->Ripple_Radius)
		.Process(this->Ion_Beam)
		.Process(this->Ion_Blast)
		.Process(this->Ion_AllowWater)
		.Process(this->Ion_Rocking)
		.Process(this->Ion_WH)
		.Process(this->Ion_Damage)
#pragma endregion

		.Process(this->DetonateParticleSystem)
		.Process(this->BridgeAbsoluteDestroyer)
		.Process(this->DamageAirThreshold)
		.Process(this->CellSpread_MaxAffect)

		.Process(this->EMP_Duration)
		.Process(this->EMP_Cap)
		.Process(this->EMP_Sparkles)

		.Process(this->CanRemoveParasytes)
		.Process(this->CanRemoveParasytes_KickOut)
		.Process(this->CanRemoveParasytes_KickOut_Paralysis)
		.Process(this->CanRemoveParasytes_ReportSound)
		.Process(this->CanRemoveParasytes_KickOut_Anim)

		.Process(this->Webby)
		.Process(this->Webby_Anims)
		.Process(this->Webby_Duration)
		.Process(this->Webby_Cap)
		.Process(this->Webby_Duration_Variation)
		.Process(this->SelfHealing_CombatDelay)
#ifdef COMPILE_PORTED_DP_FEATURES_
		.Process(DamageTextPerArmor)

#endif
		.Process(this->PaintBallDuration)

		.Process(this->KillDriver)
		.Process(this->KillDriver_KillBelowPercent)
		.Process(this->KillDriver_Owner)
		.Process(this->KillDriver_ResetVeterancy)
		.Process(this->KillDriver_Chance)
		.Process(this->ApplyModifiersOnNegativeDamage)

		.Process(this->CombatLightDetailLevel)
		.Process(this->CombatLightChance)
		.Process(this->Particle_AlphaImageIsLightFlash)
		.Process(this->Nonprovocative)

		.Process(this->SpawnsCrate_Types)
		.Process(this->SpawnsCrate_Weights)

		.Process(this->PhobosAttachEffects)

		.Process(this->Shield_HitFlash)
		.Process(this->Shield_SkipHitAnim)
		.Process(this->CombatAlert_Suppress)

		.Process(this->AffectsGround)
		.Process(this->AffectsInAir)
		.Process(this->CellSpread_Cylinder)

		.Process(this->PenetratesIronCurtain)
		.Process(this->PenetratesForceShield)
		.Process(this->Shield_RemoveAll)

		.Process(this->SuppressRevengeWeapons)
		.Process(this->SuppressRevengeWeapons_Types)
		.Process(this->SuppressReflectDamage)
		.Process(this->SuppressReflectDamage_Types)
		.Process(this->SuppressReflectDamage_Groups)
		.Process(this->RemoveParasites)
		.Process(this->Reflected)
		.Process(this->CLIsBlack)
		.Process(this->ApplyMindamage)
		.Process(this->MinDamage)
		.Process(this->IntendedTarget)

		.Process(this->KillWeapon)
		.Process(this->KillWeapon_OnFirer)
		.Process(this->KillWeapon_AffectsHouses)
		.Process(this->KillWeapon_OnFirer_AffectsHouses)
		.Process(this->KillWeapon_Affects)
		.Process(this->KillWeapon_OnFirer_Affects)

		.Process(this->MindControl_ThreatDelay)
		.Process(this->MergeBuildingDamage)

		.Process(this->BuildingSell)
		.Process(this->BuildingSell_IgnoreUnsellable)
		.Process(this->BuildingUndeploy)
		.Process(this->BuildingUndeploy_Leave)

		.Process(this->ScorchChance)
		.Process(this->ScorchPercentAtMax)
		.Process(this->CraterChance)
		.Process(this->CraterPercentAtMax)
		.Process(this->CellAnimChance)
		.Process(this->CellAnimPercentAtMax)
		.Process(this->CellAnim)
		.Process(this->ElectricAssaultLevel)
		.Process(this->AirstrikeTargets)
		.Process(this->CanKill)
		.Process(this->ElectricAssault_Requireverses)
		.Process(this->DamageSourceHealthMultiplier)
		.Process(this->DamageTargetHealthMultiplier)


		.Process(this->AffectsBelowPercent)
		.Process(this->AffectsAbovePercent)
		.Process(this->AffectsVeterancy)
		.Process(this->AffectsNeutral)

		.Process(this->PenetratesTransport_Level)
		.Process(this->PenetratesTransport_PassThrough)
		.Process(this->PenetratesTransport_FatalRate)
		.Process(this->PenetratesTransport_DamageMultiplier)
		.Process(this->PenetratesTransport_DamageAll)
		.Process(this->PenetratesTransport_CleanSound)

		.Process(this->FakeEngineer_CanRepairBridges)
		.Process(this->FakeEngineer_CanDestroyBridges)
		.Process(this->FakeEngineer_CanCaptureBuildings)
		.Process(this->FakeEngineer_BombDisarm)
		.Process(this->ReverseEngineer)

		.Process(this->UnlimboDetonate)
		.Process(this->UnlimboDetonate_Force)
		.Process(this->UnlimboDetonate_KeepTarget)
		.Process(this->UnlimboDetonate_KeepSelected)

		.Process(this->BlockType)
		.Process(this->Block_BasedOnWarhead)
		.Process(this->Block_AllowOverride)
		.Process(this->Block_IgnoreChanceModifier)
		.Process(this->Block_ChanceMultiplier)
		.Process(this->Block_ExtraChance)
		.Process(this->ImmuneToBlock)
		.Process(this->AffectsUnderground)
		.Process(this->PlayAnimUnderground)
		.Process(this->PlayAnimAboveSurface)
		.Process(this->AnimZAdjust)
		.Process(this->ApplyPerTargetEffectsOnDetonate)
		.Process(this->ReturnWarhead)
		.Process(this->ReturnWarhead_Damage)
		.Process(this->ReturnWarhead_Chance)
		.Process(this->ReturnWarhead_ApplyChancePerTarget)
		.Process(this->ReturnWarhead_FullDetonation)
		.Process(this->ReturnWarhead_AffectsTarget)
		.Process(this->ReturnWarhead_AffectsHouse)

		.Process(this->IsCellSpreadWH)
		.Process(this->IsFakeEngineer)
		.Process(this->VeterancyCheck)
		;

	PaintBallData.Serialize(Stm);
}

#include <RadarEventClass.h>

void WarheadTypeExtData::DetonateAtBridgeRepairHut(AbstractClass* pTarget, TechnoClass* pOwner, HouseClass* pFiringHouse, bool destroyBridge)
{
	auto const pBuilding = cast_to<BuildingClass*>(pTarget);

	if (!pBuilding || !pBuilding->Type->BridgeRepairHut || !pBuilding->IsAlive || pBuilding->Health <= 0)
		return;

	const CoordStruct targetCoords = pTarget->GetCenterCoords();
	const CellStruct baseCell = CellClass::Coord2Cell(targetCoords);

	// Send engineer's "enter" event
	auto const pTag = pBuilding->AttachedTag;

	if (pTag && pOwner)
		pTag->RaiseEvent(TriggerEvent::EnteredBy, pOwner, CellStruct::Empty);

	// Check a 5x5 area for bridge tiles to determine if we should repair or destroy
	bool foundWoodBridge = false;

	for (int y = -2; y <= 2; ++y)
	{
		for (int x = -2; x <= 2; ++x)
		{
			CellStruct checkCellCoords = { static_cast<short>(baseCell.X + x), static_cast<short>(baseCell.Y + y) };
			auto const checkCell = MapClass::Instance->GetCellAt(checkCellCoords);

			if (checkCell->Tile_Is_WoodBridge() || (checkCell->OverlayTypeIndex >= 74 && checkCell->OverlayTypeIndex <= 101))
				foundWoodBridge = true;

			if (foundWoodBridge)
				break;
		}

		if (foundWoodBridge)
			break;
	}

	// Destroying bridges
	if (destroyBridge)
	{
		if (foundWoodBridge) // Repair wood bridges
			MapClass::Instance->DestroyWoodBridgeAt(baseCell);
		else // Destroy concrete bridges
			MapClass::Instance->DestroyConcreteBridgeAt(baseCell);

		return;
	}

	auto const pFiringOwner = pOwner ? pOwner->Owner : pFiringHouse;

	// Repairing bridges
	if (pFiringOwner && pFiringOwner->ControlledByCurrentPlayer())
	{
		if (RadarEventClass::Create(RadarEventType::BridgeRepaired, CellClass::Coord2Cell(targetCoords)))
			VoxClass::PlayIndex(VoxClass::FindIndexById("EVA_BridgeRepaired"));
	}

	VocClass::SafeImmedietelyPlayAt(RulesClass::Instance->RepairBridgeSound, targetCoords, nullptr);

	if (foundWoodBridge) // Repair wood bridges
		MapClass::Instance->RepairWoodBridgeAt(baseCell);
	else // Repair concrete bridges
		MapClass::Instance->RepairConcreteBridgeAt(baseCell);
}

void WarheadTypeExtData::GetCritChance(TechnoClass* pFirer, double& chances) const
{
	chances = this->Crit_Chance;

	if (!pFirer)
	{
		return;
	}

	const auto pExt = TechnoExtContainer::Instance.Find(pFirer);

	if (pExt->AE.ExtraCrit.Enabled())
	{
		std::vector<AEProperties::ExtraCrit::CritDataOut> valids;
		pExt->AE.ExtraCrit.FillEligible(This(), valids);
		chances = AEProperties::ExtraCrit::Count(chances, valids);
	}
}

void WarheadTypeExtData::ApplyAttachEffects(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker)
{
	if (!pTarget)
		return;

	auto const info = &this->PhobosAttachEffects;
	PhobosAttachEffectClass::Attach(pTarget, pInvokerHouse, pInvoker, This(), info);
	PhobosAttachEffectClass::Detach(pTarget, info);
	PhobosAttachEffectClass::DetachByGroups(pTarget, info);
}

bool WarheadTypeExtData::IsHealthInThreshold(ObjectClass* pTarget) const
{
	if(!(this->AffectsAbovePercent > 0.0 || this->AffectsBelowPercent < 1.0))
		return true;

	return TechnoExtData::IsHealthInThreshold(pTarget, this->AffectsAbovePercent, this->AffectsBelowPercent);
}

bool WarheadTypeExtData::ApplySuppressDeathWeapon(TechnoClass* pVictim) const
{
	auto const absType = pVictim->WhatAmI();
	auto const pVictimType = GET_TECHNOTYPE(pVictim);

	if (!this->SuppressDeathWeapon_Exclude.Contains(pVictimType)) {

		if (this->SuppressDeathWeapon.Contains(pVictimType)){

			if (absType == UnitClass::AbsID && !this->SuppressDeathWeapon_Vehicles)
				return false;

			if (absType == InfantryClass::AbsID && !this->SuppressDeathWeapon_Infantry)
				return false;

			if (this->SuppressDeathWeapon_Chance.isset())
				return ScenarioClass::Instance->Random.RandomDouble() >= Math::abs(this->SuppressDeathWeapon_Chance.Get());

			return true;
		}
	}

	return false;
}

void WarheadTypeExtData::ApplyBuildingUndeploy(TechnoClass* pTarget) {
	const auto pBuilding = cast_to<BuildingClass*>(pTarget);

	if (!pBuilding || !pBuilding->IsAlive || pBuilding->Health <= 0 || !pBuilding->IsOnMap || pBuilding->InLimbo)
		return;

	// Higher priority for selling

	if (this->BuildingSell) {

		if ((pBuilding->CanBeSold() && !pBuilding->IsStrange()) || this->BuildingSell_IgnoreUnsellable) {
			pBuilding->SetArchiveTarget(nullptr); // Reset to ensure it must to be sold
			pBuilding->Sell(1);
		}

		return;
	}

	// CanBeSold() will also check this
	const auto mission = pBuilding->CurrentMission;

	if (mission == Mission::Construction || mission == Mission::Selling)
		return;

	const auto pType = pBuilding->Type;
	if (!pType->UndeploysInto || (pType->ConstructionYard && !GameModeOptionsClass::Instance->MCVRedeploy))
		return;

	auto cell = pBuilding->GetMapCoords();
	const auto width = pType->GetFoundationWidth();
	const auto height = pType->GetFoundationHeight(false);

	// Offset of undeployment on large-scale buildings
	if (width > 2 || height > 2)
		cell += CellStruct{ 1, 1 };

	if (this->BuildingUndeploy_Leave)
	{

		const auto pHouse = pBuilding->Owner;
		const auto pItems = Helpers::Alex::getCellSpreadItems(pBuilding->GetCoords(), 20, false, false , false , true , false);


		// Divide the surrounding units into 16 directions and record their costs

		int record[16] = { 0 };

		for (const auto& pItem : pItems)
		{
			// Only armed units that are not considered allies will be recorded

			if ((!pHouse || !pHouse->IsAlliedWith(pItem)) && pItem->IsArmed())
				record[pBuilding->GetDirectionOverObject(pItem).GetValue<4>()] 
				+= GET_TECHNOTYPE(pItem)->Cost;

		}


		int costs = 0;
		int dir = 0;


		// Starting from 16, prevent negative numbers

		for (int i = 16; i < 32; ++i)
		{

			int newCosts = 0;

			// Assign weights to values in the direction

			// The highest value being 8 times in the positive direction

			// And the lowest value being 0 times in the opposite direction

			// The greater difference between positive direction to both sides, the lower value it is

			for (int j = -7; j < 8; ++j)
				newCosts += ((8 - Math::abs(j)) * record[(i + j) & 15]);



			// Record the direction with the highest weight

			if (newCosts > costs)
			{

				dir = (i - 16);
				costs = newCosts;
			}
		}


		// If there is no threat in the surrounding area, randomly select one side

		if (!costs)
			dir = ScenarioClass::Instance->Random.RandomRanged(0, 15);


		// Reverse the direction and convert it into radians

		const double radian = -(((dir - 4) / 16.0) * Math::GAME_TWOPI);


		// Base on a location about 14 grids away

		cell.X -= static_cast<short>(14 * Math::cos(radian));
		cell.Y += static_cast<short>(14 * Math::sin(radian));


		// Find a location where the conyard can be deployed
		const auto newCell = MapClass::Instance->NearByLocation(cell, pType->UndeploysInto->SpeedType, ZoneType::None, pType->UndeploysInto->MovementZone, false, (width + 2), (height + 2), false, false, false, false, CellStruct::Empty, false, false);


		// If it can find a more suitable location, go to the new one
		if (newCell != CellStruct::Empty)

			cell = newCell;

	}



	if (const auto pCell = MapClass::Instance->TryGetCellAt(cell))
		pBuilding->SetArchiveTarget(pCell);

	pBuilding->Sell(1);
}

// =============================
// container
WarheadTypeExtContainer WarheadTypeExtContainer::Instance;

void WarheadTypeExtContainer::Clear()
{
	this->base_t::Clear();
	WarheadTypeExtData::IonBlastExt.clear();
}

bool WarheadTypeExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(WarheadTypeExtContainer::ClassName))
	{
		auto& container = root[WarheadTypeExtContainer::ClassName];

		for (auto& entry : container[WarheadTypeExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, WarheadTypeExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		size_t dataSize = container["IonBlastExt_datasize"].get<size_t>();
		std::string encoded = container["IonBlastExt_data"].get<std::string>();

		PhobosByteStream loader(dataSize);
		loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
		PhobosStreamReader reader(loader);

		reader.Process(WarheadTypeExtData::IonBlastExt);

		if (!reader.ExpectEndOfBlock())
			return false;

		return true;
	}

	return false;

}

bool WarheadTypeExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[WarheadTypeExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : WarheadTypeExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[WarheadTypeExtData::ClassName] = std::move(_extRoot);

	PhobosByteStream saver(0);
	PhobosStreamWriter writer(saver);

	writer.Process(WarheadTypeExtData::IonBlastExt);

	first_layer["IonBlastExt_datasize"] = saver.data.size();
	first_layer["IonBlastExt_data"] = Base64Handler::encodeBase64(saver.data);

	return true;
}

void WarheadTypeExtContainer::LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}

		//load anywhere other than rules
		ptr->LoadFromINI(pINI, parseFailAddr);
		//this function can be called again multiple time but without need to re-init the data
		ptr->SetInitState(InitState::Ruled);
	}

}

void WarheadTypeExtContainer::WriteToINI(ext_t::base_type* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key))
	{
		if (!pINI)
		{
			return;
		}

		ptr->WriteToINI(pINI);
	}
}

// =============================
// container hooks

ASMJIT_PATCH(0x75D1A9, WarheadTypeClass_CTOR, 0x7)
{
	GET(WarheadTypeClass*, pItem, EBP);
	WarheadTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x75E5C8, WarheadTypeClass_SDDTOR, 0x6)
{
	GET(WarheadTypeClass*, pItem, ESI);
	WarheadTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

bool FakeWarheadTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	WarheadTypeExtContainer::Instance.Find(this)->Initialize();
	bool status = this->WarheadTypeClass::LoadFromINI(pINI);
	WarheadTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6B94, FakeWarheadTypeClass::_ReadFromINI)
