#include "Body.h"

DEFINE_HOOK(0x644217, ParticleSystemTypeClass_CTOR, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	ParticleSystemTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x644276, ParticleSystemTypeClass_SDDTOR, 0x6)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	ParticleSystemTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x644830, ParticleSystemTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6447E0, ParticleSystemTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleSystemTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleSystemTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x64481F, ParticleSystemTypeClass_Load_Suffix, 0x6)
{
	ParticleSystemTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x644844, ParticleSystemTypeClass_Save_Suffix, 0x5)
{
	ParticleSystemTypeExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x644620, ParticleSystemTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x644617, ParticleSystemTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);

	ParticleSystemTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x644620);
	return 0;
}