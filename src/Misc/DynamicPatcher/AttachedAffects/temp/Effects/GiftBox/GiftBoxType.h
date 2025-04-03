#pragma once

#include <Utilities/TemplateDef.h>
#include "../CommonProperties.h"

struct GiftBoxType
{
	GiftBoxType() :
		Enable { false }
		, Gifts {}
		, Chances {}
		, Nums {}
		, Remove {true}
		, Destroy {false}
		, Delay {0}
		, RandomDelay {}
		, RandomRange {0}
		, EmptyCell { false }
		, RandomType { false }
		, RandomWeights { }
		, OpenWhenDestoryed { false }
		, CommonData { AffectWho::MASTER }
	{ }

	Valueable<bool> Enable;
	ValueableVector<TechnoTypeClass*> Gifts;
	std::vector<int> Nums;
	ValueableVector<double> Chances;
	Valueable<bool> Remove;
	Valueable<bool> Destroy;
	Valueable<int> Delay;
	Nullable<Point2D> RandomDelay;
	Valueable<int> RandomRange;
	Valueable<bool> EmptyCell;
	Valueable<bool> RandomType;
	ValueableVector<int> RandomWeights;
	Valueable<bool> OpenWhenDestoryed;
	CommonProperties CommonData;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Enable)
			.Process(Gifts)
			.Process(Nums)
			.Process(Chances)
			.Process(Remove)
			.Process(Destroy)
			.Process(Delay)
			.Process(RandomDelay)
			.Process(RandomRange)
			.Process(EmptyCell)
			.Process(RandomType)
			.Process(RandomWeights)
			.Process(OpenWhenDestoryed)
			;

		CommonData.Serialize(Stm);
	}
};
