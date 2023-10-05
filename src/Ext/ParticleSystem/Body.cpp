#include "Body.h"

#include <Ext/ParticleSystemType/Body.h>

#include <ParticleTypeClass.h>
#include <ParticleClass.h>


void NOINLINE ParticleSystemExt::ExtData::InitializeConstants()
{
	if (auto pType = OwnerObject()->Type)
	{
		if (!ParticleSystemTypeExt::ExtMap.Find(pType)->ApplyOptimization || (size_t)pType->HoldsWhat >= ParticleTypeClass::Array->Size())
			return ;

			this->HeldType = ParticleTypeClass::Array->GetItem(pType->HoldsWhat);

		if (!this->HeldType->UseLineTrail && !this->HeldType->AlphaImage) {

			auto bIsZero = (int)this->HeldType->BehavesLike;
			auto nBehave = (int)pType->BehavesLike;

			if (bIsZero <= 1)
				bIsZero = bIsZero == 0;

			if (nBehave == bIsZero) {

				if (nBehave == 0) {
					this->What = Behave::Smoke;
					return;
				}

				auto v11 = nBehave - 3;

				if (!v11) {
					this->What = Behave::Spark;
					return;
				}

				if (v11 == 1) {
					this->What = Behave::Railgun;
					return;
				}
			}
		}
	}
}

template <typename T>
void ParticleSystemExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->What)
		.Process(this->OtherParticleData)
		.Process(this->SmokeData)
		.Process(this->HeldType)
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
