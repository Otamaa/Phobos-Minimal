#pragma once

#include <AlphaShapeClass.h>

struct AttachedAlpha
{
	AlphaShapeClass* Alpha;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Alpha)
			;
	}
};