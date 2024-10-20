#include "Trajectories/PhobosTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

//#ifdef ENABLE_TRAJ_HOOKS
DEFINE_HOOK(0x4666F7, BulletClass_AI_Trajectories, 0x6)
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

	if (pTraj && pExt->LaserTrails.size()) {
		CoordStruct futureCoords
		{
			pThis->Location.X + static_cast<int>(pThis->Velocity.X),
			pThis->Location.Y + static_cast<int>(pThis->Velocity.Y),
			pThis->Location.Z + static_cast<int>(pThis->Velocity.Z)
		};

		for (auto& trail : pExt->LaserTrails)
		{
			if (!trail.LastLocation.isset())
				trail.LastLocation = pThis->Location;

			trail.Update(futureCoords);
		}
	}

	return 0;
}

//DEFINE_HOOK_AGAIN(0x467FEE , BulletClass_AI_Late_Trajectories , 0x6)
//DEFINE_HOOK(0x466781 , BulletClass_AI_Late_Trajectories , 0x6)
//{
//	GET(BulletClass* , pThis , EBP);
//	return 0x0;
//}

DEFINE_HOOK(0x467E53, BulletClass_AI_PreDetonation_Trajectories, 0x6)
{
	GET(FakeBulletClass*, pThis, EBP);

	if (auto& pTraj = pThis->_GetExtData()->Trajectory)
		pTraj->OnAIPreDetonate();

	return 0;
}

//TODO :
//DEFINE_HOOK(0x4677A8, BulletClass_AI_ChangeVelocity_Locked , 9)
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
//DEFINE_HOOK(0x466E05, BulletClass_CheckHight_UnderGround  , 8)
//{
//	GET(BulletClass*, pThis, EBP);
//    R->Stack(0x18, true);
//    R->Stack(0x20, true);
//
//	R->EAX(pThis->GetHeight());
//    return 0x466E1E;
//}

//TODO : stack modifier shenanegans
//DEFINE_HOOK(0x4683E7, BulletClass_DrawSHP_Bright ,  9)
//{
//	GET(BulletClass*, pThis, ESI);
//    //R->Stack(0, status.PaintballState.Data.GetBright(1000));
//    return 0;
//}

//TODO : stack modifier shenanegans
//DEFINE_HOOK(0x46B201, BulletClass_DrawVXL_Color , 7)
//{
//	GET_STACK(BulletClass*, pThis, 0x10 - 0x4);
//	//R->EDI(BlitterFlags.None);
//    //R->Stack(0, ColorStruct.Red.ToColorAdd().Add2RGB565());
// 	//R->Stack(0x118, PaintballState.Data.GetBright(bright));
//    return 0;
//}

DEFINE_HOOK(0x46745C, BulletClass_AI_Position_Trajectories, 0x7)
{
	GET(FakeBulletClass*, pThis, EBP);
	LEA_STACK(VelocityClass*, pSpeed, STACK_OFFS(0x1AC, 0x11C));
	LEA_STACK(VelocityClass*, pPosition, STACK_OFFS(0x1AC, 0x144));

	if (auto& pTraj = pThis->_GetExtData()->Trajectory)
		pTraj->OnAIVelocity(pSpeed, pPosition);

	return 0;
}

DEFINE_HOOK(0x4677D3, BulletClass_AI_TargetCoordCheck_Trajectories, 0x5)
{
	GET(BulletClass*, pThis, EBP);
	REF_STACK(CoordStruct, coords, STACK_OFFS(0x1A8, 0x184));

	return PhobosTrajectory::OnAITargetCoordCheck(pThis, coords);
}

DEFINE_HOOK(0x467927, BulletClass_AI_TechnoCheck_Trajectories, 0x5)
{

	GET(BulletClass*, pThis, EBP);
	GET(TechnoClass*, pTechno, ESI);

	return PhobosTrajectory::OnAITechnoCheck(pThis, pTechno);
}

DEFINE_HOOK(0x468B72, BulletClass_Unlimbo_Trajectories, 0x5)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(CoordStruct*, pCoord, STACK_OFFS(0x54, -0x4));
	GET_STACK(VelocityClass*, pVelocity, STACK_OFFS(0x54, -0x8));

	PhobosTrajectory::CreateInstance(pThis, pCoord, pVelocity);

	//if (WarheadTypeExtContainer::Instance.Find(pThis->WH)->DirectionalArmor) {
	//	BulletExtContainer::Instance.Find(pThis)->InitialBulletDir = DirStruct(Math::atan2(static_cast<double>(pThis->SourceCoords.Y - pThis->TargetCoords.Y), static_cast<double>(pThis->TargetCoords.X - pThis->SourceCoords.X)));
	//}

	return 0;
}

//#endif