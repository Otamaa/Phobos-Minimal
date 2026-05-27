#include "Body.h"

#include <GameOptionsClass.h>

#include <Ext/WarheadType/Body.h>
#include <Helpers/Macro.h>

#include <Ext/Rules/Body.h>

namespace LightEffectsTemp
{
	bool AlphaIsLightFlash = false;
}

ASMJIT_PATCH(0x48A444, AreaDamage_Particle_LightFlashSet, 0x5)
{
	GET(WarheadTypeClass*, pWH, EDI);

	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->Particle_AlphaImageIsLightFlash.Get(RulesExtData::Instance()->WarheadParticleAlphaImageIsLightFlash))
		LightEffectsTemp::AlphaIsLightFlash = true;

	return 0;
}

ASMJIT_PATCH(0x48A47E, AreaDamage_Particle_LightFlashUnset, 0x6)
{
	LightEffectsTemp::AlphaIsLightFlash = false;

	return 0;
}

ASMJIT_PATCH(0x5F5053, ObjectClass_Unlimbo_AlphaImage, 0x6)
{
	enum { SkipAlphaImage = 0x5F514B };

	int detailLevel = 0;

	if (LightEffectsTemp::AlphaIsLightFlash)
	{
		if (Phobos::Config::HideLightFlashEffects)
			return SkipAlphaImage;

		detailLevel = RulesExtData::Instance()->LightFlashAlphaImageDetailLevel;
	}

	if (detailLevel > GameOptionsClass::Instance->DetailLevel)
		return SkipAlphaImage;

	return 0;
}

ASMJIT_PATCH(0x48A62E, DoFlash_CombatLightOptions, 0x6)
{
	enum { Continue = 0x48A668, SkipFlash = 0x48A6FA };

	if (Phobos::Config::HideLightFlashEffects)
		return SkipFlash;

	GET(int, currentDetailLevel, EAX);
	GET(WarheadTypeClass*, pWH, EDI);
	GET(const int, damage, ECX);
	GET_STACK(bool, forceshow, 0xC + 0x10);
	GET(const int, bitmask, EBX);
	GET_STACK(const bool, forced, STACK_OFFSET(0xC, 0x10));

	R->ESI(damage); // Restore overridden instructions.

	bool checkColored = RulesExtData::Instance()->CombatLightDetailLevel_CheckColored;
	int detailLevel =RulesExtData::Instance()->CombatLightDetailLevel;

	if(pWH) {
		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

		if (pWHExt->CombatLightChance.isset() && pWHExt->CombatLightChance < Random2Class::Global->RandomDouble())
			return SkipFlash;

		detailLevel = pWHExt->CombatLightDetailLevel.Get(detailLevel);
		checkColored = pWHExt->CombatLightDetailLevel_CheckColored.Get(checkColored);

		if (pWHExt->CLIsBlack)
			R->EBX(SpotlightFlags::NoColor);
	}

	// (bitmask & 0xF) != 0) is true if any color channel is disabled.
	if (((detailLevel <= currentDetailLevel && RulesExtData::DetailsCurrentlyEnabled()) || (!checkColored && ((bitmask & 0xF) != 0))) && (forced || (pWH && pWH->Bright)))\
		return Continue;

	return SkipFlash;
}