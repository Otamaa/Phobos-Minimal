#include <Utilities/Debug.h>

#include <Ext/Rules/Body.h>
/***
 * the following hooks replace the original checks that disable certain visual
 * effects when the frame rate drops below a certain limit. the issue with them
 * was that they only take into account the settings from the rulesmd.ini, but
 * ignore the game speed setting. that means if the frame rate is supposed to
 * be 20 frames or less (DetailMinFrameRateNormal=15, DetailBufferZoneWidth=5),
 * then the effects are still disabled. lasers draw as ugly lines, non-damaging
 * particles don't render, spotlights aren't created, ...
 ***/

DEFINE_OVERRIDE_HOOK(0x48A634, FlashbangWarheadAt_Details, 5)
{
	const auto details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x48A64Au : 0x48A641u;
}

DEFINE_OVERRIDE_HOOK(0x5FF86E, SpotlightClass_Draw_Details, 5)
{
	const auto details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x5FF87Fu : 0x5FFF77u;
}

DEFINE_OVERRIDE_HOOK(0x422FCC, AnimClass_Draw_Details, 5)
{
	const auto details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x422FECu : 0x422FD9u;
}

DEFINE_OVERRIDE_HOOK(0x550BCA, LaserDrawClass_Draw_InHouseColor_Details, 5)
{
	const auto details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x550BD7u : 0x550BE5u;
}

DEFINE_OVERRIDE_HOOK(0x62CEC9, ParticleClass_Draw_Details, 5)
{
	const auto details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x62CEEAu : 0x62CED6u;
}

DEFINE_OVERRIDE_HOOK(0x6D7847, TacticalClass_DrawPixelEffects_Details, 5)
{
	const auto details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x6D7858u : 0x6D7BF2u;
}

DEFINE_OVERRIDE_HOOK(0x420F40, AlphaShapeClass_DrawAll_Details, 6)
{
	const auto details = RulesExt::DetailsCurrentlyEnabled();
	return details ? 0x0u : 0x421346u;
}