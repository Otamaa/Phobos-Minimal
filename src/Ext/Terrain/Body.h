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
	class ExtData final : public Extension<TerrainClass>
	{
	public:
		static constexpr size_t Canary = 0xE1E2E3E4;
		using base_type = TerrainClass;
		//static constexpr size_t ExtOffset = 0xD0;

	public:

		Handle<LightSourceClass* , UninitLightSource> LighSource {};
		Handle<AnimClass* ,UninitAnim> AttachedAnim {};

		ExtData(TerrainClass* OwnerObject) : Extension<TerrainClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
		static bool InvalidateIgnorable(AbstractClass* ptr) {
			switch (ptr->WhatAmI())
			{
			case AnimClass::AbsID:
			case LightSourceClass::AbsID:
				return false;
			}

			return true;
		}

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void InitializeLightSource();
		void InitializeAnim();
		void ClearLightSource();
		void ClearAnim();

		void Uninitialize() { }

	private:
		template <typename T>
		void Serialize(T& Stm);

	};

	class ExtContainer final : public Container<TerrainExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(TerrainExt::ExtData, "TerrainClass");
	};

	static ExtContainer ExtMap;

	static void Unlimbo(TerrainClass* pThis, CoordStruct* pCoord);
	static void CleanUp(TerrainClass* pThis);
};