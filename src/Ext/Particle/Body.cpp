#include "Body.h"

#include "../ParticleType/Body.h"
#include <Utilities/Macro.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

template<> const DWORD Extension<ParticleClass>::Canary = 0xAAAABBBB;
ParticleExt::ExtContainer ParticleExt::ExtMap;

void ParticleExt::ExtData::InitializeConstants()
{
	LaserTrails.reserve(1);
#ifdef COMPILE_PORTED_DP_FEATURES
	Trails.reserve(1);
#endif
	const auto pThis = Get();

	if (auto const pTypeExt = ParticleTypeExt::ExtMap.Find(pThis->Type))
	{
		CoordStruct nFLH = CoordStruct::Empty;
		const ColorStruct nColor = pThis->GetOwningHouse() ? pThis->GetOwningHouse()->LaserColor : ColorStruct::Empty;

		if (LaserTrails.empty() && !LaserTrailTypeClass::Array.empty())
		{
			LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

			for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
			{
				if (auto const pLaserType = LaserTrailTypeClass::Array[idxTrail].get())
				{
					LaserTrails.push_back(std::make_unique<LaserTrailClass>(pLaserType, nColor, nFLH));

				}
			}
		}
	}

#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::Construct(Get());
#endif

}

// =============================
// load / save


template <typename T>
void ParticleExt::ExtData::Serialize(T& Stm)
{
	Debug::Log("Processing Element From ParticleExt ! \n");

	Stm
		.Process(this->LaserTrails)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->Trails)
#endif
		;
}

void ParticleExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<ParticleClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void ParticleExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<ParticleClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void ParticleExt::ExtData::Uninitialize() {
	LaserTrails.clear();
#ifdef COMPILE_PORTED_DP_FEATURES
	Trails.clear();
#endif
}

// =============================
// container

ParticleExt::ExtContainer::ExtContainer() : Container("ParticleClass") { }
ParticleExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

/*
DEFINE_HOOK(0x62BB06, ParticleClass_CTOR, 0x5)
{
	GET(ParticleClass*, pItem, ESI);
	ParticleExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

bool __fastcall ObjectClass_limbo_Particle(ObjectClass* pObj, void* _) {
	if(const auto pParticle = specific_cast<ParticleClass*>(pObj)) {
		ParticleExt::ExtMap.Remove(pParticle);
	}

	return pObj->Limbo();
}

DEFINE_JUMP(CALL,0x62D9BF, GET_OFFSET(ObjectClass_limbo_Particle));

DEFINE_HOOK_AGAIN(0x62D810, ParticleClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x62D7A0, ParticleClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x62D803, ParticleClass_Load_Suffix, 0x5)
{
	ParticleExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x62D825, ParticleClass_Save_Suffix, 0x8)
{
	ParticleExt::ExtMap.SaveStatic();
	return 0;
}*/