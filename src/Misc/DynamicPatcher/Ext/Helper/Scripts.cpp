#include "Scripts.h"

#include <Misc/DynamicPatcher/Extension/BulletExt.h>
#include <Misc/DynamicPatcher/Extension/TechnoExt.h>

#include <Misc/DynamicPatcher/Ext/StateType/State/BlackHoleState.h>


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
		state = GetScript<TechnoExt, BlackHoleState>(static_cast<TechnoClass*>(pObject));
		break;
	}
	case AbstractType::Bullet:
	{
		state = GetScript<BulletExt, BlackHoleState>(static_cast<BulletClass*>(pObject));
		break;
	}
	default:
		state = nullptr;
		break;
	}
	return state != nullptr;
}
