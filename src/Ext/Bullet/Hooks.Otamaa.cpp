#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>

#include <TechnoClass.h>
#include <TacticalClass.h>

#include <Fundamentals.h>

ASMJIT_PATCH(0x46B1D6, BulletClass_DrawVXL_Palette, 0x6)
{
	GET_STACK(BulletClass*, pThis, STACK_OFFS(0xF8, 0xE4));
	GET(BulletTypeClass*, pThisType, EDX);
	GET(Point2D*, pPoint, ECX);
	GET(int, nRect_X, EBP);
	GET(int, nRect_Y, ESI);

	R->Stack(STACK_OFFS(0xF8, 0xE4), Point2D { pPoint->X + nRect_X , pPoint->Y + nRect_Y });
	R->EAX(ColorScheme::Array()->Items);
	R->ECX((pThis->InheritedColor != -1) ? pThis->InheritedColor : pThisType->Color);

	return 0x46B1F2;
}

#ifdef ENABLE_CELLSPREAD_LOCOWH
static void  ManipulateLoco(FootClass* pFirer, AbstractClass* pTarget, BulletClass* pBullet, bool Area)
{
	AbstractClass* pTarget_1 = pTarget;

	if (!Area)
	{
		if (pFirer->LocomotorTarget)
		{
			pFirer->LocomotorImblued(false);
		}

		pTarget_1 = pBullet->Target;
	}

	if (auto pFoot_T = generic_cast<FootClass*>(pTarget_1))
	{
		if (pTarget_1->WhatAmI() == AbstractType::Aircraft || pTarget_1->WhatAmI() == AbstractType::Unit)
		{
			if (auto pExt = WarheadTypeExtContainer::Instance.Find(pBullet->WH))
			{
				if (!pExt->CanTargetHouse(pBullet->GetOwningHouse(), pFoot_T))
					return;

				if (!pExt->CanDealDamage(pFoot_T))
					return;
			}

			if (auto pUnit = specific_cast<UnitClass*>(pTarget))
			{

				if (pFoot_T->RadioLinks.IsAllocated &&
					pFoot_T->RadioLinks.IsInitialized &&
					pFoot_T->RadioLinks.Items)
				{
					auto pLink = *pFoot_T->RadioLinks.Items;
					if (pLink && pLink->WhatAmI() == AbstractType::Building && ((BuildingClass*)pLink)->Type->WeaponsFactory)
					{
						if (Map[pFoot_T->Location]->GetBuilding() == pLink)
							return;
					}
				}

				if (pUnit->Deploying || pUnit->Undeploying)
				{
					return;
				}
			}

			if (pFoot_T->InWhichLayer() != Layer::Ground || pFoot_T->IsAttackedByLocomotor)
				return;

			pFirer->FootClass_ImbueLocomotor(pFoot_T, pBullet->WH->Locomotor.get());
		}
	}
}

