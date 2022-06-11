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
	using base_type = TerrainClass;

	class ExtData final : public Extension<TerrainClass>
	{
	public:

		UniqueGamePtr<LightSourceClass> LighSource;
		UniqueGamePtr<AnimClass> AttachedAnim;
		TerrainTypeExt::ExtData* TypeData;

		ExtData(TerrainClass* OwnerObject) : Extension<TerrainClass>(OwnerObject)
			, LighSource { }
			, AttachedAnim { }
			, TypeData { nullptr }
		{ }

		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); };
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override
		{
			if (LighSource.get() && (void*)LighSource.get() == ptr)
				LighSource.release();

			if (AttachedAnim.get() && (void*)AttachedAnim.get() == ptr)
				AttachedAnim.release();
		}

		virtual void Uninitialize() override
		{
			ClearLightSource();
			ClearAnim();
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void InitializeConstants() override;
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

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
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
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static void Unlimbo(TerrainClass* pThis);
	static void CleanUp(TerrainClass* pThis);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};