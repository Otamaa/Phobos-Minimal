#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/VoxelAnimType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/Macro.h>

TechnoClass* VoxelAnimExtData::GetTechnoOwner(VoxelAnimClass* pThis)
{
	auto const pTypeExt = VoxelAnimTypeExtContainer::Instance.TryFind(pThis->Type);

	if (!pTypeExt || !pTypeExt->Damage_DealtByOwner)
		return nullptr;

	auto const pExt  = VoxelAnimExtContainer::Instance.TryFind(pThis);

	if (!pExt || pExt->Initialized < InitState::Constanted || !pExt->Invoker)
		return nullptr;

	const auto pAddr = pExt->Invoker->WhatAmI();
	if (pAddr != UnitClass::AbsID
		&& pAddr != AircraftClass::AbsID
		&& pAddr != InfantryClass::AbsID
		&& pAddr != BuildingClass::AbsID)
	{
		return nullptr;
	}

	return pExt->Invoker;
}

void VoxelAnimExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(Invoker, ptr , bRemoved);
}

void VoxelAnimExtData::InitializeLaserTrails(VoxelAnimTypeExtData* pTypeExt)
{
	auto pThis = this->AttachedToObject;

	if (!LaserTrails.empty())
		return;

	auto const pInvoker = VoxelAnimExtData::GetTechnoOwner(pThis);
	auto const pOwner = pThis->OwnerHouse ?
		pThis->OwnerHouse : pInvoker ? pInvoker->Owner : HouseExtData::FindFirstCivilianHouse();

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
void VoxelAnimExtData::Serialize(T& Stm)
{
	//Debug::Log("Processing Element From VoxelAnimExt ! \n");

	 Stm
		.Process(this->Initialized)
		.Process(this->Invoker)
		.Process(this->LaserTrails)
		.Process(this->Trails)
		;
}

// =============================
// container
VoxelAnimExtContainer VoxelAnimExtContainer::Instance;
std::vector<VoxelAnimExtData*> VoxelAnimExtContainer::Pool;

DEFINE_HOOK(0x7494CE , VoxelAnimClass_CTOR, 0x6)
{
	GET(VoxelAnimClass*, pItem, ESI);

	if (auto pExt = VoxelAnimExtContainer::Instance.Allocate(pItem))
	{
		if (const auto pTypeExt = VoxelAnimTypeExtContainer::Instance.TryFind(pItem->Type))
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

	VoxelAnimExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK(0x74A970, VoxelAnimClass_Load_Prefix, 0x5)
{
	GET_STACK(VoxelAnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	VoxelAnimExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

// Before : DEFINE_HOOK(0x74A9FD, VoxelAnimClass_Load_Suffix, 0x5)
DEFINE_HOOK(0x74A9EA , VoxelAnimClass_Load_Suffix, 0x6)
{
	GET(VoxelAnimClass*, pThis, ESI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->OwnerHouse);
	VoxelAnimExtContainer::Instance.LoadStatic();

	return 0x74A9FB;
}

DEFINE_HOOK(0x74AA10, VoxelAnimClass_Save_Ext, 0x8)
{
	GET_STACK(VoxelAnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	GET_STACK(bool, isDirty, 0xC);

	const auto res = AbstractClass::_Save(pItem, pStm, isDirty);

	if(SUCCEEDED(res)){
		VoxelAnimExtContainer::Instance.PrepareStream(pItem, pStm);
		VoxelAnimExtContainer::Instance.SaveStatic();
	}

	R->EAX(res);
	return 0x74AA24;
}

static void __fastcall VoxelAnimClass_Detach(VoxelAnimClass* pThis,void* _, AbstractClass* pTarget, bool bRemoved)
{
	pThis->ObjectClass::PointerExpired(pTarget, bRemoved);
	VoxelAnimExtContainer::Instance.InvalidatePointerFor(pThis, pTarget, bRemoved);
}

DEFINE_JUMP(VTABLE ,0x7F6340 , GET_OFFSET(VoxelAnimClass_Detach))