#include "RandomWeights.h"
#include <Utilities/GeneralUtils.h>

template <typename T>
bool  RandomWeights::Serialize(T& Stm)
{
	return Stm
		.Process(this->RollChances)
		.Process(this->Weights);
}

bool RandomWeights::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return this->Serialize(Stm);
}

bool RandomWeights::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<RandomWeights*>(this)->Serialize(Stm);
}

void RandomWeights::Read(INI_EX& exINI, const char* pSection, std::string baseFlag)
{
	std::string rollChancesFlag = baseFlag;
	rollChancesFlag += ".RollChances";
	this->RollChances.Read(exINI, pSection, rollChancesFlag.c_str());

	std::string randomWeightFlag = baseFlag;
	randomWeightFlag += ".RandomWeights";

	this->Weights.clear();

	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> rowWeights {};
		rowWeights.Read(exINI, pSection, (randomWeightFlag + std::to_string(i)).c_str());

		if (!rowWeights.size())
			break;

		this->Weights.emplace_back(std::move(rowWeights));
	}

	ValueableVector<int> fallbackWeights {};
	fallbackWeights.Read(exINI, pSection, randomWeightFlag.c_str());

	if (fallbackWeights.size())
	{
		if (this->Weights.size())
			this->Weights[0] = std::move(fallbackWeights);
		else
			this->Weights.emplace_back(std::move(fallbackWeights));
	}
}

std::vector<int> RandomWeights::RollWeighted(int itemCount, Valueable<double>& rngOut)
{
	std::vector<int> nResult {};

	size_t rollsSize = this->RollChances.size();
	const size_t weightsSize = this->Weights.size();

	// Safety check: if weights is empty, return empty result
	if (weightsSize == 0 || itemCount == 0)
		return nResult;

	// If no RollChances are supplied, do only one roll with no chance filter
	const bool skipChanceFilter = (rollsSize == 0);
	if (skipChanceFilter)
		rollsSize = 1;

	for (size_t i = 0; i < rollsSize; i++)
	{
		rngOut = ScenarioClass::Instance->Random.RandomDouble();

		if (!skipChanceFilter && rngOut > Math::abs(this->RollChances[i]))
			continue;

		// If there are more rolls than weight lists, use the last weight list
		const size_t j = MinImpl(i, weightsSize - 1);
		const int index = GeneralUtils::ChooseOneWeighted(rngOut, this->Weights[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (index >= 0 && size_t(index) < size_t(itemCount))
			nResult.push_back(index);
	}

	return nResult;
}

bool RandomWeights::TryRollChance(int& outIndex, std::vector<double>& chances)
{
	size_t rollCount = this->RollChances.size();
	if (rollCount == 0)
		rollCount = 1;

	for (size_t i = 0; i < rollCount; i++)
	{
		const double dice = ScenarioClass::Instance->Random.RandomDouble();

		if (this->RollChances.size() > 0 && dice > this->RollChances[i])
			continue;

		const size_t weightIndex = MinImpl(i, this->Weights.size() - 1);
		const int selectedIndex = GeneralUtils::ChooseOneWeighted(dice, this->Weights[weightIndex]);

		bool found = true;
		const size_t chanceCount = chances.size();

		if (chanceCount > 0)
		{
			const double chanceDice = ScenarioClass::Instance->Random.RandomDouble();
			const double threshold = (chanceCount > size_t(selectedIndex))
				? chances[selectedIndex]
				: chances[chanceCount - 1];

			found = threshold >= chanceDice;
		}

		if (found)
		{
			outIndex = selectedIndex;
			return true;
		}
	}

	return false;
}