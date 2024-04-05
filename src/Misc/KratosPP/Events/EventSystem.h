#pragma once

#include <compare>
#include <map>
#include <string>

#include <Misc/KratosPP/Utils/Delegate.h>

extern void* EventArgsLate;
extern void* EventArgsEmpty;

class Event
{
public:
	auto operator <=>(const Event&) const = default;

	std::string Name;
	std::string Dest;
};

class EventSystem
{
public:
	typedef void (*HandleEvent)(EventSystem*, Event, void*);

	EventSystem(const char* name)
	{
		this->Name = name;
	}

	void AddHandler(Event e, HandleEvent func)
	{
		this->_handlers[e] += Delegate::newDelegate(func);
	}

	template<typename T, typename F>
	void AddHandler(Event e, T* _obj, F func)
	{
		_handlers[e] += newDelegate(_obj, func);
	}

	void RemoveHandler(Event e, HandleEvent func)
	{
		this->_handlers[e] -= Delegate::newDelegate(func);
	}

	template<typename T, typename F>
	void RemoveHandler(Event e, T* _obj, F func)
	{
		_handlers[e] -= newDelegate(_obj, func);
	}

	void Broadcast(Event e, void* args = EventArgsEmpty)
	{
		this->_handlers[e](this, e, args);
	}

	std::string Name;
private:
	std::map<Event, Delegate::CMultiDelegate<void, EventSystem*, Event, void*>> _handlers;
};

