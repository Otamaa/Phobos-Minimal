#include "DebugLog.h"

#include <Utilities/Debug.h>

void GameDebugLog::HookLogEnd(DWORD addr , const char* pFormat, size_t size , long long time)
{

	if (!Debug::LogFileActive())
		return;

	const int time_i = (int)time;

	if(time_i > 0){
		fprintf(Debug::LogFile, "[%x] %s [%d] , Spend %d s\n", addr , pFormat , size , time_i);
		Debug::Flush();
	}

}

void GameDebugLog::HookLogStart(DWORD addr, const char* pFormat, size_t size)
{
	if (!Debug::LogFileActive())
		return;

	fprintf(Debug::LogFile, "[%x] %s [%d] , Start\n", addr, pFormat, size);
	Debug::Flush();
}