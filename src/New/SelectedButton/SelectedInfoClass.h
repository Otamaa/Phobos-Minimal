#pragma once
#include "SelectedColumnClass.h"
#include "SelectedCameoClass.h"
#include "SelectedButtonClass.h"

#include <Utilities/VectorHelper.h>
#include <Utilities/Enum.h>

class TechnoClass;
class TechnoExtData;
struct SHPCaches;
class TechnoTypeExtData;
class BSurface;
class ObjectTypeClass;
struct SHPFile;
class SelectedInfoClass
{
public:
	static SelectedInfoClass Instance;
	static constexpr const wchar_t* Status[35] =
	{
		L"Sleep", L"Attack", L"Move", L"QueueMove", L"Retreat",
		L"Guard", L"Sticky", L"Enter", L"Capture", L"Eaten",
		L"Harvest", L"AreaGuard", L"Return", L"Stop", L"Ambush",
		L"Hunt", L"Unload", L"Sabotage", L"Construction", L"Selling",
		L"Repair", L"Rescue", L"Missile", L"Harmless", L"Open",
		L"Patrol", L"Paradrop", L"AttackMove", L"Wait", L"Produce",
		L"Deactive", L"Locomotor", L"FollowGuard", L"Unknown", L"None"
	};

	static constexpr const char* StatusEntry[35] =
	{
		"Status:Sleep", "Status:Attack", "Status:Move", "Status:QueueMove", "Status:Retreat",
		"Status:Guard", "Status:Sticky", "Status:Enter", "Status:Capture", "Status:Eaten",
		"Status:Harvest", "Status:AreaGuard", "Status:Return", "Status:Stop", "Status:Ambush",
		"Status:Hunt", "Status:Unload", "Status:Sabotage", "Status:Construction", "Status:Selling",
		"Status:Repair", "Status:Rescue", "Status:Missile", "Status:Harmless", "Status:Open",
		"Status:Patrol", "Status:Paradrop", "Status:AttackMove", "Status:Wait", "Status:Produce",
		"Status:Deactive", "Status:Locomotor", "Status:FollowGuard", "Status:Unknown", "Status:None"
	};

	struct SelectRecordStruct
	{
		TechnoTypeExtData* TypeExt { nullptr };
		int Count { 0 };
	};

	void InitClear();
	void InitIO();

	void SwitchExpand();
	void SwitchVisible();
	void UpdateVisible();
	void UpdateSelected();
	void UpdateRecordCameo(const SelectRecordStruct& Record);
	void DrawInfo();

	static BSurface* SearchMissingCameo(AbstractType absType, SHPCaches* pSHP);
	static void GetValuesForDisplay(TechnoClass* pThis, ObjectTypeClass* pFakeType, DisplayInfoType infoType, int& value, int& maxValue, int infoIndex);
	static TechnoStatus GetCurrentStatus(TechnoClass* pThis);

	int GetMaxCameo() const;
	bool CanScrollLeft() const;
	bool CanScrollRight() const;
	bool ScrollLeft();
	bool ScrollRight();

	SelectedColumnClass* MainColumn { nullptr };

	SelectedButtonClass* PushButton { nullptr };
	SelectedButtonClass* AmmoButton { nullptr };

	SelectedMainCameoClass* MainCameo { nullptr };

	SelectedNotButtonClass* InfoIconA { nullptr };
	SelectedNotButtonClass* InfoIconD { nullptr };
	SelectedNotButtonClass* InfoIconS { nullptr };

	SelectedBottomClass* MainBottom { nullptr };

	SelectedToggleClass* ToggleV { nullptr };
	SelectedToggleClass* ToggleE { nullptr };
	SelectedScrollClass* ScrollL { nullptr };
	SelectedScrollClass* ScrollR { nullptr };

	SelectedCameoClass* Cameos[20] { };
	HelperedVector<SelectRecordStruct> CurrentSelectCameo { };
	HelperedVector<TechnoExtData*> CurrentSelectTechno { };
	int MaxCameo { 0 };
	int Current { 0 };

	bool ShouldUpdate { false };
	bool SingleSelect { true };
	bool ObtainSelect { false };
	bool IsHovering { false };

public:

	static BSurface* MissingCameo;
	static SHPFile* SelectedInfo_Main;
	static SHPFile* SelectedInfo_Buff;
	static SHPFile* SelectedInfo_Button;
	static SHPFile* SelectedInfo_Bottom;
	static SHPFile* SelectedInfo_Toggle;
};
