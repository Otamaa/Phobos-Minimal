#pragma once
#include <Utilities/TemplateDef.h>

struct AnimationType
{
	AnimationType() :
		IdleAnim { nullptr }
		, ActiveAnim { nullptr }
		, HitAnim { nullptr }
		, DoneAnim { nullptr }
	{ };

	Valueable<AnimTypeClass*> IdleAnim; // 持续动画
	Valueable<AnimTypeClass*> ActiveAnim; // 激活时播放的动画
	Valueable<AnimTypeClass*> HitAnim; // 被击中时播放的动画
	Valueable<AnimTypeClass*> DoneAnim; // 结束时播放的动画

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(IdleAnim)
			.Process(ActiveAnim)
			.Process(HitAnim)
			.Process(DoneAnim)
			;
	}
};
