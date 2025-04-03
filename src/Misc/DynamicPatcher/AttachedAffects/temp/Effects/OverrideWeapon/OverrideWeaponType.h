#pragma once

#include <Utilities/TemplateDef.h>
#include "../CommonProperties.h"

struct OverrideWeaponType
{
	OverrideWeaponType() :
		Type { nullptr }
		, EliteType { nullptr }
		, Index { -1 }
		, EliteIndex { -1 }
		, Chance { 1 }
		, EliteChance { 1 }
		, CommonData {}
	{}

	Valueable<WeaponTypeClass*> Type; // 替换武器
	Valueable<WeaponTypeClass*> EliteType; // 精英替换武器
	Valueable<int> Index; // 替换武器序号
	Valueable<int> EliteIndex; // 精英替换武器序号
	Valueable<double> Chance; // 概率
	Valueable<double> EliteChance; // 精英概率
	CommonProperties CommonData;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Type)
			.Process(EliteType)
			.Process(Index)
			.Process(EliteIndex)
			.Process(Chance)
			.Process(EliteChance)
			;
		CommonData.Serialize(Stm);
	}
};
