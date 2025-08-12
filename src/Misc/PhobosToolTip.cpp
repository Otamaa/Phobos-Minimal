#include <Helpers/Macro.h>

#include "PhobosToolTip.h"

#include <Ext/BuildingType/Body.h>

#include <Ext/Side/Body.h>
#include <Ext/Surface/Body.h>
#include <Ext/Scenario/Body.h>

#include <Utilities/Cast.h>

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <HouseClass.h>

#include <GameOptionsClass.h>
#include <CCToolTip.h>
#include <BitFont.h>
#include <BitText.h>
#include <FPSCounter.h>
#include <SidebarClass.h>

#include <sstream>
#include <iomanip>

#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>
#include <Misc/PhobosGlobal.h>

#include <New/SuperWeaponSidebar/SWSidebarClass.h>
#include <New/SuperWeaponSidebar/SWButtonClass.h>
#include <New/SuperWeaponSidebar/SWColumnClass.h>
#include <New/SuperWeaponSidebar/ToggleSWButtonClass.h>

#include <YRMath.h>
#include <Phobos.h>

PhobosToolTip PhobosToolTip::Instance;

OPTIONALINLINE const wchar_t* PhobosToolTip::GetUIDescription(TechnoTypeExtData* pData) const
{
	return Phobos::Config::ToolTipDescriptions && !pData->UIDescription.Get().empty()
		? pData->UIDescription.Get().Text
		: nullptr;
}

OPTIONALINLINE const wchar_t* PhobosToolTip::GetUnbuildableUIDescription(TechnoTypeExtData* pData) const
{
	return Phobos::Config::ToolTipDescriptions && !pData->UIDescription_Unbuildable.Get().empty()
		? pData->UIDescription_Unbuildable.Get().Text
		: nullptr;
}

OPTIONALINLINE const wchar_t* PhobosToolTip::GetUIDescription(SWTypeExtData* pData) const
{
	return Phobos::Config::ToolTipDescriptions && !pData->UIDescription.Get().empty()
		? pData->UIDescription.Get().Text
		: nullptr;
}

OPTIONALINLINE int PhobosToolTip::GetBuildTime(TechnoTypeClass* pType) const
{
	// TechnoTypeClass only has 4 final classes :
	// BuildingTypeClass, AircraftTypeClass, InfantryTypeClass and UnitTypeClass
	// It has to be these four classes, otherwise pType will just be
	static char BuildTimeDatas[sizeof(BuildingClass)] = {};

	switch (pType->WhatAmI())
	{
	case BuildingTypeClass::AbsID:
		*reinterpret_cast<int*>(BuildTimeDatas) = BuildingClass::vtable;
		reinterpret_cast<BuildingClass*>(BuildTimeDatas)->Type = (BuildingTypeClass*)pType;
		break;
	case AircraftTypeClass::AbsID:
		*reinterpret_cast<int*>(BuildTimeDatas) = AircraftClass::vtable;
		reinterpret_cast<AircraftClass*>(BuildTimeDatas)->Type = (AircraftTypeClass*)pType;
		break;
	case InfantryTypeClass::AbsID:
		*reinterpret_cast<int*>(BuildTimeDatas) = InfantryClass::vtable;
		reinterpret_cast<InfantryClass*>(BuildTimeDatas)->Type = (InfantryTypeClass*)pType;
		break;
	case UnitTypeClass::AbsID:
		*reinterpret_cast<int*>(BuildTimeDatas) = UnitClass::vtable;
		reinterpret_cast<UnitClass*>(BuildTimeDatas)->Type = (UnitTypeClass*)pType;
		break;
	default:
			return 54;
	}

	const auto pTrick = reinterpret_cast<TechnoClass*>(BuildTimeDatas);
	pTrick->Owner = HouseClass::CurrentPlayer();

	const int nTimeToBuild = pTrick->TimeToBuild();
	// 54 frames at least
	return MaxImpl(54, nTimeToBuild);
}

