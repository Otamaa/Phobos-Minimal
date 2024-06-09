#pragma once
#include "EventSystem.h"
#include <Utilities/Savegame.h>

void* EventArgsLate = (void*)true;
void* EventArgsEmpty = nullptr;

class EventSystems
{
public:
	static EventSystem General;
	static EventSystem Render;
	static EventSystem Logic;
	static EventSystem SaveLoad;
};

class Events
{
public:
	static Event ExeRun;
	static Event ExeTerminate;
	static Event CmdLineParse;
	static Event DetachAll;
	static Event ObjectUnInitEvent;
	static Event PointerExpireEvent;
	static Event ScenarioClearClassesEvent;
	static Event ScenarioStartEvent;
	static Event GScreenRenderEvent;
	static Event SidebarRenderEvent;
	static Event LogicUpdateEvent;
	static Event TypeChangeEvent;
	static Event SaveGameEvent;
	static Event LoadGameEvent;
};

class SaveLoadEventArgs
{
public:
	SaveLoadEventArgs(const char* fileName, bool isStart);
	SaveLoadEventArgs(IStream* stream, bool isStart);

	bool InStream();
	bool IsStart();
	bool IsEnd();
	bool IsStartInStream();
	bool IsEndInStream();

	const char* FileName;
	IStream* Stream;
private:
	bool isStart;
	bool isStartInStream;
};

class SaveGameEventArgs : public SaveLoadEventArgs
{
public:
	using SaveLoadEventArgs::SaveLoadEventArgs;
};

class LoadGameEventArgs :public SaveLoadEventArgs
{
public:
	using SaveLoadEventArgs::SaveLoadEventArgs;
};

class TechnoClass;

class TypeChangeEventArgs
{
public:
	TechnoClass* pTechno;
	bool IsTransform;
};