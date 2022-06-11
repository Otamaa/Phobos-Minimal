#pragma once

#include "../AttachEffectBehaviour.h"

enum class EffectType
{
	None = 0
};

class EffectsBase
{
public:

	virtual EffectType WhatAmI() {
		return EffectType::None;
	}

	// 返回AE是否还存活
	virtual bool IsAlive() R1;
	// AE激活，开始生效
	virtual void Enable(ObjectClass* pOwner, HouseClass* pHouse, TechnoClass* pAttacker)RX;
	// AE关闭，销毁相关资源
	virtual void Disable(CoordStruct location)RX;
	// 重置计时器
	virtual void ResetDuration()RX;
	// 更新
	virtual void OnUpdate(ObjectClass* pOwner, bool isDead)RX;
	// 被超时空冻结更新
	virtual void OnTemporalUpdate(TechnoClass* ext, TemporalClass* pTemporal)RX;
	// 挂载AE的单位出现在地图上
	virtual void OnPut(ObjectClass* pOwner, CoordStruct* pCoord, Direction faceDir)RX;
	// 挂载AE的单位从地图隐藏
	virtual void OnRemove(ObjectClass* pOwner)RX;
	// 收到伤害
	virtual void OnReceiveDamage(ObjectClass* pOwner, int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH, ObjectClass* pAttacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse)RX;
	// 收到伤害导致死亡
	virtual void OnDestroy(ObjectClass* pOwner)RX;
	// 按下G键
	virtual void OnGuardCommand()RX;
	// 按下S键
	virtual void OnStopCommand()RX;
};

