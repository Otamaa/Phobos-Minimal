#include <MessageListClass.h>
#include <WWMouseClass.h>

#include "Body.h"

ASMJIT_PATCH(0x4A8BCE, DisplayClass_Set_View_Dimensions, 0x5)
{
	if (Phobos::Config::MessageDisplayInCenter)
	{
		const auto& pScenarioExt = ScenarioExtData::Instance();

		if (!pScenarioExt->NewMessageList) // Load game
			pScenarioExt->NewMessageList = GameCreate<MessageListClass>();

		const auto& rect = DSurface::ViewBounds;
		const auto sideWidth = rect->Width / 6;
		const auto width = rect->Width - (sideWidth * 2);
		const auto pList = pScenarioExt->NewMessageList.get();

		// Except for X and Y, they are all original values
		pList->Init((rect->X + sideWidth), (rect->Height - rect->Height / 8 - 120), 6, 98, 18, -1, -1, 0, 20, 98, width);
		pList->SetWidth(width);
	}

	return 0;
}

ASMJIT_PATCH(0x684AD3, UnknownClass_sub_684620_InitMessageList, 0x5)
{
	if (Phobos::Config::MessageDisplayInCenter)
	{
		const auto& pScenarioExt = ScenarioExtData::Instance();

		if (!pScenarioExt->NewMessageList) // Start game
			pScenarioExt->NewMessageList = GameCreate<MessageListClass>();

		const auto& rect = DSurface::ViewBounds;
		const auto sideWidth = rect->Width / 6;
		const auto width = rect->Width - (sideWidth * 2);
		const auto pList = pScenarioExt->NewMessageList.get();

		// Except for X and Y, they are all original values
		pList->Init((rect->X + sideWidth), (rect->Height - rect->Height / 8 - 120), 6, 98, 18, -1, -1, 0, 20, 98, width);
	}

	return 0;
}
