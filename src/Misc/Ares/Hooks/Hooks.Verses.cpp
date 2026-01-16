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

WeaponTypeClass* LastWeapon = nullptr;

ASMJIT_PATCH(0x6F7D3D, TechnoClass_EvaluateObject_Verses, 0x7)
{
	enum { ReturnFalse = 0x6F894F, ContinueCheck = 0x6F7D55, };

	GET(ObjectClass*, pTarget, ESI);
	GET(FakeWarheadTypeClass*, pWH, ECX);
	GET(WeaponTypeClass*, pWeapon , EBP);
	//GET_STACK(int const, nWeapon, 0x14);
	//GET(int, nArmor, EAX);

	LastWeapon = pWeapon;
	//const auto pData = WarheadTypeExtContainer::Instance.Find(pWH);
	Armor armor = TechnoExtData::GetTechnoArmor(pTarget, pWH);
	return pWH->GetVersesData(armor)->Flags.PassiveAcquire  //|| !(vsData->Verses <= 0.02)
		? ContinueCheck
		: ReturnFalse
		;
}

ASMJIT_PATCH(0x6F7EF4, TechnoClass_EvaluateObject_AttackFriendlies, 0xA)
{
	enum { SkipGameCode = 0x6F7F04 , AllowAttack = 0x6F7FE9, ContinueCheck = 0x6F7F0C };

	GET(TechnoClass*, pThis, EDI);
	GET_STACK(int const, nWeapon, 0x14);

	bool attackFriendlies = pThis->GetTechnoType()->AttackFriendlies;

	if (LastWeapon) {
		const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(LastWeapon);
		attackFriendlies = pWeaponExt->AttackFriendlies.Get(attackFriendlies);
	}

	if (!attackFriendlies)
		return ContinueCheck;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	return pTypeExt->AttackFriendlies_WeaponIdx <= -1
		|| pTypeExt->AttackFriendlies_WeaponIdx == nWeapon
		? AllowAttack : ContinueCheck;
}

ASMJIT_PATCH(0x6F85CF, TechnoClass_EvaluateObject_AttackNoThreatBuildings, 0xA)
{
	enum { CanAttack = 0x6F8604, Continue = 0x6F85D9 };

	GET(TechnoClass*, pThis, EDI);
	GET(BuildingClass*, pTarget, ESI);

	bool canAttack = pThis->Owner->IsControlledByHuman() ? RulesExtData::Instance()->AutoTarget_NoThreatBuildings :
															RulesExtData::Instance()->AutoTargetAI_NoThreatBuildings;

	if (LastWeapon) {
		const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(LastWeapon);
		canAttack = pWeaponExt->AttackNoThreatBuildings.Get(canAttack);
	}

	if (canAttack)
		return CanAttack;

	R->EAX(pTarget->GetTurrentWeapon());
	return Continue;
}

DEFINE_JUMP(LJMP, 0x700387, 0x7003BD)

ASMJIT_PATCH(0x6F85AB, TechnoClass_EvaluateObject_AggressiveAttackMove, 0x6)
{
	enum { ContinueCheck = 0x6F85E2, CanTarget = 0x6F8604 };

	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);

	// if (!pThis->Owner->IsControlledByHuman())
	// 	return CanTarget;

	if (pThis->MegaMissionIsAttackMove()) {
		const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

		if(pTypeExt->AttackMove_Aggressive.Get(RulesExtData::Instance()->AttackMove_UpdateTarget))
			return CanTarget;
	}

	if(pThis->IsStrange())
		return CanTarget;

	AffectedTechno aff = AffectedTechno::Aircraft | AffectedTechno::Infantry | AffectedTechno::Unit;
	if(LastWeapon){
		auto pExt = WarheadTypeExtContainer::Instance.Find(LastWeapon->Warhead);
		if(pExt->IsFakeEngineer)
			aff |= AffectedTechno::Building;
	}

	if(EnumFunctions::CanAffectTechnoResult(pTarget->WhatAmI(), aff))
		return CanTarget;

	return ContinueCheck;
}

#include <Ext/CaptureManager/Body.h>

