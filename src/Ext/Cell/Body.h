#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

//#include <New/Entity/FoggedObject.h>

class CellExt
{
public:

	static constexpr size_t Canary = 0x87688621;
	using base_type = CellClass;

	class ExtData final : public TExtension<CellClass>
	{
	public:

		//std::vector<TerrainClass*> AttachedTerrain;
		//int NewPowerups;
		//DynamicVectorClass<FoggedObject*> FoggedObjects;
		//<RadSiteClass*> PlacedRadSite;
		ExtData(CellClass* OwnerObject) : TExtension<CellClass>(OwnerObject)
			//, AttachedTerrain {  }
			//, PlacedRadSite { }
			//, NewPowerups {-1}
			//, FoggedObjects { }
		{ };

		virtual ~ExtData() override = default;
		virtual void Initialize() override { } //Init After INI Read
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {
			//AnnounceInvalidPointer(AttachedTerrain, ptr);
			//FoggedObjects wtf ?
		}

		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }


		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	// Don t call it without checking Tiberium existence
	// otherwise crash
	static TiberiumClass* GetTiberium(CellClass* pCell);
	static int GetOverlayIndex(CellClass* pCell ,TiberiumClass* pTiberium);
	static int GetOverlayIndex(CellClass* pCell);

};