#include "Body.h"

#include "Trajectories/PhobosTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

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
