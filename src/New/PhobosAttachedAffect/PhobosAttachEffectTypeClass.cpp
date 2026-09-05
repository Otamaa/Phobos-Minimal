#include "PhobosAttachEffectTypeClass.h"

Enumerable<PhobosAttachEffectTypeClass>::container_t Enumerable<PhobosAttachEffectTypeClass>::Array;
PhobosMap<std::string, GroupData> PhobosAttachEffectTypeClass::GroupsMap;

void PhobosAttachEffectTypeClass::AddToGroupsMap()
{
    for (const auto& group : this->Groups)  {
        PhobosAttachEffectTypeClass::GroupsMap[group].insert(this);
    }
}

std::vector<PhobosAttachEffectTypeClass*> PhobosAttachEffectTypeClass::GetTypesFromGroups(std::vector<std::string>& groupIDs)
{
	std::vector<PhobosAttachEffectTypeClass*> types;
	auto map = &PhobosAttachEffectTypeClass::GroupsMap;

	types.reserve(map->size() * 10);

	for (const auto& group : groupIDs) {
		auto iter = map->get_key_iterator(group);
		if (iter != map->end()) {
			types.insert(types.end(),iter->second.begin(), iter->second.end());
		}
	}

	std::ranges::sort(types);
	types.erase(std::ranges::unique(types).begin(), types.end());

	return types;
}

void PhobosAttachEffectTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name.c_str();

	if (IS_SAME_STR_N(pSection, GameStrings::NoneStr()))
		return;

	INI_EX exINI(pINI);

	this->Duration.Read(exINI, pSection, "Duration");
	this->Duration_ApplyFirepowerMult.Read(exINI, pSection, "Duration.ApplyFirepowerMult");
	this->Duration_ApplyArmorMultOnTarget.Read(exINI, pSection, "Duration.ApplyArmorMultOnTarget");
	this->Duration_ApplyVersus_Warhead.Read(exINI, pSection, "Duration.ApplyVersusWarhead");
	this->Cumulative.Read(exINI, pSection, "Cumulative");
	this->Cumulative_MaxCount.Read(exINI, pSection, "Cumulative.MaxCount");
	this->Powered.Read(exINI, pSection, "Powered");

	this->DiscardOn.Read(exINI, pSection, "DiscardOn");
	this->DiscardOn_RangeOverride.Read(exINI, pSection, "DiscardOn.RangeOverride");
	this->DiscardOn_MoveBasedOnDestination.Read(exINI, pSection, "DiscardOn.MoveBasedOnDestination");
	this->DiscardOn_ConsiderHarvestingAsStationary.Read(exINI, pSection, "DiscardOn.ConsiderHarvestingAsStationary");
	this->DiscardOn_Ammo_Min.Read(exINI, pSection, "DiscardOn.Ammo.MinimumAmount");
	this->DiscardOn_Ammo_Max.Read(exINI, pSection, "DiscardOn.Ammo.MaximumAmount");

	if (this->DiscardOn_Ammo_Min > this->DiscardOn_Ammo_Max)
		Debug::Log("[Developer warning][%s] DiscardOn.Ammo.MinimumAmount is greater than DiscardOn.Ammo.MaximumAmount, the ammo discard condition cannot be established.\n", pSection);

	this->DiscardOn_Health_Min.Read(exINI, pSection, "DiscardOn.Health.BelowPercent");
	this->DiscardOn_Health_Max.Read(exINI, pSection, "DiscardOn.Health.AbovePercent");

	if (this->DiscardOn_Health_Max.isset() && this->DiscardOn_Health_Min.isset() && this->DiscardOn_Health_Max.Fetch() > this->DiscardOn_Health_Min.Fetch())
		Debug::Log("[Developer warning][%s] DiscardOn.Health.AbovePercent is greater than DiscardOn.Health.BelowPercent, the health discard condition cannot be established.\n", pSection);

	this->DiscardOn_Firing_Count.Read(exINI, pSection, "DiscardOn.Firing.Count");
	this->DiscardOn_ReceivedDamage_Count.Read(exINI, pSection, "DiscardOn.ReceivedDamage.Count");
	this->DiscardOn_ReceivedDamage_AffectsHouse.Read(exINI, pSection, "DiscardOn.ReceivedDamage.AffectsHouse");
	this->DiscardOn_Missions.Read(exINI, pSection, "DiscardOn.Missions");
	this->DiscardOn_AIMissions.Read(exINI, pSection, "DiscardOn.AIMissions");

	this->DiscardOn_LandTypes.Read(exINI, pSection, "DiscardOn.LandTypes");

	this->DiscardOn_Sequences.Read(exINI, pSection, "DiscardOn.Sequences");
	this->DiscardOn_Sequences_Immediate.Read(exINI, pSection, "DiscardOn.Sequences.Immediate");

	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");
	this->PenetratesForceShield.Read(exINI, pSection, "PenetratesForceShield");
	this->Animation.Read(exINI, pSection, "Animation");
	this->CumulativeAnimations.Read(exINI, pSection, "CumulativeAnimations");
	this->CumulativeAnimations_RestartOnChange.Read(exINI, pSection, "CumulativeAnimations.RestartOnChange");
	this->Animation_ResetOnReapply.Read(exINI, pSection, "Animation.ResetOnReapply");
	this->Animation_OfflineAction.Read(exINI, pSection, "Animation.OfflineAction");
	this->Animation_TemporalAction.Read(exINI, pSection, "Animation.TemporalAction");
	this->Animation_UseInvokerAsOwner.Read(exINI, pSection, "Animation.UseInvokerAsOwner");
	this->Animation_HideIfAttachedWith.Read(exINI, pSection, "Animation.HideIfAttachedWith");

	this->ExpireWeapon.Read(exINI, pSection, "ExpireWeapon" , true);
	this->ExpireWeapon_TriggerOn.Read(exINI, pSection, "ExpireWeapon.TriggerOn");
	this->ExpireWeapon_CumulativeOnlyOnce.Read(exINI, pSection, "ExpireWeapon.CumulativeOnlyOnce");

	this->Tint_Color.Read(exINI, pSection, "Tint.Color");
	this->Tint_Intensity.Read(exINI, pSection, "Tint.Intensity");
	this->Tint_VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");

	this->FirepowerMultiplier.Read(exINI, pSection, "FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, pSection, "ArmorMultiplier");
	this->ArmorMultiplier_AllowWarheads.Read(exINI, pSection, "ArmorMultiplier.AllowWarheads");
	this->ArmorMultiplier_DisallowWarheads.Read(exINI, pSection, "ArmorMultiplier.DisallowWarheads");
	this->ArmorMultiplier_Chance.Read(exINI, pSection, "ArmorMultiplier.Chance");
	this->ArmorMultiplier_HitAnim.Read(exINI, pSection, "ArmorMultiplier.HitAnim");

	this->SpeedMultiplier.Read(exINI, pSection, "SpeedMultiplier");
	this->ROFMultiplier.Read(exINI, pSection, "ROFMultiplier");
	this->ROFMultiplier_ApplyOnCurrentTimer.Read(exINI, pSection, "ROFMultiplier.ApplyOnCurrentTimer");

	this->Cloakable.Read(exINI, pSection, "Cloakable");
	this->ForceDecloak.Read(exINI, pSection, "ForceDecloak");

	this->WeaponRange_Multiplier.Read(exINI, pSection, "WeaponRange.Multiplier");
	this->WeaponRange_ExtraRange.Read(exINI, pSection, "WeaponRange.ExtraRange");
	this->WeaponRange_AllowWeapons.Read(exINI, pSection, "WeaponRange.AllowWeapons");
	this->WeaponRange_DisallowWeapons.Read(exINI, pSection, "WeaponRange.DisallowWeapons");

	this->Crit_Multiplier.Read(exINI, pSection, "Crit.Multiplier");
	this->Crit_ExtraChance.Read(exINI, pSection, "Crit.ExtraChance");
	this->Crit_AllowWarheads.Read(exINI, pSection, "Crit.AllowWarheads");
	this->Crit_DisallowWarheads.Read(exINI, pSection, "Crit.DisallowWarheads");

	this->RevengeWeapon.Read(exINI, pSection, "RevengeWeapon", true);
	this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouses");
	this->ReceiveRelativeDamageMult.Read(exINI, pSection, "ReceiveRelativeDamageMultiplier");
	this->DisableWeapons.Read(exINI, pSection, "DisableWeapons");

	// Groups
	this->Groups.Read(exINI, pSection, "Groups");
	this->AddToGroupsMap();

	this->DisableSelfHeal.Read(exINI, pSection, "DisableSelfHeal");
	this->Untrackable.Read(exINI, pSection, "Untrackable");
	this->AnimRandomPick.Read(exINI, pSection, "Anim.RandomPick");

	this->ReflectDamage.Read(exINI, pSection, "ReflectDamage");
	this->ReflectDamage_Warhead.Read(exINI, pSection, "ReflectDamage.Warhead");
	this->ReflectDamage_Warhead_Detonate.Read(exINI, pSection, "ReflectDamage.Warhead.Detonate");
	this->ReflectDamage_Multiplier.Read(exINI, pSection, "ReflectDamage.Multiplier");
	this->ReflectDamage_AffectsHouses.Read(exINI, pSection, "ReflectDamage.AffectsHouses");
	this->ReflectDamage_AffectsHouses.Read(exINI, pSection, "ReflectDamage.AffectsHouse");
	this->ReflectDamage_Chance.Read(exINI, pSection, "ReflectDamage.Chance");
	this->ReflectDamage_Override.Read(exINI, pSection, "ReflectDamage.Override");

	this->DiscardOn_AbovePercent.Read(exINI, pSection, "DiscardOn.AbovePercent");
	this->DiscardOn_BelowPercent.Read(exINI, pSection, "DiscardOn.BelowPercent");
	this->AffectAbovePercent.Read(exINI, pSection, "AffectAbovePercent");
	this->AffectBelowPercent.Read(exINI, pSection, "AffectBelowPercent");

	this->DisableRadar.Read(exINI, pSection, "DisableRadar");
	this->DisableSpySat.Read(exINI, pSection, "DisableSpySat");

	this->Unkillable.Read(exINI, pSection, "Unkillable");

	this->ExtraWarheads.Read(exINI, pSection, "ExtraWarheads");
	this->ExtraWarheads_DamageOverrides.Read(exINI, pSection, "ExtraWarheads.DamageOverrides");
	this->ExtraWarheads_DetonationChances.Read(exINI, pSection, "ExtraWarheads.DetonationChances");
	this->ExtraWarheads_FullDetonation.Read(exINI, pSection, "ExtraWarheads.FullDetonation");

	this->FeedbackWeapon.Read(exINI, pSection, "FeedbackWeapon", true);

	this->ExpireWeapon_UseInvokerAsOwner.Read(exINI, pSection, "ExpireWeapon.UseInvokerAsOwner");
	this->RevengeWeapon_UseInvokerAsOwner.Read(exINI, pSection, "RevengeWeapon.UseInvokerAsOwner");
	this->ReflectDamage_UseInvokerAsOwner.Read(exINI, pSection, "ReflectDamage.UseInvokerAsOwner");

	this->LaserTrail_Type.Read(exINI, pSection, "LaserTrail.Type");
	this->Block_ChanceMultiplier.Read(exINI, pSection, "Block.ChanceMultiplier");
	this->Block_ExtraChance.Read(exINI, pSection, "Block.ExtraChance");

	this->AffectTypes.Read(exINI, pSection, "AffectTypes");
	this->IgnoreTypes.Read(exINI, pSection, "IgnoreTypes");
	this->AffectTargets.Read(exINI, pSection, "AffectTargets");
	this->AffectTargets.Read(exINI, pSection, "AffectsTarget");

	// Animation draw offsets.
	for (int i = 0; i < INT32_MAX; i++) {
		AnimationDrawOffsetClass offset;

		if (!offset.LoadFromINI(pINI, pSection, i))
			break;

		this->Animation_DrawOffsets.emplace_back(std::move(offset));
	}

	this->PeriodicWeapon.Read(exINI, pSection, "PeriodicWeapon", true);
	this->PeriodicWeapon_AffectsHouse.Read(exINI, pSection, "PeriodicWeapon.AffectsHouse");
	this->PeriodicWeapon_UseInvokerAsOwner.Read(exINI, pSection, "PeriodicWeapon.UseInvokerAsOwner");
	this->PeriodicWeapon_Range.Read(exINI, pSection, "PeriodicWeapon.Range");
	this->PeriodicWeapon_FiringDelay.Read(exINI, pSection, "PeriodicWeapon.FiringDelay");
	this->PeriodicWeapon_FireAll.Read(exINI, pSection, "PeriodicWeapon.FireAll");
	this->PeriodicWeapon_AffectTypes.Read(exINI, pSection, "PeriodicWeapon.AffectTypes");
	this->PeriodicWeapon_IgnoreTypes.Read(exINI, pSection, "PeriodicWeapon.IgnoreTypes");

	this->PrismRelay.Read(exINI, pSection, "PrismRelay");
	this->PrismRelay_NetworkID.Read(exINI, pSection, "PrismRelay.NetworkID");
	this->PrismRelay_Provider.Read(exINI, pSection, "PrismRelay.Provider");
	this->PrismRelay_Receiver.Read(exINI, pSection, "PrismRelay.Receiver");
	this->PrismRelay_SupportWeapon.Read(exINI, pSection, "PrismRelay.SupportWeapon");
	this->PrismRelay_MaxReceiveLinks.Read(exINI, pSection, "PrismRelay.MaxReceiveLinks");
	this->PrismRelay_MaxNodeLinks.Read(exINI, pSection, "PrismRelay.MaxNodeLinks");
	this->PrismRelay_SupportFireDelay.Read(exINI, pSection, "PrismRelay.SupportFireDelay");
	this->PrismRelay_SupportMultiplier.Read(exINI, pSection, "PrismRelay.SupportMultiplier");
	this->PrismRelay_DamageAdd.Read(exINI, pSection, "PrismRelay.DamageAdd");
	this->PrismRelay_ToAllies.Read(exINI, pSection, "PrismRelay.ToAllies");
	this->PrismRelay_AllowWeapons.Read(exINI, pSection, "PrismRelay.AllowWeapons");
	this->PrismRelay_DisallowWeapons.Read(exINI, pSection, "PrismRelay.DisallowWeapons");
	this->PrismRelay_MasterWeaponIndex.Read(exINI, pSection, "PrismRelay.MasterWeaponIndex");
	this->PrismRelay_MasterWeaponUseMultiWeaponSelection.Read(exINI, pSection, "PrismRelay.MasterWeaponUseMultiWeaponSelection");
	{
		Valueable<int> passCooldown { -1 };
		passCooldown.Read(exINI, pSection, "PrismRelay.PassCooldown");

		if (passCooldown >= 0)
			this->PrismRelay_SupportTimeout = passCooldown;
		else
			this->PrismRelay_SupportTimeout.Read(exINI, pSection, "PrismRelay.SupportTimeout");
	}
}

