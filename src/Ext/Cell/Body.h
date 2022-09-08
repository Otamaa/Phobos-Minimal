#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

//#include <New/Entity/FoggedObject.h>

class CellExt
{
public:
	static constexpr size_t Canary = 0x87688621;
	using base_type = CellClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

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
		void Initialize() { } //Init After INI Read
		//void InvalidatePointer(void* ptr, bool bRemoved) {
			//FoggedObjects wtf ?
		//}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
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

	// Dont call it without checking Tiberium existence
	// otherwise crash
	static TiberiumClass* GetTiberium(CellClass* pCell);
	static int GetOverlayIndex(CellClass* pCell ,TiberiumClass* pTiberium);
	static int GetOverlayIndex(CellClass* pCell);

};