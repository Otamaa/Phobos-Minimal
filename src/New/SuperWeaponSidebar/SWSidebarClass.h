#pragma once

#include <memory>
#include <vector>

#include <Base/Always.h>

class SWColumnClass;
class ToggleSWButtonClass;
class SWButtonClass;
class CommandClass;
class SWSidebarClass
{
public:
	bool AddColumn();
	bool RemoveColumn();

	void InitClear();
	void InitIO();

	bool AddButton(int superIdx);
	void SortButtons();

	int GetMaximumButtonCount();

	static bool IsEnabled();
	static void RecheckCameo();

private:
	static SWSidebarClass Instance;
public:

	static SWSidebarClass* Global()
	{
		return &Instance;
	}

public:
	std::vector<SWColumnClass*> Columns {};
	SWColumnClass* CurrentColumn { nullptr };
	SWButtonClass* CurrentButton { nullptr };
	ToggleSWButtonClass* ToggleButton { nullptr };
	bool DisableEntry { false };

	static CommandClass* Commands[10];
};