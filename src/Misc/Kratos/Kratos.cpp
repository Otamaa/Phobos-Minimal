#include "Kratos.h"

#include <Utilities/Stream.h>

#include <AbstractClass.h>

void ExtTypeRegistryClear(EventSystem* sender, Event e, void* args)
{
}
void InvalidatePointer(EventSystem* sender, Event e, void* args)
{
}

void DetachAll(EventSystem* sender, Event e, void* args)
{
	auto const& argsArray = reinterpret_cast<void**>(args);
	AbstractClass* pItem = (AbstractClass*)argsArray[0];
	bool all = argsArray[1];
}

void Kratos::ExeRun(EventSystem* sender, Event e, void* args)
{
}
