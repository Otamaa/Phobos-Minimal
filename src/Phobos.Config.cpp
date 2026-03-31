#include "Phobos.h"

#include <CCINIClass.h>

#include <Utilities/Debug.h>
#include <Utilities/GameConfig.h>
#include <Utilities/GeneralUtils.h>

#include <Misc/Patches.h>

#include <GameStrings.h>
#include <GameOptionsClass.h>
#include <StringTable.h>

void Phobos::Config::Read_RA2MD(){
	auto const& pRA2MD = CCINIClass::INI_RA2MD;

	Phobos::Config::ToolTipDescriptions = pRA2MD->ReadBool(PHOBOS_STR, "ToolTipDescriptions", Phobos::Config::ToolTipDescriptions);
	Phobos::Config::ToolTipBlur = pRA2MD->ReadBool(PHOBOS_STR, "ToolTipBlur", Phobos::Config::ToolTipBlur);
	Phobos::Config::PrioritySelectionFiltering = pRA2MD->ReadBool(PHOBOS_STR, "PrioritySelectionFiltering", Phobos::Config::PrioritySelectionFiltering);
	Phobos::Config::PriorityDeployFiltering = pRA2MD->ReadBool(PHOBOS_STR, "PriorityDeployFiltering", Phobos::Config::PriorityDeployFiltering);
	Phobos::Config::TypeSelectUseIFVMode = pRA2MD->ReadBool(PHOBOS_STR, "TypeSelectUseIFVMode", Phobos::Config::TypeSelectUseIFVMode);
	Phobos::Config::EnableBuildingPlacementPreview = pRA2MD->ReadBool(PHOBOS_STR, "ShowBuildingPlacementPreview", Phobos::Config::EnableBuildingPlacementPreview);
	Phobos::Config::EnableSelectBox = pRA2MD->ReadBool(PHOBOS_STR, "EnableSelectBox", Phobos::Config::EnableSelectBox);

	//Phobos::Config::RealTimeTimers = pRA2MD->ReadBool(PHOBOS_STR, "RealTimeTimers", Phobos::Config::RealTimeTimers);
	//Phobos::Config::RealTimeTimers_Adaptive = pRA2MD->ReadBool(PHOBOS_STR, "RealTimeTimers.Adaptive", Phobos::Config::RealTimeTimers_Adaptive);
	Phobos::Config::DigitalDisplay_Enable = pRA2MD->ReadBool(PHOBOS_STR, "DigitalDisplay.Enable", Phobos::Config::DigitalDisplay_Enable);
	Phobos::Config::MessageDisplayInCenter = pRA2MD->ReadBool(PHOBOS_STR, "MessageDisplayInCenter", Phobos::Config::MessageDisplayInCenter);
	Phobos::Config::MessageApplyHoverState =  pRA2MD->ReadBool(PHOBOS_STR, "MessageApplyHoverState", Phobos::Config::MessageApplyHoverState );
	Phobos::Config::MessageDisplayInCenter_BoardOpacity =  pRA2MD->ReadInteger(PHOBOS_STR, "MessageDisplayInCenter.BoardOpacity", 	Phobos::Config::MessageDisplayInCenter_BoardOpacity );
	Phobos::Config::MessageDisplayInCenter_LabelsCount = pRA2MD->ReadInteger(PHOBOS_STR, "MessageDisplayInCenter.LabelsCount", Phobos::Config::MessageDisplayInCenter_LabelsCount);
	Phobos::Config::MessageDisplayInCenter_RecordsCount = pRA2MD->ReadInteger(PHOBOS_STR, "MessageDisplayInCenter.RecordsCount", Phobos::Config::MessageDisplayInCenter_RecordsCount);

	Phobos::Config::ShowBuildingStatistics = pRA2MD->ReadBool(PHOBOS_STR, "ShowBuildingStatistics", Phobos::Config::ShowBuildingStatistics);
	Phobos::Config::ShowFlashOnSelecting = pRA2MD->ReadBool(PHOBOS_STR, "ShowFlashOnSelecting", Phobos::Config::ShowFlashOnSelecting);
	Phobos::Config::SuperWeaponSidebar_RequiredSignificance = pRA2MD->ReadInteger(PHOBOS_STR, "SuperWeaponSidebar.RequiredSignificance", Phobos::Config::SuperWeaponSidebar_RequiredSignificance);
	Phobos::Config::HideLightFlashEffects = pRA2MD->ReadBool(PHOBOS_STR, "HideLightFlashEffects", Phobos::Config::HideLightFlashEffects);
	Phobos::Config::HideLaserTrailEffects = pRA2MD->ReadBool(PHOBOS_STR, "HideLaserTrailEffects", Phobos::Config::HideLaserTrailEffects);
	Phobos::Config::HideShakeEffects = pRA2MD->ReadBool(PHOBOS_STR, "HideShakeEffects", Phobos::Config::HideShakeEffects);
	Phobos::Config::SaveGameOnScenarioStart = pRA2MD->ReadBool(PHOBOS_STR, "SaveGameOnScenarioStart", Phobos::Config::SaveGameOnScenarioStart);

	Phobos::Config::ApplyNoMoveCommand =  pRA2MD->ReadBool(PHOBOS_STR, "DefaultApplyNoMoveCommand", true);
	Phobos::Config::DistributionSpreadMode =  pRA2MD->ReadInteger(PHOBOS_STR, "DefaultDistributionSpreadMode", 2);
	Phobos::Config::DistributionSpreadMode = std::clamp(Phobos::Config::DistributionSpreadMode, 0, 3);
	Phobos::Config::DistributionFilterMode = pRA2MD->ReadInteger(PHOBOS_STR, "DefaultDistributionFilterMode", 2);
	Phobos::Config::DistributionFilterMode = std::clamp(Phobos::Config::DistributionFilterMode, 0, 3);

	if (!Phobos::Otamaa::IsAdmin)
	{
		// Custom game speeds, 6 - i so that GS6 is index 0, just like in the engine
		Phobos::Config::CampaignDefaultGameSpeed = 6 - pRA2MD->ReadInteger(PHOBOS_STR, "CampaignDefaultGameSpeed", 4);

		if (Phobos::Config::CampaignDefaultGameSpeed > 6 || Phobos::Config::CampaignDefaultGameSpeed < 0)
			Phobos::Config::CampaignDefaultGameSpeed = 2;

		{
			BYTE defaultspeed = (BYTE)Phobos::Config::CampaignDefaultGameSpeed;
			// We overwrite the instructions that force GameSpeed to 2 (GS4)
			Patch::Apply_RAW(0x55D77A, sizeof(defaultspeed), PatchType::PATCH_ , &defaultspeed);

			// when speed control is off. Doesn't need a hook.
			Patch::Apply_RAW(0x55D78D, sizeof(defaultspeed), PatchType::PATCH_, &defaultspeed);
		}
	}

}

