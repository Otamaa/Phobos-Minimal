#include "SWSidebarClass.h"
#include "SWButtonClass.h"
#include "SWColumnClass.h"
#include "ToggleSWButtonClass.h"

#include <Ext/Sidebar/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>

#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <CommandClass.h>

// =============================
// functions
std::unique_ptr<SWSidebarClass> SWSidebarClass::Instance;
CommandClass* SWSidebarClass::Commands[10];

bool SWSidebarClass::AddColumn()
{
	auto& columns = this->Columns;

	if (static_cast<int>(columns.size()) >= Phobos::UI::SuperWeaponSidebar_MaxColumns)
		return false;

	const int cameoWidth = 60;
	const auto column = GameCreate<SWColumnClass>(SWButtonClass::StartID + SuperWeaponTypeClass::Array->Count + 1 + static_cast<int>(columns.size()), 0, 0, cameoWidth + Phobos::UI::SuperWeaponSidebar_Interval, Phobos::UI::SuperWeaponSidebar_CameoHeight);

	if (!column)
		return false;

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
		columns.pop_back();
		return true;
	}

	return false;
}

void SWSidebarClass::Init_Clear()
{
	this->CurrentColumn = nullptr;
	this->CurrentButton = nullptr;

	if (const auto toggleButton = std::exchange(this->ToggleButton, nullptr))
	{
		GScreenClass::Instance->RemoveButton(toggleButton);
	}

	auto& columns = this->Columns;

	for (auto& column : columns)
	{
		column->ClearButtons();
		GScreenClass::Instance->RemoveButton(column);
	}

	columns.clear();
}

bool SWSidebarClass::AddButton(int superIdx)
{
	auto& columns = this->Columns;

	if (columns.empty() && !this->AddColumn())
		return false;

	if (std::any_of(columns.begin() , columns.end(), [superIdx](SWColumnClass* column) {
		return std::any_of(column->Buttons.begin() , column->Buttons.end() , [superIdx](SWButtonClass* button) {
			return button->SuperIndex == superIdx;
		});
	})
	) {
		return true;
	}

	const bool success = columns.back()->AddButton(superIdx);

	if (success)
		SidebarExtData::Instance()->SWSidebar_Indices.push_back_unique(superIdx);

	return success;
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

	for (const auto column : columns)
	{
		for (const auto button : column->Buttons)
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
	const int maximum = Phobos::UI::SuperWeaponSidebar_Max;
	const int cameoHarfInterval = (Phobos::UI::SuperWeaponSidebar_CameoHeight - cameoHeight) / 2;
	int location_Y = (DSurface::ViewBounds().Height - std::min(buttonCount, maximum) * Phobos::UI::SuperWeaponSidebar_CameoHeight) / 2;
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

		if (rowIdx >= maximum - columnIdx)
		{
			rowIdx = 0;
			columnIdx++;
			location_Y += Phobos::UI::SuperWeaponSidebar_CameoHeight / 2;
			location.X += cameoWidth + Phobos::UI::SuperWeaponSidebar_Interval;
			location.Y = location_Y + cameoHarfInterval;
		}
		else
		{
			location.Y += Phobos::UI::SuperWeaponSidebar_CameoHeight;
		}
	}

	for (const auto column : columns)
		column->SetHeight(column->Buttons.size() * Phobos::UI::SuperWeaponSidebar_CameoHeight);
}

int SWSidebarClass::GetMaximumButtonCount()
{
	const int firstColumn = Phobos::UI::SuperWeaponSidebar_Max;
	const int columns = std::min(firstColumn, Phobos::UI::SuperWeaponSidebar_MaxColumns);
	return (firstColumn + (firstColumn - (columns - 1))) * columns / 2;
}

bool SWSidebarClass::IsEnabled()
{
	return SidebarExtData::Instance()->SWSidebar_Enable;
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

	if (Phobos::UI::SuperWeaponSidebar &&
		pHouse->IsCurrentPlayer())
	{
		for (const auto& column : SWSidebarClass::Global()->Columns)
		{
			std::vector<int> removeButtons;
			removeButtons.reserve(column->Buttons.size());

			for (const auto& button : column->Buttons)
			{
				if (HouseClass::CurrentPlayer->Supers[button->SuperIndex]->Granted)
					continue;

				removeButtons.push_back(button->SuperIndex);
			}

			for (const auto& index : removeButtons)
			{
				if (column->RemoveButton(index))
					SidebarExtData::Instance()->SWSidebar_Indices.remove(index);
			}
		}

		SWSidebarClass::Global()->SortButtons();
		int removes = 0;

		for (const auto& column : SWSidebarClass::Global()->Columns)
		{
			if (column->Buttons.empty())
				++removes;
		}

		for (; removes > 0; --removes)
			SWSidebarClass::Global()->RemoveColumn();

		if (const auto toggleButton = SWSidebarClass::Global()->ToggleButton)
			toggleButton->UpdatePosition();
	}

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

	if (SWSidebarClass::Global()->AddButton(index))
		return ReturnFalse;

	return 0;
}

ASMJIT_PATCH(0x6A5082, SidebarClass_Init_Clear_InitializeSWSidebar, 0x5)
{
	if (!SWSidebarClass::Global())
		SWSidebarClass::Allocate();

	SWSidebarClass::Global()->Init_Clear();
	return 0;
}

ASMJIT_PATCH(0x6A5839, SidebarClass_Init_IO_InitializeSWSidebar, 0x5)
{
	if (!Phobos::UI::SuperWeaponSidebar || Unsorted::ArmageddonMode || ScenarioClass::Instance->PlayerSideIndex < 0)
		return 0;

	{
		const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);
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
			if (const auto toggleButton = GameCreate<ToggleSWButtonClass>(SWButtonClass::StartID + SuperWeaponTypeClass::Array->Count, 0, 0, width, height))
			{
				toggleButton->Zap();
				GScreenClass::Instance->AddButton(toggleButton);
				SWSidebarClass::Global()->ToggleButton = toggleButton;
				toggleButton->UpdatePosition();
			}
		}
	}

	for (const auto superIdx : SidebarExtData::Instance()->SWSidebar_Indices)
		SWSidebarClass::Global()->AddButton(superIdx);

	return 0;
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