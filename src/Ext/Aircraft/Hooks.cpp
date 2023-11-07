#include "Body.h"

#include <AircraftClass.h>
#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

//bool SecondFiringMethod = true;
//
//DEFINE_HOOK(0x6FF031, TechnoClass_Fire_CountAmmo, 0xA)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (SecondFiringMethod && Is_Aircraft(pThis))
//	{
//		auto v2 = pThis->Ammo - 1;
//		if (v2 < 0)
//			v2 = 0;
//		pThis->Ammo = v2;
//	}
//
//	return 0x0;
//}
#include <Ext/Techno/Body.h>

#ifndef SecondMode
DEFINE_HOOK(0x417FE9, AircraftClass_Mission_Attack_StrafeShots, 0x7)
{
	GET(AircraftClass* const, pThis, ECX);

	if (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe
		)
	{
		return 0;
	}

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (const auto pWeaponStr = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target))) {
		if (pWeaponStr->WeaponType) {
			if (pExt->StrafeFireCunt > 0)
				pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget3_Strafe;

			if(pExt->StrafeFireCunt < 0 && pThis->MissionStatus == (int)AirAttackStatus::FireAtTarget2_Strafe)
				pExt->StrafeFireCunt = 0;

			if (pExt->StrafeFireCunt >= WeaponTypeExtContainer::Instance.Find(pWeaponStr->WeaponType)->Strafing_Shots.Get(5))
			{
				if (!pThis->Ammo)
					pThis->__DoingOverfly = false;

				pExt->StrafeFireCunt = -1;
				pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			}

			++pExt->StrafeFireCunt;
		}
	}

	return 0;
}

//was 8
#define Hook_AircraftBurstFix(addr , Mode , ret)\
DEFINE_HOOK(addr , AircraftClass_Mission_Attack_##Mode##_Strafe_BurstFix,0x6){ \
GET(AircraftClass* const, pThis, ESI); \
AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::##Mode##); return ret; }


	Hook_AircraftBurstFix(0x4186B6, FireAt, 0x4186D7)
	Hook_AircraftBurstFix(0x418805, Strafe2, 0x418826)
	Hook_AircraftBurstFix(0x418914, Strafe3, 0x418935)
	Hook_AircraftBurstFix(0x418A23, Strafe4, 0x418A44)
	Hook_AircraftBurstFix(0x418B1F, Strafe5, 0x418B40)

DEFINE_HOOK(0x418403, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0x6) //8
{
	GET(AircraftClass*, pThis, ESI);

	pThis->loseammo_6c8 = true;

	AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt);

	return 0x418478;
}

#undef Hook_AircraftBurstFix
#endif

DEFINE_HOOK(0x414F21, AircraftClass_AI_TrailerInheritOwner, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0xC));

	pAnim->AnimClass::AnimClass(pThis->Type->Trailer, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false);

	return 0x414F47;
}

enum class AirAttackStatusP : int
{
	AIR_ATT_VALIDATE_AZ = 0x0,
	AIR_ATT_PICK_ATTACK_LOCATION = 0x1,
	AIR_ATT_TAKE_OFF = 0x2,
	AIR_ATT_FLY_TO_POSITION = 0x3,
	AIR_ATT_FIRE_AT_TARGET0 = 0x4,
	AIR_ATT_FIRE_AT_TARGET1 = 0x5,
	AIR_ATT_FIRE_AT_TARGET2 = 0x6,
	AIR_ATT_FIRE_AT_TARGET3 = 0x7,
	AIR_ATT_FIRE_AT_TARGET4 = 0x8,
	AIR_ATT_FIRE_AT_TARGET5 = 0x9,
	AIR_ATT_RETURN_TO_BASE = 0xA,
};