ASMJIT_PATCH(0x4694CB, BulletClass_Logics_Locomotor, 0x6)
{
	enum
	{
		IsNotLocoRet = 0x469705
		, Return = 0x469AA4
	};

	GET(BulletClass*, pThis, ESI);
	GET(WarheadTypeClass*, pWH, EAX);

	if (!pWH->IsLocomotor)
		return IsNotLocoRet;

	if (!pThis->Owner)
		return Return;

	auto pTarget = pThis->Target;
	if (pThis->Owner->LocomotorTarget == pTarget
	  || pThis->Owner->LocomotorSource
	  || (pThis->Owner->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None && ((FootClass*)pThis->Owner)->unknown_bool_6AC)
	{
		return Return;
	}

	if (pWH->CellSpread == 0.0)
		ManipulateLoco((FootClass*)pThis->Owner, pTarget, pThis, false);
	else
	{
		for (auto pTarget_s : Helpers::Alex::getCellSpreadItems(pTarget->GetCoords(), pWH->CellSpread, true))
		{ ManipulateLoco((FootClass*)pThis->Owner, pTarget_s, pThis, true); }
	}

	return Return;
}

ASMJIT_PATCH(0x707B95, TechnoClass_PointerExpired_LocoSource, 0x7)
{
	GET(TechnoClass*, pAttacker, ECX);
	GET(TechnoClass*, pVictim, ESI);

	if (pAttacker->LocomotorTarget == pVictim)
	{
		pAttacker->LocomotorImblued(true);
		pAttacker->LocomotorTarget = nullptr;
		pVictim->LocomotorSource = nullptr;
		return 0x707BB2;
	}

	if (!pVictim || (pVictim->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
	{
		pVictim->LocomotorSource = nullptr;
		return 0x707BB2;
	}

	if (pVictim->GetHeight() > 0)
	{
		pVictim->IsCrashing = 1;
		pVictim->IsBeingManipulated = 1;
		pVictim->BeingManipulatedBy = pVictim;
		pVictim->ChronoWarpedByHouse = pAttacker->Owner;
		pVictim->Stun();
		pVictim->SetDestination(nullptr, true);
	}

	if (auto pUnitV = specific_cast<UnitClass*>(pVictim))
		pUnitV->unknown_int_6D4 = 1;

	pVictim->SetDestination(nullptr, true);
	pVictim->LocomotorSource = nullptr;
	return 0x707BB2;
}

ASMJIT_PATCH(0x7102F9, FootClass_ImbueLocomotor_SetDestination, 0x5)
{
	GET(TechnoClass*, pThis, EBX);
	GET(FootClass*, pThat, ESI);

	pThis->LocomotorTarget = pThat;
	pThat->LocomotorSource = generic_cast<FootClass*>(pThis);

	if (!pThis->GetTechnoType()->Insignificant
	  || pThat->WhatAmI() != AbstractType::Aircraft)
	{
		pThat->SetDestination(pThis, true);
		pThat->Deselect();
	}
	else
	{
		pThis->Scatter(CoordStruct::Empty, true, false);
		pThis->Deselect();
	}

	return 0x71033A;
}

ASMJIT_PATCH(0x4666F7, BulletClass_AI_Trajectories, 0x6)
{
	enum { Detonate = 0x467E53 };

	GET(FakeBulletClass*, pThis, EBP);

	auto pExt = pThis->_GetExtData();
	auto pTypeExt  = pThis->_GetTypeExtData();

	auto& pTraj = pExt->Trajectory;

	if (!pThis->SpawnNextAnim && pTraj) {
		return pTraj->OnAI() ? Detonate : 0x0;
	}

	if (pExt->InterceptedStatus & InterceptedStatus::Targeted) {
		if (const auto pTarget = cast_to<BulletClass*>(pThis->Target)) {
			const auto pTargetTypeExt = BulletTypeExtContainer::Instance.Find(pTarget->Type);
			const auto pTargetExt = BulletExtContainer::Instance.Find(pTarget);

			if (!pTargetTypeExt->Armor.isset())
				pTargetExt->InterceptedStatus |= InterceptedStatus::Locked;
		}
	}

	if (pExt->InterceptedStatus & InterceptedStatus::Intercepted)
	{
		if (const auto pTarget = cast_to<BulletClass*>(pThis->Target))
			BulletExtContainer::Instance.Find(pTarget)->InterceptedStatus &= ~InterceptedStatus::Locked;

		if (BulletExtData::HandleBulletRemove(pThis, pExt->DetonateOnInterception, true))
			return 0x467FEE;
	}

	if (!pThis->IsAlive) {
        return 0x467FEE;
    }

	if (!PhobosTrajectory::BlockDrawTrail(pTraj)) {

		if(!pExt->LaserTrails.empty()) {
			CoordStruct futureCoords
			{
				pThis->Location.X + static_cast<int>(pThis->Velocity.X),
				pThis->Location.Y + static_cast<int>(pThis->Velocity.Y),
				pThis->Location.Z + static_cast<int>(pThis->Velocity.Z)
			};

			for (auto& trail : pExt->LaserTrails)
			{
				if (!trail->LastLocation.isset())
					trail->LastLocation = pThis->Location;

				trail->Update(futureCoords);
			}
		}

		/*TrailsManager::AI(pThis->_AsBullet());*/
	}

	if (pThis->HasParachute)
	{
		int fallRate = pExt->ParabombFallRate - pTypeExt->Parachuted_FallRate;
		int maxFallRate = pTypeExt->Parachuted_MaxFallRate.Get(RulesClass::Instance->ParachuteMaxFallRate);

		if (fallRate < maxFallRate)
			fallRate = maxFallRate;

		pExt->ParabombFallRate = fallRate;
		pThis->FallRate = fallRate;
	}

	return 0;
}

ASMJIT_PATCH(0x467AB2, BulletClass_AI_Parabomb, 0x7)
{
	GET(BulletClass*, pThis, EBP);

	if (pThis->HasParachute)
		return 0x467B1A;

	return 0;
}

ASMJIT_PATCH(0x467E53, BulletClass_AI_PreDetonation_Trajectories, 0x6)
{
	GET(FakeBulletClass*, pThis, EBP);

	if (auto& pTraj = pThis->_GetExtData()->Trajectory)
		pTraj->OnAIPreDetonate();

	return 0;
}

ASMJIT_PATCH(0x46745C, BulletClass_AI_Position_Trajectories, 0x7)
{
	GET(FakeBulletClass*, pThis, EBP);
	LEA_STACK(VelocityClass*, pSpeed, STACK_OFFS(0x1AC, 0x11C));
	LEA_STACK(VelocityClass*, pPosition, STACK_OFFS(0x1AC, 0x144));

	auto pExt =  pThis->_GetExtData();

	if (auto& pTraj = pExt->Trajectory)
		pTraj->OnAIVelocity(pSpeed, pPosition);


	// Trajectory can use Velocity only for turning Image's direction
	// The true position in the next frame will be calculate after here
	if (pExt->Trajectory) {

		if(!pExt->LaserTrails.empty()){
			CoordStruct futureCoords
			{
				static_cast<int>(pSpeed->X + pPosition->X),
				static_cast<int>(pSpeed->Y + pPosition->Y),
				static_cast<int>(pSpeed->Z + pPosition->Z)
			};
			for (auto& trail : pExt->LaserTrails)
			{
				if (!trail->LastLocation.isset())
					trail->LastLocation = pThis->Location;
				trail->Update(futureCoords);
			}
		}

		/*TrailsManager::AI(pThis->_AsBullet());*/
	}

	return 0;
}

ASMJIT_PATCH(0x4677D3, BulletClass_AI_TargetCoordCheck_Trajectories, 0x5)
{
	GET(BulletClass*, pThis, EBP);
	REF_STACK(CoordStruct, coords, STACK_OFFS(0x1A8, 0x184));

	return PhobosTrajectory::OnAITargetCoordCheck(pThis, coords);
}

ASMJIT_PATCH(0x467927, BulletClass_AI_TechnoCheck_Trajectories, 0x5)
{

	GET(BulletClass*, pThis, EBP);
	GET(TechnoClass*, pTechno, ESI);

	return PhobosTrajectory::OnAITechnoCheck(pThis, pTechno);
}

#endif
