#ifdef COMPILE_PORTED_DP_FEATURES
#include "ExtraFireData.h"
namespace ObjectTypeParser
{
	template<typename T>
	void Exec(CCINIClass* pINI, std::vector<DynamicVectorClass<T*>>& nVecDest, const char* pKey, bool bDebug = true)
	{
		for (int i = 0; i < pINI->GetKeyCount(pKey); ++i)
		{
			DynamicVectorClass<T*> _Buffer;
			char* context = nullptr;
			pINI->ReadString(pKey, pINI->GetKeyName(pKey, i), "", Phobos::readBuffer);

			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
				cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				T* buffer;
				if (Parser<T*>::TryParse(cur, &buffer))
				{
					if (buffer)
						_Buffer.AddItem(buffer);
				}
				else
				{
					if (bDebug)
						Debug::Log("ObjectTypeParser DEBUG: [%s][%d]: Error parsing [%s]\n", pKey, nVecDest.Count, cur);
				}
			}

			nVecDest.push_back(std::move(_Buffer));
			_Buffer.Clear();
		}
	}
};

void ExtraFireData::ReadRules(INI_EX& parserRules, const char* pSection_rules)
{
	AttachedWeapon.PrimaryWeapons.Read(parserRules, pSection_rules, "ExtraFire.Primary");
	AttachedWeapon.SecondaryWeapons.Read(parserRules, pSection_rules, "ExtraFire.Secondary");

	AttachedWeapon.ElitePrimaryWeapons.Read(parserRules, pSection_rules, "ExtraFire.ElitePrimary");
	AttachedWeapon.EliteSecondaryWeapons.Read(parserRules, pSection_rules, "ExtraFire.EliteSecondary");

	char nBuff[0x200];
	std::vector<WeaponTypeClass*> nDummyVec_;
	int nSize = 0;

	for (int a = 0; a < INT_MAX; a++)
	{
		_snprintf(nBuff, sizeof(nBuff), "ExtraFire.Weapon%d", a + 1);

		if (!parserRules.ReadString(pSection_rules, nBuff))
			break;

		++nSize;
		nDummyVec_.clear();
		detail::parse_values<WeaponTypeClass*>(nDummyVec_, parserRules, pSection_rules, nBuff, true);
		AttachedWeapon.WeaponX.push_back(nDummyVec_);
	}

	if (!(nSize < 0))
	{
		for (int b = 0; b < nSize; b++)
		{
			_snprintf(nBuff, sizeof(nBuff), "ExtraFire.EliteWeapon%d", b + 1);

			if (!parserRules.ReadString(pSection_rules, nBuff))
			{
				AttachedWeapon.EliteWeaponX.push_back(AttachedWeapon.WeaponX[b]);
				continue;
			}

			nDummyVec_.clear();
			detail::parse_values<WeaponTypeClass*>(nDummyVec_, parserRules, pSection_rules, nBuff, true);
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

		_snprintf(nBuffArt, sizeof(nBuffArt), "ExtraFire.Weapon%dFLH", i + 1);
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
			_snprintf(nBuffArt, sizeof(nBuffArt), "ExtraFire.EliteWeapon%dFLH", i + 1);
			nBuffReadE_.Read(parserArt, pSection_Art, nBuffArt);
			AttachedFLH.EliteWeaponXFLH.push_back(nBuffReadE_.Get(AttachedFLH.WeaponXFLH[i]));
		}
	}
}
#endif