#include "ShieldTypeClass.h"

Enumerable<ShieldTypeClass>::container_t Enumerable<ShieldTypeClass>::Array;

ShieldTypeClass::ShieldTypeClass(const char* const pTitle) : Enumerable<ShieldTypeClass> { pTitle }
, Strength { 0 }
, InitialStrength {}
, Armor { Armor::None }
, Powered { false }
, Respawn { 0.0 }
, Respawn_Rate { 0 }
, SelfHealing { 0.0 }
, SelfHealing_Rate { 0 }
, SelfHealing_RestartInCombat { true }
, SelfHealing_RestartInCombatDelay { 0 }
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
, Pips { { -1,-1,-1 } }
, Pips_Background_SHP {}
, Pips_Building { { -1,-1,-1 } }
, Pips_Building_Empty {}
, ImmuneToPsychedelic { false }
, ThreadPosed { }
, ImmuneToCrit { false }
, BreakWeapon_TargetSelf { true }
, PassthruNegativeDamage { false }
, CanBeHealed { false }
, HealCursorType { }
, HitBright { }
{};

const char* Enumerable<ShieldTypeClass>::GetMainSection()
{
	return "ShieldTypes";
}

AnimTypeClass* ShieldTypeClass::GetIdleAnimType(bool isDamaged, double healthRatio)
{
	auto damagedAnim = this->IdleAnimDamaged.Get(healthRatio);

	if (isDamaged && damagedAnim)
		return damagedAnim;
	else
		return this->IdleAnim.Get(healthRatio);
}

void ShieldTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (IS_SAME_STR_(pSection, DEFAULT_STR2))
		return;

	INI_EX exINI(pINI);

	this->Strength.Read(exINI, pSection, "Strength");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	this->Armor.Read(exINI, pSection, "Armor");
	this->Powered.Read(exINI, pSection, "Powered");

	this->Respawn.Read(exINI, pSection, "Respawn");

	Nullable<double> Respawn_Rate__InMinutes {};
	Respawn_Rate__InMinutes.Read(exINI, pSection, "Respawn.Rate");

	if (Respawn_Rate__InMinutes.isset())
		this->Respawn_Rate = (int)(Respawn_Rate__InMinutes.Get() * 900);

	this->SelfHealing.Read(exINI, pSection, "SelfHealing");

	Nullable<double> SelfHealing_Rate__InMinutes {};
	SelfHealing_Rate__InMinutes.Read(exINI, pSection, "SelfHealing.Rate");

	if (SelfHealing_Rate__InMinutes.isset())
		this->SelfHealing_Rate = (int)(SelfHealing_Rate__InMinutes.Get() * 900);

	this->SelfHealing_RestartInCombat.Read(exINI, pSection, "SelfHealing.RestartInCombat");
	this->SelfHealing_RestartInCombatDelay.Read(exINI, pSection, "SelfHealing.RestartInCombatDelay");

	this->AbsorbOverDamage.Read(exINI, pSection, "AbsorbOverDamage");
	this->BracketDelta.Read(exINI, pSection, "BracketDelta");
	this->ReceivedDamage_Minimum.Read(exINI, pSection, "ReceivedDamage.Minimum");
	this->ReceivedDamage_Maximum.Read(exINI, pSection, "ReceivedDamage.Maximum");

	this->IdleAnim_OfflineAction.Read(exINI, pSection, "IdleAnim.OfflineAction");
	this->IdleAnim_TemporalAction.Read(exINI, pSection, "IdleAnim.TemporalAction");

	this->IdleAnim.Read(exINI, pSection, "IdleAnim.%s" , nullptr, true);
	this->IdleAnimDamaged.Read(exINI, pSection, "IdleAnimDamaged.%s", nullptr, true);

	this->BreakAnim.Read(exINI, pSection, "BreakAnim", true);
	this->HitAnim.Read(exINI, pSection, "HitAnim", true);
	this->BreakWeapon.Read(exINI, pSection, "BreakWeapon", true);

	this->AbsorbPercent.Read(exINI, pSection, "AbsorbPercent");
	this->PassPercent.Read(exINI, pSection, "PassPercent");

	this->AllowTransfer.Read(exINI, pSection, "AllowTransfer");

	this->Pips.Read(exINI, pSection, "Pips");
	this->Pips_Background_SHP.Read(exINI, pSection, "Pips.Background");
	this->Pips_Building.Read(exINI, pSection, "Pips.Building");
	this->Pips_Building_Empty.Read(exINI, pSection, "Pips.Building.Empty");

	this->ImmuneToPsychedelic.Read(exINI, pSection, "ImmuneToPsychedelic");
	this->ThreadPosed.Read(exINI, pSection, "ThreadPosed");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");

	this->BreakWeapon_TargetSelf.Read(exINI , pSection , "BreakWeapon.TargetSelf");

	this->PassthruNegativeDamage.Read(exINI, pSection, "PassthruNegativeDamage");
	this->CanBeHealed.Read(exINI, pSection, "Repairable");
	this->HealCursorType.Read(exINI, pSection, "RepairCursor");
	this->HitBright.Read(exINI, pSection, "HitBright");
}

template <typename T>
void ShieldTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Strength)
		.Process(this->InitialStrength)
		.Process(this->Armor)
		.Process(this->Powered)
		.Process(this->Respawn)
		.Process(this->Respawn_Rate)
		.Process(this->SelfHealing)
		.Process(this->SelfHealing_Rate)
		.Process(this->SelfHealing_RestartInCombat)
		.Process(this->SelfHealing_RestartInCombatDelay)
		.Process(this->AbsorbOverDamage)
		.Process(this->BracketDelta)
		.Process(this->ReceivedDamage_Minimum)
		.Process(this->ReceivedDamage_Maximum)
		.Process(this->IdleAnim_OfflineAction)
		.Process(this->IdleAnim_TemporalAction)
		.Process(this->IdleAnim)
		.Process(this->BreakAnim)
		.Process(this->HitAnim)
		.Process(this->BreakWeapon)
		.Process(this->AbsorbPercent)
		.Process(this->PassPercent)
		.Process(this->AllowTransfer)
		.Process(this->Pips)
		.Process(this->Pips_Background_SHP)
		.Process(this->Pips_Building)
		.Process(this->Pips_Building_Empty)
		.Process(this->ImmuneToPsychedelic)
		.Process(this->ThreadPosed)
		.Process(this->ImmuneToCrit)
		.Process(this->BreakWeapon_TargetSelf)
		.Process(this->PassthruNegativeDamage)
		.Process(this->CanBeHealed)
		.Process(this->HealCursorType)
		.Process(this->HitBright)
		;
}

void ShieldTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ShieldTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
