#pragma once
#include <Utilities/TemplateDef.h>
#include "../CommonProperties.h"

struct DisableWeaponType
{

	DisableWeaponType() :
		WeaponDisable { false }
		, CommonData {}
	{}

	Valueable<bool> WeaponDisable;
	CommonProperties CommonData;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(WeaponDisable)
			;

		CommonData.Serialize(Stm);
	}
};