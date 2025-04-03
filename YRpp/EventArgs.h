#pragma once

#include <GeneralDefinitions.h>
#include <Timers.h>

class AbstractClass;
class ObjectClass;
class HouseClass;
struct EventArgs
{
	TriggerEvent EventType;
	HouseClass* Owner;
	ObjectClass* Object;
	CDTimerClass* ActivationFrame;
	bool* isRepeating;
	AbstractClass* Source;
};
