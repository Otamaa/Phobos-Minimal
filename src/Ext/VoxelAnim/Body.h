#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/Container.h>
#include <Ext/Abstract/Body.h>
//#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>

#include <Helpers/Macro.h>

#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

class VoxelAnimExt
{
public:
	class ExtData final : public Extension<VoxelAnimClass>
	{
	public:
		static constexpr size_t Canary = 0xAAACAACC;
		using base_type = VoxelAnimClass;
		static constexpr size_t ExtOffset = 0x144;

	public:

		TechnoClass* Invoker { nullptr };
		std::vector<LaserTrailClass> LaserTrails { };
		std::vector<UniversalTrail> Trails { };

		ExtData(VoxelAnimClass* OwnerObject) : Extension<VoxelAnimClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;
		void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

		static bool InvalidateIgnorable(AbstractClass* ptr) {

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

		void Uninitialize() {}
		void InitializeLaserTrails(VoxelAnimTypeExt::ExtData* pTypeExt);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<VoxelAnimExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static TechnoClass* GetTechnoOwner(VoxelAnimClass* pThis);

};
