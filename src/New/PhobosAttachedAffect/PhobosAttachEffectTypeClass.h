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
	Valueable<bool> PenetratesIronCurtain;
	Nullable<AnimTypeClass*> Animation;
	NullableVector<AnimTypeClass*> CumulativeAnimations;
	Valueable<bool> Animation_ResetOnReapply;
	Valueable<AttachedAnimFlag> Animation_OfflineAction;
	Valueable<AttachedAnimFlag> Animation_TemporalAction;
	Valueable<bool> Animation_UseInvokerAsOwner;
	Nullable<WeaponTypeClass*> ExpireWeapon;
	Valueable<ExpireWeaponCondition> ExpireWeapon_TriggerOn;
	Valueable<bool> ExpireWeapon_CumulativeOnlyOnce;
	Nullable<ColorStruct> Tint_Color;
	Valueable<double> Tint_Intensity;
	Valueable<AffectedHouse> Tint_VisibleToHouses;
	Valueable<double> FirepowerMultiplier;
	Valueable<double> ArmorMultiplier;
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
	Nullable<WeaponTypeClass*> RevengeWeapon;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
	Valueable<bool> DisableWeapons;

	std::vector<std::string> Groups;

	Valueable<bool> DisableSelfHeal;
	Valueable<bool> Untrackable;
	Valueable<double> ReceiveRelativeDamageMult;

	PhobosAttachEffectTypeClass(const char* const pTitle) : Enumerable<PhobosAttachEffectTypeClass>(pTitle)
		, Duration { 0 }
		, Cumulative { false }
		, Cumulative_MaxCount { -1 }
		, Powered { false }
		, DiscardOn { DiscardCondition::None }
		, PenetratesIronCurtain { false }
		, Animation {}
		, CumulativeAnimations {}
		, Animation_ResetOnReapply { false }
		, Animation_OfflineAction { AttachedAnimFlag::Hides }
		, Animation_TemporalAction { AttachedAnimFlag::None }
		, Animation_UseInvokerAsOwner { false }
		, ExpireWeapon {}
		, ExpireWeapon_TriggerOn { ExpireWeaponCondition::Expire }
		, ExpireWeapon_CumulativeOnlyOnce { false }
		, Tint_Color {}
		, Tint_Intensity { 0.0 }
		, Tint_VisibleToHouses { AffectedHouse::All }
		, FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
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
		, RevengeWeapon {}
		, RevengeWeapon_AffectsHouses { AffectedHouse::All }
		, DisableWeapons { false }
		, Groups {}
		, DisableSelfHeal { false }
		, Untrackable { false }
		, ReceiveRelativeDamageMult { 1.0 }
	{};

	constexpr FORCEINLINE bool HasTint() {
		return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
	}

	bool HasGroup(const char* pGroupID) {
		for (const auto& group : this->Groups) {
			if (IS_SAME_STR_N(group.c_str(), pGroupID)) {
				return true;
			}
		}

		return false;
	}

	bool HasGroups(std::vector<std::string> const& groupIDs, bool requireAll) {
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

	constexpr FORCEINLINE AnimTypeClass* GetCumulativeAnimation(int cumulativeCount)
	{
		if (cumulativeCount < 0 || !this->CumulativeAnimations.HasValue())
			return nullptr;

		const int index = static_cast<size_t>(cumulativeCount) >= this->CumulativeAnimations.size() ? this->CumulativeAnimations.size() - 1 : cumulativeCount - 1;

		return this->CumulativeAnimations[index];
	}

	virtual ~PhobosAttachEffectTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static std::vector<PhobosAttachEffectTypeClass*> GetTypesFromGroups(std::vector<std::string>& groupIDs);
	static std::unordered_map<std::string, std::set<PhobosAttachEffectTypeClass*>> GroupsMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

