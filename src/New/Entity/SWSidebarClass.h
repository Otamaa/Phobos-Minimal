#pragma once

#include <Utilities/PhobosMap.h>
#include <string>
#include <memory>
#include <vector>

class SWColumnClass;
class ToggleSWButtonClass;
class TacticalButtonClass;
class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();

	void Init_Clear();

	bool AddButton(int superIdx);
	void SortButtons();

	void RecordHotkey(int buttonIndex, int key);
	int GetMaximumButtonCount();

	static bool IsEnabled();

private:
	static std::unique_ptr<SWSidebarClass> Instance;
public:
	static constexpr void Allocate() {
		Instance = std::make_unique<SWSidebarClass>();
	}

	static constexpr void Remove() {
		Instance = nullptr;
	}

	static constexpr SWSidebarClass* Global()
	{
		return Instance.get();
	}

	static constexpr void Clear()
	{
		Allocate();
	}

public:
	std::vector<SWColumnClass*> Columns {};
	SWColumnClass* CurrentColumn { nullptr };
	TacticalButtonClass* CurrentButton { nullptr };
	ToggleSWButtonClass* ToggleButton { nullptr };

	std::wstring KeyCodeText[10] {};
	int KeyCodeData[10] {};
};