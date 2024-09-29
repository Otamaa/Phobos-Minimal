#include "Body.h"

#include <ThemeClass.h>
#include <Utilities/Helpers.h>

UniqueGamePtrB<SHPStruct> SideExtData::s_GraphicalTextImage = nullptr;
ConvertClass* SideExtData::s_GraphicalTextConvert = nullptr;

UniqueGamePtrB<SHPStruct> SideExtData::s_DialogBackgroundImage = nullptr;
ConvertClass* SideExtData::s_DialogBackgroundConvert = nullptr;


int SideExtData::CurrentLoadTextColor = -1;

void SideExtData::Initialize() {
	const char* pID = this->AttachedToObject->ID;

	if (IS_SAME_STR_(pID, "Nod"))
	{ //Soviets
		this->EVAIndex = 1;
		this->MessageTextColorIndex = 11;
		this->SidebarYuriFileNames = false;
		this->SidebarMixFileIndex = 2;
		this->ToolTipTextColor = ColorStruct(255, 255, 0);
	}
	else if (IS_SAME_STR_(pID, "ThirdSide"))
	{ //Yuri
		this->EVAIndex = 2;
		this->MessageTextColorIndex = 25;
		this->SidebarYuriFileNames = true;
		this->SidebarMixFileIndex = 2;
		this->ToolTipTextColor = ColorStruct(255, 255, 0);
	}
	else
	{ //Allies or any other country
		this->EVAIndex = 0;
		this->MessageTextColorIndex = 21;
		this->SidebarYuriFileNames = false;
		this->SidebarMixFileIndex = 1;
		this->ToolTipTextColor = ColorStruct(164, 210, 255);
	}

	switch (this->ArrayIndex)
	{
	case 0: // Allied
		this->ScoreMultiplayBackground = "MPASCRNL.SHP";
		this->ScoreMultiplayPalette = "MPASCRN.PAL";

		this->ScoreCampaignBackground = "ASCRBKMD.SHP";
		this->ScoreCampaignTransition = "ASCRTMD.SHP";
		this->ScoreCampaignAnimation = "ASCRAMD.SHP";
		this->ScoreCampaignPalette = "ASCORE.PAL";

		this->ScoreMultiplayBars = "mpascrnlbar~~.pcx";
		break;
	case 1: // Soviet
		this->ScoreMultiplayBackground = "MPSSCRNL.SHP";
		this->ScoreMultiplayPalette = "MPSSCRN.PAL";

		this->ScoreCampaignBackground = "SSCRBKMD.SHP";
		this->ScoreCampaignTransition = "SSCRTMD.SHP";
		this->ScoreCampaignAnimation = "SSCRAMD.SHP";
		this->ScoreCampaignPalette = "SSCORE.PAL";

		this->ScoreMultiplayBars = "mpsscrnlbar~~.pcx";
		break;
	default: // Yuri and others
		this->ScoreMultiplayBackground = "MPYSCRNL.SHP";
		this->ScoreMultiplayPalette = "MPYSCRN.PAL";

		this->ScoreCampaignBackground = "SYCRBKMD.SHP";
		this->ScoreCampaignTransition = "SYCRTMD.SHP";
		this->ScoreCampaignAnimation = "SYCRAMD.SHP";
		this->ScoreCampaignPalette = "YSCORE.PAL";

		this->ScoreMultiplayBars = "mpyscrnlbar~~.pcx";

		break;
	}
};

const char* SideExtData::GetMultiplayerScoreBarFilename(unsigned int index) const
{
	static char filename[decltype(this->ScoreMultiplayBars)::Size];
	auto const& data = this->ScoreMultiplayBars.data();

	PhobosCRT::lowercase(filename, data);

	if (auto const pMarker = strstr(filename, "~~"))
	{
		std::string number_ = std::format("{:02}" ,index + 1);
		pMarker[0] = number_[0];
		pMarker[1] = number_[1];
	}

	return filename;
}

bool SideExtData::isNODSidebar()
{
	auto const PlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (const auto pSide = SideClass::Array->GetItemOrDefault(PlayerSideIndex)) {
		return !SideExtContainer::Instance.Find(pSide)->Sidebar_GDIPositions.Get(PlayerSideIndex == 0);
	}

	return PlayerSideIndex == 0;
}

int SideExtData::GetSurvivorDivisor() const {
	return this->SurvivorDivisor.Get(this->GetDefaultSurvivorDivisor());
}

