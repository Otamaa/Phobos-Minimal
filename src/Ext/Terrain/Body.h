#pragma once
#include <TerrainClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>

#include <Ext/TerrainType/Body.h>

#include <LightSourceClass.h>
#include <CellClass.h>

class TerrainExt
{
public:
	static constexpr size_t Canary = 0xE1E2E3E4;
	using base_type = TerrainClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public Extension<TerrainClass>
	{
	public:

		LightSourceClass* LighSource;
		AnimClass* AttachedAnim;

		ExtData(TerrainClass* OwnerObject) : Extension<TerrainClass>(OwnerObject)
			, LighSource { }
			, AttachedAnim { }
		{ }

		virtual ~ExtData() {
			CallDTOR(LighSource);
			CallDTOR(AttachedAnim);
		};

		void InvalidatePointer(void *ptr, bool bRemoved);
		void Uninitialize()
		{
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		void InitializeConstants();
		void InitializeLightSource();
		void InitializeAnim();
		void ClearLightSource();
		void ClearAnim();

	private:
		template <typename T>
		void Serialize(T& Stm);

	};

	class ExtContainer final : public Container<TerrainExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		bool InvalidateExtDataIgnorable(void* const ptr) const
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Anim:
			case AbstractType::LightSource:
				return false;
			default:
				return true;
			}
		}

		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;

	static void Unlimbo(TerrainClass* pThis);
	static void CleanUp(TerrainClass* pThis);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};