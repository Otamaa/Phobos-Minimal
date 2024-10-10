#pragma once

#include <Ext/Rules/Body.h>

#include <Utilities/Enumerable.h>

#include <New/Type/CursorTypeClass.h>


class ShieldTypeClass final : public Enumerable<ShieldTypeClass>
{
public:
	Valueable<int> Strength;
	Nullable<int> InitialStrength;

	Nullable<double> ConditionYellow;
	Nullable<double> ConditionRed;

	Valueable<Armor> Armor;
	Valueable<bool> Powered;
	Valueable<double> Respawn;
	Valueable<int> Respawn_Rate;
	Valueable<double> SelfHealing;
	Valueable<int> SelfHealing_Rate;
	Valueable<bool> SelfHealing_RestartInCombat;
	Valueable<int> SelfHealing_RestartInCombatDelay;
	ValueableVector<BuildingTypeClass*> SelfHealing_EnabledBy;

	Valueable<bool> AbsorbOverDamage;
	Valueable<int> BracketDelta;
	Valueable<AttachedAnimFlag> IdleAnim_OfflineAction;
	Valueable<AttachedAnimFlag> IdleAnim_TemporalAction;
	Damageable<AnimTypeClass*> IdleAnim;
	Damageable<AnimTypeClass*> IdleAnimDamaged;
	Valueable<AnimTypeClass*> BreakAnim;
	Valueable<AnimTypeClass*> HitAnim;
	Valueable<WeaponTypeClass*> BreakWeapon;
	Valueable<double> AbsorbPercent;
	Valueable<double> PassPercent;
	Valueable<int> ReceivedDamage_Minimum;
	Valueable<int> ReceivedDamage_Maximum;

	Nullable<bool> AllowTransfer;

	Valueable<Point3D> Pips;
	Nullable<SHPStruct*> Pips_Background_SHP;
	Valueable<Point3D> Pips_Building;
	Nullable<int> Pips_Building_Empty;
	Valueable<bool> Pips_HideIfNoStrength;

	Valueable<bool> ImmuneToPsychedelic;
	Nullable<int> ThreadPosed;
	Valueable<bool> ImmuneToCrit;

	Valueable<bool> BreakWeapon_TargetSelf;

	Valueable<bool> PassthruNegativeDamage;
	Valueable<bool> CanBeHealed;
	NullableIdx<CursorTypeClass> HealCursorType;

	Valueable<bool> HitFlash;
	Nullable<int> HitFlash_FixedSize;
	Valueable<bool> HitFlash_Red;
	Valueable<bool> HitFlash_Green;
	Valueable<bool> HitFlash_Blue;
	Valueable<bool> HitFlash_Black;

	Nullable<ColorStruct> Tint_Color;
	Valueable<double> Tint_Intensity;
	Valueable<AffectedHouse> Tint_VisibleToHouses;

	ValueableVector<TechnoTypeClass*> InheritArmor_Allowed { };
	ValueableVector<TechnoTypeClass*> InheritArmor_Disallowed { };
	Valueable<bool> InheritArmorFromTechno { };
public:

	ShieldTypeClass::ShieldTypeClass(const char* const pTitle);

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void constexpr inline AddDefaults() {
		FindOrAllocate(DEFAULT_STR2);
	}

	AnimTypeClass* GetIdleAnimType(bool isDamaged, double healthRatio);

	constexpr inline double GetConditionYellow() {
		return this->ConditionYellow.Get(RulesExtData::Instance()->Shield_ConditionYellow);
	}

	constexpr inline double GetConditionRed() {
		return this->ConditionRed.Get(RulesExtData::Instance()->Shield_ConditionRed);
	}

	constexpr inline bool HasTint() const {
		return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};
