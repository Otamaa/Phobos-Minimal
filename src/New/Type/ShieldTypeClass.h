#pragma once

#include <Ext/Rules/Body.h>

#include <Utilities/Enumerable.h>

#include <New/Type/CursorTypeClass.h>

#include <Point3D.h>

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

	Nullable<HealthBarTypeClass*> ShieldBar;
	Nullable<Point3D> Pips;

	Nullable<SHPStruct*> Pips_Background_SHP;

	Nullable<Point3D> Pips_Building;

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

	ValueableVector<TechnoTypeClass*> InheritArmor_Allowed;
	ValueableVector<TechnoTypeClass*> InheritArmor_Disallowed;
	Valueable<bool> InheritArmorFromTechno;
public:

	ShieldTypeClass(const char* const pTitle) : Enumerable<ShieldTypeClass> { pTitle }
		, Strength { 0 }
		, InitialStrength {}
		, ConditionYellow {}
		, ConditionRed {}
		, Armor { Armor::None }
		, Powered { false }
		, Respawn { 0.0 }
		, Respawn_Rate { 0 }
		, SelfHealing { 0.0 }
		, SelfHealing_Rate { 0 }
		, SelfHealing_RestartInCombat { true }
		, SelfHealing_RestartInCombatDelay { 0 }
		, SelfHealing_EnabledBy {}
		, AbsorbOverDamage { false }
		, BracketDelta { 0 }
		, IdleAnim_OfflineAction { AttachedAnimFlag::Hides }
		, IdleAnim_TemporalAction { AttachedAnimFlag::Hides }
		, IdleAnim {}
		, IdleAnimDamaged {}
		, BreakAnim {}
		, HitAnim {}
		, BreakWeapon {}
		, AbsorbPercent { 1.0 }
		, PassPercent { 0.0 }
		, ReceivedDamage_Minimum { INT32_MIN }
		, ReceivedDamage_Maximum { INT32_MAX }
		, AllowTransfer {}
		, ShieldBar {}
		, Pips {}
		, Pips_Background_SHP {}
		, Pips_Building {}
		, Pips_Building_Empty {}
		, Pips_HideIfNoStrength { false }
		, ImmuneToPsychedelic { false }
		, ThreadPosed { }
		, ImmuneToCrit { false }
		, BreakWeapon_TargetSelf { true }
		, PassthruNegativeDamage { false }
		, CanBeHealed { false }
		, HealCursorType { }
		, HitFlash { false }
		, HitFlash_FixedSize {}
		, HitFlash_Red { true }
		, HitFlash_Green { true }
		, HitFlash_Blue { true }
		, HitFlash_Black { false }
		, Tint_Color {}
		, Tint_Intensity { 0.0 }
		, Tint_VisibleToHouses { AffectedHouse::All }
		, InheritArmor_Allowed {}
		, InheritArmor_Disallowed {}
		, InheritArmorFromTechno {}
	{};

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void COMPILETIMEEVAL OPTIONALINLINE AddDefaults() {
		FindOrAllocate(DEFAULT_STR2);
	}

	AnimTypeClass* GetIdleAnimType(bool isDamaged, double healthRatio);

	COMPILETIMEEVAL OPTIONALINLINE double GetConditionYellow() {
		return this->ConditionYellow.Get(RulesExtData::Instance()->Shield_ConditionYellow);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetConditionRed() {
		return this->ConditionRed.Get(RulesExtData::Instance()->Shield_ConditionRed);
	}

	COMPILETIMEEVAL OPTIONALINLINE bool HasTint() const {
		return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};
