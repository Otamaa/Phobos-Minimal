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
#include <Utilities/Helpers.h>

#include <New/Entity/FlyingStrings.h>

PhobosMap<IonBlastClass*, WarheadTypeExt::ExtData*> WarheadTypeExt::IonBlastExt;

void WarheadTypeExt::ExtData::Initialize()
{
	Launchs.reserve(2);
	SuppressDeathWeapon.reserve(8);
	SuppressDeathWeapon_Exclude.reserve(8);
	LimboKill_IDs.reserve(8);
	DetonatesWeapons.reserve(8);
	AttachTag_Types.reserve(8);
	AttachTag_Ignore.reserve(8);
	DeadBodies.reserve(8);
	SquidSplash.reserve(RulesClass::Instance->SplashList.Count);
	DebrisAnimTypes.reserve(RulesClass::Instance->MetallicDebris.Count);
	DetonateOnAllMapObjects_IgnoreTypes.reserve(8);
	DetonateOnAllMapObjects_AffectTypes.reserve(8);
	Shield_AffectTypes.reserve(8);
	Shield_AttachTypes.reserve(8);
	Shield_RemoveTypes.reserve(8);
	InfDeathAnims.reserve(200);
	IsNukeWarhead = IS_SAME_STR_N(RulesExt::Global()->NukeWarheadName.data(), this->Get()->ID);
}

void WarheadTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
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

	if (!pINI->GetSection(pSection)) {	
		return;
	}

	// writing custom verses parser just because
	if (exINI.ReadString(pSection, GameStrings::Verses()))
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

	this->Shield_Penetrate_Types.Read(exINI, pSection, "Shield.Penetrate.Types");
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

	this->Parasite_DisableRocking.Read(exINI, pSection, "Parasite.DisableRocking");
	this->Parasite_GrappleAnim.Read(exINI, pSection, "Parasite.GrappleAnim");
	this->Parasite_ParticleSys.Read(exINI, pSection, "Parasite.ParticleSystem");
	this->Parasite_TreatInfantryAsVehicle.Read(exINI, pSection, "Parasite.TreatInfantryAsVehicle");
	this->Parasite_InvestationWP.Read(exINI, pSection, "Parasite.DamagingWeapon");
	this->Parasite_Damaging_Chance.Read(exINI, pSection, "Parasite.DamagingChance");

	for (auto const& ArmorType : ArmorTypeClass::Array)
	{
		Nullable<AnimTypeClass*> pAnimReaded;
		pAnimReaded.Read(exINI, pSection, ArmorType->HitAnim_Tag.data(), true);

		if(pAnimReaded.isset() && pAnimReaded != nullptr)
			ArmorHitAnim[ArmorType.get()] = pAnimReaded.Get();
	}

	this->IsNukeWarhead.Read(exINI, pSection, "IsNukeWarhead");

	this->PreImpactAnim.Read(exINI, pSection, "PreImpactAnim");
	if (this->IsNukeWarhead && !this->PreImpactAnim.isset()) {
		this->PreImpactAnim = AnimTypeClass::Find(GameStrings::NUKEBALL());
	}

	this->NukeFlashDuration.Read(exINI, pSection, "NukeFlash.Duration");
	if (this->IsNukeWarhead && !this->NukeFlashDuration.isset()) {
		this->NukeFlashDuration = 30;
	}

	this->Remover.Read(exINI, pSection, "Remover");
	this->Remover_Anim.Read(exINI, pSection, "Remover.Anim");
	this->PermaMC.Read(exINI, pSection, "MindControl.Permanent");
	this->Sound.Read(exINI, pSection, GameStrings::Sound());

	this->Converts.Read(exINI, pSection, "Converts");
	this->ConvertsPair.Read(exINI, pSection, "ConvertsPair");

	this->DeadBodies.Read(exINI, pSection, "DeadBodies");
	this->AffectEnemies_Damage_Mod.Read(exINI, pSection, "AffectEnemies.DamageModifier");
	this->AffectOwner_Damage_Mod.Read(exINI, pSection, "AffectOwner.DamageModifier");
	this->AffectAlly_Damage_Mod.Read(exINI, pSection, "AffectAlly.DamageModifier");

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
	this->SuppressDeathWeapon_Chance.Read(exINI ,pSection, "DeathWeapon.SuppressChance", true);

	this->DeployedDamage.Read(exINI, pSection, "Damage.Deployed");
	this->Temporal_WarpAway.Read(exINI, pSection, "Temporal.WarpAway");
	this->Supress_LostEva.Read(exINI, pSection, "UnitLost.Suppress");
	this->Temporal_HealthFactor.Read(exINI, pSection, "Temporal.HealthFactor");

