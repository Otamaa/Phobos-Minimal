#include "Body.h"

#include "../ParticleType/Body.h"
#include <Utilities/Macro.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

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
	//Debug::Log("Processing Element From ParticleExt ! \n");

	Stm
		.Process(this->LaserTrails)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->Trails)
#endif
		;
}

void ParticleExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<ParticleClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ParticleExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<ParticleClass>::SaveToStream(Stm);
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

DEFINE_HOOK(0x62BB13, ParticleClass_CTOR, 0x5)
{
	GET(ParticleClass*, pItem, ESI);
	ParticleExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x62BCED, ParticleClass_DTOR, 0xA)
DEFINE_HOOK(0x62D9CD, ParticleClass_DTOR, 0xA)
{
	GET(ParticleClass* const, pItem, ESI);
	ParticleExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x62D810, ParticleClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x62D7A0, ParticleClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParticleClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParticleExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x62D801, ParticleClass_Load_Suffix, 0x6)
{
	ParticleExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x62D825, ParticleClass_Save_Suffix, 0x7)
{
	GET(const HRESULT , nRest, EAX);

	ParticleExt::ExtMap.GetSavingObject()->byte130 = true;
	if (SUCCEEDED(nRest)) {
		ParticleExt::ExtMap.SaveStatic();
	}

	return 0x62D82C;
}

static void __fastcall ParticleClass_Detach(ParticleClass* pThis, void* _, AbstractClass* pTarget, bool bRemove)
{
	pThis->ObjectClass::PointerExpired(pTarget ,bRemove);

	if (auto pExt = ParticleExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(pTarget, bRemove);
}

DEFINE_JUMP(VTABLE, 0x7EF97C, GET_OFFSET(ParticleClass_Detach))