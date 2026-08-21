#pragma once

class SmudgeExtData
{
public:

	static bool ShouldRemoveSmudgeCell(const int index, const int time, const int current);
	static void UpdateSmudgeState();
};

