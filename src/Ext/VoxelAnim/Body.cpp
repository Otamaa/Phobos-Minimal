#include "Body.h"
#include <Ext/House/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>

TechnoClass* VoxelAnimExt::GetTechnoOwner(VoxelAnimClass* pThis)
{
	auto const pTypeExt = VoxelAnimTypeExt::ExtMap.TryFind(pThis->Type);

	if (!pTypeExt || !pTypeExt->Damage_DealtByOwner)
		return nullptr;

	auto const pExt  = VoxelAnimExt::ExtMap.TryFind(pThis);

	if (!pExt || pExt->GetInitStatus() < InitState::Constanted || !pExt->Invoker)
		return nullptr;

	const auto pAddr = (((DWORD*)pExt->Invoker)[0]);
	if (pAddr != UnitClass::vtable
		&& pAddr != AircraftClass::vtable
		&& pAddr != InfantryClass::vtable
		&& pAddr != BuildingClass::vtable)
	{
		return nullptr;
	}

	return pExt->Invoker;
}

void VoxelAnimExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(Invoker, ptr);
}

void VoxelAnimExt::ExtData::InitializeLaserTrails(VoxelAnimTypeExt::ExtData* pTypeExt)
{
	auto pThis = Get();

	if (!LaserTrails.empty())
		return;

	auto const pInvoker = VoxelAnimExt::GetTechnoOwner(pThis);
	auto const pOwner = pThis->OwnerHouse ? 
		pThis->OwnerHouse : pInvoker ? pInvoker->Owner : HouseExt::FindCivilianSide();

	if(!pTypeExt->LaserTrail_Types.empty())
		LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

	for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
	{
		LaserTrails.emplace_back(LaserTrailTypeClass::Array[idxTrail].get(), pOwner->LaserColor);
	}
}

// =============================
// load / save
template <typename T>
void VoxelAnimExt::ExtData::Serialize(T& Stm)
{
	//Debug::Log("Processing Element From VoxelAnimExt ! \n");

	 Stm
		.Process(this->Initialized)
		.Process(Invoker)
		.Process(LaserTrails)
		.Process(Trails)
		;
}

// =============================
// container
VoxelAnimExt::ExtContainer VoxelAnimExt::ExtMap;

VoxelAnimExt::ExtContainer::ExtContainer() : Container("VoxelAnimClass") { }
VoxelAnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x7494CE , VoxelAnimClass_CTOR, 0x6)
{
	GET(VoxelAnimClass*, pItem, ESI);

	if (auto pExt = VoxelAnimExt::ExtMap.Allocate(pItem))
	{
		if (const auto pTypeExt = VoxelAnimTypeExt::ExtMap.TryFind(pItem->Type))
		{
			//ID = Get()->Type->ID;
			if (!pTypeExt->LaserTrail_Types.empty())
				pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

			pExt->InitializeLaserTrails(pTypeExt);
			TrailsManager::Construct(pItem);
		}
	}

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

static void FC VoxelAnimClass_Detach(VoxelAnimClass* pThis,void* _, AbstractClass* pTarget, bool bRemove)
{
	pThis->ObjectClass::PointerExpired(pTarget, bRemove);
	VoxelAnimExt::ExtMap.InvalidatePointerFor(pThis, pTarget, bRemove);
}

DEFINE_JUMP(VTABLE ,0x7F6340 , GET_OFFSET(VoxelAnimClass_Detach))