#include <exception>
#include <Windows.h>

#include <GeneralStructures.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <AircraftClass.h>
#include <MapClass.h>
#include <ObjectTypeClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/TechnoExt.h>
#include <Misc/Kratos/Extension/WarheadTypeExt.h>

#include <Misc/Kratos/Ext/Common/CommonStatus.h>
#include <Misc/Kratos/Ext/TechnoType/TechnoStatus.h>
#include <Misc/Kratos/Ext/TechnoType/AircraftAttitude.h>
#include <Misc/Kratos/Ext/TechnoType/AircraftDive.h>
#include <Misc/Kratos/Ext/TechnoType/AircraftGuard.h>

#include <Locomotor/Cast.h>

ASMJIT_PATCH(0x639DD8, PlanningManager_AllowAircraftsWaypoint, 0x5)
{
	GET(TechnoClass*, pTechno, ESI);
	switch (pTechno->WhatAmI())
	{
	case AbstractType::Infantry:
	case AbstractType::Unit:
		return 0x639DDD;
	case AbstractType::Aircraft:
		if (!pTechno->GetTechnoType()->Spawned)
		{
			return 0x639DDD;
		}
	}
	return 0x639E03;
}

#ifndef _ENABLE_HOOKS

#pragma region DrawShadow
// Phobos took over the entire shadow rendering process, so skip Phobos's Hook here
ASMJIT_PATCH(0x73C47A, UnitClass_DrawAsVXL_Shadow_SkipPhobos, 0x5)
{
	if (AudioVisual::Data()->AllowTakeoverPhobosShadowMaker)
	{
		GET(UnitClass*, pThis, EBP);
		int height = pThis->GetHeight();
		R->EAX(height);
		ILocomotion* pLoco = pThis->Locomotor;
		if (pThis->CloakState != CloakState::Uncloaked || pThis->Type->NoShadow || !pLoco->Is_To_Have_Shadow())
		{
			// 彻底跳过阴影绘制过程，不论是Phobos还是原版
			return 0x73C5C9;
		}
		// 只跳过Phobos的Hook, 继续下一步
		return 0x73C485;
	}
	return 0;
}

// Phobos took over the entire shadow rendering process, so skip Phobos's Hook here
ASMJIT_PATCH(0x4147F9, AircraftClass_Draw_Shadow_SkipPhobos, 0x6)
{
	if (AudioVisual::Data()->AllowTakeoverPhobosShadowMaker)
	{
		GET(AircraftClass*, pThis, EBP);
		ILocomotion* pLoco = pThis->Locomotor;
		R->EAX(pLoco);
		if (pThis->Type->NoShadow || pThis->CloakState != CloakState::Uncloaked || pThis->IsSinking || !pLoco->Is_To_Have_Shadow())
		{
			// 彻底跳过阴影绘制过程，不论是Phobos还是原版
			return 0x4148A5;
		}
		// 只跳过Phobos的Hook, 继续下一步
		return 0x4147FF;
	}
	return 0;
}

// Phobos skip the entire shadow process, there are no phobos if this hook action
ASMJIT_PATCH(0x707323, TechnoClass_DrawShadow_SkipAircraftScale, 0x5)
{
	// There are no have Phobos b39
	return 0x707331;
}

