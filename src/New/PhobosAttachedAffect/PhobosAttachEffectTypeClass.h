#pragma once

#include <set>
#include <unordered_map>

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>

class PhobosAttachEffectTypeClass final : public Enumerable<PhobosAttachEffectTypeClass>
{
public:
	Valueable<int> Duration;
	Valueable<bool> Cumulative;
	Valueable<int> Cumulative_MaxCount;
	Valueable<bool> Powered;
	Valueable<DiscardCondition> DiscardOn;
	Nullable<Leptons> DiscardOn_RangeOverride;
	Valueable<bool> PenetratesIronCurtain;
	Nullable<bool> PenetratesForceShield;
	Valueable<AnimTypeClass*> Animation;
	NullableVector<AnimTypeClass*> CumulativeAnimations;
	Valueable<bool> CumulativeAnimations_RestartOnChange;
	Valueable<bool> Animation_ResetOnReapply;
	Valueable<AttachedAnimFlag> Animation_OfflineAction;
	Valueable<AttachedAnimFlag> Animation_TemporalAction;
	Valueable<bool> Animation_UseInvokerAsOwner;
	ValueableVector<PhobosAttachEffectTypeClass*> Animation_HideIfAttachedWith;
	Valueable<WeaponTypeClass*> ExpireWeapon;
	Valueable<ExpireWeaponCondition> ExpireWeapon_TriggerOn;
	Valueable<bool> ExpireWeapon_CumulativeOnlyOnce;
	Nullable<ColorStruct> Tint_Color;
	Valueable<double> Tint_Intensity;
	Valueable<AffectedHouse> Tint_VisibleToHouses;
	Valueable<double> FirepowerMultiplier;

	Valueable<double> ArmorMultiplier;
	ValueableVector<WarheadTypeClass*> ArmorMultiplier_AllowWarheads;
	ValueableVector<WarheadTypeClass*> ArmorMultiplier_DisallowWarheads;

	Valueable<double> SpeedMultiplier;
	Valueable<double> ROFMultiplier;
	Valueable<bool> ROFMultiplier_ApplyOnCurrentTimer;
	Valueable<bool> Cloakable;
	Valueable<bool> ForceDecloak;
	Valueable<double> WeaponRange_Multiplier;
	Valueable<double> WeaponRange_ExtraRange;
	ValueableVector<WeaponTypeClass*> WeaponRange_AllowWeapons;
	ValueableVector<WeaponTypeClass*> WeaponRange_DisallowWeapons;
	Valueable<double> Crit_Multiplier;
	Valueable<double> Crit_ExtraChance;
	ValueableVector<WarheadTypeClass*> Crit_AllowWarheads;
	ValueableVector<WarheadTypeClass*> Crit_DisallowWarheads;
	Valueable<WeaponTypeClass*> RevengeWeapon;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
	Valueable<bool> DisableWeapons;

	ValueableVector<std::string> Groups;

	Valueable<bool> DisableSelfHeal;
	Valueable<bool> Untrackable;
	Valueable<double> ReceiveRelativeDamageMult;
	Valueable<bool> AnimRandomPick;

	Valueable<bool> ReflectDamage;
	Nullable<WarheadTypeClass*> ReflectDamage_Warhead;
	Valueable<bool> ReflectDamage_Warhead_Detonate;
	Valueable<double> ReflectDamage_Multiplier;
	Valueable<AffectedHouse> ReflectDamage_AffectsHouses;

	Nullable<double> ReflectDamage_Chance;
	Nullable<int> ReflectDamage_Override;

	Nullable<double> DiscardOn_AbovePercent;
	Nullable<double> DiscardOn_BelowPercent;
	Nullable<double> AffectAbovePercent;
	Nullable<double> AffectBelowPercent;

