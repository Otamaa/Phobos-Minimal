#pragma once

#include <Utilities/PhobosMap.h>
#include <string>
#include <memory>
#include <vector>

class SWColumnClass;
class ToggleSWButtonClass;
class SWButtonClass;
class CommandClass;
class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();

	void Init_Clear();

	bool AddButton(int superIdx);
	void SortButtons();

	int GetMaximumButtonCount();

	static bool IsEnabled();

private:
	inline static std::unique_ptr<SWSidebarClass> Instance;
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

		//for(auto& com : Commands)
		//  com = nullptr;
	}

public:
	std::vector<SWColumnClass*> Columns {};
	SWColumnClass* CurrentColumn { nullptr };
	SWButtonClass* CurrentButton { nullptr };
	ToggleSWButtonClass* ToggleButton { nullptr };

	static inline CommandClass* Commands[10];
};