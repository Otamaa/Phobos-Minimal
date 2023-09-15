#include <Ext/TechnoType/Body.h>

static constexpr std::array<const char*, 17u> const SubName = {
	"Normal", "Repair" ,"MachineGun", "Flak" , "Pistol" , "Sniper" , "Shock",
	"Explode" , "BrainBlast" , "RadCannon" , "Chrono" , "TerroristExplode" ,
	"Cow" , "Initiate" , "Virus" , "YuriPrime" , "Guardian"
};

void NOINLINE LoadTurrets(TechnoTypeClass* pType, CCINIClass* pINI)
{
	INI_EX iniEx(pINI);

	const auto pSection = pType->ID;
	const int weaponCount = pType->WeaponCount >= 0 ? pType->WeaponCount : 0;
	const int addamount = weaponCount - TechnoTypeClass::MaxWeapons < 0 ? 0 : weaponCount - TechnoTypeClass::MaxWeapons;

	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	pExt->AdditionalTurrentWeapon.resize(addamount, -1);
	pExt->WeaponUINameX.resize(weaponCount);

	char buffer[0x20u];
	//read default
	for (size_t i = 0; i < SubName.size(); ++i)
	{
		const auto pName = SubName[i];

		_snprintf_s(buffer, sizeof(buffer), "%sTurretWeapon", pName);
		Valueable<int> read_buff { -1 };

		read_buff.Read(iniEx, pSection, buffer);

		if (read_buff >= 0)
		{
			_snprintf_s(buffer, sizeof(buffer), "%sTurretIndex", pName);
			Valueable<int> read_buff_ { int(i < 4u ? i : 0u) };

			read_buff_.Read(iniEx, pSection, buffer);

			if (read_buff_ >= 0) {
				*(read_buff < 18 ? (pType->TurretWeapon + read_buff) :
				(pExt->AdditionalTurrentWeapon.data() + (read_buff - TechnoTypeClass::MaxWeapons))) = read_buff_;
			}
		}
	}

	CSFText* CSF_ = pExt->WeaponUINameX.data();
	for (size_t i = 0; i < (size_t)weaponCount; ++i, ++CSF_)
	{
		_snprintf_s(buffer, sizeof(buffer), "WeaponTurretIndex%u", i + 1);
		Nullable<int> read_buff {};

		read_buff.Read(iniEx, pSection, buffer);
		int* result = i < 18 ?
			pType->TurretWeapon + i :
			pExt->AdditionalTurrentWeapon.data() + (i - TechnoTypeClass::MaxWeapons);

		if (read_buff.isset() && read_buff >= 0) {
			*result = read_buff;
		}

		if (*result < 0 || (pType->TurretCount > 0 && *result >= pType->TurretCount)) {
			Debug::Log("Weapon %d on [%s] has an invalid turret index of %d.\n", i + 1, pSection, *result);
			//*result = 0; //avoid crash
		}

		_snprintf_s(buffer, 0x20u, "WeaponUIName%u", i + 1);
		if (iniEx.ReadString(pSection, buffer))
			*CSF_ = iniEx.c_str();
	}
}

int* GetTurretWeaponIndex(TechnoTypeClass* pType, size_t idx)
{
	if (idx < TechnoTypeClass::MaxWeapons) {
		return pType->TurretWeapon + idx;
	}

	const int resultidx = (idx - TechnoTypeClass::MaxWeapons);
	const auto& vec = &TechnoTypeExt::ExtMap.Find(pType)->AdditionalTurrentWeapon;

	if ((size_t)resultidx < vec->size())
		return vec->data() + resultidx;

	Debug::Log("Techno[%s] Trying to get AdditionalTurretWeaponIndex with out of bound index[%d]\n", pType->ID, idx);
	return nullptr;
}

WeaponStruct*GetWeapon(TechnoTypeClass* pType, int const idx, bool elite)
{
	const auto pExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto Vectors = &(elite ? pExt->AdditionalEliteWeaponDatas : pExt->AdditionalWeaponDatas);

	if((size_t)idx < Vectors->size())
		return Vectors->data() + idx;

	Debug::Log("Techno[%s] Trying to get AdditionalWeapon with out of bound index[%d]\n", pType->ID, idx);
	return nullptr;
}

