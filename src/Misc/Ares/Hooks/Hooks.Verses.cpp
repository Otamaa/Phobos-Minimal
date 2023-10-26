#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Bomb/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

DEFINE_OVERRIDE_HOOK(0x75DDCC, WarheadTypeClass_GetVerses_Skipvanilla, 0x7)
{
	// should really be doing something smarter due to westwood's weirdass code, but cannot be bothered atm
	// will fix if it is reported to actually break things
	// this breaks 51E33D which stops infantry with verses (heavy=0 & steel=0) from targeting non-infantry at all
	// (whoever wrote that code must have quite a few gears missing in his head)

	return 0x75DE98;
}

void Debug(ObjectClass* pTarget, int nArmor, VersesData* pData, WarheadTypeClass* pWH, const char* pAdd , size_t arrSize )
{
	auto const pArmor = ArmorTypeClass::FindFromIndex(nArmor);

	Debug::Log("[%s] WH[%d][%s] against [%d - %s] Flag :  FF %d , PA %d , RR %d [%fl] \n",
			pAdd,
			arrSize,
			pWH->get_ID(),
			nArmor,
			pArmor->Name.data(),
			pData->Flags.ForceFire,
			pData->Flags.PassiveAcquire,
			pData->Flags.Retaliate,
			pData->Verses);
}

DEFINE_HOOK(0x489180, MapClass_GetTotalDamage, 0x6)
{
	GET(int, damage, ECX);
	GET(WarheadTypeClass*, pWH, EDX);
	GET_STACK(int, armorIdx, 0x4);
	GET_STACK(int, distance, 0x8);

	if (damage == 0
		|| ScenarioClass::Instance->SpecialFlags.StructEd.Inert)
	{
		R->EAX(0);
		return 0x48926A;
	}

	const auto pExt = WarheadTypeExtContainer::Instance.Find(pWH);
	const double Atmax = (double)damage * (double)pWH->PercentAtMax;
	const int cellSpreadRadius = Game::F2I(pWH->CellSpread * 256.0);
	const auto vsData = &pExt->Verses[armorIdx];

	int damageTotal = damage;

	if (fabs(Atmax - damage) >= 1e-4 && cellSpreadRadius != 0)
		damageTotal = Game::F2I((damage - Atmax) * (cellSpreadRadius - distance) / cellSpreadRadius + Atmax);

	if (damage > 0)
		damageTotal = Game::F2I((damageTotal < 0 ? 0 : damageTotal) * vsData->Verses);
	else
		damageTotal = Game::F2I((damageTotal > 0 ? 0 : damageTotal) * vsData->Verses);

	R->EAX(damageTotal >= RulesClass::Instance->MaxDamage ? RulesClass::Instance->MaxDamage : damageTotal);
	return 0x48926A;
}

DEFINE_DISABLE_HOOK(0x489235, GetTotalDamage_Verses_ares)

DEFINE_OVERRIDE_HOOK(0x6F7D3D, TechnoClass_CanAutoTargetObject_Verses, 0x7)
{
	enum { ReturnFalse = 0x6F894F, ContinueCheck = 0x6F7D55, };

	//GET(ObjectClass*, pTarget, ESI);
	GET(WarheadTypeClass*, pWH, ECX);
	GET(int, nArmor, EAX);

	const auto pData = WarheadTypeExtContainer::Instance.Find(pWH);

	//if ((size_t)nArmor > ArmorTypeClass::Array.size())
	//	Debug::Log(__FUNCTION__" Armor is more that avaible ArmorTypeClass \n");

	const auto vsData = &pData->Verses[nArmor];

	return vsData->Flags.PassiveAcquire  //|| !(vsData->Verses <= 0.02)
		? ContinueCheck
		: ReturnFalse
		;
}

DEFINE_OVERRIDE_HOOK(0x6FCB6A, TechnoClass_CanFire_Verses, 0x7)
{
	enum { FireIllegal = 0x6FCB7E, ContinueCheck = 0x6FCBCD, };

	GET(ObjectClass*, pTarget, EBP);
	GET(WarheadTypeClass*, pWH, EDI);
	GET(int, nArmor, EAX);

	const auto pData = WarheadTypeExtContainer::Instance.Find(pWH);
	const auto vsData = &pData->Verses[nArmor];

	//if ((size_t)nArmor > ArmorTypeClass::Array.size())
	//	Debug::Log(__FUNCTION__" Armor is more that avaible ArmorTypeClass \n");

	// i think there is no way for the techno know if it attack using force fire or not
	if (vsData->Flags.ForceFire || vsData->Verses != 0.0)
	{
		if (pWH->BombDisarm &&
			(!pTarget->AttachedBomb ||
			!BombExtContainer::Instance.Find(pTarget->AttachedBomb)->Weapon->Ivan_Detachable)
		) {
			return FireIllegal;
		}

		if (pWH->IvanBomb && pTarget->AttachedBomb)
			return FireIllegal;

		return  ContinueCheck;
	}

	return FireIllegal;
}

