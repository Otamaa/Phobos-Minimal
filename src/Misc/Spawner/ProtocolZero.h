#pragma once

/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/
#include <GeneralDefinitions.h>

class EventClass;
enum class LatencyLevelEnum : uint8_t
{
	LATENCY_LEVEL_INITIAL = 0,

	LATENCY_LEVEL_1 = 1,
	LATENCY_LEVEL_2 = 2,
	LATENCY_LEVEL_3 = 3,
	LATENCY_LEVEL_4 = 4,
	LATENCY_LEVEL_5 = 5,
	LATENCY_LEVEL_6 = 6,
	LATENCY_LEVEL_7 = 7,
	LATENCY_LEVEL_8 = 8,
	LATENCY_LEVEL_9 = 9,

	LATENCY_LEVEL_MAX = LATENCY_LEVEL_9,
	LATENCY_SIZE = 1 + LATENCY_LEVEL_MAX
};

class LatencyLevel
{
public:
	static LatencyLevelEnum CurentLatencyLevel;
	static uint8_t NewFrameSendRate;

	static void Apply(LatencyLevelEnum newLatencyLevel);
	static void __forceinline Apply(uint8_t newLatencyLevel)
	{
		Apply(static_cast<LatencyLevelEnum>(newLatencyLevel));
	}

	static constexpr int maxAhead[]  = {
		/* 0 */ 1

		/* 1 */ ,4
		/* 2 */ ,6
		/* 3 */ ,12
		/* 4 */ ,16
		/* 5 */ ,20
		/* 6 */ ,24
		/* 7 */ ,28
		/* 8 */ ,32
		/* 9 */ ,36
	};

	static FORCEINLINE constexpr int GetMaxAhead(LatencyLevelEnum latencyLevel) {
		return maxAhead[(int)latencyLevel];
	}

	static constexpr wchar_t* message[] = {
		/* 0 */ L"CnCNet: Latency mode set to: 0 - Initial" // Players should never see this, if it doesn't then it's a bug

		/* 1 */ ,L"CnCNet: Latency mode set to: 1 - Best"
		/* 2 */ ,L"CnCNet: Latency mode set to: 2 - Super"
		/* 3 */ ,L"CnCNet: Latency mode set to: 3 - Excellent"
		/* 4 */ ,L"CnCNet: Latency mode set to: 4 - Very Good"
		/* 5 */ ,L"CnCNet: Latency mode set to: 5 - Good"
		/* 6 */ ,L"CnCNet: Latency mode set to: 6 - Good"
		/* 7 */ ,L"CnCNet: Latency mode set to: 7 - Default"
		/* 8 */ ,L"CnCNet: Latency mode set to: 8 - Default"
		/* 9 */ ,L"CnCNet: Latency mode set to: 9 - Default"
	};

	static FORCEINLINE constexpr wchar_t* GetLatencyMessage(LatencyLevelEnum latencyLevel) {
		return message[(int)latencyLevel];
	}

	static constexpr LatencyLevelEnum FromResponseTime(uint8_t rspTime)
	{
		for (auto i = LatencyLevelEnum::LATENCY_LEVEL_1; i < LatencyLevelEnum::LATENCY_LEVEL_MAX; i = static_cast<LatencyLevelEnum>(1 + static_cast<char>(i)))
		{
			if (rspTime <= GetMaxAhead(i))
				return static_cast<LatencyLevelEnum>(i);
		}

		return LatencyLevelEnum::LATENCY_LEVEL_MAX;
	}

};
