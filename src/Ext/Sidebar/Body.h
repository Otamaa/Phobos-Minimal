#pragma once

#include <SidebarClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <map>

#include <ToggleClass.h>

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


class TacticalButtonClass : public ToggleClass
{
public:
	TacticalButtonClass() = default;
	TacticalButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height);

	virtual ~TacticalButtonClass();

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag fags, DWORD* pKey, KeyModifier modifier) override;

	bool LaunchSuper(int superIdx);

public:
	static bool AddButton(int superIdx);
	static bool RemoveButton(int superIdx);
	static void ClearButtons();
	static void SortButtons();

public:
	static HelperedVector<TacticalButtonClass*> Buttons;
	static bool Initialized;
	static TacticalButtonClass* CurrentButton;

	bool IsHovering { false };
	int SuperIndex { -1 };
};