ASMJIT_PATCH(0x6FCAFA, TechnoClass_CanFire_Verses, 0x8)
{
	enum {
		FireIllegal = 0x6FCB7E,
		ContinueCheck = 0x6FCBCD,
		ContinueCheckB = 0x6FCCBD,
		ForceNewValue = 0x6FCBA6,
		TargetIsNotTechno = 0x6FCCBD
	};

	GET(TechnoClass*, pThis , ESI);
	GET(TechnoClass*, pTarget, EBP);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if(!pTarget){
		return TargetIsNotTechno;
	}

	if(pTarget->IsSinking)
		return FireIllegal;

	auto pWH = (FakeWarheadTypeClass*)pWeapon->Warhead;

	if(pWH->Parasite && pTarget->IsIronCurtained()) {
		return FireIllegal;
	}

	if(pWH->MindControl){
		if(auto pManager = (FakeCaptureManagerClass*)pThis->CaptureManager) {
			if(!pManager->__CanCapture(pTarget)){
				return FireIllegal;
			}
		}
	}

	Armor armor = TechnoExtData::GetTechnoArmor(pTarget, pWH);
	const auto vsData = pWH->GetVersesData(armor);

	// i think there is no way for the techno know if it attack using force fire or not
	if (vsData->Flags.ForceFire || vsData->Verses != 0.0) {
		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

		if(pWHExt->FakeEngineer_CanCaptureBuildings || pWHExt->FakeEngineer_BombDisarm) {
			const int weaponRange = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);
			const int currentRange = pThis->DistanceFrom(pTarget);

			if (pWHExt->FakeEngineer_BombDisarm
				&& pTarget->AttachedBomb
				&& BombExtContainer::Instance.Find(pTarget->AttachedBomb)->Weapon->Ivan_Detachable) {

				if (currentRange <= weaponRange)
					R->EAX(FireError::OK);
				else
					R->EAX(FireError::RANGE); // Out of range

				return ForceNewValue;
			}

			if (pWHExt->FakeEngineer_CanCaptureBuildings) {
				const auto pBuilding = cast_to<BuildingClass*, false>(pTarget);
				const bool UneableToCapture = !pBuilding
				|| !pBuilding->IsAlive
				|| pBuilding->Health <= 0
				|| pThis->Owner->IsAlliedWith(pTarget)
				|| (!pBuilding->Type->Capturable && !pBuilding->Type->NeedsEngineer);

				if (!UneableToCapture) {
					if (currentRange <= weaponRange)
						R->EAX(FireError::OK);
					else
						R->EAX(FireError::RANGE); // Out of range

					return ForceNewValue;
				}
			}
		}

		if (pWH->BombDisarm &&
			(!pTarget->AttachedBomb ||
				!BombExtContainer::Instance.Find(pTarget->AttachedBomb)->Weapon->Ivan_Detachable)
		) {
			return FireIllegal;
		}

		if (pWH->IvanBomb && pTarget->AttachedBomb)
			return FireIllegal;

		// Skips bridge-related coord checks to allow AA to target air units on bridges over water.
		if(pTarget->IsInAir()){
			return ContinueCheckB;
		}

		//elevation related checks
		return  ContinueCheck;
	}

	return FireIllegal;
}

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

// shield weapon selecting is applied earlier
//ASMJIT_PATCH(0x6F36E3, TechnoClass_SelectWeapon_Verses, 0x5)
//{
//	enum
//	{
//		UseSecondary = 0x6F3745,
//		ContinueCheck = 0x6F3754,
//		UsePrimary = 0x6F37AD,
//	};
//
//	GET(TechnoClass*, pTarget, EBP);
//	GET_STACK(WeaponTypeClass*, pSecondary, 0x10); //secondary
//	GET_STACK(WeaponTypeClass*, pPrimary, 0x14); //primary
//
//	const int nArmor = (int)TechnoExtData::GetArmor(pTarget);
//	if ((size_t)nArmor > ArmorTypeClass::Array.size())
//		Debug::LogInfo(__FUNCTION__" Armor is more that avaible ArmorTypeClass ");
//
//	const auto vsData_Secondary = &WarheadTypeExtContainer::Instance.Find(pSecondary->Warhead)->Verses[nArmor];
//
//	if (vsData_Secondary->Verses == 0.0)
//		return UsePrimary;
//
//	const auto vsData_Primary = &WarheadTypeExtContainer::Instance.Find(pPrimary->Warhead)->Verses[nArmor];
//
//	return vsData_Primary->Verses != 0.0 ? ContinueCheck : UseSecondary;
//}

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
