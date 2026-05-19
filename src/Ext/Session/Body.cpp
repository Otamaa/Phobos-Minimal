#include "Body.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <Misc/Spawner/Main.h>

int FakeSessionClass::_Game_GetLinkedColor(PlayerColorSlot idx)
{
	int ret = 0;

	// Game_GetLinkedColor converts vanilla dropdown color index into color scheme index ([Colors] from rules)
	// if spawner feeds us a number, it will be used to look up color scheme directly
	// Original Author : Morton

	if (SpawnerMain::Configs::Enabled && Phobos::UI::UnlimitedColor && idx != PlayerColorSlot::Random)
	{
		ret = Math::abs((int)idx) << 1;
	}
	else
	{

		{
			// get the slot
			ColorData* slot = nullptr;
			if (idx == PlayerColorSlot::Random || (int)idx == Phobos::Config::colorCount)
			{
				// observer color
				slot = &Phobos::UI::Colors[0];
			}
			else if ((int)idx < Phobos::Config::colorCount)
			{
				// house color
				slot = &Phobos::UI::Colors[(int)idx + 1];
			}

			// retrieve the color scheme index

			if (slot) {
				if (slot->colorSchemeIndex == -1) {
					slot->colorSchemeIndex = ColorScheme::FindIndex(slot->colorScheme);

					if (slot->colorSchemeIndex == -1) {
						Debug::Log("Color scheme %s not found.\n", slot->colorScheme);
						slot->colorSchemeIndex = 4;
					}
				}

				ret = slot->colorSchemeIndex;
			}
		}
	}

	ret += 1;
	const int ColorShemeArrayCount = ColorScheme::Array->Count;

	if ((size_t)ret >= (size_t)ColorShemeArrayCount)
		Debug::FatalErrorAndExit("Trying To get Player Color[idx %d , %d(%d)] that more than ColorScheme Array Count [%d]!", idx, ret, ret - 1, ColorShemeArrayCount);

	return ret;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x69A310, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x48D961	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x4FC2C7	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x55E58D	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x55E900	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x55E95C	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x55E9CC	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x55EC0A	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x55EC6A	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x55ECAA	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x55F0DE	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x642BE1	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x642C0A	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x6880D2	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x6881D3	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x6DE0D9	, FakeSessionClass::_Game_GetLinkedColor);
DEFINE_FUNCTION_JUMP(CALL, 0x6E0D95	, FakeSessionClass::_Game_GetLinkedColor);