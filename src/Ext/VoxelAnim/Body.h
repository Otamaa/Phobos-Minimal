#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>

#include <Helpers/Macro.h>

#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

class VoxelAnimExtData final
{
public:
	static constexpr size_t Canary = 0xAAACAACC;
	using base_type = VoxelAnimClass;
	static constexpr size_t ExtOffset = 0x144;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	TechnoClass* Invoker { nullptr };
	std::vector<LaserTrailClass> LaserTrails { };
	std::vector<UniversalTrail> Trails { };

	VoxelAnimExtData() noexcept = default;
	~VoxelAnimExtData() noexcept = default;

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	static bool InvalidateIgnorable(AbstractClass* ptr)
	{

		switch (ptr->WhatAmI())
		{
		case AircraftClass::AbsID:
		case BuildingClass::AbsID:
		case InfantryClass::AbsID:
		case UnitClass::AbsID:
			return false;
		}

		return true;
	}

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void InitializeLaserTrails(VoxelAnimTypeExtData* pTypeExt);
	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(VoxelAnimExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static TechnoClass* GetTechnoOwner(VoxelAnimClass* pThis);
};

class VoxelAnimExtContainer final : public Container<VoxelAnimExtData>
{
public:
	static std::vector<VoxelAnimExtData*> Pool;
	static VoxelAnimExtContainer Instance;

	VoxelAnimExtData* AllocateUnchecked(VoxelAnimClass* key)
	{
		VoxelAnimExtData* val = nullptr;
		if (!Pool.empty())
		{
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
			val->VoxelAnimExtData::VoxelAnimExtData();
		}
		else
		{
			val = new VoxelAnimExtData();
		}

		if (val)
		{
			val->AttachedToObject = key;
			return val;
		}

		return nullptr;
	}

	VoxelAnimExtData* Allocate(VoxelAnimClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (VoxelAnimExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(VoxelAnimClass* key)
	{
		if (VoxelAnimExtData* Item = TryFind(key))
		{
			Item->~VoxelAnimExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			this->ClearExtAttribute(key);
		}
	}

	void Clear()
	{
		if (!Pool.empty())
		{
			auto ptr = Pool.front();
			Pool.erase(Pool.begin());
			if (ptr)
			{
				delete ptr;
			}
		}
	}

	CONSTEXPR_NOCOPY_CLASSB(VoxelAnimExtContainer, VoxelAnimExtData, "VoxelAnimClass");
};

