#ifdef COMPILE_PORTED_DP_FEATURES
#include "ExtraFireData.h"

void ExtraFireData::ReadRules(INI_EX& parserRules, const char* pSection_rules)
{
	detail::ReadVectorsAlloc(AttachedWeapon.PrimaryWeapons, parserRules, pSection_rules, "ExtraFire.Primary");
	detail::ReadVectorsAlloc(AttachedWeapon.SecondaryWeapons, parserRules, pSection_rules, "ExtraFire.Secondary");

	detail::ReadVectorsAlloc(AttachedWeapon.ElitePrimaryWeapons, parserRules, pSection_rules, "ExtraFire.ElitePrimary");
	detail::ReadVectorsAlloc(AttachedWeapon.EliteSecondaryWeapons, parserRules, pSection_rules, "ExtraFire.EliteSecondary");

	char nBuff[0x200];
	std::vector<WeaponTypeClass*> nDummyVec_;
	int nSize = 0;

	for (int a = 0; a < INT_MAX; a++)
	{
		IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "ExtraFire.Weapon%d", a + 1);

		if (!parserRules.ReadString(pSection_rules, nBuff))
			break;

		++nSize;
		nDummyVec_.clear();
		detail::parse_Alloc_values<WeaponTypeClass*>(nDummyVec_, parserRules, pSection_rules, nBuff, true);
		AttachedWeapon.WeaponX.push_back(nDummyVec_);
	}

	if (!(nSize < 0))
	{
		for (int b = 0; b < nSize; b++)
		{
			IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "ExtraFire.EliteWeapon%d", b + 1);

			if (!parserRules.ReadString(pSection_rules, nBuff))
			{
				AttachedWeapon.EliteWeaponX.push_back(AttachedWeapon.WeaponX[b]);
				continue;
			}

			nDummyVec_.clear();
			detail::parse_Alloc_values<WeaponTypeClass*>(nDummyVec_, parserRules, pSection_rules, nBuff, true);
			AttachedWeapon.EliteWeaponX.push_back(nDummyVec_);
		}
	}

	nDummyVec_.clear();

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

	char nBuffArt[0x200];
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

	if (!(nSize < 0))
	{
		for (int i = 0; i < nSize; i++)
		{
			Nullable<CoordStruct> nBuffReadE_;
			IMPL_SNPRNINTF(nBuffArt, sizeof(nBuffArt), "ExtraFire.EliteWeapon%dFLH", i + 1);
			nBuffReadE_.Read(parserArt, pSection_Art, nBuffArt);
			AttachedFLH.EliteWeaponXFLH.push_back(nBuffReadE_.Get(AttachedFLH.WeaponXFLH[i]));
		}
	}
}
#endif