#pragma once
#include "SelectedColumnClass.h"
#include "SelectedCameoClass.h"
#include "SelectedButtonClass.h"

#include <Utilities/VectorHelper.h>
#include <Utilities/Enum.h>

class TechnoClass;
struct SHPCaches;
class TechnoTypeExtData;
class BSurface;
class ObjectTypeClass;
struct SHPFile;
class SelectedInfoClass
{
public:
	static SelectedInfoClass Instance;

	static const wchar_t* Status[35];
	static const char* StatusEntry[35];

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
	HelperedVector<TechnoClass*> CurrentSelectTechno { };
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
