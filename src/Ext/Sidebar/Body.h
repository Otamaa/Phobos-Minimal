#pragma once

#include <SidebarClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <map>

class SidebarExtData final
{
private:
	static std::unique_ptr<SidebarExtData> Data;

public:

	static constexpr size_t Canary = 0x51DEBA12;
	using base_type = SidebarClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	SidebarExtData() noexcept = default;
	~SidebarExtData() noexcept = default;

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	static IStream* g_pStm;
	static std::array<SHPReference*, 4u> TabProducingProgress;

	static void Allocate(SidebarClass* pThis);
	static void Remove(SidebarClass* pThis);

	constexpr FORCEINLINE static SidebarExtData* Instance()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(SidebarClass::Instance);
	}

	static void DrawProducingProgress();
};