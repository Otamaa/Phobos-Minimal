#include "Phobos.h"

#include <CCINIClass.h>

#include <Utilities/Debug.h>
#include <Utilities/GameConfig.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

#include <Misc/Patches.h>
#include <Misc/CSF.h>
#include <Misc/PhobosGlobal.h>

#include <GameStrings.h>
#include <GameOptionsClass.h>
#include <StringTable.h>

#include <Phobos.INI.h>

#include <Misc/Spawner/Main.h>

#include <Ext/Convert/Body.h>

#include <IPXManagerClass.h>

#include <SessionClass.h>

_GET_FUNCTION_ADDRESS(ConvertClassExt::AllocBlitters, GetConvertClassExtAllocBlittersAddress);
_GET_FUNCTION_ADDRESS(ConvertClassExt::DeallocBlitters, GetConvertClassExtDeallocBlittersAddress);

// very early load and creation of RA2MD.INI
ASMJIT_PATCH(0x6BC099 , WInMain_CreateRA2MD_Override, 0x7){

	PhobosINIContainer::Ra2_INI = std::make_unique<PhobosINIClass>();
	auto _ra2MD = GameCreate<RawFileClass>(GameStrings::RA2MD_INI());
	;

	if (!CCINIClass::INI_RA2MD->ReadCCFile(_ra2MD) || !PhobosINIContainer::Ra2_INI->LoadFile(_ra2MD)) {
		Debug::FatalError("Failed to load %s !" , GameStrings::RA2MD_INI());
		return 0x0;
	}

	{
		Game::bVideoBackBuffer = PhobosINIContainer::Ra2_INI->Read<bool>("Video", "VideoBackBuffer").value_or(1);
		Game::bAllowModeToggle = PhobosINIContainer::Ra2_INI->Read<bool>("Video", "AllowModeToggle").value_or(0);
		GameOptionsClass::Instance->ScreenWidth = PhobosINIContainer::Ra2_INI->Read<int>("Video", "ScreenWidth").value_or(GameOptionsClass::Instance->ScreenWidth);
		GameOptionsClass::Instance->ScreenHeight = PhobosINIContainer::Ra2_INI->Read<int>("Video", "ScreenHeight").value_or(GameOptionsClass::Instance->ScreenHeight);
	}

	const char* const pDebugSection = "Debug";

	{
		const auto simd_level = PhobosINIContainer::Ra2_INI->ReadString(pDebugSection, "MaxSimdLevel", Simd::GetLevelName(Phobos::Config::MaxSimdLevel));
		Debug::Log("Config MaxSimdLevel raw value: %s\n", simd_level.c_str());
		Phobos::Config::MaxSimdLevel = Simd::ParseLevel(simd_level.c_str(), Phobos::Config::MaxSimdLevel);
	}

	PhobosGlobal::Instance()->LoadGlobalsConfig();
	SpawnerMain::ReadRA2MDConfig(PhobosINIContainer::Ra2_INI.get());
	SpawnerMain::ApplyStaticOptions();

	Simd::Initialize(Phobos::Config::MaxSimdLevel);

	if (Simd::GetCurrentLevel() > Simd::Level::Vanilla)
	{
		Patch::Apply_LJMP(0x48EBF0, GetConvertClassExtAllocBlittersAddress);
		Patch::Apply_LJMP(0x490490, GetConvertClassExtDeallocBlittersAddress);
	}

	{
		int sock = PhobosINIContainer::Ra2_INI->Read<int>("Network", "Socket").value_or(0);
		if (sock > 0)
		{
			sock += 0x4000;
			if (sock >= 0x4000 && sock < 0x8000)
			{
				IPXManagerClass::Instance->Set_Socket(sock);
			}
		}

		const auto DestNet = PhobosINIContainer::Ra2_INI->ReadString("Network", "DestNet", nullptr);

		if (!DestNet.empty()) {
			const auto splitted = PhobosCRT::SplitString(DestNet, ".");
			unsigned char addr[4] {};
			size_t i = 0;

			for (i < splitted.size() && i < 4; ++i;) {
				if (!PhobosCRT::ScanHex(splitted[i], addr[i]))
					break;
			}

			if (i >= 4) {
				IPXAddressClass::IsBridge = true;
				unsigned char node[6];
				std::memset(node, 0xFF, sizeof(node));
				IPXAddressClass::Instance->Assign(addr, node, 0);
			}
		}
	}

	R->Base<FileClass*>(-0x70C, _ra2MD);
	return 0x6BC28E;
}


