#include "SWSidebarClass.h"
#include "SWButtonClass.h"
#include "SWColumnClass.h"
#include "ToggleSWButtonClass.h"

#include <Ext/Sidebar/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Rules/Body.h>

#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <CommandClass.h>
#include <HouseClass.h>

// =============================
// functions
SWSidebarClass SWSidebarClass::Instance;
CommandClass* SWSidebarClass::Commands[10];

bool SWSidebarClass::AddColumn()
{
	auto& columns = this->Columns;
	const int columnsCount = static_cast<int>(columns.size());

	if (static_cast<int>(columnsCount) >= Phobos::UI::SuperWeaponSidebar_MaxColumns)
		return false;

	const int firstColumn = Phobos::UI::SuperWeaponSidebar_Max;
	const int maxButtons = Phobos::UI::SuperWeaponSidebar_Pyramid ? firstColumn - columnsCount : firstColumn;

	if (maxButtons <= 0)
		return false;

	const int cameoWidth = 60;
	const auto column = GameCreate<SWColumnClass>(maxButtons, 0, 0, cameoWidth + Phobos::UI::SuperWeaponSidebar_Interval, Phobos::UI::SuperWeaponSidebar_CameoHeight);

	column->Zap();
	GScreenClass::Instance->AddButton(column);
	return true;
}

bool SWSidebarClass::RemoveColumn()
{
	auto& columns = this->Columns;

	if (columns.empty())
		return false;

	if (const auto backColumn = columns.back())
	{
		AnnounceInvalidPointer(SWSidebarClass::Global()->CurrentColumn, backColumn);
		GScreenClass::Instance->RemoveButton(backColumn);
		GameDelete<true, false>(backColumn);
		columns.pop_back();
		return true;
	}

	return false;
}

void SWSidebarClass::InitClear()
{
	this->CurrentColumn = nullptr;
	this->CurrentButton = nullptr;

	if (const auto toggleButton = std::exchange(this->ToggleButton, nullptr))
	{
		GScreenClass::Instance->RemoveButton(toggleButton);
		GameDelete<true, false>(toggleButton);
	}

	for (auto& column : this->Columns) {
		column->ClearButtons();
		GScreenClass::Instance->RemoveButton(column);
		GameDelete<true, false>(column);
	}

	this->Columns.clear();
}

void SWSidebarClass::InitIO()
{
	if (!Phobos::UI::SuperWeaponSidebar || Unsorted::ArmageddonMode)
		return;

	const auto pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex);

	if (pSide) {
		const auto pSideExt = SideExtContainer::Instance.Find(pSide);
		const auto pOnPCX = pSideExt->SuperWeaponSidebar_OnPCX.GetSurface();
		const auto pOffPCX = pSideExt->SuperWeaponSidebar_OffPCX.GetSurface();
		int width = 0, height = 0;

		if (pOnPCX)
		{
			if (pOffPCX)
			{
				width = std::max(pOnPCX->Get_Width(), pOffPCX->Get_Width());
				height = std::max(pOnPCX->Get_Height(), pOffPCX->Get_Height());
			}
			else
			{
				width = pOnPCX->Get_Width();
				height = pOnPCX->Get_Height();
			}
		}
		else if (pOffPCX)
		{
			width = pOffPCX->Get_Width();
			height = pOffPCX->Get_Height();
		}

		if (width > 0 && height > 0)
		{
			if (const auto toggleButton = GameCreate<ToggleSWButtonClass>(0, 0, width, height))
			{
				toggleButton->Zap();
				GScreenClass::Instance->AddButton(toggleButton);
				SWSidebarClass::Global()->ToggleButton = toggleButton;
				toggleButton->UpdatePosition();
			}
		}
	}

	for (const auto &superIdx : ScenarioExtData::Instance()->SWSidebar_Indices)
		SWSidebarClass::Instance.AddButton(superIdx);
}

bool SWSidebarClass::AddButton(int superIdx)
{
	if (!Phobos::UI::SuperWeaponSidebar || this->DisableEntry)
		return false;

	const auto pSWType = SuperWeaponTypeClass::Array->GetItemOrDefault(superIdx);
	if (!pSWType)
		return false;

	const auto pSWExt = SWTypeExtContainer::Instance.Find(pSWType);

	if (!pSWExt->SW_ShowCameo || !pSWExt->SuperWeaponSidebar_Allow.Get(RulesExtData::Instance()->SuperWeaponSidebar_AllowByDefault))
		return false;

	const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

	if ((pSWExt->SuperWeaponSidebar_RequiredHouses & ownerBits) == 0)
		return false;

	if (pSWExt->SuperWeaponSidebar_Significance < Phobos::Config::SuperWeaponSidebar_RequiredSignificance)
		return false;

	auto& columns = this->Columns;

	for (auto& col : columns) {
		if (std::any_of(col->Buttons.begin() , col->Buttons.end(),
			[superIdx](SWButtonClass* const button) { return button->SuperIndex == superIdx; }))
			return true; //already exist
	}
	if (columns.empty() && !this->AddColumn())
		return false;

	return columns.back()->AddButton(superIdx);
}

