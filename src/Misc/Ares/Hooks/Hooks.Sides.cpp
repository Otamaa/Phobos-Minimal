#include <Ext/HouseType/Body.h>
#include <Ext/Side/Body.h>

ASMJIT_PATCH(0x534FB1, Sides_MixFileIndex, 5)
{
	GET(int, n, ESI);

	int MixIdx = n;
	if (n >= 0)
		MixIdx = SideExtContainer::Instance.Find(SideClass::Array->Items[n])->SidebarMixFileIndex - 1;

	R->EBX(MixIdx);
	R->ESI(MixIdx);
	R->Stack(0x10, n);
	return ((MixIdx >> 0x1F) & 0xFFFFFFC3) + 0x535003;
}

DWORD MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2)
{
	GET(ScenarioClass*, pScen, EAX);

	if (SideClass* pSide = SideClass::Array->get_or_default(pScen->PlayerSideIndex)) {
		return SideExtContainer::Instance.Find(pSide)->SidebarYuriFileNames
			? dwReturnAddress1
			: dwReturnAddress2
			;
	}

	return 0;
}

ASMJIT_PATCH(0x72FA1A, Sides_MixFileYuriFiles1, 7)
{ return MixFileYuriFiles(R, 0x72FA23, 0x72FA6A); }

ASMJIT_PATCH(0x72F370, Sides_MixFileYuriFiles2, 7)
{ return MixFileYuriFiles(R, 0x72F379, 0x72F3A0); }

ASMJIT_PATCH(0x72FBC0, Sides_MixFileYuriFiles3, 5)
{ return MixFileYuriFiles(R, 0x72FBCE, 0x72FBF5); }

#include <CCToolTip.h>

ASMJIT_PATCH(0x72F440, Game_InitializeToolTipColor, 0xA)
{
	GET(int, idxSide, ECX);

	if (SideClass* pSide = SideClass::Array->get_or_default(idxSide)) {
		CCToolTip::ToolTipTextColor = SideExtContainer::Instance.Find(pSide)->ToolTipTextColor;
		return 0x72F495;
	}

	return 0;
}

ASMJIT_PATCH(0x72D730, Game_LoadMultiplayerScoreAssets, 5)
{
	GET(const int, idxSide, ECX);
	auto pSide = SideClass::Array->get_or_default(idxSide);
	auto pExt = SideExtContainer::Instance.Find(pSide);

	static COMPILETIMEEVAL reference<bool , 0xB0FBB8u> const MultiplayerScoreAssetsAlreadyLoaded{};
	if (!MultiplayerScoreAssetsAlreadyLoaded)
	{
		static COMPILETIMEEVAL reference<SHPStruct* , 0xB0FB1Cu> const MPxSCRNy_SHP {};
		static COMPILETIMEEVAL reference<bool , 0xB0FC7Du> const MPxSCRNy_Loaded {};

		// load the images
		MPxSCRNy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreMultiplayBackground, MPxSCRNy_Loaded());

		static COMPILETIMEEVAL reference<BytePalette* , 0xB0FBB0u> const MPxSCRN_Palette {};
		static COMPILETIMEEVAL reference<ConvertClass* , 0xB0FBB4u> const MPxSCRN_Convert {};

		// load the palette
		ConvertClass::CreateFromFile(pExt->ScoreMultiplayPalette, MPxSCRN_Palette(), MPxSCRN_Convert());

		MultiplayerScoreAssetsAlreadyLoaded = true;
	}

	return 0x72D775;
}

ASMJIT_PATCH(0x72D300, Game_LoadCampaignScoreAssets, 5)
{
	GET(const int, idxSide, ECX);
	auto pSide = SideClass::Array->get_or_default(idxSide);
	auto pExt = SideExtContainer::Instance.Find(pSide);

	static COMPILETIMEEVAL reference<bool , 0xB0FBACu> const CampaignScoreAssetsAlreadyLoaded {};

	if (!CampaignScoreAssetsAlreadyLoaded)
	{
		static COMPILETIMEEVAL reference<SHPStruct* , 0xB0FB34u> const SxCRBKyy_SHP {};
		static COMPILETIMEEVAL reference<SHPStruct* , 0xB0FB00u> const SxCRTyy_SHP {};
		static COMPILETIMEEVAL reference<SHPStruct* , 0xB0FB30u> const SxCRAyy_SHP {};

		static COMPILETIMEEVAL reference<bool , 0xB0FC70u> const SxCRBKyy_Loaded {};
		static COMPILETIMEEVAL reference<bool ,0xB0FC71u> const SxCRTyy_Loaded {};
		static COMPILETIMEEVAL reference<bool , 0xB0FC72u> const SxCRAyy_Loaded {};

		// load the images
		SxCRBKyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignBackground, SxCRBKyy_Loaded());
		SxCRTyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignTransition, SxCRTyy_Loaded());
		SxCRAyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignAnimation, SxCRAyy_Loaded());

		// load the palette
		static COMPILETIMEEVAL reference<BytePalette* , 0xB0FBA4u> const xSCORE_Palette {};
		static COMPILETIMEEVAL reference<ConvertClass* , 0xB0FBA8u> const xSCORE_Convert {};
		ConvertClass::CreateFromFile(pExt->ScoreCampaignPalette, xSCORE_Palette(), xSCORE_Convert());

		CampaignScoreAssetsAlreadyLoaded = true;
	}

	return 0x72D345;
}