#ifdef COMPILE_PORTED_DP_FEATURES_
	auto ReadHitTextData = [this, &exINI, pSection](const char* pBaseKey, bool bAllocate = true)
	{
		char tempBuffer[0x80];
		for (size_t i = 0; i < ArmorTypeClass::Array.size(); ++i)
		{
			const auto pArmor = ArmorTypeClass::Array[i].get();
			IMPL_SNPRNINTF(tempBuffer, sizeof(flag), "%s.%s.", pBaseKey, pArmor->Name.data());
			DamageTextPerArmor[i].Read(exINI, pSection, tempBuffer);
		}
	};

	ReadHitTextData("DamageText");

#endif

	this->PaintBallDuration.Read(exINI, pSection, "PaintBall.Duration");
	this->PaintBallData.Read(exINI, pSection);
#pragma endregion

	this->AttachedEffect.Read(exINI);
	this->DetonatesWeapons.Read(exINI, pSection, "DetonatesWeapons");
	this->LimboKill_IDs.Read(exINI, pSection, "LimboKill.IDs");
	this->LimboKill_Affected.Read(exINI, pSection, "LimboKill.Affected");
	this->InfDeathAnim.Read(exINI, pSection, "InfDeathAnim");
	this->Culling_BelowHP.Read(exINI, pSection, "Culling.%sBelowHealth");
	this->Culling_Chance.Read(exINI, pSection, "Culling.%sChance");

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
	this->Launchs.clear();
	for (size_t i = 0; ; ++i)
	{
		char nBuff[0x80];
		Nullable<SuperWeaponTypeClass*> LaunchWhat_Dummy { };
		IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Type", i);
		LaunchWhat_Dummy.Read(exINI, pSection, nBuff, true);

		if (!LaunchWhat_Dummy.isset() || !LaunchWhat_Dummy.Get())
			break;

		LauchSWData nData;
		if (!nData.Read(exINI, pSection, i, LaunchWhat_Dummy))
			break;

		this->Launchs.push_back(std::move(nData));
	}

	this->Conventional_IgnoreUnits.Read(exINI, pSection, "Conventional.IgnoreUnits");

	this->InflictLocomotor.Read(exINI, pSection, "InflictLocomotor");
	this->RemoveInflictedLocomotor.Read(exINI, pSection, "RemoveInflictedLocomotor");
	this->Rocker_Damage.Read(exINI, pSection, "Rocker.Damage");
	this->NukePayload_LinkedSW.Read(exINI, pSection, "NukePayload.LinkedSW");
	this->IC_Duration.Read(exINI, pSection, "IronCurtain.Duration");
	this->IC_Cap.Read(exINI, pSection, "IronCurtain.Cap");

#pragma region Ion
	this->Ion.Read(exINI, pSection, "IonCannon");

	Nullable<int> Ion_ripple{};

	Ion_ripple.Read(exINI, pSection, "Ripple.Radius");
	if (Ion_ripple.isset())
		this->Ripple_Radius = this->Ripple_Radius.Get();
	else
		this->Ripple_Radius.Read(exINI, pSection, "IonCannon.Ripple");

	this->Ion_Beam.Read(exINI, pSection, "IonCannon.Beam");
	this->Ion_Blast.Read(exINI, pSection, "IonCannon.Blast");
	this->Ion_AllowWater.Read(exINI, pSection, "IonCannon.AllowWater");
	this->Ion_Rocking.Read(exINI, pSection, "IonCannon.Rock");
	this->Ion_WH.Read(exINI, pSection, "IonCannon.Warhead");
	this->Ion_Damage.Read(exINI, pSection, "IonCannon.Damage");

	this->DetonateParticleSystem.Read(exINI, pSection, "DetonateParticleSystems");
