#include "Body.h"

#include <EventClass.h>

#include <Utilities/Debug.h>
#include <HouseClass.h>
#include <SessionClass.h>
#include <IPXManagerClass.h>

bool ProtocolZero::Enable = false;
int ProtocolZero::WorstMaxAhead = 24;
unsigned char ProtocolZero::MaxLatencyLevel = 0xff;

void ProtocolZero::SendResponseTime2()
{
	if (SessionClass::IsSingleplayer())
		return;

	static int NextSendFrame = 6 * SendResponseTimeInterval;
	int currentFrame = Unsorted::CurrentFrame;

	if (NextSendFrame >= currentFrame)
		return;

	const int ipxResponseTime = IPXManagerClass::Instance->ResponseTime();
	if (ipxResponseTime <= -1)
		return;

	ProtocolZero::NetData event;
	event.Type = ProtocolZero::GetEventType();
	event.HouseIndex = (char)HouseClass::CurrentPlayer->ArrayIndex;
	event.Frame = currentFrame + Game::Network::MaxAhead;
	event.ResponseTime2.MaxAhead = (int8_t)ipxResponseTime + 1;
	event.ResponseTime2.LatencyLevel = (uint8_t)LatencyLevel::FromResponseTime((uint8_t)ipxResponseTime);
	static_assert(sizeof(NetData) == sizeof(EventClass), "Invalid Size!");

	if (EventClass::AddEvent(reinterpret_cast<EventClass*>(&event)))
	{
		NextSendFrame = currentFrame + SendResponseTimeInterval;
		Debug::Log("[Spawner] Player %d sending response time of %d, LatencyMode = %d, Frame = %d\n"
			, event.HouseIndex
			, event.ResponseTime2.MaxAhead
			, event.ResponseTime2.LatencyLevel
			, currentFrame
		);
	}
	else
	{
		++NextSendFrame;
	}
}