void Phobos::Config::Read_UIMD(){
	GameConfig UIMD { GameStrings::UIMD_INI() };

	UIMD.OpenINIAction([](CCINIClass* pINI) {
		 Debug::Log("Loading early %s file.\n", GameStrings::UIMD_INI());
	
		 {
			 Debug::LogInfo("--------- Loading Ares global settings -----------");
			 auto Ini = pINI;

			 if (Ini)
			 {
				 auto const section2 = GameStrings::Colors();
				 colorCount = std::clamp(Ini->ReadInteger(section2, "Count", colorCount), 8, 17);

				 auto const ParseColorInt = [&Ini](const char* section, const char* key, int defColor) -> int
					 {
						 ColorStruct ndefault(defColor & 0xFF, (defColor >> 8) & 0xFF, (defColor >> 16) & 0xFF);
						 auto const color = Ini->ReadColor(section, key, ndefault);
						 return color.R | color.G << 8 | color.B << 16;
					 };

				 auto const section = "UISettings";

				 auto const ReadColor = [&Ini, section2, ParseColorInt]
				 (
					 const std::string& name,
					 ColorData& value,
					 int colorRGB,
					 const char* defTooltip,
					 const char* defColorScheme
				 )
					 {
						 // load the tooltip string

						 if (Ini->ReadString(section2, (name + ".Tooltip").c_str(), defTooltip, Phobos::readBuffer))
							 value.sttToolTipSublineText = StringTable::FetchString(Phobos::readBuffer);

						 if (Ini->ReadString(section2, (name + ".ColorScheme").c_str(), defColorScheme, Phobos::readBuffer))
							 PhobosCRT::strCopy(value.colorScheme, Phobos::readBuffer);

						 value.colorRGB = ParseColorInt(section2, (name + ".DisplayColor").c_str(), colorRGB);
						 value.colorSchemeIndex = -1;
						 value.selectedIndex = -1;
					 };

				 // menu colors. the color of labels, button texts, list items, stuff and others
				 Phobos::UI::uiColorText = ParseColorInt(section, "Color.Text", 0xFFFF);

				 // original color schemes
				 static COMPILETIMEEVAL reference<int, 0x8316A8, 0x9> const DefaultColors {};
				 COMPILETIMEEVAL const char* Slot_tags[] = {
					 "Slot1", "Slot2", "Slot3", "Slot4",
					 "Slot5", "Slot6", "Slot7", "Slot8",
					 "Slot9", "Slot10", "Slot11", "Slot12",
					 "Slot13", "Slot14", "Slot15", "Slot16"
				 };

				 ReadColor("Observer", Phobos::UI::Colors[0], DefaultColors[8], GameStrings::STT_PlayerColorObserver, GameStrings::LightGrey);
				 ReadColor(Slot_tags[0], Phobos::UI::Colors[1], DefaultColors[0], GameStrings::STT_PlayerColorGold, GameStrings::LightGold);
				 ReadColor(Slot_tags[1], Phobos::UI::Colors[2], DefaultColors[1], GameStrings::STT_PlayerColorRed, GameStrings::DarkRed);
				 ReadColor(Slot_tags[2], Phobos::UI::Colors[3], DefaultColors[2], GameStrings::STT_PlayerColorBlue, "DarkBlue");
				 ReadColor(Slot_tags[3], Phobos::UI::Colors[4], DefaultColors[3], GameStrings::STT_PlayerColorGreen, "DarkGreen");
				 ReadColor(Slot_tags[4], Phobos::UI::Colors[5], DefaultColors[4], GameStrings::STT_PlayerColorOrange, "Orange");
				 ReadColor(Slot_tags[5], Phobos::UI::Colors[6], DefaultColors[5], GameStrings::STT_PlayerColorSkyBlue, "DarkSky");
				 ReadColor(Slot_tags[6], Phobos::UI::Colors[7], DefaultColors[6], GameStrings::STT_PlayerColorPurple, "Purple");
				 ReadColor(Slot_tags[7], Phobos::UI::Colors[8], DefaultColors[7], GameStrings::STT_PlayerColorPink, "Magenta");

				 // additional color schemes so just increasing Count will produce nice colors
				 ReadColor(Slot_tags[8], Phobos::UI::Colors[9], 0xEF5D94, "STT:PlayerColorLilac", "NeonBlue");
				 ReadColor(Slot_tags[9], Phobos::UI::Colors[10], 0xE7FF73, "STT:PlayerColorLightBlue", "LightBlue");
				 ReadColor(Slot_tags[10], Phobos::UI::Colors[11], 0x63EFFF, "STT:PlayerColorLime", GameStrings::Yellow);
				 ReadColor(Slot_tags[11], Phobos::UI::Colors[12], 0x5AC308, "STT:PlayerColorTeal", GameStrings::Green);
				 ReadColor(Slot_tags[12], Phobos::UI::Colors[13], 0x0055BD, "STT:PlayerColorBrown", GameStrings::Red);
				 ReadColor(Slot_tags[13], Phobos::UI::Colors[14], 0x808080, "STT:PlayerColorCharcoal", GameStrings::Grey);

				 // blunt stuff
				 ReadColor(Slot_tags[14], Phobos::UI::Colors[15], DefaultColors[8], "NOSTR:LightGrey", GameStrings::LightGrey);
				 ReadColor(Slot_tags[15], Phobos::UI::Colors[16], DefaultColors[8], "NOSTR:LightGrey", GameStrings::LightGrey);

				 Phobos::UI::uiColorTextButton = ParseColorInt(section, "Color.Button.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextRadio = ParseColorInt(section, "Color.Radio.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextCheckbox = ParseColorInt(section, "Color.Checkbox.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextLabel = ParseColorInt(section, "Color.Label.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextList = ParseColorInt(section, "Color.List.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextCombobox = ParseColorInt(section, "Color.Combobox.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextGroupbox = ParseColorInt(section, "Color.Groupbox.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextSlider = ParseColorInt(section, "Color.Slider.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextEdit = ParseColorInt(section, "Color.Edit.Text", Phobos::UI::uiColorText);
				 Phobos::UI::uiColorTextObserver = ParseColorInt(section, "Color.Observer.Text", 0xEEEEEE);
				 Phobos::UI::uiColorCaret = ParseColorInt(section, "Color.Caret", 0xFFFF);
				 Phobos::UI::uiColorSelection = ParseColorInt(section, "Color.Selection", 0xFF);
				 Phobos::UI::uiColorSelectionCombobox = ParseColorInt(section, "Color.Combobox.Selection", Phobos::UI::uiColorSelection);
				 Phobos::UI::uiColorSelectionList = ParseColorInt(section, "Color.List.Selection", Phobos::UI::uiColorSelection);
				 Phobos::UI::uiColorSelectionObserver = ParseColorInt(section, "Color.Observer.Selection", 0x626262);
				 Phobos::UI::uiColorBorder1 = ParseColorInt(section, "Color.Border1", 0xC5BEA7);
				 Phobos::UI::uiColorBorder2 = ParseColorInt(section, "Color.Border2", 0x807A68);
				 Phobos::UI::uiColorDisabled = ParseColorInt(section, "Color.Disabled", 0x9F);
				 Phobos::UI::uiColorDisabledLabel = ParseColorInt(section, "Color.Label.Disabled", Phobos::UI::uiColorDisabled);
				 Phobos::UI::uiColorDisabledCombobox = ParseColorInt(section, "Color.Combobox.Disabled", Phobos::UI::uiColorDisabled);
				 Phobos::UI::uiColorDisabledSlider = ParseColorInt(section, "Color.Slider.Disabled", Phobos::UI::uiColorDisabled);
				 Phobos::UI::uiColorDisabledButton = ParseColorInt(section, "Color.Button.Disabled", 0xA7);
				 Phobos::UI::uiColorDisabledCheckbox = ParseColorInt(section, "Color.Checkbox.Disabled", Phobos::UI::uiColorDisabled);
				 Phobos::UI::uiColorDisabledList = ParseColorInt(section, "Color.List.Disabled", Phobos::UI::uiColorDisabled);
				 Phobos::UI::uiColorDisabledObserver = ParseColorInt(section, "Color.Observer.Disabled", 0x8F8F8F);

				 // read the mod's version info
				 if (Ini->ReadString("VersionInfo", GameStrings::Name, Phobos::readDefval, Phobos::readBuffer, std::size(ModName)))
				 {
					 PhobosCRT::strCopy(ModName, Phobos::readBuffer);
				 }

				 if (Ini->ReadString("VersionInfo", "Version", Phobos::readDefval, Phobos::readBuffer, std::size(ModVersion)))
				 {
					 PhobosCRT::strCopy(ModVersion, Phobos::readBuffer);
				 }

				 SafeChecksummer crc {};
				 crc.operator()(ModName);
				 crc.operator()(ModVersion);
				 ModIdentifier = Ini->ReadInteger("VersionInfo", "Identifier", static_cast<int>(crc.operator unsigned int()));

				 Debug::LogInfo("Color count is {}", colorCount);
				 Debug::LogInfo("Mod is {0} ({1}) with 0x{2:x}",
					 ModName,
					 ModVersion,
					 (unsigned)ModIdentifier
				 );
			 }

			 Debug::LogInfo("-------------------Complete ----------------------");
		 }

	 // LoadingScreen
	 {
		 Phobos::UI::DisableEmptySpawnPositions =
			 pINI->ReadBool("LoadingScreen", "DisableEmptySpawnPositions", Phobos::UI::DisableEmptySpawnPositions);
	 }

	 // UISettings
	 {
		 pINI->ReadString(UISETTINGS_SECTION, "ShowBriefingResumeButtonLabel", "GUI:Resume", Phobos::readBuffer);
		 Phobos::UI::ShowBriefingResumeButtonLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"");

		 pINI->ReadString(UISETTINGS_SECTION, "ShowBriefingResumeButtonStatusLabel", "STT:BriefingButtonReturn", Phobos::readBuffer);
		 strcpy_s(Phobos::UI::ShowBriefingResumeButtonStatusLabel, Phobos::readBuffer);

		 Phobos::Config::ShowPowerDelta = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowPowerDelta", Phobos::Config::ShowPowerDelta);
		 Phobos::Config::ShowHarvesterCounter = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowHarvesterCounter", Phobos::Config::ShowHarvesterCounter);
		 Phobos::Config::ShowWeedsCounter = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowWeedsCounter", Phobos::Config::ShowWeedsCounter);


		 Phobos::UI::Power_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_POWER_FORMAT_B", L"Power = %d");
		 Phobos::UI::Drain_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DRAIN_FORMAT_B", L"Drain = %d");
		 Phobos::UI::Storage_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_STORAGE_FORMAT", L"Storage = %.3lf");
		 Phobos::UI::BuidingFakeLabel = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FAKE", L"FAKE");
		 Phobos::UI::Radar_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_RADAR", L"Radar");
		 Phobos::UI::Tech_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_TECHBUILDING", L"TechBuilding");
		 Phobos::UI::Spysat_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_SPYSAT", L"SpySat");
	 }

	 // ToolTips
	 {
		 Phobos::UI::ExtendedToolTips = pINI->ReadBool(GameStrings::ToolTips(), "ExtendedToolTips", Phobos::UI::ExtendedToolTips);
		 Phobos::UI::AnchoredToolTips = pINI->ReadBool(GameStrings::ToolTips(), "AnchoredToolTips", Phobos::UI::AnchoredToolTips);
		 Phobos::UI::MaxToolTipWidth = pINI->ReadInteger(GameStrings::ToolTips(), "MaxWidth", Phobos::UI::MaxToolTipWidth);

		 pINI->ReadString(GameStrings::ToolTips(), "CostLabel", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::CostLabel = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"$");

		 pINI->ReadString(GameStrings::ToolTips(), "PowerLabel", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::PowerLabel = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"\u26a1"); // ⚡

		 pINI->ReadString(GameStrings::ToolTips(), "PowerBlackoutLabel", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::PowerBlackoutLabel = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"\u26a1\u274c"); // ⚡❌

		 pINI->ReadString(GameStrings::ToolTips(), "TimeLabel", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::TimeLabel = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"\u231a"); // ⌚

		 pINI->ReadString(GameStrings::ToolTips(), "PercentLabel", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::PercentLabel = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"\u231a"); // ⌚

		 pINI->ReadString(GameStrings::ToolTips(), "RadarJammedLabel", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::BuidingRadarJammedLabel = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"Radar Jammed");

		 pINI->ReadString(GameStrings::ToolTips(), "SWShotsFormat", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::SWShotsFormat = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"%d/%d shots");

		 pINI->ReadString(GameStrings::ToolTips, "BattlePoints.Label", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::BattlePoints_Label = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"\u2605: "); // ★:

	 }

	 // Sidebar
	 {
		 Phobos::UI::ShowHarvesterCounter =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "HarvesterCounter.Show", Phobos::UI::ShowHarvesterCounter);

		 pINI->ReadString(SIDEBAR_SECTION_T, "HarvesterCounter.Label", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::HarvesterLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26cf"); // ⛏

		 Phobos::UI::HarvesterCounter_ConditionYellow =
			 pINI->ReadDouble(SIDEBAR_SECTION_T, "HarvesterCounter.ConditionYellow", Phobos::UI::HarvesterCounter_ConditionYellow);

		 Phobos::UI::HarvesterCounter_ConditionRed =
			 pINI->ReadDouble(SIDEBAR_SECTION_T, "HarvesterCounter.ConditionRed", Phobos::UI::HarvesterCounter_ConditionRed);

		 Phobos::UI::ShowProducingProgress =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "ProducingProgress.Show", Phobos::UI::ShowProducingProgress);

		 Phobos::UI::WeedsCounter_Show =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "WeedsCounter.Show", Phobos::UI::WeedsCounter_Show);

		 Phobos::UI::ShowPowerDelta =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "PowerDelta.Show", Phobos::UI::ShowPowerDelta);

		 Phobos::UI::PowerDelta_ConditionYellow =
			 pINI->ReadDouble(SIDEBAR_SECTION_T, "PowerDelta.ConditionYellow", Phobos::UI::PowerDelta_ConditionYellow);

		 Phobos::UI::PowerDelta_ConditionRed =
			 pINI->ReadDouble(SIDEBAR_SECTION_T, "PowerDelta.ConditionRed", Phobos::UI::PowerDelta_ConditionRed);

		 Phobos::Config::TogglePowerInsteadOfRepair =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "TogglePowerInsteadOfRepair", Phobos::Config::TogglePowerInsteadOfRepair);

		 Phobos::UI::CenterPauseMenuBackground =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "CenterPauseMenuBackground", Phobos::UI::CenterPauseMenuBackground);

		 pINI->ReadString(SIDEBAR_SECTION_T, "BattlePointsSidebar.Label", GameStrings::NoneStr(), Phobos::readBuffer);
		 Phobos::UI::BattlePointsSidebar_Label = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"\u2605: "); // ★:

		 Phobos::UI::BattlePointsSidebar_Label_InvertPosition =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "BattlePointsSidebar.Label.InvertPosition", Phobos::UI::BattlePointsSidebar_Label_InvertPosition);

		 Phobos::UI::BattlePointsSidebar_AlwaysShow =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "BattlePointsSidebar.AlwaysVisible", Phobos::UI::BattlePointsSidebar_AlwaysShow);

		 Phobos::UI::SuperWeaponSidebar =
			 pINI->ReadBool(GameStrings::SideBar(), "SuperWeaponSidebar", Phobos::UI::SuperWeaponSidebar );

		 Phobos::UI::SuperWeaponSidebar_Interval =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.Interval", Phobos::UI::SuperWeaponSidebar_Interval);

		 Phobos::UI::SuperWeaponSidebar_LeftOffset =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.LeftOffset", Phobos::UI::SuperWeaponSidebar_LeftOffset);

		 Phobos::UI::SuperWeaponSidebar_CameoHeight =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.CameoHeight", Phobos::UI::SuperWeaponSidebar_CameoHeight);

		 Phobos::UI::SuperWeaponSidebar_Max =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.Max", Phobos::UI::SuperWeaponSidebar_Max);

		 Phobos::UI::SuperWeaponSidebar_MaxColumns =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.MaxColumns", Phobos::UI::SuperWeaponSidebar_MaxColumns);

		 Phobos::UI::SuperWeaponSidebar_Pyramid =
			 pINI->ReadBool(GameStrings::SideBar(), "SuperWeaponSidebar.Pyramid", Phobos::UI::SuperWeaponSidebar_Pyramid);

		 //
		 Phobos::UI::SuperWeaponSidebar =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "SuperWeaponSidebar", Phobos::UI::SuperWeaponSidebar);

		 Phobos::UI::SuperWeaponSidebar_Interval =
			 pINI->ReadInteger(SIDEBAR_SECTION_T, "SuperWeaponSidebar.Interval", Phobos::UI::SuperWeaponSidebar_Interval);

		 Phobos::UI::SuperWeaponSidebar_LeftOffset =
			 pINI->ReadInteger(SIDEBAR_SECTION_T, "SuperWeaponSidebar.LeftOffset", Phobos::UI::SuperWeaponSidebar_LeftOffset);

		 Phobos::UI::SuperWeaponSidebar_CameoHeight =
			 pINI->ReadInteger(SIDEBAR_SECTION_T, "SuperWeaponSidebar.CameoHeight", Phobos::UI::SuperWeaponSidebar_CameoHeight);

		 Phobos::UI::SuperWeaponSidebar_Max =
			 pINI->ReadInteger(SIDEBAR_SECTION_T, "SuperWeaponSidebar.Max", Phobos::UI::SuperWeaponSidebar_Max);

		 Phobos::UI::SuperWeaponSidebar_MaxColumns =
			 pINI->ReadInteger(SIDEBAR_SECTION_T, "SuperWeaponSidebar.MaxColumns", Phobos::UI::SuperWeaponSidebar_MaxColumns);

		 Phobos::UI::SuperWeaponSidebar_Pyramid =
			 pINI->ReadBool(SIDEBAR_SECTION_T, "SuperWeaponSidebar.Pyramid", Phobos::UI::SuperWeaponSidebar_Pyramid);
		 //

		 Phobos::UI::SuperWeaponSidebar_LeftOffset = MinImpl(Phobos::UI::SuperWeaponSidebar_Interval, Phobos::UI::SuperWeaponSidebar_LeftOffset);
		 Phobos::UI::SuperWeaponSidebar_CameoHeight = MaxImpl(48, Phobos::UI::SuperWeaponSidebar_CameoHeight);

		 const int reserveHeight = 96;
		 const int screenHeight = GameOptionsClass::Instance->ScreenHeight - reserveHeight;

		 if (Phobos::UI::SuperWeaponSidebar_Max > 0)
			 Phobos::UI::SuperWeaponSidebar_Max = MinImpl(Phobos::UI::SuperWeaponSidebar_Max, screenHeight / Phobos::UI::SuperWeaponSidebar_CameoHeight);
		 else
			 Phobos::UI::SuperWeaponSidebar_Max = screenHeight / Phobos::UI::SuperWeaponSidebar_CameoHeight;

		}

	});
}

