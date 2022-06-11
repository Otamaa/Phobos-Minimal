#pragma once
#include <Utilities/TemplateDef.h>
#include "../CommonProperties.h"

struct AutoWeaponType
{
	AutoWeaponType() :
		WeaponIndex { -1 }
		, EliteWeaponIndex {}
		, WeaponTypes {}
		, EliteWeaponTypes {}
		, RandomTypesNum { 0 }
		, EliteRandomTypesNum {}
		, FireOnce { false }
		, FireFLH { {0,0,0} }
		, EliteFireFLH { }
		, TargetFLH { {0,0,0} }
		, EliteTargetFLH { }
		, MoveTo {}
		, EliteMoveTo { }
		, FireToTarget { false }
		, IsOnTurret { true }
		, IsOnWorld { false }
		, IsAttackerMark { false }
		, ReceiverAttack { true }
		, ReceiverOwnBullet { false }
		, CommonData {}
	{ }

	Valueable<int> WeaponIndex; // 使用单位自身的武器
	Nullable<int> EliteWeaponIndex; // 精英时使用单位自身的武器
	ValueableVector<WeaponTypeClass*> WeaponTypes; // 武器类型
	NullableVector<WeaponTypeClass*> EliteWeaponTypes; // 精英武器类型
	Valueable<int> RandomTypesNum; // 随机使用几个武器
	Nullable<int> EliteRandomTypesNum; // 精英时随机使用几个武器
	Valueable<bool> FireOnce; // 发射后销毁
	Valueable<CoordStruct> FireFLH; // 开火相对位置
	Nullable<CoordStruct> EliteFireFLH; // 精英开火相对位置
	Valueable<CoordStruct> TargetFLH; // 目标相对位置
	Nullable<CoordStruct> EliteTargetFLH; // 精英目标相对位置
	Nullable<CoordStruct> MoveTo; // 以开火位置为坐标0点，计算TargetFLH
	Nullable<CoordStruct> EliteMoveTo; // 以开火位置为坐标0点，计算EliteTargetFLH
	Valueable<bool> FireToTarget; // 朝附加对象的目标开火，如果附加的对象没有目标，不开火
	Valueable<bool> IsOnTurret; // 相对炮塔或者身体
	Valueable<bool> IsOnWorld; // 相对世界

	// 攻击者标记
	Valueable<bool> IsAttackerMark; // 允许附加对象和攻击者进行交互
	Valueable<bool> ReceiverAttack; // 武器由AE的接受者发射
	Valueable<bool> ReceiverOwnBullet; // 武器所属是AE的接受者

	CommonProperties CommonData;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(WeaponIndex)
			.Process(EliteWeaponIndex)
			.Process(WeaponTypes)
			.Process(EliteWeaponTypes)
			.Process(RandomTypesNum)
			.Process(EliteRandomTypesNum)
			.Process(FireOnce)
			.Process(FireFLH)
			.Process(EliteFireFLH)
			.Process(TargetFLH)
			.Process(EliteTargetFLH)
			.Process(MoveTo)
			.Process(EliteMoveTo)
			.Process(FireToTarget)
			.Process(IsOnTurret)
			.Process(IsOnWorld)
			.Process(IsAttackerMark)
			.Process(ReceiverAttack)
			.Process(ReceiverOwnBullet)
			;

		CommonData.Serialize(Stm);
	}
};
