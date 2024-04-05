#include "EventSystems.h"
#include "EventSystem.h"

EventSystem EventSystems::General("General");
EventSystem EventSystems::Render("Render");
EventSystem EventSystems::Logic("Logic");
EventSystem EventSystems::SaveLoad("SaveLoad");


Event Events::ExeRun = Event("ExeRun", "Raised when YR run");
Event Events::ExeTerminate = Event("ExeTerminate", "Raised when YR terminate");
Event Events::CmdLineParse = Event("CmdLineParse", "Raised when YR load cmd");
Event Events::DetachAll = Event("DetachAll", "Raised when an AbstractClass detach from all");
Event Events::ObjectUnInitEvent = Event("ObjectUnInit", "Raised when an ObjectClass is ready to delete");
Event Events::PointerExpireEvent = Event("AnnounceExpiredPointer", "Raised when an AbstractClass pointer expired");
Event Events::ScenarioClearClassesEvent = Event("ScenarioClearClasses", "Raised when scenario is cleaning classes");
Event Events::ScenarioStartEvent = Event("ScenarioStart", "Raised when scenario start");
Event Events::GScreenRenderEvent = Event("GScreenClass_Render", "Raised when GScreen is Render");
Event Events::SidebarRenderEvent = Event("SidebarClass_Draw_It", "Raised when Sidebar is Render");
Event Events::LogicUpdateEvent = Event("LogicClassUpdate", "Raised when LogicClass update");
Event Events::TypeChangeEvent = Event("TechnoClass_TypeChange", "Raised when Techno's type is changed");
Event Events::SaveGameEvent = Event("SaveGame", "Raised when saving game");
Event Events::LoadGameEvent = Event("LoadGame", "Raised when loading game");

SaveLoadEventArgs::SaveLoadEventArgs(const char* fileName, bool isStart)
{
	this->FileName = fileName;
	this->Stream = nullptr;
	this->isStart = isStart;
	this->isStartInStream = false;
}

SaveLoadEventArgs::SaveLoadEventArgs(IStream* stream, bool isStart)
{
	this->FileName = nullptr;
	this->Stream = stream;
	this->isStart = false;
	this->isStartInStream = isStart;
}

bool SaveLoadEventArgs::InStream()
{
	return Stream != nullptr;
}

bool SaveLoadEventArgs::IsStart()
{
	return isStart;
}

bool SaveLoadEventArgs::IsEnd()
{
	return !IsStart() && !InStream();
}

bool SaveLoadEventArgs::IsStartInStream()
{
	return isStartInStream;
}

bool SaveLoadEventArgs::IsEndInStream()
{
	return !IsStartInStream() && InStream();
}
