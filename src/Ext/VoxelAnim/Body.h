#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
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
	using base_type = VoxelAnimClass;

	class ExtData final : public Extension<VoxelAnimClass>
	{
	public:

		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
#endif
		ExtData(VoxelAnimClass* OwnerObject) : Extension<VoxelAnimClass>(OwnerObject)
			, LaserTrails { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
		{ }

		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); };
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
			if (VoxelAnimExt::Invokers.find(OwnerObject()) != VoxelAnimExt::Invokers.end()) {
				if (VoxelAnimExt::Invokers.at(OwnerObject()) == ptr)
					VoxelAnimExt::Invokers.erase(OwnerObject());
			}
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void InitializeConstants() override;
		virtual void Uninitialize() override {
			VoxelAnimExt::Invokers.erase(OwnerObject());
		}

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

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Aircraft:
			case AbstractType::Building:
			case AbstractType::Infantry:
			case AbstractType::Unit:
				return false;
			default:
				return true;
			}
		}

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static std::map<VoxelAnimClass*, TechnoClass*> Invokers; //make an minimal map to severely reduce lookups
	static TechnoClass* GetTechnoOwner(VoxelAnimClass* pThis, bool DealthByOwner);

};