DEFINE_HOOK_AGAIN(0x73C4F8, TechnoClass_DrawShadow, 0x7) // InAir
DEFINE_HOOK_AGAIN(0x73C58E, TechnoClass_DrawShadow, 0x7) // OnGround
ASMJIT_PATCH(0x414876, TechnoClass_DrawShadow, 0x7) // Aircraft
{
	GET(TechnoClass*, pTechno, EBP);
	GET(Matrix3D*, pMatrix, EAX);
	TechnoTypeClass* pType = pTechno->GetTechnoType();
	if (pType->ConsideredAircraft || pTechno->WhatAmI() == AbstractType::Aircraft)
	{
		// 修复子机导弹的影子位置
		if (pType->MissileSpawn && !pType->NoShadow)
		{
			CoordStruct location = pTechno->GetCoords();
			CellClass* pCell = MapClass::Instance->TryGetCellAt(location);
			location.Z = pCell->GetCoordsWithBridge().Z;
			Point2D pos = ToClientPos(location);
			R->Stack(0x30, pos);
		}
		// 缩放影子
		pMatrix->Scale(AudioVisual::Data()->VoxelShadowScaleInAir);
		if (pType->MissileSpawn || pTechno->IsInAir())
		{
			// 调整倾斜时影子的纵向比例
			FootClass* pFoot = flag_cast_to<FootClass*, true>(pTechno);
			// 从Matrix中读取的角度不可用
			float x = 0; // 倾转轴
			float y = 0; // 俯仰轴
			// 火箭的俯仰角度，由RocketLoco记录
			if (RocketLocomotionClass* rLoco = locomotion_cast<RocketLocomotionClass*>(pFoot->Locomotor))
			{
				x = pTechno->AngleRotatedSideways;
				y = rLoco->CurrentPitch;
			}
			else if (FlyLocomotionClass* fLoco = locomotion_cast<FlyLocomotionClass*>(pFoot->Locomotor))
			{
				if (fLoco->Is_Moving_Now())
				{
					x = (float)pType->RollAngle;
				}
				else
				{
					x = pTechno->AngleRotatedSideways;
				}
				// 飞行器的俯仰角度有Techno->AngleRotatedForwards, tt.PitchAngle
				// 根据速度结算
				if (fLoco->TargetSpeed <= pType->PitchSpeed)
				{
					y = pTechno->AngleRotatedForwards;
				}
				else
				{
					y = (float)pType->PitchAngle;
				}
				// 加上姿态的角度
				if (AircraftAttitude* attitude = GetScript<TechnoExt, AircraftAttitude>(pTechno))
				{
					y += attitude->PitchAngle;
				}
			}
			else
			{
				x = pTechno->AngleRotatedSideways;
				y = pTechno->AngleRotatedForwards;
			}
			float scaleY = 1.0;
			float absX = Math::abs(x);
			if (absX >= 0.005)
			{
				scaleY = (float)Math::cos(absX);
				pMatrix->ScaleY(scaleY);
			}
			float scaleX = 1.0;
			float absY = Math::abs(y);
			if (absY >= 0.005)
			{
				scaleX = (float)Math::cos(absY);
				pMatrix->ScaleX(scaleX);
			}
			if (scaleY != 1.0 || scaleX != 1.0)
			{
				pType->ReleaseAllVoxelCaches();
			}
		}
	}
	return 0;
}
#pragma endregion

#pragma region Aircraft Attitude

ASMJIT_PATCH(0x4CF80D, FlyLocomotionClass_Draw_Matrix, 0x5)
{
	FlyLocomotionClass* pFly = (FlyLocomotionClass*)(R->ESI() - 4);
	if (TechnoClass* pTechno = pFly->LinkedTo)
	{
		AircraftAttitude* attitude = nullptr;
		if (TryGetScript<TechnoExt, AircraftAttitude>(pTechno, attitude) && attitude->PitchAngle != 0)
		{
			GET_STACK(Matrix3D, matrix3D, 0x8);
			matrix3D.RotateY(attitude->PitchAngle);
			R->Stack(0x8, matrix3D);
		}
	}
	return 0;
}

ASMJIT_PATCH(0x4CF4C5, FlyLocomotionClass_FlightLevel_ResetA, 0xA)
{
	GET(FlyLocomotionClass*, pFly, EBP);
	int flightLevel = RulesClass::Instance->FlightLevel;
	if (TechnoClass* pTechno = pFly->LinkedTo)
	{
		AircraftDive* dive = nullptr;
		if (TryGetScript<TechnoExt, AircraftDive>(pTechno, dive)
			&& dive->DiveStatus == AircraftDive::AircraftDiveStatus::DIVEING)
		{
			flightLevel = dive->GetAircraftDiveData()->FlightLevel;
		}
		else
		{
			flightLevel = pTechno->GetTechnoType()->GetFlightLevel();
		}
	}
	R->EAX(flightLevel);
	return 0x4CF4CF;
}

