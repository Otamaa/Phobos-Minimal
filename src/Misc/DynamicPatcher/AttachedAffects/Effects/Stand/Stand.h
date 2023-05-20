#pragma once

#include "../Base.h"
#include "StandType.h"
#include "StandHelpers.h"
#include "../../LocationMark.h"

class Stand : public Effect<StandType>
{
public:

	Stand() : Effect<StandType> { MyType::Stand }
		, pStand { nullptr }
		, isBuilding { false }
		, onStopCommand { false }
		, notBeHuman { false }
	{ }

	Stand(const StandType& nType) : Effect<StandType> { MyType::Stand }
		, pStand { nullptr }
		, isBuilding { false }
		, onStopCommand { false }
		, notBeHuman { false }
	{ SetTypeData(nType); }

	 bool IsAlive() override {
		if (!pStand
			//|| Helpers_DP::IsDead(pStand)
			)
		{
			pStand = nullptr;
			return false;
		}
		return true;
	}

	 void OnEnable(ObjectClass* pObject, HouseClass* pHouse, TechnoClass* pAttacker) override
	 {
		 CreateAndPutStand(pObject, pHouse);
	 }

	 void Disable(CoordStruct location) override
	 {
		 if (!pStand)
			 return;

		 ExplodesOrDisappear(false);
	 }

	 void OnUpdate(ObjectClass* pObject, bool isDead) override
	 {
		 switch (pObject->WhatAmI())
		 {
		 case AbstractType::Bullet:
			 UpdateState((BulletClass*)pObject);
			 break;
		 case AbstractType::Infantry:
		 case AbstractType::Unit:
		 case AbstractType::Aircraft:
		 case AbstractType::Building:
			 UpdateState((TechnoClass* )pObject);
			 break;
		 default:
			 return;
		 }
	 }

	 void UpdateState(BulletClass* pBullet)
	 {
		 if (!TypeData)
			 return;

		 // Synch Target
		 RemoveStandIllegalTarget();
		 auto target = pBullet->Target;

		 if (TypeData->SameTarget && target)
		 {
			 pStand->SetTarget(target);
		 }
		 if (TypeData->SameLoseTarget && !target)
		 {
			 pStand->SetTarget(target);
			 if (!target && pStand->SpawnManager)
			 {
				 pStand->SpawnManager->NewTarget = target;
				 pStand->SetTarget(target);
			 }
		 }
	 }

	void UpdateState(TechnoClass* pMaster);

	void UpdateLocation(LocationMark mark)
	{
		SetLocation(mark.Location);
		SetDirection(mark.Direction, false);
	}

	void OnPut(ObjectClass* pOwner, CoordStruct* pCoord, DirStruct faceDir) override
	{
		if (!pStand)
			return;

		if (pStand->InLimbo)
		{
			CoordStruct location = *pCoord;
			if (!TryPutStand(location))
			{
				Disable(location);
			}
		}
	}

	void OnRemove(ObjectClass* pOwner) override
	{
		pStand->Limbo();
	}

	void OnDestroy(ObjectClass* pObject) override
	{
		notBeHuman = TypeData->ExplodesWithMaster;
		pStand->QueueMission(Mission::Sleep, true);
	}

	void OnStopCommand() override;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:

	void CreateAndPutStand(ObjectClass* pObject, HouseClass* pHouse);

	bool TryPutStand(CoordStruct location)
	{
		if (!pStand)
			return false;

		if (auto pCell = MapClass::Instance->TryGetCellAt(location))
		{
			auto occFlags = pCell->OccupationFlags;
			pStand->OnBridge = pCell->ContainsBridge();
			CoordStruct xyz = pCell->GetCoordsWithBridge();
			xyz.Z = MapClass::Instance->GetCellFloorHeight(xyz);
			++Unsorted::IKnowWhatImDoing;
			auto bSucceed = pStand->Unlimbo(xyz, Direction::E);
			--Unsorted::IKnowWhatImDoing;

			if(bSucceed)
				pCell->OccupationFlags = occFlags;

			return bSucceed;
		}
		return false;
	}

	void ExplodesOrDisappear(bool remove)
	{
		if (!TypeData || !pStand)
			return;

		if (TypeData->Explodes.Get() || notBeHuman)
		{
			if (remove) {
				pStand->Limbo();
				//GameDelete(pStand);
			} else {
				auto nDamage = pStand->Health;
				pStand->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, nullptr, true, pStand->GetTechnoType()->Crewed, nullptr);

			}
		}
		else
		{
			pStand->UnInit();
			//GameDelete(pStand);
		}

		pStand = nullptr;
	}

	void RemoveStandIllegalTarget()
	{
		if (AbstractClass* pStandTarget = pStand->Target)
		{
			int i = pStand->SelectWeapon(pStandTarget);
			FireError fireError = pStand->GetFireError(pStandTarget, i, true);
			switch (fireError)
			{
			case FireError::ILLEGAL:
			case FireError::CANT:
			case FireError::MOVING:
			case FireError::RANGE:
				pStand->SetTarget(nullptr);
				break;
			}
		}
	}

	void RemoveStandTarget()
	{
		pStand->Target = nullptr;
		pStand->SetTarget(nullptr);
		pStand->QueueMission(Mission::Area_Guard, true);
		if (pStand->SpawnManager)
		{
			pStand->SpawnManager->NewTarget = nullptr;
			pStand->SpawnManager->Target = nullptr;
			pStand->SpawnManager->SetTarget(nullptr);
		}
	}

	void SetLocation(CoordStruct location)
	{
		pStand->SetLocation(location);
		pStand->SetFocus(nullptr);
	}

	void SetDirection(DirStruct direction, bool forceSetTurret)
	{
		if (pStand->HasTurret() || TypeData->LockDirection)
		{
			pStand->PrimaryFacing.set(direction);
		}

		if ((!pStand->Target || TypeData->LockDirection)
			&& !pStand->GetTechnoType()->TurretSpins)
		{
			if (forceSetTurret)
			{
				ForceSetFacing(direction);
			}
			else
			{
				TurnTurretFacing(direction);
			}
		}
	}

	void TurnTurretFacing(DirStruct targetDir)
	{
		if (pStand->HasTurret())
		{
			pStand->SecondaryFacing.turn(targetDir);
		}
		else
		{
			pStand->PrimaryFacing.turn(targetDir);
		}
	}

	void ForceSetFacing(DirStruct targetDir)
	{
		pStand->PrimaryFacing.set(targetDir);
		pStand->SecondaryFacing.set(targetDir);
	}

public:
	TechnoClass* pStand;
private:
	bool isBuilding = false;
	bool onStopCommand = false;
	bool notBeHuman = false;

	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::Log("Processing Element From Animation ! \n");
		return Stm
			.Process(pStand)
			.Process(isBuilding)
			.Process(onStopCommand)
			.Process(notBeHuman)
			.Success()
			;
	}

};