int SideExtData::GetDefaultSurvivorDivisor() const
{
	switch (this->ArrayIndex)
	{
	case 0:
		return RulesClass::Instance->AlliedSurvivorDivisor;
	case 1:
		return RulesClass::Instance->SovietSurvivorDivisor;
	case 2:
		return RulesClass::Instance->ThirdSurvivorDivisor;
	}

	// 100 is the base of the default value
	return 100 * (this->ArrayIndex + 1);
}

InfantryTypeClass* SideExtData::GetCrew() const
{
	return this->Crew.Get(this->GetDefaultCrew());
}

InfantryTypeClass* SideExtData::GetDefaultCrew() const
{
	switch (this->ArrayIndex)
	{
	case 1:
		return RulesClass::Instance->SovietCrew;
	case 2:
		return RulesClass::Instance->ThirdCrew;
	}

	return RulesClass::Instance->AlliedCrew;
}

InfantryTypeClass* SideExtData::GetEngineer() const
{
	return this->Engineer.Get(RulesClass::Instance->Engineer);
}

InfantryTypeClass* SideExtData::GetTechnician() const
{
	return this->Technician.Get(RulesClass::Instance->Technician);
}

UnitTypeClass* SideExtData::GetHunterSeeker() const
{
	return this->HunterSeeker.Get();
}

Iterator<TechnoTypeClass*> SideExtData::GetParaDropTypes() const
{
	if (this->ParaDropTypes.HasValue() && this->ParaDropNum.HasValue())
	{
		return this->ParaDropTypes;
	}

	return this->GetDefaultParaDropTypes();
}

Iterator<InfantryTypeClass*> SideExtData::GetDefaultParaDropTypes() const
{

	switch (this->ArrayIndex)
	{
	case 1:
		return RulesClass::Instance->SovParaDropInf;
	case 2:
		return RulesClass::Instance->YuriParaDropInf;
	}

	//return SovParaDropInf would be correct, but Ares < 0.6 does this:
	return RulesClass::Instance->AllyParaDropInf;
}

Iterator<int> SideExtData::GetDefaultParaDropNum() const
{
	switch (this->ArrayIndex)
	{
	case 1:
		return RulesClass::Instance->SovParaDropNum;
	case 2:
		return RulesClass::Instance->YuriParaDropNum;
	}

	//return SovParaDropNum would be correct, but Ares < 0.6 does this:
	return RulesClass::Instance->AllyParaDropNum;
}

Iterator<int> SideExtData::GetParaDropNum() const
{
	if (this->ParaDropTypes.HasValue() && this->ParaDropNum.HasValue()) {
		return this->ParaDropNum;
	}

	return this->GetDefaultParaDropNum();
}

Iterator<int> SideExtData::GetBaseDefenseCounts() const
{
	if (this->BaseDefenseCounts.HasValue()) {
		return this->BaseDefenseCounts;
	}

	return this->GetDefaultBaseDefenseCounts();
}

Iterator<int> SideExtData::GetDefaultBaseDefenseCounts() const
{
	switch (this->ArrayIndex)
	{
	case 0:
		return RulesClass::Instance->AlliedBaseDefenseCounts;
	case 1:
		return RulesClass::Instance->SovietBaseDefenseCounts;
	case 2:
		return RulesClass::Instance->ThirdBaseDefenseCounts;
	default:
		//return Iterator<int>(); would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedBaseDefenseCounts;
	}
}

Iterator<BuildingTypeClass*> SideExtData::GetBaseDefenses() const
{
	if (this->BaseDefenses.HasValue()) {
		return this->BaseDefenses;
	}

	return this->GetDefaultBaseDefenses();
}

Iterator<BuildingTypeClass*> SideExtData::GetDefaultBaseDefenses() const
{
	switch (this->ArrayIndex)
	{
	case 0:
		return RulesClass::Instance->AlliedBaseDefenses;
	case 1:
		return RulesClass::Instance->SovietBaseDefenses;
	case 2:
		return RulesClass::Instance->ThirdBaseDefenses;
	default:
		//return Iterator<BuildingTypeClass*>(); would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedBaseDefenses;
	}
}

InfantryTypeClass* SideExtData::GetDisguise() const
{
	if (this->Disguise.isset()) {
		return this->Disguise;
	}

	return this->GetDefaultDisguise();
}