//#pragma optimize("", off )
//int Mission_Attack(AircraftClass* pThis)
//{
//	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
//
//	if (/*!SecondFiringMethod &&*/ !(pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
//		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe))
//	{
//		const auto pWeaponStr = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target));
//
//		if (pWeaponStr && pWeaponStr->WeaponType)
//		{
//			int fireCount = pThis->MissionStatus - 4;
//			if (fireCount > 1 &&
//				WeaponTypeExtContainer::Instance.Find(pWeaponStr->WeaponType)->Strafing_Shots < fireCount)
//			{
//
//				if (!pThis->Ammo)
//					pThis->__DoingOverfly = false;
//
//				pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
//			}
//		}
//	}
//
//	switch ((AirAttackStatusP)pThis->MissionStatus)
//	{
//	case AirAttackStatusP::AIR_ATT_VALIDATE_AZ:
//	{
//		pThis->__DoingOverfly = false;
//
//		pThis->MissionStatus = //(pThis->Target ? -9 : 0) + (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE
//			!pThis->Target ? 11 : (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//
//		return 1;
//	}
//	case AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION:
//	{
//		pThis->__DoingOverfly = false;
//
//		if (pThis->loseammo_6c8)
//		{
//			pThis->loseammo_6c8 = false;
//			/*if(!SecondFiringMethod)*/
//				pThis->Ammo -= 1;
//		}
//
//		if (pThis->Target && pThis->Ammo)
//		{
//			pThis->SetDestination(pThis->GoodTargetLoc_(pThis->Target), 1);
//			pThis->MissionStatus = (int)(pThis->Destination != 0 ?
//				AirAttackStatusP::AIR_ATT_FLY_TO_POSITION : AirAttackStatusP::AIR_ATT_RETURN_TO_BASE);
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//		}
//
//		const auto v92 = pThis->GetCurrentMissionControl();
//		return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//	}
//	case AirAttackStatusP::AIR_ATT_FLY_TO_POSITION:
//	{
//		if (pThis->loseammo_6c8)
//		{
//			pThis->loseammo_6c8 = 0;
//
//			/*if(!SecondFiringMethod)*/
//				pThis->Ammo -= 1;
//		}
//
//		pThis->__DoingOverfly = false;
//
//		if (!pThis->Target || !pThis->Ammo)
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//
//		if (pThis->Is_Strafe())
//		{
//			const auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//
//			if (pThis->DistanceFrom(pThis->Target) < pPrimary->Range)
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				return 1;
//			}
//
//			pThis->SetDestination(pThis->Target, 1);
//		}
//		else
//		{
//			if (pThis->Is_Locked())
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				return 1;
//			}
//
//			if (!pThis->Locomotor->Is_Moving_Now()) {
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				return 1;
//			}
//		}
//
//		if (pThis->Destination)
//		{
//			const auto v13 = pThis->DistanceFrom(pThis->Destination);
//
//			if (v13 >= 512)
//			{
//				auto v16 = pThis->Destination->GetCoords();
//				auto v17 = pThis->GetFLH(0,CoordStruct::Empty);
//
//				DirStruct nDir {};
//
//				if (v17.X != v16.X || v17.Y != v16.Y) {
//					nDir.SetRadian<65536>(Math::atan2(double(v17.Y - v16.Y), double(v16.X - v17.X))); ;
//				}
//
//				pThis->SecondaryFacing.Set_Desired(nDir);
//
//				return 1;
//			}
//			else
//			{
//				pThis->SecondaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//
//				if (v13 >= 16)
//				{
//					return 1;
//				}
//				else
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//					pThis->SetDestination(nullptr, 1);
//					return 1;
//				}
//			}
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0:
//	{
//		if (!pThis->Target || !pThis->Ammo)
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//
//		if (/*SecondFiringMethod ||*/ !pThis->Is_Strafe())
//		{
//			pThis->PrimaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//			pThis->SecondaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//		}
//
//		auto v26 = pThis->SelectWeapon(pThis->Target);
//		auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//
//		switch (pThis->GetFireError(pThis->Target, v26, 1))
//		{
//		case FireError::OK:
//		{
//			pThis->loseammo_6c8 = 1;
//
//			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt , v26);
//			//pThis->Fire(pThis->Target, v26);
//
//			if(pThis->Target)
//			{
//				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//					->ScatterContent(pThis->Location, true, false, false);
//			}
//
//			if (pThis->Is_Strafe())
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET2;
//				pThis->__DoingOverfly = 1;
//				return pPrimary->ROF;
//			}
//
//			if (pThis->Is_Locked())
//			{
//				const auto v37 = pThis->Ammo;
//				bool v38 = v37 == 0;
//				bool v39 = v37 < 0;
//				pThis->__DoingOverfly = 1;
//				pThis->MissionStatus = !v39 && !v38 ?
//					(int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION : (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//				return pPrimary->ROF;
//			}
//
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1;
//			return 1;
//		}
//		case FireError::FACING:
//		{
//			if (!pThis->Ammo)
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//				return 1;
//			}
//
//			if (!pThis->IsCloseEnoughToAttack(pThis->Target) || /*!SecondFiringMethod ||*/ pThis->Is_Strafe())
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//			}
//			else if (pThis->Is_Locked())
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//			}
//			else
//			{
//				pThis->MissionStatus = pTypeExt->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle) ?
//					(int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1 : (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//			}
//
//			return !pThis->Is_Strafe() ? 1 : 45;
//		}
//		case FireError::REARM:
//		{
//			return 1;
//		}
//		case FireError::CLOAKED:
//		{
//			pThis->Uncloak(0);
//			return 1;
//		}
//		default:
//		{
//			if (!pThis->Ammo)
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//				return 1;
//			}
//
//			if (pThis->Is_Strafe())
//			{
//				return 1;
//			}
//
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1;
//			return 1;
//		}
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1:
//	{
//		if (pThis->Target)
//		{
//			pThis->PrimaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//			pThis->SecondaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//
//			auto v44 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v44, 1))
//			{
//			case FireError::OK:
//			{
//				AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt , v44);
//				//pThis->Fire(pThis->Target, v44);
//
//				if (pThis->Target)
//				{
//					MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//						->ScatterContent(pThis->Location, true, false, false);
//				}
//
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					const auto v92 = pThis->GetCurrentMissionControl();
//					return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//				}
//
//				pThis->MissionStatus = pTypeExt->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle) ?
//					(int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1 : (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//
//				const auto v92 = pThis->GetCurrentMissionControl();
//				return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//			}
//			case FireError::FACING:
//			{
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					const auto v92 = pThis->GetCurrentMissionControl();
//					return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//				}
//
//				if (!pThis->IsCloseEnoughToAttack(pThis->Target) || pThis->Is_Strafe())
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//				}
//				else
//				{
//					pThis->MissionStatus = pTypeExt->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle) ?
//						(int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1 : (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				}
//
//				if (!pThis->Is_Strafe())
//				{
//					const auto v92 = pThis->GetCurrentMissionControl();
//					return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//				}
//
//				return 45;
//			}
//			case FireError::REARM:
//			{
//				const auto v92 = pThis->GetCurrentMissionControl();
//				return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//			}
//			case FireError::CLOAKED:
//			{
//				pThis->Uncloak(0);
//				const auto v92 = pThis->GetCurrentMissionControl();
//				return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//			}
//			default:
//			{
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					const auto v92 = pThis->GetCurrentMissionControl();
//					return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//				}
//
//				if (pThis->IsCloseEnoughToAttack(pThis->Target))
//				{
//					pThis->MissionStatus = pTypeExt->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle) ?
//						(int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1 : (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				}
//				else
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//				}
//
//				const auto v92 = pThis->GetCurrentMissionControl();
//				return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//			}
//			}
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET2:
//	{
//		if (pThis->Target)
//		{
//			auto v52 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v52, 1))
//			{
//			case FireError::OK:
//			case FireError::FACING:
//			case FireError::CLOAKED:
//				break;
//			case FireError::RANGE:
//				pThis->SetDestination(pThis->Target, 1);
//				break;
//			default:
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					pThis->__DoingOverfly = 0;
//				}
//
//				return 1;
//			}
//
//			auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe2 , v52);
//			//pThis->Fire(pThis->Target, v52);
//
//			if (pThis->Target)
//			{
//				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//					->ScatterContent(pThis->Location, true, false, false);
//			}
//
//			pThis->SetDestination(pThis->Target, 1);
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET3;
//			return pPrimary->ROF;
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET3:
//	{
//		if (pThis->Target)
//		{
//			auto v26 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v26, 1))
//			{
//			case FireError::OK:
//			case FireError::FACING:
//			case FireError::CLOAKED:
//				break;
//			case FireError::RANGE:
//				pThis->SetDestination(pThis->Target, 1);
//				break;
//			default:
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					pThis->__DoingOverfly = 0;
//				}
//
//				return 1;
//			}
//
//			auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe3, v26);
//			//pThis->Fire(pThis->Target, v26);
//
//			if (pThis->Target)
//			{
//				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//					->ScatterContent(pThis->Location, true, false, false);
//			}
//
//			pThis->SetDestination(pThis->Target, 1);
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET4;
//			return pPrimary->ROF;
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET4:
//	{
//		if (pThis->Target)
//		{
//			auto v26 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v26, 1))
//			{
//			case FireError::OK:
//			case FireError::FACING:
//			case FireError::CLOAKED:
//				break;
//			case FireError::RANGE:
//				pThis->SetDestination(pThis->Target, 1);
//				break;
//			default:
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					pThis->__DoingOverfly = 0;
//				}
//
//				return 1;
//			}
//
//			auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe4, v26);
//			//pThis->Fire(pThis->Target, v26);
//
//			if (pThis->Target)
//			{
//				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//					->ScatterContent(pThis->Location, true, false, false);
//			}
//
//			pThis->SetDestination(pThis->Target, 1);
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET5;
//
//			return pPrimary->ROF;
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET5:
//	{
//		if (pThis->Target)
//		{
//			auto v26 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v26, 1))
//			{
//			case FireError::OK:
//			case FireError::FACING:
//			case FireError::RANGE:
//			case FireError::CLOAKED:
//			{
//				auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//				AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe5, v26);
//				//pThis->Fire(pThis->Target, v26);
//
//				if (pThis->Target)
//				{
//					MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//						->ScatterContent(pThis->Location, true, false, false);
//				}
//
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FLY_TO_POSITION;
//
//				return (pPrimary->Range + 1024) / pThis->Type->Speed;
//			}
//			default:
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					pThis->__DoingOverfly = 0;
//				}
//
//				return 1;
//			}
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_RETURN_TO_BASE:
//	{
//		pThis->__DoingOverfly = 0;
//
//		if (pThis->loseammo_6c8)
//		{
//			const auto v85 = pThis->Ammo;
//			pThis->loseammo_6c8 = 0;
//
//			if (v85 > 0) {
//				pThis->Ammo = v85 - 1;
//			}
//		}
//
//		if (pThis->Ammo) {
//			if (pThis->Target) {
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//				return 1;
//			}
//		}
//		else if (pThis->IsALoaner || pThis->Owner->IsControlledByCurrentPlayer())
//		{
//			pThis->SetTarget(nullptr);
//		}
//
//		pThis->__DoingOverfly = false;
//		pThis->SetDestination(
//			MapClass::Instance->GetCellAt(
//			MapClass::Instance->PickCellOnEdge(
//			pThis->Owner->GetCurrentEdge(),
//				CellStruct::Empty,
//				CellStruct::Empty,
//				SpeedType::Winged,
//				true,
//				MovementZone::Normal)), true
//		);
//
//		pThis->retreating_idle = false;
//		if (pThis->Airstrike && pThis->Ammo > 0)
//		{
//			pThis->QueueMission(Mission::Retreat, false);
//			pThis->retreating_idle = true;
//		}
//		else
//		{
//			pThis->EnterIdleMode(false, true);
//			pThis->retreating_idle = true;
//		}
//
//		return 1;
//	}
//	default:
//	{
//		const auto v92 = pThis->GetCurrentMissionControl();
//		return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//	}
//	}
//}
//#pragma optimize("", on )

// there is some funky shit happening here
// the code is 90% close to decomp but it result different
// need to investigae
// disabled atm !
//DEFINE_HOOK(0x417FE0, AircraftClass_MI_Attack_Handle, 0x6)
//{
//	R->EAX(Mission_Attack(R->ECX<AircraftClass*>()));
//	return 0x418D54;
//}