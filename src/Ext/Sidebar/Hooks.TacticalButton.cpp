#include "Body.h"

#include <Ext/SWType/Body.h>

#include <Misc/PhobosToolTip.h>

DEFINE_HOOK(0x692419, DisplayClass_ProcessClickCoords_TacticalButton, 0x7)
{
	return TacticalButtonClass::CurrentButton ? 0x6925FC : 0;
}

DEFINE_HOOK(0x4AE51E, DisplayClass_GetToolTip_TacticalButton, 0x6)
{
	enum { ApplyToolTip = 0x4AE69D };

	if (auto button = TacticalButtonClass::CurrentButton){
		PhobosToolTip::Instance.IsCameo = true;
		PhobosToolTip::Instance.HelpText(HouseClass::CurrentPlayer->Supers[button->SuperIndex]);
		R->EAX(PhobosToolTip::Instance.GetBuffer());
		return ApplyToolTip;
	}

	return 0;
}

DEFINE_HOOK(0x72426F, ToolTipManager_ProcessMessage_TacticalButton, 0x5)
{
	if (TacticalButtonClass::CurrentButton)
		R->EDX(0);

	return 0;
}

DEFINE_HOOK(0x72428C, ToolTipManager_ProcessMessage_TacticalButton2, 0x5)
{
	return TacticalButtonClass::CurrentButton ? 0x724297 : 0;
}

DEFINE_HOOK(0x724B2E, ToolTipManager_SetX_TacticalButtons, 0x6)
{
	if (const auto button = TacticalButtonClass::CurrentButton)
	{
		R->EDX(button->Rect.X + button->Rect.Width);
		R->EAX(button->Rect.Y + 27);
	}

	return 0;
}

DEFINE_HOOK(0x6A6300, SidebarClass_AddCameo_SuperWeapon_TacticalButton, 0x6)
{
	enum { SkipGameCode = 0x6A6606 };

	if (Phobos::UI::ExclusiveSuperWeaponSidebar)
	{
		GET_STACK(AbstractType, whatAmI, 0x4);
		GET_STACK(int, index, 0x8);

		switch (whatAmI)
		{
		case AbstractType::Super:
		case AbstractType::SuperWeaponType:
		case AbstractType::Special:
			if (const auto pSWType = SuperWeaponTypeClass::Array->GetItemOrDefault(index))
			{
				const auto pSWExt = SWTypeExtContainer::Instance.Find(pSWType);

				if (pSWExt->AllowInExclusiveSidebar && (pSWExt->SW_ShowCameo || !pSWExt->SW_AutoFire))
				{
					TacticalButtonClass::AddButton(index);
					R->AL(false);
					return SkipGameCode;
				}
			}
			break;

		default:
			break;
		}
	}

	return 0;
}

//DEFINE_HOOK(0x6A60A0, SidebarClass_StripClass_Redreaw, 0xA) {
//	GET_STACK(int, tab_, 0x4);
//
//	if (tab_ == 1){
//		TacticalButtonClass::Initialized = false;
//	}
//	return 0x0;
//}

DEFINE_HOOK(0x6A5030, SidebarClass_Init_Clear_InitializedTacticalButton, 0x6)
{
	TacticalButtonClass::Initialized = false;
	TacticalButtonClass::ClearButtons();
	return 0;
}

DEFINE_HOOK(0x55B6B3, LogicClass_AI_InitializedTacticalButton, 0x5)
{
	if (!TacticalButtonClass::Initialized) {
		TacticalButtonClass::Initialized = true;
		TacticalButtonClass::ClearButtons();
		const auto pCurrent = HouseClass::CurrentPlayer();

		if (!pCurrent || pCurrent->Defeated)
			return 0;

		for (const auto pSuper : pCurrent->Supers)
		{
			const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

			if (!pSuper->Granted || !pSWExt->IsAvailable(pCurrent))
				continue;


			if (pSWExt->AllowInExclusiveSidebar && (pSWExt->SW_ShowCameo || !pSWExt->SW_AutoFire)) {

				auto& buttons = TacticalButtonClass::Buttons;

				if (buttons.any_of([pSuper](TacticalButtonClass* const button) { return button->SuperIndex == pSuper->Type->ArrayIndex; }))
					continue;

				buttons.emplace_back(GameCreate<TacticalButtonClass>(pSuper->Type->ArrayIndex + 2200, pSuper->Type->ArrayIndex, 0, 0, 60, 48));
			}
		}

		TacticalButtonClass::SortButtons();
	}
	return 0;
}