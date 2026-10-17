#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

void FakeWeaponTypeClass::_Set_Speed()
{
	if (this->Projectile && !this->Projectile->ROT) {
		const auto gravity = BulletTypeExtData::GetAdjustedGravity(this->Projectile);
		this->Speed = Game::AdjustRangeWithGravity(this->Range, gravity);
	}
}

int FakeWeaponTypeClass::_Get_Speed(int a2)
{
	if (!this->Projectile || this->Projectile->ROT) {
		return this->Speed;
	}

	const auto  gravity = BulletTypeExtData::GetAdjustedGravity(this->Projectile);
	return Game::AdjustRangeWithGravity(a2, gravity);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7729F0, FakeWeaponTypeClass::_Set_Speed)
DEFINE_FUNCTION_JUMP(LJMP, 0x773070, FakeWeaponTypeClass::_Get_Speed)