void SWSidebarClass::SortButtons()
{
	auto& columns = this->Columns;

	if (columns.empty())
	{
		if (const auto toggleButton = this->ToggleButton)
			toggleButton->UpdatePosition();

		return;
	}

	std::vector<SWButtonClass*> vec_Buttons;
	vec_Buttons.reserve(this->GetMaximumButtonCount());

	for (const auto& column : columns)
	{
		for (const auto& button : column->Buttons)
			vec_Buttons.emplace_back(button);

		column->ClearButtons(false);
	}

	const unsigned int ownerBits = 1u << HouseClass::CurrentPlayer->Type->ArrayIndex;

	std::stable_sort(vec_Buttons.begin(), vec_Buttons.end(), [ownerBits](SWButtonClass* const a, SWButtonClass* const b)
	{
		const auto pExtA = SWTypeExtContainer::Instance.TryFind(SuperWeaponTypeClass::Array->GetItemOrDefault(a->SuperIndex));
		const auto pExtB = SWTypeExtContainer::Instance.TryFind(SuperWeaponTypeClass::Array->GetItemOrDefault(b->SuperIndex));

		if (pExtB && (pExtB->SuperWeaponSidebar_PriorityHouses & ownerBits) && (!pExtA || !(pExtA->SuperWeaponSidebar_PriorityHouses & ownerBits)))
			return false;

		if ((!pExtB || !(pExtB->SuperWeaponSidebar_PriorityHouses & ownerBits)) && pExtA && (pExtA->SuperWeaponSidebar_PriorityHouses & ownerBits))
			return true;

		return BuildType::SortsBefore(AbstractType::Special, a->SuperIndex, AbstractType::Special, b->SuperIndex);
	});

	const int buttonCount = static_cast<int>(vec_Buttons.size());
	const int cameoWidth = 60, cameoHeight = 48;
	const int firstColumn = Phobos::UI::SuperWeaponSidebar_Max;
	const int cameoHarfInterval = (Phobos::UI::SuperWeaponSidebar_CameoHeight - cameoHeight) / 2;
	int location_Y = (DSurface::ViewBounds->Height - std::min(buttonCount, firstColumn) * Phobos::UI::SuperWeaponSidebar_CameoHeight) / 2;
	Point2D location = { Phobos::UI::SuperWeaponSidebar_LeftOffset, location_Y + cameoHarfInterval };
	int rowIdx = 0, columnIdx = 0;

	for (const auto button : vec_Buttons)
	{
		const auto column = columns[columnIdx];

		if (rowIdx == 0)
		{
			const auto pTopPCX = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex])->SuperWeaponSidebar_TopPCX.GetSurface();
			column->SetPosition(location.X - Phobos::UI::SuperWeaponSidebar_LeftOffset, location_Y - (pTopPCX ? pTopPCX->Get_Height() : 0));
		}

		column->Buttons.emplace_back(button);
		button->SetColumn(columnIdx);
		button->SetPosition(location.X, location.Y);
		rowIdx++;

		const int currentCapacity = Phobos::UI::SuperWeaponSidebar_Pyramid ? firstColumn - columnIdx : firstColumn;

		if (rowIdx >= currentCapacity)
		{
			rowIdx = 0;
			columnIdx++;

			if (Phobos::UI::SuperWeaponSidebar_Pyramid)
				location_Y += Phobos::UI::SuperWeaponSidebar_CameoHeight / 2;

			location.X += cameoWidth + Phobos::UI::SuperWeaponSidebar_Interval;
			location.Y = location_Y + cameoHarfInterval;
		}
		else
		{
			location.Y += Phobos::UI::SuperWeaponSidebar_CameoHeight;
		}
	}

	for (const auto& column : columns)
		column->SetHeight(column->Buttons.size() * Phobos::UI::SuperWeaponSidebar_CameoHeight);
}

int SWSidebarClass::GetMaximumButtonCount()
{
	const int firstColumn = Phobos::UI::SuperWeaponSidebar_Max;

	if (Phobos::UI::SuperWeaponSidebar_Pyramid)
	{
		const int columns = std::min(firstColumn, Phobos::UI::SuperWeaponSidebar_MaxColumns);
		return (firstColumn + (firstColumn - (columns - 1))) * columns / 2;
	}

	return firstColumn * Phobos::UI::SuperWeaponSidebar_MaxColumns;
}

bool SWSidebarClass::IsEnabled()
{
	return ScenarioExtData::Instance()->SWSidebar_Enable;
}

