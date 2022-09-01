#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

class TechnoTypeClass;
class GiftBoxData
{
public:

	Valueable<bool> Enable;
	ValueableVector<TechnoTypeClass*> Gifts;
	DynamicVectorClass<int> Nums;
	ValueableVector<int> RandomWeights;
	ValueableVector<double> Chances;
	Valueable<bool> UseChancesAndWeight;
	Valueable<bool> Remove;
	Valueable<bool> Destroy;
	Valueable<int> Delay;
	Valueable<int> DelayMin;
	Valueable<int> DelayMax;
	Valueable<int> RandomRange;
	Valueable<bool> EmptyCell;
	Valueable<bool> RandomType;
	Valueable<bool> OpenWhenDestoryed;
	Nullable<Point2D> RandomDelay;
	Nullable<double> OpenWhenHealthPercent;

	GiftBoxData()
		: Enable { false }
		, Gifts { }
		, Nums { }
		, RandomWeights { }
		, Chances { }
		, UseChancesAndWeight { false }
		, Remove { true }
		, Destroy { false }
		, Delay { 0 }
		, DelayMin { 0 }
		, DelayMax { 0 }
		, RandomRange { 0 }
		, EmptyCell { false }
		, RandomType { false }
		, OpenWhenDestoryed { false }
		, RandomDelay { }
		, OpenWhenHealthPercent { }
	{ }

	~GiftBoxData() = default;

	void Read(INI_EX& parser, const char* pSection)
	{
		Gifts.Read(parser, pSection, "GiftBox.Types");

		if (!Gifts.empty())
		{
			Enable = true;
			auto const nBaseSize = (int)Gifts.size();
			Nums.Clear();
			Nums.Reserve(nBaseSize);
			Nums.Count = nBaseSize;
			auto const pNumKey = "GiftBox.Nums";

			for (auto& nSpawnMult : Nums)
				nSpawnMult = 1;

			if (parser.ReadString(pSection, pNumKey))
			{
				int nCount = 0;
				char* context = nullptr;
				for (char* cur = CRT::strtok(parser.value(), Phobos::readDelims, &context); cur; cur = CRT::strtok(nullptr, Phobos::readDelims, &context))
				{

					int buffer = 1;
					if (Parser<int>::TryParse(cur, &buffer))
						Nums[nCount] = buffer;

					if (++nCount >= nBaseSize)
						break;
				}
			}

			UseChancesAndWeight.Read(parser, pSection, "GiftBox.UseChancesAndWeight");
			RandomWeights.Read(parser, pSection, "GiftBox.RandomWeights");
			Chances.Read(parser, pSection, "GiftBox.Chances");
			Remove.Read(parser, pSection, "GiftBox.Remove");
			Destroy.Read(parser, pSection, "GiftBox.Explodes");
			Delay.Read(parser, pSection, "GiftBox.Delay");
			RandomDelay.Read(parser, pSection, "GiftBox.RandomDelay");

			if (RandomDelay.isset() && (abs(DelayMax) > 0 || abs(DelayMin) > 0))
			{
				DelayMin = abs(RandomDelay.Get().X);
				DelayMax = abs(RandomDelay.Get().Y);

				if (DelayMin > DelayMax)
					std::swap(DelayMin, DelayMax);
			}

			RandomRange.Read(parser, pSection, "GiftBox.RandomRange");
			EmptyCell.Read(parser, pSection, "GiftBox.RandomToEmptyCell");
			RandomType.Read(parser, pSection, "GiftBox.RandomType");
			OpenWhenDestoryed.Read(parser, pSection, "GiftBox.OpenWhenDestoryed");
			OpenWhenHealthPercent.Read(parser, pSection, "OpenWhenHealthPercent");
		}
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Processing Element From GiftBoxData ! \n");

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
			;
	}
};
#endif