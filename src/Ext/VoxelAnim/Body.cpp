#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/VoxelAnimType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/Macro.h>

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>

#include <Phobos.SaveGame.h>

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
	auto pThis = this->This();

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

bool VoxelAnimExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(VoxelAnimExtContainer::ClassName))
	{
		auto& container = root[VoxelAnimExtContainer::ClassName];

		for (auto& entry : container[VoxelAnimExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, VoxelAnimExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool VoxelAnimExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[VoxelAnimExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : VoxelAnimExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[VoxelAnimExtData::ClassName] = std::move(_extRoot);

	return true;
}

// =================================
ASMJIT_PATCH(0x7494CE , VoxelAnimClass_CTOR, 0x6)
{
	GET(VoxelAnimClass*, pItem, ESI);

	if(pItem->Type){

		auto pExt = VoxelAnimExtContainer::Instance.Allocate(pItem);
		const auto pTypeExt = VoxelAnimTypeExtContainer::Instance.Find(pItem->Type);

		if (!pTypeExt->LaserTrail_Types.empty())
				pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

		pExt->InitializeLaserTrails(pTypeExt);
		TrailsManager::Construct(pItem);
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
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(pTarget, bRemoved);
}

DEFINE_FUNCTION_JUMP(VTABLE ,0x7F6340 , FakeVoxelAnimClass::_Detach)

