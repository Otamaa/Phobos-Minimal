#include "Body.h"

#include <ThemeClass.h>
#include <Utilities/Helpers.h>

void SideExt::ExtData::Initialize() {

	const char* pID = this->OwnerObject()->ID;

	if (IS_SAME_STR_(pID, "Nod"))
	{ //Soviets
		this->EVAIndex = 1;
		this->MessageTextColorIndex = 11;
		this->SidebarYuriFileNames = false;
		this->ToolTipTextColor = ColorStruct(255, 255, 0);
	}
	else if (IS_SAME_STR_(pID, "ThirdSide"))
	{ //Yuri
		this->EVAIndex = 2;
		this->MessageTextColorIndex = 25;
		this->SidebarYuriFileNames = true;
		this->ToolTipTextColor = ColorStruct(255, 255, 0);
	}
	else
	{ //Allies or any other country
		this->EVAIndex = 0;
		this->MessageTextColorIndex = 21;
		this->SidebarYuriFileNames = false;
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
		break;
	case 1: // Soviet
		this->ScoreMultiplayBackground = "MPSSCRNL.SHP";
		this->ScoreMultiplayPalette = "MPSSCRN.PAL";

		this->ScoreCampaignBackground = "SSCRBKMD.SHP";
		this->ScoreCampaignTransition = "SSCRTMD.SHP";
		this->ScoreCampaignAnimation = "SSCRAMD.SHP";
		this->ScoreCampaignPalette = "SSCORE.PAL";
		break;
	default: // Yuri and others
		this->ScoreMultiplayBackground = "MPYSCRNL.SHP";
		this->ScoreMultiplayPalette = "MPYSCRN.PAL";

		this->ScoreCampaignBackground = "SYCRBKMD.SHP";
		this->ScoreCampaignTransition = "SYCRTMD.SHP";
		this->ScoreCampaignAnimation = "SYCRAMD.SHP";
		this->ScoreCampaignPalette = "YSCORE.PAL";
		break;
	}
};

bool SideExt::isNODSidebar()
{
	auto const PlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (const auto pSide = SideClass::Array->GetItemOrDefault(PlayerSideIndex)) {
		return !SideExt::ExtMap.Find(pSide)->Sidebar_GDIPositions.Get(PlayerSideIndex == 0);
	}

	return PlayerSideIndex == 0;
}

int SideExt::ExtData::GetSurvivorDivisor() const {
	return this->SurvivorDivisor.Get(this->GetDefaultSurvivorDivisor());
}

int SideExt::ExtData::GetDefaultSurvivorDivisor() const
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

InfantryTypeClass* SideExt::ExtData::GetCrew() const
{
	return this->Crew.Get(this->GetDefaultCrew());
}

InfantryTypeClass* SideExt::ExtData::GetDefaultCrew() const
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

InfantryTypeClass* SideExt::ExtData::GetEngineer() const
{
	return this->Engineer.Get(RulesClass::Instance->Engineer);
}

InfantryTypeClass* SideExt::ExtData::GetTechnician() const
{
	return this->Technician.Get(RulesClass::Instance->Technician);
}

UnitTypeClass* SideExt::ExtData::GetHunterSeeker() const
{
	return this->HunterSeeker.Get();
}

Iterator<TechnoTypeClass*> SideExt::ExtData::GetParaDropTypes() const
{
	if (this->ParaDropTypes.HasValue() && this->ParaDropNum.HasValue())
	{
		return this->ParaDropTypes;
	}

	return this->GetDefaultParaDropTypes();
}

Iterator<InfantryTypeClass*> SideExt::ExtData::GetDefaultParaDropTypes() const
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

Iterator<int> SideExt::ExtData::GetDefaultParaDropNum() const
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

Iterator<int> SideExt::ExtData::GetParaDropNum() const
{
	if (this->ParaDropTypes.HasValue() && this->ParaDropNum.HasValue()) {
		return this->ParaDropNum;
	}

	return this->GetDefaultParaDropNum();
}

Iterator<int> SideExt::ExtData::GetBaseDefenseCounts() const
{
	if (this->BaseDefenseCounts.HasValue()) {
		return this->BaseDefenseCounts;
	}

	return this->GetDefaultBaseDefenseCounts();
}

Iterator<int> SideExt::ExtData::GetDefaultBaseDefenseCounts() const
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

Iterator<BuildingTypeClass*> SideExt::ExtData::GetBaseDefenses() const
{
	if (this->BaseDefenses.HasValue()) {
		return this->BaseDefenses;
	}

	return this->GetDefaultBaseDefenses();
}

Iterator<BuildingTypeClass*> SideExt::ExtData::GetDefaultBaseDefenses() const
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

InfantryTypeClass* SideExt::ExtData::GetDisguise() const
{
	if (this->Disguise.isset()) {
		return this->Disguise;
	}

	return this->GetDefaultDisguise();
}

InfantryTypeClass* SideExt::ExtData::GetDefaultDisguise() const
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

void SideExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
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

	// remove all types that cannot paradrop
	Helpers::Alex::remove_non_paradroppables(this->ParaDropTypes, pSection, "ParaDrop.Types");

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
}

// =============================
// load / save

template <typename T>
void SideExt::ExtData::Serialize(T& Stm)
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

		.Process(this->ScoreCampaignBackground)
		.Process(this->ScoreCampaignTransition)
		.Process(this->ScoreCampaignAnimation)
		.Process(this->ScoreCampaignPalette)
		;
}

// =============================
// container
SideExt::ExtContainer SideExt::ExtMap;

// =============================
// container hooks

DEFINE_HOOK(0x6A4600, SideClass_CTOR, 0x6)
{
	GET(SideClass*, pItem, ESI);
	GET(int, nIdx, EAX);

	if(auto pExt = SideExt::ExtMap.Allocate(pItem))
		pExt->ArrayIndex = nIdx;

	return 0;
}

DEFINE_HOOK(0x6A499F, SideClass_SDDTOR, 0x6)
{
	GET(SideClass*, pItem, ESI);

	SideExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6A48A0, SideClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6A4780, SideClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(SideClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SideExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6A488B, SideClass_Load_Suffix, 0x6)
{
   	SideExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x6A48FC, SideClass_Save_Suffix, 0x5)
{
	SideExt::ExtMap.SaveStatic();
	return 0;
}

// DEFINE_HOOK(0x679A10, SideClass_LoadAllFromINI, 0x5)
// {
// 	GET_STACK(CCINIClass*, pINI, 0x4);
//
// 	for (auto pSide : *SideClass::Array) {
// 		SideExt::ExtMap.LoadFromINI(pSide, pINI, !pINI->GetSection(pSide->ID));
// 	}
//
// 	return 0;
// }

/*
FINE_HOOK(6725C4, RulesClass_Addition_Sides, 8)
{
	GET(SideClass *, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x38);

	SideExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
*/