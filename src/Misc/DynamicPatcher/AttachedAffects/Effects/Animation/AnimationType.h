#pragma once
#include <Utilities/TemplateDef.h>
#include "../BaseType.h"

struct AnimationType : public EffectType
{
	AnimationType() :
		  Enable { false }
		, IdleAnim { nullptr }
		, ActiveAnim { nullptr }
		, HitAnim { nullptr }
		, DoneAnim { nullptr }
	{ };

	AnimationType(const AnimationType& nAnother) :
		  Enable { nAnother.Enable }
		, IdleAnim { nAnother.IdleAnim.Get() }
		, ActiveAnim { nAnother.ActiveAnim.Get() }
		, HitAnim { nAnother.HitAnim.Get() }
		, DoneAnim { nAnother.DoneAnim.Get() }
	{ };

	bool Enable;
	Valueable<AnimTypeClass*> IdleAnim; // 持续动画
	Valueable<AnimTypeClass*> ActiveAnim; // 激活时播放的动画
	Valueable<AnimTypeClass*> HitAnim; // 被击中时播放的动画
	Valueable<AnimTypeClass*> DoneAnim; // 结束时播放的动画

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Enable)
			.Process(IdleAnim)
			.Process(ActiveAnim)
			.Process(HitAnim)
			.Process(DoneAnim)
			.Success()
			;
	}
};
