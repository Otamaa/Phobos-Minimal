//This file initializes static constant values.

#include <YRPP.h>
#include <YRPPGlobal.h>
#include <ASMMacros.h>
#include <YRPPCore.h>
#include <Unsorted.h>
#include <Helpers/Macro.h>

#include <ArrayClasses.h>
#include <TacticalClass.h>
#include <TechnoClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <BuildingClass.h>
#include <SlaveManagerClass.h>
#include <RulesClass.h>
#include <Drawing.h>
#include <MapClass.h>
#include <WarheadTypeClass.h>
#include <HouseClass.h>
#include <SuperClass.h>
#include <FactoryClass.h>
#include <ScenarioClass.h>
#include <FootClass.h>
#include <LocomotionClass.h>
#include <GameOptionsClass.h>
#include <WWMouseClass.h>
#include <CoordStruct.h>
#include <Fixed.h>
#include <CCINIClass.h>
#include <Surface.h>

const CoordStruct CoordStruct::Empty = { 0,0,0 };
const ColorStruct ColorStruct::Empty = { 0,0,0 };
const Color16Struct Color16Struct::Empty = { 0,0,0 };
const CellStruct CellStruct::Empty = { 0,0 };
const CellStruct CellStruct::DefaultUnloadCell = { 3 , 1 };
const Point2D Point2D::Empty = { 0,0 };
const Point2DBYTE Point2DBYTE::Empty = { 0u,0u };
const Point3D Point3D::Empty = { 0,0,0 };

