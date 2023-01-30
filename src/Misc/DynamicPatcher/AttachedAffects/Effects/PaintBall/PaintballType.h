#pragma once

#include <Utilities/TemplateDefB.h>
#include "../CommonProperties.h"

struct PaintballType
{
	PaintballType():
		Color { {0,0,0} }
		, IsHouseColor { false }
		, BrightMultiplier { 1.0f }
		, CommonData {}
	{}

	Valueable<ColorStruct> Color; // 颜色
	Valueable<bool> IsHouseColor; // 使用所属色
	Valueable<float> BrightMultiplier; // 亮度系数
	CommonProperties CommonData;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Color)
			.Process(IsHouseColor)
			.Process(BrightMultiplier)
			;

		CommonData.Serialize(Stm);
	}

};