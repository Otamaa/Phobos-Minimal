#include <Ext/HouseType/Body.h>
#include <Ext/Side/Body.h>

DWORD MixFileYuriFiles(REGISTERS* R, DWORD dwReturnAddress1, DWORD dwReturnAddress2)
{
	GET(ScenarioClass*, pScen, EAX);

	if (SideClass* pSide = SideClass::Array->GetItemOrDefault(pScen->PlayerSideIndex)) {
		return SideExtContainer::Instance.Find(pSide)->SidebarYuriFileNames
			? dwReturnAddress1
			: dwReturnAddress2
			;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x72FA1A, Sides_MixFileYuriFiles1, 7)
{ return MixFileYuriFiles(R, 0x72FA23, 0x72FA6A); }

DEFINE_OVERRIDE_HOOK(0x72F370, Sides_MixFileYuriFiles2, 7)
{ return MixFileYuriFiles(R, 0x72F379, 0x72F3A0); }

DEFINE_OVERRIDE_HOOK(0x72FBC0, Sides_MixFileYuriFiles3, 5)
{ return MixFileYuriFiles(R, 0x72FBCE, 0x72FBF5); }

#include <CCToolTip.h>

DEFINE_OVERRIDE_HOOK(0x72F440, Game_InitializeToolTipColor, 0xA)
{
	GET(int, idxSide, ECX);

	if (SideClass* pSide = SideClass::Array->GetItemOrDefault(idxSide)) {
		CCToolTip::ToolTipTextColor = SideExtContainer::Instance.Find(pSide)->ToolTipTextColor;
		return 0x72F495;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x72D730, Game_LoadMultiplayerScoreAssets, 5)
{
	GET(const int, idxSide, ECX);
	auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
	auto pExt = SideExtContainer::Instance.Find(pSide);

	auto& AlreadyLoaded = *reinterpret_cast<bool*>(0xB0FBB8);

	if (!AlreadyLoaded)
	{

		// load the images
		auto& MPxSCRNy_SHP = *reinterpret_cast<SHPStruct**>(0xB0FB1C);
		auto& MPxSCRNy_Loaded = *reinterpret_cast<bool*>(0xB0FC7D);

		MPxSCRNy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreMultiplayBackground, MPxSCRNy_Loaded);

		// load the palette
		auto& MPxSCRN_Palette = *reinterpret_cast<BytePalette**>(0xB0FBB0);
		auto& MPxSCRN_Convert = *reinterpret_cast<ConvertClass**>(0xB0FBB4);

		ConvertClass::CreateFromFile(pExt->ScoreMultiplayPalette, MPxSCRN_Palette, MPxSCRN_Convert);

		AlreadyLoaded = true;
	}

	return 0x72D775;
}

DEFINE_OVERRIDE_HOOK(0x72D300, Game_LoadCampaignScoreAssets, 5)
{
	GET(const int, idxSide, ECX);
	auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
	auto pExt = SideExtContainer::Instance.Find(pSide);

	auto& AlreadyLoaded = *reinterpret_cast<bool*>(0xB0FBAC);

	if (!AlreadyLoaded)
	{

		// load the images
		auto& SxCRBKyy_SHP = *reinterpret_cast<SHPStruct**>(0xB0FB34);
		auto& SxCRTyy_SHP = *reinterpret_cast<SHPStruct**>(0xB0FB00);
		auto& SxCRAyy_SHP = *reinterpret_cast<SHPStruct**>(0xB0FB30);

		auto& SxCRBKyy_Loaded = *reinterpret_cast<bool*>(0xB0FC70);
		auto& SxCRTyy_Loaded = *reinterpret_cast<bool*>(0xB0FC71);
		auto& SxCRAyy_Loaded = *reinterpret_cast<bool*>(0xB0FC72);

		SxCRBKyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignBackground, SxCRBKyy_Loaded);
		SxCRTyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignTransition, SxCRTyy_Loaded);
		SxCRAyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignAnimation, SxCRAyy_Loaded);

		// load the palette
		auto& xSCORE_Palette = *reinterpret_cast<BytePalette**>(0xB0FBA4);
		auto& xSCORE_Convert = *reinterpret_cast<ConvertClass**>(0xB0FBA8);

		ConvertClass::CreateFromFile(pExt->ScoreCampaignPalette, xSCORE_Palette, xSCORE_Convert);

		AlreadyLoaded = true;
	}

	return 0x72D345;
}

DEFINE_OVERRIDE_HOOK(0x72B690, LoadScreenPal_Load, 0)
{
	GET(int, n, EDI);

	HouseTypeExtData* pData = nullptr;
	if (auto pThis = HouseTypeClass::Array->GetItemOrDefault(n))
	{
		pData = HouseTypeExtContainer::Instance.Find(pThis);
	}

	const char* pPALFile = nullptr;

	if (pData)
	{
		pPALFile = pData->LoadScreenPalette;
	}
	else if (n == 0)
	{
		pPALFile = "mplsu.pal";	//need to recode cause I broke the code with the jump
	}
	else
	{
		return 0x72B6B6;
	}

	//some ASM-less magic! =)
	auto ppPalette = reinterpret_cast<BytePalette**>(0xB0FB94);
	auto ppDestination = reinterpret_cast<ConvertClass**>(0xB0FB98);
	ConvertClass::CreateFromFile(pPALFile, *ppPalette, *ppDestination);

	return 0x72B804;
}
