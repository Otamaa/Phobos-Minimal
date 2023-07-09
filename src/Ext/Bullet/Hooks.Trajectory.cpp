#include "Trajectories/PhobosTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

//#ifdef ENABLE_TRAJ_HOOKS
DEFINE_HOOK(0x4666F7, BulletClass_AI_Trajectories, 0x6)
{
	enum { Detonate = 0x467E53 };

	GET(BulletClass*, pThis, EBP);

	if (!pThis->SpawnNextAnim)
	{
		if (auto& pTraj = BulletExt::ExtMap.Find(pThis)->Trajectory)
			return pTraj->OnAI() ? Detonate : 0x0;
	}

	return 0;
}

DEFINE_HOOK(0x467E53, BulletClass_AI_PreDetonation_Trajectories, 0x6)
{
	GET(BulletClass*, pThis, EBP);

	if (auto& pTraj = BulletExt::ExtMap.Find(pThis)->Trajectory)
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
	GET(BulletClass*, pThis, EBP);
	LEA_STACK(VelocityClass*, pSpeed, STACK_OFFS(0x1AC, 0x11C));
	LEA_STACK(VelocityClass*, pPosition, STACK_OFFS(0x1AC, 0x144));

	if (auto& pTraj = BulletExt::ExtMap.Find(pThis)->Trajectory)
		pTraj->OnAIVelocity(pSpeed, pPosition);

	return 0;
}

DEFINE_HOOK(0x4677D3, BulletClass_AI_TargetCoordCheck_Trajectories, 0x5)
{
	enum { SkipCheck = 0x4678F8, ContinueAfterCheck = 0x467879, Detonate = 0x467E53 };

	GET(BulletClass*, pThis, EBP);
	REF_STACK(CoordStruct, coords, STACK_OFFS(0x1A8, 0x184));

	if (auto& pTraj = BulletExt::ExtMap.Find(pThis)->Trajectory)
	{
		switch (pTraj->OnAITargetCoordCheck(coords))
		{
		case TrajectoryCheckReturnType::SkipGameCheck:
			return SkipCheck;
		case TrajectoryCheckReturnType::SatisfyGameCheck:
			return ContinueAfterCheck;
		case TrajectoryCheckReturnType::Detonate:
			return Detonate;
		default:
			break;
		}
	}

	return 0;
}

DEFINE_HOOK(0x467927, BulletClass_AI_TechnoCheck_Trajectories, 0x5)
{
	enum { SkipCheck = 0x467A2B, ContinueAfterCheck = 0x4679EB, Detonate = 0x467E53 };

	GET(BulletClass*, pThis, EBP);
	GET(TechnoClass*, pTechno, ESI);

	if (auto& pTraj = BulletExt::ExtMap.Find(pThis)->Trajectory)
	{
		switch (pTraj->OnAITechnoCheck(pTechno))
		{
		case TrajectoryCheckReturnType::SkipGameCheck:
			return SkipCheck;
		case TrajectoryCheckReturnType::SatisfyGameCheck:
			return ContinueAfterCheck;
		case TrajectoryCheckReturnType::Detonate:
			return Detonate;
		default:
			break;
		}
	}

	return 0;
}

DEFINE_HOOK(0x468B72, BulletClass_Unlimbo_Trajectories, 0x5)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(CoordStruct*, pCoord, STACK_OFFS(0x54, -0x4));
	GET_STACK(VelocityClass*, pVelocity, STACK_OFFS(0x54, -0x8));

	PhobosTrajectory::CreateInstance(pThis, pCoord, pVelocity);

	//if (WarheadTypeExt::ExtMap.Find(pThis->WH)->DirectionalArmor) {
	//	BulletExt::ExtMap.Find(pThis)->InitialBulletDir = DirStruct(std::atan2(static_cast<double>(pThis->SourceCoords.Y - pThis->TargetCoords.Y), static_cast<double>(pThis->TargetCoords.X - pThis->SourceCoords.X)));
	//}

	return 0;
}

//#endif