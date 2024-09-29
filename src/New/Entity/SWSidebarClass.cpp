#include "SWSidebarClass.h"
#include "TacticalButtonClass.h"
#include "SWColumnClass.h"
#include "ToggleSWButtonClass.h"

#include <Ext/Sidebar/Body.h>
#include <Ext/Side/Body.h>

std::unique_ptr<SWSidebarClass> SWSidebarClass::Instance = nullptr;

static constexpr std::array <std::pair<int, const wchar_t*>, 114> KeyMap {
			std::make_pair(0x00, L"  "),
			std::make_pair(0x01, L"MouseLeft"),
			std::make_pair(0x02, L"MouseRight"),
			std::make_pair(0x03, L"Cancel"),
			std::make_pair(0x04, L"MouseCenter"),
			std::make_pair(0x08, L"Back"),
			std::make_pair(0x09, L"Tab"),
			std::make_pair(0x0C, L"Clear"),
			std::make_pair(0x0D, L"Enter"),
			std::make_pair(0x10, L"Shift"),
			std::make_pair(0x11, L"Ctrl"),
			std::make_pair(0x12, L"Alt"),
			std::make_pair(0x13, L"Pause"),
			std::make_pair(0x14, L"CapsLock"),
			std::make_pair(0x1B, L"Esc"),
			std::make_pair(0x20, L"Space"),
			std::make_pair(0x21, L"PageUp"),
			std::make_pair(0x22, L"PageDown"),
			std::make_pair(0x23, L"End"),
			std::make_pair(0x24, L"Home"),
			std::make_pair(0x25, L"Left"),
			std::make_pair(0x26, L"Up"),
			std::make_pair(0x27, L"Right"),
			std::make_pair(0x28, L"Down"),
			std::make_pair(0x29, L"Select"),
			std::make_pair(0x2A, L"Print"),
			std::make_pair(0x2B, L"Execute"),
			std::make_pair(0x2C, L"PrintScreen"),
			std::make_pair(0x2D, L"Insert"),
			std::make_pair(0x2E, L"Delete"),
			std::make_pair(0x2F, L"Help"),
			std::make_pair(0x30, L"0"),
			std::make_pair(0x31, L"1"),
			std::make_pair(0x32, L"2"),
			std::make_pair(0x33, L"3"),
			std::make_pair(0x34, L"4"),
			std::make_pair(0x35, L"5"),
			std::make_pair(0x36, L"6"),
			std::make_pair(0x37, L"7"),
			std::make_pair(0x38, L"8"),
			std::make_pair(0x39, L"9"),
			std::make_pair(0x41, L"A"),
			std::make_pair(0x42, L"B"),
			std::make_pair(0x43, L"C"),
			std::make_pair(0x44, L"D"),
			std::make_pair(0x45, L"E"),
			std::make_pair(0x46, L"F"),
			std::make_pair(0x47, L"G"),
			std::make_pair(0x48, L"H"),
			std::make_pair(0x49, L"I"),
			std::make_pair(0x4A, L"J"),
			std::make_pair(0x4B, L"K"),
			std::make_pair(0x4C, L"L"),
			std::make_pair(0x4D, L"M"),
			std::make_pair(0x4E, L"N"),
			std::make_pair(0x4F, L"O"),
			std::make_pair(0x50, L"P"),
			std::make_pair(0x51, L"Q"),
			std::make_pair(0x52, L"R"),
			std::make_pair(0x53, L"S"),
			std::make_pair(0x54, L"T"),
			std::make_pair(0x55, L"U"),
			std::make_pair(0x56, L"V"),
			std::make_pair(0x57, L"W"),
			std::make_pair(0x58, L"X"),
			std::make_pair(0x59, L"Y"),
			std::make_pair(0x5A, L"Z"),
			std::make_pair(0x5B, L"LWin"),
			std::make_pair(0x5C, L"RWin"),
			std::make_pair(0x5D, L"Menu"),
			std::make_pair(0x60, L"Num0"),
			std::make_pair(0x61, L"Num1"),
			std::make_pair(0x62, L"Num2"),
			std::make_pair(0x63, L"Num3"),
			std::make_pair(0x64, L"Num4"),
			std::make_pair(0x65, L"Num5"),
			std::make_pair(0x66, L"Num6"),
			std::make_pair(0x67, L"Num7"),
			std::make_pair(0x68, L"Num8"),
			std::make_pair(0x69, L"Num9"),
			std::make_pair(0x6A, L"Num*"),
			std::make_pair(0x6B, L"Num+"),
			std::make_pair(0x6C, L"Separator"),
			std::make_pair(0x6D, L"Num-"),
			std::make_pair(0x6E, L"Num."),
			std::make_pair(0x6F, L"Num/"),
			std::make_pair(0x70, L"F1"),
			std::make_pair(0x71, L"F2"),
			std::make_pair(0x72, L"F3"),
			std::make_pair(0x73, L"F4"),
			std::make_pair(0x74, L"F5"),
			std::make_pair(0x75, L"F6"),
			std::make_pair(0x76, L"F7"),
			std::make_pair(0x77, L"F8"),
			std::make_pair(0x78, L"F9"),
			std::make_pair(0x79, L"F10"),
			std::make_pair(0x7A, L"F11"),
			std::make_pair(0x7B, L"F12"),
			std::make_pair(0x90, L"NumLock"),
			std::make_pair(0x91, L"ScrollLock"),
			std::make_pair(0xBA, L";"),
			std::make_pair(0xBB, L"="),
			std::make_pair(0xBC, L","),
			std::make_pair(0xBD, L"-"),
			std::make_pair(0xBE, L"."),
			std::make_pair(0xBF, L"/"),
			std::make_pair(0xC0, L"`"),
			std::make_pair(0xDB, L"["),
			std::make_pair(0xDC, L"\\"),
			std::make_pair(0xDD, L"]"),
			std::make_pair(0xDE, L"'"),
			std::make_pair(static_cast<int>(WWKey::Shift), L"Shift"),
			std::make_pair(static_cast<int>(WWKey::Ctrl), L"Ctrl"),
			std::make_pair(static_cast<int>(WWKey::Alt), L"Alt")
};

