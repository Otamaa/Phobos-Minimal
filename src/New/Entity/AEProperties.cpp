#include "AEProperties.h"

#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

#include <ExtraHeaders/StackVector.h>

struct AEAccumulator
{
	// Multipliers (use double for precision)
	double ROF_Mult = 1.0;
	double ReceiveRelativeDamageMult = 1.0;
	double FP_Mult = 1.0;
	double Armor_Mult = 1.0;
	double Speed_Mult = 1.0;

	// Boolean flags
	bool Cloak = false;
	bool forceDecloak = false;
	bool disableWeapons = false;
	bool disableSelfHeal = false;
	bool untrackable = false;
	bool disableRadar = false;
	bool disableSpySat = false;
	bool unkillable = false;
	bool hasExtraWH = false;
	bool hasFeedbackWeapon = false;
	bool hasTint = false;
	bool reflectsDamage = false;
	bool hasOnFireDiscardables = false;

	// Use small vectors (stack-allocated for small sizes)
	StackVector<AEPropertiesExtraRange::RangeData, 4> extraRanges;
	StackVector<AEPropertiesExtraCrit::CritData, 4> extraCrits;
	StackVector<AEPropertiesArmorMult::MultData, 4> armorMults;

	// Optional timer for ROF
	std::optional<double> cur_timerAE;

	// Helper to accumulate weapon range data
	void AccumulateWeaponRange(
		double rangeMult,
		double extraRange,
		const std::vector<WeaponTypeClass*>& allowWeapons,
		const std::vector<WeaponTypeClass*>& disallowWeapons);

	// Helper to accumulate crit data
	void AccumulateCrit(
		double critMult,
		double critExtra,
		const std::vector<WarheadTypeClass*>& allowWarheads,
		const std::vector<WarheadTypeClass*>& disallowWarheads);

	// Helper to accumulate armor mult data
	void AccumulateArmorMult(
		double armorMult,
		const std::vector<WarheadTypeClass*>& allowWarheads,
		const std::vector<WarheadTypeClass*>& disallowWarheads);
};

// Helper function to update component (create/update/remove)
template<typename TComp, typename TData>
void UpdateAEComponent(TechnoExtData* ext, TComp*& cache, TData&& data);

// Specialized version for ArmorMult (uses 'mults' not 'ranges')
template<>
void UpdateAEComponent<AEPropertiesArmorMult, StackVector<AEPropertiesArmorMult::MultData, 4>>(
	TechnoExtData* ext,
	AEPropertiesArmorMult*& cache,
	StackVector<AEPropertiesArmorMult::MultData, 4>&& data);

// Main recalculation function declaration
namespace AEPropertiesHelper
{
	void Recalculate(TechnoClass* pTechno);
}

void AEAccumulator::AccumulateWeaponRange(
	double rangeMult,
	double extraRange,
	const std::vector<WeaponTypeClass*>& allowWeapons,
	const std::vector<WeaponTypeClass*>& disallowWeapons)
{
	auto& range = extraRanges.container().emplace_back();
	range.rangeMult = rangeMult;
	range.extraRange = extraRange * Unsorted::LeptonsPerCell;

	// Copy allow/disallow lists
	for (auto weapon : allowWeapons)
	{
		range.allow.insert(weapon);
	}
	for (auto weapon : disallowWeapons)
	{
		range.disallow.insert(weapon);
	}
}

// Accumulator helper: Add crit data
void AEAccumulator::AccumulateCrit(
	double critMult,
	double critExtra,
	const std::vector<WarheadTypeClass*>& allowWarheads,
	const std::vector<WarheadTypeClass*>& disallowWarheads)
{
	auto& crit = extraCrits.container().emplace_back();
	crit.Mult = critMult;
	crit.extra = critExtra;

	// Copy allow/disallow lists
	for (auto warhead : allowWarheads)
	{
		crit.allow.insert(warhead);
	}
	for (auto warhead : disallowWarheads)
	{
		crit.disallow.insert(warhead);
	}
}

// Accumulator helper: Add armor mult data
void AEAccumulator::AccumulateArmorMult(
	double armorMult,
	const std::vector<WarheadTypeClass*>& allowWarheads,
	const std::vector<WarheadTypeClass*>& disallowWarheads)
{
	auto& mult = armorMults.container().emplace_back();
	mult.Mult = armorMult;

	// Copy allow/disallow lists
	for (auto warhead : allowWarheads)
	{
		mult.allow.insert(warhead);
	}
	for (auto warhead : disallowWarheads) {
		mult.disallow.insert(warhead);
	}
}