void Phobos::Config::Read_RULESMD()
{
	CCINIClass* const pINI = CCINIClass::INI_Rules();

	//Debug::Log("Loading early %s file.\n", GameStrings::RULESMD_INI());

	 // uncomment this to enable dll usage warning
	 //Phobos::ThrowUsageWarning(&INI_RulesMD);

	 if (!Phobos::Otamaa::IsAdmin)
		 Phobos::Config::DevelopmentCommands = pINI->ReadBool(GLOBALCONTROLS_SECTION, "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);

	 Phobos::Config::ArtImageSwap = pINI->ReadBool(GameStrings::General(), "ArtImageSwap", Phobos::Config::ArtImageSwap);
	 Phobos::Config::UnitPowerDrain = pINI->ReadBool(GameStrings::General(), "UnitPowerDrain", Phobos::Config::UnitPowerDrain);
	 Phobos::UI::UnlimitedColor = pINI->ReadBool(GameStrings::General(), "SkirmishUnlimitedColors", Phobos::UI::UnlimitedColor);

	 if (pINI->ReadBool(GameStrings::General(), "CustomGS", Phobos::Misc::CustomGS))
	 {
		 Phobos::Misc::CustomGS = true;

		 //char tempBuffer[0x20];
		 for (size_t i = 0; i <= 6; ++i)
		 {
			 std::string _buffer = "CustomGS";
			 _buffer += std::to_string(6 - i);

			 int temp = pINI->ReadInteger(GameStrings::General(), (_buffer + ".ChangeDelay").c_str(), -1);
			 if (temp >= 0 && temp <= 6)
				 Phobos::Misc::CustomGS_ChangeDelay[i] = 6 - temp;

			 temp = pINI->ReadInteger(GameStrings::General(), (_buffer + ".DefaultDelay").c_str(), -1);
			 if (temp >= 1)
				 Phobos::Misc::CustomGS_DefaultDelay[i] = 6 - temp;

			 temp = pINI->ReadInteger(GameStrings::General(), (_buffer + ".ChangeInterval").c_str(), -1);
			 if (temp >= 1)
				 Phobos::Misc::CustomGS_ChangeInterval[i] = temp;
		 }
	 }

	 if (pINI->ReadBool(GameStrings::General(), "FixTransparencyBlitters", false))
	 {
		 BlittersFix::Apply();
	 }

	 Phobos::Config::MultiThreadSinglePlayer = pINI->ReadBool(GameStrings::General(), "MultiThreadSinglePlayer", Phobos::Config::MultiThreadSinglePlayer);
	 Phobos::Config::SaveVariablesOnScenarioEnd = pINI->ReadBool(GameStrings::General(), "SaveVariablesOnScenarioEnd", Phobos::Config::SaveVariablesOnScenarioEnd);
	 Phobos::Config::ApplyShadeCountFix = pINI->ReadBool(GameStrings::AudioVisual(), "ApplyShadeCountFix", Phobos::Config::ApplyShadeCountFix);
	 Phobos::Config::SuperWeaponSidebarCommands = pINI->ReadBool("GlobalControls", "SuperWeaponSidebarKeysEnabled", Phobos::Config::SuperWeaponSidebarCommands);
	 Phobos::Config::AllowSwitchNoMoveCommand = pINI->ReadBool("GlobalControls", "AllowSwitchNoMoveCommand", Phobos::Config::AllowDistributionCommand);
	 Phobos::Config::AllowDistributionCommand = pINI->ReadBool("GlobalControls", "AllowDistributionCommand", Phobos::Config::AllowDistributionCommand);
	 Phobos::Config::AllowDistributionCommand_SpreadMode = pINI->ReadBool("GlobalControls", "AllowDistributionCommand.SpreadMode", Phobos::Config::AllowDistributionCommand_SpreadMode);
	 Phobos::Config::AllowDistributionCommand_SpreadModeScroll = pINI->ReadBool("GlobalControls", "AllowDistributionCommand.SpreadModeScroll", Phobos::Config::AllowDistributionCommand_SpreadModeScroll);
	 Phobos::Config::AllowDistributionCommand_FilterMode = pINI->ReadBool("GlobalControls", "AllowDistributionCommand.FilterMode", Phobos::Config::AllowDistributionCommand_FilterMode);
	 Phobos::Config::AllowDistributionCommand_AffectsAllies = pINI->ReadBool("GlobalControls", "AllowDistributionCommand.AffectsAllies", Phobos::Config::AllowDistributionCommand_AffectsAllies);
	 Phobos::Config::AllowDistributionCommand_AffectsEnemies = pINI->ReadBool("GlobalControls", "AllowDistributionCommand.AffectsEnemies", Phobos::Config::AllowDistributionCommand_AffectsEnemies);
}
