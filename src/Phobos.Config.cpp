#include "Phobos.h"

#include <CCINIClass.h>

#include <Utilities/Debug.h>
#include <Utilities/GameConfig.h>

#include <Misc/Ares/Hooks/Header.h>
#include <Misc/Patches.h>

#include <GameStrings.h>
#include <GameOptionsClass.h>

void Phobos::Config::Read()
{
	auto const& pRA2MD = CCINIClass::INI_RA2MD;

	Phobos::Config::ToolTipDescriptions = pRA2MD->ReadBool(PHOBOS_STR, "ToolTipDescriptions", Phobos::Config::ToolTipDescriptions);
	Phobos::Config::ToolTipBlur = pRA2MD->ReadBool(PHOBOS_STR, "ToolTipBlur", Phobos::Config::ToolTipBlur);
	Phobos::Config::PrioritySelectionFiltering = pRA2MD->ReadBool(PHOBOS_STR, "PrioritySelectionFiltering", Phobos::Config::PrioritySelectionFiltering);
	Phobos::Config::EnableBuildingPlacementPreview = pRA2MD->ReadBool(PHOBOS_STR, "ShowBuildingPlacementPreview", Phobos::Config::EnableBuildingPlacementPreview);
	Phobos::Config::EnableSelectBrd = pRA2MD->ReadBool(PHOBOS_STR, "EnableSelectBrd", Phobos::Config::EnableSelectBrd);

	//Phobos::Config::RealTimeTimers = pRA2MD->ReadBool(PHOBOS_STR, "RealTimeTimers", Phobos::Config::RealTimeTimers);
	//Phobos::Config::RealTimeTimers_Adaptive = pRA2MD->ReadBool(PHOBOS_STR, "RealTimeTimers.Adaptive", Phobos::Config::RealTimeTimers_Adaptive);
	Phobos::Config::DigitalDisplay_Enable = pRA2MD->ReadBool(PHOBOS_STR, "DigitalDisplay.Enable", Phobos::Config::DigitalDisplay_Enable);
	Phobos::Config::ShowBuildingStatistics = pRA2MD->ReadBool(PHOBOS_STR, "ShowBuildingStatistics", Phobos::Config::ShowBuildingStatistics);
	Phobos::Config::ShowFlashOnSelecting = pRA2MD->ReadBool(PHOBOS_STR, "ShowFlashOnSelecting", Phobos::Config::ShowFlashOnSelecting);

	if (!Phobos::Otamaa::IsAdmin)
	{

		// Custom game speeds, 6 - i so that GS6 is index 0, just like in the engine
		Phobos::Config::CampaignDefaultGameSpeed = 6 - pRA2MD->ReadInteger(PHOBOS_STR, "CampaignDefaultGameSpeed", 4);

		if (Phobos::Config::CampaignDefaultGameSpeed > 6 || Phobos::Config::CampaignDefaultGameSpeed < 0)
			Phobos::Config::CampaignDefaultGameSpeed = 2;

		{
			BYTE defaultspeed = (BYTE)Phobos::Config::CampaignDefaultGameSpeed;
			// We overwrite the instructions that force GameSpeed to 2 (GS4)
			Patch::Apply_RAW(0x55D77A, sizeof(defaultspeed), &defaultspeed);

			// when speed control is off. Doesn't need a hook.
			Patch::Apply_RAW(0x55D78D, sizeof(defaultspeed), &defaultspeed);
		}
	}

	GameConfig UIMD { GameStrings::UIMD_INI() };

	UIMD.OpenINIAction([](CCINIClass* pINI)
 {
	 Debug::LogInfo("Loading early {} file", GameStrings::UIMD_INI());
	 AresGlobalData::ReadAresRA2MD(pINI);

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


		 Phobos::UI::Power_Label = GeneralUtils::LoadStringUnlessMissing("TXT_POWER_FORMAT_B", L"Power = %d");
		 Phobos::UI::Drain_Label = GeneralUtils::LoadStringUnlessMissing("TXT_DRAIN_FORMAT_B", L"Drain = %d");
		 Phobos::UI::Storage_Label = GeneralUtils::LoadStringUnlessMissing("TXT_STORAGE_FORMAT", L"Storage = %.3lf");
		 Phobos::UI::BuidingFakeLabel = GeneralUtils::LoadStringUnlessMissing("TXT_FAKE", L"FAKE");
		 Phobos::UI::Radar_Label = GeneralUtils::LoadStringUnlessMissing("TXT_RADAR", L"Radar");
		 Phobos::UI::Spysat_Label = GeneralUtils::LoadStringUnlessMissing("TXT_SPYSAT", L"SpySat");
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
		 Phobos::UI::SWShotsFormat = GeneralUtils::LoadStringUnlessMissing(Phobos::readBuffer, L"%d/%d shots"); // ⌚
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

		 Phobos::UI::SuperWeaponSidebar =
			 pINI->ReadBool(GameStrings::SideBar(), "SuperWeaponSidebar", Phobos::UI::SuperWeaponSidebar);

		 Phobos::UI::SuperWeaponSidebar_Interval =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.Interval", Phobos::UI::SuperWeaponSidebar_Interval);

		 Phobos::UI::SuperWeaponSidebar_LeftOffset =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.LeftOffset", Phobos::UI::SuperWeaponSidebar_LeftOffset);

		 Phobos::UI::SuperWeaponSidebar_LeftOffset = std::min(Phobos::UI::SuperWeaponSidebar_Interval, Phobos::UI::SuperWeaponSidebar_LeftOffset);

		 Phobos::UI::SuperWeaponSidebar_CameoHeight =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.CameoHeight", Phobos::UI::SuperWeaponSidebar_CameoHeight);

		 Phobos::UI::SuperWeaponSidebar_CameoHeight = std::max(48, Phobos::UI::SuperWeaponSidebar_CameoHeight);

		 Phobos::UI::SuperWeaponSidebar_Max =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.Max", Phobos::UI::SuperWeaponSidebar_Max);

		 const int reserveHeight = 96;
		 const int screenHeight = GameOptionsClass::Instance->ScreenHeight - reserveHeight;

		 if (Phobos::UI::SuperWeaponSidebar_Max > 0)
			 Phobos::UI::SuperWeaponSidebar_Max = std::min(Phobos::UI::SuperWeaponSidebar_Max, screenHeight / Phobos::UI::SuperWeaponSidebar_CameoHeight);
		 else
			 Phobos::UI::SuperWeaponSidebar_Max = screenHeight / Phobos::UI::SuperWeaponSidebar_CameoHeight;

		 Phobos::UI::SuperWeaponSidebar_MaxColumns =
			 pINI->ReadInteger(GameStrings::SideBar(), "SuperWeaponSidebar.MaxColumns", Phobos::UI::SuperWeaponSidebar_MaxColumns);
	 }

	});

	GameConfig RULESMD { GameStrings::RULESMD_INI() };

	RULESMD.OpenINIAction([](CCINIClass* pINI)
 {
	 Debug::LogInfo("Loading early {} file", GameStrings::RULESMD_INI());

	 // uncomment this to enable dll usage warning
	 //Phobos::ThrowUsageWarning(&INI_RulesMD);

	 if (!Phobos::Otamaa::IsAdmin)
		 Phobos::Config::DevelopmentCommands = pINI->ReadBool(GLOBALCONTROLS_SECTION, "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);

	 Phobos::Otamaa::DisableCustomRadSite = pINI->ReadBool(PHOBOS_STR, "DisableCustomRadSite", Phobos::Otamaa::DisableCustomRadSite);
	 Phobos::Config::ArtImageSwap = pINI->ReadBool(GameStrings::General(), "ArtImageSwap", Phobos::Config::ArtImageSwap);
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
	 Phobos::Config::HideLightFlashEffects = CCINIClass::INI_RA2MD->ReadBool(PHOBOS_STR, "HideLightFlashEffects", Phobos::Config::HideLightFlashEffects);
	 Phobos::Config::SaveVariablesOnScenarioEnd = pINI->ReadBool(GameStrings::General(), "SaveVariablesOnScenarioEnd", Phobos::Config::SaveVariablesOnScenarioEnd);
	 Phobos::Config::ApplyShadeCountFix = pINI->ReadBool(GameStrings::AudioVisual(), "ApplyShadeCountFix", Phobos::Config::ApplyShadeCountFix);
	 Phobos::Config::SaveGameOnScenarioStart = CCINIClass::INI_RA2MD->ReadBool(PHOBOS_STR, "SaveGameOnScenarioStart", Phobos::Config::SaveGameOnScenarioStart);

	});
}
