#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

DEFINE_OVERRIDE_HOOK(0x75DDCC, Verses_OrigParser, 0x7)
{
	// should really be doing something smarter due to westwood's weirdass code, but cannot be bothered atm
	// will fix if it is reported to actually break things
	// this breaks 51E33D which stops infantry with verses (heavy=0 & steel=0) from targeting non-infantry at all
	// (whoever wrote that code must have quite a few gears missing in his head)
	return 0x75DE98;
}

DEFINE_OVERRIDE_HOOK(0x489235, GetTotalDamage_Verses, 0x8)
{
	GET(WarheadTypeClass*, pWH, EDI);
	GET(int, nArmor, EDX);
	GET(int, nDamage, ECX);

	auto pExt = WarheadTypeExt::ExtMap.Find(pWH);
	auto vsData = &pExt->Verses[nArmor];

	R->EAX(static_cast<int>((nDamage * vsData->Verses)));
	return 0x489249;
}

DEFINE_OVERRIDE_HOOK(0x6F7D3D, TechnoClass_CanAutoTargetObject_Verses, 0x7)
{
	enum { ReturnFalse = 0x6F894F, ContinueCheck = 0x6F7D55, };

	GET(WarheadTypeClass*, pWH, ECX);
	GET(int, nArmor, EAX);

	auto pData = WarheadTypeExt::ExtMap.Find(pWH);
	auto vsData = &pData->Verses[nArmor];

	return vsData->Flags.PassiveAcquire  //|| !(vsData->Verses <= 0.02)
		? ContinueCheck
		: ReturnFalse
		;
}

DEFINE_OVERRIDE_HOOK(0x6FCB6A, TechnoClass_CanFire_Verses, 0x7)
{
	enum { FireIllegal = 0x6FCB7E, ContinueCheck = 0x6FCB8D, };

	GET(WarheadTypeClass*, pWH, EDI);
	GET(int, nArmor, EAX);

	auto pData = WarheadTypeExt::ExtMap.Find(pWH);
	auto vsData = &pData->Verses[nArmor];

	return vsData->Flags.ForceFire || vsData->Verses != 0.0
		? ContinueCheck
		: FireIllegal
		;
}

DEFINE_OVERRIDE_HOOK(0x70CEA0, TechnoClass_EvalThreatRating_TargetWeaponWarhead_Verses, 0x6)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(WarheadTypeClass*, pWH, EAX);
	GET_STACK(double, mult, 0x18);
	GET(TechnoTypeClass*, pThisType, EBX);

	auto pData = WarheadTypeExt::ExtMap.Find(pWH);
	auto vsData = &pData->Verses[(int)pThisType->Armor];

	double nMult = 0.0;

	if (pTarget->Target == pThis)
		nMult = -(mult * vsData->Verses);
	else
		nMult = mult * vsData->Verses;

	R->Stack(0x10, nMult);
	return 0x70CED2;
}

DEFINE_OVERRIDE_HOOK(0x70CF45, TechnoClass_EvalThreatRating_ThisWeaponWarhead_Verses, 0xB)
{
	//GET(WeaponTypeClass*, pWeapon, EBX);
	GET(WarheadTypeClass*, pWH, ECX);
	GET(int, nArmor, EAX);
	GET_STACK(double, dmult, 0x10);
	GET_STACK(double, dCoeff, 0x30);

	auto pData = WarheadTypeExt::ExtMap.Find(pWH);
	auto vsData = &pData->Verses[nArmor];

	R->Stack(0x10, dCoeff * vsData->Verses + dmult);
	return 0x70CF58;
}

DEFINE_OVERRIDE_HOOK(0x6F36E3, TechnoClass_SelectWeapon_Verses, 0x5)
{
	enum
	{
		UseSecondary = 0x6F3745,
		ContinueCheck = 0x6F3754,
		UsePrimary = 0x6F37AD,
	};

	GET(TechnoClass*, pTarget, EBP);
	GET_STACK(WeaponTypeClass*, pWeapon_A, 0x10);
	GET_STACK(WeaponTypeClass*, pWeapon_B, 0x14);

	int nArmor = (int)pTarget->GetTechnoType()->Armor;
	auto vsData_A = &WarheadTypeExt::ExtMap.Find(pWeapon_A->Warhead)->Verses[nArmor];

	if (vsData_A->Verses == 0.0)
		return UsePrimary;

	auto vsData_B = &WarheadTypeExt::ExtMap.Find(pWeapon_B->Warhead)->Verses[nArmor];

	return vsData_B->Verses != 0.0 ? ContinueCheck : UseSecondary;
}

DEFINE_OVERRIDE_HOOK(0x708AF7, TechnoClass_ShouldRetaliate_Verses, 0x7)
{
	enum { Retaliate = 0x708B0B, DoNotRetaliate = 0x708B17 };

	GET(WarheadTypeClass*, pWH, ECX);
	GET(int, nArmor, EAX);

	auto pData = WarheadTypeExt::ExtMap.Find(pWH);
	auto vsData = &pData->Verses[nArmor];

	return vsData->Flags.Retaliate //|| !(vsData->Verses <= 0.0099999998)
		? Retaliate
		: DoNotRetaliate
		;
}

DEFINE_OVERRIDE_HOOK(0x4753F0, ArmorType_FindIndex, 0xA)
{
	GET(CCINIClass*, pINI, ECX);
	if (ArmorTypeClass::Array.empty())
	{
		ArmorTypeClass::AddDefaults();
	}

	GET_STACK(const char*, Section, 0x4);
	GET_STACK(const char*, Key, 0x8);
	//GET_STACK(int, fallback, 0xC);

	char buf[0x64];
	pINI->ReadString(Section, Key, Phobos::readDefval, buf);

	//if(strnlen_s(buf,sizeof(buf)) > 0) {

	int idx = ArmorTypeClass::FindIndexById(buf);

	if (idx < 0)
	{
		idx = 0;

		if (strlen(buf))
			Debug::INIParseFailed(Section, Key, buf);
	}

	R->EAX(idx);
	//}

	return 0x475430;
}