// =============================
// functions

bool SWSidebarClass::AddColumn()
{
	auto& columns = this->Columns;

	if (static_cast<int>(columns.size()) >= Phobos::UI::ExclusiveSWSidebar_MaxColumn)
		return false;

	const auto column = DLLCreate<SWColumnClass>(TacticalButtonClass::StartID + SuperWeaponTypeClass::Array->Count + 1 + static_cast<int>(columns.size()), 0, 0, 60 + Phobos::UI::ExclusiveSWSidebar_Interval, 48);

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

		DLLCallDTOR(backColumn);
		columns.erase(columns.end() - 1);
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
		DLLCallDTOR(toggleButton);
	}

	auto& columns = this->Columns;

	for (auto& column : columns)
	{
		column->ClearButtons();
		GScreenClass::Instance->RemoveButton(column);
		DLLCallDTOR(column);
		column = nullptr;
	}

	columns.clear();
}

bool SWSidebarClass::AddButton(int superIdx)
{
	auto& columns = this->Columns;

	if (columns.empty() && !this->AddColumn())
		return false;

	if (std::any_of(columns.begin() , columns.end(),
		[superIdx](SWColumnClass* column) {
			return std::any_of(column->Buttons.begin() , column->Buttons.end(),
				[superIdx](TacticalButtonClass* button) {
					return button->SuperIndex == superIdx;
				});
		}
	)) {
		return true;
	}

	const bool success = columns.back()->AddButton(superIdx);

	if (success)
		SidebarExtData::Instance()->SWSidebar_Indices.AddUnique(superIdx);

	return success;
}

