#include "Body.h"

#include <Ext/SWType/Body.h>

#include <Misc/PhobosToolTip.h>

#include <New/SuperWeaponSidebar/SWSidebarClass.h>
#include <New/SuperWeaponSidebar/ToggleSWButtonClass.h>

#include <New/MessageHandler/MessageColumnClass.h>

#pragma region NewButtonsRelated

ASMJIT_PATCH(0x692419, DisplayClass_ProcessClickCoords_SkipOnNewButtons, 0x7)
{
	enum { DoNothing = 0x6925FC };

	return (SWSidebarClass::IsEnabled() && SWSidebarClass::Global()->CurrentColumn
		|| SWSidebarClass::Global()->ToggleButton && SWSidebarClass::Global()->ToggleButton->IsHovering
		|| MessageColumnClass::Instance.IsBlocked())
		? DoNothing : 0;
}

ASMJIT_PATCH(0x6A5082, SidebarClass_InitClear_InitializeNewButtons, 0x5)
{
	SWSidebarClass::Global()->InitClear();
	MessageColumnClass::Instance.InitClear();
	return 0;
}

ASMJIT_PATCH(0x6A5839, SidebarClass_InitIO_InitializeNewButtons, 0x5)
{
	SWSidebarClass::Global()->InitIO();
	MessageColumnClass::Instance.InitIO();
	return 0;
}