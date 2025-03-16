#include <ScenarioClass.h>
#include "Body.h"

ASMJIT_PATCH(0x6A5090, RadarClass_InitForHouse, 0x5)
{
	R->EAX(SideExtData::isNODSidebar());
	return 0x6A509B;
}

ASMJIT_PATCH(0x652EAB, SidebarClass_InitPositions, 0x6)
{
	R->EAX(SideExtData::isNODSidebar());
	return 0x652EB7;
}

ASMJIT_PATCH(0x6A51E9, SidebarClass_InitGUI, 0x6)
{
	SidebarClass::ObjectHeight = 0x32;
	const bool pos = SideExtData::isNODSidebar();
	R->ESI(pos);
	R->EDX(pos);
	return 0x6A5205;
}

// PowerBar Positions
ASMJIT_PATCH(0x63FB5D, PowerClass_DrawIt, 0x6)
{
	R->EAX(SideExtData::isNODSidebar());
	return 0x63FB63;
}

// PowerBar Tooltip Positions
ASMJIT_PATCH(0x6403DF, PowerClass_InitGUI, 0x6)
{
	R->ESI(SideExtData::isNODSidebar());
	return 0x6403E5;
}
