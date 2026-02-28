#include "Common.h"

#include <Misc/Kratos/Common/EventSystems/EventSystem.h>

char KratosCommon::readBuffer[KratosCommon::readLength];
wchar_t KratosCommon::wideBuffer[KratosCommon::readLength];
const char KratosCommon::readDelims[4] = ",";

bool KratosCommon::IsScenarioClear = false;

void KratosCommon::CmdLineParse(EventSystem* sender, Event e, void* args)
{
	auto const& argsArray = reinterpret_cast<void**>(args);
	char** ppArgs = (char**)argsArray[0];
	int nNumArgs = (int)argsArray[1];
}

void KratosCommon::ExeRun(EventSystem* sender, Event e, void* args)
{

}

void KratosCommon::ExeTerminate(EventSystem* sender, Event e, void* args)
{

}