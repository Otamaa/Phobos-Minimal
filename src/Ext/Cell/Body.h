#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/FoggedObject.h>

class CellExt
{
public:
	class ExtData final : public Extension<CellClass>
	{
	public:

		static constexpr size_t Canary = 0x87688621;
		using base_type = CellClass;

	public:

		DynamicVectorClass<FoggedObject*> FoggedObjects {};
		ExtData(CellClass* OwnerObject) : Extension<CellClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<CellExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(CellExt::ExtData, "CellClass");
	};

	static ExtContainer ExtMap;

	// Don t call it without checking Tiberium existence
	// otherwise crash
	static TiberiumClass* GetTiberium(CellClass* pCell);
	static int GetOverlayIndex(CellClass* pCell ,TiberiumClass* pTiberium);
	static int GetOverlayIndex(CellClass* pCell);
};