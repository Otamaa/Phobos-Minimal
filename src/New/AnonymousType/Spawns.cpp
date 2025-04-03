#include "Spawns.h"
#include <Ext/Anim/Body.h>

void Spawns::SpawnAnims(TechnoClass* pInvoker,
	HouseClass* pOwner,
	CoordStruct& where
)
{
	if (this->Anims.empty())
		return;

	int animCount = this->Anims.size();
	int numsCount = this->Nums.size();
	std::vector<AnimTypeClass*> selected;
	if (this->RandomType)
	{
		int times = 1;
		if (numsCount > 0)
		{
			times = 0;
			for (int& num : this->Nums)
			{
				times += num;
			}
		}

		int maxValue = 0;
		PhobosMap<Point2D, int> targetPad = GeneralUtils::MakeTargetPad(this->RandomWeights, animCount, maxValue);

		for (int i = 0; i < times; i++)
		{
			int index = GeneralUtils::Hit(targetPad, maxValue);

			if (GeneralUtils::Bingo(this->Chances, index))
			{
				selected.push_back(this->Anims[index]);
			}
		}
	}
	else
	{
		for (int index = 0; index < animCount; index++)
		{
			auto id = this->Anims[index];
			int times = 1;
			if (numsCount > 0 && index < numsCount)
			{
				times = this->Nums[index];
			}
			for (int i = 0; i < times; i++)
			{
				if (GeneralUtils::Bingo(this->Chances, index))
				{
					selected.push_back(id);
				}
			}
		}
	}

	for (auto selec : selected)
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(selec, where),
			pOwner,
			nullptr,
			pInvoker,
			false
		);
	}
}