InfantryTypeClass* SideExtData::GetDefaultDisguise() const
{
	switch (this->ArrayIndex)
	{
	case 0:
		return RulesClass::Instance->AlliedDisguise;
	case 1:
		return RulesClass::Instance->SovietDisguise;
	case 2:
		return RulesClass::Instance->ThirdDisguise;
	default:
		//return RulesClass::Instance->ThirdDisguise; would be correct, but Ares < 0.5 does this:
		return RulesClass::Instance->AlliedDisguise;
	}
}

void SideExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	const char* pSection = pThis->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);
	this->Sidebar_GDIPositions.Read(exINI, pSection, "Sidebar.GDIPositions");
	this->IngameScore_WinTheme = pINI->ReadTheme(pSection, "IngameScore.WinTheme", this->IngameScore_WinTheme);
	this->IngameScore_LoseTheme = pINI->ReadTheme(pSection, "IngameScore.LoseTheme", this->IngameScore_LoseTheme);
	this->Sidebar_HarvesterCounter_Offset.Read(exINI, pSection, "Sidebar.HarvesterCounter.Offset");
	this->Sidebar_HarvesterCounter_Yellow.Read(exINI, pSection, "Sidebar.HarvesterCounter.ColorYellow");
	this->Sidebar_HarvesterCounter_Red.Read(exINI, pSection, "Sidebar.HarvesterCounter.ColorRed");
	this->Sidebar_ProducingProgress_Offset.Read(exINI, pSection, "Sidebar.ProducingProgress.Offset");
	this->Sidebar_PowerDelta_Offset.Read(exINI, pSection, "Sidebar.PowerDelta.Offset");
	this->Sidebar_PowerDelta_Green.Read(exINI, pSection, "Sidebar.PowerDelta.ColorGreen");
	this->Sidebar_PowerDelta_Grey.Read(exINI, pSection, "Sidebar.PowerDelta.ColorGrey");
	this->Sidebar_PowerDelta_Yellow.Read(exINI, pSection, "Sidebar.PowerDelta.ColorYellow");
	this->Sidebar_PowerDelta_Red.Read(exINI, pSection, "Sidebar.PowerDelta.ColorRed");
	this->Sidebar_PowerDelta_Align.Read(exINI, pSection, "Sidebar.PowerDelta.Align");

	this->ToolTip_Background_Color.Read(exINI, pSection, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, pSection, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, pSection, "ToolTip.Background.BlurSize");

	this->GClock_Shape.Read(exINI, pSection, "GClock.Shape");
	this->GClock_Transculency.Read(exINI, pSection, "GClock.Transculency");
	//this->GClock_Palette.Read(pINI, pSection, "GClock.Palette");

	this->SurvivorDivisor.Read(exINI, pSection, "SurvivorDivisor");
	this->Crew.Read(exINI, pSection, "Crew", true);
	this->Engineer.Read(exINI, pSection, "Engineer", true);
	this->Technician.Read(exINI, pSection, "Technician", true);
	this->ParaDropPlane.Read(exINI, pSection, "ParaDrop.Aircraft");
	this->SpyPlane.Read(exINI, pSection, "SpyPlane.Aircraft" , true);
	this->HunterSeeker.Read(exINI, pSection, "HunterSeeker", true);

	this->ParaDropTypes.Read(exINI, pSection, "ParaDrop.Types" , true);
	this->ParaDropNum.Read(exINI, pSection, "ParaDrop.Num");

	this->MessageTextColorIndex.Read(exINI, pSection, "MessageTextColor");
	this->ParachuteAnim.Read(exINI, pSection, "Parachute.Anim", true);

	this->EVAIndex.Read(exINI, pSection, "EVA.Tag");
	this->BriefingTheme = pINI->ReadTheme(pSection, "BriefingTheme", this->BriefingTheme);

	this->BaseDefenseCounts.Read(exINI, pSection, "AI.BaseDefenseCounts");
	this->BaseDefenses.Read(exINI, pSection, "AI.BaseDefenses", true);

	this->Disguise.Read(exINI, pSection, "DefaultDisguise", true);

	this->SidebarYuriFileNames.Read(exINI, pSection, "Sidebar.YuriFileNames");
	this->ToolTipTextColor.Read(exINI, pSection, "ToolTipColor");

	this->ScoreCampaignBackground.Read(pINI, pSection, "CampaignScore.Background");
	this->ScoreCampaignTransition.Read(pINI, pSection, "CampaignScore.Transition");
	this->ScoreCampaignAnimation.Read(pINI, pSection, "CampaignScore.Animation");
	this->ScoreCampaignPalette.Read(pINI, pSection, "CampaignScore.Palette");

	this->Sidebar_WeedsCounter_Offset.Read(exINI, pSection, "Sidebar.WeedsCounter.Offset");
	this->Sidebar_WeedsCounter_Color.Read(exINI, pSection, "Sidebar.WeedsCounter.Color");

	this->GraphicalTextImage.Read(pINI, pSection, "GraphicalText.Image");
	this->GraphicalTextPalette.Read(pINI, pSection, "GraphicalText.Palette");

	this->DialogBackgroundImage.Read(pINI, pSection, "DialogBackground.Image");
	this->DialogBackgroundPalette.Read(pINI, pSection, "DialogBackground.Palette");

	this->ScoreMultiplayBackground.Read(pINI, pSection, "MultiplayerScore.Background");
	this->ScoreMultiplayPalette.Read(pINI, pSection, "MultiplayerScore.Palette");
	this->ScoreMultiplayBars.Read(pINI, pSection, "MultiplayerScore.Bars");

	for (unsigned int i = 0; i < 10; ++i) {
		auto pFilename = this->GetMultiplayerScoreBarFilename(i);
		if (!PCX::Instance->GetSurface(pFilename)) {
			PCX::Instance->LoadFile(pFilename);
		}
	}

	this->ScoreCampaignThemeUnderPar.Read(pINI, pSection, "CampaignScore.UnderParTheme");
	this->ScoreCampaignThemeOverPar.Read(pINI, pSection, "CampaignScore.OverParTheme");

	this->ScoreMultiplayThemeWin.Read(pINI, pSection, "MultiplayerScore.WinTheme");
	this->ScoreMultiplayThemeLose.Read(pINI, pSection, "MultiplayerScore.LoseTheme");

	this->SidebarMixFileIndex.Read(exINI, pSection, "Sidebar.MixFileIndex");
	this->MouseShape.Read(exINI, pSection, "MouseShape");

	this->ExclusiveSWSidebar_TopPCX.Read(pINI, pSection, "ExclusiveSWSidebar.TopPCX");
	this->ExclusiveSWSidebar_CenterPCX.Read(pINI, pSection, "ExclusiveSWSidebar.CenterPCX");
	this->ExclusiveSWSidebar_BottomPCX.Read(pINI, pSection, "ExclusiveSWSidebar.BottomPCX");
	this->ExclusiveSWSidebar_ToggleShape.Read(exINI, pSection, "ExclusiveSWSidebar.ToggleShape");
}

