#include <typeinfo>

#include "EventSystem.h"
#include <Utilities/Debug.h>

void* EventArgsLate = (void*)true;
void* EventArgsEmpty = nullptr;

// 事件管理器
EventSystem EventSystems::General("General"); // 全局事件
EventSystem EventSystems::Render("Render"); // 单位渲染事件
EventSystem EventSystems::Logic("Logic"); // 单位逻辑事件
EventSystem EventSystems::SaveLoad("SaveLoad"); // 存档事件

// 游戏事件
Event Events::DetachAll = Event("DetachAll", "Raised when an AbstractClass detach from all");
Event Events::ObjectUnInitEvent = Event("ObjectUnInit", "Raised when an ObjectClass is ready to delete");
Event Events::PointerExpireEvent = Event("AnnounceExpiredPointer", "Raised when an AbstractClass pointer expired");
Event Events::ScenarioClearClassesEvent = Event("ScenarioClearClasses", "Raised when scenario is cleaning classes");
Event Events::ScenarioStartEvent = Event("ScenarioStart", "Raised when scenario start");
// 渲染事件
Event Events::GScreenRenderEvent = Event("GScreenClass_Render", "Raised when GScreen is Render");
// 逻辑事件
Event Events::LogicUpdateEvent = Event("LogicClassUpdate", "Raised when LogicClass update");
Event Events::TypeChangeEvent = Event("TechnoClass_TypeChange", "Raised when Techno's type is changed");
// 存档事件
Event Events::SaveGameEvent = Event("SaveGame", "Raised when saving game");
Event Events::LoadGameEvent = Event("LoadGame", "Raised when loading game");