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
#ifndef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = 0xD0;
#endif

	class ExtData final : public TExtension<TerrainClass>
	{
	public:

		UniqueGamePtr<LightSourceClass> LighSource;
		UniqueGamePtr<AnimClass> AttachedAnim;

		ExtData(TerrainClass* OwnerObject) : TExtension<TerrainClass>(OwnerObject)
			, LighSource { }
			, AttachedAnim { }
		{ }

		virtual ~ExtData() override = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override {
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Anim:
			case AbstractType::LightSource:
				return false;
			}

			return true;
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;

		virtual void InitializeConstants() override;

		void InitializeLightSource();
		void InitializeAnim();
		void ClearLightSource();
		void ClearAnim();

		void Uninitialize() { }

	private:
		template <typename T>
		void Serialize(T& Stm);

	};

	class ExtContainer final : public TExtensionContainer<TerrainExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void Unlimbo(TerrainClass* pThis, CoordStruct* pCoord);
	static void CleanUp(TerrainClass* pThis);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};