#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/VoxelAnimType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/Macro.h>

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>

VoxelAnimExtData::~VoxelAnimExtData() { };

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
	//Debug::LogInfo("Processing Element From VoxelAnimExt ! ");

	 Stm
		.Process(this->Initialized)
		.Process(this->Invoker, true)
		.Process(this->LaserTrails)
		.Process(this->Trails)
		.Process(this->TrailerSpawnDelayTimer)
		;
}

// =============================
// container
VoxelAnimExtContainer VoxelAnimExtContainer::Instance;

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
#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeVoxelAnimClass::_Load(IStream* pStm)
{

	VoxelAnimExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->VoxelAnimClass::Load(pStm);

	if (SUCCEEDED(res))
		VoxelAnimExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeVoxelAnimClass::_Save(IStream* pStm, bool clearDirty)
{

	VoxelAnimExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->VoxelAnimClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		VoxelAnimExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F632C, FakeVoxelAnimClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6330, FakeVoxelAnimClass::_Save)

void FakeVoxelAnimClass::_Detach(AbstractClass* pTarget, bool bRemoved)
{
	this->ObjectClass::PointerExpired(pTarget, bRemoved);
	VoxelAnimExtContainer::Instance.InvalidatePointerFor(this, pTarget, bRemoved);
}

DEFINE_FUNCTION_JUMP(VTABLE ,0x7F6340 , FakeVoxelAnimClass::_Detach)