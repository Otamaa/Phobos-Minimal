#include "SpawnSupportData.h"
#ifdef COMPILE_PORTED_DP_FEATURES
void SpawnSupportFLHData::Read(INI_EX& nParser, const char* pSection)
{
	SpawnSupportFLH.Read(nParser, pSection, "SupportWeaponFLH");
	EliteSpawnSupportFLH.Read(nParser, pSection, "EliteSupportWeaponFLH");

	if (EliteSpawnSupportFLH.Get() == CoordStruct::Empty)
		EliteSpawnSupportFLH = SpawnSupportFLH;

	SpawnHitFLH.Read(nParser, pSection, "SupportWeaponHitFLH");
	EliteSpawnHitFLH.Read(nParser, pSection, "EliteSupportWeaponHitFLH");

	if (EliteSpawnHitFLH.Get() == CoordStruct::Empty)
		EliteSpawnHitFLH = SpawnHitFLH;
}

void SpawnSupportData::Read(INI_EX& nParser, const char* pSection)
{
	Enable.Read(nParser, pSection, "SupportSpawns");

	if (Enable)
	{
		SupportWeapon.Read(nParser, pSection, "SupportSpawns.Weapon", true);
		EliteSupportWeapon.Read(nParser, pSection, "SupportSpawns.EliteWeapon", true);

		if (!EliteSupportWeapon)
			SupportWeapon = EliteSupportWeapon;

		SwitchFLH.Read(nParser, pSection, "SupportSpawns.SwitchFLH");
		Always.Read(nParser, pSection, "SupportSpawns.AlwaysFire");
	}

	FireOnce.Read(nParser, pSection, "SpawnFireOnce");
	Delay.Read(nParser, pSection, "SpawnFireOnceDelay");
}
#endif