#include "CrateTypeClass.h"

#include <AnimClass.h>

Enumerable<CrateTypeClass>::container_t Enumerable<CrateTypeClass>::Array;

void CrateTypeClass::PlayAllAffects(CoordStruct loc, CoordStruct locsound, bool isPlayerControlled) const
{
	if (const auto pAnimType = this->Anim) {
		GameCreate<AnimClass>(pAnimType, loc, 0, 1, 0x600, 0, 0);
	}

	if (!isPlayerControlled)
		return;

	VocClass::SafeImmedietelyPlayAt(this->Sound, &locsound, nullptr);
	VoxClass::PlayIndex(this->Eva);
}

void CrateTypeClass::AddDefaults()
{
	if (Empty()) {
		for (auto crate : Powerups::Effects) {
			Debug::LogInfo("Creating default Crate of [{}]", crate);
			Allocate(crate);
		}
	}
}

void CrateTypeClass::ReadFromPowerups(CCINIClass* pINI)
{
	if (!pINI->GetSection("Powerups"))
		return;

	for (size_t i = 0; i < CrateTypeClass::Count(); ++i) {
		auto& pCrate = CrateTypeClass::Array[i];
		char readBuffer[Phobos::readLength];

		if (!pINI->ReadString(
			"Powerups",
			pCrate->Name.data(),
			"0,NONE",
			readBuffer))
		{
			continue;
		}

		// Share count
		if (char* token = strtok(readBuffer, ",")) {
			pCrate->Weight = atoi(PhobosCRT::trim(token).c_str());
		}

		// Animation
		if (char* token = strtok(nullptr, ",")) {
			pCrate->Anim = AnimTypeClass::Find(PhobosCRT::trim(token).c_str());
		}

		// Placeable on water
		if (char* token = strtok(nullptr, ",")) {
			auto _Naval = PhobosCRT::trim(token);
			if (IS_SAME_STR_(_Naval.c_str(), "yes")) {
				pCrate->Naval = true;
			} else if (IS_SAME_STR_(_Naval.c_str(), "no")) {
				pCrate->Naval = false;
			}
		}

		// Crate value/chance
		if (char* token = strtok(nullptr, ",")) {
			auto arg = PhobosCRT::trim(token);

			if (strchr(arg.c_str(), '%')) {
				pCrate->Argument = (atof(arg.c_str()) * 0.01);
			} else {
				pCrate->Argument = atof(arg.c_str());
			}
		}
	}
}

void CrateTypeClass::ReadListFromINI(CCINIClass* pINI) {

	for (size_t i = 0; i < CrateTypeClass::Count(); ++i) {

		if(i < Powerups::Effects.size()) {
			//the array is same position and size dont need to do shit
			if(auto pAlloc = CrateTypeClass::Array[i].get()){

				switch (Powerup(i))
				{
				case Powerup::Money:
					pAlloc->Sound = RulesClass::Instance->CrateMoneySound; break;
				case Powerup::HealBase:
					pAlloc->Sound = RulesClass::Instance->HealCrateSound; break;
				case Powerup::Armor:
					pAlloc->Sound = RulesClass::Instance->CrateArmourSound; break;
					pAlloc->Eva = VoxClass::FindIndexById(GameStrings::EVA_UnitArmorUpgraded());
				case Powerup::Speed:
					pAlloc->Sound = RulesClass::Instance->CrateSpeedSound; break;
					pAlloc->Eva = VoxClass::FindIndexById(GameStrings::EVA_UnitSpeedUpgraded());
				case Powerup::Firepower:
					pAlloc->Sound = RulesClass::Instance->CrateFireSound; break;
					pAlloc->Eva = VoxClass::FindIndexById(GameStrings::EVA_UnitFirePowerUpgraded());
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

		CrateTypeClass::GetArray()[i]->LoadFromINI(pINI);
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
	this->Eva.Read(exINI, section, "Crate.EVA");
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
		.Process(this->Eva)
		.Process(this->Speed)
		;
}

void CrateTypeClass::LoadFromStream(PhobosStreamReader &Stm) { this->Serialize(Stm); }
void CrateTypeClass::SaveToStream(PhobosStreamWriter &Stm) { this->Serialize(Stm); }