template <typename T>
void PhobosAttachEffectTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Duration)
		.Process(this->Duration_ApplyFirepowerMult)
		.Process(this->Duration_ApplyArmorMultOnTarget)
		.Process(this->Duration_ApplyVersus_Warhead)
		.Process(this->Cumulative)
		.Process(this->Cumulative_MaxCount)
		.Process(this->Powered)
		.Process(this->DiscardOn)
		.Process(this->DiscardOn_RangeOverride)
		.Process(this->DiscardOn_MoveBasedOnDestination)
		.Process(this->DiscardOn_ConsiderHarvestingAsStationary)
		.Process(this->DiscardOn_Ammo_Min)
		.Process(this->DiscardOn_Ammo_Max)
		.Process(this->DiscardOn_Health_Min)
		.Process(this->DiscardOn_Health_Max)
		.Process(this->DiscardOn_Firing_Count)
		.Process(this->DiscardOn_ReceivedDamage_Count)
		.Process(this->DiscardOn_ReceivedDamage_AffectsHouse)
		.Process(this->DiscardOn_Missions)
		.Process(this->DiscardOn_AIMissions)
		.Process(this->DiscardOn_LandTypes)
		.Process(this->DiscardOn_Sequences)
		.Process(this->DiscardOn_Sequences_Immediate)
		.Process(this->PenetratesIronCurtain)
		.Process(this->PenetratesForceShield)
		.Process(this->Animation)
		.Process(this->CumulativeAnimations)
		.Process(this->CumulativeAnimations_RestartOnChange)
		.Process(this->Animation_ResetOnReapply)
		.Process(this->Animation_OfflineAction)
		.Process(this->Animation_TemporalAction)
		.Process(this->Animation_UseInvokerAsOwner)
		.Process(this->Animation_HideIfAttachedWith)
		.Process(this->ExpireWeapon)
		.Process(this->ExpireWeapon_TriggerOn)
		.Process(this->ExpireWeapon_CumulativeOnlyOnce)
		.Process(this->ExpireWeapon_UseInvokerAsOwner)
		.Process(this->Tint_Color)
		.Process(this->Tint_Intensity)
		.Process(this->Tint_VisibleToHouses)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
		.Process(this->ArmorMultiplier_AllowWarheads)
		.Process(this->ArmorMultiplier_DisallowWarheads)
		.Process(this->ArmorMultiplier_Chance)
	  	.Process(this-> ArmorMultiplier_HitAnim)
		.Process(this->SpeedMultiplier)
		.Process(this->ROFMultiplier)
		.Process(this->ROFMultiplier_ApplyOnCurrentTimer)
		.Process(this->Cloakable)
		.Process(this->ForceDecloak)
		.Process(this->WeaponRange_Multiplier)
		.Process(this->WeaponRange_ExtraRange)
		.Process(this->WeaponRange_AllowWeapons)
		.Process(this->WeaponRange_DisallowWeapons)
		.Process(this->Crit_Multiplier)
		.Process(this->Crit_ExtraChance)
		.Process(this->Crit_AllowWarheads)
		.Process(this->Crit_DisallowWarheads)
		.Process(this->RevengeWeapon)
		.Process(this->RevengeWeapon_AffectsHouses)
		.Process(this->RevengeWeapon_UseInvokerAsOwner)
		.Process(this->ReceiveRelativeDamageMult)
		.Process(this->DisableWeapons)
		.Process(this->Groups)
		.Process(this->DisableSelfHeal)
		.Process(this->Untrackable)
		.Process(this->AnimRandomPick)

		.Process(this->ReflectDamage)
		.Process(this->ReflectDamage_Warhead)
		.Process(this->ReflectDamage_Warhead_Detonate)
		.Process(this->ReflectDamage_Multiplier)
		.Process(this->ReflectDamage_AffectsHouses)

		.Process(this->ReflectDamage_Chance)
		.Process(this->ReflectDamage_Override)
		.Process(this->ReflectDamage_UseInvokerAsOwner)
		.Process(this->DiscardOn_AbovePercent)
		.Process(this->DiscardOn_BelowPercent)
		.Process(this->AffectAbovePercent)
		.Process(this->AffectBelowPercent)

		.Process(this->DisableRadar)
		.Process(this->DisableSpySat)
		.Process(this->Unkillable)

		.Process(this->ExtraWarheads)
		.Process(this->ExtraWarheads_DamageOverrides)
		.Process(this->ExtraWarheads_DetonationChances)
		.Process(this->ExtraWarheads_FullDetonation)

		.Process(this->FeedbackWeapon)
		.Process(this->LaserTrail_Type)
		.Process(this->Block_ChanceMultiplier)
		.Process(this->Block_ExtraChance)

		.Process(this->AffectTypes)
		.Process(this->IgnoreTypes)
		.Process(this->AffectTargets)

		.Process(this->Animation_DrawOffsets)

		.Process(this->PeriodicWeapon)
		.Process(this->PeriodicWeapon_AffectsHouse)
		.Process(this->PeriodicWeapon_UseInvokerAsOwner)
		.Process(this->PeriodicWeapon_Range)
		.Process(this->PeriodicWeapon_FiringDelay)
		.Process(this->PeriodicWeapon_FireAll)
		.Process(this->PeriodicWeapon_AffectTypes)
		.Process(this->PeriodicWeapon_IgnoreTypes)

		.Process(this->PrismRelay)
		.Process(this->PrismRelay_NetworkID)
		.Process(this->PrismRelay_Provider)
		.Process(this->PrismRelay_Receiver)
		.Process(this->PrismRelay_SupportWeapon)
		.Process(this->PrismRelay_MaxReceiveLinks)
		.Process(this->PrismRelay_MaxNodeLinks)
		.Process(this->PrismRelay_SupportFireDelay)
		.Process(this->PrismRelay_SupportMultiplier)
		.Process(this->PrismRelay_DamageAdd)
		.Process(this->PrismRelay_ToAllies)
		.Process(this->PrismRelay_AllowWeapons)
		.Process(this->PrismRelay_DisallowWeapons)
		.Process(this->PrismRelay_MasterWeaponIndex)
		.Process(this->PrismRelay_MasterWeaponUseMultiWeaponSelection)
		.Process(this->PrismRelay_SupportTimeout)

		;
}

void PhobosAttachEffectTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void PhobosAttachEffectTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

bool PhobosAttachEffectTypeClass::LoadGlobals(PhobosStreamReader& Stm)
{
	bool result = Enumerable<PhobosAttachEffectTypeClass>::LoadGlobals(Stm)
		&& Stm.Process(GroupsMap);

	return result;
}

bool PhobosAttachEffectTypeClass::SaveGlobals(PhobosStreamWriter& Stm)
{
	bool result = Enumerable<PhobosAttachEffectTypeClass>::SaveGlobals(Stm)
		&& Stm.Process(GroupsMap);

	return result;
}
