#include "Body.h"

ParticleTypeExt::ExtContainer ParticleTypeExt::ExtMap;

void ParticleTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->Get();
    const char* pID = this->Get()->ID;

	if (!pINI->GetSection(pID))
		return;

    INI_EX exINI(pINI);

	switch (pThis->BehavesLike)
	{
	case BehavesLike::Smoke:
	{
	//	this->DeleteWhenReachWater.Read(exINI, pID, "Smoke.DeleteWhenReachWater");
	}
	break;
	case BehavesLike::Fire:
	{
	//	this->ExpireAfterDamaging.Read(exINI, pID, "Fire.ExpireAfterDamaging");
	//	this->DamagingAnim.Read(exINI, pID, "Fire.DamagingAnim");
	}
	break;
	default:
		break;
	}

	this->LaserTrail_Types.Read(exINI, pID, "LaserTrail.Types");

#ifdef COMPILE_PORTED_DP_FEATURES
	this->Trails.Read(exINI, pID, false);
#endif
}

// =============================
// load / save
template <typename T>
void ParticleTypeExt::ExtData::Serialize(T& Stm)
{
    Stm
		.Process(this->LaserTrail_Types)
        ;

#ifdef COMPILE_PORTED_DP_FEATURES
	this->Trails.Serialize(Stm);
#endif
}

void ParticleTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
    Extension<ParticleTypeClass>::Serialize(Stm);
    this->Serialize(Stm);
}

void ParticleTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
    Extension<ParticleTypeClass>::Serialize(Stm);
    this->Serialize(Stm);
}

void ParticleTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {}

bool ParticleTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ParticleTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

ParticleTypeExt::ExtContainer::ExtContainer() : Container("ParticleTypeClass") {}
ParticleTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x644DBB, ParticleTypeClass_CTOR, 0x5)
{
    GET(ParticleTypeClass*, pItem, ESI);
    ParticleTypeExt::ExtMap.FindOrAllocate(pItem);

    return 0;
}

//ares 6A5A3B 7
DEFINE_HOOK(0x644E40, ParticleTypeClass_DTOR, 0x5)
{
	GET(ParticleTypeClass*, pItem, ECX);
	ParticleTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6457A0, ParticleTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x645660, ParticleTypeClass_SaveLoad_Prefix, 0x5)
{
    GET_STACK(ParticleTypeClass*, pItem, 0x4);
    GET_STACK(IStream*, pStm, 0x8);

    ParticleTypeExt::ExtMap.PrepareStream(pItem, pStm);

    return 0;
}

DEFINE_HOOK(0x64578C, ParticleTypeClass_Load_Suffix, 0x5)
{
    ParticleTypeExt::ExtMap.LoadStatic();
    return 0;
}

DEFINE_HOOK(0x64580A, ParticleTypeClass_Save_Suffix, 0x5)
{
	ParticleTypeExt::ExtMap.SaveStatic();
    return 0;
}

DEFINE_HOOK(0x645405, ParticleTypeClass_LoadFromINI, 0x5)
{
    GET(ParticleTypeClass*, pItem, ESI);
    GET_STACK(CCINIClass*, pINI, STACK_OFFS(0xDC , -0x4));

    ParticleTypeExt::ExtMap.LoadFromINI(pItem, pINI);
    return 0;
}