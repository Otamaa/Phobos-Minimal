#include "AEProperties.h"

#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

void AEProperties::Recalculate(TechnoClass* pTechno)
{
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);

	auto _AresAE = &pExt->AeData;
	auto _AEProp = &pExt->AE;

	// #region Multiplier accumulators

	double ROF_Mult = 1.0;
	double ReceiveRelativeDamageMult = 1.0;
	double FP_Mult = _AEProp->Crate_FirepowerMultiplier;
	double Armor_Mult = _AEProp->Crate_ArmorMultiplier;
	double Speed_Mult = _AEProp->Crate_SpeedMultiplier;

	// #endregion

	// #region Boolean flag accumulators

	bool Cloak = GET_TECHNOTYPE(pTechno)->Cloakable
		|| pTechno->HasAbility(AbilityType::Cloak)
		|| pExt->AE.flags.Cloakable;

	bool forceDecloak = false;
	bool disableWeapons = false;
	bool disableSelfHeal = false;
	bool untrackable = false;
	bool disableRadar = false;
	bool disableSpySat = false;
	bool unkillable = false;
	bool hasExtraWH = false;
	bool hasFeedbackWeapon = false;

	bool wasTint = _AEProp->flags.HasTint;
	bool hasTint = false;
	bool reflectsDamage = false;
	bool hasOnFireDiscardables = false;

	// #endregion

	// #region Transient data — clear and rebuild

	auto extraRangeData = &_AEProp->ExtraRange;
	auto extraCritData = &_AEProp->ExtraCrit;
	auto armormultData = &_AEProp->ArmorMultData;

	extraRangeData->Clear();
	extraCritData->Clear();
	armormultData->Clear();

	// #endregion

	std::optional<double> cur_timerAE {};

	// #region Loop 1: Ares AE system

	for (const auto& aeData : _AresAE->Data)
	{
		auto aeType = aeData.Type;

		if (aeType->ROFMultiplier_ApplyOnCurrentTimer)
		{
			if (!cur_timerAE.has_value())
				cur_timerAE = aeType->ROFMultiplier;
			else
				cur_timerAE.value() *= aeType->ROFMultiplier;
		}

		ROF_Mult *= aeType->ROFMultiplier;
		ReceiveRelativeDamageMult += aeType->ReceiveRelativeDamageMult;
		FP_Mult *= aeType->FirepowerMultiplier;
		Speed_Mult *= aeType->SpeedMultiplier;
		Armor_Mult *= aeType->ArmorMultiplier;

		Cloak |= aeType->Cloakable;
		forceDecloak |= aeType->ForceDecloak;
		disableWeapons |= aeType->DisableWeapons;
		disableSelfHeal |= aeType->DisableSelfHeal;
		untrackable |= aeType->Untrackable;
		disableRadar |= aeType->DisableRadar;
		disableSpySat |= aeType->DisableSpySat;
		unkillable |= aeType->Unkillable;
		hasExtraWH |= !aeType->ExtraWarheads.empty();

		if (aeType->WeaponRange_Multiplier != 1.0 || aeType->WeaponRange_ExtraRange != 0.0)
		{
			auto& entry = extraRangeData->ranges.emplace_back();
			entry.rangeMult = aeType->WeaponRange_Multiplier;
			entry.extraRange = aeType->WeaponRange_ExtraRange * Unsorted::LeptonsPerCell;
			entry.allow = &aeType->WeaponRange_AllowWeapons;
			entry.disallow = &aeType->WeaponRange_DisallowWeapons;
		}
	}

	// #endregion

	// #region Loop 2: Phobos AE system

	for (const auto& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect || !attachEffect->IsActive())
			continue;

		auto const type = attachEffect->GetType();

		FP_Mult *= type->FirepowerMultiplier;
		Speed_Mult *= type->SpeedMultiplier;
		ROF_Mult *= type->ROFMultiplier;
		Armor_Mult *= type->ArmorMultiplier;
		ReceiveRelativeDamageMult += type->ReceiveRelativeDamageMult;

		Cloak |= type->Cloakable;
		forceDecloak |= type->ForceDecloak;
		disableWeapons |= type->DisableWeapons;
		disableSelfHeal |= type->DisableSelfHeal;
		untrackable |= type->Untrackable;
		disableRadar |= type->DisableRadar;
		disableSpySat |= type->DisableSpySat;
		unkillable |= type->Unkillable;
		hasExtraWH |= !type->ExtraWarheads.empty();
		hasFeedbackWeapon |= type->FeedbackWeapon != nullptr;
		hasTint |= type->HasTint();
		reflectsDamage |= type->ReflectDamage;
		hasOnFireDiscardables |= (type->DiscardOn & DiscardCondition::Firing) != DiscardCondition::None;

		if (type->ROFMultiplier_ApplyOnCurrentTimer)
		{
			if (!cur_timerAE.has_value())
				cur_timerAE = type->ROFMultiplier;
			else
				cur_timerAE.value() *= type->ROFMultiplier;
		}

		if (type->WeaponRange_Multiplier != 1.0 || type->WeaponRange_ExtraRange != 0.0)
		{
			auto& entry = extraRangeData->ranges.emplace_back();
			entry.rangeMult = type->WeaponRange_Multiplier;
			entry.extraRange = type->WeaponRange_ExtraRange * Unsorted::LeptonsPerCell;
			entry.allow = &type->WeaponRange_AllowWeapons;
			entry.disallow = &type->WeaponRange_DisallowWeapons;
		}

		if (type->Crit_Multiplier != 1.0 || type->Crit_ExtraChance != 0.0)
		{
			auto& entry = extraCritData->ranges.emplace_back();
			entry.Mult = type->Crit_Multiplier;
			entry.extra = type->Crit_ExtraChance;
			entry.allow = &type->Crit_AllowWarheads;
			entry.disallow = &type->Crit_DisallowWarheads;
		}

		if (type->ArmorMultiplier != 1.0)
		{
			auto& entry = armormultData->mults.emplace_back();
			entry.Mult = type->ArmorMultiplier;
			entry.allow = &type->ArmorMultiplier_AllowWarheads;
			entry.disallow = &type->ArmorMultiplier_DisallowWarheads;
		}
	}

	// #endregion

	// #region Apply ROF timer adjustment

	if (cur_timerAE.has_value() && cur_timerAE > 0.0)
	{
		const int timeleft = pTechno->RearmTimer.GetTimeLeft();

		if (timeleft > 0)
			pTechno->RearmTimer.Start(int(timeleft * cur_timerAE.value()));
		else
			pTechno->RearmTimer.Stop();

		pTechno->ROF = static_cast<int>(pTechno->ROF * cur_timerAE.value());
	}

	// #endregion

	// #region Write back results

	pTechno->FirepowerMultiplier = FP_Mult;
	pTechno->ArmorMultiplier = Armor_Mult;
	_AEProp->ROFMultiplier = ROF_Mult;
	_AEProp->ReceiveRelativeDamageMult = ReceiveRelativeDamageMult;
	pTechno->Cloakable = Cloak;

	_AEProp->flags.ForceDecloak = forceDecloak;
	_AEProp->flags.DisableWeapons = disableWeapons;
	_AEProp->flags.DisableSelfHeal = disableSelfHeal;
	_AEProp->flags.Untrackable = untrackable;
	_AEProp->flags.HasTint = hasTint;
	_AEProp->flags.ReflectDamage = reflectsDamage;
	_AEProp->flags.HasOnFireDiscardables = hasOnFireDiscardables;
	_AEProp->flags.Unkillable = unkillable;
	_AEProp->flags.HasExtraWarheads = hasExtraWH;
	_AEProp->flags.HasFeedbackWeapon = hasFeedbackWeapon;

	if (((bool)_AEProp->flags.DisableRadar != disableRadar) || ((bool)_AEProp->flags.DisableSpySat != disableSpySat))
		pTechno->Owner->RecheckRadar = true;

	_AEProp->flags.DisableRadar = disableRadar;
	_AEProp->flags.DisableSpySat = disableSpySat;

	if (pTechno->AbstractFlags & AbstractFlags::Foot)
		((FootClass*)pTechno)->SpeedMultiplier = Speed_Mult;

	if (wasTint || hasTint)
		pExt->Tints.Update();

	// #endregion
}