#include "../TechnoStatus.h"

#include <Misc/Kratos/Extension/WeaponTypeExt.h>

#include <Misc/Kratos/Ext/Helper/Scripts.h>
#include <Misc/Kratos/Ext/Helper/Status.h>

void TechnoStatus::RockerPitch(WeaponTypeClass* pWeapon)
{
	WeaponTypeExt::TypeData* weaponData = nullptr;
	if (TryGetTypeData<WeaponTypeExt, WeaponTypeExt::TypeData>(pWeapon, weaponData) && weaponData->RockerPitch > 0)
	{
		float gamma = weaponData->RockerPitch;
		// 无炮塔的简单情况
		if (!pTechno->HasTurret())
		{
			pTechno->RockingForwardsPerFrame = -gamma;
			pTechno->RockingSidewaysPerFrame = 0.0f;
			return;
		}

		// 计算炮塔相对车体的旋转角度，直接使用short
		short turretRaw = pTechno->F_GetRealfacing().Current().Raw;
		float bodyRaw = pTechno->PrimaryFacing.Current().Raw;
		// 相对角度，处理为int
		int diff = turretRaw - bodyRaw;
		// 转回short范围，范围为[-32768, 32767]，对应[-pi, pi]
		// 转换为[-1,1]的范围，用于cos/sin
		float normalized = static_cast<float>(diff) / 32768.0f;
		float theta = normalized * Math::GAME_PI;
		// 计算cos/sin，用于计算前后倾斜幅度
		float cosTheta = cosf(theta);
		float sinTheta = sinf(theta);

		pTechno->RockingForwardsPerFrame = -gamma * cosTheta;
		pTechno->RockingSidewaysPerFrame = -gamma * sinTheta;
	}
}


void TechnoStatus::OnFire_RockerPitch(AbstractClass* pTarget, int weaponIdx)
{
	if (_isVoxel)
	{
		if (WeaponTypeClass* pWeapon = pTechno->GetWeapon(weaponIdx)->WeaponType)
		{
			RockerPitch(pWeapon);
		}
	}
}