void Phobos::Config::Read_RA2MD()
{

	{

		Phobos::Config::ToolTipDescriptions = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "ToolTipDescriptions").value_or(Phobos::Config::ToolTipDescriptions);
		Phobos::Config::ToolTipBlur = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "ToolTipBlur").value_or(Phobos::Config::ToolTipBlur);
		Phobos::Config::PrioritySelectionFiltering = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "PrioritySelectionFiltering").value_or(Phobos::Config::PrioritySelectionFiltering);
		Phobos::Config::PriorityDeployFiltering = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "PriorityDeployFiltering").value_or(Phobos::Config::PriorityDeployFiltering);
		Phobos::Config::TypeSelectUseIFVMode = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "TypeSelectUseIFVMode").value_or(Phobos::Config::TypeSelectUseIFVMode);
		Phobos::Config::EnableBuildingPlacementPreview = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "ShowBuildingPlacementPreview").value_or(Phobos::Config::EnableBuildingPlacementPreview);
		Phobos::Config::EnableSelectBox = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "EnableSelectBox").value_or(Phobos::Config::EnableSelectBox);

		//Phobos::Config::RealTimeTimers = PhobosINIContainer::Ra2_INI->Read<bool>((PHOBOS_STR, "RealTimeTimers", Phobos::Config::RealTimeTimers);
		//Phobos::Config::RealTimeTimers_Adaptive = PhobosINIContainer::Ra2_INI->Read<bool>((PHOBOS_STR, "RealTimeTimers.Adaptive", Phobos::Config::RealTimeTimers_Adaptive);
		Phobos::Config::DigitalDisplay_Enable = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "DigitalDisplay.Enable").value_or(Phobos::Config::DigitalDisplay_Enable);
		Phobos::Config::MessageDisplayInCenter = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "MessageDisplayInCenter").value_or(Phobos::Config::MessageDisplayInCenter);
		Phobos::Config::MessageApplyHoverState = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "MessageApplyHoverState").value_or(Phobos::Config::MessageApplyHoverState);
		Phobos::Config::MessageDisplayInCenter_BoardOpacity = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "MessageDisplayInCenter.BoardOpacity").value_or(Phobos::Config::MessageDisplayInCenter_BoardOpacity);
		Phobos::Config::MessageDisplayInCenter_LabelsCount = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "MessageDisplayInCenter.LabelsCount").value_or(Phobos::Config::MessageDisplayInCenter_LabelsCount);
		Phobos::Config::MessageDisplayInCenter_RecordsCount = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "MessageDisplayInCenter.RecordsCount").value_or(Phobos::Config::MessageDisplayInCenter_RecordsCount);

		Phobos::Config::ShowBuildingStatistics = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "ShowBuildingStatistics").value_or(Phobos::Config::ShowBuildingStatistics);
		Phobos::Config::ShowFlashOnSelecting = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "ShowFlashOnSelecting").value_or(Phobos::Config::ShowFlashOnSelecting);
		Phobos::Config::SuperWeaponSidebar_RequiredSignificance = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "SuperWeaponSidebar.RequiredSignificance").value_or(Phobos::Config::SuperWeaponSidebar_RequiredSignificance);
		Phobos::Config::HideLightFlashEffects = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "HideLightFlashEffects").value_or(Phobos::Config::HideLightFlashEffects);
		Phobos::Config::HideLaserTrailEffects = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "HideLaserTrailEffects").value_or(Phobos::Config::HideLaserTrailEffects);
		Phobos::Config::HideShakeEffects = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "HideShakeEffects").value_or(Phobos::Config::HideShakeEffects);
		Phobos::Config::SaveGameOnScenarioStart = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "SaveGameOnScenarioStart").value_or(Phobos::Config::SaveGameOnScenarioStart);

		Phobos::Config::ApplyNoMoveCommand = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "DefaultApplyNoMoveCommand").value_or(true);
		Phobos::Config::DistributionSpreadMode = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "DefaultDistributionSpreadMode").value_or(2);
		Phobos::Config::DistributionFilterMode = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "DefaultDistributionFilterMode").value_or(2);	

		Phobos::Config::ShowPowerPlantEnhancerRange = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "ShowPowerPlantEnhancerRange").value_or(Phobos::Config::ShowPowerPlantEnhancerRange);
		Phobos::Config::ShowGameTime = PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "ShowGameTime").value_or(Phobos::Config::ShowGameTime);
		Phobos::Config::ShowGameTime_BoardOpacity = PhobosINIContainer::Ra2_INI->Read<int>(PHOBOS_STR, "ShowGameTime.BoardOpacity").value_or(Phobos::Config::ShowGameTime_BoardOpacity);
	}

	Phobos::Config::DistributionSpreadMode = std::clamp(Phobos::Config::DistributionSpreadMode, 0, 3);
	Phobos::Config::DistributionFilterMode = std::clamp(Phobos::Config::DistributionFilterMode, 0, 3);

	if (!Phobos::Otamaa::IsAdmin)
	{
		// Custom game speeds, 6 - i so that GS6 is index 0, just like in the engine
		Phobos::Config::CampaignDefaultGameSpeed = 6 - PhobosINIContainer::Ra2_INI->Read<bool>(PHOBOS_STR, "CampaignDefaultGameSpeed").value_or(4);

		if (Phobos::Config::CampaignDefaultGameSpeed > 6 || Phobos::Config::CampaignDefaultGameSpeed < 0)
			Phobos::Config::CampaignDefaultGameSpeed = 2;

		{
			BYTE defaultspeed = (BYTE)Phobos::Config::CampaignDefaultGameSpeed;
			// We overwrite the instructions that force GameSpeed to 2 (GS4)
			Patch::Apply_RAW(0x55D77A, sizeof(defaultspeed), PatchType::PATCH_, &defaultspeed);

			// when speed control is off. Doesn't need a hook.
			Patch::Apply_RAW(0x55D78D, sizeof(defaultspeed), PatchType::PATCH_, &defaultspeed);
		}
	}
}

