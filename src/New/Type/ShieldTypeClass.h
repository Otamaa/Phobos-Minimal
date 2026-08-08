#pragma once

#include <Ext/Rules/Body.h>

#include <Utilities/Enumerable.h>

#include <New/Type/CursorTypeClass.h>

#include <Point3D.h>

#include <ColorStruct.h>

class ShieldTypeClass final : public Enumerable<ShieldTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "ShieldTypes";
	static COMPILETIMEEVAL const char* ClassName = "ShieldTypeClass";

public:
	Valueable<int> Strength { 0 };
	Nullable<int> InitialStrength {};

	Nullable<double> ConditionYellow {};
	Nullable<double> ConditionRed {};

	Valueable<Armor> Armor { Armor::None };
	Valueable<bool> Powered { false };
	Valueable<double> Respawn { 0.0 };
	Valueable<int> Respawn_Rate { 0 };
	Valueable<bool> Respawn_RestartInCombat { true };
	Valueable<int> Respawn_RestartInCombatDelay { 0 };
	ValueableVector<AnimTypeClass*> Respawn_Anim {};
	Valueable<WeaponTypeClass*> Respawn_Weapon {};
	Valueable<double> SelfHealing { 0.0 };
	Valueable<int> SelfHealing_Rate { 0 };
	Valueable<bool> SelfHealing_RestartInCombat { true };
	Valueable<int> SelfHealing_RestartInCombatDelay { 0 };
	ValueableVector<BuildingTypeClass*> SelfHealing_EnabledBy {};

	Valueable<bool> AbsorbOverDamage { false };
	Valueable<int> BracketDelta { 0 };
	Valueable<AttachedAnimFlag> IdleAnim_OfflineAction { AttachedAnimFlag::Hides };
	Valueable<AttachedAnimFlag> IdleAnim_TemporalAction { AttachedAnimFlag::Hides };
	Damageable<AnimTypeClass*> IdleAnim {};
	Damageable<AnimTypeClass*> IdleAnimDamaged {};
	Valueable<AnimTypeClass*> BreakAnim {};
	Valueable<AnimTypeClass*> HitAnim {};
	Valueable<WeaponTypeClass*> BreakWeapon {};
	Valueable<double> AbsorbPercent { 1.0 };
	Valueable<double> PassPercent { 0.0 };
	Valueable<int> ReceivedDamage_Minimum { INT32_MIN };
	Valueable<int> ReceivedDamage_Maximum { INT32_MAX };

	Nullable<bool> AllowTransfer {};

	Valueable<Point3D> Pips { {-1, -1, -1} };
	Nullable<SHPCaches*> Pips_Background_SHP {};
	Valueable<Point3D> Pips_Building { {-1, -1, -1} };
	Nullable<int> Pips_Building_Empty {};
	Valueable<bool> Pips_HideIfNoStrength { false };

	Valueable<bool> ImmuneToPsychedelic { false };
	Nullable<int> ThreadPosed {};
	Valueable<bool> ImmuneToCrit { false };

	Valueable<bool> BreakWeapon_TargetSelf { true };

	Valueable<bool> PassthruNegativeDamage { false };
	Valueable<bool> CanBeHealed { false };
	NullableIdx<CursorTypeClass> HealCursorType {};

	Valueable<bool> HitFlash { false };
	Nullable<int> HitFlash_FixedSize {};
	Valueable<bool> HitFlash_Red { true };
	Valueable<bool> HitFlash_Green { true };
	Valueable<bool> HitFlash_Blue { true };
	Valueable<bool> HitFlash_Black { false };

	Valueable<ColorStruct> Tint_Color {};
	Valueable<double> Tint_Intensity { 0.0 };
	Valueable<AffectedHouse> Tint_VisibleToHouses { AffectedHouse::All };

	ValueableVector<TechnoTypeClass*> InheritArmor_Allowed {};
	ValueableVector<TechnoTypeClass*> InheritArmor_Disallowed {};
	Valueable<bool> InheritArmorFromTechno {};
	Valueable<bool> CanBlock { false };

	Nullable<bool> UseArmorplier {};

public:

	ShieldTypeClass(const char* const pTitle) : Enumerable<ShieldTypeClass>(pTitle) {}
	virtual ~ShieldTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void COMPILETIMEEVAL OPTIONALINLINE AddDefaults() {
		FindOrAllocate(DEFAULT_STR2);
	}

	AnimTypeClass* GetIdleAnimType(bool isDamaged, double healthRatio);

	COMPILETIMEEVAL OPTIONALINLINE double GetConditionYellow() {
		return this->ConditionYellow.Get(FakeRulesClass::Instance()->Shield_ConditionYellow);
	}

	COMPILETIMEEVAL OPTIONALINLINE double GetConditionRed() {
		return this->ConditionRed.Get(FakeRulesClass::Instance()->Shield_ConditionRed);
	}

	COMPILETIMEEVAL OPTIONALINLINE bool HasTint() const {
		return this->Tint_Color.Get() != ColorStruct::Empty || this->Tint_Intensity != 0.0;
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};
