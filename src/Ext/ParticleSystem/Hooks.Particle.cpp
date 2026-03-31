#include "Body.h"

#include <Ext/Particle/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>

#include <Notifications.h>
#include <Ext/Rules/Body.h>
#include <SpotlightClass.h>

ASMJIT_PATCH(0x6458D7, ParticleTypeClass_ReadINI_BehavesLike_B, 0x6)
{
	GET(const char*, pResult, EBX);

	for (size_t i = 0; i < ParticleTypeClass::BehavesString.c_size(); ++i)
	{
		if (IS_SAME_STR_(pResult, ParticleTypeClass::BehavesString[i]))
		{
			switch (i)
			{
			case 0:
				R->EDI(ParticleTypeBehavesLike::Gas);
				return 0x6458FF;
			case 1:
				R->EDI(ParticleTypeBehavesLike::Smoke);
				return 0x6458FF;
			case 2:
				R->EDI(ParticleTypeBehavesLike::Fire);
				return 0x6458FF;
			case 3:
				R->EDI(ParticleTypeBehavesLike::Spark);
				return 0x6458FF;
			case 4:
				R->EDI(ParticleTypeBehavesLike::Railgun);
				return 0x6458FF;
			default:
				break;
			}
		}
	}

	if (IS_SAME_STR_(pResult, "Web"))
	{
		R->EDI(ParticleTypeBehavesLike(5)); //result;
		return 0x6453FF;
	}

	R->EDI(ParticleTypeBehavesLike::None); //result;
	return 0x6458FF;
}

ASMJIT_PATCH(0x644857, ParticleSystemTypeClass_ReadINI_BehavesLike_B, 0x6)
{
	GET(const char*, pResult, EBX);

	for (size_t i = 0; i < ParticleSystemTypeClass::BehavesString.c_size(); ++i)
	{
		if (IS_SAME_STR_(pResult, ParticleSystemTypeClass::BehavesString[i]))
		{
			switch (i)
			{
			case 0:
				R->EDI(ParticleSystemTypeBehavesLike::Smoke);
				return 0x64487F;
			case 1:
				R->EDI(ParticleSystemTypeBehavesLike::Gas);
				return 0x64487F;
			case 2:
				R->EDI(ParticleSystemTypeBehavesLike::Fire);
				return 0x64487F;
			case 3:
				R->EDI(ParticleSystemTypeBehavesLike::Spark);
				return 0x64487F;
			case 4:
				R->EDI(ParticleSystemTypeBehavesLike::Railgun);
				return 0x64487F;
			default:
				break;
			}
		}
	}

	if (IS_SAME_STR_(pResult, "Web")) {
		R->EDI(ParticleSystemTypeBehavesLike(5)); //result;
		return 0x64487F;
	}

	R->EDI(ParticleSystemTypeBehavesLike::None); //result;
	return 0x64487F;
}

ASMJIT_PATCH(0x72590E, AnnounceInvalidPointer_Particle, 0x9)
{
	GET(AbstractType, nWhat, EBX);

	if (nWhat == AbstractType::Particle)
	{
		GET(ParticleClass*, pThis, ESI);

		if (auto pSys = pThis->ParticleSystem) {
			pSys->Particles.erase(pThis);
		}

		return 0x725C08;
	}

	return nWhat == AbstractType::ParticleSystem ?
		0x725917 : 0x7259DA;
}

ASMJIT_PATCH(0x62D015, ParticleClass_Draw_Palette, 6)
{
	GET(ParticleClass*, pThis, EDI);

	ConvertClass* pConvert = FileSystem::ANIM_PAL();
	const auto pTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);
	if (const auto pConvertData = pTypeExt->Palette.GetConvert()) {
		pConvert = pConvertData;
	}

	R->EDX(pConvert);
	return 0x62D01B;
}

ASMJIT_PATCH(0x6D9427, TacticalClass_DrawUnits_ParticleSystems, 9)
{
	GET(Layer, layer, EAX);

	if (layer == Layer::Air)
		ParticleSystemExtData::UpdateInAir();

	//return layer == Layer::Ground ? 0x6D9430 : 0x6D95A1;
	// Fixed position and layer of info tip and reveal production cameo on selected building
	// Part1
	// Author: Belonit
	return 0x6D95A1;
}

ASMJIT_PATCH(0x62E380, ParticleSystemClass_SpawnParticle, 0xA)
{
	GET(ParticleSystemClass*, pThis, ECX);

	return ParticleSystemExtContainer::Instance.Find(pThis)->What != ParticleSystemExtData::Behave::None
		? 0x62E428 : 0;
}

ASMJIT_PATCH(0x62E2AD, ParticleSystemClass_Draw, 6)
{
	GET(ParticleSystemClass*, pThis, EDI);
	GET(ParticleSystemTypeClass*, pThisType, EAX);

	if (pThisType->ParticleCap > 0)
	{
		R->ECX(pThis->Particles.Count +
			ParticleSystemExtContainer::Instance.Find(pThis)->OtherParticleData.size());
	}
	else
	{
		R->ECX(0);
	}

	return 0x62E2B3;
}

//donot detach the type so we can identify bug
DEFINE_JUMP(LJMP, 0x62E15D, 0x62E163);