#pragma region Imports
ALIAS(Imports::FP_OleSaveToStream, Imports::OleSaveToStream, 0x7E15F4);
ALIAS(Imports::FP_OleLoadFromStream, Imports::OleLoadFromStream, 0x7E15F8);
ALIAS(Imports::FP_CoRegisterClassObject, Imports::CoRegisterClassObject, 0x7E15D8);
ALIAS(Imports::FP_CoRevokeClassObject, Imports::CoRevokeClassObject, 0x7E15CC);
ALIAS(Imports::FP_TimeGetTime, Imports::TimeGetTime, 0x7E1530);
ALIAS(Imports::FP_GetUpdateRect, Imports::GetUpdateRect, 0x7E139C);
ALIAS(Imports::FP_GetKeyState, Imports::GetKeyState, 0x7E13A8);
ALIAS(Imports::FP_DefWindowProcA, Imports::DefWindowProcA, 0x7E1394);
ALIAS(Imports::FP_MoveWindow, Imports::MoveWindow, 0x7E1398);
ALIAS(Imports::FP_GetFocus, Imports::GetFocus, 0x7E13A0);
ALIAS(Imports::FP_GetDC, Imports::GetDC, 0x7E13A4);
ALIAS(Imports::FP_GetActiveWindow, Imports::GetActiveWindow, 0x7E13AC);
ALIAS(Imports::FP_GetCapture, Imports::GetCapture, 0x7E13B0);
ALIAS(Imports::FP_GetDlgCtrlID, Imports::GetDlgCtrlID, 0x7E13B4);
ALIAS(Imports::FP_ChildWindowFromPointEx, Imports::ChildWindowFromPointEx, 0x7E13B8);
ALIAS(Imports::FP_GetWindowRect, Imports::GetWindowRect, 0x7E13BC);
ALIAS(Imports::FP_GetCursorPos, Imports::GetCursorPos, 0x7E13C0);
ALIAS(Imports::FP_CloseWindow, Imports::CloseWindow, 0x7E13C4);
ALIAS(Imports::FP_EndDialog, Imports::EndDialog, 0x7E13C8);
ALIAS(Imports::FP_SetFocus, Imports::SetFocus, 0x7E13CC);
ALIAS(Imports::FP_SetDlgItemTextA, Imports::SetDlgItemTextA, 0x7E13D0);
ALIAS(Imports::FP_DialogBoxParamA, Imports::DialogBoxParamA, 0x7E13D4);
ALIAS(Imports::FP_DialogBoxIndirectParamA, Imports::DialogBoxIndirectParamA, 0x7E13D8);
ALIAS(Imports::FP_ShowCursor, Imports::ShowCursor, 0x7E13DC);
ALIAS(Imports::FP_GetAsyncKeyState, Imports::GetAsyncKeyState, 0x7E13E0);
ALIAS(Imports::FP_ToAscii, Imports::ToAscii, 0x7E13E4);
ALIAS(Imports::FP_MapVirtualKeyA, Imports::MapVirtualKeyA, 0x7E13E8);
ALIAS(Imports::FP_GetSystemMetrics, Imports::GetSystemMetrics, 0x7E13EC);
ALIAS(Imports::FP_SetWindowPos, Imports::SetWindowPos, 0x7E13F0);
ALIAS(Imports::FP_DestroyWindow, Imports::DestroyWindow, 0x7E13F4);
ALIAS(Imports::FP_ReleaseCapture, Imports::ReleaseCapture, 0x7E13F8);
ALIAS(Imports::FP_SetCapture, Imports::SetCapture, 0x7E13FC);
ALIAS(Imports::FP_AdjustWindowRectEx, Imports::AdjustWindowRectEx, 0x7E1400);
ALIAS(Imports::FP_GetMenu, Imports::GetMenu, 0x7E1404);
ALIAS(Imports::FP_AdjustWindowRect, Imports::AdjustWindowRect, 0x7E1408);
ALIAS(Imports::FP_GetSysColor, Imports::GetSysColor, 0x7E140C);
ALIAS(Imports::FP_IsDlgButtonChecked, Imports::IsDlgButtonChecked, 0x7E1410);
ALIAS(Imports::FP_CheckDlgButton, Imports::CheckDlgButton, 0x7E1414);
ALIAS(Imports::FP_WaitForInputIdle, Imports::WaitForInputIdle, 0x7E1418);
ALIAS(Imports::FP_GetTopWindow, Imports::GetTopWindow, 0x7E141C);
ALIAS(Imports::FP_GetForegroundWindow, Imports::GetForegroundWindow, 0x7E1420);
ALIAS(Imports::FP_LoadIconA, Imports::LoadIconA, 0x7E1424);
ALIAS(Imports::FP_SetActiveWindow, Imports::SetActiveWindow, 0x7E1428);
ALIAS(Imports::FP_RedrawWindow, Imports::RedrawWindow, 0x7E142C);
ALIAS(Imports::FP_GetWindowContextHelpId, Imports::GetWindowContextHelpId, 0x7E1430);
ALIAS(Imports::FP_WinHelpA, Imports::WinHelpA, 0x7E1434);
ALIAS(Imports::FP_ChildWindowFromPoint, Imports::ChildWindowFromPoint, 0x7E1438);
ALIAS(Imports::FP_LoadCursorA, Imports::LoadCursorA, 0x7E143C);
ALIAS(Imports::FP_SetCursor, Imports::SetCursor, 0x7E1440);
ALIAS(Imports::FP_PostQuitMessage, Imports::PostQuitMessage, 0x7E1444);
ALIAS(Imports::FP_FindWindowA, Imports::FindWindowA, 0x7E1448);
ALIAS(Imports::FP_SetCursorPos, Imports::SetCursorPos, 0x7E144C);
ALIAS(Imports::FP_CreateDialogIndirectParamA, Imports::CreateDialogIndirectParamA, 0x7E1450);
ALIAS(Imports::FP_GetKeyNameTextA, Imports::GetKeyNameTextA, 0x7E1454);
ALIAS(Imports::FP_ScreenToClient, Imports::ScreenToClient, 0x7E1458);
ALIAS(Imports::FP_LockWindowUpdate, Imports::LockWindowUpdate, 0x7E145C);
ALIAS(Imports::FP_MessageBoxA, Imports::MessageBoxA, 0x7E1460);
ALIAS(Imports::FP_ReleaseDC, Imports::ReleaseDC, 0x7E1464);
ALIAS(Imports::FP_WindowFromPoint, Imports::WindowFromPoint, 0x7E1468);
ALIAS(Imports::FP_UpdateWindow, Imports::UpdateWindow, 0x7E146C);
ALIAS(Imports::FP_SetWindowLongA, Imports::SetWindowLongA, 0x7E1470);
ALIAS(Imports::FP_GetWindowLongA, Imports::GetWindowLongA, 0x7E1474);
ALIAS(Imports::FP_ValidateRect, Imports::ValidateRect, 0x7E1478);
ALIAS(Imports::FP_IntersectRect, Imports::IntersectRect, 0x7E147C);
ALIAS(Imports::FP_MessageBoxIndirectA, Imports::MessageBoxIndirectA, 0x7E1480);
ALIAS(Imports::FP_PeekMessageA, Imports::PeekMessageA, 0x7E1484);
ALIAS(Imports::FP_CallWindowProcA, Imports::CallWindowProcA, 0x7E1488);
ALIAS(Imports::FP_KillTimer, Imports::KillTimer, 0x7E148C);
ALIAS(Imports::FP_SendDlgItemMessageA, Imports::SendDlgItemMessageA, 0x7E1490);
ALIAS(Imports::FP_SetTimer, Imports::SetTimer, 0x7E1494);
ALIAS(Imports::FP_ShowWindow, Imports::ShowWindow, 0x7E1498);
ALIAS(Imports::FP_InvalidateRect, Imports::InvalidateRect, 0x7E149C);
ALIAS(Imports::FP_EnableWindow, Imports::EnableWindow, 0x7E14A0);
ALIAS(Imports::FP_SendMessageA, Imports::SendMessageA, 0x7E14A4);
ALIAS(Imports::FP_GetDlgItem, Imports::GetDlgItem, 0x7E14A8);
ALIAS(Imports::FP_PostMessageA, Imports::PostMessageA, 0x7E14AC);
ALIAS(Imports::FP_wsprintfA, Imports::wsprintfA, 0x7E14B0);
ALIAS(Imports::FP_SetRect, Imports::SetRect, 0x7E14B4);
ALIAS(Imports::FP_ClientToScreen, Imports::ClientToScreen, 0x7E14B8);
ALIAS(Imports::FP_TranslateMessage, Imports::TranslateMessage, 0x7E14BC);
ALIAS(Imports::FP_DispatchMessageA, Imports::DispatchMessageA, 0x7E14C0);
ALIAS(Imports::FP_GetClientRect, Imports::GetClientRect, 0x7E14C4);
ALIAS(Imports::FP_GetWindow, Imports::GetWindow, 0x7E14C8);
ALIAS(Imports::FP_BringWindowToTop, Imports::BringWindowToTop, 0x7E14CC);
ALIAS(Imports::FP_SetForegroundWindow, Imports::SetForegroundWindow, 0x7E14D0);
ALIAS(Imports::FP_CreateWindowExA, Imports::CreateWindowExA, 0x7E14D4);
ALIAS(Imports::FP_RegisterClassA, Imports::RegisterClassA, 0x7E14D8);
ALIAS(Imports::FP_GetClassNameA, Imports::GetClassNameA, 0x7E14DC);
ALIAS(Imports::FP_IsWindowVisible, Imports::IsWindowVisible, 0x7E14E0);
ALIAS(Imports::FP_EnumChildWindows, Imports::EnumChildWindows, 0x7E14E4);
ALIAS(Imports::FP_IsWindowEnabled, Imports::IsWindowEnabled, 0x7E14E8);
ALIAS(Imports::FP_GetParent, Imports::GetParent, 0x7E14EC);
ALIAS(Imports::FP_GetNextDlgTabItem, Imports::GetNextDlgTabItem, 0x7E14F0);
ALIAS(Imports::FP_IsDialogMessageA, Imports::IsDialogMessageA, 0x7E14F4);
ALIAS(Imports::FP_TranslateAcceleratorA, Imports::TranslateAcceleratorA, 0x7E14F8);
ALIAS(Imports::FP_CharToOemBuffA, Imports::CharToOemBuffA, 0x7E14FC);
ALIAS(Imports::FP_BeginPaint, Imports::BeginPaint, 0x7E1500);
ALIAS(Imports::FP_EndPaint, Imports::EndPaint, 0x7E1504);
ALIAS(Imports::FP_CreateDialogParamA, Imports::CreateDialogParamA, 0x7E1508);
ALIAS(Imports::FP_GetWindowTextA, Imports::GetWindowTextA, 0x7E150C);
ALIAS(Imports::FP_RegisterHotKey, Imports::RegisterHotKey, 0x7E1510);
ALIAS(Imports::FP_InterlockedIncrement, Imports::InterlockedIncrement, 0x7E11C8);
ALIAS(Imports::FP_InterlockedDecrement, Imports::InterlockedDecrement, 0x7E11CC);
ALIAS(Imports::FP_DeleteCriticalSection, Imports::DeleteCriticalSection, 0x7E11E4);
ALIAS(Imports::FP_EnterCriticalSection, Imports::EnterCriticalSection, 0x7E11E8);
ALIAS(Imports::FP_LeaveCriticalSection, Imports::LeaveCriticalSection, 0x7E11EC);
ALIAS(Imports::FP_InitializeCriticalSection, Imports::InitializeCriticalSection, 0x7E11F4);
ALIAS(Imports::FP_Sleep, Imports::Sleep, 0x7E11F0);