void NOINLINE ReadWeaponStructDatas(TechnoTypeClass* pType, CCINIClass* pRules)
{
	INI_EX iniEx(pRules);
	INI_EX iniEX_art(CCINIClass::INI_Art());

	const auto pSection = pType->ID;
	const auto pSection_art = pType->ImageFile;
	const int additionalamount = pType->WeaponCount - TechnoTypeClass::MaxWeapons < 0 ? 0 : pType->WeaponCount - TechnoTypeClass::MaxWeapons;
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	//Debug::Log("Resize Additional Weapon [%s] [%d]\n", pSection, additionalamount);

	pExt->AdditionalWeaponDatas.resize(additionalamount);
	pExt->AdditionalEliteWeaponDatas.resize(additionalamount);
	pExt->WeaponUINameX.resize(pType->WeaponCount);
	pExt->AdditionalTurrentWeapon.resize(additionalamount);

	//Debug::Log("After Resize Additional Weapon [%s] [%d- E %d]\n", pSection, pExt->AdditionalWeaponDatas.size() , pExt->AdditionalEliteWeaponDatas.size());


	for (int i = 0; i < pType->WeaponCount; ++i)
	{
		const int NextIdx = i < TechnoTypeClass::MaxWeapons ? i : i - TechnoTypeClass::MaxWeapons;
		//Debug::Log("Next Weapon Idx for [%s] [%d]\n", pSection, NextIdx);

		char buffer[0x40];
		char bufferWeapon[0x40];

		//=================================Normal============================//
		_snprintf(bufferWeapon, sizeof(bufferWeapon), "Weapon%u", i + 1);
		Valueable<WeaponTypeClass*> buffWeapon_ {};
		buffWeapon_.Read(iniEx, pSection, bufferWeapon, true);

		_snprintf(buffer, sizeof(buffer), "%sFLH", bufferWeapon);
		Valueable<CoordStruct> bufFLH_ {};
		bufFLH_.Read(iniEX_art, pSection_art, buffer);

		_snprintf(buffer, sizeof(buffer), "%sBarrelLength", bufferWeapon);
		Valueable<int> bufBrlLngth_ {};
		bufBrlLngth_.Read(iniEX_art, pSection_art, buffer);

		_snprintf(buffer, sizeof(buffer), "%sBarrelThickness", bufferWeapon);
		Valueable<int> bufBrlthic_ {};
		bufBrlthic_.Read(iniEX_art, pSection_art, buffer);

		_snprintf(buffer, sizeof(buffer), "%sTurretLocked", bufferWeapon);
		Valueable<bool> bufturrlck_ {};
		bufturrlck_.Read(iniEX_art, pSection_art, buffer);

		//=================================Elite============================//
		_snprintf(bufferWeapon, sizeof(bufferWeapon), "EliteWeapon%u", i + 1);
		Nullable<WeaponTypeClass*> buffWeapon_N {};
		buffWeapon_N.Read(iniEx, pSection, bufferWeapon, true);

		_snprintf(buffer, sizeof(buffer), "%sFLH", bufferWeapon);
		Nullable<CoordStruct> bufFLH_N {};
		bufFLH_N.Read(iniEX_art, pSection_art, buffer);

		_snprintf(buffer, sizeof(buffer), "%sBarrelLength", bufferWeapon);
		Nullable<int> bufBrlLngth_N {};
		bufBrlLngth_N.Read(iniEX_art, pSection_art, buffer);

		_snprintf(buffer, sizeof(buffer), "%sBarrelThickness", bufferWeapon);
		Nullable<int> bufBrlthic_N {};
		bufBrlthic_N.Read(iniEX_art, pSection_art, buffer);

		_snprintf(buffer, sizeof(buffer), "%sTurretLocked", bufferWeapon);
		Nullable<bool> bufturrlck_N {};
		bufturrlck_N.Read(iniEX_art, pSection_art, buffer);

		WeaponStruct temp { buffWeapon_ , bufFLH_.Get() , bufBrlLngth_, bufBrlthic_,  bufturrlck_ };
		WeaponStruct tempe { buffWeapon_N.Get(buffWeapon_), bufFLH_N.Get(bufFLH_), bufBrlLngth_N.Get(bufBrlLngth_), bufBrlthic_N.Get(bufBrlthic_), bufturrlck_N.Get(bufturrlck_) };
		std::memcpy((i < TechnoTypeClass::MaxWeapons ? pType->Weapon : pExt->AdditionalWeaponDatas.data()) + NextIdx, &temp, sizeof(WeaponStruct));
		std::memcpy((i < TechnoTypeClass::MaxWeapons ? pType->EliteWeapon : pExt->AdditionalEliteWeaponDatas.data()) + NextIdx, &tempe, sizeof(WeaponStruct));
	}
}

