#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/FoggedObject.h>

class CellExt
{
public:
	using base_type = CellClass;

	class ExtData final : public Extension<CellClass>
	{
	public:

		//int NewPowerups;
		//DynamicVectorClass<FoggedObject*> FoggedObjects;

		ExtData(CellClass* OwnerObject) : Extension<CellClass>(OwnerObject)
			//, NewPowerups {-1}
			//, FoggedObjects { }
        { };

		virtual ~ExtData() = default;
	///	virtual size_t Size() override { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {
			//FoggedObjects wtf ?
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

    };
	class ExtContainer final : public Container<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};