#pragma endregion

#pragma region GlobalVarDeclaration
ALIAS(MouseClass, Map, 0x87F7E8)
ALIAS(GScreenClass, GScreen, 0x87F7E8)
ALIAS(CellClass, WorkingCellInstance, 0xABDC50)
ALIAS(RulesClass*,RulesGlobal,0x8871E0)
ALIAS(ScenarioClass*, ScenarioGlobal ,0xA8B230)
ALIAS(Random2Class, Random2Global ,0x886B88)
ALIAS(ParticleSystemClass*, ParticleSystemGlobal ,0xA8ED78)
ALIAS(GameOptionsClass, GameOptions,0xA8EB60)
ALIAS(GameModeOptionsClass, GameModeOptions ,0xA8B250)
ALIAS(TacticalClass*, TacticalGlobal,0x887324)
ALIAS(MessageListClass, MessageListGlobal,0xA8BC60)
ALIAS(SessionClass, SessionGlobal,0xA8B238)
ALIAS(WWMouseClass*, WWMouse,0x887640)
ALIAS(BombListClass , BombList , 0x87F5D8u)
ALIAS(DynamicVectorClass<ULONG>, ClassFactories, 0xB0BC88)
#pragma endregion

ALIAS(LPCRITICAL_SECTION, CRT_Critical_Sections, 0x87C2A8);
ALIAS(LPCRITICAL_SECTION, _87C2EC_Critical_Sections, 0x87C2EC);
ALIAS(LPCRITICAL_SECTION, _87C2DC_Critical_Sections, 0x87C2DC);
ALIAS(LPCRITICAL_SECTION, _87C2CC_Critical_Sections, 0x87C2CC);
ALIAS(LPCRITICAL_SECTION, _87C2AC_Critical_Sections, 0x87C2AC);

ALIAS(HANDLE, CRT_Heap, 0xB78B9C);
ALIAS(volatile LONG, _unguarded_readlc_active, 0xB78BA4);

#pragma region Array
ARRAY2D_DEF(0xB4669Cu, short, Wave_LUT_Pythagoras, 300, 300);
ARRAY2D_DEF(0xABC7F8u, Point2D, LaserClass_DrawData, 8, 2);
ARRAY2D_DEF(0x88A118u, char, AlphaShapeArray, 256, 256);
ARRAY2D_DEF(0xB07E80u,SelectClass*, SelectButton, 1, 14);
#pragma endregion

void SlaveManagerClass::ZeroOutSlaves() {
	for(const auto& pNode : this->SlaveNodes) {
		if(auto pSlave = pNode->Slave) {
			pSlave->SlaveOwner = nullptr;
		}
		pNode->Slave = nullptr;
		pNode->State = SlaveControlStatus::Dead;
		pNode->RespawnTimer.Start(this->RegenRate);
	}
}

bool ObjectClass::IsOnMyView() const
{
	auto const coords = this->GetCoords();
	auto const Point = TacticalClass::Instance->CoordsToView(coords);
	return Point.X > Drawing::SurfaceDimensions_Hidden().X
		&& Point.Y > Drawing::SurfaceDimensions_Hidden().Y
		&& Point.X < Drawing::SurfaceDimensions_Hidden().X + Drawing::SurfaceDimensions_Hidden().Width
		&& Point.Y < Drawing::SurfaceDimensions_Hidden().Y + Drawing::SurfaceDimensions_Hidden().Height;

}

bool ObjectClass::IsGreenToYellowHP() const
{
	return this->Health / this->GetTechnoType()->Strength
		>= RulesClass::Instance->ConditionYellow;
}

bool ObjectClass::IsFullHP() const
{ return this->GetHealthPercentage() >= RulesClass::Instance->ConditionGreen; }

double ObjectClass::GetHealthPercentage_() const
{ return static_cast<double>(this->Health) / this->GetType()->Strength; }

int HouseClass::CountOwnedNow(const TechnoTypeClass* const pItem) const {
	switch(pItem->WhatAmI()) {
	case AbstractType::BuildingType:
		return this->CountOwnedNow(
			static_cast<BuildingTypeClass const*>(pItem));

	case AbstractType::UnitType:
		return this->CountOwnedNow(
			static_cast<UnitTypeClass const*>(pItem));

	case AbstractType::InfantryType:
		return this->CountOwnedNow(
			static_cast<InfantryTypeClass const*>(pItem));

	case AbstractType::AircraftType:
		return this->CountOwnedNow(
			static_cast<AircraftTypeClass const*>(pItem));

	default:
		__assume(0);
	}
}

