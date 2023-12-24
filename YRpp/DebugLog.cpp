#include "DebugLog.h"

#include <Utilities/Debug.h>

void GameDebugLog::HookLogEnd(DWORD addr , const char* pFormat, size_t size , long long time)
{
#ifndef aaa
	if (!Debug::LogFileActive())
		return;
#endif
	const int time_i = (int)time;
#ifndef aaa
	if(time_i > 0){
		fprintf(Debug::LogFile, "[%x] %s [%d] , Spend %d s\n", addr , pFormat , size , time_i);
		Debug::Flush();
	}
#else
	//if (time_i > 0)
		//GameDebugLog::Log("[%x] %s [%d] , Spend %d s frame %d\n", addr, pFormat, size, time_i , Unsorted::CurrentFrame());
#endif
}

void GameDebugLog::HookLogStart(DWORD addr, const char* pFormat, size_t size)
{
	//if (!Debug::LogFileActive())
	//	return;
	//
	//fprintf(Debug::LogFile, "[%x] %s [%d] , Start\n", addr, pFormat, size);
	//Debug::Flush();
}