// Helper: Update component (create/update/remove)
template<typename TComp, typename TData>
void UpdateAEComponent(TechnoExtData* ext, TData&& data)
{
	if (!data.container().empty())
	{
		// Data exists - ensure component exists
		auto pComp = Phobos::gEntt->try_get<TComp>(ext->MyEntity);

		if (!pComp) {
			pComp = &Phobos::gEntt->emplace<TComp>(ext->MyEntity);
		}

		// Move data into component (no copy!)
		pComp->ranges.clear();
		pComp->ranges.reserve(data.container().size());
		for (auto& item : data.container()) {
			pComp->ranges.push_back(std::move(item));
		}
	}
	else {
		Phobos::gEntt->remove<TComp>(ext->MyEntity);
	}
}

// Specialization for ArmorMult (uses 'mults' not 'ranges')
template<>
void UpdateAEComponent<AEPropertiesArmorMult, StackVector<AEPropertiesArmorMult::MultData, 4>>(
	TechnoExtData* ext,
	StackVector<AEPropertiesArmorMult::MultData, 4>&& data)
{
	if (!data.container().empty()) {
		auto pComp = Phobos::gEntt->try_get<AEPropertiesArmorMult>(ext->MyEntity);

		if (!pComp) {
			pComp = &Phobos::gEntt->emplace<AEPropertiesArmorMult>(ext->MyEntity);
		}

		pComp->mults.clear();
		pComp->mults.reserve(data.container().size());
		for (auto& item : data.container())
		{
			pComp->mults.push_back(std::move(item));
		}
	} else {
		Phobos::gEntt->remove<AEPropertiesArmorMult>(ext->MyEntity);
	}
}

