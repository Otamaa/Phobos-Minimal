#pragma once

#include <SidebarClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <map>

class SidebarExt
{
public:
	static IStream* g_pStm;
	static std::array<UniqueGamePtrB<SHPStruct>, 4u> TabProducingProgress;

	class ExtData final : public Extension<SidebarClass>
	{
	public:
		static constexpr size_t Canary = 0x51DEBA12;
		using base_type = SidebarClass;

	public:

		ExtData(SidebarClass* OwnerObject) : Extension<SidebarClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};
private:
	static std::unique_ptr<ExtData> Data;
public:
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

	static void DrawProducingProgress();
};