int HouseClass::CountOwnedAndPresent(const TechnoTypeClass* const pItem) const {
	switch(pItem->WhatAmI()) {
	case AbstractType::BuildingType:
		return this->CountOwnedAndPresent(
			static_cast<BuildingTypeClass const*>(pItem));

	case AbstractType::UnitType:
		return this->CountOwnedAndPresent(
			static_cast<UnitTypeClass const*>(pItem));

	case AbstractType::InfantryType:
		return this->CountOwnedAndPresent(
			static_cast<InfantryTypeClass const*>(pItem));

	case AbstractType::AircraftType:
		return this->CountOwnedAndPresent(
			static_cast<AircraftTypeClass const*>(pItem));

	default:
		__assume(0);
	}
}

int HouseClass::CountOwnedEver(TechnoTypeClass const* const pItem) const {
	switch(pItem->WhatAmI()) {
	case AbstractType::BuildingType:
		return this->CountOwnedEver(
			static_cast<BuildingTypeClass const*>(pItem));

	case AbstractType::UnitType:
		return this->CountOwnedEver(
			static_cast<UnitTypeClass const*>(pItem));

	case AbstractType::InfantryType:
		return this->CountOwnedEver(
			static_cast<InfantryTypeClass const*>(pItem));

	case AbstractType::AircraftType:
		return this->CountOwnedEver(
			static_cast<AircraftTypeClass const*>(pItem));

	default:
		__assume(0);
	}
}

bool HouseClass::CanExpectToBuild(const TechnoTypeClass* const pItem) const {
	auto const parentOwnerMask = this->Type->FindParentCountryIndex();
	return this->CanExpectToBuild(pItem, parentOwnerMask);
}

bool HouseClass::CanExpectToBuild(const TechnoTypeClass* const pItem, int const idxParent) const {
	auto const parentOwnerMask = 1u << idxParent;
	if(pItem->InOwners(parentOwnerMask)) {
		if(this->InRequiredHouses(pItem)) {
			if(!this->InForbiddenHouses(pItem)) {
				auto const BaseSide = pItem->AIBasePlanningSide;
				if(BaseSide == -1 || BaseSide == this->Type->SideIndex) {
					return true;
				}
			}
		}
	}
	return false;
}

int HouseClass::FindSuperWeaponIndex(SuperWeaponType const type) const {
	for(int i = 0; i < this->Supers.Count; ++i) {
		if(this->Supers.Items[i]->Type->Type == type) {
			return i;
		}
	}
	return -1;
}

SuperClass* HouseClass::FindSuperWeapon(SuperWeaponType const type) const {
	auto index = this->FindSuperWeaponIndex(type);
	return this->Supers.GetItemOrDefault(index);
}

bool HouseClass::IsIonCannonEligibleTarget(const TechnoClass* const pTechno) const {
	if(pTechno->InWhichLayer() == Layer::Ground && pTechno->IsAlive && !pTechno->InLimbo) {
		return true;
	}

	// hard difficulty shoots the tank in the factory
	if(this->AIDifficulty == AIDifficulty::Hard) {
		for(const auto* pFactory : *FactoryClass::Array) {
			if(pFactory->Object == pTechno
				&& pFactory->Production.Timer.Duration
				&& !pFactory->IsSuspended)
			{
				return true;
			}
		}
	}

	return false;
}

CellStruct FootClass::GetRandomDirection(FootClass* pFoot)
{
	CellStruct nRet = CellStruct::Empty;

	if (auto pCell = MapClass::Instance->GetCellAt(pFoot->GetCoords()))
	{
		int rnd = ScenarioClass::Instance->Random.RandomRanged(0, 7);
		for (int j = 0; j < 8; ++j)
		{
			// get the direction in an overly verbose way
			int dir = ((j + rnd) % 8) & 7;

			if (auto pNeighbour = pCell->GetNeighbourCell(dir))
			{
				if (pFoot->IsCellOccupied(pNeighbour, -1, -1, nullptr, true) == Move::OK)
				{
					nRet = pNeighbour->MapCoords;
					break;
				}
			}
		}
	}

	return nRet;
}

int TechnoClass::GetIonCannonValue(AIDifficulty const difficulty) const {
	const auto& rules = *RulesClass::Instance;

	const TypeList<int>* pValues = nullptr;
	int value = 1;

	if(auto pUnit = abstract_cast<const UnitClass*>(this)) {
		auto pType = pUnit->Type;

		if(pType->Harvester) {
			pValues = &rules.AIIonCannonHarvesterValue;

		} else if(rules.BuildConst.FindItemIndex(pType->DeploysInto) != -1) {
			pValues = &rules.AIIonCannonMCVValue;

		} else if(pType->Passengers > 0) {
			pValues = &rules.AIIonCannonAPCValue;

		} else {
			value = 2;
		}

	} else if(auto pBuilding = abstract_cast<const BuildingClass*>(this)) {
		auto pType = pBuilding->Type;

		if(pType->Factory == AbstractType::BuildingType) {
			pValues = &rules.AIIonCannonConYardValue;

		} else if(pType->Factory == AbstractType::UnitType && !pType->Naval) {
			pValues = &rules.AIIonCannonWarFactoryValue;

		} else if(pType->PowerBonus > pType->PowerDrain) {
			pValues = &rules.AIIonCannonPowerValue;

		} else if(pType->IsBaseDefense) {
			pValues = &rules.AIIonCannonBaseDefenseValue;

		} else if(pType->IsPlug) {
			pValues = &rules.AIIonCannonPlugValue;

		} else if(pType->IsTemple) {
			pValues = &rules.AIIonCannonTempleValue;

		} else if(pType->HoverPad) {
			pValues = &rules.AIIonCannonHelipadValue;

		} else if(rules.BuildConst.FindItemIndex(pType) != -1) {
			pValues = &rules.AIIonCannonTechCenterValue;

		} else {
			value = 4;
		}

	} else if(auto pInfantry = abstract_cast<const InfantryClass*>(this)) {
		auto pType = pInfantry->Type;

		if(pType->Engineer) {
			pValues = &rules.AIIonCannonEngineerValue;

		} else if(pType->VehicleThief) {
			pValues = &rules.AIIonCannonThiefValue;

		} else {
			value = 2;
		}
	}

	if(pValues) {
		value = pValues->GetItemOrDefault(static_cast<int>(difficulty), value);
	}

	return value;
}

