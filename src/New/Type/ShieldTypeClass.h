#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/CursorTypeClass.h>

class ShieldTypeClass final : public Enumerable<ShieldTypeClass>
{
public:
	Valueable<int> Strength;
	Nullable<int> InitialStrength;
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
	Nullable<AnimTypeClass*> BreakAnim;
	Nullable<AnimTypeClass*> HitAnim;
	Nullable<WeaponTypeClass*> BreakWeapon;
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
	Nullable<SpotlightFlags> HitBright;
public:

	ShieldTypeClass::ShieldTypeClass(const char* const pTitle);
	virtual ~ShieldTypeClass() override  = default;
	//{
	//	if(Pips_Background_SHP.Get(nullptr)) {
	//		GameDelete(Pips_Background_SHP.Get());
	//		Pips_Background_SHP.Reset();
	//	}
	//};

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static void AddDefaults() {
		FindOrAllocate(DEFAULT_STR2);
	}

	AnimTypeClass* GetIdleAnimType(bool isDamaged, double healthRatio);

private:
	template <typename T>
	void Serialize(T& Stm);
};
