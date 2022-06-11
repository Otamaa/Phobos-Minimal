#pragma once
#include <Utilities/TemplateDef.h>

struct TransformType
{
	TransformType()  :
		ToType { nullptr }
	{ }

	Valueable<TechnoTypeClass*> ToType;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(ToType)
			;
	}
};