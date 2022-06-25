#pragma once

#include <VoxelAnimClass.h>

#include <Ext/Abstract/Body.h>
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

	class ExtData final : public TExtension<VoxelAnimClass>
	{
	public:

		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		TechnoClass* Invoker;
#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
#endif
		ExtData(VoxelAnimClass* OwnerObject) : TExtension<VoxelAnimClass>(OwnerObject)
			, LaserTrails { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
		{ }

		virtual ~ExtData() = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {

			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Aircraft:
			case AbstractType::Building:
			case AbstractType::Infantry:
			case AbstractType::Unit:
				AnnounceInvalidPointer(Invoker, ptr);
				break;
			}

		}

		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void InitializeConstants() override;
		virtual void Uninitialize() override {
		}

		void InitializeLaserTrails(VoxelAnimTypeExt::ExtData* pTypeExt);
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	__declspec(noinline) static VoxelAnimExt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::VoxelAnim ? reinterpret_cast<VoxelAnimExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	class ExtContainer final : public TExtensionContainer<VoxelAnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static TechnoClass* GetTechnoOwner(VoxelAnimClass* pThis, bool DealthByOwner);

};
