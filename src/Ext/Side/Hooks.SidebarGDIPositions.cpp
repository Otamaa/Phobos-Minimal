#include <ScenarioClass.h>
#include "Body.h"

bool SideExt::isNODSidebar()
{
	auto const PlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (auto const pSide = SideClass::Array->GetItemOrDefault(PlayerSideIndex)) {
		if (auto const pData = SideExt::ExtMap.Find(pSide)) {
			return !pData->Sidebar_GDIPositions;
		}
	}
	return PlayerSideIndex;
}

DEFINE_HOOK(0x6A5090, RadarClass_InitForHouse, 0x5)
{
	R->EAX(SideExt::isNODSidebar());
	return 0x6A509B;
}

DEFINE_HOOK(0x652EAB, SidebarClass_InitPositions, 0x6)
{
	R->EAX(SideExt::isNODSidebar());
	return 0x652EB7;
}

DWORD& SidebarClass__OBJECT_HEIGHT = *reinterpret_cast<DWORD*>(0xB0B500);

DEFINE_HOOK(0x6A51E9, SidebarClass_InitGUI, 0x6)
{
	SidebarClass__OBJECT_HEIGHT = 0x32;
	const bool pos = SideExt::isNODSidebar();
	R->ESI(pos);
	R->EDX(pos);
	return 0x6A5205;
}

// PowerBar Positions
DEFINE_HOOK(0x63FB5D, PowerClass_DrawIt, 0x6)
{
	R->EAX(SideExt::isNODSidebar());
	return 0x63FB63;
}

// PowerBar Tooltip Positions
DEFINE_HOOK(0x6403DF, PowerClass_InitGUI, 0x6)
{
	R->ESI(SideExt::isNODSidebar());
	return 0x6403E5;
}