TechnoTypeClass* BuildingClass::GetSecretProduction() const {
	auto const pType = this->Type;

	if(pType->SecretInfantry) {
		return pType->SecretInfantry;
	}
	if(pType->SecretUnit) {
		return pType->SecretUnit;
	}
	if(pType->SecretBuilding) {
		return pType->SecretBuilding;
	}
	return this->SecretProduction;
}

void InfantryClass::RemoveMe_FromGunnerTransport()
{
	if (auto pTransport = this->Transporter)
	{
		if (auto pUnit = specific_cast<UnitClass*>(pTransport))
		{
			if (pUnit->GetTechnoType()->Gunner)
			{
				pUnit->RemoveGunner(this);
			}
		}
	}
}

bool BuildingClass::BuildingUnderAttack()
{
	if (this->Owner)
	{
		this->Owner->BuildingUnderAttack(this);
		return true;
	}

	return false;
}

#pragma warning(push)
#pragma warning(disable : 4244)

const Fixed Fixed::_1_2(1, 2);		// 1/2
const Fixed Fixed::_1_3(1, 3);		// 1/3
const Fixed Fixed::_1_4(1, 4);		// 1/4
const Fixed Fixed::_3_4(3, 4);		// 3/4
const Fixed Fixed::_2_3(2, 3);		// 2/3

Fixed::Fixed(int numerator, int denominator)
{
	if (denominator == 0)
	{
		Data.Raw = 0U;
	}
	else
	{
		Data.Raw = (unsigned int)(((unsigned __int64)numerator * PRECISION) / denominator);
	}
}

Fixed::Fixed(const char* ascii)
{
	if (ascii == nullptr)
	{
		Data.Raw = 0U;
		return;
	}

	char const* wholepart = ascii;

	while (isspace(*ascii))
	{
		ascii++;
	}

	char const* tptr = ascii;
	while (isdigit(*tptr))
	{
		tptr++;
	}

	if (*tptr == '%')
	{
		Data.Raw = (unsigned short)(((unsigned __int64)CRT::atoi(ascii) * PRECISION) / 100ULL);
	}
	else
	{

		Data.Composite.Whole = Data.Composite.Fraction = 0U;
		if (wholepart && *wholepart != '.')
		{
			Data.Composite.Whole = (unsigned char)CRT::atoi(wholepart);
		}

		const char* fracpart = CRT::strchr(ascii, '.');
		if (fracpart) fracpart++;
		if (fracpart)
		{
			unsigned short frac = (unsigned short)CRT::atoi(fracpart);

			int len = 0;
			unsigned int base = 1;
			char const* fptr = fracpart;
			while (isdigit(*fptr))
			{
				fptr++;
				len++;
				base *= 10U;
			}

			Data.Composite.Fraction = (unsigned char)(((unsigned __int64)frac * PRECISION) / base);
		}
	}
}

int Fixed::To_ASCII(char* buffer, int maxlen) const
{
	if (buffer == nullptr) return 0;

	unsigned int whole = Data.Composite.Whole;
	unsigned int frac = ((unsigned int)Data.Composite.Fraction * 1000U) / PRECISION;
	char tbuffer[32];

	if (frac == 0)
	{
		sprintf_s(tbuffer, "%u", whole);
	}
	else
	{
		sprintf_s(tbuffer, "%u.%02u", whole, frac);

		char* ptr = &tbuffer[CRT::strlen(tbuffer) - 1];
		while (*ptr == '0')
		{
			*ptr = '\0';
			ptr--;
		}
	}

	if (maxlen == -1)
	{
		maxlen = CRT::strlen(tbuffer) + 1;
	}

	CRT::strncpy(buffer, tbuffer, maxlen);

	int len = CRT::strlen(tbuffer);
	if (len < maxlen - 1) return(len);
	return(maxlen - 1);
}

const char* Fixed::As_ASCII() const
{
	static char buffer[32];

	To_ASCII(buffer, sizeof(buffer));
	return buffer;
}
#pragma warning(pop)

CoordStruct WWMouseClass::GetCoordsUnderCursor()
{
	CoordStruct nbuffer { -1,-1,-1 };
	Point2D nBuffer2D;
	WWMouseClass::Instance->GetCoords_NotVirtual(nBuffer2D);

	if (nBuffer2D.X >= 0 && nBuffer2D.Y >= 0)
	{
		CellStruct nBufferCell;
		TacticalClass::Instance->Coordmap_viewportpos_tocellpos_Click_Cell_Calc(nBufferCell, nBuffer2D);
		nbuffer.X = nBufferCell.X * 256;
		nbuffer.Y = nBufferCell.Y * 256;
		nbuffer.Z = 0;
	}

	return nbuffer;
}

CellStruct WWMouseClass::GetCellUnderCursor()
{
	CellStruct nbuffer { -1,-1 };
	Point2D nBuffer2D;
	WWMouseClass::Instance->GetCoords_NotVirtual(nBuffer2D);

	if (nBuffer2D.X >= 0 && nBuffer2D.Y >= 0)
		TacticalClass::Instance->Coordmap_viewportpos_tocellpos_Click_Cell_Calc(nbuffer, nBuffer2D);

	return nbuffer;
}

bool LocomotionClass::End_Piggyback(YRComPtr<ILocomotion> &pLoco)
{
	if (!pLoco)
	{
		Game::RaiseError(E_POINTER);
	}

	if (YRComPtr<IPiggyback> pPiggy = pLoco)
	{
		if (pPiggy->Is_Piggybacking())
		{
			// this frees the current locomotor
			pLoco.reset(nullptr);

			// this restores the old one
			auto res = pPiggy->End_Piggyback(pLoco.pointer_to());
			if (FAILED(res))
			{
				Game::RaiseError(res);
			}
			return (res == S_OK);
		}
	}

	return false;
}