void SWSidebarClass::SortButtons()
{
	auto& columns = this->Columns;

	if (columns.empty())
		return;

	columns.erase(
			std::remove_if(columns.begin(), columns.end(),
				[](SWColumnClass* const column)
				{ return column->Buttons.empty(); }
			),
			columns.end()
	);

	if (columns.empty())
	{
		if (const auto& toggleButton = this->ToggleButton)
			toggleButton->UpdatePosition();

		return;
	}

	std::vector<TacticalButtonClass*> vec_Buttons;
	vec_Buttons.reserve(this->GetMaximumButtonCount());

	for (const auto& column : columns)
	{
		for (const auto button : column->Buttons)
			vec_Buttons.emplace_back(button);

		column->ClearButtons(false);
	}

	std::stable_sort(vec_Buttons.begin(), vec_Buttons.end(), [](TacticalButtonClass* const a, TacticalButtonClass* const b)
	{ return BuildType::SortsBefore(AbstractType::Special, a->SuperIndex, AbstractType::Special, b->SuperIndex); });

	const int buttonCount = static_cast<int>(vec_Buttons.size());
	const int cameoWidth = 60, cameoHeight = 48;
	const int maximum = Phobos::UI::ExclusiveSWSidebar_Max;
	Point2D location = { 0, (DSurface::ViewBounds().Height - std::min(buttonCount, maximum) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int rowIdx = 0, columnIdx = 0;

	for (const auto button : vec_Buttons)
	{
		const auto column = columns[columnIdx];

		if (rowIdx == 0)
			column->SetPosition(location.X, location.Y);

		column->Buttons.emplace_back(button);
		button->SetColumn(columnIdx);
		button->SetPosition(location.X, location.Y);
		rowIdx++;

		if (rowIdx >= maximum - columnIdx)
		{
			rowIdx = 0;
			columnIdx++;
			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth + Phobos::UI::ExclusiveSWSidebar_Interval, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	for (const auto& column : columns)
		column->SetHeight(column->Buttons.size() * 48);

	if (const auto toggleButton = this->ToggleButton)
		toggleButton->UpdatePosition();
}

static constexpr auto GetKeyName(int Key) {
	for (auto& map : KeyMap) {
		if (map.first == Key)
			return map.second;
	}

	return L"Unknown";
}

void SWSidebarClass::RecordHotkey(int buttonIndex, int key)
{
	const int index = buttonIndex - 1;

	if (this->KeyCodeData[index] == key)
		return;

	this->KeyCodeData[index] = key;
	std::wostringstream oss;

	if (key & static_cast<int>(WWKey::Shift)){
		constexpr auto key_str = GetKeyName(static_cast<int>(WWKey::Shift));
		oss << key_str << L"+";
	}

	if (key & static_cast<int>(WWKey::Ctrl)) {
		constexpr auto key_str = GetKeyName(static_cast<int>(WWKey::Ctrl));
		oss << key_str << L"+";
	}

	if (key & static_cast<int>(WWKey::Alt)) {
		constexpr auto key_str = GetKeyName(static_cast<int>(WWKey::Alt));
		oss << key_str << L"+";
	}

	oss << GetKeyName(key & 0xFF);
	this->KeyCodeText[index] = oss.str();
}

int SWSidebarClass::GetMaximumButtonCount()
{
	const int firstColumn = Phobos::UI::ExclusiveSWSidebar_Max;
	const int columns = std::min(firstColumn, Phobos::UI::ExclusiveSWSidebar_MaxColumn);
	return (firstColumn + (firstColumn - (columns - 1))) * columns / 2;
}

bool SWSidebarClass::IsEnabled()
{
	return SidebarExtData::Instance()->SWSidebar_Enable;
}

// Hooks

DEFINE_HOOK(0x692419, DisplayClass_ProcessClickCoords_SWSidebar, 0x7)
{
	enum { Nothing = 0x6925FC };

	if (SWSidebarClass::IsEnabled() && SWSidebarClass::Global()->CurrentColumn)
		return Nothing;

	const auto toggleButton = SWSidebarClass::Global()->ToggleButton;

	return toggleButton && toggleButton->IsHovering ? Nothing : 0;
}

DEFINE_HOOK(0x4F92FB, HouseClass_UpdateTechTree_SWSidebar, 0x7)
{
	enum { SkipGameCode = 0x4F9302 };

	GET(HouseClass*, pHouse, ESI);

	pHouse->UpdateSuperWeaponsUnavailable();
	if (pHouse->IsCurrentPlayer())
	{

		for (const auto& column : SWSidebarClass::Global()->Columns)
		{
			for (const auto& button : column->Buttons)
			{

				if (HouseClass::CurrentPlayer->Supers[button->SuperIndex]->Granted)
					continue;

				if (column->RemoveButton(button->SuperIndex))
					SidebarExtData::Instance()->SWSidebar_Indices.Remove(button->SuperIndex);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6A6300, SidebarClass_AddCameo_SuperWeapon_SWSidebar, 0x6)
{
	enum { SkipGameCode = 0x6A6606 };

	GET_STACK(AbstractType, whatAmI, 0x4);
	GET_STACK(int, index, 0x8);

	switch (whatAmI)
	{
	case AbstractType::Super:
	case AbstractType::SuperWeaponType:
	case AbstractType::Special:
		if (SWSidebarClass::Global()->AddButton(index))
		{
			R->AL(false);
			return SkipGameCode;
		}
		break;

	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x6A5082, SidebarClass_Init_Clear_InitializeSWSidebar, 0x5)
{
	SWSidebarClass::Global()->Init_Clear();
	return 0;
}

DEFINE_HOOK(0x6A5839, SidebarClass_Init_IO_InitializeSWSidebar, 0x5)
{
	if (!Phobos::UI::ExclusiveSWSidebar)
		return 0;

	const auto pSideExt = SideExtContainer::Instance.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]);

	if (const auto toggleShape = pSideExt->ExclusiveSWSidebar_ToggleShape.Get())
	{
		if (const auto toggleButton = DLLCreate<ToggleSWButtonClass>(TacticalButtonClass::StartID + SuperWeaponTypeClass::Array->Count, 0, 0, toggleShape->Width, toggleShape->Height))
		{
			toggleButton->Zap();
			GScreenClass::Instance->AddButton(toggleButton);
			SWSidebarClass::Global()->ToggleButton = toggleButton;
			toggleButton->UpdatePosition();
		}
	}

	for (const auto& superIdx : SidebarExtData::Instance()->SWSidebar_Indices)
		SWSidebarClass::Global()->AddButton(superIdx);

	return 0;
}

// Shortcuts keys hooks
DEFINE_HOOK(0x533E69, UnknownClass_sub_533D20_LoadKeyboardCodeFromINI, 0x6)
{
	GET(CommandClass*, pCommand, ESI);
	GET(int, key, EDI);

	const char* name = pCommand->GetName();
	char buffer[29];

	for (int idx = 1; idx <= 10; idx++) {
		sprintf_s(buffer, "SW Sidebar Shortcuts Num %02d", idx);

		if (IS_SAME_STR_(name,buffer))
			SWSidebarClass::Global()->RecordHotkey(idx, key);
	}

	return 0;
}

DEFINE_HOOK(0x5FB992, UnknownClass_sub_5FB320_SaveKeyboardCodeToINI, 0x6)
{
	GET(CommandClass*, pCommand, ECX);
	GET(int, key, EAX);

	const char* name = pCommand->GetName();
	char buffer[30];

	for (int idx = 1; idx <= 10; idx++) {
		sprintf_s(buffer, "SW Sidebar Shortcuts Num %02d", idx);

		if (IS_SAME_STR_(name, buffer))
			SWSidebarClass::Global()->RecordHotkey(idx, key);
	}

	return 0;
}