#include "Stand.h"
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Body.h>
#include <LocomotionClass.h>
#include <DriveLocomotionClass.h>

void Stand::CreateAndPutStand(ObjectClass* pObject, HouseClass* pHouse)
{
	if (!TypeData)
		return;

	CoordStruct location = pObject->GetCoords();

	if (auto Type = TechnoTypeClass::Find(TypeData->Type.data()))
	{
		pStand = (TechnoClass*)Type->CreateObject(pHouse);
		if (pStand)
		{
			if (auto ext = TechnoExtContainer::Instance.Find(pStand))
			{
				if (TypeData->VirtualUnit.Get()) {
					ext->VirtualUnit = TypeData->VirtualUnit;
				}

				switch (pObject->WhatAmI())
				{
				case AbstractType::Bullet:
				{
					auto pBullet = (BulletClass*)pObject;
					ext->MyMaster = pBullet->Owner;
				}	break;
				default:
				{
					auto pTechno = (TechnoClass*)pObject;
					ext->MyMaster = pTechno;
					//if (pTechno->Owner && pHouse == pTechno->Owner)
					//{
					 // auto masterExt = TechnoExtContainer::Instance.Find((TechnoClass*)pObject);
					  //ext->AnotherData.MyManager->PaintballState = masterExt.AttachEffectManager.PaintballState;
					//}
				}	break;
				}
			}

			// 初始化替身
			pStand->UpdatePlacement(PlacementType::Remove);
			pStand->IsOnMap = false;
			pStand->NeedsRedraw = true;
			bool canGuard = pHouse->IsControlledByHuman();
			if (pStand->WhatAmI() == AbstractType::Building)
			{
				isBuilding = true;
				canGuard = true;
			}
			else
			{
				auto const pLoco = ((FootClass*)pStand)->Locomotor.get();
				pLoco->Lock();
			}

			// only computer units can hunt
			Mission mission = canGuard ? Mission::Guard : Mission::Hunt;
			pStand->QueueMission(mission, false);

			// 在格子位置刷出替身单位
			if (!TryPutStand(location))
			{
				// 刷不出来？
				Disable(location);
				return;
			}

			// 放置到指定位置
			LocationMark locationMark = StandHelper::GetLocation(pObject, TypeData);
			if (locationMark.Location)
			{
				SetLocation(locationMark.Location);
				ForceSetFacing(locationMark.Direction);
			}

			// Logger.Log("{0} - 创建替身[{1}]{2}", Game.CurrentFrame, Type.Type, pStand.Pointer);
		}
	}
}

