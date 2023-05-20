#ifdef COMPILE_PORTED_DP_FEATURES
#include "ExtraFireData.h"

void ExtraFireData::ReadRules(INI_EX& parserRules, const char* pSection_rules)
{
	detail::parse_values(AttachedWeapon.PrimaryWeapons, parserRules, pSection_rules, "ExtraFire.Primary");
	detail::parse_values(AttachedWeapon.SecondaryWeapons, parserRules, pSection_rules, "ExtraFire.Secondary");

	detail::parse_values(AttachedWeapon.ElitePrimaryWeapons, parserRules, pSection_rules, "ExtraFire.ElitePrimary");
	detail::parse_values(AttachedWeapon.EliteSecondaryWeapons, parserRules, pSection_rules, "ExtraFire.EliteSecondary");

	char nBuff[0x200];
	int nSize = 0;
	AttachedWeapon.WeaponX.clear();
	AttachedWeapon.EliteWeaponX.clear();

	for (int a = 0; a < INT_MAX; a++)
	{
		IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "ExtraFire.Weapon%d", a + 1);

		if (!parserRules.ReadString(pSection_rules, nBuff))
			break;

		++nSize;
		std::vector<WeaponTypeClass*> Extra_WPN;
		detail::parse_Alloc_values<WeaponTypeClass*>(Extra_WPN, parserRules, pSection_rules, nBuff, true);
		AttachedWeapon.WeaponX.push_back(Extra_WPN);
	}

	if (!AttachedWeapon.WeaponX.empty())
	{
		AttachedWeapon.EliteWeaponX.resize(nSize);

		for (int b = 0; b < nSize; b++)
		{
			IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "ExtraFire.EliteWeapon%d", b + 1);

			if (!parserRules.ReadString(pSection_rules, nBuff)) {
				AttachedWeapon.EliteWeaponX[b] = AttachedWeapon.WeaponX[b];
				continue;
			}

			std::vector<WeaponTypeClass*> Extra_WPNE;
			detail::parse_Alloc_values<WeaponTypeClass*>(Extra_WPNE, parserRules, pSection_rules, nBuff, true);
			AttachedWeapon.EliteWeaponX[b] = Extra_WPNE;
		}
	}

}

void ExtraFireData::ReadArt(INI_EX& parserArt, const char* pSection_Art)
{
	// read flh from Art.ini
	AttachedFLH.PrimaryWeaponFLH.Read(parserArt, pSection_Art, "ExtraFire.PrimaryFLH");

	AttachedFLH.ElitePrimaryWeaponFLH.Read(parserArt, pSection_Art, "ExtraFire.ElitePrimaryFLH");

	if (!AttachedFLH.ElitePrimaryWeaponFLH.isset())
		AttachedFLH.ElitePrimaryWeaponFLH = AttachedFLH.PrimaryWeaponFLH;

	AttachedFLH.SecondaryWeaponFLH.Read(parserArt, pSection_Art, "ExtraFire.SecondaryFLH");

	AttachedFLH.EliteSecondaryWeaponFLH.Read(parserArt, pSection_Art, "ExtraFire.EliteSecondaryFLH");

	if (!AttachedFLH.EliteSecondaryWeaponFLH.isset())
		AttachedFLH.EliteSecondaryWeaponFLH = AttachedFLH.SecondaryWeaponFLH;

	char nBuffArt[0x40];
	int nSize = 0;

	for (int i = 0; i < INT_MAX; i++)
	{
		Nullable<CoordStruct> nBuffRead_;

		IMPL_SNPRNINTF(nBuffArt, sizeof(nBuffArt), "ExtraFire.Weapon%dFLH", i + 1);
		nBuffRead_.Read(parserArt, pSection_Art, nBuffArt);

		if (!nBuffRead_.isset())
			break;

		++nSize;
		AttachedFLH.WeaponXFLH.push_back(nBuffRead_);
	}

	if (!AttachedFLH.WeaponXFLH.empty())
	{
		AttachedFLH.EliteWeaponXFLH.reserve(nSize);

		for (int i = 0; i < nSize; i++)
		{
			Nullable<CoordStruct> nBuffReadE_;
			IMPL_SNPRNINTF(nBuffArt, sizeof(nBuffArt), "ExtraFire.EliteWeapon%dFLH", i + 1);
			nBuffReadE_.Read(parserArt, pSection_Art, nBuffArt);
			AttachedFLH.EliteWeaponXFLH[i] = nBuffReadE_.Get(AttachedFLH.WeaponXFLH[i]);
		}
	}
}
#endif