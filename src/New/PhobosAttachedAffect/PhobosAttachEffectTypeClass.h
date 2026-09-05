#pragma once

#include <set>
#include <unordered_map>

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/VectorSet.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <New/Entity/AnimationDrawOffsetClass.h>

#include <ColorStruct.h>

class PhobosAttachEffectTypeClass;
struct GroupData
{
    VectorSet<PhobosAttachEffectTypeClass*> types;

	bool load(PhobosStreamReader& Stm, bool RegisterForChange) {
		return
				Stm
					.Process(types ,RegisterForChange)
					.Success();
	}

	bool save(PhobosStreamWriter& Stm) const {
		return
				Stm
					.Process(types)
					.Success();
	}

    size_t size() const { return types.size(); }
    auto begin() const { return types.begin(); }
    auto end() const { return types.end(); }
    void insert(PhobosAttachEffectTypeClass* item) { types.insert(item); }
};

class PhobosAttachEffectTypeClass final : public Enumerable<PhobosAttachEffectTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "AttachEffectTypes";
	static COMPILETIMEEVAL const char* ClassName = "PhobosAttachEffectTypeClass";

public:
	Valueable<int> Duration { 0 };
	Valueable<bool> Duration_ApplyFirepowerMult { false };
	Valueable<bool> Duration_ApplyArmorMultOnTarget { false };
	Valueable<WarheadTypeClass*> Duration_ApplyVersus_Warhead {};

	Valueable<bool> Cumulative { false };
	Valueable<int> Cumulative_MaxCount { -1 };
	Valueable<bool> Powered { false };

	Valueable<DiscardCondition> DiscardOn { DiscardCondition::None };
	Nullable<Leptons> DiscardOn_RangeOverride {};
	Nullable<bool> DiscardOn_MoveBasedOnDestination {};
	Nullable<bool> DiscardOn_ConsiderHarvestingAsStationary {};
	Valueable<int> DiscardOn_Ammo_Min { -1 };
	Valueable<int> DiscardOn_Ammo_Max { -1 };
	Nullable<double> DiscardOn_Health_Min {};
	Nullable<double> DiscardOn_Health_Max {};
	Valueable<int> DiscardOn_Firing_Count { 1 };
	Valueable<int> DiscardOn_ReceivedDamage_Count { 1 };
	Valueable<AffectedHouse> DiscardOn_ReceivedDamage_AffectsHouse { AffectedHouse::All };
	ValueableVector<Mission> DiscardOn_Missions {};
	NullableVector<Mission> DiscardOn_AIMissions {};
	ValueableVector<DoType> DiscardOn_Sequences {};
	Nullable<bool>DiscardOn_Sequences_Immediate {};
	Valueable<LandTypeFlags> DiscardOn_LandTypes { LandTypeFlags::None };

	Valueable<bool> PenetratesIronCurtain { false };
	Nullable<bool> PenetratesForceShield {};
	Valueable<AnimTypeClass*> Animation {};
	ValueableVector<AnimTypeClass*> CumulativeAnimations {};
	Valueable<bool> CumulativeAnimations_RestartOnChange { true };
	Valueable<bool> Animation_ResetOnReapply { false };
	Valueable<AttachedAnimFlag> Animation_OfflineAction { AttachedAnimFlag::Hides };
	Valueable<AttachedAnimFlag> Animation_TemporalAction { AttachedAnimFlag::None };
	Valueable<bool> Animation_UseInvokerAsOwner { false };
	ValueableVector<PhobosAttachEffectTypeClass*> Animation_HideIfAttachedWith {};
	Valueable<WeaponTypeClass*> ExpireWeapon { nullptr };
	Valueable<ExpireWeaponCondition> ExpireWeapon_TriggerOn { ExpireWeaponCondition::Expire };
	Valueable<bool> ExpireWeapon_CumulativeOnlyOnce { false };
	Valueable<bool> ExpireWeapon_UseInvokerAsOwner { false };
	Valueable<ColorStruct> Tint_Color {};
	Valueable<double> Tint_Intensity { 0.0 };
	Valueable<AffectedHouse> Tint_VisibleToHouses { AffectedHouse::All };
	Valueable<double> FirepowerMultiplier { 1.0 };

	Valueable<double> ArmorMultiplier { 1.0 };
	ValueableVector<WarheadTypeClass*> ArmorMultiplier_AllowWarheads {};
	ValueableVector<WarheadTypeClass*> ArmorMultiplier_DisallowWarheads {};
	Valueable<double> ArmorMultiplier_Chance { 1.0 };
	ValueableVector<AnimTypeClass*> ArmorMultiplier_HitAnim {};

	Valueable<double> SpeedMultiplier { 1.0 };
	Valueable<double> ROFMultiplier { 1.0 };
	Valueable<bool> ROFMultiplier_ApplyOnCurrentTimer { true };
	Valueable<bool> Cloakable { false };
	Valueable<bool> ForceDecloak { false };
	Valueable<double> WeaponRange_Multiplier { 1.0 };
	Valueable<double> WeaponRange_ExtraRange { 0.0 };
	ValueableVector<WeaponTypeClass*> WeaponRange_AllowWeapons {};
	ValueableVector<WeaponTypeClass*> WeaponRange_DisallowWeapons {};
	Valueable<double> Crit_Multiplier { 1.0 };
	Valueable<double> Crit_ExtraChance { 0.0 };
	ValueableVector<WarheadTypeClass*> Crit_AllowWarheads {};
	ValueableVector<WarheadTypeClass*> Crit_DisallowWarheads {};
	Valueable<WeaponTypeClass*> RevengeWeapon { nullptr };
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses { AffectedHouse::All };
	Valueable<bool> RevengeWeapon_UseInvokerAsOwner { false };
	Valueable<bool> DisableWeapons { false };

	ValueableVector<std::string> Groups {};

	Valueable<bool> DisableSelfHeal { false };
	Valueable<bool> Untrackable { false };
	Valueable<double> ReceiveRelativeDamageMult { 1.0 };
	Valueable<bool> AnimRandomPick { false };

	Valueable<bool> ReflectDamage { false };
	Nullable<WarheadTypeClass*> ReflectDamage_Warhead {};
	Valueable<bool> ReflectDamage_Warhead_Detonate { false };
	Valueable<double> ReflectDamage_Multiplier { 1.0 };
	Valueable<AffectedHouse> ReflectDamage_AffectsHouses { AffectedHouse::All };

	Nullable<double> ReflectDamage_Chance {};
	Nullable<int> ReflectDamage_Override {};
	Valueable<bool> ReflectDamage_UseInvokerAsOwner { false };

	Nullable<double> DiscardOn_AbovePercent {};
	Nullable<double> DiscardOn_BelowPercent {};
	Nullable<double> AffectAbovePercent {};
	Nullable<double> AffectBelowPercent {};

	Valueable<bool> DisableRadar {};
	Valueable<bool> DisableSpySat {};

	Valueable<bool> Unkillable {};

	ValueableVector<WarheadTypeClass*> ExtraWarheads {};
	ValueableVector<int> ExtraWarheads_DamageOverrides {};
	ValueableVector<double> ExtraWarheads_DetonationChances {};
	ValueableVector<bool> ExtraWarheads_FullDetonation {};

	Valueable<WeaponTypeClass*> FeedbackWeapon {};

	ValueableIdx<LaserTrailTypeClass*> LaserTrail_Type { -1 };

	Valueable<double> Block_ChanceMultiplier { 1.0 };
	Valueable<double> Block_ExtraChance { 0.0 };

	ValueableVector<TechnoTypeClass*> AffectTypes {};
	ValueableVector<TechnoTypeClass*> IgnoreTypes {};
	Valueable<AffectedTarget> AffectTargets { AffectedTarget::All };

	std::vector<AnimationDrawOffsetClass> Animation_DrawOffsets {};

	Valueable<WeaponTypeClass*> PeriodicWeapon {};
	Valueable<AffectedHouse> PeriodicWeapon_AffectsHouse { AffectedHouse::All };
	Valueable<bool> PeriodicWeapon_UseInvokerAsOwner { false };
	Valueable<Leptons> PeriodicWeapon_Range { Leptons(0) };
	Valueable<int> PeriodicWeapon_FiringDelay { 0 };
	Valueable<bool> PeriodicWeapon_FireAll { false };
	ValueableVector<TechnoTypeClass*> PeriodicWeapon_AffectTypes {};
	ValueableVector<TechnoTypeClass*> PeriodicWeapon_IgnoreTypes {};

	Valueable<bool> PrismRelay { false };
	Valueable<int> PrismRelay_NetworkID { 0 };
	Valueable<bool> PrismRelay_Provider { true };
	Valueable<bool> PrismRelay_Receiver { true };
	Valueable<WeaponTypeClass*> PrismRelay_SupportWeapon {};
	Valueable<int> PrismRelay_MaxReceiveLinks { -1 };
	Valueable<int> PrismRelay_MaxNodeLinks { -1 };
	Valueable<int> PrismRelay_SupportFireDelay { 0 };
	Valueable<double> PrismRelay_SupportMultiplier { 1.0 };
	Valueable<int> PrismRelay_DamageAdd { 0 };
	Valueable<bool> PrismRelay_ToAllies { false };
	ValueableVector<WeaponTypeClass*> PrismRelay_AllowWeapons {};
	ValueableVector<WeaponTypeClass*> PrismRelay_DisallowWeapons {};
	Valueable<int> PrismRelay_MasterWeaponIndex { -1 };
	Valueable<bool> PrismRelay_MasterWeaponUseMultiWeaponSelection { false };
	Valueable<int> PrismRelay_SupportTimeout { 45 };

	PhobosAttachEffectTypeClass(const char* pTitle) : Enumerable<PhobosAttachEffectTypeClass>(pTitle)	{};
	virtual ~PhobosAttachEffectTypeClass() = default;

	COMPILETIMEEVAL FORCEDINLINE bool HasAnim() {
		if (this->Cumulative)
			return this->CumulativeAnimations.size() > 0 || this->Animation != nullptr;
		else
			return this->Animation != nullptr;
	}

	COMPILETIMEEVAL FORCEDINLINE bool HasTint() {
		return this->Tint_Color.Get() != ColorStruct::Empty  || this->Tint_Intensity != 0.0;
	}

	COMPILETIMEEVAL bool HasGroup(const char* pGroupID) {
		for (const auto& group : this->Groups) {
			if (group == pGroupID) {
				return true;
			}
		}

		return false;
	}

	COMPILETIMEEVAL bool HasGroups(std::vector<std::string> const& groupIDs, bool requireAll) {
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

	COMPILETIMEEVAL FORCEDINLINE AnimTypeClass* GetCumulativeAnimation(int cumulativeCount)
	{
		if (cumulativeCount < 0)
			return nullptr;

		const int index = static_cast<size_t>(cumulativeCount) >= this->CumulativeAnimations.size() ? this->CumulativeAnimations.size() - 1 : cumulativeCount - 1;

		return this->CumulativeAnimations[index];
	}

	COMPILETIMEEVAL FORCEDINLINE  bool HasAnim() const
	{
		if (this->Cumulative)
			return this->CumulativeAnimations.size() > 0 || this->Animation != nullptr;
		else
			return this->Animation != nullptr;
	}

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);
	void AddToGroupsMap();

	static std::vector<PhobosAttachEffectTypeClass*> GetTypesFromGroups(std::vector<std::string>& groupIDs);
	static PhobosMap<std::string, GroupData> GroupsMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static OPTIONALINLINE COMPILETIMEEVAL void Clear() {
		Enumerable<PhobosAttachEffectTypeClass>::Clear();
		GroupsMap.clear();
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