void Phobos::Config::Read_UIMD()
{
	CCFileClass file(GameStrings::UIMD_INI());

	Phobos::UI::Power_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_POWER_FORMAT_B", L"Power = %d");
	Phobos::UI::Drain_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DRAIN_FORMAT_B", L"Drain = %d");
	Phobos::UI::Storage_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_STORAGE_FORMAT", L"Storage = %.3lf");
	Phobos::UI::BuidingFakeLabel = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FAKE", L"FAKE");
	Phobos::UI::Radar_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_RADAR", L"Radar");
	Phobos::UI::Tech_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_TECHBUILDING", L"TechBuilding");
	Phobos::UI::Spysat_Label = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_SPYSAT", L"SpySat");
	Phobos::UI::BuidingRadarJammedLabel = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_RADARJAMMED", L"Radar Jammed");
	Phobos::UI::ShowBriefingResumeButtonLabel = GeneralUtils::LoadStringUnlessMissingNoChecks("GUI:Resume", L"");
	Phobos::UI::GameTimeText = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_GAMETIME", L"Time:");

	if (!file.IsAvaible())
		return;

	PhobosINIContainer::Ui_INI = std::make_unique<PhobosINIClass>();

	CCINIClass::INI_UIMD->ReadCCFile(&file, false, false);
	PhobosINIContainer::Ui_INI ->LoadFile(&file);

	//theoritically this is fine , it is global instance
	//it never became null just empty state
	Debug::Log("Loading early %s file.\n", GameStrings::UIMD_INI());

	auto pINI = CCINIClass::INI_UIMD.operator->();

	{
		Debug::LogInfo("--------- Loading Ares global settings -----------");

		{
			auto const section2 = GameStrings::Colors();
			auto const ParseColorInt = [&](const char* section, const char* key, int defColor) -> int {
				ColorStruct ndefault(defColor & 0xFF, (defColor >> 8) & 0xFF, (defColor >> 16) & 0xFF);
				int _color[3];
				if(!PhobosINIContainer::Ui_INI->Read3<int>(section, key, _color))
					return ndefault;

				return ((char)_color[0]) | ((char)_color[1]) << 8 | ((char)_color[2]) << 16;
			};

			{

				colorCount = std::clamp(PhobosINIContainer::Ui_INI->Read<int>(section2, "Count").value_or(colorCount), 8, 17);

				auto const ReadColor = [pINI, section2, ParseColorInt]
				(
					const std::string& name,
					ColorData& value,
					int colorRGB,
					const char* defTooltip,
					const char* defColorScheme
				)
					{
						// load the tooltip string
						const auto _sst = PhobosINIContainer::Ui_INI->ReadString(section2, name + ".Tooltip", defTooltip);
						if (!_sst.empty())
							value.sttToolTipSublineText = CSFLoader::FetchStringManager(_sst.c_str(), nullptr, nullptr, -1);

						const auto _ssheme = PhobosINIContainer::Ui_INI->ReadString(section2, name + ".ColorScheme", defColorScheme);

						if (!_ssheme.empty())
							PhobosCRT::strCopy(value.colorScheme, _ssheme.c_str());

						value.colorRGB = ParseColorInt(section2, (name + ".DisplayColor").c_str(), colorRGB);
						value.colorSchemeIndex = -1;
						value.selectedIndex = -1;
					};

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
			}

			auto const section = "UISettings";

			{
				// menu colors. the color of labels, button texts, list items, stuff and others
				Phobos::UI::uiColorText = ParseColorInt(section, "Color.Text", 0xFFFF);

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
			}

			auto sectionVersionInfo = "VersionInfo";

		{

				// read the mod's version info
				const auto _ModName = PhobosINIContainer::Ui_INI->ReadString(sectionVersionInfo, GameStrings::Name(), "");

				if (!_ModName.empty()) {
					PhobosCRT::strCopy(ModName, _ModName.c_str());
				}

				const auto _Version = PhobosINIContainer::Ui_INI->ReadString(sectionVersionInfo, "Version", "");

				if (!_Version.empty()) {
					PhobosCRT::strCopy(ModVersion, _Version.c_str());
				}

				SafeChecksummer crc {};
				crc.operator()((const char*)ModName);
				crc.operator()((const char*)ModVersion);
				ModIdentifier = PhobosINIContainer::Ui_INI->Read<int>("VersionInfo", "Identifier").value_or(static_cast<int>(crc.operator unsigned int()));

				Debug::LogInfo("Color count is {}", colorCount);
				Debug::LogInfo("Mod is {0} ({1}) with 0x{2:x}",
					ModName,
					ModVersion,
					(unsigned)ModIdentifier
				);
			}
		}

		Debug::LogInfo("-------------------Complete ----------------------");
	}

	{
		Phobos::UI::DisableEmptySpawnPositions = PhobosINIContainer::Ui_INI->Read<bool>("LoadingScreen", "DisableEmptySpawnPositions").value_or(Phobos::UI::DisableEmptySpawnPositions);
	}

	{
		const auto _Resume = PhobosINIContainer::Ui_INI->ReadString(UISETTINGS_SECTION, "ShowBriefingResumeButtonLabel", "GUI:Resume");
		Phobos::UI::ShowBriefingResumeButtonLabel = GeneralUtils::LoadStringOrDefault(_Resume.c_str(), L"");

		const auto _brief  = PhobosINIContainer::Ui_INI->ReadString(UISETTINGS_SECTION, "ShowBriefingResumeButtonStatusLabel", "STT:BriefingButtonReturn");
		PhobosCRT::strCopy(Phobos::UI::ShowBriefingResumeButtonStatusLabel, _brief.c_str());
	}

	{
		Phobos::Config::ShowPowerDelta = PhobosINIContainer::Ui_INI->Read<bool>(PHOBOS_STR, "ShowPowerDelta").value_or(Phobos::Config::ShowPowerDelta);
		Phobos::Config::ShowHarvesterCounter = PhobosINIContainer::Ui_INI->Read<bool>(PHOBOS_STR, "ShowHarvesterCounter").value_or(Phobos::Config::ShowHarvesterCounter);
		Phobos::Config::ShowWeedsCounter = PhobosINIContainer::Ui_INI->Read<bool>(PHOBOS_STR, "ShowWeedsCounter").value_or(Phobos::Config::ShowWeedsCounter);
	}
	auto ReadNullableCSF = [&](NullableCSF& value ,const char* pSesction,  const char* pKey) {
		std::string _result = PhobosINIContainer::Ui_INI->ReadString(pSesction, pKey, GameStrings::NoneStr());
		value.Set(GeneralUtils::LoadStringUnlessMissing(_result.c_str(), value.c_str()));
	};

	{

		Phobos::UI::ExtendedToolTips = PhobosINIContainer::Ui_INI->Read<bool>(GameStrings::ToolTips(), "ExtendedToolTips").value_or(Phobos::UI::ExtendedToolTips);
		Phobos::UI::AnchoredToolTips = PhobosINIContainer::Ui_INI->Read<bool>(GameStrings::ToolTips(), "AnchoredToolTips").value_or(Phobos::UI::AnchoredToolTips);
		Phobos::UI::MaxToolTipWidth = PhobosINIContainer::Ui_INI->Read<int>(GameStrings::ToolTips(), "MaxWidth").value_or(Phobos::UI::MaxToolTipWidth);

		ReadNullableCSF(Phobos::UI::CostLabel,GameStrings::ToolTips(),"CostLabel");
		ReadNullableCSF(Phobos::UI::PowerLabel,GameStrings::ToolTips(), "PowerLabel");
		ReadNullableCSF(Phobos::UI::PowerBlackoutLabel,GameStrings::ToolTips(), "PowerBlackoutLabel");
		ReadNullableCSF(Phobos::UI::TimeLabel,GameStrings::ToolTips(), "TimeLabel");
		ReadNullableCSF(Phobos::UI::PercentLabel,GameStrings::ToolTips(), "PercentLabel");
		ReadNullableCSF(Phobos::UI::SWShotsFormat,GameStrings::ToolTips(), "SWShotsFormat");
		ReadNullableCSF(Phobos::UI::BattlePoints_Label,GameStrings::ToolTips(), "BattlePoints.Label");
	}

	{
		Phobos::UI::ShowHarvesterCounter =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "HarvesterCounter.Show").value_or(Phobos::UI::ShowHarvesterCounter);

		ReadNullableCSF(Phobos::UI::HarvesterLabel , SIDEBAR_SECTION_T, "HarvesterCounter.Label");

		Phobos::UI::HarvesterCounter_ConditionYellow =
			PhobosINIContainer::Ui_INI->Read<double>(SIDEBAR_SECTION_T, "HarvesterCounter.ConditionYellow").value_or(Phobos::UI::HarvesterCounter_ConditionYellow);

		Phobos::UI::HarvesterCounter_ConditionRed =
			PhobosINIContainer::Ui_INI->Read<double>(SIDEBAR_SECTION_T, "HarvesterCounter.ConditionRed").value_or(Phobos::UI::HarvesterCounter_ConditionRed);

		Phobos::UI::ShowProducingProgress =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "ProducingProgress.Show").value_or(Phobos::UI::ShowProducingProgress);

		Phobos::UI::WeedsCounter_Show =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "WeedsCounter.Show").value_or(Phobos::UI::WeedsCounter_Show);

		Phobos::UI::ShowPowerDelta =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "PowerDelta.Show").value_or(Phobos::UI::ShowPowerDelta);

		Phobos::UI::PowerDelta_ConditionYellow =
			PhobosINIContainer::Ui_INI->Read<double>(SIDEBAR_SECTION_T, "PowerDelta.ConditionYellow").value_or(Phobos::UI::PowerDelta_ConditionYellow);

		Phobos::UI::PowerDelta_ConditionRed =
			PhobosINIContainer::Ui_INI->Read<double>(SIDEBAR_SECTION_T, "PowerDelta.ConditionRed").value_or(Phobos::UI::PowerDelta_ConditionRed);

		Phobos::Config::TogglePowerInsteadOfRepair =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "TogglePowerInsteadOfRepair").value_or(Phobos::Config::TogglePowerInsteadOfRepair);

		Phobos::UI::CenterPauseMenuBackground =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "CenterPauseMenuBackground").value_or(Phobos::UI::CenterPauseMenuBackground);

		ReadNullableCSF(Phobos::UI::BattlePointsSidebar_Label, SIDEBAR_SECTION_T, "BattlePointsSidebar.Label");

		Phobos::UI::BattlePointsSidebar_Label_InvertPosition =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "BattlePointsSidebar.Label.InvertPosition").value_or(Phobos::UI::BattlePointsSidebar_Label_InvertPosition);

		Phobos::UI::BattlePointsSidebar_AlwaysShow =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "BattlePointsSidebar.AlwaysVisible").value_or(Phobos::UI::BattlePointsSidebar_AlwaysShow);

		Phobos::UI::SuperWeaponSidebar =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "SuperWeaponSidebar").value_or(Phobos::UI::SuperWeaponSidebar);

		Phobos::UI::SuperWeaponSidebar_Interval =
			PhobosINIContainer::Ui_INI->Read<int>(SIDEBAR_SECTION_T, "SuperWeaponSidebar.Interval").value_or(Phobos::UI::SuperWeaponSidebar_Interval);

		Phobos::UI::SuperWeaponSidebar_LeftOffset =
			PhobosINIContainer::Ui_INI->Read<int>(SIDEBAR_SECTION_T, "SuperWeaponSidebar.LeftOffset").value_or(Phobos::UI::SuperWeaponSidebar_LeftOffset);

		Phobos::UI::SuperWeaponSidebar_CameoHeight =
			PhobosINIContainer::Ui_INI->Read<int>(SIDEBAR_SECTION_T, "SuperWeaponSidebar.CameoHeight").value_or(Phobos::UI::SuperWeaponSidebar_CameoHeight);

		Phobos::UI::SuperWeaponSidebar_Max =
			PhobosINIContainer::Ui_INI->Read<int>(SIDEBAR_SECTION_T, "SuperWeaponSidebar.Max").value_or(Phobos::UI::SuperWeaponSidebar_Max);

		Phobos::UI::SuperWeaponSidebar_MaxColumns =
			PhobosINIContainer::Ui_INI->Read<int>(SIDEBAR_SECTION_T, "SuperWeaponSidebar.MaxColumns").value_or(Phobos::UI::SuperWeaponSidebar_MaxColumns);

		Phobos::UI::SuperWeaponSidebar_Pyramid =
			PhobosINIContainer::Ui_INI->Read<bool>(SIDEBAR_SECTION_T, "SuperWeaponSidebar.Pyramid").value_or(Phobos::UI::SuperWeaponSidebar_Pyramid);

		Phobos::UI::CreditsIndicator_MaxStep =
			PhobosINIContainer::Ui_INI->Read<int>(SIDEBAR_SECTION_T, "CreditsIndicator.MaxStep").value_or(Phobos::UI::CreditsIndicator_MaxStep);
	}

	{
		Phobos::UI::SuperWeaponSidebar =
			PhobosINIContainer::Ui_INI->Read<bool>(GameStrings::SideBar(), "SuperWeaponSidebar").value_or(Phobos::UI::SuperWeaponSidebar);

		Phobos::UI::SuperWeaponSidebar_Interval =
			PhobosINIContainer::Ui_INI->Read<int>(GameStrings::SideBar(), "SuperWeaponSidebar.Interval").value_or(Phobos::UI::SuperWeaponSidebar_Interval);

		Phobos::UI::SuperWeaponSidebar_LeftOffset =
			PhobosINIContainer::Ui_INI->Read<int>(GameStrings::SideBar(), "SuperWeaponSidebar.LeftOffset").value_or(Phobos::UI::SuperWeaponSidebar_LeftOffset);

		Phobos::UI::SuperWeaponSidebar_CameoHeight =
			PhobosINIContainer::Ui_INI->Read<int>(GameStrings::SideBar(), "SuperWeaponSidebar.CameoHeight").value_or(Phobos::UI::SuperWeaponSidebar_CameoHeight);

		Phobos::UI::SuperWeaponSidebar_Max =
			PhobosINIContainer::Ui_INI->Read<int>(GameStrings::SideBar(), "SuperWeaponSidebar.Max").value_or(Phobos::UI::SuperWeaponSidebar_Max);

		Phobos::UI::SuperWeaponSidebar_MaxColumns =
			PhobosINIContainer::Ui_INI->Read<int>(GameStrings::SideBar(), "SuperWeaponSidebar.MaxColumns").value_or(Phobos::UI::SuperWeaponSidebar_MaxColumns);

		Phobos::UI::SuperWeaponSidebar_Pyramid =
			PhobosINIContainer::Ui_INI->Read<bool>(GameStrings::SideBar(), "SuperWeaponSidebar.Pyramid").value_or(Phobos::UI::SuperWeaponSidebar_Pyramid);
	}

	Phobos::UI::SuperWeaponSidebar_LeftOffset = MinImpl(Phobos::UI::SuperWeaponSidebar_Interval, Phobos::UI::SuperWeaponSidebar_LeftOffset);
	Phobos::UI::SuperWeaponSidebar_CameoHeight = MaxImpl(48, Phobos::UI::SuperWeaponSidebar_CameoHeight);

	const int reserveHeight = 96;
	const int screenHeight = GameOptionsClass::Instance->ScreenHeight - reserveHeight;

	if (Phobos::UI::SuperWeaponSidebar_Max > 0)
		Phobos::UI::SuperWeaponSidebar_Max = MinImpl(Phobos::UI::SuperWeaponSidebar_Max, screenHeight / Phobos::UI::SuperWeaponSidebar_CameoHeight);
	else
		Phobos::UI::SuperWeaponSidebar_Max = screenHeight / Phobos::UI::SuperWeaponSidebar_CameoHeight;
}

