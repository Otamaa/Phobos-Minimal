#include "Body.h"
#include <Ext/House/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>

VoxelAnimExt::ExtContainer VoxelAnimExt::ExtMap;

TechnoClass* VoxelAnimExt::GetTechnoOwner(VoxelAnimClass* pThis, bool DealthByOwner)
{
	if (!DealthByOwner)
		return nullptr;

	return VoxelAnimExt::ExtMap.Find(pThis)->Invoker;
}

void VoxelAnimExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	if (this->InvalidateIgnorable(ptr))
		return;

	AnnounceInvalidPointer(Invoker, ptr);
}

void VoxelAnimExt::ExtData::InitializeLaserTrails(VoxelAnimTypeExt::ExtData* pTypeExt)
{
	auto pThis = Get();

	if (LaserTrails.size())
		return;

	auto const pInvoker = VoxelAnimExt::GetTechnoOwner(Get(), pTypeExt->Damage_DealtByOwner.Get());
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
#ifdef COMPILE_PORTED_DP_FEATURES
	Trails.reserve(1);
#endif
	if (auto pTypeExt = VoxelAnimTypeExt::ExtMap.Find<true>(Get()->Type))
	{
		//ID = Get()->Type->ID;
		if (pTypeExt->LaserTrail_Types.size() > 0)
			LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

		InitializeLaserTrails(pTypeExt);
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsManager::Construct(Get());
#endif
	}
}

// =============================
// load / save
template <typename T>
void VoxelAnimExt::ExtData::Serialize(T& Stm)
{
	Debug::Log("Processing Element From VoxelAnimExt ! \n");

	 Stm
		//.Process(ID)
		.Process(Invoker)
		.Process(LaserTrails)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(Trails)
#endif
		;
}

void VoxelAnimExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TExtension<VoxelAnimClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void VoxelAnimExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TExtension<VoxelAnimClass>::SaveToStream(Stm);
	this->Serialize(Stm);
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

VoxelAnimExt::ExtContainer::ExtContainer() : TExtensionContainer("VoxelAnimClass") { }
VoxelAnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x7498C3, VoxelAnimClass_CTOR, 0x5)
DEFINE_HOOK(0x7498B0, VoxelAnimClass_CTOR, 0x5)
{
	GET(VoxelAnimClass*, pItem, ESI);
#ifndef ENABLE_NEWHOOKS
	VoxelAnimExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	VoxelAnimExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x749B02, VoxelAnimClass_DTOR, 0xA)
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

// Before : DEFINE_HOOK(0x74A9FD, VoxelAnimClass_Load_Suffix, 0x5)
DEFINE_HOOK(0x74A9EA , VoxelAnimClass_Load_Suffix, 0x6)
{
	GET(VoxelAnimClass*, pThis, ESI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->OwnerHouse);
	VoxelAnimExt::ExtMap.LoadStatic();

	return 0x74A9FB;
}

DEFINE_HOOK(0x74AA24, VoxelAnimClass_Save_Suffix, 0x3)
{
	GET(const HRESULT, nRes, EAX);

	if(SUCCEEDED(nRes))
		VoxelAnimExt::ExtMap.SaveStatic();

	return 0;
}

static void __fastcall VoxelAnimClass_Detach(VoxelAnimClass* pThis,void* _, AbstractClass* pTarget, bool bRemove)
{
	pThis->ObjectClass::PointerExpired(pTarget, bRemove);

	if (auto pExt = VoxelAnimExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(pTarget, bRemove);
}

DEFINE_JUMP(VTABLE ,0x7F6340 , GET_OFFSET(VoxelAnimClass_Detach))