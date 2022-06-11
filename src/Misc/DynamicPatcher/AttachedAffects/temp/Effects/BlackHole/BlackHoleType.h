#pragma once
#include <Utilities/TemplateDef.h>
#include "../CommonProperties.h"

struct BlackHoleType
{
	BlackHoleType() :

		// 基础设置
		Range { 0 }
		, EliteRange {}
		, Rate { 15 }
		, EliteRate {}
		// 类型过滤
		, AffectTypes {}
		, NotAffectTypes {}
		, AffectTechno { false }
		, OnlyAffectTechno { false }
		, AffectMissile { true }
		, AffectTorpedo { true }
		, AffectCannon { false }
		// 敌我识别
		, AffectsOwner { false }
		, AffectsAllies { false }
		, AffectsEnemies { true }
		, CommonData {}
	{ }

	// 基础设置
	Valueable<int> Range;
	Nullable<int> EliteRange;
	Valueable<int> Rate;
	Nullable<int> EliteRate;
	// 类型过滤
	ValueableVector<ObjectTypeClass*> AffectTypes;
	ValueableVector<ObjectTypeClass*> NotAffectTypes;
	Valueable<bool> AffectTechno;
	Valueable<bool> OnlyAffectTechno;
	Valueable<bool> AffectMissile;
	Valueable<bool> AffectTorpedo;
	Valueable<bool> AffectCannon;
	// 敌我识别
	Valueable<bool> AffectsOwner;
	Valueable<bool> AffectsAllies;
	Valueable<bool> AffectsEnemies;

	CommonProperties CommonData;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Range)
			.Process(EliteRange)
			.Process(Rate)
			.Process(EliteRate)
			.Process(AffectTypes)
			.Process(NotAffectTypes)
			.Process(AffectTechno)
			.Process(OnlyAffectTechno)
			.Process(AffectMissile)
			.Process(AffectTorpedo)
			.Process(AffectCannon)
			.Process(AffectsOwner)
			.Process(AffectsAllies)
			.Process(AffectsEnemies)
			;

		CommonData.Serialize(Stm);
	}
};