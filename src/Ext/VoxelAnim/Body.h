#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>

#include <Helpers/Macro.h>

#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>

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
	static VoxelAnimExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(VoxelAnimExtContainer, VoxelAnimExtData, "VoxelAnimClass");
};

