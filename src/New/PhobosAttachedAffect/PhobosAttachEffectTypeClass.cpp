#include "PhobosAttachEffectTypeClass.h"

std::unordered_map<std::string, std::set<PhobosAttachEffectTypeClass*>> PhobosAttachEffectTypeClass::GroupsMap;
Enumerable<PhobosAttachEffectTypeClass>::container_t Enumerable<PhobosAttachEffectTypeClass>::Array;

bool PhobosAttachEffectTypeClass::HasTint()
{
	return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
}

bool PhobosAttachEffectTypeClass::HasGroup(const char* pGroupID)
{
	for (const auto& group : this->Groups) {
		if (IS_SAME_STR_N(group.c_str(), pGroupID)) {
			return true;
		}
	}

	return false;
}

bool PhobosAttachEffectTypeClass::HasGroups(std::vector<std::string> const& groupIDs, bool requireAll)
{
	size_t foundCount = 0;

	for (const auto& group : this->Groups) {
		for (const auto& requiredGroup : groupIDs) {
			if (IS_SAME_STR_N(group.c_str(), requiredGroup.c_str())) {

				if (!requireAll)
					return true;

				foundCount++;
			}
		}
	}

	return !requireAll ? false : foundCount >= groupIDs.size();
}

std::vector<PhobosAttachEffectTypeClass*> PhobosAttachEffectTypeClass::GetTypesFromGroups(std::vector<std::string>& groupIDs)
{
	std::set<PhobosAttachEffectTypeClass*> types;
	auto map = &PhobosAttachEffectTypeClass::GroupsMap;

	for (const auto& group : groupIDs) {
		if (map->contains(group)) {
			auto const values = &map->at(group);
			types.insert(values->begin(), values->end());
		}
	}

	return std::vector<PhobosAttachEffectTypeClass*>(types.begin(), types.end());
}

AnimTypeClass* PhobosAttachEffectTypeClass::GetCumulativeAnimation(int cumulativeCount)
{
	if (cumulativeCount < 0 || !this->CumulativeAnimations.HasValue())
		return nullptr;

	int index = static_cast<size_t>(cumulativeCount) >= this->CumulativeAnimations.size() ? this->CumulativeAnimations.size() - 1 : cumulativeCount - 1;

	return this->CumulativeAnimations.at(index);
}

template<>
const char* Enumerable<PhobosAttachEffectTypeClass>::GetMainSection()
{
	return "AttachEffectTypes";
}

void PhobosAttachEffectTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;

	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->Duration.Read(exINI, pSection, "Duration");
	this->Cumulative.Read(exINI, pSection, "Cumulative");
	this->Cumulative_MaxCount.Read(exINI, pSection, "Cumulative.MaxCount");
	this->Powered.Read(exINI, pSection, "Powered");
	this->DiscardOn.Read(exINI, pSection, "DiscardOn");
	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");

	this->Animation.Read(exINI, pSection, "Animation");
	this->CumulativeAnimations.Read(exINI, pSection, "CumulativeAnimations");
	this->Animation_ResetOnReapply.Read(exINI, pSection, "Animation.ResetOnReapply");
	this->Animation_OfflineAction.Read(exINI, pSection, "Animation.OfflineAction");
	this->Animation_TemporalAction.Read(exINI, pSection, "Animation.TemporalAction");
	this->Animation_UseInvokerAsOwner.Read(exINI, pSection, "Animation.UseInvokerAsOwner");

	this->ExpireWeapon.Read(exINI, pSection, "ExpireWeapon" , true);
	this->ExpireWeapon_TriggerOn.Read(exINI, pSection, "ExpireWeapon.TriggerOn");
	this->ExpireWeapon_CumulativeOnlyOnce.Read(exINI, pSection, "ExpireWeapon.CumulativeOnlyOnce");

	this->Tint_Color.Read(exINI, pSection, "Tint.Color");
	this->Tint_Intensity.Read(exINI, pSection, "Tint.Intensity");
	this->Tint_VisibleToHouses.Read(exINI, pSection, "Tint.VisibleToHouses");

	this->FirepowerMultiplier.Read(exINI, pSection, "FirepowerMultiplier");
	this->ArmorMultiplier.Read(exINI, pSection, "ArmorMultiplier");
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
	exINI.ParseStringList(this->Groups, pSection, "Groups");
	auto const map = &PhobosAttachEffectTypeClass::GroupsMap;

	for (const auto&  group : this->Groups) {
		if (!map->contains(group)) {
			map->insert(std::make_pair(group, std::set<PhobosAttachEffectTypeClass*>{this}));
		} else {
			map->at(group).insert(this);
		}
	}

	this->DisableSelfHeal.Read(exINI, pSection, "DisableSelfHeal");
	this->Untrackable.Read(exINI, pSection, "Untrackable");
}

template <typename T>
void PhobosAttachEffectTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Duration)
		.Process(this->Cumulative)
		.Process(this->Cumulative_MaxCount)
		.Process(this->Powered)
		.Process(this->DiscardOn)
		.Process(this->PenetratesIronCurtain)
		.Process(this->Animation)
		.Process(this->CumulativeAnimations)
		.Process(this->Animation_ResetOnReapply)
		.Process(this->Animation_OfflineAction)
		.Process(this->Animation_TemporalAction)
		.Process(this->Animation_UseInvokerAsOwner)
		.Process(this->ExpireWeapon)
		.Process(this->ExpireWeapon_TriggerOn)
		.Process(this->ExpireWeapon_CumulativeOnlyOnce)
		.Process(this->Tint_Color)
		.Process(this->Tint_Intensity)
		.Process(this->Tint_VisibleToHouses)
		.Process(this->FirepowerMultiplier)
		.Process(this->ArmorMultiplier)
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
		.Process(this->ReceiveRelativeDamageMult)
		.Process(this->DisableWeapons)
		.Process(this->Groups)
		.Process(this->DisableSelfHeal)
		.Process(this->Untrackable)
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
