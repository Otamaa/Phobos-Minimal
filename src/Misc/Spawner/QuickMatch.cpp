
#include <Utilities/Macro.h>
#include "Main.h"

namespace QuickMatch
{
	const wchar_t* PlayerString = L"Player";
}

ASMJIT_PATCH(0x643AA5, ProgressScreenClass_643720_HideName, 0x8)
{
	if ((SpawnerMain::Configs::Enabled && SpawnerMain::GetGameConfigs()->QuickMatch) == false)
		return 0;

	REF_STACK(wchar_t*, pPlayerName, STACK_OFFSET(0x5C, 8));
	pPlayerName = const_cast<wchar_t*>(QuickMatch::PlayerString);

	return 0;
}

ASMJIT_PATCH(0x65837A, RadarClass_658330_HideName, 0x6)
{
	if ((SpawnerMain::Configs::Enabled && SpawnerMain::GetGameConfigs()->QuickMatch) == false)
		return 0;

	R->ECX(QuickMatch::PlayerString);
	return 0x65837A + 0x6;
}

ASMJIT_PATCH(0x64B156, ModeLessDialog_64AE50_HideName, 0x9)
{
	if ((SpawnerMain::Configs::Enabled && SpawnerMain::GetGameConfigs()->QuickMatch) == false)
		return 0;

	R->EDX(QuickMatch::PlayerString);
	return 0x64B156 + 0x9;
}

ASMJIT_PATCH(0x648EA8, WaitForPlayers_HideName, 0x6)
{
	if ((SpawnerMain::Configs::Enabled && SpawnerMain::GetGameConfigs()->QuickMatch) == false)
		return 0;

	R->EAX(QuickMatch::PlayerString);
	return 0x648EB3;
}
