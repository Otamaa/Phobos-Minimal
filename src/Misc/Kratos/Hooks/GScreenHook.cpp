
#include <exception>

#include <Helpers/Macro.h>

#include <Misc/Kratos/Common/EventSystems/EventSystem.h>

#include <Misc/Kratos/Ext/Common/PrintTextManager.h>

class GScreenHook
{
public:
	GScreenHook()
	{
		EventSystems::Render.AddHandler(Events::GScreenRenderEvent, PrintTextManager::PrintRollingText);
	}
};

static GScreenHook _gScreenHook;

#ifndef _ENABLE_HOOKS

// SidebarClass_5F38C0
ASMJIT_PATCH(0x6A70EB, SidebarClass_DrawIt, 0x6)
{
	EventSystems::Render.Broadcast(Events::SidebarRenderEvent);
	return 0;
}
#endif