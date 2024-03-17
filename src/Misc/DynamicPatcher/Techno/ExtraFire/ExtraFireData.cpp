#include "ExtraFireData.h"

void ExtraFireData::ReadRules(INI_EX& parserRules, const char* pSection_rules)
{
	detail::ReadVectorsAlloc(AttachedWeapon.PrimaryWeapons, parserRules, pSection_rules, "ExtraFire.Primary");
	detail::ReadVectorsAlloc(AttachedWeapon.SecondaryWeapons, parserRules, pSection_rules, "ExtraFire.Secondary");

	detail::ReadVectorsAlloc(AttachedWeapon.ElitePrimaryWeapons, parserRules, pSection_rules, "ExtraFire.ElitePrimary");
	detail::ReadVectorsAlloc(AttachedWeapon.EliteSecondaryWeapons, parserRules, pSection_rules, "ExtraFire.EliteSecondary");

	char nBuff[0x200] {};
	int nSize = 0;
	AttachedWeapon.WeaponX.clear();
	AttachedWeapon.EliteWeaponX.clear();

	for (int a = 0; a < INT_MAX; a++)
	{
		if (!parserRules.ReadString(pSection_rules, (std::string("ExtraFire.Weapon") + std::to_string(a + 1)).c_str()))
			break;

		++nSize;
		detail::parse_Alloc_values<WeaponTypeClass*>(AttachedWeapon.WeaponX.emplace_back(), parserRules, pSection_rules, nBuff, true);
	}

	if (!AttachedWeapon.WeaponX.empty())
	{
		AttachedWeapon.EliteWeaponX.resize(nSize);

		for (int b = 0; b < nSize; b++)
		{
			std::string _buffer("ExtraFire.EliteWeapon");
			_buffer += std::to_string(b + 1);

			if (!parserRules.ReadString(pSection_rules, _buffer.c_str())) {
				AttachedWeapon.EliteWeaponX[b] = AttachedWeapon.WeaponX[b];
				continue;
			}

			detail::parse_Alloc_values<WeaponTypeClass*>(AttachedWeapon.EliteWeaponX[b], parserRules, pSection_rules, _buffer.c_str(), true);
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

	int nSize = 0;

	for (int i = 0; i < INT_MAX; i++)
	{
		Nullable<CoordStruct> nBuffRead_;
		std::string _base("ExtraFire.Weapon");
		_base += std::to_string(i + 1);

		nBuffRead_.Read(parserArt, pSection_Art, (_base + "FLH").c_str());

		if (!nBuffRead_.isset())
			break;

		++nSize;
		AttachedFLH.WeaponXFLH.push_back(nBuffRead_);
	}

	if (!AttachedFLH.WeaponXFLH.empty())
	{
		AttachedFLH.EliteWeaponXFLH.resize(nSize);

		for (int i = 0; i < nSize; i++)
		{
			Nullable<CoordStruct> nBuffReadE_;
			std::string _base("ExtraFire.EliteWeapon");
			_base += std::to_string(i + 1);

			nBuffReadE_.Read(parserArt, pSection_Art, (_base + "FLH").c_str());
			AttachedFLH.EliteWeaponXFLH[i] = nBuffReadE_.Get(AttachedFLH.WeaponXFLH[i]);
		}
	}
}
