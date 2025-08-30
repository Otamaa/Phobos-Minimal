#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/VoxelAnimType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/Macro.h>

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>

TechnoClass* VoxelAnimExtData::GetTechnoOwner(VoxelAnimClass* pThis)
{
	auto const pTypeExt = VoxelAnimTypeExtContainer::Instance.TryFind(pThis->Type);

	if (!pTypeExt || !pTypeExt->Damage_DealtByOwner)
		return nullptr;

	auto const pExt  = VoxelAnimExtContainer::Instance.TryFind(pThis);

	if (!pExt || !pExt->Invoker)
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

void VoxelAnimExtData::InitializeLaserTrails(VoxelAnimTypeExtData* pTypeExt)
{
	auto pThis = this->AttachedToObject;

	if (!LaserTrails.empty())
		return;

	auto const pInvoker = VoxelAnimExtData::GetTechnoOwner(pThis);
	auto const pOwner = pThis->OwnerHouse ?
		pThis->OwnerHouse : pInvoker ? pInvoker->Owner : HouseExtData::FindFirstCivilianHouse();

	LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

	for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
	{
		LaserTrails.emplace_back(
			std::move(std::make_unique<LaserTrailClass>(
			LaserTrailTypeClass::Array[idxTrail].get(), pOwner->LaserColor)));
	}
}

// =============================
// load / save
template <typename T>
void VoxelAnimExtData::Serialize(T& Stm)
{
	//Debug::LogInfo("Processing Element From VoxelAnimExt ! ");

	 Stm
		.Process(this->Invoker, true)
		.Process(this->LaserTrails)
		.Process(this->Trails)
		.Process(this->TrailerSpawnDelayTimer)
		;
}

// =============================
// container
VoxelAnimExtContainer VoxelAnimExtContainer::Instance;
std::vector<VoxelAnimExtData*> Container<VoxelAnimExtData>::Array;

// =================================
ASMJIT_PATCH(0x7494CE , VoxelAnimClass_CTOR, 0x6)
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

ASMJIT_PATCH(0x749B02, VoxelAnimClass_DTOR, 0xA)
{
	GET(VoxelAnimClass*, pItem, ECX);

	VoxelAnimExtContainer::Instance.Remove(pItem);

	return 0;
}

void FakeVoxelAnimClass::_Detach(AbstractClass* pTarget, bool bRemoved)
{
	this->ObjectClass::PointerExpired(pTarget, bRemoved);
	VoxelAnimExtContainer::Instance.InvalidatePointerFor(this, pTarget, bRemoved);
}

DEFINE_FUNCTION_JUMP(VTABLE ,0x7F6340 , FakeVoxelAnimClass::_Detach)