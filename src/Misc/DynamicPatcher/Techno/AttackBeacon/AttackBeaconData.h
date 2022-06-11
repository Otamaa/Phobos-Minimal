#pragma once
#include <Utilities/TemplateDef.h>

struct AttackBeaconData
{
	ValueableVector<TechnoTypeClass*> Types;
	DynamicVectorClass<int> Num;
	Valueable<int> Rate;
	Valueable<int> Delay;
	Valueable<int> RangeMin;
	Valueable<int> RangeMax;
	Valueable<bool> Close;
	Valueable<bool> Force;
	Valueable<int> Count;
	Valueable<bool> TargetToCell;
	Valueable<bool> AffectsOwner;
	Valueable<bool> AffectsAllies;
	Valueable<bool> AffectsEnemies;


	AttackBeaconData() :
		Types {	}
		, Num { }
		, Rate { 0 }
		, Delay { 0 }
		, RangeMin { 0 }
		, RangeMax { -1 }
		, Close { true }
		, Force { false }
		, Count { 1 }
		, TargetToCell { false }
		, AffectsOwner { true }
		, AffectsAllies { false }
		, AffectsEnemies { false }

	{	}

	void Read(INI_EX& parserRules, const char* pSection_rules);

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Loading Element From AttackBeaconData ! \n");

		Stm
			.Process(Types)
			.Process(Num)
			.Process(Rate)
			.Process(Delay)
			.Process(RangeMin)
			.Process(RangeMax)
			.Process(Close)
			.Process(Force)
			.Process(Count)
			.Process(TargetToCell)
			.Process(AffectsOwner)
			.Process(AffectsAllies)
			.Process(AffectsEnemies)
			;
	}
};