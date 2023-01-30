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
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public TExtension<VoxelAnimClass>
	{
	public:
		//FixedString<0x32> ID;
		TechnoClass* Invoker;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
#endif
		ExtData(VoxelAnimClass* OwnerObject) : TExtension<VoxelAnimClass>(OwnerObject)
			//, ID { }
			, Invoker { nullptr }
			, LaserTrails { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
		{ }

		virtual ~ExtData() = default;
		void InvalidatePointer(void* ptr, bool bRemoved);

		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		void InitializeConstants();
		void Uninitialize() {}

		void InitializeLaserTrails(VoxelAnimTypeExt::ExtData* pTypeExt);
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<VoxelAnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		bool InvalidateExtDataIgnorable(void* const ptr) const
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Aircraft:
			case AbstractType::Building:
			case AbstractType::Infantry:
			case AbstractType::Unit:
				return false;
			}

			return true;
		}

	    void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static TechnoClass* GetTechnoOwner(VoxelAnimClass* pThis, bool DealthByOwner);

};
