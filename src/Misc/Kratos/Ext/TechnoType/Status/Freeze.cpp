#include "../TechnoStatus.h"

#include <Misc/Kratos/Ext/Helper/Status.h>

DestroyAnimData* TechnoStatus::GetDestroyAnimData()
{
	if (!_destroyAnimData)
	{
		_destroyAnimData = INI::GetConfig<DestroyAnimData>(INI::Rules, pTechno->GetTechnoType()->ID)->Data;
	}
	return _destroyAnimData;
}

void TechnoStatus::OnUpdate_Freeze()
{
	Freezing = Freeze->IsAlive() || GetDestroyAnimData()->Wreck; // 我是残骸
	if (Freezing)
	{
		if (!_cantMoveFlag)
		{
			// 清除所有目标
			ClearAllTarget(pTechno);
			// 马上停止活动
			if (FootClass* pFoot = flag_cast_to<FootClass*, true>(pTechno))
			{
				ForceStopMoving(pFoot);
			}
		}
	}
	else
	{
		_cantMoveFlag = false;
	}
}