OPTIONALINLINE int PhobosToolTip::GetPower(TechnoTypeClass* pType) const
{
	switch (pType->WhatAmI())
	{
	case AbstractType::AircraftType:
	case AbstractType::InfantryType:
	case AbstractType::UnitType:
	{
		return TechnoTypeExtContainer::Instance.Find(pType)->Power;
	}
	case AbstractType::BuildingType:
	{
		auto pBldType = (BuildingTypeClass*)pType;
		return pBldType->PowerBonus - pBldType->PowerDrain;
	}
	default:
		break;
	}
	return 0;
}

void PhobosToolTip::HelpText(const BuildType* cameo)
{
	if (cameo->ItemType == AbstractType::Special) {
		this->HelpText(HouseClass::CurrentPlayer->Supers.Items[cameo->ItemIndex]);
	} else {
		this->HelpText(ObjectTypeClass::FetchTechnoType(cameo->ItemType, cameo->ItemIndex));
	}
}

struct TimerDatas {
	int m_buildtimeresult;
	int m_second;
	int m_min;
};
#ifdef debug_timer
static std::map<TechnoTypeClass*, TimerDatas> Timers;
#endif
void PhobosToolTip::HelpText(TechnoTypeClass* pType)
{
	if (!pType)
		return;

	const auto pData = TechnoTypeExtContainer::Instance.Find(pType);

	const int nBuildTime = TickTimeToSeconds(this->GetBuildTime(pType));

	const int nSec = nBuildTime % 60;
	const int nMin = nBuildTime / 60 /* % 60*/;
	// int nHour = TickTimeToSeconds(nBuildTime) / 60 / 60;

#ifdef debug_timer
	if (!Timers.contains(pType))
		Timers.insert({ pType, {nBuildTime , nSec , nMin} });
	else
	{
		auto data = &Timers[pType];

		if (data->m_buildtimeresult != nBuildTime)
			Debug::FatalError("[%s] change BuildTime result from [%d] to [%d]!", pType->ID, data->m_buildtimeresult, nBuildTime);

		if (data->m_second != nSec)
			Debug::FatalError("[%s] change Second result from [%d] to [%d] [BuildTime %s]!", pType->ID, data->m_second, nSec , data->m_buildtimeresult);

		if (data->m_min != nMin)
			Debug::FatalError("[%s] change Min result from [%d] to [%d]!", pType->ID, data->m_min, nMin);
	}
#endif
	const int cost = pType->GetActualCost(HouseClass::CurrentPlayer);
	//const auto pHouseExt = HouseExtContainer::Instance.Find(HouseClass::CurrentPlayer());

#ifndef _useFMT
	this->TextBuffer.clear();
	fmt::format_to(std::back_inserter(this->TextBuffer),
		L"{}\n{}{}{} {}{:02}:{:02}",
		pType->UIName,
		(cost < 0 ? L"+" : L""),
		Phobos::UI::CostLabel,
		Math::abs(cost),
		Phobos::UI::TimeLabel,
		nMin,
		nSec
	);

	if (auto const nPower = this->GetPower(pType)) {
		fmt::format_to(std::back_inserter(this->TextBuffer),
			L" {}{}{:01}",
			Phobos::UI::PowerLabel,
			nPower > 0 ? L"+" : L"",
			nPower);
	}

	if (auto pDesc = this->GetUIDescription(pData)) {
		fmt::format_to(std::back_inserter(this->TextBuffer), L"\n{}", pDesc);
	}

	if (pData->Cameo_AlwaysExist.Get(RulesExtData::Instance()->Cameo_AlwaysExist)) {
		auto& vec = ScenarioExtData::Instance()->OwnedExistCameoTechnoTypes;

		if (vec.contains(pType)) {
			if (auto pExDesc = this->GetUnbuildableUIDescription(pData))
				fmt::format_to(std::back_inserter(this->TextBuffer), L"\n{}", pExDesc);
		}
	}

#else
	std::wostringstream oss;
	oss << pType->UIName << L"\n"
		<< (cost < 0 ? L"+" : L"")
		<< Phobos::UI::CostLabel << Math::abs(cost) << L" "
		<< Phobos::UI::TimeLabel
		// << std::setw(2) << std::setfill(L'0') << nHour << L":"
		<< std::setw(2) << std::setfill(L'0') << nMin << L":"
		<< std::setw(2) << std::setfill(L'0') << nSec;

	if (auto const nPower = this->GetPower(pType))
	{
		oss << L" " << Phobos::UI::PowerLabel;
		if (nPower > 0)
			oss << L"+";
		oss << std::setw(1) << nPower;
	}


	// the value is not consistent
	// so showing it here will just cause fuckton of confusion
	//const int nPoints = pHouseExt->CalculateBattlePoints(pType);
	//if (Math::abs(nPoints)) {
	//	oss << L"\n";
	//
	//	if (nPoints > 0)
	//		oss << Phobos::UI::BattlePoints_Label << L"+" << nPoints;
	//	else if (nPoints < 0)
	//		oss << Phobos::UI::BattlePoints_Label << L"-" << nPoints;
	//
	//}

	if (auto pDesc = this->GetUIDescription(pData))
		oss << L"\n" << pDesc;

	if (pData->Cameo_AlwaysExist.Get(RulesExtData::Instance()->Cameo_AlwaysExist)) {
		auto& vec = ScenarioExtData::Instance()->OwnedExistCameoTechnoTypes;

		if (vec.contains(pType)) {
			if (auto pExDesc = this->GetUnbuildableUIDescription(pData))
				oss << L"\n" << pExDesc;
		}
	}

	this->TextBuffer = oss.str();

#endif
}

