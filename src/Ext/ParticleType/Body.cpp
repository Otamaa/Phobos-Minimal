#include "Body.h"

void ParticleTypeExt::ExtData::Initialize() {
	LaserTrail_Types.reserve(2);
}

void ParticleTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	const char* pID = this->Get()->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);

	switch (pThis->BehavesLike)
	{
	case BehavesLike::Smoke:
	{
		this->DeleteWhenReachWater.Read(exINI, pID, "Smoke.DeleteWhenReachWater");
	}
	break;
	case BehavesLike::Fire:
	{
	//	this->ExpireAfterDamaging.Read(exINI, pID, "Fire.ExpireAfterDamaging");
	//	this->DamagingAnim.Read(exINI, pID, "Fire.DamagingAnim");
	}
	break;
	case BehavesLike::Railgun:
	{ 
		this->ReadjustZ.Read(exINI, pID , "ReadjustZCoord");
		break;
	}
	default:
		break;
	}

	this->LaserTrail_Types.Read(exINI, pID, "LaserTrail.Types");

	this->Trails.Read(exINI, pID, false);

	this->Palette.Read(exINI, pID, "CustomPalette");
	this->DamageRange.Read(exINI, pID, "DamageRange");
}

// =============================
// load / save
template <typename T>
void ParticleTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->LaserTrail_Types)
		.Process(this->ReadjustZ)
		.Process(this->Palette)
		.Process(this->DamageRange)
		.Process(this->DeleteWhenReachWater)
		;

	this->Trails.Serialize(Stm);
}

// =============================
// container
ParticleTypeExt::ExtContainer ParticleTypeExt::ExtMap;
ParticleTypeExt::ExtContainer::ExtContainer() : Container("ParticleTypeClass") {}
ParticleTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x644DBB, ParticleTypeClass_CTOR, 0x5)
{
	GET(ParticleTypeClass*, pItem, ESI);
	ParticleTypeExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x645A42, ParticleTypeClass_SDDTOR, 0xA)
{
	GET(ParticleTypeClass*, pItem, ESI);
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

DEFINE_HOOK_AGAIN(0x645414 , ParticleTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x645405, ParticleTypeClass_LoadFromINI, 0x5)
{
	GET(ParticleTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0xDC , -0x4));

	ParticleTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x645414);
	return 0;
}