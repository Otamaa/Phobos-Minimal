#pragma once
#include <Utilities/TemplateDef.h>

class TechnoTypeClass;
class GiftBoxData
{
public:

	Valueable<bool> Enable { false };
	ValueableVector<TechnoTypeClass*> Gifts { };
	std::vector<int> Nums { };
	ValueableVector<int> RandomWeights { };
	ValueableVector<double> Chances { };
	Valueable<bool> UseChancesAndWeight { false };
	Valueable<bool> Remove { true };
	Valueable<bool> Destroy { false };
	Valueable<int> Delay { 0 };
	Valueable<int> DelayMin { 0 };
	Valueable<int> DelayMax { 0 };
	Valueable<int> RandomRange { 0 };
	Valueable<bool> EmptyCell { false };
	Valueable<bool> RandomType { false };
	Valueable<bool> OpenWhenDestoryed { false };
	Nullable<PartialVector2D<int>> RandomDelay { };
	Nullable<double> OpenWhenHealthPercent { };

	Valueable<bool> CheckPathfind { false };

	void Read(INI_EX& parser, const char* pSection);

	template <typename T>
	void Serialize(T& Stm)
	{
		//Debug::LogInfo("Processing Element From GiftBoxData ! ");

		Stm
			.Process(Enable)
			.Process(Gifts)
			.Process(Nums)
			.Process(RandomWeights)
			.Process(Remove)
			.Process(Destroy)
			.Process(Delay)
			.Process(DelayMin)
			.Process(DelayMax)
			.Process(RandomRange)
			.Process(EmptyCell)
			.Process(RandomType)
			.Process(OpenWhenDestoryed)
			.Process(RandomDelay)
			.Process(OpenWhenHealthPercent)
			.Process(CheckPathfind)
			;

		//Stm.RegisterChange(this);
	}
};
