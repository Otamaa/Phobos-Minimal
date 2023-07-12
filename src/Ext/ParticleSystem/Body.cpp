#include "Body.h"

#include <ParticleTypeClass.h>
#include <ParticleClass.h>


void ParticleSystemExt::ExtData::InitializeConstants()
{
	auto const pThis = this->OwnerObject();
	if (auto pType = pThis->Type)
	{
		if (pType->HoldsWhat > 0)
		{
			auto pHoldType = ParticleTypeClass::Array->Items[pType->HoldsWhat];
			this->AdditionalHeldType = pHoldType;
			if (!pHoldType->UseLineTrail && !pHoldType->AlphaImage)
			{
				// Note : wtf is there , the switch statement become shit statemets
				this->Behave = pHoldType->BehavesLike;
			}
		}
	}
}

template <typename T>
void ParticleSystemExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Behave)
		.Process(this->PreCalculatedParticlesData)
		.Process(this->SomeArray_b)
		.Process(this->AdditionalHeldType)
		;
}

// =============================
// container
ParticleSystemExt::ExtContainer ParticleSystemExt::ExtMap;
ParticleSystemExt::ExtContainer::ExtContainer() : Container("ParticleSystemClass") { }
ParticleSystemExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x62DF05, ParticleSystemClass_CTOR, 0x5)
{
	GET(ParticleSystemClass*, pItem, ESI);
	ParticleSystemExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x62E26B, ParticleSystemClass_DTOR, 0x6)
{
	GET(ParticleSystemClass* const, pItem, ESI);
	ParticleSystemExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x62FF20, ParticleSystemClass_SaveLoad_Prefix, 0x7)
DEFINE_HOOK(0x630090, ParticleSystemClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleSystemClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleSystemExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x630088, ParticleSystemClass_Load_Suffix, 0x5)
{
	ParticleSystemExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6300F3, ParticleSystemClass_Save_Suffix, 0x6)
{
	ParticleSystemExt::ExtMap.SaveStatic();
	return 0x0;
}