DEFINE_OVERRIDE_HOOK(0x70CEA0, TechnoClass_EvalThreatRating_TargetWeaponWarhead_Verses, 0x6)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(WarheadTypeClass*, pTargetWH, EAX);
	GET_STACK(double, mult, 0x18);
	GET(TechnoTypeClass*, pThisType, EBX);

	const auto pData = WarheadTypeExtContainer::Instance.Find(pTargetWH);

	//if ((int)pThisType->Armor > ArmorTypeClass::Array.size())
	//	Debug::Log(__FUNCTION__" Armor is more that avaible ArmorTypeClass \n");

	const auto vsData = &pData->Verses[(int)pThisType->Armor];

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
	//GET(ObjectClass*, pTarget, ESI);
	//GET(WeaponTypeClass*, pWeapon, EBX);
	GET(WarheadTypeClass*, pWH, ECX);
	GET(int, nArmor, EAX);
	GET_STACK(double, dmult, 0x10);
	GET_STACK(double, dCoeff, 0x30);

	const auto pData = WarheadTypeExtContainer::Instance.Find(pWH);

	//if ((size_t)nArmor > ArmorTypeClass::Array.size())
	//	Debug::Log(__FUNCTION__" Armor is more that avaible ArmorTypeClass \n");

	const auto vsData = &pData->Verses[nArmor];

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
	GET_STACK(WeaponTypeClass*, pSecondary, 0x10); //secondary
	GET_STACK(WeaponTypeClass*, pPrimary, 0x14); //primary

	const int nArmor = (int)pTarget->GetTechnoType()->Armor;
	//if ((size_t)nArmor > ArmorTypeClass::Array.size())
	//	Debug::Log(__FUNCTION__" Armor is more that avaible ArmorTypeClass \n");

	const auto vsData_Secondary = &WarheadTypeExtContainer::Instance.Find(pSecondary->Warhead)->Verses[nArmor];

	if (vsData_Secondary->Verses == 0.0)
		return UsePrimary;

	const auto vsData_Primary = &WarheadTypeExtContainer::Instance.Find(pPrimary->Warhead)->Verses[nArmor];

	return vsData_Primary->Verses != 0.0 ? ContinueCheck : UseSecondary;
}

DEFINE_OVERRIDE_HOOK(0x708AF7, TechnoClass_ShouldRetaliate_Verses, 0x7)
{
	enum { Retaliate = 0x708B0B, DoNotRetaliate = 0x708B17 };

	GET(WarheadTypeClass*, pWH, ECX);
	GET(int, nArmor, EAX);

	const auto pData = WarheadTypeExtContainer::Instance.Find(pWH);
	//if ((size_t)nArmor > ArmorTypeClass::Array.size())
	//	Debug::Log(__FUNCTION__" Armor is more that avaible ArmorTypeClass \n");

	const auto vsData = &pData->Verses[nArmor];

	return vsData->Flags.Retaliate //|| !(vsData->Verses <= 0.0099999998)
		? Retaliate
		: DoNotRetaliate
		;
}

DEFINE_OVERRIDE_HOOK(0x4753F0, ArmorType_FindIndex, 0xA)
{
	GET(CCINIClass*, pINI, ECX);

	if(ArmorTypeClass::Array.empty())
	 ArmorTypeClass::AddDefaults();

	GET_STACK(const char*, Section, 0x4);
	GET_STACK(const char*, Key, 0x8);
	GET_STACK(int, fallback, 0xC);

	int nResult = fallback;
	char buf[0x64];

	if (pINI->ReadString(Section, Key, Phobos::readDefval, buf) > 0) {

		nResult = ArmorTypeClass::FindIndexById(buf);

		if (nResult < 0) {
			nResult = 0;
			Debug::INIParseFailed(Section, Key, buf , "Expect Valid ArmorType !");
		}
	}

	R->EAX(nResult);

	return 0x475430;
}