#pragma endregion

	if (this->InflictLocomotor && pThis->Locomotor == _GUID())
	{
		Debug::Log("[Developer warning][%s] InflictLocomotor is specified but Locomotor is not set!", pSection);
		this->InflictLocomotor = false;
	}

	if ((this->InflictLocomotor || this->RemoveInflictedLocomotor) && pThis->IsLocomotor)
	{
		Debug::Log("[Developer warning][%s] InflictLocomotor=yes/RemoveInflictedLocomotor=yes can't be specified while IsLocomotor is set!", pSection);
		this->InflictLocomotor = this->RemoveInflictedLocomotor = false;
	}

	if (this->InflictLocomotor && this->RemoveInflictedLocomotor)
	{
		Debug::Log("[Developer warning][%s] InflictLocomotor=yes and RemoveInflictedLocomotor=yes can't be set simultaneously!", pSection);
		this->InflictLocomotor = this->RemoveInflictedLocomotor = false;
	}
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

void WarheadTypeExt::ExtData::EvaluateArmor(WarheadTypeClass* OwnerObject)
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

void WarheadTypeExt::ExtData::ApplyRecalculateDistanceDamage(ObjectClass* pVictim, args_ReceiveDamage* pArgs)
{
	if ((pVictim->AbstractFlags & AbstractFlags::Techno) == AbstractFlags::None)
		return;

	const auto pVictimTechno = static_cast<TechnoClass*>(pVictim);

	if (!this->RecalculateDistanceDamage.Get() || !pArgs->Attacker)
		return;

	if (!this->RecalculateDistanceDamage_IgnoreMaxDamage && *pArgs->Damage == RulesClass::Instance->MaxDamage)
		return;

	const auto pThisType = pVictimTechno->GetTechnoType();
	const auto range = pArgs->Attacker->DistanceFrom(pVictim);
	const auto range_factor = range / (this->RecalculateDistanceDamage_Add_Factor.Get() * 256);
	const auto add = (this->RecalculateDistanceDamage_Add.Get() * range_factor);

	const auto multiply = std::pow((this->RecalculateDistanceDamage_Multiply.Get()), range_factor);

	auto nAddDamage = add * multiply;
	if (this->RecalculateDistanceDamage_ProcessVerses)
		nAddDamage *=
		// GeneralUtils::GetWarheadVersusArmor(this->Get() , pThisType->Armor)
		this->GetVerses(pThisType->Armor).Verses
		;

	auto const nEligibleAddDamage = std::clamp((int)nAddDamage,
		this->RecalculateDistanceDamage_Min.Get(), this->RecalculateDistanceDamage_Max.Get());

	*pArgs->Damage += nEligibleAddDamage;

	if (this->RecalculateDistanceDamage_Display || Phobos::Debug_DisplayDamageNumbers)
	{
		TechnoClass* pOwner = this->RecalculateDistanceDamage_Display_AtFirer ? pArgs->Attacker : pVictimTechno;
		FlyingStrings::AddMoneyString(true, *pArgs->Damage, pOwner,
			AffectedHouse::All, pOwner->Location,
			this->RecalculateDistanceDamage_Display_Offset, Drawing::DefaultColors[(int)DefaultColorList::Yellow]);
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
		const auto pType = pTechno->GetTechnoType();

		if (pTechno->InLimbo
			|| !pTechno->IsOnMap
			|| !pTechno->IsAlive
			|| !pTechno->Health
			|| pTechno->IsSinking
			|| !pType
			)
			return false;

		if (pType->Immune)
			return false;

		if (auto const pBld = specific_cast<BuildingClass*>(pTechno))
		{
			auto const pBldExt = BuildingExt::ExtMap.Find(pBld);

			if (this->LimboKill_IDs.empty() && pBldExt->LimboID != -1)
			{
				return false;
			}

			if (pBld->Type->InvisibleInGame)
				return false;
		}

		if (const auto pFoot = abstract_cast<FootClass*>(pTechno))
		{
			if (TechnoExt::IsChronoDelayDamageImmune(pFoot))
				return false;
		}

		if (pTechno->IsBeingWarpedOut())
			return false;

		if (!SkipVerses && EffectsRequireVerses.Get()) {
			return (std::abs(this->GetVerses(TechnoExt::GetTechnoArmor(pTechno , this->Get())).Verses) >= 0.001);
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
			if (MapClass::GetTotalDamage(RulesClass::Instance->MaxDamage, Get(), nArmor, 0) == 0.0)
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
	if (!EnumFunctions::IsTechnoEligibleB(pTechno, this->DetonateOnAllMapObjects_AffectTargets))
		return FullMapDetonateResult::TargetNotEligible;

	if (!CanDealDamage(pTechno, false, !this->DetonateOnAllMapObjects_RequireVerses.Get()))
		return FullMapDetonateResult::TargetNotDamageable;

	auto const pType = pTechno->GetTechnoType();

	if (!EnumFunctions::CanTargetHouse(this->DetonateOnAllMapObjects_AffectHouses, pOwner, pTechno->Owner))
		return FullMapDetonateResult::TargetHouseNotEligible;

	if(!this->DetonateOnAllMapObjects_AffectTypes.empty() 
		&& !this->DetonateOnAllMapObjects_AffectTypes.Contains(pType))
		return FullMapDetonateResult::TargetRestricted;

	if (!this->DetonateOnAllMapObjects_IgnoreTypes.empty() &&
		this->DetonateOnAllMapObjects_IgnoreTypes.Contains(pType))
		return FullMapDetonateResult::TargetRestricted;

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

bool WarheadTypeExt::ExtData::ApplyCulling(TechnoClass* pSource, ObjectClass* pTarget) const
{
	auto const pThis = OwnerObject();

	if (!pThis->Culling || !pSource)
		return false;

	if (auto const pTargetTechno = generic_cast<TechnoClass*>(pTarget))
	{
		if (TechnoExt::IsCullingImmune(pTargetTechno))
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

void WarheadTypeExt::ExtData::ApplyRelativeDamage(ObjectClass* pTarget, args_ReceiveDamage* pArgs) const
{
	if (!this->RelativeDamage)
		return;

	{
		auto const pWhat = GetVtableAddr(pTarget);
		int nRelativeVal = 0;

		switch (pWhat)
		{
		case UnitClass::vtable:
		{
			const auto pUnit = static_cast<UnitClass*>(pTarget);

			if (pUnit->Type->ConsideredAircraft)
				nRelativeVal = RelativeDamage_AirCraft.Get();
			else if (pUnit->Type->Organic)
				nRelativeVal = RelativeDamage_Infantry.Get();
			else
				nRelativeVal = RelativeDamage_Unit.Get();
		}
		break;
		case AircraftClass::vtable:
		{
			const auto pAir = static_cast<AircraftClass*>(pTarget);

			if (pAir->Type->Organic)
				nRelativeVal = RelativeDamage_Infantry.Get();
			else
				nRelativeVal = RelativeDamage_AirCraft.Get();
		}
		break;
		case BuildingClass::vtable:
			nRelativeVal = RelativeDamage_Building.Get();
			break;
		case InfantryClass::vtable:
			nRelativeVal = RelativeDamage_Infantry.Get();
			break;
		case TerrainClass::vtable:
			nRelativeVal = RelativeDamage_Terrain.Get();
			break;
		}

		if (nRelativeVal)
		{

			if (nRelativeVal < 0)
				nRelativeVal *= pTarget->Health / -100;
			else
				if (const auto pType = pTarget->GetType())
					nRelativeVal *= pType->Strength / 100;
		}

		if (*pArgs->Damage < 0)
			nRelativeVal = -nRelativeVal;

		*pArgs->Damage = nRelativeVal;
	}
}

bool WarheadTypeExt::ExtData::GoBerzerkFor(FootClass* pVictim, int* damage)
{
	int nDur = this->Berzerk_dur.Get(*damage);
	auto const pType = pVictim->GetTechnoType();

	if (nDur != 0)
	{
		if (nDur > 0)
		{
			if (auto pData = TechnoTypeExt::ExtMap.Find(pType))
			{
				nDur = static_cast<int>(nDur * pData->Berzerk_Modifier.Get());
			}
		}

		//Default way game modify duration
		nDur = MapClass::GetTotalDamage(nDur, this->OwnerObject(), pType->Armor, 0);

		const int oldValue = (!pVictim->Berzerk ? 0 : pVictim->BerzerkDurationLeft);
		const int newValue = Helpers::Alex::getCappedDuration(oldValue, nDur, this->Berzerk_cap.Get());

		//auto const underBrzBefore = (oldValue > 0);
		//auto const underBrzAfter = (pVictim->Berzerk && pVictim->BerzerkDurationLeft > 0);
		//auto const newlyUnderBrz = !underBrzBefore && underBrzAfter;
		if (oldValue == newValue)
			return this->Berzerk_dealDamage.Get();

		if (oldValue <= 0)
		{
			if (newValue > 0)
			{
				pVictim->GoBerzerkFor(newValue);
			}
		}
		else
		{

			if (newValue > 0)
			{
				pVictim->GoBerzerkFor(newValue);
			}
			else
			{

				auto const nLeft = pVictim->BerzerkDurationLeft - newValue;
				if (nLeft <= 0)
				{
					pVictim->BerzerkDurationLeft = 0;
					pVictim->Berzerk = false;
					pVictim->SetTarget(nullptr);
					TechnoExt::SetMissionAfterBerzerk(pVictim);
				}
				else
				{
					pVictim->BerzerkDurationLeft -= nLeft;
				}
			}
		}

		return this->Berzerk_dealDamage.Get();
	}

	return false; //default
}

AnimTypeClass* WarheadTypeExt::ExtData::GetArmorHitAnim(int Armor)
{
	const auto pArmor = ArmorTypeClass::Array[Armor].get();

	if (this->ArmorHitAnim.empty())
		return nullptr;

	if (this->ArmorHitAnim.contains(pArmor))
		return this->ArmorHitAnim[pArmor];
	else if (pArmor->DefaultTo != -1)
	{
		for (auto pDefArmor = ArmorTypeClass::Array[pArmor->DefaultTo].get();
			pDefArmor && pDefArmor->DefaultTo != -1;
			pDefArmor = ArmorTypeClass::Array[pDefArmor->DefaultTo].get())
		{
			if (this->ArmorHitAnim.contains(pDefArmor))
				return this->ArmorHitAnim[pDefArmor];
		}
	}

	return nullptr;
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage, bool targetCell)
{
	if (targetCell && !pTarget)
	{
		Debug::Log("WarheadTypeExt::Detonate asking for targetCell but pTarget is nullptr ! \n");
		return;
	}

	AbstractClass* pATarget = !targetCell ? static_cast<AbstractClass*>(pTarget) : pTarget->GetCell();
	WarheadTypeExt::DetonateAt(pThis, pATarget, CoordStruct::Empty, pOwner, damage);
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool targetCell)
{
	if (targetCell && !coords)
	{
		Debug::Log("WarheadTypeExt::Detonate asking for targetCell but coords is invalid ! \n");
		return;
	}

	AbstractClass* pTarget = !targetCell ? nullptr : coords ? MapClass::Instance->GetCellAt(coords) : nullptr;
	WarheadTypeExt::DetonateAt(pThis, pTarget, coords, pOwner, damage);
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

	if (pOwner && !Is_Techno(pOwner)) {	
		Debug::Log("WarheadTypeExt::DetonateAt[%s] delivering damage from unknown source [%x] !", pThis->get_ID(), pOwner);
		pOwner = nullptr;
	}
	if (BulletClass* pBullet = BulletTypeExt::ExtMap.Find(pType)->CreateBullet(pTarget, pOwner,
		damage, pThis, 0, 0, pThis->Bright, true))
	{
		pBullet->MoveTo(coords, { 0 ,0, 0 });

		//something like 0x6FF08B
		const auto pCellCoord = MapClass::Instance->GetCellAt(coords);
		if (pCellCoord->ContainsBridge())
			pBullet->OnBridge = true;

		BulletExt::DetonateAt(pBullet, pTarget, pOwner, coords);
	}
}

void WarheadTypeExt::CreateIonBlast(WarheadTypeClass* pThis, const CoordStruct& coords)
{
	const auto pExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pExt->Ion || pExt->Ripple_Radius.isset() && pExt->Ripple_Radius > 0)
	{
		auto pIon = GameCreate<IonBlastClass>(coords);
		pIon->DisableIonBeam = !pExt->Ion;
		IonBlastExt.insert(pIon, pExt);
	}
}
// =============================
// load / save

template <typename T>
void WarheadTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
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
		.Process(this->Shield_Penetrate_Types)
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

		.Process(this->GattlingStage)
		.Process(this->GattlingRateUp)
		.Process(this->ReloadAmmo)

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
		.Process(this->Converts)
		.Process(this->ConvertsPair)
		.Process(this->StealMoney)
		.Process(this->Steal_Display_Houses)
		.Process(this->Steal_Display)
		.Process(this->Steal_Display_Offset)
		.Process(this->DeadBodies)
		.Process(this->AffectEnemies_Damage_Mod)
		.Process(this->AffectOwner_Damage_Mod)
		.Process(this->AffectAlly_Damage_Mod)
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

		.Process(this->Culling_BelowHP)
		.Process(this->Culling_Chance)

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
		.Process(this->Rocker_Damage)
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
#ifdef COMPILE_PORTED_DP_FEATURES_
		.Process(DamageTextPerArmor)

#endif
		.Process(PaintBallDuration)
		;

	PaintBallData.Serialize(Stm);
}

bool WarheadTypeExt::ExtData::ApplySuppressDeathWeapon(TechnoClass* pVictim)
{
	auto const absType = GetVtableAddr(pVictim);
	auto const pVictimType = pVictim->GetTechnoType();

	if (SuppressDeathWeapon_Exclude.Contains(pVictimType) )
		return false;

	if (absType == UnitClass::vtable && !SuppressDeathWeapon_Vehicles)
		return false;

	if (absType == InfantryClass::vtable && !SuppressDeathWeapon_Infantry)
		return false;

	if(!SuppressDeathWeapon.Contains(pVictimType))
	 	return false;

	if(SuppressDeathWeapon_Chance.isset())
		return ScenarioClass::Instance->Random.RandomDouble() >= abs(SuppressDeathWeapon_Chance.Get());

	return true;
}

// =============================
// container
WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;

WarheadTypeExt::ExtContainer::ExtContainer() : Container("WarheadTypeClass") { }
WarheadTypeExt::ExtContainer::~ExtContainer() = default;

bool WarheadTypeExt::ExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(WarheadTypeExt::IonBlastExt)
		.Success();
}

bool WarheadTypeExt::ExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(WarheadTypeExt::IonBlastExt)
		.Success();
}

void WarheadTypeExt::ExtContainer::Clear()
{
	WarheadTypeExt::IonBlastExt.clear();
}

//void WarheadTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks

DEFINE_HOOK(0x75D1A9, WarheadTypeClass_CTOR, 0x7)
{
	GET(WarheadTypeClass*, pItem, EBP);
	WarheadTypeExt::ExtMap.Allocate(pItem);
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

 // is return not valid
DEFINE_HOOK_AGAIN(0x75DEAF, WarheadTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x75DEA0 , WarheadTypeClass_LoadFromINI, 0x5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x75DEAF);

	//0x75DE9A do net set isOrganic here , just skip it to next adrress to execute ares hook
	return// R->Origin() == 0x75DE9A ? 0x75DEA0 : 
		0;
}
