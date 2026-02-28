#include "../TechnoStatus.h"

#include <FootClass.h>
#include <Locomotor/JumpjetLocomotionClass.h>

#include <Misc/Kratos/Common/INI/INI.h>

#include <Misc/Kratos/Ext/Helper/Gift.h>
#include <Misc/Kratos/Ext/Helper/Scripts.h>

#include <Misc/Kratos/Ext/EffectType/AttachEffectScript.h>
#include <Misc/Kratos/Ext/ObjectType/AttachEffect.h>


DeployToTransformData* TechnoStatus::GetTransformData()
{
	if (!_transformData)
	{
		_transformData = INI::GetConfig<DeployToTransformData>(INI::Rules, pTechno->GetTechnoType()->ID)->Data;
	}
	return _transformData;
}

DeployToAttachData* TechnoStatus::GetDeployAttachData()
{
	if (!_deployAttachData)
	{
		_deployAttachData = INI::GetConfig<DeployToAttachData>(INI::Rules, pTechno->GetTechnoType()->ID)->Data;
	}
	return _deployAttachData;
}

void TechnoStatus::OnUpdate_DeployTo()
{
	bool transform = GetTransformData()->Enable;
	bool attachEffect = GetDeployAttachData()->Enable;

	if (!transform && !attachEffect)
	{
		return;
	}

	switch (DeployState)
	{
	case DeployState::None:
		if (IsInfantry())
		{
			// 步兵序列为Deploy和Undeploy时，即是开始部署
			DoType sequence = cast_to<InfantryClass*, true>(pTechno)->SequenceAnim;
			if (sequence == DoType::Deploy)
			{
				DeployState = DeployState::Deploying;
			}
			else if (sequence == DoType::Undeploy)
			{
				DeployState = DeployState::Undeploying;
			}
		}
		else if (IsUnit())
		{
			// 载具任务处于unload时，即是开始部署
			if (pTechno->CurrentMission == Mission::Unload)
			{
				DeployState = DeployState::Deploying;
				// 如果是IsSimpleDeploy， 在部署完成会设置Deployed，否则会设置为false
				if (cast_to<UnitClass*, true>(pTechno)->Deployed)
				{
					DeployState = DeployState::Undeploying;
				}
			}
		}
		break;
	case DeployState::Deploying:
		if (IsInfantry())
		{
			// 步兵序列为Deployed时，即是部署完成
			DoType sequence = cast_to<InfantryClass*, true>(pTechno)->SequenceAnim;
			if (sequence == DoType::Deployed)
			{
				DeployState = DeployState::Deployed;
			}
		}
		else if (IsUnit())
		{
			// 如果是IsSimpleDeploy， 在部署完成会设置Deployed，否则会设置为false
			if (cast_to<UnitClass*, true>(pTechno)->Deployed)
			{
				DeployState = DeployState::Deployed;
				break;
			}
			// 如果是其他类型，DeployFire，需要在Hook中判断是否部署完成，此处不处理
			if (pTechno->CurrentMission != Mission::Unload)
			{
				DeployState = DeployState::None;
			}
		}
		break;
	case DeployState::Undeploying:
		if (IsInfantry())
		{
			// 步兵序列不为Undeploy时，即是部署完成
			DoType sequence = cast_to<InfantryClass*, true>(pTechno)->SequenceAnim;
			if (sequence != DoType::Undeploy)
			{
				DeployState = DeployState::Undeployed;
			}
		}
		else if (IsUnit())
		{
			// 如果是IsSimpleDeploy， 在部署完成会设置Deployed，否则会设置为false
			if (cast_to<UnitClass*, true>(pTechno)->Deployed)
			{
				DeployState = DeployState::Undeployed;
				break;
			}
			// 如果是其他类型，DeployFire，需要在Hook中判断是否部署完成，此处不处理
			if (pTechno->CurrentMission != Mission::Unload)
			{
				DeployState = DeployState::None;
			}
		}
		break;
	}

	// 部署触发变形或附加AE
	if (DeployState == DeployState::Deployed)
	{
		DeployState = DeployState::None;

		if (GetTransformData()->DeployTo)
		{
			// 部署变形，在单位部署完成后触发
			GiftBox->Start(&GetTransformData()->DeployToTransform);
		}
		else if (GetDeployAttachData()->DeployTo)
		{
			// 部署附加AE
			AttachEffect* aeManager = AEManager();
			if (aeManager)
			{
				// 附加AE
				aeManager->Attach(GetDeployAttachData()->DeployToAttachEffects, GetDeployAttachData()->DeployToAttachChances, false);
			}
		}
	}
	else if (DeployState == DeployState::Undeployed)
	{
		DeployState = DeployState::None;

		if (GetTransformData()->UndeployTo)
		{
			// 卸载变形，在单位卸载完成后触发
			GiftBox->Start(&GetTransformData()->UndeployToTransform);
		}
		else if (GetDeployAttachData()->UndeployTo)
		{
			// 卸载附加AE
			AttachEffect* aeManager = AEManager();
			if (aeManager)
			{
				// 附加AE
				aeManager->Attach(GetDeployAttachData()->UndeployToAttachEffects, GetDeployAttachData()->UndeployToAttachChances, false);
			}
		}
	}
}
