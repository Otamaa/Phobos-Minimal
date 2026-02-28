#include "MissileHoming.h"

#include <Locomotor/RocketLocomotionClass.h>

#include <Misc/Kratos/Ext/Helper/FLH.h>

#include <Locomotor/Cast.h>

MissileHomingData* MissileHoming::GetHomingData()
{
	if (!_homingData)
	{
		_homingData = INI::GetConfig<MissileHomingData>(INI::Rules, pTechno->GetTechnoType()->ID)->Data;
	}
	return _homingData;
}

void MissileHoming::Setup()
{
	_homingData = nullptr;
	if (IsRocket())
	{
		if (!IsHoming)
		{
			IsHoming = GetHomingData()->Homing;
		}
	}
	else
	{
		Disable();
	}
}

bool MissileHoming::KamikazeUpdateTarget(Kamikaze::KamikazeControl* pKamikazeControl)
{
	if (!IsDeadOrInvisible(pTechno))
	{
		// 首先设置目的地，防止乱飞
		pTechno->SetDestination(pKamikazeControl->Cell, true);
		// Kamikaze导弹跟踪，重设目标
		if (IsHoming and HomingTarget)
		{
			// HomingTarget可以是Techno或者Cell
			TechnoClass* pTargetTechno = nullptr;
			if (CastToTechno(HomingTarget, pTargetTechno))
			{
				pTechno->SetTarget(HomingTarget);
				if (!IsDeadOrInvisibleOrCloaked(pTargetTechno))
				{
					HomingTargetLocation = pTargetTechno->GetCoords();
					// 更新KamikazeControl的Cell，以便KamikazeContainer_Update_SetTargetAfter可以修改导弹的目标位置
					if (CellClass* pCell = MapClass::Instance->GetCellAt(HomingTargetLocation))
					{
						pKamikazeControl->Cell = pCell;
						pTechno->SetDestination(pCell, true);
					}
				}
			}
		}
		return true;
	}
	return false;
}

void MissileHoming::Awake()
{
	Setup();
	EventSystems::General.AddHandler(Events::ObjectUnInitEvent, this, &MissileHoming::OnTechnoDelete);
}

void MissileHoming::OnUpdate()
{
	// 调整起飞姿态，面相目标
	if (!_initHomingFlag)
	{
		_initHomingFlag = true;
		if (GetHomingData()->FacingTarget)
		{
			CoordStruct sourcePos = pTechno->GetCoords();
			CoordStruct targetPos = flag_cast_to<FootClass*, true>(pTechno)->Locomotor->Destination();
			DirStruct dir = Point2Dir(sourcePos, targetPos);
			pTechno->PrimaryFacing.Set_Current(dir);
			pTechno->SecondaryFacing.Set_Current(dir);
		}
	}
	// 读取目标位置修改飞行目的地
	if (IsHoming && !IsDeadOrInvisible(pTechno))
	{
		// 导弹开始飞行后，修改目的地为追踪的目标位置
		RocketLocomotionClass* pLoco = locomotion_cast<RocketLocomotionClass*>(flag_cast_to<FootClass*, true>(pTechno)->Locomotor);
		if (pLoco)
		{
			// 目标可能在准备阶段就死了，不论如何首先记录下最后的位置
			if (HomingTarget && HomingTargetLocation.IsEmpty())
			{
				HomingTargetLocation = HomingTarget->GetCoords();
			}
			if ((int)pLoco->MissionState > 2)
			{
				// 导弹飞行过程中不会更改目标，一但失去目标说明目标已经死亡
				AbstractClass* pTarget = pTechno->Target;
				TechnoClass* pTargetTechno = nullptr;
				if (!HomingTarget || pTarget != HomingTarget || !CastToTechno(pTarget, pTargetTechno) || IsDeadOrInvisibleOrCloaked(pTargetTechno))
				{
					// 目标消失，关闭追踪
					IsHoming = false;
					HomingTarget = nullptr;
					// 重新设置目标，以便下一次更新时可以追踪到正确的位置
					CoordStruct newLocation = HomingTargetLocation;
					if (newLocation.IsEmpty())
					{
						// 使用导弹所在位置作为下一次追踪的目标位置
						newLocation = pTechno->GetCoords();
					}
					if (CellClass* pCell = MapClass::Instance->GetCellAt(newLocation))
					{
						pTechno->SetTarget(pCell);
						pTechno->SetDestination(pCell, true);
					}
				}
				else if (HomingTarget)
				{
					// 追踪目标位置
					if (CastToTechno(HomingTarget, pTargetTechno))
					{
						// 如果目标消失，导弹会追到最后一个位置然后爆炸
						if (!IsDeadOrInvisibleOrCloaked(pTargetTechno))
						{
							HomingTargetLocation = pTargetTechno->GetCoords();
						}
					}
				}
				if (!HomingTargetLocation.IsEmpty())
				{
					// 更新导弹目的地位置
					pLoco->MovingDestination = HomingTargetLocation;
				}
			}
		}
	}
}
