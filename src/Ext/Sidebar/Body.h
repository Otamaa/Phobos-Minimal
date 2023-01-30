#pragma once

#include <SidebarClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <map>

class SidebarExt
{
public:
	static constexpr size_t Canary = 0x51DEBA12;
	using base_type = SidebarClass;

	class ExtData final : public Extension<SidebarClass>
	{
	public:

		ExtData(SidebarClass* OwnerObject) : Extension<SidebarClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;
		void Uninitialize() { }
		// void InvalidatePointer(void* ptr, bool bRemoved) { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static ArrayWrapper<SHPStruct* , 4u> TabProducingProgress;

	static void Allocate(SidebarClass* pThis);
	static void Remove(SidebarClass* pThis);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(SidebarClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		// Global()->InvalidatePointer(ptr, removed);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};