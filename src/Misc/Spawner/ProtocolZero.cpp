#include "ProtocolZero.h"

#include <EventClass.h>

#include <Utilities/Debug.h>
#include <HouseClass.h>
#include <SessionClass.h>
#include <IPXManagerClass.h>

#include <Ext/Event/Body.h>

#include <Helpers/Macro.h>

#pragma region defines
LatencyLevelEnum LatencyLevel::CurentLatencyLevel { LatencyLevelEnum::LATENCY_LEVEL_INITIAL };
uint8_t LatencyLevel::NewFrameSendRate { 3 };
static COMPILETIMEEVAL wchar_t* message[] = {
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

wchar_t* LatencyLevel::GetLatencyMessage(LatencyLevelEnum latencyLevel)
{
	return message[(int)latencyLevel];
}

#pragma endregion

void LatencyLevel::Apply(LatencyLevelEnum newLatencyLevel)
{
	if (newLatencyLevel > LatencyLevelEnum::LATENCY_LEVEL_MAX)
		newLatencyLevel = LatencyLevelEnum::LATENCY_LEVEL_MAX;

	auto maxLatencyLevel = static_cast<LatencyLevelEnum>(EventExt::ProtocolZero::MaxLatencyLevel);
	if (newLatencyLevel > maxLatencyLevel)
		newLatencyLevel = maxLatencyLevel;

	if (newLatencyLevel <= CurentLatencyLevel)
		return;

	Debug::LogInfo("[Spawner] Player {}, Loss mode ({}, {}) Frame = {}"
		, PhobosCRT::WideStringToString(HouseClass::CurrentPlayer->UIName)
		, (int)newLatencyLevel
		, (int)CurentLatencyLevel
		, (int)Unsorted::CurrentFrame()
	);

	CurentLatencyLevel = newLatencyLevel;
	NewFrameSendRate = static_cast<unsigned char>(newLatencyLevel);
	Game::Network::PreCalcFrameRate = 60;
	Game::Network::PreCalcMaxAhead = GetMaxAhead(newLatencyLevel);

	MessageListClass::Instance->PrintMessage(GetLatencyMessage(newLatencyLevel), (int)(RulesClass::Instance->MessageDelay * 900), ColorScheme::White, true);
}

// ASMJIT_PATCH(0x55DDA0, MainLoop_AfterRender_ProtocolZero, 0x5)
// {
//
//
// 	return 0;
// }

ASMJIT_PATCH(0x647BEB, QueueAIMultiplayer_ProtocolZero1, 0x9)
{
	if (EventExt::ProtocolZero::Enable)
		return 0x647BF4;

	return (R->ESI() >= 5)
		? 0x647BF4
		: 0x647F36;
}

ASMJIT_PATCH(0x647EB4, QueueAIMultiplayer_ProtocolZero2, 0x8)
{
	if (EventExt::ProtocolZero::Enable)
	{
		R->AL(LatencyLevel::NewFrameSendRate);
		R->ECX((DWORD)Unsorted::CurrentFrame);

		return 0x647EBE;
	}

	return 0;
}

ASMJIT_PATCH(0x647DF2, QueueAIMultiplayer_ProtocolZero3, 0x5)
{
	if (EventExt::ProtocolZero::Enable)
	{
		R->EDX((DWORD)Game::Network::MaxAhead & 0xffff);

		return 0x647DF2 + 0x5;
	}

	return 0;
}

ASMJIT_PATCH(0x4C8011, EventClassExecute_ProtocolZero_DisableGame, 0x8)
{
	return EventExt::ProtocolZero::Enable ? 0x4C8024 : 0;
}

ASMJIT_PATCH(0x64C598, ExecuteDoList_ProtocolZero_DisableLog, 0x6)
{
	enum { break_ = 0x64C63D };

	if (EventExt::ProtocolZero::Enable) {
		auto dl = (uint8_t)R->DL();

		if (dl == (uint8_t)EventType::EMPTY)
			return break_;

		if (dl == (uint8_t)EventType::PROCESS_TIME)
			return break_;
	}

	return 0;
}

ASMJIT_PATCH(0x647E6B, QueueAIMultiplayer_ProtocolZero_SetTiming, 0x5)
{
	if (EventExt::ProtocolZero::Enable)
	{
		GET(int, NewRetryDelta, EBP);
		GET(int, NewRetryTimeout, EAX);

		Debug::LogInfo("[Spawner] NewRetryDelta = {}, NewRetryTimeout = {}, FrameSendRate = {}, CurentLatencyLevel = {}"
			, NewRetryDelta
			, NewRetryTimeout
			, (int)Game::Network::FrameSendRate()
			, (int)LatencyLevel::CurentLatencyLevel
		);
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x64771D, QueueAIMultiplayer_ProtocolZero_SetTiming, 0x5)


ASMJIT_PATCH(0x647CC5, QueueAIMultiplayer_ProtocolZero_ResponseTime, 0x5)
{
	if (EventExt::ProtocolZero::Enable)
	{
		R->EAX(EventExt::ProtocolZero::WorstMaxAhead);
		return R->Origin() + 0x5;
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x6476CB, QueueAIMultiplayer_ProtocolZero_ResponseTime, 0x5)
