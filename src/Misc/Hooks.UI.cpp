#include <Phobos.h>

#include <Utilities/Macro.h>
#include <PreviewClass.h>
#include <Surface.h>
#include <CreditClass.h>
#include <ThemeClass.h>

#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Scenario/Body.h>

#include <Utilities/Debug.h>
#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x777C41, UI_ApplyAppIcon, 0x9)
{
	if (!Phobos::AppIconPath.empty()) {
		Debug::Log("Applying AppIcon from \"%s\"\n", Phobos::AppIconPath.c_str());
		R->EAX(LoadImageA(NULL, Phobos::AppIconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
		return 0x777C4A;
	}

	return 0;
}

DEFINE_HOOK(0x640B8D, LoadingScreen_DisableEmptySpawnPositions, 0x6)
{
	GET(bool const, esi, ESI);
	return(Phobos::UI::DisableEmptySpawnPositions || !esi) ? 0x640CE2: 0x640B93;
}

 //Allow size = 0 for map previews
DEFINE_HOOK(0x641B41, LoadingScreen_SkipPreview, 0x8)
{
	enum { Continue = 0x0 , return_true = 0x641D59 };

	GET(RectangleStruct*, pRect, EAX);
	return (pRect->Width > 0 && pRect->Height > 0)
		? Continue : return_true;
}

DEFINE_HOOK(0x641EE0, PreviewClass_ReadPreview, 0x6)
{
	GET(PreviewClass*, pThis, ECX);
	GET_STACK(const char*, lpMapFile, 0x4);

	CCFileClass file { lpMapFile };
	bool bResult = false;

	if (file.Exists() && file.Open(FileAccessMode::Read))
	{
		CCINIClass ini {};
		ini.ReadCCFile(&file, true);
		ini.CurrentSection = nullptr;
		ini.CurrentSectionName = nullptr;

		ScenarioClass::Instance->ReadStartPoints(ini);

		bResult = pThis->ReadPreviewPack(ini);
	}

	R->EAX(bResult);
	return 0x64203D;
}

DEFINE_HOOK(0x4A25E3, CreditsClass_GraphicLogic_Additionals , 0x8)
{
	const auto pPlayer = HouseClass::CurrentPlayer();
	if (HouseExtData::IsObserverPlayer(pPlayer) || pPlayer->Defeated)
		return 0x0;

	const auto pSide = SideClass::Array->GetItemOrDefault(pPlayer->SideIndex);

	if (!pSide)
		return 0x0;

	//GET(CreditClass*, pThis, EBP);

	const auto pSideExt = SideExtContainer::Instance.Find(pSide);

	wchar_t counter[0x20];

	 if (Phobos::UI::ShowHarvesterCounter)
	 {
	 	const auto nActive = HouseExtData::ActiveHarvesterCount(pPlayer);
	 	const auto nTotal = HouseExtData::TotalHarvesterCount(pPlayer);
	 	const auto nPercentage = nTotal == 0 ? 1.0 : (double)nActive / (double)nTotal;

	 	const ColorStruct clrToolTip = nPercentage > Phobos::UI::HarvesterCounter_ConditionYellow
	 		? Drawing::TooltipColor() : nPercentage > Phobos::UI::HarvesterCounter_ConditionRed
	 		? pSideExt->Sidebar_HarvesterCounter_Yellow : pSideExt->Sidebar_HarvesterCounter_Red;

	 	swprintf_s(counter, L"%ls%d/%d", Phobos::UI::HarvesterLabel, nActive, nTotal);

	 	Point2D vPos {
	 		DSurface::Sidebar->Get_Width() / 2 + 50 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().X,
	 		2 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().Y
	 	};

	 	RectangleStruct vRect = DSurface::Sidebar->Get_Rect();
	 	DSurface::Sidebar->DSurfaceDrawText(counter, &vRect, &vPos, Drawing::RGB2DWORD(clrToolTip), 0,
	 		TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12);
	 }

	if (Phobos::UI::ShowPowerDelta)
	{
		ColorStruct clrToolTip { };

		if (pPlayer->PowerBlackoutTimer.InProgress())
		{
			clrToolTip = pSideExt->Sidebar_PowerDelta_Grey.Get();
			swprintf_s(counter, L"%ls", Phobos::UI::PowerBlackoutLabel);
		}
		else
		{
			const int delta = pPlayer->PowerOutput - pPlayer->PowerDrain;
			const double percent = pPlayer->PowerOutput != 0
				? (double)pPlayer->PowerDrain / (double)pPlayer->PowerOutput : pPlayer->PowerDrain != 0
				? Phobos::UI::PowerDelta_ConditionRed * 2.f : Phobos::UI::PowerDelta_ConditionYellow;

			clrToolTip = percent < Phobos::UI::PowerDelta_ConditionYellow
				? pSideExt->Sidebar_PowerDelta_Green.Get() : LESS_EQUAL(percent, Phobos::UI::PowerDelta_ConditionRed)
				? pSideExt->Sidebar_PowerDelta_Yellow.Get() : pSideExt->Sidebar_PowerDelta_Red;

			swprintf_s(counter, L"%ls%+d", Phobos::UI::PowerLabel, delta);
		}

		const auto TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
				| static_cast<int>(pSideExt->Sidebar_PowerDelta_Align.Get()));

		Point2D vPos {
			DSurface::Sidebar->Get_Width() / 2 - 70 + pSideExt->Sidebar_PowerDelta_Offset.Get().X,
			2 + pSideExt->Sidebar_PowerDelta_Offset.Get().Y
		};

		RectangleStruct vRect = DSurface::Sidebar->Get_Rect();
		DSurface::Sidebar->DSurfaceDrawText(counter, &vRect, &vPos, Drawing::RGB2DWORD(clrToolTip), 0, TextFlags);
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6CE8AA, Replace_XXICON_With_New, 0x7)   //SWTypeClass::Load
DEFINE_HOOK_AGAIN(0x6CEE31, Replace_XXICON_With_New, 0x7)   //SWTypeClass::ReadINI
DEFINE_HOOK_AGAIN(0x716D13, Replace_XXICON_With_New, 0x7)   //TechnoTypeClass::Load
DEFINE_HOOK(0x715A4D, Replace_XXICON_With_New, 0x7)         //TechnoTypeClass::ReadINI
{
	char pFilename[0x20];
	strcpy_s(pFilename, RulesExtData::Instance()->MissingCameo.data());
	_strlwr_s(pFilename);

	if (_stricmp(pFilename, GameStrings::XXICON_SHP())
		&& strstr(pFilename, GameStrings::dot_SHP()))
	{
		if (const auto pFile = FileSystem::LoadFile(RulesExtData::Instance()->MissingCameo.data(), false))
		{
			R->EAX(pFile);
			return R->Origin() + 0xC;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6A8463, StripClass_OperatorLessThan_CameoPriority, 5)
{
	enum {
		rTrue = 0x6A8692,
		rFalse = 0x6A86A0,
		rTrue_ = 0x6A8477,
		rFalse_ = 0x6A8468
	};

	GET_STACK(TechnoTypeClass* const, pLeft, STACK_OFFS(0x1C, 0x8));
	GET_STACK(TechnoTypeClass* const, pRight, STACK_OFFS(0x1C, 0x4));
	GET_STACK(int const, idxLeft, STACK_OFFS(0x1C, -0x8));
	GET_STACK(int const, idxRight, STACK_OFFS(0x1C, -0x10));
	GET_STACK(AbstractType const, rttiLeft, STACK_OFFS(0x1C, -0x4));
	GET_STACK(AbstractType const, rttiRight, STACK_OFFS(0x1C, -0xC));

	const auto pLeftTechnoExt = TechnoTypeExtContainer::Instance.TryFind(pLeft);
	const auto pRightTechnoExt = TechnoTypeExtContainer::Instance.TryFind(pRight);

	const auto pLeftSWExt = (rttiLeft == AbstractType::Special || rttiLeft == AbstractType::Super || rttiLeft == AbstractType::SuperWeaponType)
		? SWTypeExtContainer::Instance.TryFind(SuperWeaponTypeClass::Array->GetItemOrDefault(idxLeft)) : nullptr;
	const auto pRightSWExt = (rttiRight == AbstractType::Special || rttiRight == AbstractType::Super || rttiRight == AbstractType::SuperWeaponType)
		? SWTypeExtContainer::Instance.TryFind(SuperWeaponTypeClass::Array->GetItemOrDefault(idxRight)) : nullptr;

	if ((pLeftTechnoExt || pLeftSWExt) && (pRightTechnoExt || pRightSWExt))
	{
		const auto leftPriority = pLeftTechnoExt ? pLeftTechnoExt->CameoPriority : pLeftSWExt->CameoPriority;
		const auto rightPriority = pRightTechnoExt ? pRightTechnoExt->CameoPriority : pRightSWExt->CameoPriority;

		if (leftPriority > rightPriority)
			return rTrue;
		else if (rightPriority > leftPriority)
			return rFalse;
	}

	// Restore overridden instructions
	GET(AbstractType, rtti1, ESI);
	return rtti1 == AbstractType::Special ? rTrue_ : rFalse_;
}

namespace BriefingTemp
{
	bool ShowBriefing = false;
}

// Check if briefing dialog should be played before starting scenario.
DEFINE_HOOK(0x683E41, ScenarioClass_Start_ShowBriefing, 0x6)
{
	enum { SkipGameCode = 0x683E6B };

	GET_STACK(bool, showBriefing, STACK_OFFSET(0xFC, -0xE9));

	// Don't show briefing dialog for non-campaign games or on restarts etc.
	if (!ScenarioExtData::Instance()->ShowBriefing || !showBriefing || !SessionClass::Instance->IsCampaign())
		return 0;

	BriefingTemp::ShowBriefing = true;

	int theme = ScenarioExtData::Instance()->BriefingTheme;

	if (theme == -1)
	{
		SideClass* pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex);

		if (const auto pSideExt = SideExtContainer::Instance.Find(pSide))
			theme = pSideExt->BriefingTheme;
	}

	if (theme != -1)
		ThemeClass::Instance->Queue(theme);

	// Skip over playing scenario theme.
	return SkipGameCode;
}

// Show the briefing dialog before entering game loop.
DEFINE_HOOK(0x48CE85, MainGame_ShowBriefing, 0x5)
{
	enum { SkipGameCode = 0x48CE8A };
	// Restore overridden instructions.
	SessionClass::Instance->Resume();

	if (BriefingTemp::ShowBriefing)
	{
		// Show briefing dialog.
		Game::SpecialDialog = 9;
		Game::ShowSpecialDialog();
		BriefingTemp::ShowBriefing = false;

		// Play scenario theme.
		int theme = ScenarioClass::Instance->ThemeIndex;

		if (theme == -1)
			ThemeClass::Instance->Stop(true);
		else
			ThemeClass::Instance->Queue(theme);
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x683F66, PauseGame_ShowBriefing, 0x5)
{
	enum { SkipGameCode = 0x683FAA };

	// Skip redrawing the screen if we're gonna show the briefing screen immediately after loading screen finishes
	if (BriefingTemp::ShowBriefing)
		return SkipGameCode;

	return 0;
}

#pragma region Otamaa
/*
namespace GClockTemp
{
	SuperWeaponTypeClass* Super;
	TechnoTypeClass* Techno;
}

DEFINE_HOOK(0x6A9941, StripClass_DrawIt_GetSWData, 0x7)
{
	GET(SuperWeaponTypeClass*, pThis, EAX);
	GClockTemp::Super = pThis;
	return 0x0;
}

DEFINE_HOOK(0x6A9779, StripClass_DrawIt_GetTechnoData, 0x5)
{
	GET(TechnoTypeClass*, pThis, EAX);
	GClockTemp::Techno = pThis;
	return 0x0;
}

static void FC StripClass_Draw_GClockSHP(
	Surface* Surface,
	ConvertClass* Pal,
	SHPStruct* SHP,
	int FrameIndex,
	const Point2D* const Position,
	const RectangleStruct* const Bounds,
	BlitterFlags Flags,
	int Remap,
	int ZAdjust,
	ZGradient ZGradientDescIndex,
	int Brightness,
	int TintColor,
	SHPStruct* ZShape,
	int ZShapeFrame,
	int XOffset,
	int YOffset
)
{
	int Gclock_int = -1;
	auto const pPlayer = HouseClass::CurrentPlayer();

	if(auto const pSide = SideClass::Array->GetItemOrDefault(pPlayer->SideIndex)) {
		if(auto const pSideExt = SideExtContainer::Instance.Find(pSide)) {
			SHP = pSideExt->GClock_Shape.Get(SHP);
			Gclock_int = pSideExt->GClock_Transculency.Get(-1);
			//Pal = pSideExt->GClock_Palette.GetOrDefaultConvert(Pal);
		}
	}

	if (auto const pSuper = GClockTemp::Super)
	{
		if (auto const pExt = SWTypeExtContainer::Instance.Find(pSuper))
		{
			SHP = pExt->GClock_Shape.Get(SHP);
			Gclock_int = pExt->GClock_Transculency.Get(-1);
			Pal = pExt->GClock_Palette.GetOrDefaultConvert(Pal);
			GClockTemp::Super = nullptr;
		}
	}else
	if (auto const pTechno = GClockTemp::Techno)
	{
		if (auto const pExt = TechnoTypeExtContainer::Instance.Find(pTechno))
		{
			SHP = pExt->GClock_Shape.Get(SHP);
			Gclock_int = pExt->GClock_Transculency.Get(-1);
			Pal = pExt->GClock_Palette.GetOrDefaultConvert(Pal);
			GClockTemp::Techno = nullptr;
		}
	}

	if (Gclock_int != -1)
		Flags = BlitterFlags::bf_400 | EnumFunctions::GetTranslucentLevel(Gclock_int);

	CC_Draw_Shape(Surface, Pal, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust, ZGradientDescIndex, Brightness, TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_JUMP(CALL,0x6A9E97, GET_OFFSET(StripClass_Draw_GClockSHP));
*/
//DEFINE_HOOK(0x6A9E9C, StripClass_Draw_GClock_ClearContext, 0x6)
//{
//	GClockTemp::Techno = nullptr;
//	GClockTemp::Super = nullptr;
//	return 0x0;
//}
#pragma endregion
