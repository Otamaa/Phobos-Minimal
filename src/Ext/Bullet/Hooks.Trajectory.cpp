#include "Trajectories/PhobosTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

//#ifdef ENABLE_TRAJ_HOOKS
ASMJIT_PATCH(0x4666F7, BulletClass_AI_Trajectories, 0x6)
{
	enum { Detonate = 0x467E53 };

	GET(FakeBulletClass*, pThis, EBP);

	auto pExt = pThis->_GetExtData();
	auto& pTraj = pExt->Trajectory;

	if (!pThis->SpawnNextAnim && pTraj) {
		return pTraj->OnAI() ? Detonate : 0x0;
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

		TrailsManager::AI(pThis->_AsBullet());
	}

	return 0;
}

//ASMJIT_PATCH_AGAIN(0x467FEE , BulletClass_AI_Late_Trajectories , 0x6)
//ASMJIT_PATCH(0x466781 , BulletClass_AI_Late_Trajectories , 0x6)
//{
//	GET(BulletClass* , pThis , EBP);
//	return 0x0;
//}

ASMJIT_PATCH(0x467E53, BulletClass_AI_PreDetonation_Trajectories, 0x6)
{
	GET(FakeBulletClass*, pThis, EBP);

	if (auto& pTraj = pThis->_GetExtData()->Trajectory)
		pTraj->OnAIPreDetonate();

	return 0;
}

//TODO :
//ASMJIT_PATCH(0x4677A8, BulletClass_AI_ChangeVelocity_Locked , 9)
//{
//	//GET(BulletClass*, pThis, EBP);
//	GET_STACK(VelocityClass, nVel, 0x1AC - 0x140);
//	REF_STACK(CoordStruct, nResultCoord, 0x1AC - 0x184);
//
//	nResultCoord.X = (int)nVel.X;
//	nResultCoord.Y= (int)nVel.Y;
//	nResultCoord.Z= (int)nVel.Z;
//    return 0x4677D3;
//}

//TODO :
//ASMJIT_PATCH(0x466E05, BulletClass_CheckHight_UnderGround  , 8)
//{
//	GET(BulletClass*, pThis, EBP);
//    R->Stack(0x18, true);
//    R->Stack(0x20, true);
//
//	R->EAX(pThis->GetHeight());
//    return 0x466E1E;
//}

//TODO : stack modifier shenanegans
//ASMJIT_PATCH(0x4683E7, BulletClass_DrawSHP_Bright ,  9)
//{
//	GET(BulletClass*, pThis, ESI);
//    //R->Stack(0, status.PaintballState.Data.GetBright(1000));
//    return 0;
//}

//TODO : stack modifier shenanegans
//ASMJIT_PATCH(0x46B201, BulletClass_DrawVXL_Color , 7)
//{
//	GET_STACK(BulletClass*, pThis, 0x10 - 0x4);
//	//R->EDI(BlitterFlags.None);
//    //R->Stack(0, ColorStruct.Red.ToColorAdd().Add2RGB565());
// 	//R->Stack(0x118, PaintballState.Data.GetBright(bright));
//    return 0;
//}

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

		TrailsManager::AI(pThis->_AsBullet());
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

ASMJIT_PATCH(0x468B72, BulletClass_Unlimbo_Trajectories, 0x5)
{
	GET(FakeBulletClass*, pThis, EBX);
	GET_STACK(CoordStruct*, pCoord, STACK_OFFS(0x54, -0x4));
	GET_STACK(VelocityClass*, pOriginalVelocity, STACK_OFFS(0x54, -0x8));

	PhobosTrajectory::CreateInstance(pThis, pCoord, pOriginalVelocity);
	//if (WarheadTypeExtContainer::Instance.Find(pThis->WH)->DirectionalArmor) {
	//	BulletExtContainer::Instance.Find(pThis)->InitialBulletDir = DirStruct(Math::atan2(static_cast<double>(pThis->SourceCoords.Y - pThis->TargetCoords.Y), static_cast<double>(pThis->TargetCoords.X - pThis->SourceCoords.X)));
	//}

	return 0;
}

//#endif