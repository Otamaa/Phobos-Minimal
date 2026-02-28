#include "Scripts.h"

#include <Misc/Kratos/Extension/BulletExt.h>
#include <Misc/Kratos/Extension/TechnoExt.h>

#include <Misc/Kratos/Ext/StateType/State/BlackHoleState.h>


BlackHoleState* GetBlackHoleState(ObjectClass* pObject)
{
	BlackHoleState* state = nullptr;
	TryGetBlackHoleState(pObject, state);
	return state;
}

bool TryGetBlackHoleState(ObjectClass* pObject, BlackHoleState*& state)
{
	switch (pObject->WhatAmI())
	{
	case AbstractType::Building:
	case AbstractType::Infantry:
	case AbstractType::Unit:
	case AbstractType::Aircraft:
	{
		state = GetScript<TechnoExt, BlackHoleState>(flag_cast_to<TechnoClass*, true>(pObject));
		break;
	}
	case AbstractType::Bullet:
	{
		state = GetScript<BulletExt, BlackHoleState>(cast_to<BulletClass*, true>(pObject));
		break;
	}
	default:
		state = nullptr;
		break;
	}
	return state != nullptr;
}