void AEProperties::Recalculate(TechnoClass* pTechno)
{
	auto pExt = TechnoExtContainer::Instance.Find(pTechno);
	auto _AresAE = pExt->Get_AresAEData();
	auto _AEProp = pExt->Get_AEProperties();

	// Stack-allocated accumulator (no heap allocation for small sizes!)
	AEAccumulator acc;

	// Initialize from current state (convert to double for calculation)
	acc.FP_Mult = static_cast<double>(_AEProp->FirepowerMultiplier);
	acc.Armor_Mult = static_cast<double>(_AEProp->ArmorMultiplier);
	acc.Speed_Mult = static_cast<double>(_AEProp->SpeedMultiplier);
	acc.Cloak = GET_TECHNOTYPE(pTechno)->Cloakable || pTechno->HasAbility(AbilityType::Cloak) || pExt->AE.flags.Cloakable;

	if(_AresAE) {
		// === Accumulate from Ares AE ===
		for (const auto& aeData : _AresAE->Data)
		{
			auto type = aeData.Type;

			// Accumulate multipliers
			acc.ROF_Mult *= type->ROFMultiplier;
			acc.ReceiveRelativeDamageMult += type->ReceiveRelativeDamageMult;
			acc.FP_Mult *= type->FirepowerMultiplier;
			acc.Speed_Mult *= type->SpeedMultiplier;
			acc.Armor_Mult *= type->ArmorMultiplier;

			// Accumulate flags
			acc.Cloak |= type->Cloakable;
			acc.forceDecloak |= type->ForceDecloak;
			acc.disableWeapons |= type->DisableWeapons;
			acc.disableSelfHeal |= type->DisableSelfHeal;
			acc.untrackable |= type->Untrackable;
			acc.disableRadar |= type->DisableRadar;
			acc.disableSpySat |= type->DisableSpySat;
			acc.unkillable |= type->Unkillable;
			acc.hasExtraWH |= type->ExtraWarheads.size() > 0;

			// Handle ROF timer
			if (type->ROFMultiplier_ApplyOnCurrentTimer)
			{
				if (!acc.cur_timerAE.has_value())
					acc.cur_timerAE = type->ROFMultiplier;
				else
					acc.cur_timerAE.value() *= type->ROFMultiplier;
			}

			// Handle weapon range
			if (!(type->WeaponRange_Multiplier == 1.0 && type->WeaponRange_ExtraRange == 0.0))
			{
				acc.AccumulateWeaponRange(
					type->WeaponRange_Multiplier,
					type->WeaponRange_ExtraRange,
					type->WeaponRange_AllowWeapons,
					type->WeaponRange_DisallowWeapons
				);
			}
		}
	}

	// === Accumulate from Phobos AE ===
	for (const auto& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect || !attachEffect->IsActive())
			continue;

		auto type = attachEffect->GetType();

		// Accumulate multipliers
		acc.FP_Mult *= type->FirepowerMultiplier;
		acc.Speed_Mult *= type->SpeedMultiplier;
		acc.ROF_Mult *= type->ROFMultiplier;
		acc.ReceiveRelativeDamageMult += type->ReceiveRelativeDamageMult;

		// Accumulate flags
		acc.Cloak |= type->Cloakable;
		acc.forceDecloak |= type->ForceDecloak;
		acc.disableWeapons |= type->DisableWeapons;
		acc.disableSelfHeal |= type->DisableSelfHeal;
		acc.untrackable |= type->Untrackable;
		acc.unkillable |= type->Unkillable;
		acc.disableRadar |= type->DisableRadar;
		acc.disableSpySat |= type->DisableSpySat;
		acc.hasExtraWH |= type->ExtraWarheads.size() > 0;
		acc.hasFeedbackWeapon |= type->FeedbackWeapon != nullptr;
		acc.hasTint |= type->HasTint();
		acc.reflectsDamage |= type->ReflectDamage;
		acc.hasOnFireDiscardables |= (type->DiscardOn & DiscardCondition::Firing) != DiscardCondition::None;

		// Handle ROF timer
		if (type->ROFMultiplier_ApplyOnCurrentTimer)
		{
			if (!acc.cur_timerAE.has_value())
				acc.cur_timerAE = type->ROFMultiplier;
			else
				acc.cur_timerAE.value() *= type->ROFMultiplier;
		}

		// Handle weapon range
		if (!(type->WeaponRange_Multiplier == 1.0 && type->WeaponRange_ExtraRange == 0.0))
		{
			acc.AccumulateWeaponRange(
				type->WeaponRange_Multiplier,
				type->WeaponRange_ExtraRange,
				type->WeaponRange_AllowWeapons,
				type->WeaponRange_DisallowWeapons
			);
		}

		// Handle crit
		if (!(type->Crit_Multiplier == 1.0 && type->Crit_ExtraChance == 0.0))
		{
			acc.AccumulateCrit(
				type->Crit_Multiplier,
				type->Crit_ExtraChance,
				type->Crit_AllowWarheads,
				type->Crit_DisallowWarheads
			);
		}

		// Handle armor multiplier
		if (type->ArmorMultiplier != 1.0)
		{
			acc.AccumulateArmorMult(
				type->ArmorMultiplier,
				type->ArmorMultiplier_AllowWarheads,
				type->ArmorMultiplier_DisallowWarheads
			);
		}
	}

	// === Apply ROF timer adjustment ===
	if (acc.cur_timerAE.has_value() && acc.cur_timerAE > 0.0)
	{
		const int timeleft = pTechno->RearmTimer.GetTimeLeft();

		if (timeleft > 0)
		{
			pTechno->RearmTimer.Start(static_cast<int>(timeleft * acc.cur_timerAE.value()));
		}
		else
		{
			pTechno->RearmTimer.Stop();
		}

		pTechno->ROF = static_cast<int>(pTechno->ROF * acc.cur_timerAE.value());
	}

	// === Apply accumulated multipliers to game objects ===
	pTechno->FirepowerMultiplier = acc.FP_Mult;
	pTechno->ArmorMultiplier = acc.Armor_Mult;
	pTechno->Cloakable = acc.Cloak;

	if (pTechno->AbstractFlags & AbstractFlags::Foot)
	{
		static_cast<FootClass*>(pTechno)->SpeedMultiplier = acc.Speed_Mult;
	}

	// === Store accumulated values in AE core (keep as double) ===
	_AEProp->ROFMultiplier = acc.ROF_Mult;
	_AEProp->ReceiveRelativeDamageMult = acc.ReceiveRelativeDamageMult;

	// Update flags in bitfield
	_AEProp->AllFlags = 0;
	if (acc.forceDecloak) _AEProp->ForceDecloak = true;
	if (acc.disableWeapons) _AEProp->DisableWeapons = true;
	if (acc.disableSelfHeal) _AEProp->DisableSelfHeal = true;
	if (acc.untrackable) _AEProp->Untrackable = true;
	if (acc.hasTint) _AEProp->HasTint = true;
	if (acc.reflectsDamage) _AEProp->ReflectDamage = true;
	if (acc.hasOnFireDiscardables) _AEProp->HasOnFireDiscardables = true;
	if (acc.unkillable) _AEProp->Unkillable = true;
	if (acc.hasExtraWH) _AEProp->HasExtraWarheads = true;
	if (acc.hasFeedbackWeapon) _AEProp->HasFeedbackWeapon = true;

	// === Handle radar changes ===
	bool oldDisableRadar = _AEProp->DisableRadar;
	bool oldDisableSpySat = _AEProp->DisableSpySat;

	_AEProp->DisableRadar = acc.disableRadar;
	_AEProp->DisableSpySat = acc.disableSpySat;

	if ((oldDisableRadar != acc.disableRadar) || (oldDisableSpySat != acc.disableSpySat))
	{
		pTechno->Owner->RecheckRadar = true;
	}

	// === Update optional components (create/update/remove as needed) ===
	UpdateAEComponent<AEPropertiesExtraRange, StackVector<AEPropertiesExtraRange::RangeData, 4>>(pExt, std::move(acc.extraRanges));
	UpdateAEComponent<AEPropertiesExtraCrit, StackVector<AEPropertiesExtraCrit::CritData, 4>>(pExt, std::move(acc.extraCrits));
	UpdateAEComponent<AEPropertiesArmorMult, StackVector<AEPropertiesArmorMult::MultData, 4>>(pExt, std::move(acc.armorMults));

	// Update range modifier flag
	_AEProp->HasRangeModifier = (pExt->Get_AEPropertiesExtraRange() != nullptr);

	// === Update tint if needed ===
	bool wasTint = _AEProp->HasTint;
	if ((wasTint || acc.hasTint))
	{
		pExt->Tints.Update();
	}
}

