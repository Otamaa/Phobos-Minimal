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

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/Trails.h>
#endif

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

		TechnoClass* Invoker;
		std::vector<LaserTrailClass> LaserTrails;
#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<UniversalTrail> Trails;
#endif
		ExtData(VoxelAnimClass* OwnerObject) : Extension<VoxelAnimClass>(OwnerObject)
			, Invoker { nullptr }
			, LaserTrails { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
		{ }

		virtual ~ExtData() override = default;
		void InvalidatePointer(void* ptr, bool bRemoved);

		bool InvalidateIgnorable(void* const ptr) const {

			switch (GetVtableAddr(ptr))
			{
			case AircraftClass::vtable:
			case BuildingClass::vtable:
			case InfantryClass::vtable:
			case UnitClass::vtable:
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
