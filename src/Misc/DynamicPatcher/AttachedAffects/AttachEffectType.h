#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include "Effects/Animation/AnimationType.h"
#include "Effects/AttachStatus/AttachStatusType.h"
/*#include "Effects/AutoWeapon/AutoWeaponType.h"
#include "Effects/BlackHole/BlackHoleType.h"
#include "Effects/DestroySelf/DestroySelfType.h"
#include "Effects/DisableWeapon/DisableWeaponType.h"
#include "Effects/FireSuper/FireSuperType.h"
#include "Effects/GiftBox/GiftBoxType.h"
#include "Effects/OverrideWeapon/OverrideWeaponType.h"
#include "Effects/PaintBall/PaintballType.h"*/
#include "Effects/Stand/StandType.h"
//#include "Effects/Tranform/TransformType.h"

enum class CumulativeMode : int
{
	NO = 0, YES = 1, ATTACKER = 2
};


class AttachEffectType final : public Enumerable<AttachEffectType>
{
public:

	AttachEffectType(const char* const pTitle) : Enumerable<AttachEffectType>(pTitle)
		, AffectTypes { }
		, NotAffectTypes { }
		, Duration { 1 }
		, HoldDuration { true }
		, Delay { 0 }
		, RandomDelay { false }
		, MinDelay { 0 }
		, MaxDelay { 0 }
		, InitialDelay { -1 }
		, InitialRandomDelay { false }
		, InitialMinDelay { 0 }
		, InitialMaxDelay { 0 }
		, DiscardOnEntry { false }
		, PenetratesIronCurtain { false }
		, FromTransporter { true }
		, OwnerTarget { false }
		, Cumulative { CumulativeMode::NO }
		, ResetDurationOnReapply { false }
		, Group { -1 }
		, OverrideSameGroup { false }
		, Next {}
		, AttachOnceInTechnoType { false }
		, AttachWithDamage { false }
		, AffectBullet { false }
		, OnlyAffectBullet { false }
		, AffectMissile { true }
		, AffectTorpedo { true }
		, AffectCannon { false }
		, AnimationTypeData {}
		/*
		, AttachStatusTypeData {}
		, AutoWeaponTypeData {}
		, BlackHoleTypeData {}
		, DestroySelfTypeData {}
		, DisableWeaponTypeData {}
		, FireSuperTypeData {}
		, GiftBoxTypeData {}
		, OveerrideWeaponTypeData {}
		, PaintBallTypeData {}*/
		, StandTypeData {}
		//, TransformTypeData {}
	{ }

	ValueableVector<const char*> AffectTypes; // 可影响的单位
	ValueableVector<const char*> NotAffectTypes; // 不可影响的单位
	Valueable<int> Duration; // 持续时间
	Valueable<bool> HoldDuration; // 无限时间
	Valueable<int> Delay; // 不可获得同名的延迟
	Valueable<bool> RandomDelay; // 随机延迟
	Valueable<int> MinDelay; // 随机最小值
	Valueable<int> MaxDelay; // 随机最大值
	Valueable<int> InitialDelay; // 生效前的初始延迟
	Valueable<bool> InitialRandomDelay; // 随机初始延迟
	Valueable<int> InitialMinDelay; // 随机最小值
	Valueable<int> InitialMaxDelay; // 随机最大值
	Valueable<bool> DiscardOnEntry; // 离开地图则失效
	Valueable<bool> PenetratesIronCurtain; // 弹头附加，影响铁幕
	Valueable<bool> FromTransporter; // 弹头附加，乘客附加时，视为载具
	Valueable<bool> OwnerTarget; // 弹头附加，属于被赋予对象
	CumulativeMode Cumulative; // 可叠加
	Valueable<bool> ResetDurationOnReapply; // 不可叠加时，重复获得时是否重置计时器
	Valueable<int> Group; // 分组，同一个分组的效果互相影响，削减或增加持续时间
	Valueable<bool> OverrideSameGroup; // 是否覆盖同一个分组
	PhobosFixedString<0x100> Next; // 结束后播放下一个AE

	// 赋予对象过滤
	Valueable<bool> AttachOnceInTechnoType; // 写在TechnoType上只在创建时赋予一次
	Valueable<bool> AttachWithDamage; // 弹头附加，随着伤害附加，而不是按弹头爆炸位置附加，如在使用AmbientDamage时
	Valueable<bool> AffectBullet; // 弹头附加，附加抛射体
	Valueable<bool> OnlyAffectBullet; // 弹头附加，只附加抛射体
	Valueable<bool> AffectMissile; // 弹头附加，影响ROT>0
	Valueable<bool> AffectTorpedo; // 弹头附加，影响Level=yes
	Valueable<bool> AffectCannon; // 弹头附加，影响Arcing=yes

	AnimationType AnimationTypeData;
	AttachStatusType AttachStatusTypeData;

	/*AutoWeaponType AutoWeaponTypeData;
	BlackHoleType BlackHoleTypeData;
	DestroySelfType DestroySelfTypeData;
	DisableWeaponType DisableWeaponTypeData;
	AEFireSuperType FireSuperTypeData;
	GiftBoxType GiftBoxTypeData;
	OverrideWeaponType OveerrideWeaponTypeData;
	PaintballType PaintBallTypeData;*/
	StandType StandTypeData;
	//TransformType TransformTypeData;

	void LoadFromINI(CCINIClass* pINI) override;
	void LoadFromStream(PhobosStreamReader& Stm) override;
	void SaveToStream(PhobosStreamWriter& Stm) override;

	int GetDuration() {
		return HoldDuration ? -1 : Duration;
	}

private:
	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(AffectTypes)
			.Process(NotAffectTypes)
			.Process(Duration)
			.Process(HoldDuration)
			.Process(Delay)
			.Process(RandomDelay)
			.Process(MinDelay)
			.Process(MaxDelay)
			.Process(InitialDelay)
			.Process(InitialRandomDelay)
			.Process(InitialMinDelay)
			.Process(InitialMaxDelay)
			.Process(DiscardOnEntry)
			.Process(PenetratesIronCurtain)
			.Process(FromTransporter)
			.Process(OwnerTarget)
			.Process(Cumulative)
			.Process(ResetDurationOnReapply)
			.Process(Group)
			.Process(OverrideSameGroup)
			.Process(Next)

			.Process(AttachOnceInTechnoType)
			.Process(AttachWithDamage)
			.Process(AffectBullet)
			.Process(OnlyAffectBullet)
			.Process(AffectMissile)
			.Process(AffectTorpedo)
			.Process(AffectCannon)
			.Process(AnimationTypeData)
			.Process(AttachStatusTypeData)
			/*AutoWeaponTypeData.Serialize(Stm);
			BlackHoleTypeData.Serialize(Stm);
			DestroySelfTypeData.Serialize(Stm);
			DisableWeaponTypeData.Serialize(Stm);
			FireSuperTypeData.Serialize(Stm);
			GiftBoxTypeData.Serialize(Stm);
			OveerrideWeaponTypeData.Serialize(Stm);
			PaintBallTypeData.Serialize(Stm);*/
			.Process(StandTypeData)
			//TransformTypeData.Serialize(Stm);
			;
	}
};
