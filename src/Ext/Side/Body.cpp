#include "Body.h"

#include <ThemeClass.h>
#include <Utilities/Helpers.h>

void SideExt::ExtData::Initialize()
{
	auto const nIdx = SideClass::Array->FindItemIndex(this->Get());
	this->ArrayIndex = nIdx;
	this->Sidebar_GDIPositions = nIdx == 0; // true = Allied
};

bool SideExt::isNODSidebar()
{
	auto const PlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (const auto pSide = SideClass::Array->GetItemOrDefault(PlayerSideIndex)) {
		return !SideExt::ExtMap.Find(pSide)->Sidebar_GDIPositions;
	}

	return PlayerSideIndex;
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
	if (this->ParaDropTypes.HasValue() && this->ParaDropNum.HasValue())
	{
		return this->ParaDropNum;
	}

	return this->GetDefaultParaDropNum();
}


void SideExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
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

	this->ParaDropTypes.Read(exINI, pSection, "ParaDrop.Types");

	// remove all types that cannot paradrop
	Helpers::Alex::remove_non_paradroppables(this->ParaDropTypes, pSection, "ParaDrop.Types");

	this->ParaDropNum.Read(exINI, pSection, "ParaDrop.Num");
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
		;
}

// =============================
// container
SideExt::ExtContainer SideExt::ExtMap;

SideExt::ExtContainer::ExtContainer() : Container("SideClass") { }
SideExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x6A4609, SideClass_CTOR, 0x7)
{
	GET(SideClass*, pItem, ESI);
	//GET(int, nIdx, ECX);
	SideExt::ExtMap.Allocate(pItem);
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

DEFINE_HOOK(0x679A10, SideClass_LoadAllFromINI, 0x5)
{
	GET_STACK(CCINIClass*, pINI, 0x4);

	for (auto pSide : *SideClass::Array) {
		SideExt::ExtMap.LoadFromINI(pSide, pINI, !pINI->GetSection(pSide->ID));
	}

	return 0;
}

/*
FINE_HOOK(6725C4, RulesClass_Addition_Sides, 8)
{
	GET(SideClass *, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x38);

	SideExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
*/