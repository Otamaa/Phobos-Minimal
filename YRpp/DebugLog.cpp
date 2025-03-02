#include "DebugLog.h"

#include <Utilities/Debug.h>

void GameDebugLog::HookLogEnd(DWORD addr , const char* pFormat, size_t size , long long time)
{

	//if (!Debug::LogFileActive())
	//	return;

	//const int time_i = (int)time;

	//if(time_i > 0){
	//	Debug::g_MainLogger->trace("[{0:x}] {1} [{2}] , Spend {3} s", addr, pFormat, size, time_i);
	//}

}

void GameDebugLog::HookLogStart(DWORD addr, const char* pFormat, size_t size)
{
	//if (!Debug::LogFileActive())
	//	return;

	//Debug::g_MainLogger->trace("[{0:x}] {1} [{2}] , Start", addr, pFormat, size);
}