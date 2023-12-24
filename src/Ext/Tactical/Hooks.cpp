#include "Body.h"
#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x6D4CD9, PrintTimerOnTactical_BlinkColor, 0x6)
{
	R->EDI(ColorScheme::Array->Items[RulesExtData::Instance()->TimerBlinkColorScheme]);
	return 0x6D4CE2;
}