int PhobosToolTip::TickTimeToSeconds(int tickTime)
{
	//if(!Phobos::Config::RealTimeTimers)
		return tickTime / 15;

	//if (Phobos::Config::RealTimeTimers_Adaptive ||
	//GameOptionsClass::Instance->GameSpeed == 0
	//|| (Phobos::Misc::CustomGS && !SessionClass::IsMultiplayer())
	//)
	//{
	//const auto nCur = (int)FPSCounter::CurrentFrameRate;
	//	return tickTime / MaxImpl(nCur, 1);
	//}

	//return tickTime / (60 / GameOptionsClass::Instance->GameSpeed);
}

void PhobosToolTip::HelpText(SuperClass* pSuper)
{
	const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
#ifndef _useFMT

	this->TextBuffer.clear();
	bool showSth = false;

	fmt::format_to(std::back_inserter(this->TextBuffer),
		L"{}", pSuper->Type->UIName
	);

	if (const int nCost = Math::abs(pData->Money_Amount.Get())) {
		fmt::format_to(std::back_inserter(this->TextBuffer),
			L"\n{}{}{}",
			pData->Money_Amount > 0 ? L"+" : L"",
			Phobos::UI::CostLabel,
			nCost
		);

		showSth = true;
	}

	if (int nPoints = Math::abs(pData->BattlePoints_Amount.Get())) {
		fmt::format_to(std::back_inserter(this->TextBuffer),
			L"\n{}{}{}",
			Phobos::UI::BattlePoints_Label,
			pData->BattlePoints_Amount > 0 ? L"+" : L"-",
			nPoints
		);

		showSth = true;
	}

	const int rechargeTime = TickTimeToSeconds(pSuper->GetRechargeTime());

	if (rechargeTime > 0) {
		const int nSec = rechargeTime % 60;
		const int nMin = rechargeTime / 60 /* % 60*/;
		const int nHour = rechargeTime / 60 / 60;

		if(nHour) {
			fmt::format_to(std::back_inserter(this->TextBuffer),
				L"{}{}{} {:02}{:02}:{:02}",
				!showSth ? L"\n" : L"",
				showSth ? L" " : L"",
				Phobos::UI::TimeLabel,
				nHour,
				nMin,
				nSec
			);
		}
		else
		{
			fmt::format_to(std::back_inserter(this->TextBuffer),
				L"{}{}{} {:02}:{:02}",
				!showSth ? L"\n" : L"",
				showSth ? L" " : L"",
				Phobos::UI::TimeLabel,
				nMin,
				nSec
			);
		}

		showSth = true;

	}

	auto const& sw_ext = HouseExtContainer::Instance.Find(pSuper->Owner)->GetShotCount(pSuper->Type);

	if (pData->SW_Shots > 0) {
		wchar_t buffer[64];
		swprintf_s(buffer, Phobos::UI::SWShotsFormat, (pData->SW_Shots - sw_ext.Count), pData->SW_Shots);

		fmt::format_to(std::back_inserter(this->TextBuffer),
			L"{}{}{}",
			!showSth ? L"\n" : L"",
			(showSth ? L" " : L""),
			buffer
		);
	}

	if (pData->SW_Power.isset()) {
		const auto nPower = pData->SW_Power.Get();

		if (nPower != 0) {
			fmt::format_to(std::back_inserter(this->TextBuffer),
				L" {}{}{:01}",
				Phobos::UI::PowerLabel,
				(nPower ? L"+" : L""),
				nPower
			);
		}
	}

	if (auto pDesc = GetUIDescription(pData))
		fmt::format_to(std::back_inserter(this->TextBuffer),
			L"\n{}",
			pDesc
		);
#else

	std::wostringstream oss;
	oss << pSuper->Type->UIName;
	bool showSth = false;

	if (const int nCost = Math::abs(pData->Money_Amount.Get()))
	{
		oss << L"\n";

		if (pData->Money_Amount > 0)
			oss << '+';

		oss << Phobos::UI::CostLabel << nCost;
		showSth = true;
	}

	if (int nPoints = Math::abs(pData->BattlePoints_Amount.Get()))
	{
		oss << L"\n";

		if (pData->BattlePoints_Amount > 0)
			oss << Phobos::UI::BattlePoints_Label << L"+" << nPoints;
		else if (pData->BattlePoints_Amount < 0)
			oss << Phobos::UI::BattlePoints_Label << L"-" << nPoints;

		showSth = true;
	}

	const int rechargeTime = TickTimeToSeconds(pSuper->GetRechargeTime());

	if (rechargeTime > 0)
	{
		if (!showSth)
			oss << L"\n";

		const int nSec = rechargeTime % 60;
		const int nMin = rechargeTime / 60 /* % 60*/;
		// int nHour = TickTimeToSeconds(pType->RechargeTime) / 60 / 60;

		oss << (showSth ? L" " : L"") << Phobos::UI::TimeLabel
			// << std::setw(2) << std::setfill(L'0') << nHour << L":"
			<< std::setw(2) << std::setfill(L'0') << nMin << L":"
			<< std::setw(2) << std::setfill(L'0') << nSec;

		showSth = true;
	}

	auto const& sw_ext = HouseExtContainer::Instance.Find(pSuper->Owner)->GetShotCount(pSuper->Type);
	int sw_shots = pData->SW_Shots;
	int remain_shots = pData->SW_Shots - sw_ext.Count;
	if (sw_shots > 0) {

		if (!showSth)
			oss << L"\n";

		wchar_t buffer[64];
		swprintf_s(buffer, Phobos::UI::SWShotsFormat, remain_shots, sw_shots);
		oss << (showSth ? L" " : L"") << buffer;
	}

	if(pData->SW_Power.isset()) {
		const auto nPower = pData->SW_Power;

		if (nPower != 0) {
			oss << (L" ") << Phobos::UI::PowerLabel;
			if (nPower > 0)
				oss << L"+";

			oss << std::setw(1) << nPower;
		}
	}

	if (auto pDesc = GetUIDescription(pData))
		oss << L"\n" << pDesc;

	this->TextBuffer = oss.str();
#endif
}