bool AEPropertiesExtraRange::RangeData::Eligible(WeaponTypeClass* who)
{
	bool allowed = false;

	if (allow.begin() != allow.end())
	{
		for (auto iter_allow = allow.begin(); iter_allow != allow.end(); ++iter_allow)
		{
			if (*iter_allow == who)
			{
				allowed = true;
				break;
			}
		}
	}
	else
	{
		allowed = true;
	}

	if (allowed && disallow.begin() != disallow.end())
	{
		for (auto iter_disallow = disallow.begin(); iter_disallow != disallow.end(); ++iter_disallow)
		{
			if (*iter_disallow == who)
			{
				allowed = false;
				break;
			}
		}
	}

	return allowed;
}

bool AEPropertiesExtraCrit::CritData::Eligible(WarheadTypeClass * who)
{

	bool allowed = false;

	if (allow.begin() != allow.end())
	{
		for (auto iter_allow = allow.begin(); iter_allow != allow.end(); ++iter_allow)
		{
			if (*iter_allow == who)
			{
				allowed = true;
				break;
			}
		}
	}
	else
	{
		allowed = true;
	}

	if (allowed && disallow.begin() != disallow.end())
	{
		for (auto iter_disallow = disallow.begin(); iter_disallow != disallow.end(); ++iter_disallow)
		{
			if (*iter_disallow == who)
			{
				allowed = false;
				break;
			}
		}
	}

	return allowed;
}

bool AEPropertiesArmorMult::MultData::Eligible(WarheadTypeClass* who)
{
	bool allowed = false;

	if (allow.begin() != allow.end())
	{
		for (auto iter_allow = allow.begin(); iter_allow != allow.end(); ++iter_allow)
		{
			if (*iter_allow == who)
			{
				allowed = true;
				break;
			}
		}
	}
	else
	{
		allowed = true;
	}

	if (allowed && disallow.begin() != disallow.end())
	{
		for (auto iter_disallow = disallow.begin(); iter_disallow != disallow.end(); ++iter_disallow)
		{
			if (*iter_disallow == who)
			{
				allowed = false;
				break;
			}
		}
	}

	return allowed;
}