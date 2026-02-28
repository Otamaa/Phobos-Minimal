#pragma once

class EventSystem;
class Event;
class KratosCommon
{
public:

	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static const char readDelims[4];

	static bool IsScenarioClear;

	static void CmdLineParse(EventSystem* sender, Event e, void* args);

	static void ExeRun(EventSystem* sender, Event e, void* args);
	static void ExeTerminate(EventSystem* sender, Event e, void* args);


};