#pragma once

#include "TemplateDef.h"

struct RandomWeights
{
	void Read(INI_EX& exINI, const char* pSection, std::string baseFlag);

	std::vector<int> RollWeighted(int max, Valueable<double>& RandomBuffer);
	bool TryRollChance(int& outIndex, std::vector<double>& chances);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool  Serialize(T& Stm);

public:
	ValueableVector<float> RollChances {};
	std::vector<std::vector<int>> Weights {};
};
