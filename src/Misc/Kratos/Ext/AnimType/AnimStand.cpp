#include "AnimStand.h"

#include <Misc/Kratos/Ext/Helper/Scripts.h>

#include <Misc/Kratos/Ext/TechnoType/TechnoStatus.h>

#include <Ext/House/Body.h>

StandData* AnimStand::GetStandData()
{
	if (!_data)
	{
		_data = INI::GetConfig<StandData>(INI::Art, pAnim->Type->ID)->Data;
	}
	return _data;
}

void AnimStand::CreateAndPutStand()
{
	CoordStruct location = pAnim->GetCoords();

	StandData* data = GetStandData();
	if (TechnoTypeClass* pType = TechnoTypeClass::Find(data->Type.c_str()))
	{
		HouseClass* pHouse = pAnim->Owner;
		if (!pHouse)
		{
			pHouse = HouseExtData::FindFirstCivilianHouse();
		}
		// Make Stand
		pStand = flag_cast_to<TechnoClass*, true>(pType->CreateObject(pHouse));
		if (pStand)
		{
			// 初始化设置
			if (TechnoStatus* status = GetStatus<TechnoExt, TechnoStatus>(pStand))
			{
				status->VirtualUnit = data->VirtualUnit;
				status->SetupStand(*data, nullptr);
				status->MyMasterIsAnim = true;
			}
			pStand->Mark(MarkType::Remove); // Mark(MarkType::Up)
			bool canGuard = pHouse->IsControlledByHuman();
			if (pStand->WhatAmI() == AbstractType::Building)
			{
				canGuard = true;
			}
			else
			{
				flag_cast_to<FootClass*, true>(pStand)->Locomotor->Lock();
			}
			// only computer units can hunt
			Mission mission = canGuard ? Mission::Guard : Mission::Hunt;
			pStand->QueueMission(mission, false);
			// 放出地图
			if (!TryPutTechno(pStand, location, nullptr, true))
			{
				Debug::Log("Cannot put the Anim[%s]'s Stand on the map.\n", pAnim->Type->ID);
				Disable();
				return;
			}
			// 朝向
			DirStruct targetDir = DirNormalized(data->Offset.Direction, 16);
			pStand->PrimaryFacing.Set_Current(targetDir);
			pStand->SecondaryFacing.Set_Current(targetDir);
		}
	}
	else
	{
		Debug::Log("Warning: Anim [%s] has a unknow type stand [%s].\n", pAnim->Type->ID, data->Type.c_str());
		Disable();
	}
}

void AnimStand::SetLocation(CoordStruct location)
{
	// 动画没有朝向，固定朝北
	DirStruct targetDir;
	CoordStruct targetPos = GetFLHAbsoluteCoords(location, GetStandData()->Offset.Offset, targetDir);
	pStand->SetLocation(targetPos);
}

void AnimStand::OnUpdate()
{
	if (!_initFlag)
	{
		_initFlag = true;
		// 创建替身
		CreateAndPutStand();
	}
	// 移动替身的位置
	if (!IsDeadOrInvisible(pStand))
	{
		// reset state
		pStand->Mark(MarkType::Remove); // 拔起，不在地图上
		CoordStruct location = pAnim->GetCoords();
		SetLocation(location);
	}
	else
	{
		Disable();
	}
}

void AnimStand::OnDone()
{
	// 移除替身
	TechnoClass* pTemp = pStand;
	pStand = nullptr;
	if (pTemp)
	{
		StandData* data = GetStandData();
		bool explodes = (data->Explodes || data->ExplodesWithRocket) && !pTemp->BeingWarpedOut && !pTemp->WarpingOut;
		TechnoStatus* standStatus = nullptr;
		if (TryGetStatus<TechnoExt>(pTemp, standStatus))
		{
			standStatus->DestroySelf->DestroyNow(!explodes);
			// 如果替身处于Limbo状态，OnUpdate不会执行，需要手动触发
			if (pTemp->InLimbo && !KratosCommon::IsScenarioClear)
			{
				standStatus->OnUpdate();
			}
		}
		else
		{
			if (explodes)
			{
				pTemp->TakeDamage(pTemp->Health + 1, pTemp->GetTechnoType()->Crewed);
			}
			else
			{
				pTemp->Limbo();
				pTemp->UnInit(); // 替身攻击建筑时死亡会导致崩溃，莫名其妙的bug
			}
		}
	}
	Disable();
}

