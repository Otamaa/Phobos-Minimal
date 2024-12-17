#include "Body.h"

#include <Ext/SWType/Body.h>

#include <Misc/PhobosToolTip.h>

DEFINE_HOOK(0x6A5030, SidebarClass_Init_Clear_InitializedTacticalButton, 0x6)
{
	SWButtonClass::Initialized = false;
	SWButtonClass::ClearButtons();
	return 0;
}

DEFINE_HOOK(0x55B6B3, LogicClass_AI_InitializedTacticalButton, 0x5)
{
	if (!SWButtonClass::Initialized) {
		SWButtonClass::Initialized = true;
		SWButtonClass::ClearButtons();
		const auto pCurrent = HouseClass::CurrentPlayer();

		if (!pCurrent || pCurrent->Defeated)
			return 0;

		for (const auto pSuper : pCurrent->Supers)
		{
			const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

			if (!pSuper->Granted || !pSWExt->IsAvailable(pCurrent))
				continue;


			if (pSWExt->AllowInSuperWeaponSidebar && (pSWExt->SW_ShowCameo || !pSWExt->SW_AutoFire)) {

				auto& buttons = SWButtonClass::Buttons;

				if (buttons.any_of([pSuper](SWButtonClass* const button) { return button->SuperIndex == pSuper->Type->ArrayIndex; }))
					continue;

				GameCreate<SWButtonClass>(pSuper->Type->ArrayIndex + 2200, pSuper->Type->ArrayIndex, 0, 0, 60, 48);
			}
		}

		SWButtonClass::SortButtons();
	}
	return 0;
}