void SWSidebarClass::RecheckCameo()
{
	auto sidebar = SWSidebarClass::Global();

	for (const auto& column : sidebar->Columns)
	{
		std::vector<int> removeButtons;
		removeButtons.reserve(column->Buttons.size());

		for (const auto& button : column->Buttons)
		{
			if (HouseClass::CurrentPlayer->Supers[button->SuperIndex]->Granted)
				continue;

			removeButtons.push_back(button->SuperIndex);
		}

		if (removeButtons.size())
			HouseClass::CurrentPlayer->RecheckTechTree = true;

		for (const auto& index : removeButtons)
			column->RemoveButton(index);
	}

	sidebar->SortButtons();
	int removes = 0;

	for (const auto& column : sidebar->Columns)
	{
		if (column->Buttons.empty())
			++removes;
	}

	for (; removes > 0; --removes)
		sidebar->RemoveColumn();

	if (const auto toggleButton = sidebar->ToggleButton)
		toggleButton->UpdatePosition();
}

// Hooks

ASMJIT_PATCH(0x692419, DisplayClass_ProcessClickCoords_SWSidebar, 0x7)
{
	enum { Nothing = 0x6925FC };

	if (SWSidebarClass::IsEnabled() && SWSidebarClass::Global()->CurrentColumn)
		return Nothing;

	const auto toggleButton = SWSidebarClass::Global()->ToggleButton;

	return toggleButton && toggleButton->IsHovering ? Nothing : 0;
}

ASMJIT_PATCH(0x4F92FB, HouseClass_UpdateTechTree_SWSidebar, 0x7)
{
	enum { SkipGameCode = 0x4F9302 };

	GET(HouseClass*, pHouse, ESI);

	pHouse->UpdateSuperWeaponsUnavailable();

	if (pHouse->IsCurrentPlayer())
		SWSidebarClass::RecheckCameo();

	return SkipGameCode;
}

ASMJIT_PATCH(0x6A6316, SidebarClass_AddCameo_SuperWeapon_SWSidebar, 0x6)
{
	enum { ReturnFalse = 0x6A65FF };

	GET_STACK(AbstractType, whatAmI, STACK_OFFSET(0x14, 0x4));
	GET_STACK(int, index, STACK_OFFSET(0x14, 0x8));

	if (whatAmI != AbstractType::Special
			&& whatAmI != AbstractType::SuperWeaponType
			&& whatAmI != AbstractType::Super)
		return 0;

	if (SWSidebarClass::Global()->AddButton(index)) {
		ScenarioExtData::Instance()->SWSidebar_Indices.emplace(index);
		return ReturnFalse;
	}

	return 0;
}

ASMJIT_PATCH(0x6A5082, SidebarClass_Init_Clear_InitializeSWSidebar, 0x5)
{
	SWSidebarClass::Global()->InitClear();
	return 0;
}

ASMJIT_PATCH(0x6A5839, SidebarClass_Init_IO_InitializeSWSidebar, 0x5)
{
	SWSidebarClass::Global()->InitIO();
	return 0;
}

//hmmm ??
ASMJIT_PATCH(0x6AA790, StripClass_RecheckCameo_RemoveCameo, 0x6)
{
	enum { ShouldRemove = 0x6AA7B6, ShouldNotRemove = 0x6AAA68 };

	GET(BuildType*, pItem, ESI);

	const auto pCurrent = HouseClass::CurrentPlayer();
	const auto& supers = pCurrent->Supers;

	if (supers.ValidIndex(pItem->ItemIndex) && supers[pItem->ItemIndex]->Granted)
	{
		if (SWSidebarClass::Global()->AddButton(pItem->ItemIndex))
			ScenarioExtData::Instance()->SWSidebar_Indices.emplace(pItem->ItemIndex);
		else
			return ShouldNotRemove;
	}

	return ShouldRemove;
}

// Shortcuts keys hooks
//ASMJIT_PATCH(0x533E69, UnknownClass_sub_533D20_LoadKeyboardCodeFromINI, 0x6)
//{
//	GET(CommandClass*, pCommand, ESI);
//	GET(int, key, EDI);
//
//	const char* name = pCommand->GetName();
//	char buffer[29];
//
//	for (int idx = 1; idx <= 10; idx++) {
//		sprintf_s(buffer, "SW Sidebar Shortcuts Num %02d", idx);
//
//		if (IS_SAME_STR_(name,buffer))
//			SWSidebarClass::Global()->RecordHotkey(idx, key);
//	}
//
//	return 0;
//}

//ASMJIT_PATCH(0x5FB992, UnknownClass_sub_5FB320_SaveKeyboardCodeToINI, 0x6)
//{
//	GET(CommandClass*, pCommand, ECX);
//	GET(int, key, EAX);
//
//	const char* name = pCommand->GetName();
//	char buffer[30];
//
//	for (int idx = 1; idx <= 10; idx++) {
//		sprintf_s(buffer, "SW Sidebar Shortcuts Num %02d", idx);
//
//		if (IS_SAME_STR_(name, buffer))
//			SWSidebarClass::Global()->RecordHotkey(idx, key);
//	}
//
//	return 0;
//}