// this config only read once and only once
void Phobos::Config::Read_RULESMD()
{
	CCINIClass* const pINI = CCINIClass::INI_Rules();

	//Debug::Log("Loading early %s file.\n", GameStrings::RULESMD_INI());

	 // uncomment this to enable dll usage warning
	 //Phobos::ThrowUsageWarning(&INI_RulesMD);

	if (pINI->GetSection(GLOBALCONTROLS_SECTION)){

		if (!Phobos::Otamaa::IsAdmin)
			Phobos::Config::DevelopmentCommands = pINI->ReadBool(GLOBALCONTROLS_SECTION, "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);

		Phobos::Config::SuperWeaponSidebarCommands = pINI->ReadBool(GLOBALCONTROLS_SECTION, "SuperWeaponSidebarKeysEnabled", Phobos::Config::SuperWeaponSidebarCommands);
		Phobos::Config::AllowSwitchNoMoveCommand = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowSwitchNoMoveCommand", Phobos::Config::AllowDistributionCommand);
		Phobos::Config::AllowDistributionCommand = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowDistributionCommand", Phobos::Config::AllowDistributionCommand);
		Phobos::Config::AllowDistributionCommand_SpreadMode = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowDistributionCommand.SpreadMode", Phobos::Config::AllowDistributionCommand_SpreadMode);
		Phobos::Config::AllowDistributionCommand_SpreadModeScroll = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowDistributionCommand.SpreadModeScroll", Phobos::Config::AllowDistributionCommand_SpreadModeScroll);
		Phobos::Config::AllowDistributionCommand_FilterMode = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowDistributionCommand.FilterMode", Phobos::Config::AllowDistributionCommand_FilterMode);
		Phobos::Config::AllowDistributionCommand_AffectsAllies = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowDistributionCommand.AffectsAllies", Phobos::Config::AllowDistributionCommand_AffectsAllies);
		Phobos::Config::AllowDistributionCommand_AffectsEnemies = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowDistributionCommand.AffectsEnemies", Phobos::Config::AllowDistributionCommand_AffectsEnemies);
	}

	if(pINI->GetSection(GameStrings::General())) { 

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

		if (pINI->ReadBool(GameStrings::General(), "FixTransparencyBlitters", Phobos::Config::FixTransparencyBlitters))
		{
			Phobos::Config::FixTransparencyBlitters = true;
		}

		Phobos::Config::MultiThreadSinglePlayer = pINI->ReadBool(GameStrings::General(), "MultiThreadSinglePlayer", Phobos::Config::MultiThreadSinglePlayer);
		Phobos::Config::SaveVariablesOnScenarioEnd = pINI->ReadBool(GameStrings::General(), "SaveVariablesOnScenarioEnd", Phobos::Config::SaveVariablesOnScenarioEnd);
	}

	if (pINI->GetSection(GameStrings::AudioVisual())) {
		Phobos::Config::ApplyShadeCountFix = pINI->ReadBool(GameStrings::AudioVisual(), "ApplyShadeCountFix", Phobos::Config::ApplyShadeCountFix);
	}

}