ASMJIT_PATCH(0x72B690, LoadScreenPal_Load, 0xA)
{
	GET(int, n, EDI);

	HouseTypeExtData* pData = nullptr;
	if (auto pThis = HouseTypeClass::Array->get_or_default(n))
	{
		pData = HouseTypeExtContainer::Instance.Find(pThis);
	}

	const char* pPALFile = nullptr;

	if (pData) {
		pPALFile = pData->LoadScreenPalette;
	}
	else if (n == 0) {
		pPALFile = GameStrings::MPLSU_PAL();	//need to recode cause I broke the code with the jump
	}
	else
	{
		return 0x72B6B6;
	}

	//some ASM-less magic! =)
	static COMPILETIMEEVAL reference<BytePalette* , 0xB0FB94u> const _0xB0FB94u_{};
	static COMPILETIMEEVAL reference<ConvertClass* , 0xB0FB98> const _0xB0FB98u_{};
	ConvertClass::CreateFromFile(pPALFile, _0xB0FB94u_(), _0xB0FB98u_());

	return 0x72B804;
}

ASMJIT_PATCH(0x6847B7, ScenarioClass_PrepareMapAndUDP, 6)
{
	GET(HouseTypeClass*, pType, EAX);

	SideExtData::CurrentLoadTextColor = -1;

	if (auto pData = HouseTypeExtContainer::Instance.Find(pType)) {
		if (pData->LoadTextColor != -1) {
			SideExtData::CurrentLoadTextColor = pData->LoadTextColor;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x686D7F, INIClass_ReadScenario_CacheSP, 6)
{
	LEA_STACK(INIClass*, pINI, 0x1C);

	const char* pDefault = "";
	const char* pID = ScenarioClass::Instance->FileName;

	if (!CRT::_strnicmp(pID, "SOV", 3))
	{
		pDefault = GameStrings::SovietLoad();
	}
	else if (!CRT::_strnicmp(pID, "YUR", 3))
	{
		pDefault = "YuriLoad";
	}
	else if (!CRT::_strnicmp(pID, "TUT", 3))
	{
		pDefault = GameStrings::LightGrey();
	}
	else
	{
		pDefault = GameStrings::AlliedLoad();
	}

	SideExtData::CurrentLoadTextColor = -1;

	if (pINI->ReadString(ScenarioClass::Instance->FileName, "LoadScreenText.Color", pDefault, Phobos::readBuffer)) {
		if (ColorScheme* pCS = ColorScheme::Find(Phobos::readBuffer)) {
			SideExtData::CurrentLoadTextColor = pCS->ArrayIndex; // TODO: check if off by one. see ColorScheme.h
		}
	}

	return 0;
}

ASMJIT_PATCH(0x53534C, Game_LoadUI_LoadSideData, 7)
{
	SideExtData::UpdateGlobalFiles();
	return 0;
}

ASMJIT_PATCH(0x6D4E79, TacticalClass_DrawOverlay_GraphicalText, 6)
{
	auto pConvert = SideExtData::GetGraphicalTextConvert();
	auto pShp = SideExtData::GetGraphicalTextImage();

	R->EBX(pConvert);
	R->ESI(pShp);

	return (pConvert && pShp) ? 0x6D4E8D : 0x6D4EF4;
}

ASMJIT_PATCH(0x622223, sub_621E90_DialogBackground, 6)
{
	auto pShp = SideExtData::s_DialogBackgroundImage;
	auto pConvert = SideExtData::s_DialogBackgroundConvert.GetConvert();

	if(pConvert && pShp) {
		R->EDI(pShp);
		R->EAX(pConvert);
		return 0x62226A;
	}

	return 0;
}

// music piece when loading a match or mission
int idxLoadingTheme = -2;

ASMJIT_PATCH(0x683C70, sub_683AB0_LoadingScoreA, 7)
{
	LEA_STACK(CCINIClass*, pINI, STACK_OFFS(0xFC, 0xE0));

	// magic value for the default loading theme
	idxLoadingTheme = -2;

	if (SessionClass::Instance->GameMode == GameMode::Campaign) {
		// single player missions read from the scenario
		idxLoadingTheme = pINI->ReadTheme(GameStrings::Basic(), "LoadingTheme", -2);

	}
	else {
		// override the default for multiplayer matches
		if (auto pSpot = SessionClass::Instance->StartSpots[0]) {
			if (((size_t)pSpot->Country) < HouseTypeClass::Array->size()) {

				const auto pType = HouseTypeClass::Array->Items[pSpot->Country];
				const auto pSide = SideClass::Array->Items[pType->SideIndex];

				// get theme from the side
				idxLoadingTheme = CCINIClass::INI_Rules()->ReadTheme(pSide->ID, "LoadingTheme", -2);

				// ...then from the house
				idxLoadingTheme = CCINIClass::INI_Rules()->ReadTheme(pType->ID, "LoadingTheme", idxLoadingTheme);
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x683D05, sub_683AB0_LoadingScoreB, 5)
{
	R->EAX(idxLoadingTheme);
	return (idxLoadingTheme == -2) ? 0 : 0x683D14;
}

ASMJIT_PATCH(0x5C9B75, Global_DrawScoreScreen_ScoreTheme, 5)
{
	REF_STACK(const char*, pTheme, 0x0);

	if (!HouseClass::IsCurrentPlayerObserver())
	{
		if(auto pSide = SideClass::Array->get_or_default(ScenarioClass::Instance->PlayerSideIndex)){
			pTheme = HouseClass::CurrentPlayer->Defeated
				? SideExtContainer::Instance.Find(pSide)->ScoreMultiplayThemeLose
				: SideExtContainer::Instance.Find(pSide)->ScoreMultiplayThemeWin;
		}
	}

	return 0;
}

DWORD FORCEDINLINE LoadTextColor(REGISTERS* R, DWORD dwReturnAddress)
{
	// if there is a cached LoadTextColor, use that.
	if (auto pCS = ColorScheme::Array->get_or_default(SideExtData::CurrentLoadTextColor)) {
		R->EAX(pCS);
		return dwReturnAddress;
	}

	return 0;
}

// WRONG! Stoopidwood passes CD= instead of Side= into singleplayer campaigns, TODO: fix that shit
ASMJIT_PATCH(0x642B36, Sides_LoadTextColor1, 5)
{ return LoadTextColor(R, 0x68CAA9); }

// WRONG! Stoopidwood passes CD= instead of Side= into singleplayer campaigns, TODO: fix that shit
ASMJIT_PATCH(0x643BB9, Sides_LoadTextColor2, 5)
{ return LoadTextColor(R, 0x643BEF); }

ASMJIT_PATCH(0x642B91, Sides_LoadTextColor3, 5)
{ return LoadTextColor(R, 0x68CAA9); }

ASMJIT_PATCH(0x5CA110, Game_GetMultiplayerScoreScreenBar, 5)
{
	GET(unsigned int, idxBar, ECX);

	BSurface* ret = nullptr;
	if (auto pSide = SideClass::Array->get_or_default(ScenarioClass::Instance->PlayerSideIndex)){
		ret = PCX::Instance->GetSurface(SideExtContainer::Instance.Find(pSide)->GetMultiplayerScoreBarFilename(idxBar));
	}

	R->EAX(ret);
	return 0x5CA41D;
}

// issue 906
// do not draw a box below the label text if there is none.
ASMJIT_PATCH(0x553E54, LoadProgressMgr_Draw_SkipShadowOnNullString, 6)
{
	GET(wchar_t*, pBrief, ESI);

	if (!pBrief || !CRT::wcslen(pBrief))
	{
		return 0x554027;
	}

	return 0;
}

// do not draw a box for the country name.
ASMJIT_PATCH(0x553820, LoadProgressMgr_Draw_SkipShadowOnNullString2, 5)
{
	GET(wchar_t*, pCountry, EDI);

	if (!pCountry || !CRT::wcslen(pCountry))
	{
		return 0x5539E4;
	}

	return 0;
}

// do not draw a box for an empty LoadingEx string
ASMJIT_PATCH(0x55403D, LoadProgressMgr_Draw_SkipShadowOnNullString3, 6)
{
	GET(wchar_t*, pLoading, EAX);

	if (!pLoading || !CRT::wcslen(pLoading))
	{
		return 0x554097;
	}

	return 0;
}

// score music for single player missions
static const char* pSinglePlayerScoreTheme = nullptr;

ASMJIT_PATCH(0x6C922C, ScoreDialog_Handle_ScoreThemeFirst, 5)
{
	GET(int, elapsed, EDI);
	GET(int, par, ESI);

	auto pScen = ScenarioClass::Instance();
	const char* pTitle = nullptr;
	const char* pMessage = nullptr;
	auto pSide = SideClass::Array->get_or_default(pScen->PlayerSideIndex);

	// replicate skipped instructions, and also update the score id

	if (elapsed > par)
	{
		pTitle = pScen->OverParTitle;
		pMessage = pScen->OverParMessage;

		if(pSide)
			pSinglePlayerScoreTheme = SideExtContainer::Instance.Find(pSide)->ScoreCampaignThemeOverPar;
	}
	else
	{
		pTitle = pScen->UnderParTitle;
		pMessage = pScen->UnderParMessage;

		if (pSide)
			pSinglePlayerScoreTheme = SideExtContainer::Instance.Find(pSide)->ScoreCampaignThemeUnderPar;
	}

	R->ECX(pTitle);
	R->ESI(pMessage);
	return 0x6C924F;
}

ASMJIT_PATCH(0x6C935C, ScoreDialog_Handle_ScoreThemeSecond, 5)
{
	REF_STACK(const char*, pTheme, 0x0);

	if (pSinglePlayerScoreTheme) {
		pTheme = pSinglePlayerScoreTheme;
	}

	return 0;
}
