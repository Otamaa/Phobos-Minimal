#include <Ext/TechnoType/Body.h>

#include "Header.h"

#include <Utilities/Macro.h>

//TechnoTypeClass_LoadFromINI_Weapons2
DEFINE_JUMP(LJMP, 0x715B1F, 0x715F9E);

DEFINE_HOOK(0x7128C0, TechnoTypeClass_LoadFromINI_Weapons1, 6)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	TechnoTypeExt_ExtData::ReadWeaponStructDatas(pThis, pINI);
	return 0x712A8F;
}

DEFINE_HOOK(0x7177C0, TechnoTypeClass_GetWeapon, 0xB)
{
	GET_STACK(int, idx, 0x4);
	GET(TechnoTypeClass*, pThis, ECX);

	if (idx < 18)
		return 0x0;

	R->EAX(TechnoTypeExt_ExtData::GetWeapon(pThis, idx - TechnoTypeClass::MaxWeapons, false));
	return 0x7177D4;
}

DEFINE_HOOK(0x7177E0, TechnoTypeClass_GetEliteWeapon, 0xB)
{
	GET_STACK(int, idx, 0x4);
	GET(TechnoTypeClass*, pThis, ECX);

	if (idx < 18)
		return 0x0;

	R->EAX(TechnoTypeExt_ExtData::GetWeapon(pThis, idx - TechnoTypeClass::MaxWeapons, true));
	return 0x7177F4;
}

DEFINE_HOOK(0x747BCF, UnitTypeClass_LoadFromINI_Turrets, 5)
{
	GET(UnitTypeClass*, pThis, ESI);
	GET(CCINIClass*, pINI, EBX);

	if (pThis->Gunner)
		TechnoTypeExt_ExtData::LoadTurrets(pThis, pINI);

	return 0x747E90;
}

DEFINE_HOOK(0x70DC70, TechnoClass_SwitchGunner, 6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, nWeaponIdx, 0x4);

	const auto pType = pThis->GetTechnoType();

	if (!pType->IsChargeTurret)
	{
		if (nWeaponIdx < 0 || nWeaponIdx >= pType->WeaponCount)
			nWeaponIdx = 0;

		pThis->CurrentTurretNumber = *TechnoTypeExt_ExtData::GetTurretWeaponIndex(pType, nWeaponIdx);
		pThis->CurrentWeaponNumber = nWeaponIdx;
	}

	return 0x70DCDB;
}

DEFINE_HOOK(0x7178B0, TechnoTypeClass_GetWeaponTurretIndex, 0xB)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(int, nWeaponIdx, 0x4);
	R->EAX(*TechnoTypeExt_ExtData::GetTurretWeaponIndex(pThis, nWeaponIdx));
	return 0x7178BB;
}

DEFINE_HOOK(0x746B89, UnitClass_GetUIName, 8)
{
	GET(UnitClass*, pThis, ESI);
	const auto pType = pThis->Type;
	const auto nCurWp = pThis->CurrentWeaponNumber;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	const wchar_t* Text =
		(size_t)nCurWp < pTypeExt->WeaponUINameX.size()
		? pTypeExt->WeaponUINameX[nCurWp].Text : L"Error Invalid Name";

	R->EAX(Text);

	return Text != nullptr ? 0x746C78 : 0;
}

DEFINE_HOOK(0x717890, TechnoTypeClass_SetWeaponTurretIndex, 8)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(int, nTurIdx, 0x4);
	GET_STACK(int, nWeaponIdx, 0x8);

	*TechnoTypeExt_ExtData::GetTurretWeaponIndex(pThis, nWeaponIdx) = nTurIdx;
	return 0x71789F;
}