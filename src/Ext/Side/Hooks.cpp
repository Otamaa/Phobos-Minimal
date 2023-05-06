#include <ScenarioClass.h>
#include <HouseClass.h>
#include <ThemeClass.h>

#include "Body.h"

// ingame music switch when defeated
DEFINE_HOOK_AGAIN(0x4FCB7D, HouseClass_WinLose_Theme, 0x5)
DEFINE_HOOK(0x4FCD66, HouseClass_WinLose_Theme, 0x5)
{
	const HouseClass* pThis = HouseClass::CurrentPlayer;

	if (const auto pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex)) {
		const auto pData = SideExt::ExtMap.Find(pSide);
		const auto themeIndex = (pThis->IsWinner) ? pData->IngameScore_WinTheme : pData->IngameScore_LoseTheme;

		if (themeIndex >= 0) {
			ThemeClass::Instance->Play(themeIndex);
		}
	}

	return 0;
}