ASMJIT_PATCH(0x4CF3E3, FlyLocomotionClass_FlightLevel_ResetB, 0x9)
{
	GET(FlyLocomotionClass*, pFly, EBP);
	if (TechnoClass* pTechno = pFly->LinkedTo)
	{
		AircraftDive* dive = nullptr;
		if (TryGetScript<TechnoExt, AircraftDive>(pTechno, dive)
			&& dive->DiveStatus == AircraftDive::AircraftDiveStatus::DIVEING)
		{
			return 0x4CF4D2;
		}
	}
	return 0;
}

//ASMJIT_PATCH(0x4CF3C5, FlyLocomotionClass_4CEFB0, 0x6)
//{
//	// 调整飞机的朝向，有目标时获取目标的朝向，没有目标时获得默认朝向，此时EAX为0
//	// EAX是目标DirStruct的指针
//	// ECX是当前Facing的指针
//	// ESI是飞机的指针的指针
//	GET(DirStruct*, pDirEAX, EAX);
//	if (!pDirEAX)
//	{
//		GET(DirStruct*, pDir, EDX);
//		GET(TechnoClass**, ppTechno, ESI);
//		TechnoClass* pTechno = *ppTechno;
//		// 如果是Spawnd就全程强制执行
//		// Mission是Enter的普通飞机就不管
//		if (pTechno->IsInAir()
//			&& (pTechno->GetTechnoType()->Spawned || pTechno->CurrentMission != Mission::Enter))
//		{
//			pDir->SetDir(16,pTechno->SecondaryFacing.Current().GetValue());
//		}
//	}
//	return 0;
//}

// Hook address form Otamma
//ASMJIT_PATCH(0x41B760, IFlyControl_Landing_Direction, 0x6)
//{
//	GET_STACK(IFlyControl*, pAircraft, 0x4); // IFlyControl*
//	int poseDir = RulesClass::Instance->PoseDir;
//	AircraftAttitude* attitude = nullptr;
//	TechnoClass* pTechno = static_cast<AircraftClass*>(pAircraft);
//	if (TryGetScript<TechnoExt, AircraftAttitude>(pTechno, attitude)
//		&& attitude->TryGetAirportDir(poseDir))
//	{
//		R->EAX(poseDir);
//		return 0x41B7C1;
//	}
//	return 0;
//}

#pragma endregion

#pragma region Aircraft Guard
ASMJIT_PATCH(0x41A697, AircraftClass_Mission_Guard_NoTarget_Enter, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	AircraftGuard* fighter = nullptr;
	if (TryGetScript<TechnoExt, AircraftGuard>(pTechno, fighter)
		&& fighter->IsAreaGuardRolling())
	{
		// 不返回机场，而是继续前进直到目的地
		return 0x41A6AC;
	}
	return 0;
}

ASMJIT_PATCH(0x41A96C, AircraftClass_Mission_GuardArea_NoTarget_Enter, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	AircraftGuard* fighter = nullptr;
	if (TryGetScript<TechnoExt, AircraftGuard>(pTechno, fighter))
	{
		// 不返回机场，而是继续前进直到目的地
		fighter->StartAreaGuard();
		return 0x41A97A;
	}
	return 0;
}

ASMJIT_PATCH(0x4CF780, FlyLocomotionClass_Draw_Matrix_Rolling, 0x5)
{
	FlyLocomotionClass* pFly = (FlyLocomotionClass*)(R->ESI() - 4);
	TechnoClass* pTechno = pFly->LinkedTo;
	AircraftGuard* fighter = nullptr;
	if (pTechno && pTechno->GetTechnoType()->RollAngle != 0
		&& TryGetScript<TechnoExt, AircraftGuard>(pTechno, fighter)
		&& fighter->State == AircraftGuard::AircraftGuardStatus::ROLLING)
	{
		// 保持倾斜
		if (fighter->Clockwise)
		{
			return 0x4CF7B0; // 右倾
		}
		else
		{
			return 0x4CF7DF; // 左倾
		}
	}
	return 0;
}

#pragma endregion

#endif