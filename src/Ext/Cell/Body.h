#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

//#include <New/Entity/FoggedObject.h>

class CellExt
{
public:
	using base_type = CellClass;

	class ExtData final : public TExtension<CellClass>
	{
	public:

		//int NewPowerups;
		//DynamicVectorClass<FoggedObject*> FoggedObjects;

		ExtData(CellClass* OwnerObject) : TExtension<CellClass>(OwnerObject)
			//, NewPowerups {-1}
			//, FoggedObjects { }
        { };

		virtual ~ExtData() = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void Initialize() override { } //Init After INI Read
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {
			//FoggedObjects wtf ?
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

    };

	_declspec(noinline) static CellExt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::Cell
			? reinterpret_cast<CellExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	class ExtContainer final : public TExtensionContainer<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};