// Hooks
ASMJIT_PATCH(0x4AE51E, DisplayClass_GetToolTip_TacticalButton, 0x6)
{
	if (SWSidebarClass::IsEnabled() && !ScenarioClass::Instance->UserInputLocked)
	{
		const auto swSidebar = SWSidebarClass::Global();

		if (const auto button = swSidebar->CurrentButton)
		{
			PhobosToolTip::Instance.IsCameo = true;
			const auto pSuper = HouseClass::CurrentPlayer->Supers[button->SuperIndex];

			if (Phobos::UI::ExtendedToolTips)
			{
				PhobosToolTip::Instance.HelpText(pSuper);
				R->EAX(PhobosToolTip::Instance.GetBuffer());
			}
			else
			{
				R->EAX(pSuper->Type->UIName);
			}

			return 0x4AE69D;
		}
		else if (swSidebar->CurrentColumn || (swSidebar->ToggleButton && swSidebar->ToggleButton->IsHovering))
		{
			R->EAX(0);
			return 0x4AE69D;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x724247, ToolTipManager_ProcessMessage_SetDelayTimer, 0x6)
{
	return SWSidebarClass::IsEnabled() && SWSidebarClass::Global()->CurrentButton ? 0x72429E : 0;
}

ASMJIT_PATCH(0x72428C, ToolTipManager_ProcessMessage_Redraw, 0x5)
{
	return SWSidebarClass::IsEnabled() && SWSidebarClass::Global()->CurrentButton ? 0x724297 : 0;
}

ASMJIT_PATCH(0x724B28, ToolTipManager_SetX_TacticalButtons, 0x6)
{
	if (SWSidebarClass::IsEnabled()) {
		if (const auto button = SWSidebarClass::Global()->CurrentButton) {
			R->EDX(button->Rect.X + button->Rect.Width);
			R->EAX(button->Rect.Y + 27);
			return 0x724B2E;
		}
	}

	return 0;
}


// TODO: reimplement CCToolTip::Draw2 completely
ASMJIT_PATCH(0x478EE1, CCToolTip_Draw2_SetBuffer, 0x6)
{
	if (Phobos::UI::ExtendedToolTips && PhobosToolTip::Instance.IsCameo)
		R->EDI(PhobosToolTip::Instance.GetBuffer());

	return 0;
}

ASMJIT_PATCH(0x478E10, CCToolTip_Draw1, 0x6) //0
{
	GET(CCToolTip*, pThis, ECX);
	GET_STACK(bool, bFullRedraw, 0x4);

	// !onSidebar or (onSidebar && ExtToolTip::IsCameo)
	if (!bFullRedraw || PhobosToolTip::Instance.IsCameo)
	{
		PhobosToolTip::Instance.IsCameo = false;
		PhobosToolTip::Instance.SlaveDraw = false;

		pThis->ToolTipManager::Process();	//this function re-create CCToolTip
	}

	if (pThis->CurrentToolTip)
	{
		if (!bFullRedraw)
			PhobosToolTip::Instance.SlaveDraw = PhobosToolTip::Instance.IsCameo;

		pThis->FullRedraw = bFullRedraw;
		pThis->DrawText(pThis->CurrentToolTipData);
	}
	return 0x478E25;
}

ASMJIT_PATCH(0x478E4A, CCToolTip_Draw2_SetSurface, 0x6)
{
	if (PhobosToolTip::Instance.SlaveDraw || RulesExtData::Instance()->ToolTip_ExcludeSidebar)
	{
		R->ESI(DSurface::Composite());
		return 0x478ED3;
	}
	return 0;
}

ASMJIT_PATCH(0x478EF8, CCToolTip_Draw2_SetMaxWidth, 0x5)
{
	if (PhobosToolTip::Instance.IsCameo)
	{
		if (Phobos::UI::MaxToolTipWidth > 0)
			R->EAX(Phobos::UI::MaxToolTipWidth);
		else
			R->EAX(DSurface::ViewBounds->Width);

	}
	return 0;
}

ASMJIT_PATCH(0x478F52, CCToolTip_Draw2_SetX, 0x8)
{
	if (PhobosToolTip::Instance.SlaveDraw && !RulesExtData::Instance()->ToolTip_ExcludeSidebar)
		R->EAX(R->EAX() + DSurface::Sidebar->Get_Width());

	return 0;
}

ASMJIT_PATCH(0x478F77, CCToolTip_Draw2_SetY, 0x6)
{
	if (PhobosToolTip::Instance.IsCameo)
	{
		LEA_STACK(RectangleStruct*, Rect, STACK_OFFS(0x3C, 0x20));

		const int maxHeight = DSurface::ViewBounds->Height - 32;

		if (Rect->Height > maxHeight)
			Rect->Y += maxHeight - Rect->Height;

		if (Rect->Y < 0)
			Rect->Y = 0;
	}
	return 0;
}

// TODO in the future
//
//ASMJIT_PATCH(0x478E30, CCToolTip_Draw2, 0x7)
//{
//	GET(CCToolTip*, pThis, ECX);
//	GET_STACK(ToolTipManagerData*, pManagerData, 0x4);
//
//	DSurface* pSurface = nullptr;
//
//	RectangleStruct bounds = pManagerData->Dimension;
//
//	if (GameOptionsClass::Instance->SidebarSide == 1)
//	{
//		int nR = DSurface::ViewBounds->X + DSurface::ViewBounds->Width;
//		if (bounds.X + pManagerData->Dimension.Width <= nR)
//			pSurface = DSurface::Composite;
//		else
//		{
//			if (!pThis->FullRedraw || bounds.X < nR)
//				return 0x479048;
//			pSurface = DSurface::Sidebar;
//			bounds.X -= nR;
//			*reinterpret_cast<bool*>(0xB0B518) = true;
//		}
//	}
//	else
//	{
//		int nR = DSurface::SidebarBounds->X + DSurface::SidebarBounds->Width;
//		if (bounds.X < nR)
//		{
//			if (!pThis->FullRedraw || bounds.X + pManagerData->Dimension.Width >= nR)
//				return 0x479048;
//			pSurface = DSurface::Sidebar;
//			*reinterpret_cast<bool*>(0xB0B518) = true;
//		}
//		else
//		{
//			pSurface = DSurface::Composite;
//			bounds.X -= nR;
//		}
//	}
//
//	if (pSurface)
//	{
//		BitFont::Instance->GetTextDimension(
//			PhobosToolTip::Instance.GetBuffer(), bounds.Width, bounds.Height,
//			Phobos::UI::MaxToolTipWidth > 0 ? Phobos::UI::MaxToolTipWidth : DSurface::WindowBounds->Width);
//
//		if (pManagerData->Dimension.Width + bounds.X > pSurface->GetWidth())
//			bounds.X = pSurface->GetWidth() - pManagerData->Dimension.Width;
//
//		bounds.Width = pManagerData->Dimension.Width;
//		bounds.Height += 4;
//
//		BitFont::Instance->field_41 = 1;
//		BitFont::Instance->SetBounds(&bounds);
//		BitFont::Instance->Color = static_cast<WORD>(Drawing::RGB2DWORD(191, 98, 10));
//
//		BitText::Instance->DrawText(BitFont::Instance, pSurface, PhobosToolTip::Instance.GetBuffer(),
//			bounds.X + 4, bounds.Y + 2, bounds.Width, bounds.Height, 0, 0, 0);
//	}
//
//	return 0x479048;
//}

// If tooltip rectangle width is constrained, make sure
// there is a padding zone so text isn't drawn into border
ASMJIT_PATCH(0x479029, CCToolTip_Draw2_SetPadding, 0x5)
{
	if (PhobosToolTip::Instance.IsCameo) {
		if (Phobos::UI::MaxToolTipWidth > 0)
			R->EDX(R->EDX() - 5);
	}

	return 0;
}

void NAKED _CCToolTip_Draw2_FillRect_RET()
{
	ADD_ESP(8); // We need to handle origin two push here...
	JMP(0x478FE1);
}

ASMJIT_PATCH(0x478FDC, CCToolTip_Draw2_FillRect, 0x5)
{
	GET(SurfaceExt*, pThis, ESI);
		//GET(int, color, EDI);
	LEA_STACK(RectangleStruct*, pRect, STACK_OFFS(0x44, 0x10));

	const bool isCameo = PhobosToolTip::Instance.IsCameo;
	const auto swSidebar = SWSidebarClass::Global();

	if (isCameo && Phobos::UI::AnchoredToolTips
		&&Phobos::UI::ExtendedToolTips
		&& Phobos::Config::ToolTipDescriptions)
	{
		LEA_STACK(LTRBStruct*, pTextRect, STACK_OFFSET(0x44, -0x20));


		if (const auto pButton = swSidebar->CurrentButton)
		{
			// Being too far away may actually be bad
			/*
			const auto& columns = SWSidebarClass::Instance.Columns;
			const int size = columns.size();
			const int y = pButton->Y + 3;
			auto pColumn = columns.back();
			if (columns.size() > 1 && y > (pColumn->Y + pColumn->Height))
				pColumn = columns[size - 2];
			const int x = pColumn->X + pColumn->Width + 2;
			*/
			const auto pColumn = swSidebar->Columns[pButton->ColumnIndex];
			const int x = pColumn->Rect.X + pColumn->Rect.Width + 2;
			const int y = pButton->Rect.Y + 3;
			pRect->X = x;
			pTextRect->Right += (x - pTextRect->Left);
			pTextRect->Left = x;
			pRect->Y = y;
			pTextRect->Bottom += (y - pTextRect->Top);
			pTextRect->Top = y;
		}
		else
		{
			const int x = DSurface::SidebarBounds->X - pRect->Width - 2;
			pRect->X = x;
			pTextRect->Left = x;
			pRect->Y -= 40;
			pTextRect->Top -= 40;
		}
	}
	else if (const auto pButton = swSidebar->CurrentButton)
	{
		LEA_STACK(LTRBStruct*, pTextRect, STACK_OFFSET(0x44, -0x20));

		const int x = pButton->Rect.X + pButton->Rect.Width;
		const int y = pButton->Rect.Y + 43;
		pRect->X = x;
		pTextRect->Right += (x - pTextRect->Left);
		pTextRect->Left = x;
		pRect->Y = y;
		pTextRect->Bottom += (y - pTextRect->Top);
		pTextRect->Top = y;
	}

	// Should we make some SideExt items as static to improve the effeciency?
	// Though it might not be a big improvement... - secsome
	const int nPlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (const auto pSide = SideClass::Array->GetItemOrDefault(nPlayerSideIndex))
	{
		if (const auto pData = SideExtContainer::Instance.Find(pSide))
		{
			if(isCameo)
				SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true;

			pThis->Fill_Rect_Trans(pRect
				, pData->ToolTip_Background_Color.GetEx(RulesExtData::Instance()->ToolTip_Background_Color)
				, pData->ToolTip_Background_Opacity.Get(RulesExtData::Instance()->ToolTip_Background_Opacity)
			);

			if (Phobos::Config::ToolTipBlur)
				pThis->BlurRect(*pRect, pData->ToolTip_Background_BlurSize.Get(RulesExtData::Instance()->ToolTip_Background_BlurSize));

			return (int)_CCToolTip_Draw2_FillRect_RET;
		}
	}

	return 0;
}