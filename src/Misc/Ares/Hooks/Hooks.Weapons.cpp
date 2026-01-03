#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include "Header.h"

#include <Utilities/Macro.h>

ASMJIT_PATCH(0x7128B2, TechnoTypeClass_ReadINI_MultiWeapon, 0x6)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	enum { ReadWeaponX = 0x7128C0 };

	INI_EX exINI(pINI);
	const char* pSection = pThis->ID;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis);

	pTypeExt->MultiWeapon.Read(exINI, pSection, "MultiWeapon");
	bool multiWeapon = pThis->HasMultipleTurrets() || pTypeExt->MultiWeapon.Get();

	if (pTypeExt->ReadMultiWeapon != multiWeapon) {

		auto clearWeapon = [pThis](int index) {
			auto& pWeapon = pThis->Weapon[index];
			auto& pEliteWeapon = pThis->EliteWeapon[index];

			pWeapon = WeaponStruct();
			pEliteWeapon = WeaponStruct();
		};

		clearWeapon(0);
		clearWeapon(1);

		pTypeExt->ReadMultiWeapon = multiWeapon;
	}

	auto& SecondaryList = pTypeExt->MultiWeapon_IsSecondary;

	if (pTypeExt->MultiWeapon.Get())
	{
		pTypeExt->MultiWeapon_SelectCount.Read(exINI, pSection, "MultiWeapon.SelectCount");

		int weaponCount = pThis->WeaponCount;

		if (weaponCount > 0)
		{
			ValueableVector<int> isSecondary;
			isSecondary.Read(exINI, pSection, "MultiWeapon.IsSecondary");

			if (!isSecondary.empty())
			{
				SecondaryList.resize(weaponCount, false);

				for (int weaponIndex : isSecondary)
				{
					if (weaponIndex >= weaponCount || weaponIndex < 0)
						continue;

					SecondaryList[weaponIndex] = true;
				}
			}
		}
	}
	else
	{
		SecondaryList.clear();
	}

	return multiWeapon ? ReadWeaponX : 0;
}

ASMJIT_PATCH(0x715B10, TechnoTypeClass_ReadINI_MultiWeapon2, 0x7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	enum { ReadWeaponX = 0x715B1F, Continue = 0x715B17 };

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis);

	if (pTypeExt->ReadMultiWeapon)
		return ReadWeaponX;

	R->AL(pThis->HasMultipleTurrets());
	return Continue;
}

//TechnoTypeClass_LoadFromINI_Weapons2
DEFINE_JUMP(LJMP, 0x715B1F, 0x715F9E);

ASMJIT_PATCH(0x7128C0, TechnoTypeClass_LoadFromINI_Weapons1, 6)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(CCINIClass*, pINI, ESI);
	TechnoTypeExt_ExtData::ReadWeaponStructDatas(pThis, pINI);
	return 0x712A8F;
}

WeaponStruct* __fastcall FakeTechnoTypeClass::__GetWeapon(TechnoTypeClass* pThis , discard_t ,int which) {
	if (which < TechnoTypeClass::MaxWeapons)
		return pThis->Weapon + which;

	which -= TechnoTypeClass::MaxWeapons;
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis);
	const auto Vectors = &pExt->AdditionalWeaponDatas;

	if ((size_t)which < Vectors->size())
		return Vectors->data() + which;

	Debug::LogInfo("Techno[{}] Trying to get AdditionalWeapon with out of bound index[{}]", pThis->ID, which);
	return nullptr;
}

WeaponStruct* __fastcall FakeTechnoTypeClass::__GetEliteWeapon(TechnoTypeClass* pThis , discard_t ,int which) {
	if (which < TechnoTypeClass::MaxWeapons)
		return pThis->EliteWeapon + which;

	which -= TechnoTypeClass::MaxWeapons;
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis);
	const auto Vectors = &pExt->AdditionalEliteWeaponDatas;

	if ((size_t)which < Vectors->size())
		return Vectors->data() + which;

	Debug::LogInfo("Techno[{}] Trying to get AdditionalEliteWeapon with out of bound index[{}]", pThis->ID, which);
	return nullptr;
}

int __fastcall FakeTechnoTypeClass::__GetWeaponTurretIndex(TechnoTypeClass* pThis , discard_t ,int which) {
	if (which < TechnoTypeClass::MaxWeapons) {
		return *(pThis->TurretWeapon + which);
	}

	which -= TechnoTypeClass::MaxWeapons;
	const auto& vec = &TechnoTypeExtContainer::Instance.Find(pThis)->AdditionalTurrentWeapon;

	if ((size_t)which < vec->size())
		return *(vec->data() + which);

	Debug::LogInfo("Techno[{}] Trying to get AdditionalTurretWeaponIndex with out of bound index[{}]", pThis->ID, which);
	return 0;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7177C0, FakeTechnoTypeClass::__GetWeapon);
DEFINE_FUNCTION_JUMP(LJMP, 0x7177E0, FakeTechnoTypeClass::__GetEliteWeapon);
DEFINE_FUNCTION_JUMP(LJMP, 0x7178B0, FakeTechnoTypeClass::__GetWeaponTurretIndex);

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

	const auto pType = GET_TECHNOTYPE(pThis);

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