void SideExtData::UpdateGlobalFiles()
{
	// clear old data
	SideExtData::s_GraphicalTextImage = nullptr;
	SideExtData::s_GraphicalTextConvert = nullptr;

	SideExtData::s_DialogBackgroundImage = nullptr;
	SideExtData::s_DialogBackgroundConvert = nullptr;

	int idxSide = ScenarioClass::Instance->PlayerSideIndex;
	auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
	if (!pSide) {
		return;
	}

	auto pExt = SideExtContainer::Instance.Find(pSide);

	// load graphical text shp
	if (pExt->GraphicalTextImage)
	{
		auto pShp = FileSystem::AllocateFile<SHPStruct>(pExt->GraphicalTextImage);
		SideExtData::s_GraphicalTextImage.reset(pShp);
	}

	// load graphical text palette and create convert
	if (pExt->GraphicalTextPalette) {
		SideExtData::s_GraphicalTextConvert =
			PaletteManager::FindOrAllocate(pExt->GraphicalTextPalette)
				->GetConvert<PaletteManager::Mode::Temperate>();
	}

	// load dialog background shp
	if (pExt->DialogBackgroundImage)
	{
		auto pShp = FileSystem::AllocateFile<SHPStruct>(pExt->DialogBackgroundImage);
		SideExtData::s_DialogBackgroundImage.reset(pShp);
	}

	// load dialog background palette and create convert
	if (pExt->DialogBackgroundPalette)
	{
		SideExtData::s_DialogBackgroundConvert =
			PaletteManager::FindOrAllocate(pExt->DialogBackgroundPalette)
				->GetConvert<PaletteManager::Mode::Default>();
	}
}

// =============================
// load / save