void LocomotionClass::ChangeLocomotorTo(FootClass *Object, const CLSID &clsid)
{
	// remember the current one
	YRComPtr<ILocomotion> Original(Object->Locomotor);

	// create a new locomotor and link it
	auto NewLoco = CreateInstance(clsid);
	NewLoco->Link_To_Object(Object);

	// get piggy interface and piggy original
	YRComPtr<IPiggyback> Piggy(NewLoco);
	Piggy->Begin_Piggyback(Original.get());

	// replace the current locomotor
	Object->Locomotor = std::move(NewLoco);
}


void TechnoClass::ReleaseCaptureManager() const
{
	if (auto pManager = this->CaptureManager)
		pManager->FreeAll();
}

void TechnoClass::SuspendWorkSlaveManager() const
{
	if (auto pManager = this->SlaveManager)
		pManager->SuspendWork();
}

void TechnoClass::ResumeWorkSlaveManager() const
{
	if (auto pManager = this->SlaveManager)
		pManager->ResumeWork();
}

void TechnoClass::DetechMyTemporal() const
{
	if (this->IsWarpingSomethingOut())
		if (auto pTemporal = this->TemporalImUsing)
			pTemporal->LetGo();
}

MissionControlClass* TechnoClass::GetMissionControlCurrent() const
{
	return this->GetMissionControl(MissionFlags::CurrentMission);
}

double TechnoClass::GetCurrentMissionRate() const
{
	auto const control = this->GetMissionControl(MissionFlags::CurrentMission);
	auto const doubleval = 900.0; // 0x7E27F8
	return control->Rate * doubleval;
}

int TechnoClass::GetIonCannonValue(AIDifficulty difficulty, int maxHealth) const {
	// what TS does
	if (maxHealth > 0 && this->Health > maxHealth) {
		return (this->WhatAmI() == AbstractType::Building) ? 3 : 1;
	}

	return this->GetIonCannonValue(difficulty);
}

bool PCX::LoadFile(const char *pFileName, int flag1, int flag2)
{
	if (Instance->GetSurface(pFileName, nullptr)) {
		return true;
	}
	return Instance->ForceLoadFile(pFileName, flag1, flag2);
}

void LoadProgressManager::DrawText(const wchar_t *pText, int X, int Y, DWORD dwColor)
{
	if (auto pManager = LoadProgressManager::Instance())
	{
		if (auto pSurface = pManager->ProgressSurface)
		{
			pSurface->DrawText_Old(pText, X, Y, dwColor);
		}
	}
}

bool FootClass::LiberateMember(int idx, byte count)
{
	if (this->BelongsToATeam())
	{
		this->Team->LiberateMember(this, idx, count);
		return true;
	}

	return false;
}

void InfantryClass::UnslaveMe()
{
	if (auto pSlave = this->SlaveOwner)
	{
		if (auto pManager = pSlave->SlaveManager)
		{
			pManager->LostSlave(this);
		}
	}
}

template<class T>
TypeList<T*> CCINIClass::Get_TypeList(const char* section, const char* entry, const TypeList<T*> defvalue, const DynamicVectorClass<T*>& heap)
{
	char buffer[1024];

	if (INIClass::ReadString(section, entry, "", buffer, sizeof(buffer)) > 0)
	{

		TypeList<T> list;

		char* name = CRT::strtok(buffer, ",");
		while (name)
		{

			for (int index = 0; index < heap.Count; ++index)
			{
				T* ptr = const_cast<T*>(T::FindOrAllocate(name));
				if (ptr)
				{
					list.AddItem(ptr);
				}
			}

			name = CRT::strtok(nullptr, ",");
		}

		return list;
	}

	return defvalue;
}

template<class T>
bool CCINIClass::Put_TypeList(const char* section, const char* entry, const TypeList<T*> value)
{
	char buffer[1024] = { '\0' };

	for (int index = 0; index < value.Count; ++index)
	{
		if (buffer[0] != '\0')
		{
			CRT::strcat(buffer, ",");
		}
		CRT::strcat(buffer, value[index]->Name);
	}

	return WriteString(section, entry, buffer);
}

#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4239)
#pragma warning(disable : 4838)
static void Sort_Vertices(Point2D* p1, Point2D* p2, Point2D* p3)
{
	Point2D* temp;
	if (p1->Y > p2->Y)
	{
		temp = p1;
		p1 = p2;
		p2 = temp;
	}
	if (p1->Y > p3->Y)
	{
		temp = p1;
		p1 = p3;
		p3 = temp;
	}
	if (p2->Y > p3->Y)
	{
		temp = p2;
		p2 = p3;
		p3 = temp;
	}
}


static void Fill_Triangle_Top(Surface& surface, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color)
{
	if (point2.X > point3.X)
	{
		Point2D temp = point2;
		point2 = point3;
		point3 = temp;
	}
	float a = (point2.X - point1.X) / (point2.Y - point1.Y);
	float b = (point3.X - point1.X) / (point3.Y - point1.Y);
	float left = point1.X;
	float right = point1.X;
	for (int idy = point1.Y; idy <= point2.Y; ++idy)
	{
		for (int idx = left; idx <= right; ++idx)
		{
			Point2D nBuffer = { idx, idy };
			surface.Put_Pixel(nBuffer, color);
		}
		left += a;
		right += b;
	}
}


static void Fill_Triangle_Bottom(Surface& surface, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color)
{
	if (point1.X > point2.X)
	{
		Point2D temp = point2;
		point2 = point1;
		point1 = temp;
	}
	float a = (point3.X - point1.X) / (point3.Y - point1.Y);
	float b = (point3.X - point2.X) / (point3.Y - point2.Y);
	float left = point3.X;
	float right = point3.X;
	for (int idy = point3.Y; idy > point2.Y; --idy)
	{
		for (int idx = left; idx <= right; ++idx)
		{
			Point2D nBuffer = { idx, idy };
			surface.Put_Pixel(nBuffer, color);
		}
		left -= a;
		right -= b;
	}
}


bool DSurface::Draw_Triangle(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color)
{
	Draw_Line_Rect(rect, point1, point2, color);
	Draw_Line_Rect(rect, point2, point3, color);
	Draw_Line_Rect(rect, point3, point1, color);

	return true;
}


