#pragma once
#include <Utilities/TemplateDef.h>
#include "../CommonProperties.h"

struct DestroySelfType
{
	DestroySelfType() :
		Enable { false }
		, Delay { 0 }
		, Peaceful { false }
		, CommonData { }
	{}

	Valueable<bool> Enable;
	Valueable<int> Delay;
	Valueable<bool> Peaceful;
	CommonProperties CommonData;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Enable)
			.Process(Delay)
			.Process(Peaceful)
			;

		CommonData.Serialize(Stm);
	}

};