void Stand::UpdateState(TechnoClass* pMaster)
{
	if (!TypeData)
		return;

	if (pMaster->IsSinking && TypeData->RemoveAtSinking)
	{
		ExplodesOrDisappear(true);
		return;
	}

	pStand->UpdatePlacement(PlacementType::Remove);
	pStand->IsOnMap = false;
	pStand->NeedsRedraw = true;
	if (pStand->WhatAmI() != AbstractType::Building)
	{
		((FootClass*)pStand)->Locomotor.get()->Lock();
	}

	if (TypeData->SameHouse.Get())
	{
		// sync Owner
		pStand->Owner = pMaster->Owner;
	}

	// sync Tilt
	if (!TypeData->IsTrain.Get())
	{
		pStand->AngleRotatedForwards = pMaster->AngleRotatedForwards;
		pStand->AngleRotatedSideways = pMaster->AngleRotatedSideways;
	}

	// sync State
	pStand->IsSinking = pMaster->IsSinking;
	pStand->__shipsink_3CA = pMaster->__shipsink_3CA;
	pStand->InLimbo = pMaster->InLimbo;
	pStand->OnBridge = pMaster->OnBridge;
	pStand->CloakState = pMaster->CloakState;
	pStand->BeingWarpedOut = pMaster->BeingWarpedOut;
	pStand->Deactivated = pMaster->Deactivated; // 遥控坦克
	pStand->IronCurtainTimer = pMaster->IronCurtainTimer;
	pStand->IdleActionTimer = pMaster->IdleActionTimer;
	pStand->IronTintTimer = pMaster->IronTintTimer;
	pStand->CloakDelayTimer = pMaster->CloakDelayTimer;

	pStand->Berzerk = pMaster->Berzerk;
	pStand->EMPLockRemaining = pMaster->EMPLockRemaining;
	pStand->ShouldLoseTargetNow = pMaster->ShouldLoseTargetNow;

	// sync Promote
	if (TypeData->PromoteFormMaster && pStand->GetTechnoType()->Trainable)
	{
		pStand->Veterancy = pMaster->Veterancy;
	}

	// sync PrimaryFactory
	pStand->IsPrimaryFactory = pMaster->IsPrimaryFactory;

	// get mission
	Mission masterMission = pMaster->CurrentMission;

	// check power off and moving
	bool masterIsBuilding = (pMaster->WhatAmI() == AbstractType::Building);
	bool masterPowerOff = pMaster->Owner->IsNoPower();
	bool masterIsMoving = masterMission == Mission::Move || masterMission == Mission::AttackMove;

	if (masterIsBuilding) {
		if (pMaster->Owner == pStand->Owner)
		{
			auto pBuilding = (BuildingClass*)pMaster;
			if (!masterPowerOff) {
				masterPowerOff = !pBuilding->HasPower;
			}
		}
	} else if (!masterIsMoving) {
		auto pFoot = (FootClass*)pMaster;
		masterIsMoving = pFoot->Locomotor.get()->Is_Moving() && pFoot->GetCurrentSpeed() > 0;
	}

	ILocomotion* masterLoco = nullptr;
	ILocomotion* standLoco = nullptr;
	if (!masterIsBuilding)
	{
		masterLoco = ((FootClass*)pMaster)->Locomotor.get();
	}
	if (!isBuilding)
	{
		standLoco = ((FootClass*)pStand)->Locomotor.get();
	}

	// sync Moving anim
	if (TypeData->IsTrain && !isBuilding)
	{
		// switch (pStand->Base.Base.WhatAmI())
		// {
		//     case AbstractType.Infantry:
		//         Pointer<InfantryClass> pInf = pStand.Pointer.Convert<InfantryClass>();
		//         // pInf.Convert<FootClass>()->Inf_PlayAnim(SequenceAnimType.FIRE_WEAPON);

		//         pInf->SequenceAnim = SequenceAnimType.FIRE_WEAPON;
		//         break;
		//     case AbstractType.Unit:

		//         break;
		// }
		// CoordStruct sourcePos = pStand->Base.Base.GetCoords();
		// ILocomotion loco = pStand.Pointer.Convert<FootClass>()->Locomotor;
		// Guid locoId = loco.ToLocomotionClass()->GetClassID();
		// if (LocomotionClass.Walk == locoId)
		// {
		//     Pointer<WalkLocomotionClass> pLoco = loco.ToLocomotionClass<WalkLocomotionClass>();
		//     if (masterIsMoving)
		//     {
		//         pLoco->Destination = ExHelper.GetFLHAbsoluteCoords(pStand.Pointer, new CoordStruct(1024, 0, 0));
		//         pLoco->IsMoving = false;
		//     }
		//     else
		//     {
		//         pLoco->Destination = default;
		//         pLoco->IsMoving = false;
		//     }
		// }
		// else if (LocomotionClass.Mech == locoId)
		// {
		//     Pointer<MechLocomotionClass> pLoco = loco.ToLocomotionClass<MechLocomotionClass>();
		//     if (masterIsMoving)
		//     {
		//         pLoco->Destination = ExHelper.GetFLHAbsoluteCoords(pStand.Pointer, new CoordStruct(1024, 0, 0));
		//         pLoco->IsMoving = true;
		//     }
		//     else
		//     {
		//         pLoco->Destination = default;
		//         pLoco->IsMoving = false;
		//     }

		// }
	}
	else if (TypeData->SameTilter && masterLoco && standLoco)
	{
		CLSID masterLocoId;
		((LocomotionClass*)masterLoco)->GetClassID(&masterLocoId);
		CLSID standLocoId;
		((LocomotionClass*)standLoco)->GetClassID(&standLocoId);

		if (LocomotionClass::CLSIDs::Drive == masterLocoId &&
			LocomotionClass::CLSIDs::Drive == standLocoId)
		{
			auto pMasterLoco = (DriveLocomotionClass*)masterLoco;
			auto pStandLoco = (DriveLocomotionClass*)standLoco;
			pStandLoco->Ramp1 = pMasterLoco->Ramp1;
			pStandLoco->Ramp2 = pMasterLoco->Ramp2;
		}
	}

	// check fire
	bool powerOff = TypeData->Powered && masterPowerOff;
	bool canFire = !powerOff && (TypeData->MobileFire || !masterIsMoving);
	if (canFire)
	{
		// sync mission
		switch (masterMission)
		{
		case Mission::Guard:
		case Mission::Area_Guard:
			Mission standMission = pStand->CurrentMission;
			if (standMission != Mission::Attack)
			{
				pStand->QueueMission(masterMission, true);
			}
			break;
		}
	}
	else
	{
		RemoveStandTarget();
		onStopCommand = false;
		pStand->QueueMission(Mission::Sleep, true);
	}

	// sync target
	if (TypeData->ForceAttackMaster)
	{
		if (!powerOff)
		{
			pStand->SetTarget(pMaster);
		}
	}
	else
	{
		if (!onStopCommand)
		{
			// sync Target
			RemoveStandIllegalTarget();
			auto target = pMaster->Target;
			if (target)
			{
				if (TypeData->SameTarget && canFire)
				{
					pStand->SetTarget(target);
				}
			}
			else
			{
				if (TypeData->SameLoseTarget || !canFire)
				{
					RemoveStandTarget();
				}
			}
		}
		else
		{
			onStopCommand = false;
		}
	}
}

void Stand::OnStopCommand()
{
	RemoveStandTarget();
	onStopCommand = true;
	auto ext = TechnoExtContainer::Instance.Find(pStand);
	if (auto pManager = ext->AnotherData.MyManager.get())
		pManager->StopCommand();
}