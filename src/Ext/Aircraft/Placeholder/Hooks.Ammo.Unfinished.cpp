
#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

/*
	Otamaa : almost there , just some weird incosistency with vanilla one need more test !
*/

int MissionAttack_Strafe(AircraftClass* pThis, bool CheckAmmo)
{
	if (auto pTarget = pThis->Target)
	{
		const int weaponIndex = pThis->SelectWeapon(pTarget);

		switch (pThis->GetFireError(pTarget, weaponIndex, true))
		{
		case FireError::OK:
		case FireError::FACING:
		case FireError::CLOAKED:
		{
			goto Execute_Firing;
		}
		case FireError::RANGE:
		{
			if (pThis->MissionStatus != 9)
				pThis->SetDestination(pTarget, true);
		Execute_Firing:
			auto weaponType = pThis->GetWeapon(weaponIndex)->WeaponType;
			auto pWeaponTypeExt = WeaponTypeExtContainer::Instance.Find(weaponType);

			if (weaponType->Burst > 0)
			{
				for (int i = 0; i < weaponType->Burst; i++)
				{
					if (weaponType->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
						pThis->CurrentBurstIndex = pThis->MissionStatus - 4;

					pThis->Fire(pThis->Target, weaponIndex);
				}
			}


			MapClass::Instance->GetCellAt(pTarget->GetCoords())
				->ScatterContent(pThis->Location, true, false, false);
			pThis->SetDestination(pTarget, true);

			if (CheckAmmo)
			{
				if (!pThis->Ammo)
					pThis->MissionStatus = 3;

				return (pThis->GetWeapon(weaponIndex)->WeaponType->Range + 0x400) / pThis->Type->Speed;
			}
			else
			{
				if (pThis->MissionStatus == 9)
				{
					pThis->MissionStatus = 3;
					return (pThis->GetWeapon(weaponIndex)->WeaponType->Range + 0x400) / pThis->Type->Speed;
				}

				pThis->MissionStatus += 1;
			}

			return pThis->GetWeapon(weaponIndex)->WeaponType->Range;
		}
		default:
		{
			if (!pThis->Ammo)
			{
				pThis->MissionStatus = 10;
				pThis->__DoingOverfly = false;
			}

			return 1;
		}
		}
	}
	else
	{
		pThis->MissionStatus = 10;
		return 1;
	}
}

int Mission_Attack(AircraftClass* pThis)
{
	const bool bCond = (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe);

	if (!bCond)
	{
		if (pThis->Target)
		{
			const int weaponIndex = pThis->SelectWeapon(pThis->Target);

			if (const auto pWeaponStr = pThis->GetWeapon(weaponIndex))
			{
				if (pWeaponStr->WeaponType)
				{
					if (auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeaponStr->WeaponType))
					{
						int fireCount = pThis->MissionStatus - 4;
						if (fireCount > 1 && pWeaponExt->Strafing_Shots < fireCount)
						{
							if (!pThis->Ammo)
								pThis->__DoingOverfly = false;

							pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
						}
					}
				}
			}
		}
	}

	switch (static_cast<AirAttackStatusIDB>(pThis->MissionStatus))
	{
	case AirAttackStatusIDB::AIR_ATT_VALIDATE_AZ:
	{
		pThis->__DoingOverfly = 0;
		pThis->MissionStatus = pThis->Target ? (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION : (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
		return 1;
	}
	case AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION:
	{
		pThis->__DoingOverfly = 0;
		if (pThis->loseammo_6c8)
		{
			pThis->loseammo_6c8 = 0;
			--pThis->Ammo;
		}

		if (pThis->Target && pThis->Ammo)
		{
			pThis->SetDestination(pThis->GoodTargetLoc_(pThis->Target), true);
			pThis->MissionStatus = pThis->Destination != 0 ? (int)AirAttackStatusIDB::AIR_ATT_FLY_TO_POSITION : (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
		}
		else
		{
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
		}

		return int(pThis->GetCurrentMissionControl()->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
	}
	case AirAttackStatusIDB::AIR_ATT_FLY_TO_POSITION:
	{
		if (pThis->loseammo_6c8)
		{
			pThis->loseammo_6c8 = 0;
			--pThis->Ammo;
		}

		pThis->__DoingOverfly = 0;

		if (!pThis->Target || !pThis->Ammo)
		{
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
			return 1;
		}

		if (pThis->Is_Strafe())
		{
			//follow vanilla , get primary
			if (pThis->DistanceFrom(pThis->Target) < pThis->GetWeapon(0)->WeaponType->Range)
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
				return 1;
			}

			pThis->SetDestination(pThis->Target, 1);
		}
		else
		{
			if (pThis->Is_Locked())
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
				return 1;
			}

			if (!pThis->Locomotor->Is_Moving_Now())
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
				return 1;
			}
		}

		if (pThis->Destination)
		{
			const auto nDistance = pThis->DistanceFrom(pThis->Destination);
			if (nDistance >= 512)
			{
				auto nCorrd = pThis->Destination->GetCoords();
				CoordStruct nFLH;
				pThis->GetFLH(&nFLH ,0, CoordStruct::Empty);
				DirStruct nDirDef { 0 };

				if (nFLH.X != nCorrd.X || nFLH.Y != nCorrd.Y)
				{
					nDirDef.SetRadian<65536>(Math::atan2(static_cast<double>(nFLH.Y - nCorrd.Y), static_cast<double>(nFLH.X - nCorrd.X)));
					pThis->SecondaryFacing.Set_Desired(nDirDef);
				}
			}
			else
			{
				if (pThis->Ammo && pThis->Target) {
					pThis->SecondaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
				}

				if (!(nDistance >= 16)) {
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
					pThis->SetDestination(0, true);
				}
			}
		}
		else
		{
			pThis->MissionStatus = 1;
		}
		return  1;

	}
	case AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0:
	{
		if (!pThis->Target || !pThis->Ammo)
		{
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
			return 1;
		}

		if (!pThis->Is_Strafe())
		{
			const auto v23 = pThis->GetDirectionOverObject(pThis->Target);
			pThis->PrimaryFacing.Set_Desired(v23);
			pThis->SecondaryFacing.Set_Desired(v23);
		}

		const auto v26 = pThis->SelectWeapon(pThis->Target);
		const auto pWeapon = pThis->GetWeapon(v26)->WeaponType;
		const auto pRimaryWeapon = pThis->GetWeapon(0)->WeaponType;

		switch (pThis->GetFireError(pThis->Target, v26, true))
		{
		case FireError::OK:
		{
			pThis->loseammo_6c8 = 1;

			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt, v26, pWeapon);

			if(pThis->Target) { //Ares Untarget1
				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())->ScatterContent(pThis->Location, 1, 0, 0);
			}

			if (pThis->Is_Strafe())
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET2;
				pThis->__DoingOverfly = 1;
				return pRimaryWeapon->ROF;
			}

			if (!pThis->Is_Locked())
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET1;
				return 1;
			}

			pThis->__DoingOverfly = 1;
			pThis->MissionStatus = !(pThis->Ammo < 0) && !(pThis->Ammo == 0) ? (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION : (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
			return pRimaryWeapon->ROF;
		}
		case FireError::FACING:
		{
			if (!pThis->Ammo)
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
				return 1;
			}

			if (!pThis->IsCloseEnoughToAttack(pThis->Target) || pThis->Is_Strafe())
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION;
			}
			else if (pThis->Is_Locked())
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
			}
			else
			{
				pThis->MissionStatus = RulesClass::Instance->CurleyShuffle ? (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION : (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
			}

			return pThis->Is_Strafe() ? 45 : 1;
		}
		case FireError::REARM:
			return 1;
		case FireError::CLOAKED:
		{
			pThis->Uncloak(false);
			return 1;
		}
		default:
		{
			if (!pThis->Ammo)
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
				return 1;
			}

			if (pThis->Is_Strafe())
			{
				return 1;
			}

			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET1;
			return 1;
		}
		}
	}
	case AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET1:
	{
		if (pThis->Target)
		{
			const auto v41 = pThis->GetDirectionOverObject(pThis->Target);
			pThis->PrimaryFacing.Set_Desired(v41);
			pThis->SecondaryFacing.Set_Desired(v41);

			const auto v44 = pThis->SelectWeapon(pThis->Target);
			switch (pThis->GetFireError(pThis->Target, v44, 1))
			{
			case FireError::OK:
			{
				AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt, v44);
				if(pThis->Target) { //Ares Untarget 2
					MapClass::Instance->GetCellAt(pThis->Target->GetCoords())->ScatterContent(pThis->Location, 1, 0, 0);
				}
				if (!pThis->Ammo)
				{
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
				}

				pThis->MissionStatus = RulesClass::Instance->CurleyShuffle ? (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION : (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
				return int(pThis->GetCurrentMissionControl()->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
			}
			case FireError::FACING:
			{
				if (!pThis->Ammo)
				{
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
				}
				if (!pThis->IsCloseEnoughToAttack(pThis->Target) || pThis->Is_Strafe())
				{
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION;
				}
				else
				{
					pThis->MissionStatus = RulesClass::Instance->CurleyShuffle ? (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION : (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
				}
				if (!pThis->Is_Strafe())
				{
					return int(pThis->GetCurrentMissionControl()->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
				}

				return 45;
			}
			case FireError::REARM:
				return int(pThis->GetCurrentMissionControl()->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
			case FireError::CLOAKED:
			{
				pThis->Uncloak(false);
				return int(pThis->GetCurrentMissionControl()->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
			}
			default:
			{
				if (!pThis->Ammo)
				{
					pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
				}
				if (pThis->IsCloseEnoughToAttack(pThis->Target))
				{
					pThis->MissionStatus = RulesClass::Instance->CurleyShuffle ? (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION : (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET0;
				}
				else
				{
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION;
				}

				return int(pThis->GetCurrentMissionControl()->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
			}
			}
		}
		else
		{
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
			return 1;
		}
	}
	case AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET2:
	{
		if (pThis->Target)
		{
			const auto v52 = pThis->SelectWeapon(pThis->Target);
			const auto pWeapon = pThis->SelectWeapon(pThis->Target);

			switch (pThis->GetFireError(pThis->Target, v52, 1))
			{
			case FireError::OK:
			case FireError::FACING:
			case FireError::CLOAKED:
				break;
			case FireError::RANGE:
			{
				pThis->SetDestination(pThis->Target, 1);
				break;
			}
			default:
			{
				if (!pThis->Ammo)
				{
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
					pThis->__DoingOverfly = 0;
				}
				return  1;
			}
			}


			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe2, v52);
			if (pThis->Target) { //Ares Untarget 3
				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())->ScatterContent(pThis->Location, 1, 0, 0);
				pThis->SetDestination(pThis->Target, 1);
			}
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET3;

			return pThis->GetWeapon(0)->WeaponType->ROF;
		}
		else
		{
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
			return 1;
		}
	}
	case AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET3:
	{
		if (pThis->Target)
		{
			const auto v61 = pThis->SelectWeapon(pThis->Target);
			switch (pThis->GetFireError(pThis->Target, v61, 1))
			{
			case FireError::OK:
			case FireError::FACING:
			case FireError::CLOAKED:
				break;
			case FireError::RANGE:
			{
				pThis->SetDestination(pThis->Target, 1);
				break;
			}
			default:
			{
				if (!pThis->Ammo)
				{
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
					pThis->__DoingOverfly = 0;
				}
				return  1;
			}
			}

			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe3, v61);

			if(pThis->Target) { //Ares Untarget 4
				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())->ScatterContent(pThis->Location, 1, 0, 0);
				pThis->SetDestination(pThis->Target, 1);
			}

			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET4;
			return pThis->GetWeapon(0)->WeaponType->ROF;
		}
		else
		{
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
			return 1;
		}
	}
	case AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET4:
	{
		if (pThis->Target)
		{
			const auto v69 = pThis->SelectWeapon(pThis->Target);
			switch (pThis->GetFireError(pThis->Target, v69, 1))
			{
			case FireError::OK:
			case FireError::FACING:
			case FireError::CLOAKED:
				break;
			case FireError::RANGE:
			{
				pThis->SetDestination(pThis->Target, 1);
				break;
			}
			default:
			{
				if (!pThis->Ammo)
				{
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
					pThis->__DoingOverfly = 0;
				}
				return  1;
			}
			}

			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe4, v69);
			if (pThis->Target) { //Ares Untarget 5
				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())->ScatterContent(pThis->Location, 1, 0, 0);
				pThis->SetDestination(pThis->Target, 1);
			}
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET5;
			return pThis->GetWeapon(0)->WeaponType->ROF;
		}
		else
		{
			pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			return 1;
		}
	}
	case AirAttackStatusIDB::AIR_ATT_FIRE_AT_TARGET5:
	{
		if (pThis->Target)
		{
			const auto v77 = pThis->SelectWeapon(pThis->Target);
			switch (pThis->GetFireError(pThis->Target, v77, 1))
			{
			case FireError::OK:
			case FireError::FACING:
			case FireError::RANGE:
			case FireError::CLOAKED:
			{
				AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe5, v77);

				if (pThis->Target) { // Ares untarget 6
					MapClass::Instance->GetCellAt(pThis->Target->GetCoords())->ScatterContent(pThis->Location, 1, 0, 0);
				}

				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_FLY_TO_POSITION;
				return (pThis->GetWeapon(0)->WeaponType->Range + 0x400) / pThis->Type->Speed;
			}
			default:
			{
				if (!pThis->Ammo)
				{
					pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
					pThis->__DoingOverfly = 0;
				}
				return  1;
			}
			}
		}
		else
		{
			pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE;
			return 1;
		}
	}
	case AirAttackStatusIDB::AIR_ATT_RETURN_TO_BASE:
	{
		pThis->__DoingOverfly = 0;

		if (pThis->loseammo_6c8)
		{
			pThis->loseammo_6c8 = 0;

			if (pThis->Ammo > 0) {
				--pThis->Ammo;
			}
		}

		if (pThis->Ammo)
		{
			if (pThis->Target)
			{
				pThis->MissionStatus = (int)AirAttackStatusIDB::AIR_ATT_PICK_ATTACK_LOCATION;
				return 1;
			}
		}
		else if (pThis->IsALoaner || (pThis->Owner && pThis->Owner->IsControlledByCurrentPlayer()))
		{
			pThis->SetTarget(nullptr);
		}

		auto v86 = pThis->Owner;
		pThis->__DoingOverfly = 0;
		auto v87 = v86 ? v86->GetCurrentEdge() : Edge::North;
		auto v88 = MapClass::Instance->PickCellOnEdge(
				  v87,
				  CellStruct::Empty,
				  CellStruct::Empty,
				  SpeedType::Winged,
				  1,
				  MovementZone::None);

		pThis->SetDestination(MapClass::Instance->GetCellAt(v88), true);
		pThis->retreating_idle = 0;
		if (pThis->Airstrike && pThis->Ammo > 0)
		{
			pThis->QueueMission(Mission::Retreat, 0);
			pThis->retreating_idle = 1;
			return 1;
		}
		else
		{
			pThis->EnterIdleMode(0, 1);
			pThis->retreating_idle = 1;
			return  1;
		}
	}
	default:
		return int(pThis->GetCurrentMissionControl()->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
	}
}

DEFINE_HOOK(0x417FE0, Aircraft_MI_Attack, 0x6)
{
	GET(AircraftClass*, pThis, ECX);
	R->EAX(Mission_Attack(pThis));
	return 0x418D54;
}

#ifdef SecondMode
//Strafe4
DEFINE_HOOK_AGAIN(0x4189BB, AircraftClass_Mi_Attack_Strafe, 0x6)
//Strafe3
DEFINE_HOOK_AGAIN(0x4188AC, AircraftClass_Mi_Attack_Strafe, 0x6)
//strafe2
DEFINE_HOOK_AGAIN(0x41879D, AircraftClass_Mi_Attack_Strafe, 0x6)
//strafe5

DEFINE_HOOK(0x418ACA, AircraftClass_Mi_Attack_Strafe, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	R->EAX(MissionAttack_Strafe(pThis, pExt->Aircraft_DecreaseAmmo.Get()));
	return 0x4187B2;
}

DEFINE_HOOK_AGAIN(0x4180BA, AircraftClass_Mi_Attack_SkipDecreasingAmmo, 0x6)
DEFINE_HOOK(0x418050, AircraftClass_Mi_Attack_SkipDecreasingAmmo, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	return !pExt->Aircraft_DecreaseAmmo.Get() ? 0x418056 :
		0x0;
}

DEFINE_HOOK(0x4182BF, AircraftClass_Mi_Attack_StrafeTurn, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	auto const pFly = static_cast<IFlyControl*>(pThis);
	if (!pExt->Aircraft_DecreaseAmmo.Get() ||
		!pFly->Is_Strafe())
	{
		const auto nDir = pThis->GetDirectionOverObject(pThis->Target);
		pThis->PrimaryFacing.Set_Desired(nDir);
		pThis->SecondaryFacing.Set_Desired(nDir);
	}

	R->EBX(pFly);
	return 0x418311;
}

DEFINE_HOOK(0x41839D, AircraftClass_Mi_Attack_StrafeFireError, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	return !pExt->Aircraft_DecreaseAmmo.Get() ||
		!pThis->Is_Strafe() ? 0x4183A7 : 0x4183DB;
}
#endif