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
	static constexpr size_t Canary = 0xAAACAACC;
	using base_type = VoxelAnimClass;
	static constexpr size_t ExtOffset = 0x144;

	class ExtData final : public Extension<VoxelAnimClass>
	{
	public:
		//FixedString<0x32> ID;
		TechnoClass* Invoker;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
#endif
		ExtData(VoxelAnimClass* OwnerObject) : Extension<VoxelAnimClass>(OwnerObject)
			//, ID { }
			, Invoker { nullptr }
			, LaserTrails { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override {
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


		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void InitializeConstants()override;

		void Uninitialize() {}
		void InitializeLaserTrails(VoxelAnimTypeExt::ExtData* pTypeExt);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<VoxelAnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static TechnoClass* GetTechnoOwner(VoxelAnimClass* pThis);

};
