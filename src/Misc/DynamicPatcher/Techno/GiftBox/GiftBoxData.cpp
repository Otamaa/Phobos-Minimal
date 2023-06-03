#include "GiftBoxData.h"
void GiftBoxData::Read(INI_EX& parser, const char* pSection)
{
	Gifts.Read(parser, pSection, "GiftBox.Types");

	if (!Gifts.empty())
	{
		Enable = true;
		auto const nBaseSize = (int)Gifts.size();
		Nums.clear();
		Nums.resize(nBaseSize);

		auto const pNumKey = "GiftBox.Nums";

		std::fill(Nums.begin(), Nums.end(), 1);

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
		OpenWhenHealthPercent.Read(parser, pSection, "GiftBox.OpenWhenHealthPercent");
		CheckPathfind.Read(parser, pSection, "GiftBox.ConsiderPathFinding");
	}
}