/**
 *
 *
 *  @author: Darth Jane
 */
bool DSurface::Fill_Triangle(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, unsigned color)
{
	if (!rect.Is_Valid())
	{
		return false;
	}

	RectangleStruct r1 = RectangleStruct::Intersect(Get_Rect(), rect, nullptr, nullptr);

	if (!r1.Is_Valid())
	{
		return false;
	}

	Point2D r1_tl = r1.Top_Left();
	unsigned short* buffptr = (unsigned short*)Lock(r1_tl.X, r1_tl.Y);
	if (buffptr == nullptr)
	{
		return false;
	}

	/**
	 *  At first sort the three vertices by y-coordinate ascending so v1 is the topmost vertice.
	 */
	Sort_Vertices(&point1, &point2, &point3);

	/**
	 *  Here we know that point1.Y <= point2.Y <= point3.Y
	 *  check for trivial case of bottom-flat triangle.
	 */
	if (point2.Y == point3.Y)
	{
		Fill_Triangle_Top(*this, point1, point2, point3, color);

		/**
		 *  Check for trivial case of top-flat triangle.
		 */
	}
	else if (point1.Y == point2.Y)
	{
		Fill_Triangle_Bottom(*this, point1, point2, point3, color);

		/**
		 *  General case - split the triangle in a topflat and bottom-flat one.
		 */
	}
	else
	{
		Point2D point4 { (int)(point1.X + ((float)(point2.Y - point1.Y) / (float)(point3.Y - point1.Y)) * (point3.X - point1.X)), point2.Y };
		Fill_Triangle_Top(*this, point1, point2, point4, color);
		Fill_Triangle_Bottom(*this, point2, point4, point3, color);
	}

	return true;
}


bool DSurface::Fill_Triangle_Trans(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, ColorStruct& rgb, unsigned opacity)
{
	// TODO
	return false;
}


bool DSurface::Draw_Quad(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, unsigned color)
{
	Draw_Line_Rect(rect, point1, point2, color);
	Draw_Line_Rect(rect, point2, point3, color);
	Draw_Line_Rect(rect, point3, point4, color);
	Draw_Line_Rect(rect, point4, point1, color);

	return true;
}


bool DSurface::Fill_Quad(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, unsigned color)
{
	Fill_Triangle(rect, point1, point2, point3, color);
	Fill_Triangle(rect, point2, point3, point4, color);

	return true;
}


bool DSurface::Fill_Quad_Trans(RectangleStruct& rect, Point2D& point1, Point2D& point2, Point2D& point3, Point2D& point4, ColorStruct& rgb, unsigned opacity)
{
	// TODO
	return true;
}


/**
 *  Draw a circle.
 *
 *  Uses a modified Bresenham's Circle Drawing algorithm.
 */
void DSurface::Fill_Circle(const Point2D center, unsigned radius, RectangleStruct rect, unsigned color)
{
	Point2D pt { radius, 0 };
	Point2D sxy = Point2D::Empty;
	Point2D dxy = Point2D::Empty;

	/**
	 *  The roundness factor of the circle.
	 *  0 is circle. 50 is rect.
	 */
	int roundness_val = 2;

	/**
	 *  Calculate start decision delta.
	 */
	int d = 3 - (roundness_val * radius);

	do
	{

		dxy = center + Point2D { pt.X, pt.Y };
		sxy = center + Point2D { -pt.X, pt.Y };
		Draw_Line_Rect(rect, sxy, dxy, color);

		dxy = center + Point2D { pt.Y, pt.X };
		sxy = center + Point2D { -pt.Y, pt.X };
		Draw_Line_Rect(rect, sxy, dxy, color);

		dxy = center + Point2D { pt.X, -pt.Y };
		sxy = center + Point2D { -pt.X, -pt.Y };
		Draw_Line_Rect(rect, sxy, dxy, color);

		dxy = center + Point2D { pt.Y, -pt.X };
		sxy = center + Point2D { -pt.Y, -pt.X };
		Draw_Line_Rect(rect, sxy, dxy, color);

		/**
		 *  Check decision and update it, x and y.
		 */
		if (d < 0)
		{

			/**
			 *  Calculate delta for vertical pixel.
			 */
			d += (4 * pt.Y) + 6;

		}
		else
		{

			/**
			 *  Calculate delta for diagonal pixel.
			 */
			d += 4 * (pt.Y - pt.X) + 10;
			pt.X--;
		}

		++pt.Y;

	}
	while (pt.X >= pt.Y);
}


void DSurface::Fill_Circle_Trans(const Point2D center, unsigned radius, RectangleStruct rect, ColorStruct& rgb, unsigned opacity)
{
	Fill_Ellipse_Trans(center, radius, radius, rect, rgb, opacity);
}


void DSurface::Draw_Circle(const Point2D center, unsigned radius, RectangleStruct rect, unsigned color)
{
	Draw_Ellipse(center, radius, radius, rect, color);
}


bool DSurface::Fill_Ellipse(Point2D point, int radius_x, int radius_y, RectangleStruct clip, unsigned color)
{
	// TODO
	return false;
}


bool DSurface::Fill_Ellipse_Trans(Point2D point, int radius_x, int radius_y, RectangleStruct clip, ColorStruct& rgb, unsigned opacity)
{
	// TODO
	return false;
}


