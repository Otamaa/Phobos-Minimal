#include "RocketTypeClass.h"

Enumerable<RocketTypeClass>::container_t Enumerable<RocketTypeClass>::Array;

// pretty nice, eh
const char* Enumerable<RocketTypeClass>::GetMainSection()
{
	return "RocketTypes";
}

void RocketTypeClass::ReadListFromINI(CCINIClass* pINI, bool bDebug) {
	size_t i = 0;
	for (auto& pItem : Array) {
		pItem->LoadFromINI_B(pINI, i++);
	}
}

void RocketTypeClass::LoadFromINI_B(CCINIClass* pINI, size_t idx){

	if (idx == 0 || idx == 1 || idx == 2) {

		std::string pSection = this->Name.c_str();

		INI_EX exINI(pINI);

		detail::read(this->RocketData.PauseFrames, exINI, GameStrings::General, (pSection + "PauseFrames").c_str());
		detail::read(this->RocketData.TiltFrames, exINI, GameStrings::General, (pSection + "TiltFrames").c_str());
		detail::read(this->RocketData.PitchInitial, exINI, GameStrings::General, (pSection + "PitchInitial").c_str());
		detail::read(this->RocketData.PitchFinal, exINI, GameStrings::General, (pSection + "PitchFinal").c_str());
		detail::read(this->RocketData.TurnRate, exINI, GameStrings::General, (pSection + "TurnRate").c_str());

		// sic! integer read like a float.
		float buffer = 0.0f;
		if (detail::read(buffer, exINI, GameStrings::General, (pSection + "RaiseRate").c_str())) {
			this->RocketData.RaiseRate = int(buffer);
		}

		detail::read(this->RocketData.Acceleration, exINI, GameStrings::General, (pSection + "Acceleration").c_str());
		detail::read(this->RocketData.Altitude, exINI, GameStrings::General, (pSection + "Altitude").c_str());
		detail::read(this->RocketData.Damage, exINI, GameStrings::General, (pSection + "Damage").c_str());
		detail::read(this->RocketData.EliteDamage, exINI, GameStrings::General, (pSection + "EliteDamage").c_str());
		detail::read(this->RocketData.BodyLength, exINI, GameStrings::General, (pSection + "BodyLength").c_str());
		detail::read(this->RocketData.LazyCurve, exINI, GameStrings::General, (pSection + "LazyCurve").c_str());
		detail::read(this->RocketData.Type, exINI, GameStrings::General, (pSection + "Type").c_str());

		this->TrailerAnim = AnimTypeClass::Find(GameStrings::V3TRAIL());
		this->TakeoffAnim = AnimTypeClass::Find(GameStrings::V3TAKEOFF());

		switch (idx)
		{
		case 0:
		{
			this->PreLauchAnim = AnimTypeClass::Find(GameStrings::V3TAKEOFF());
			this->Offset->X = 40;
			this->Offset->Y = 40;
			break;
		}
		case 1:
		{
			break;
		}
		case 2:
		{
			break;
		}
		default:
			break;
		}

	}

	this->LoadFromINI(pINI);
}

void RocketTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name.c_str();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	detail::read(this->RocketData.PauseFrames, exINI, pSection,"PauseFrames");
	detail::read(this->RocketData.TiltFrames, exINI, pSection,"TiltFrames");
	detail::read(this->RocketData.PitchInitial, exINI, pSection,"PitchInitial");
	detail::read(this->RocketData.PitchFinal, exINI, pSection,"PitchFinal");
	detail::read(this->RocketData.TurnRate, exINI, pSection,"TurnRate");

	// sic! integer read like a float.
	float buffer = 0.0f;
	if (detail::read(buffer, exINI, pSection,"RaiseRate")) {
		this->RocketData.RaiseRate = int(buffer);
	}

	detail::read(this->RocketData.Acceleration, exINI, pSection,"Acceleration");
	detail::read(this->RocketData.Altitude, exINI, pSection,"Altitude");
	detail::read(this->RocketData.Damage, exINI, pSection,"Damage");
	detail::read(this->RocketData.EliteDamage, exINI, pSection,"EliteDamage");
	detail::read(this->RocketData.BodyLength, exINI, pSection,"BodyLength");
	detail::read(this->RocketData.LazyCurve, exINI, pSection,"LazyCurve");
	detail::read(this->RocketData.Type, exINI, pSection,"Type");

	std::string flag_raise = "RaiseBeforeLaunching";

	bool placeholder {};
	if (detail::read(placeholder, exINI, pSection, flag_raise.c_str())) {
		this->Raise.SetAll(placeholder);
	}

	detail::read(this->Raise.Rookie, exINI, pSection, (std::string(EnumFunctions::Rank_ToStrings[(int)Rank::Rookie]) + "." + flag_raise).c_str());
	detail::read(this->Raise.Veteran, exINI, pSection, (std::string(EnumFunctions::Rank_ToStrings[(int)Rank::Veteran]) + "." + flag_raise).c_str());
	detail::read(this->Raise.Elite, exINI, pSection, (std::string(EnumFunctions::Rank_ToStrings[(int)Rank::Elite]) + "." + flag_raise).c_str());

	this->Offset.Read(exINI, pSection, "CoordOffset");
	this->Warhead.Read(exINI, pSection, "Warhead");
	this->EliteWarhead.Read(exINI, pSection, "EliteWarhead");
	this->TakeoffAnim.Read(exINI, pSection, "TakeOffAnim");
	this->PreLauchAnim.Read(exINI, pSection, "PreLaunchAnim");
	this->TrailerAnim.Read(exINI, pSection, "TrailerAnim");
	this->TrailerSeparation.Read(exINI, pSection, "TrailerSeparation");
	this->Weapon.Read(exINI, pSection, "Weapon");
	this->EliteWeapon.Read(exINI, pSection, "EliteWeapon");

}