DEFINE_DISABLE_HOOK(0x715B1F, TechnoTypeClass_LoadFromINI_Weapons2_ares) //, 6
DEFINE_JUMP(LJMP, 0x715B1F, 0x715F9E);

DEFINE_OVERRIDE_HOOK(0x7128C0, TechnoTypeClass_LoadFromINI_Weapons1, 6)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	ReadWeaponStructDatas(pThis, pINI);
	return 0x712A8F;
}

DEFINE_OVERRIDE_HOOK(0x7177C0, TechnoTypeClass_GetWeapon, 0xB)
{
	GET_STACK(int, idx, 0x4);
	GET(TechnoTypeClass*, pThis, ECX);

	if (idx < 18)
		return 0x0;

	R->EAX(GetWeapon(pThis, idx - TechnoTypeClass::MaxWeapons, false));
	return 0x7177D4;
}

DEFINE_OVERRIDE_HOOK(0x7177E0, TechnoTypeClass_GetEliteWeapon, 0xB)
{
	GET_STACK(int, idx, 0x4);
	GET(TechnoTypeClass*, pThis, ECX);

	if (idx < 18)
		return 0x0;

	R->EAX(GetWeapon(pThis, idx - TechnoTypeClass::MaxWeapons, true));
	return 0x7177F4;
}

DEFINE_OVERRIDE_HOOK(0x747BCF, UnitTypeClass_LoadFromINI_Turrets, 5)
{
	GET(UnitTypeClass*, pThis, ESI);
	GET(CCINIClass*, pINI, EBX);

	if (pThis->Gunner)
		LoadTurrets(pThis, pINI);

	return 0x747E90;
}

DEFINE_OVERRIDE_HOOK(0x70DC70, TechnoClass_SwitchGunner, 6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, nWeaponIdx, 0x4);

	const auto pType = pThis->GetTechnoType();

	if (!pType->IsChargeTurret)
	{
		if (nWeaponIdx < 0 || nWeaponIdx >= pType->WeaponCount)
			nWeaponIdx = 0;

		pThis->CurrentTurretNumber = *GetTurretWeaponIndex(pType, nWeaponIdx);
		pThis->CurrentWeaponNumber = nWeaponIdx;
	}

	return 0x70DCDB;
}

DEFINE_OVERRIDE_HOOK(0x7178B0, TechnoTypeClass_GetWeaponTurretIndex, 0xB)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(int, nWeaponIdx, 0x4);
	R->EAX(*GetTurretWeaponIndex(pThis, nWeaponIdx));
	return 0x7178BB;
}

DEFINE_OVERRIDE_HOOK(0x746B89, UnitClass_GetUIName, 8)
{
	GET(UnitClass*, pThis, ESI);
	const auto pType = pThis->Type;
	const auto nCurWp = pThis->CurrentWeaponNumber;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	const wchar_t* Text =
		(size_t)nCurWp < pTypeExt->WeaponUINameX.size()
		? pTypeExt->WeaponUINameX[nCurWp].Text : nullptr;

	R->EAX(Text);

	return Text != nullptr ? 0x746C78 : 0;
}

DEFINE_OVERRIDE_HOOK(0x717890, TechnoTypeClass_SetWeaponTurretIndex, 8)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(int, nTurIdx, 0x4);
	GET_STACK(int, nWeaponIdx, 0x8);

	*GetTurretWeaponIndex(pThis, nWeaponIdx) = nTurIdx;
	return 0x71789F;
}