template <typename T>
void SideExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->ArrayIndex)
		.Process(this->Sidebar_GDIPositions)

		.Process(this->IngameScore_WinTheme)
		.Process(this->IngameScore_LoseTheme)

		.Process(this->Sidebar_HarvesterCounter_Offset)
		.Process(this->Sidebar_HarvesterCounter_Yellow)
		.Process(this->Sidebar_HarvesterCounter_Red)
		.Process(this->Sidebar_ProducingProgress_Offset)
		.Process(this->Sidebar_PowerDelta_Offset)
		.Process(this->Sidebar_PowerDelta_Green)
		.Process(this->Sidebar_PowerDelta_Grey)
		.Process(this->Sidebar_PowerDelta_Yellow)
		.Process(this->Sidebar_PowerDelta_Red)
		.Process(this->Sidebar_PowerDelta_Align)
		.Process(this->Sidebar_WeedsCounter_Offset)
		.Process(this->Sidebar_WeedsCounter_Color)
		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)

		.Process(this->GClock_Shape)
		.Process(this->GClock_Transculency)
		//.Process(this->GClock_Palette)


		.Process(this->SurvivorDivisor)
		.Process(this->Crew)
		.Process(this->Engineer)
		.Process(this->Technician)
		.Process(this->ParaDropPlane)
		.Process(this->SpyPlane)
		.Process(this->HunterSeeker)

		.Process(this->ParaDropTypes)
		.Process(this->ParaDropNum)

		.Process(this->MessageTextColorIndex)

		.Process(this->ParachuteAnim)
		.Process(this->EVAIndex)
		.Process(this->BriefingTheme)

		.Process(this->BaseDefenses)
		.Process(this->BaseDefenseCounts)

		.Process(this->Disguise)

		.Process(this->SidebarYuriFileNames)
		.Process(this->ToolTipTextColor)

		.Process(this->ScoreMultiplayBackground)
		.Process(this->ScoreMultiplayPalette)
		.Process(this->ScoreMultiplayBars)

		.Process(this->ScoreCampaignBackground)
		.Process(this->ScoreCampaignTransition)
		.Process(this->ScoreCampaignAnimation)
		.Process(this->ScoreCampaignPalette)

		.Process(this->ScoreCampaignThemeUnderPar)
		.Process(this->ScoreCampaignThemeOverPar)
		.Process(this->ScoreMultiplayThemeWin)
		.Process(this->ScoreMultiplayThemeLose)

		.Process(this->SidebarMixFileIndex)
		.Process(this->MouseShape)

		.Process(this->ExclusiveSWSidebar_TopPCX)
		.Process(this->ExclusiveSWSidebar_CenterPCX)
		.Process(this->ExclusiveSWSidebar_BottomPCX)
		.Process(this->ExclusiveSWSidebar_ToggleShape)
		;
}

bool SideExtData::LoadGlobals(PhobosStreamReader& Stm)
{
	auto ret = Stm
		.Process(SideExtData::CurrentLoadTextColor)
		.Success();

	SideExtData::UpdateGlobalFiles();

	return ret;
}

bool SideExtData::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(SideExtData::CurrentLoadTextColor)
		.Success();
}

// =============================
// container
SideExtContainer SideExtContainer::Instance;
// =============================
// container hooks

DEFINE_HOOK(0x6A4600, SideClass_CTOR, 0x6)
{
	GET(SideClass*, pItem, ESI);
	GET(int, nIdx, EAX);

	if(auto pExt = SideExtContainer::Instance.Allocate(pItem))
		pExt->ArrayIndex = nIdx;

	return 0;
}

DEFINE_HOOK(0x6A499F, SideClass_SDDTOR, 0x6)
{
	GET(SideClass*, pItem, ESI);

	SideExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6A48A0, SideClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6A4780, SideClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(SideClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SideExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6A488B, SideClass_Load_Suffix, 0x6)
{
   	SideExtContainer::Instance.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x6A48FC, SideClass_Save_Suffix, 0x5)
{
	SideExtContainer::Instance.SaveStatic();
	return 0;
}

 //DEFINE_HOOK(0x679A10, SideClass_LoadAllFromINI, 0x5)
 //{
 //	GET_STACK(CCINIClass*, pINI, 0x4);

 //	for (auto pSide : *SideClass::Array) {
 //		SideExtContainer::Instance.LoadFromINI(pSide, pINI, !pINI->GetSection(pSide->ID));
 //	}

 //	return 0;
 //}

/*
FINE_HOOK(6725C4, RulesClass_Addition_Sides, 8)
{
	GET(SideClass *, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x38);

	SideExtContainer::Instance.LoadFromINI(pItem, pINI);
	return 0;
}
*/