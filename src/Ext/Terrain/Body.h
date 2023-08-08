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

		UniqueGamePtrB<LightSourceClass> LighSource { };
		AnimClass* AttachedAnim { nullptr };

		ExtData(TerrainClass* OwnerObject) : Extension<TerrainClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void InvalidatePointer(void* ptr, bool bRemoved);
		static bool InvalidateIgnorable(void* ptr) {
			switch (GetVtableAddr(ptr))
			{
			case AnimClass::vtable:
			case LightSourceClass::vtable:
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
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void Unlimbo(TerrainClass* pThis, CoordStruct* pCoord);
	static void CleanUp(TerrainClass* pThis);
};