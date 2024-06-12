#pragma once

#include <map>
#include <list>
#include <string>

#include <Misc/DynamicPatcher/Helpers/MyDelegate.h>

struct IStream;

class EventSystem;
class Event;

extern void* EventArgsLate;
extern void* EventArgsEmpty;

typedef void (*HandleEvent)(EventSystem*, Event, void*);

// 事件类型
class Event
{
public:
	auto operator <=>(const Event&) const = default;

	std::string Name;
	std::string Dest;
};

// 事件管理器
class EventSystem
{
public:

	EventSystem(const char* name) :
		Name { name } , _handlers {}
	{ }

	~EventSystem() = default;

	void AddHandler(Event e, HandleEvent func) {
		this->_handlers[e] += Delegate::newDelegate(func);
	}

	template<typename T, typename F>
	void AddHandler(Event e, T* _obj, F func) {
		_handlers[e] += Delegate::newDelegate(_obj, func);
	}

	void RemoveHandler(Event e, HandleEvent func) {
		this->_handlers[e] -= Delegate::newDelegate(func);
	}

	template<typename T, typename F>
	void RemoveHandler(Event e, T* _obj, F func) {
		_handlers[e] -= Delegate::newDelegate(_obj, func);
	}

	void Broadcast(Event e, void* args = EventArgsEmpty) {
		this->_handlers[e](this, e, args);
	}

	std::string Name;
private:
	std::map<Event, Delegate::CMultiDelegate<void, EventSystem*, Event, void* >> _handlers;
};

//Global events
class EventSystems
{
public:
	// 事件管理器
	static EventSystem General;
	static EventSystem Render;
	static EventSystem Logic;
	static EventSystem SaveLoad;
};

//ObjectBased events
class Events
{
public:

	// 游戏进程事件
	static Event DetachAll;
	static Event ObjectUnInitEvent;
	static Event PointerExpireEvent;
	static Event ScenarioClearClassesEvent;
	static Event ScenarioStartEvent;
	// 渲染事件
	static Event GScreenRenderEvent;
	// 单位逻辑事件
	static Event LogicUpdateEvent;
	static Event TypeChangeEvent;
	// 游戏保存事件
	static Event SaveGameEvent;
	// 游戏读取事件
	static Event LoadGameEvent;
};

// 事件参数
class SaveLoadEventArgs
{
public:
	constexpr SaveLoadEventArgs(const char* fileName, bool isStart) : FileName { fileName }
		, Stream { nullptr }
		, isStart { isStart }
		, isStartInStream { false }
	{ }

	constexpr SaveLoadEventArgs(IStream* stream, bool isStart) : FileName { }
		, Stream { stream }
		, isStart { false }
		, isStartInStream { isStart }
	{ }

	constexpr bool InStream()
	{
		return Stream != nullptr;
	}

	constexpr bool IsStart()
	{
		return isStart;
	}

	constexpr bool IsEnd()
	{
		return !IsStart() && !InStream();
	}

	constexpr bool IsStartInStream()
	{
		return isStartInStream;
	}

	constexpr bool IsEndInStream()
	{
		return !IsStartInStream() && InStream();
	}

	std::string FileName;
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
