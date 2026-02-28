#pragma once

#include <Misc/Kratos/Common.h>
#include <Misc/Kratos/Common/EventSystems/EventSystem.h>

class Kratos
{
public:

	static void ExeRun(EventSystem* sender, Event e, void* args);

};

void ExtTypeRegistryClear(EventSystem* sender, Event e, void* args);
void DetachAll(EventSystem* sender, Event e, void* args);
void InvalidatePointer(EventSystem* sender, Event e, void* args);