#include "CrateTypeClass.h"

const char * Enumerable<CrateTypeClass>::GetMainSection() { return "CrateTypes"; }

void CrateTypeClass::ReadListFromINI(CCINIClass* pINI) {

	for (size_t i = 0; i < Array.size(); ++i) {

		if(i < Powerups::Effects.size()) {
			if(auto pAlloc = CrateTypeClass::Find(Powerups::Effects[i])){
				pAlloc->Weight = Powerups::Weights[i];
				pAlloc->Argument = Powerups::Arguments[i];
				pAlloc->Naval = Powerups::Naval[i];
				pAlloc->Anim = AnimTypeClass::Array->GetItemOrDefault(Powerups::Anims[i]);

				switch (Powerup(i))
				{
				case Powerup::Money:
					pAlloc->Sound = RulesClass::Instance->CrateMoneySound; break;
				case Powerup::HealBase:
					pAlloc->Sound = RulesClass::Instance->HealCrateSound; break;
				case Powerup::Armor:
					pAlloc->Sound = RulesClass::Instance->CrateArmourSound; break;
				case Powerup::Speed:
					pAlloc->Sound = RulesClass::Instance->CrateSpeedSound; break;
				case Powerup::Firepower:
					pAlloc->Sound = RulesClass::Instance->CrateFireSound; break;
				case Powerup::Reveal:
					pAlloc->Sound = RulesClass::Instance->CrateRevealSound; break;
				case Powerup::Unit:
					pAlloc->Sound = RulesClass::Instance->CrateUnitSound; break;
				case Powerup::Veteran:
					pAlloc->Sound = RulesClass::Instance->CratePromoteSound; break;
				default:
					break;
				}
			}
		}

		Array[i]->LoadFromINI(pINI);
	}
}

void CrateTypeClass::ReadFromINIList(CCINIClass* pINI)
{
	CrateTypeClass::AddDefaults();
	CrateTypeClass::LoadFromINIOnlyTheList(pINI);
}

void CrateTypeClass::LoadFromINI(CCINIClass *pINI)
{
	const char *section = this->Name.c_str();

	INI_EX exINI(pINI);

	this->Weight.Read(exINI, section, "Crate.Weight");
	this->Anim.Read(exINI, section, "Crate.Anim");
	this->Argument.Read(exINI, section, "Crate.Argument");
	this->Naval.Read(exINI, section, "Crate.Naval");
	this->Sound.Read(exINI, section, "Crate.Sound");
	this->Speed = pINI->ReadSpeedType(section, "Crate.SpeedType", this->Speed);
	//this->Super.Read(exINI, section, "Crate.SuperWeapon",true);
	//this->SuperGrant.Read(exINI, section, "Crate.SuperWeaponGrant");

	//this->WeaponType.Read(exINI, section, "Crate.Weapon", true);

	//this->Chance.Read(exINI, section, "Crate.Chance");
	//this->Anim.Read(exINI, section, "Crate.Anim");
	//this->Type.Read(exINI, section, "Crate.Type");
	//this->AllowWater.Read(exINI, section, "Crate.AllowWater");
	//this->Sound.Read(exINI, section, "Crate.Sound");
	//this->Eva.Read(exINI, section, "Crate.EVA");

	//this->Unit.Read(exINI, section, "Crate.Units");

	//this->MoneyMin.Read(exINI, section, "Crate.MoneyMin");
	//this->MoneyMax.Read(exINI, section, "Crate.MoneyMax");

}

template <typename T>
void CrateTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Weight)
		.Process(this->Anim)
		.Process(this->Argument)
		.Process(this->Naval)
		.Process(this->Sound)
		.Process(this->Speed)
		;
}

void CrateTypeClass::LoadFromStream(PhobosStreamReader &Stm) { this->Serialize(Stm); }
void CrateTypeClass::SaveToStream(PhobosStreamWriter &Stm) { this->Serialize(Stm); }