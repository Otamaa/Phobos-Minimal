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
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Bomb/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

ASMJIT_PATCH(0x75DDCC, WarheadTypeClass_GetVerses_Skipvanilla, 0x7)
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

	Debug::LogInfo("[{}] WH[{}][{}] against [{} - {}] Flag :  FF {} , PA {} , RR {} [{}] ",
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

#ifndef _test

DEFINE_FUNCTION_JUMP(LJMP, 0x489180, FakeWarheadTypeClass::ModifyDamageA);
DEFINE_FUNCTION_JUMP(CALL, 0x5F540F, FakeWarheadTypeClass::ModifyDamageA);
DEFINE_FUNCTION_JUMP(CALL, 0x6FDD29, FakeWarheadTypeClass::ModifyDamageA);
DEFINE_FUNCTION_JUMP(CALL, 0x701D64, FakeWarheadTypeClass::ModifyDamageA);

#else

ASMJIT_PATCH(0x489180, MapClass_GetTotalDamage, 0x6)
{
	GET(int, damage, ECX);
	GET(FakeWarheadTypeClass*, pWH, EDX);
	GET_STACK(Armor, armor, 0x4);
	GET_STACK(int, distance, 0x8);
	//GET_STACK(DWORD, caller, 0x0);

	int res = 0;

	if (damage == 0
		|| ScenarioClass::Instance->SpecialFlags.StructEd.Inert
		|| !pWH
		)
	{
		R->EAX(res);
		return 0x48926A;
	}

	const auto pExt = pWH->_GetExtData();

	if (damage > 0 || pExt->ApplyModifiersOnNegativeDamage)
	{
		if (pExt->ApplyMindamage)
			damage = MaxImpl(pExt->MinDamage >= 0 ? pExt->MinDamage : RulesClass::Instance->MinDamage, damage);

		const double dDamage = (double)damage;
		const float fDamage = (float)damage;
		const double dCellSpreadRadius = pWH->CellSpread * Unsorted::d_LeptonsPerCell;
		const int cellSpreadRadius = int(dCellSpreadRadius);

		const float Atmax = float(dDamage * pWH->PercentAtMax);
		const auto vsData = pWH->GetVersesData(armor);

		if (Atmax != dDamage && cellSpreadRadius) {
			res = int((fDamage - Atmax) * (double)(cellSpreadRadius - distance) / (double)cellSpreadRadius + Atmax);
		} else {
			res = damage;
		}

		if(!pExt->ApplyModifiersOnNegativeDamage)
			res = int(double(res <= 0 ? 0 : res) * vsData->Verses);
		else
			res = int(res * vsData->Verses);

		/**
		 *	Allow damage to drop to zero only if the distance would have
		 *	reduced damage to less than 1/4 full damage. Otherwise, ensure
		 *	that at least one damage point is done.
		 */
		if (pExt->ApplyMindamage && distance < 4)
			damage = MaxImpl(damage, pExt->MinDamage >= 0 ? pExt->MinDamage : RulesClass::Instance->MinDamage);

		damage = MinImpl(damage, RulesClass::Instance->MaxDamage);

	} else {
		res = distance >= 8 ? 0 : damage;
	}

	R->EAX(res);
	return 0x48926A;
}

ASMJIT_PATCH(0x489235, GetTotalDamage_Verses, 0x8)
{
	GET(FakeWarheadTypeClass*, pWH, EDI);
	GET(int, nArmor, EDX);
	GET(int, nDamage, ECX);

	const auto vsData = &pWH->_GetExtData()->Verses[nArmor];

	R->EAX(static_cast<int>((nDamage * vsData->Verses)));
	return 0x489249;
}
#endif

DEFINE_JUMP(LJMP, 0x700387, 0x7003BD)

#include <Ext/CaptureManager/Body.h>

ASMJIT_PATCH(0x70CEA0, TechnoClass_EvalThreatRating_TargetWeaponWarhead_Verses, 0x6)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(FakeWarheadTypeClass*, pTargetWH, EAX);
	GET_STACK(double, mult, 0x18);
	//GET(TechnoTypeClass*, pThisType, EBX);

	//const auto pData = WarheadTypeExtContainer::Instance.Find(pTargetWH);
	const auto armor = TechnoExtData::GetTechnoArmor(pThis, pTargetWH);
	const auto vsData = pTargetWH->GetVersesData(armor);

	double nMult = 0.0;

	if (pTarget->Target == pThis)
		nMult = -(mult * vsData->Verses);
	else
		nMult = mult * vsData->Verses;

	R->Stack(0x10, nMult);
	return 0x70CED2;
}

ASMJIT_PATCH(0x70CF45, TechnoClass_EvalThreatRating_ThisWeaponWarhead_Verses, 0xB)
{
	GET(ObjectClass*, pTarget, ESI);
	//GET(WeaponTypeClass*, pWeapon, EBX);
	GET(FakeWarheadTypeClass*, pWH, ECX);
	//GET(int, nArmor, EAX);
	GET_STACK(double, dmult, 0x10);
	GET_STACK(double, dCoeff, 0x30);

	Armor armor = TechnoExtData::GetTechnoArmor(pTarget , pWH);
	R->Stack(0x10, dCoeff * pWH->GetVersesData(armor)->Verses + dmult);

	return 0x70CF58;
}

ASMJIT_PATCH(0x4753F0, ArmorType_FindIndex, 0xA)
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
