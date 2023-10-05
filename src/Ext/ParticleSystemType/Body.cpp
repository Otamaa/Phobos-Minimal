#include "Body.h"

void ParticleSystemTypeExt::ExtData::Initialize() {
}

void ParticleSystemTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	const char* pID = this->Get()->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);

	this->ApplyOptimization.Read(exINI, pID, "ApplyOptimization");
}

// =============================
// load / save
template <typename T>
void ParticleSystemTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->ApplyOptimization)
		;
}

// =============================
// container
ParticleSystemTypeExt::ExtContainer ParticleSystemTypeExt::ExtMap;
ParticleSystemTypeExt::ExtContainer::ExtContainer() : Container("ParticleSystemTypeClass") {}
ParticleSystemTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x644217, ParticleSystemTypeClass_CTOR, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	ParticleSystemTypeExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x644276, ParticleSystemTypeClass_SDDTOR, 0x6)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	ParticleSystemTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x644830, ParticleSystemTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6447E0, ParticleSystemTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleSystemTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleSystemTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x64481F, ParticleSystemTypeClass_Load_Suffix, 0x6)
{
	ParticleSystemTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x644844, ParticleSystemTypeClass_Save_Suffix, 0x5)
{
	ParticleSystemTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x644620, ParticleSystemTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x644617, ParticleSystemTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleSystemTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);

	ParticleSystemTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x644620);
	return 0;
}