#include "Body.h"
#include <Ext/House/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>

template<> const DWORD Extension<VoxelAnimClass>::Canary = 0xAAAAAACC;
VoxelAnimExt::ExtContainer VoxelAnimExt::ExtMap;

TechnoClass* VoxelAnimExt::GetTechnoOwner(VoxelAnimClass* pThis, bool DealthByOwner)
{
	if (!DealthByOwner)
		return nullptr;

	if (auto pExt = VoxelAnimExt::ExtMap.Find(pThis))
		return pExt->Invoker;

	return nullptr;
}

void VoxelAnimExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(Invoker, ptr);
}

void VoxelAnimExt::ExtData::InitializeLaserTrails(VoxelAnimTypeExt::ExtData* pTypeExt)
{
	auto pThis = OwnerObject();

	if (LaserTrails.size())
		return;

	auto const pInvoker = VoxelAnimExt::GetTechnoOwner(OwnerObject(), pTypeExt->Damage_DealtByOwner.Get());
	auto const pOwner = pThis->OwnerHouse ? pThis->OwnerHouse : pInvoker ? pInvoker->Owner : HouseExt::FindCivilianSide();

	size_t  nTotal = 0;

	if(pTypeExt->LaserTrail_Types.size() > 0)
	LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

	for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
	{
		if (auto const pLaserType = LaserTrailTypeClass::Array[idxTrail].get())
		{
			LaserTrails.emplace_back(std::make_unique<LaserTrailClass>
				(pLaserType, pOwner->LaserColor));
			++nTotal;
		}
	}

	if (nTotal > 0)
		LaserTrails.resize(nTotal);
	else
		LaserTrails.clear();
}

void VoxelAnimExt::ExtData::InitializeConstants()
{
	LaserTrails.reserve(1);
#ifdef COMPILE_PORTED_DP_FEATURES
	Trails.reserve(1);
#endif
	if (auto pTypeExt = VoxelAnimTypeExt::GetExtData(OwnerObject()->Type))
	{
		if (pTypeExt->LaserTrail_Types.size() > 0)
			LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

		InitializeLaserTrails(pTypeExt);
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsManager::Construct(OwnerObject());
#endif
	}
}

// =============================
// load / save
template <typename T>
void VoxelAnimExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(LaserTrails)
		.Process(Invoker)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(Trails)
#endif
		;
}

void VoxelAnimExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<VoxelAnimClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void VoxelAnimExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<VoxelAnimClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void VoxelAnimExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved)
{
}

bool VoxelAnimExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool VoxelAnimExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

VoxelAnimExt::ExtContainer::ExtContainer() : Container("VoxelAnimClass") { }
VoxelAnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//DEFINE_HOOK(0x749951, VoxelAnimClass_CTOR, 0xC)
DEFINE_HOOK(0x74942E, VoxelAnimClass_CTOR, 0xC)
{
	GET(VoxelAnimClass*, pItem, ESI);
	VoxelAnimExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x7499F1, VoxelAnimClass_DTOR, 0x5)
{
	GET(VoxelAnimClass*, pItem, ECX);

	VoxelAnimExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x74A970, VoxelAnimClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x74AA10, VoxelAnimClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(VoxelAnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	VoxelAnimExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x74A9FB, VoxelAnimClass_Load_Suffix, 0x5)
{
	VoxelAnimExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x74AA24, VoxelAnimClass_Save_Suffix, 0x5)
{
	VoxelAnimExt::ExtMap.SaveStatic();
	return 0;
}