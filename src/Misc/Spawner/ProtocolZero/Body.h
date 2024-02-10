#pragma once

/*
* Copyright (c) 2012, 2013, 2014 Toni Spets <toni.spets@iki.fi>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include <GeneralDefinitions.h>

class EventClass;
class ProtocolZero
{
private:
	static constexpr int SendResponseTimeInterval = 30;

public:
	static constexpr int ResponseTime2 = 0x30;

#pragma pack(push, 1)
	struct NetData {
		EventType Type;
		bool IsExecuted;
		char HouseIndex;
		uint32_t Frame;
		
		union
		{
			char DataBuffer[104];

			struct ResponseTime2
			{
				char MaxAhead;
				uint8_t LatencyLevel;
			} ResponseTime2;
		};
	};
#pragma pack(pop)


	static bool Enable;
	static unsigned char MaxLatencyLevel;
	static int WorstMaxAhead;

	static void SendResponseTime2();
	static void HandleResponseTime2(EventClass* event);

	static inline constexpr EventType GetEventType() {
		return static_cast<EventType>(ProtocolZero::ResponseTime2);
	}
};

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

	static int GetMaxAhead(LatencyLevelEnum latencyLevel);
	static const wchar_t* GetLatencyMessage(LatencyLevelEnum latencyLevel);
	static LatencyLevelEnum FromResponseTime(uint8_t rspTime);
};