bool DSurface::Put_Pixel_Trans(Point2D& point, ColorStruct& rgb, unsigned opacity)
{
	int bpp = Get_Bytes_Per_Pixel();
	if (bpp != 2)
	{
		return false;
	}

	opacity = Math::min((int)opacity, 100);

	unsigned scale = (opacity * 255) / 100;
	unsigned short delta = (255 - scale) & 0xFFFF;

	unsigned int red_max = (unsigned int)(255 >> RedRight) << RedLeft;
	unsigned int green_max = (unsigned int)(255 >> GreenRight) << GreenLeft;
	unsigned int blue_max = (unsigned int)(255 >> BlueRight) << BlueLeft;

	unsigned short color = RGBA_To_Pixel(rgb.R, rgb.G, rgb.B);

	unsigned rscaled = scale * (color & red_max);
	unsigned gscaled = scale * (color & green_max);
	unsigned bscaled = scale * (color & blue_max);

	unsigned short rmax = red_max & 0xFFFF;
	unsigned short gmax = green_max & 0xFFFF;
	unsigned short bmax = blue_max & 0xFFFF;

	unsigned short* current_pixel = (unsigned short*)Lock(point.X, point.Y);
	*current_pixel = (unsigned short)
		(((*current_pixel & rmax) * (delta + rscaled) >> 8) & rmax)
		| (((*current_pixel & gmax) * (delta + gscaled) >> 8) & gmax)
		| (((*current_pixel & bmax) * (delta + bscaled) >> 8) & bmax);

	Unlock();

	return true;
}

#pragma warning(pop)

CellStruct TechnoClass::FindExitCell(TechnoClass* pDocker, CellStruct nDefault) const
{ JMP_THIS(0x70AD50); }

ConvertClass* ConvertClass::CreateFromFile(const char* pal_filename) {
	CCFileClass file;
	file.SetFileName(pal_filename);
	file.Open(FileAccessMode::Read);

	if (!file.Exists()) {
		return nullptr;
	}

	void* data = CCFileClass::Load_Alloc_Data(file);
	if (!data)
	{
		return nullptr;
	}

	BytePalette loaded_pal;
	std::memcpy(&loaded_pal, data, sizeof(BytePalette));

	ConvertClass* drawer = GameCreate<ConvertClass>(loaded_pal, FileSystem::TEMPERAT_PAL(), DSurface::Primary(), 1 , false);
	return drawer;
}

void Game::Unselect_All_Except(AbstractType rtti)
{
	int index = 0;
	while (index < (*ObjectClass::CurrentObjects).Count)
	{

		if ((*ObjectClass::CurrentObjects)[index]->What_Am_I() == rtti)
		{
			++index;
			continue;
		}

		int count_before = (*ObjectClass::CurrentObjects).Count;
		(*ObjectClass::CurrentObjects)[index]->Deselect();

		if (count_before <= (*ObjectClass::CurrentObjects).Count)
		{
			(*ObjectClass::CurrentObjects).Remove((*ObjectClass::CurrentObjects)[index]);
		}
	}
}

void Game::Unselect_All_Except(ObjectTypeClass* objecttype)
{
	int index = 0;
	while (index < (*ObjectClass::CurrentObjects).Count)
	{

		if ((*ObjectClass::CurrentObjects)[index]->GetType() == objecttype)
		{
			++index;
			continue;
		}

		int count_before = (*ObjectClass::CurrentObjects).Count;
		(*ObjectClass::CurrentObjects)[index]->Deselect();

		if (count_before <= (*ObjectClass::CurrentObjects).Count)
		{
			(*ObjectClass::CurrentObjects).Remove((*ObjectClass::CurrentObjects)[index]);
		}
	}
}

void Game::Unselect_All_Except(ObjectClass* object)
{
	int index = 0;
	while (index < (*ObjectClass::CurrentObjects).Count)
	{

		if ((*ObjectClass::CurrentObjects)[index] == object)
		{
			++index;
			continue;
		}

		int count_before = (*ObjectClass::CurrentObjects).Count;
		(*ObjectClass::CurrentObjects)[index]->Deselect();

		if (count_before <= (*ObjectClass::CurrentObjects).Count)
		{
			(*ObjectClass::CurrentObjects).Remove((*ObjectClass::CurrentObjects)[index]);
		}
	}
}

	std::array<const TileTypeData, 21> CellClass::TileArray =
	{ {
		{TileType::Unk, 0x0},
		{TileType::Tunnel, 0x484AB0},
		{TileType::Water, 0x485060},
		{TileType::Blank, 0x486380},
		{TileType::Ramp, 0x4863A0},
		{TileType::Cliff, 0x4863D0},
		{TileType::Shore, 0x4865B0},
		{TileType::Wet, 0x4865D0},
		{TileType::MiscPave, 0x486650},
		{TileType::Pave, 0x486670},
		{TileType::DirtRoad, 0x486690},
		{TileType::PavedRoad, 0x4866D0},
		{TileType::PavedRoadEnd, 0x4866F0},
		{TileType::PavedRoadSlope, 0x486710},
		{TileType::Median, 0x486730},
		{TileType::Bridge, 0x486750},
		{TileType::WoodBridge, 0x486770},
		{TileType::ClearToSandLAT, 0x486790},
		{TileType::Green, 0x4867B0},
		{TileType::NotWater, 0x4867E0},
		{TileType::DestroyableCliff, 0x486900},
	}};

const char* const FileClass::FileErrorToString[] =
 {
		  "Non-error. "
		, "Operation not permitted. "
		, "No such file or directory. "
		, "No such process. "
		, "Interrupted function call. "
		, "Input/output error. "
		, "No such device or address. "
		, "Argument list too long. "
		, "Exec format error. "
		, "Bad file descriptor. "
		, "No child processes. "
		, "Resource temporarily unavailable. "
		, "Not enough space/cannot allocate memory. "
		, "Permission denied. "
		, "Bad address. "
		, "Unknown error 15. "
		, "Device or resource busy. "
		, "File exists. "
		, "Improper link. "
		, "No such device. "
		, "Not a directory. "
		, "Is a directory. "
		, "Invalid argument. "
		, "Too many open files in system. "
		, "Too many open files. "
		, "Unknown error 26. "
		, "Inappropriate I/O control operation. "
		, "File too large. "
		, "No space left on device. "
		, "Invalid seek. "
		, "Read-only filesystem. "
		, "Too many links. "
		, "Broken pipe. "
		, "Mathematics argument out of domain of function. "
		, "Result too large. "
		, "Unknown error 36. "
		, "Resource deadlock avoided. "
		, "Filename too long. "
		, "No locks available. "
		, "Function not implemented. "
		, "Directory not empty. "
		, "Invalid or incomplete multibyte or wide character. "
};
