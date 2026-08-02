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

	INI_EX exINI (pINI);

    for (size_t i = 0; i < CrateTypeClass::Count(); ++i) {
        auto& pCrate = CrateTypeClass::Array[i];
        char readBuffer[Phobos::readLength];

        if (!pINI->ReadString("Powerups", pCrate->Name.data(), "0,NONE", readBuffer))
            continue;

		const auto fields = PhobosCRT::SplitStringFixed<4u>(readBuffer, ",", true);

        if (fields.IsPresent(0)) pCrate->Weight = std::stoi(PhobosCRT::TrimToString(fields[0]).c_str());
        if (fields.IsPresent(1)) pCrate->Anim = AnimTypeClass::FindOrAllocate(PhobosCRT::TrimToString(fields[1]).c_str());
		if (fields.IsPresent(2)) Parser<bool>::TryParse(PhobosCRT::TrimToString(fields[2]).c_str(), pCrate->Naval.operator->());
		if (fields.IsPresent(3)) Parser<double>::TryParse(PhobosCRT::TrimToString(fields[3]).c_str(), pCrate->Argument.operator->());
    }
}

void NOINLINE SetDefaultProperties(CrateTypeClass* pAlloc, int i, CCINIClass* pINI)
{
	switch (Powerup(i))
	{
	case Powerup::Money:
		pAlloc->Sound = RulesClass::Instance->CrateMoneySound;
		break;
	case Powerup::HealBase:
		pAlloc->Sound = RulesClass::Instance->HealCrateSound;
		break;
	case Powerup::Armor:
		pAlloc->Sound = RulesClass::Instance->CrateArmourSound;
		pAlloc->Eva = VoxClass::FindIndexById(GameStrings::EVA_UnitArmorUpgraded());
		break;
	case Powerup::Speed:
		pAlloc->Sound = RulesClass::Instance->CrateSpeedSound;
		pAlloc->Eva = VoxClass::FindIndexById(GameStrings::EVA_UnitSpeedUpgraded());
		break;
	case Powerup::Firepower:
		pAlloc->Sound = RulesClass::Instance->CrateFireSound;
		pAlloc->Eva = VoxClass::FindIndexById(GameStrings::EVA_UnitFirePowerUpgraded());
		break;
	case Powerup::Reveal:
		pAlloc->Sound = RulesClass::Instance->CrateRevealSound;
		break;
	case Powerup::Unit:
		pAlloc->Sound = RulesClass::Instance->CrateUnitSound;
		break;
	case Powerup::Veteran:
		pAlloc->Sound = RulesClass::Instance->CratePromoteSound;
		break;
	default:
		break;
	}

	pAlloc->LoadFromINI(pINI);
}
void CrateTypeClass::ReadListFromINI(CCINIClass* pINI) {

	for (size_t i = 0; i < CrateTypeClass::Count(); ++i) {
		SetDefaultProperties(CrateTypeClass::Array[i].get(), i, pINI);
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