void ProtocolZero::HandleResponseTime2(EventClass* event)
{
	if (ProtocolZero::Enable == false || SessionClass::IsSingleplayer())
		return;

	ProtocolZero::NetData* netData = reinterpret_cast<ProtocolZero::NetData*>(event);

	if (netData->ResponseTime2.MaxAhead == 0)
	{
		Debug::Log("[Spawner] Returning because event->MaxAhead == 0\n");
		return;
	}

	static int32_t PlayerMaxAheads[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static uint8_t PlayerLatencyMode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static int32_t PlayerLastTimingFrame[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	int32_t houseIndex = netData->HouseIndex;
	PlayerMaxAheads[houseIndex] = (int32_t)netData->ResponseTime2.MaxAhead;
	PlayerLatencyMode[houseIndex] = netData->ResponseTime2.LatencyLevel;
	PlayerLastTimingFrame[houseIndex] = netData->Frame;

	uint8_t setLatencyMode = 0;
	int maxMaxAheads = 0;

	for (char i = 0; i < (char)std::size(PlayerMaxAheads); ++i)
	{
		if (Unsorted::CurrentFrame >= (PlayerLastTimingFrame[i] + (SendResponseTimeInterval * 4)))
		{
			PlayerMaxAheads[i] = 0;
			PlayerLatencyMode[i] = 0;
		}
		else
		{
			maxMaxAheads = PlayerMaxAheads[i] > maxMaxAheads ? PlayerMaxAheads[i] : maxMaxAheads;
			if (PlayerLatencyMode[i] > setLatencyMode)
				setLatencyMode = PlayerLatencyMode[i];
		}
	}

	ProtocolZero::WorstMaxAhead = maxMaxAheads;
	LatencyLevel::Apply(setLatencyMode);
}

LatencyLevelEnum LatencyLevel::CurentLatencyLevel = LatencyLevelEnum::LATENCY_LEVEL_INITIAL;
unsigned char LatencyLevel::NewFrameSendRate = 3;

void LatencyLevel::Apply(LatencyLevelEnum newLatencyLevel)
{
	if (newLatencyLevel > LatencyLevelEnum::LATENCY_LEVEL_MAX)
		newLatencyLevel = LatencyLevelEnum::LATENCY_LEVEL_MAX;

	auto maxLatencyLevel = static_cast<LatencyLevelEnum>(ProtocolZero::MaxLatencyLevel);
	if (newLatencyLevel > maxLatencyLevel)
		newLatencyLevel = maxLatencyLevel;

	if (newLatencyLevel <= CurentLatencyLevel)
		return;

	Debug::Log("[Spawner] Player %ls, Loss mode (%d, %d) Frame = %d\n"
		, HouseClass::CurrentPlayer->UIName
		, newLatencyLevel
		, CurentLatencyLevel
		, (int)Unsorted::CurrentFrame
	);

	CurentLatencyLevel = newLatencyLevel;
	NewFrameSendRate = static_cast<unsigned char>(newLatencyLevel);
	Game::Network::PreCalcFrameRate = 60;
	Game::Network::PreCalcMaxAhead = GetMaxAhead(newLatencyLevel);

	MessageListClass::Instance->PrintMessage(GetLatencyMessage(newLatencyLevel), 270, ColorScheme::White, true);
}

int LatencyLevel::GetMaxAhead(LatencyLevelEnum latencyLevel)
{
	const int maxAhead[] =
	{
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

	return maxAhead[(int)latencyLevel];
}

const wchar_t* LatencyLevel::GetLatencyMessage(LatencyLevelEnum latencyLevel)
{
	const wchar_t* message[] =
	{
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

	return message[(int)latencyLevel];
}

LatencyLevelEnum LatencyLevel::FromResponseTime(unsigned char rspTime)
{
	for (auto i = LatencyLevelEnum::LATENCY_LEVEL_1; i < LatencyLevelEnum::LATENCY_LEVEL_MAX; i = static_cast<LatencyLevelEnum>(1 + static_cast<char>(i)))
	{
		if (rspTime <= GetMaxAhead(i))
			return static_cast<LatencyLevelEnum>(i);
	}

	return LatencyLevelEnum::LATENCY_LEVEL_MAX;
}

// Otamaa : was 0x55DDA0 , but this conflict with Multithread stuffs
// so moved bit lowe , hopefully nothing bad happen here
DEFINE_HOOK(0x55DDAA, MainLoop_AfterRender_ProtocolZero, 0x5)
{
	if (ProtocolZero::Enable)
		ProtocolZero::SendResponseTime2();

	return 0;
}

DEFINE_HOOK(0x647BEB, QueueAIMultiplayer_ProtocolZero1, 0x9)
{
	if (ProtocolZero::Enable)
		return 0x647BF4;

	return (R->ESI() >= 5)
		? 0x647BF4
		: 0x647F36;
}

DEFINE_HOOK(0x647EB4, QueueAIMultiplayer_ProtocolZero2, 0x8)
{
	if (ProtocolZero::Enable)
	{
		R->AL(LatencyLevel::NewFrameSendRate);
		R->ECX((DWORD)Unsorted::CurrentFrame);

		return 0x647EBE;
	}

	return 0;
}

DEFINE_HOOK(0x647DF2, QueueAIMultiplayer_ProtocolZero3, 0x5)
{
	if (ProtocolZero::Enable)
	{
		R->EDX((DWORD)Game::Network::MaxAhead & 0xffff);

		return 0x647DF2 + 0x5;
	}

	return 0;
}

DEFINE_HOOK(0x4C8011, EventClassExecute_ProtocolZero, 0x8)
{
	if (ProtocolZero::Enable)
		return 0x4C8024;

	return 0;
}

DEFINE_HOOK(0x64C598, ExecuteDoList_ProtocolZero, 0x6)
{
	if (ProtocolZero::Enable)
	{
		auto dl = (uint8_t)R->DL();

		if (dl == (uint8_t)EventType::EMPTY)
			return 0x64C63D;

		if (dl == (uint8_t)EventType::PROCESS_TIME)
			return 0x64C63D;

		if (dl == (uint8_t)ProtocolZero::ResponseTime2)
			return 0x64C63D;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x64771D, QueueAIMultiplayer_ProtocolZero_SetTiming, 0x5)
DEFINE_HOOK(0x647E6B, QueueAIMultiplayer_ProtocolZero_SetTiming, 0x5)
{
	if (ProtocolZero::Enable)
	{
		GET(int, NewRetryDelta, EBP);
		GET(int, NewRetryTimeout, EAX);

		Debug::Log("[Spawner] NewRetryDelta = %d, NewRetryTimeout = %d, FrameSendRate = %d, CurentLatencyLevel = %d\n"
			, NewRetryDelta
			, NewRetryTimeout
			, (int)Game::Network::FrameSendRate
			, (int)LatencyLevel::CurentLatencyLevel
		);
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6476CB, QueueAIMultiplayer_ProtocolZero_ResponseTime, 0x5)
DEFINE_HOOK(0x647CC5, QueueAIMultiplayer_ProtocolZero_ResponseTime, 0x5)
{
	if (ProtocolZero::Enable)
	{
		R->EAX(ProtocolZero::WorstMaxAhead);
		return R->Origin() + 0x5;
	}

	return 0;
}