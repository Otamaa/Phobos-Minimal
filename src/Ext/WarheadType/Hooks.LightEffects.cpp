#include <GameOptionsClass.h>

#include <Ext/WarheadType/Body.h>
#include <Helpers/Macro.h>

// DEFINE_HOOK(0x48A444 , AreaDamage_Particle_Handle , 0x5){
// 	GET(WarheadTypeClass*, pWH, EDI);
// 	GET_BASE(HouseClass* , pHouse , 0x14);
// 	GET(CoordStruct* , pCoord , ESI);
//
// 	auto pParticleSystem = GameCreate<ParticleSystemClass>(pWH->Parasite,pCoord,nullptr ,nullptr , &CoordStruct::Empty, pHouse);
// 	ParticleSystemExtContainer::Instance.Find(pParticleSystem)->
// }

namespace LightEffectsTemp
{
	bool AlphaIsLightFlash = false;
}

DEFINE_HOOK(0x48A444, AreaDamage_Particle_LightFlashSet, 0x5)
{
	GET(WarheadTypeClass*, pWH, EDI);

	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->Particle_AlphaImageIsLightFlash.Get(RulesExtData::Instance()->WarheadParticleAlphaImageIsLightFlash))
		LightEffectsTemp::AlphaIsLightFlash = true;

	return 0;
}

DEFINE_HOOK(0x48A47E, AreaDamage_Particle_LightFlashUnset, 0x6)
{
	LightEffectsTemp::AlphaIsLightFlash = false;

	return 0;
}

DEFINE_HOOK(0x5F5053, ObjectClass_Unlimbo_AlphaImage, 0x6)
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

DEFINE_HOOK(0x48A62E, DoFlash_CombatLightOptions, 0x6)
{
	enum { Continue = 0x48A668, SkipFlash = 0x48A6FA };

	if (Phobos::Config::HideLightFlashEffects)
		return SkipFlash;

	GET(int, currentDetailLevel, EAX);
	GET(WarheadTypeClass*, pWH, EDI);
	GET_STACK(bool, forceshow, 0xC + 0x10);
	//GET(DWORD, bitMask, EBX);

	R->ESI(R->ECX());

	const DWORD bit = R->BL();

	if (!pWH || (!forceshow && !pWH->Bright)) //check first requirements
		return SkipFlash;

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	const int detailLevel = pWHExt->CombatLightDetailLevel.Get(RulesExtData::Instance()->CombatLightDetailLevel);

	if ((detailLevel <= currentDetailLevel && RulesExtData::DetailsCurrentlyEnabled())
		|| (bit == 0xF)) //check detail level , FPS level  , and Bit
	{
		//check chance if set
		if (pWHExt->CombatLightChance.isset() && pWHExt->CombatLightChance < Random2Class::Global->RandomDouble())
			return SkipFlash;

		if (pWHExt->CLIsBlack)
			R->EBX(SpotlightFlags::NoColor);

		return Continue;
	}

	return SkipFlash;
}