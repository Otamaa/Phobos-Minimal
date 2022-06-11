#pragma once

#include "../Base.h"

class Animation : public EffectsBase
{
	AnimClass* pAnim;
	bool OnwerIsDead;
	
	Animation() :
		pAnim { nullptr }
		, OnwerIsDead { false }
	{ }

	virtual void OnEnable(ObjectClass* pObject, HouseClass* pHouse, TechnoClass* pAttacker) override
	{
		// 激活动画
		// Logger.Log("效果激活，播放激活动画{0}", Type.ActiveAnim);
		if (!string.IsNullOrEmpty(Type.ActiveAnim))
		{
			Pointer<AnimTypeClass> pAnimType = AnimTypeClass.ABSTRACTTYPE_ARRAY.Find(Type.ActiveAnim);
			if (!pAnimType.IsNull)
			{
				Pointer<AnimClass> pAnim = YRMemory.Create<AnimClass>(pAnimType, pObject.Ref.Base.GetCoords());
				pAnim.Ref.SetOwnerObject(pObject);
				pAnim.SetAnimOwner(pObject);
			}
		}
		// 持续动画
		CreateAnim(pObject);
	}
};