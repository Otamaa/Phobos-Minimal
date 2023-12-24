#include "Syringe.h"

#ifdef DEBUG_HOOK
std::chrono::steady_clock::time_point DebugData::StartTime;

void DebugData::Start(DWORD origin, const char* funcName, int size)
{
	GameDebugLog::HookLogStart(origin, funcName, size);
	DebugData::StartTime = std::chrono::high_resolution_clock::now();
}

void DebugData::End(DWORD origin, const char* funcName, int size)
{
	auto stop = std::chrono::high_resolution_clock::now();
	auto ret = std::chrono::duration_cast<std::chrono::microseconds>(StartTime - stop);
	const auto nRes = abs(ret.count());

	GameDebugLog::HookLogEnd(origin, funcName, size, nRes);
}

void DebugData::StartO(DWORD origin, const char* funcName, int size)
{
	GameDebugLog::HookLogStart(origin, funcName, size);
	DebugData::StartTime = std::chrono::high_resolution_clock::now();
}

void DebugData::EndO(DWORD origin, const char* funcName, int size)
{
	const auto nRes = abs(std::chrono::duration_cast<std::chrono::microseconds>
				(StartTime - std::chrono::high_resolution_clock::now()).count());

	GameDebugLog::HookLogEnd(origin, funcName, size, nRes);
}
#endif