#pragma once

#include <Misc/Kratos/Common.h>
#include <Misc/Kratos/Common/EventSystems/EventSystem.h>

class AbstractClass;
class Kratos
{
public:

	static void ExeRun(EventSystem* sender, Event e, void* args);


	static void LogicUpdate_Early();
	static void LogicUpdate_End();
	static void EnsureSeeded(unsigned long seed);
	static void ScenarioClearFlagSet();
	static void ScenarioClearFlagUnset();
	static void DetachFromAll(AbstractClass* const pInvalid, bool const removed);
	static void GScreenRender_Early();
	static void GSCreenRender_End();
};

void ExtTypeRegistryClear(EventSystem* sender, Event e, void* args);
void DetachAll(EventSystem* sender, Event e, void* args);
void InvalidatePointer(EventSystem* sender, Event e, void* args);