	PhobosAttachEffectTypeClass(const char* pTitle) : Enumerable<PhobosAttachEffectTypeClass>(pTitle)
		, Duration { 0 }
		, Cumulative { false }
		, Cumulative_MaxCount { -1 }
		, Powered { false }
		, DiscardOn { DiscardCondition::None }
		, DiscardOn_RangeOverride {}
		, PenetratesIronCurtain { false }
		, PenetratesForceShield {}
		, Animation {}
		, CumulativeAnimations {}
		, CumulativeAnimations_RestartOnChange { true }
		, Animation_ResetOnReapply { false }
		, Animation_OfflineAction { AttachedAnimFlag::Hides }
		, Animation_TemporalAction { AttachedAnimFlag::None }
		, Animation_UseInvokerAsOwner { false }
		, Animation_HideIfAttachedWith {}
		, ExpireWeapon { nullptr }
		, ExpireWeapon_TriggerOn { ExpireWeaponCondition::Expire }
		, ExpireWeapon_CumulativeOnlyOnce { false }
		, Tint_Color {}
		, Tint_Intensity { 0.0 }
		, Tint_VisibleToHouses { AffectedHouse::All }
		, FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, ArmorMultiplier_AllowWarheads {}
		, ArmorMultiplier_DisallowWarheads {}
		, SpeedMultiplier { 1.0 }
		, ROFMultiplier { 1.0 }
		, ROFMultiplier_ApplyOnCurrentTimer { true }
		, Cloakable { false }
		, ForceDecloak { false }
		, WeaponRange_Multiplier { 1.0 }
		, WeaponRange_ExtraRange { 0.0 }
		, WeaponRange_AllowWeapons {}
		, WeaponRange_DisallowWeapons {}
		, Crit_Multiplier { 1.0 }
		, Crit_ExtraChance { 0.0 }
		, Crit_AllowWarheads {}
		, Crit_DisallowWarheads {}
		, RevengeWeapon { nullptr }
		, RevengeWeapon_AffectsHouses { AffectedHouse::All }
		, DisableWeapons { false }
		, Groups {}
		, DisableSelfHeal { false }
		, Untrackable { false }
		, ReceiveRelativeDamageMult { 1.0 }
		, AnimRandomPick { false }

		, ReflectDamage { false }
		, ReflectDamage_Warhead {}
		, ReflectDamage_Warhead_Detonate { false }
		, ReflectDamage_Multiplier { 1.0 }
		, ReflectDamage_AffectsHouses { AffectedHouse::All }

		, ReflectDamage_Chance {}
		, ReflectDamage_Override {}

		, DiscardOn_AbovePercent {}
		, DiscardOn_BelowPercent {}
		, AffectAbovePercent {}
		, AffectBelowPercent {}
	{};

	constexpr FORCEINLINE bool HasTint() {
		return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
	}

	constexpr bool HasGroup(const char* pGroupID) {
		for (const auto& group : this->Groups) {
			if (group == pGroupID) {
				return true;
			}
		}

		return false;
	}

	constexpr bool HasGroups(std::vector<std::string> const& groupIDs, bool requireAll) {
		size_t foundCount = 0;

		for (const auto& group : this->Groups) {
			for (const auto& requiredGroup : groupIDs) {
				if (group == requiredGroup) {

					if (!requireAll)
						return true;

					foundCount++;
				}
			}
		}

		return !requireAll ? false : foundCount >= groupIDs.size();
	}

	constexpr FORCEINLINE AnimTypeClass* GetCumulativeAnimation(int cumulativeCount)
	{
		if (cumulativeCount < 0 || !this->CumulativeAnimations.HasValue() || this->CumulativeAnimations.empty())
			return nullptr;

		const int index = static_cast<size_t>(cumulativeCount) >= this->CumulativeAnimations.size() ? this->CumulativeAnimations.size() - 1 : cumulativeCount - 1;

		return this->CumulativeAnimations[index];
	}

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static std::vector<PhobosAttachEffectTypeClass*> GetTypesFromGroups(std::vector<std::string>& groupIDs);
	static PhobosMap<std::string, std::set<PhobosAttachEffectTypeClass*>> GroupsMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

