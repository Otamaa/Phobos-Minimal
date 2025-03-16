#include <Ext/TechnoType/Body.h>

#include "Header.h"

#include <Utilities/Macro.h>

//TechnoTypeClass_LoadFromINI_Weapons2
DEFINE_JUMP(LJMP, 0x715B1F, 0x715F9E);

ASMJIT_PATCH(0x7128C0, TechnoTypeClass_LoadFromINI_Weapons1, 6)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	TechnoTypeExt_ExtData::ReadWeaponStructDatas(pThis, pINI);
	return 0x712A8F;
}

WeaponStruct* FakeTechnoTypeClass::GetWeapon(int which) { 
	if (which < TechnoTypeClass::MaxWeapons)
		return this->Weapon + which;

	which -= TechnoTypeClass::MaxWeapons;
	const auto pExt = TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)this);
	const auto Vectors = &pExt->AdditionalWeaponDatas;

	if ((size_t)which < Vectors->size())
		return Vectors->data() + which;

	Debug::LogInfo("Techno[{}] Trying to get AdditionalWeapon with out of bound index[{}]", this->ID, which);
	return nullptr;
}

WeaponStruct* FakeTechnoTypeClass::GetEliteWeapon(int which) { 
	if (which < TechnoTypeClass::MaxWeapons)
		return this->EliteWeapon + which;

	which -= TechnoTypeClass::MaxWeapons;
	const auto pExt = TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)this);
	const auto Vectors = &pExt->AdditionalEliteWeaponDatas;

	if ((size_t)which < Vectors->size())
		return Vectors->data() + which;

	Debug::LogInfo("Techno[{}] Trying to get AdditionalEliteWeapon with out of bound index[{}]", this->ID, which);
	return nullptr;
}

int FakeTechnoTypeClass::GetWeaponTurretIndex(int which) { 
	if (which < TechnoTypeClass::MaxWeapons) {
		return *(this->TurretWeapon + which);
	}

	which -= TechnoTypeClass::MaxWeapons;
	const auto& vec = &TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)this)->AdditionalTurrentWeapon;

	if ((size_t)which < vec->size())
		return *(vec->data() + which);

	Debug::LogInfo("Techno[{}] Trying to get AdditionalTurretWeaponIndex with out of bound index[{}]", this->ID, which);
	return 0;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7177C0, FakeTechnoTypeClass::GetWeapon);
DEFINE_FUNCTION_JUMP(LJMP, 0x7177E0, FakeTechnoTypeClass::GetEliteWeapon);
DEFINE_FUNCTION_JUMP(LJMP, 0x7178B0, FakeTechnoTypeClass::GetWeaponTurretIndex);

ASMJIT_PATCH(0x747BCF, UnitTypeClass_LoadFromINI_Turrets, 5)
{
	GET(UnitTypeClass*, pThis, ESI);
	GET(CCINIClass*, pINI, EBX);

	if (pThis->Gunner)
		TechnoTypeExt_ExtData::LoadTurrets(pThis, pINI);

	return 0x747E90;
}

ASMJIT_PATCH(0x70DC70, TechnoClass_SwitchGunner, 6)
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

ASMJIT_PATCH(0x746B89, UnitClass_GetUIName, 8)
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

ASMJIT_PATCH(0x717890, TechnoTypeClass_SetWeaponTurretIndex, 8)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(int, nTurIdx, 0x4);
	GET_STACK(int, nWeaponIdx, 0x8);

	*TechnoTypeExt_ExtData::GetTurretWeaponIndex(pThis, nWeaponIdx) = nTurIdx;
	return 0x71789F;
}