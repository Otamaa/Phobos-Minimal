#include "ObserverUI.h"

// Engine headers that used to be pulled in through ObserverUI.h. They are needed by the
// bodies below, not by the declarations, so they live here now.
#include <HouseClass.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <TechnoTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>
#include <FactoryClass.h>
#include <TacticalClass.h>
#include <ScenarioClass.h>
#include <Surface.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/TaskForce/Body.h>
#include <Ext/House/Body.h>

#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <Utilities/Constructs.h>
#include <Fundamentals.h>
#include <ColorScheme.h>
#include <BitFont.h>
#include <BitText.h>
#include <WWMouseClass.h>
#include <MouseClass.h>
#include <Drawing.h>
#include <StringTable.h>
#include <RulesClass.h>
#include <New/Entity/ShieldClass.h>
#include <PCX.h>
#include <MissionClass.h>
#include <CommandClass.h>

#include <Misc/PhobosToolTip.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <map>
#include <sstream>
#include <iterator>
#include <unordered_map>
#include <utility>
#include <vector>
#include <windows.h>

ObserverUIClass ObserverUIClass::Instance;

// ---- ObserverUIPalette ----------------------------------------------------------------

void ObserverUIPalette::Rebuild()
{
	this->White = Drawing::RGB2DWORD(255, 255, 255);
	this->TextBright = Drawing::RGB2DWORD(220, 220, 220);
	this->TextLabel = Drawing::RGB2DWORD(200, 200, 200);
	this->TextMuted = Drawing::RGB2DWORD(180, 180, 180);
	this->TextDim = Drawing::RGB2DWORD(160, 160, 160);
	this->BorderLight = Drawing::RGB2DWORD(140, 140, 140);
	this->TextFaint = Drawing::RGB2DWORD(120, 120, 120);
	this->BorderNeutral = Drawing::RGB2DWORD(100, 100, 100);
	this->Disabled = Drawing::RGB2DWORD(90, 90, 90);
	this->BorderIdle = Drawing::RGB2DWORD(80, 80, 80);
	this->BorderPanel = Drawing::RGB2DWORD(60, 60, 60);
	this->Black = Drawing::RGB2DWORD(0, 0, 0);
	this->Good = Drawing::RGB2DWORD(0, 255, 0);
	this->Bad = Drawing::RGB2DWORD(255, 50, 50);
	this->SoftBad = Drawing::RGB2DWORD(255, 90, 90);
	this->Target = Drawing::RGB2DWORD(255, 120, 120);
	this->Danger = Drawing::RGB2DWORD(255, 0, 0);
	this->CloseHover = Drawing::RGB2DWORD(220, 40, 40);
	this->Warning = Drawing::RGB2DWORD(255, 255, 0);
	this->Veteran = Drawing::RGB2DWORD(255, 215, 0);
	this->Highlight = Drawing::RGB2DWORD(0, 255, 255);
	this->Accent = Drawing::RGB2DWORD(100, 220, 255);
	this->Shield = Drawing::RGB2DWORD(100, 200, 255);
	this->Destination = Drawing::RGB2DWORD(180, 220, 255);
}

const ObserverUIPalette& ObserverUIPalette::Get()
{
	static ObserverUIPalette palette;
	static const DSurface* pCachedSurface = nullptr;

	// Drawing::RGB2DWORD depends on the back buffer pixel format, so the table has to be
	// rebuilt whenever the composite surface is recreated (resolution / video mode change).
	const DSurface* const pComposite = DSurface::Composite();

	if (pComposite != pCachedSurface)
	{
		pCachedSurface = pComposite;
		palette.Rebuild();
	}

	return palette;
}

const ObserverUIPalette& UIColors()
{
	return ObserverUIPalette::Get();
}

// =====================================================================================
// Helper bodies - the declarations live in ObserverUI.h
// =====================================================================================

// ---- BitFontStateGuard ----------------------------------------------------------------

BitFontStateGuard::BitFontStateGuard()
	: Bounds(BitFont::Instance->Bounds)
	, Color(BitFont::Instance->Color)
	, Field41(BitFont::Instance->field_41)
{}

BitFontStateGuard::~BitFontStateGuard()
{
	BitFont::Instance->Bounds = this->Bounds;
	BitFont::Instance->Color = this->Color;
	BitFont::Instance->field_41 = this->Field41;
}

void BitFontStateGuard::Clip(const RectangleStruct& rect, bool enableClipping) const
{
	LTRBStruct bounds { rect.X, rect.Y, rect.X + rect.Width, rect.Y + rect.Height };
	BitFont::Instance->field_41 = enableClipping ? 1 : 0;
	BitFont::Instance->SetBounds(&bounds);
}

// ---- ObserverUIHelpers ----------------------------------------------------------------

bool ObserverUIHelpers::IntersectRects(const RectangleStruct& first, const RectangleStruct& second, RectangleStruct& out)
{
	int const left = std::max(first.X, second.X);
	int const top = std::max(first.Y, second.Y);
	int const right = std::min(first.X + first.Width, second.X + second.Width);
	int const bottom = std::min(first.Y + first.Height, second.Y + second.Height);

	if (left < right && top < bottom)
	{
		out = RectangleStruct { left, top, right - left, bottom - top };
		return true;
	}

	return false;
}

bool ObserverUIHelpers::HitTest(const RectangleStruct& rect, const Point2D& point)
{
	return point.X >= rect.X && point.X <= (rect.X + rect.Width)
		&& point.Y >= rect.Y && point.Y <= (rect.Y + rect.Height);
}

bool ObserverUIHelpers::HitTestActive(const RectangleStruct& rect, const Point2D& point)
{
	return rect.Width > 0 && HitTest(rect, point);
}

Point2D ObserverUIHelpers::MousePosition()
{
	if (auto const pMouse = WWMouseClass::Instance())
		return Point2D { pMouse->GetX(), pMouse->GetY() };

	return Point2D { 0, 0 };
}

bool ObserverUIHelpers::IsKeyDown(int virtualKey)
{
	return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

int ObserverUIHelpers::ScreenWidth()
{
	return DSurface::Composite() ? DSurface::Composite->Width : 1024;
}

int ObserverUIHelpers::ScreenHeight()
{
	return DSurface::Composite() ? DSurface::Composite->Height : 768;
}

bool ObserverUIHelpers::IsMultiplayerSession()
{
	auto const mode = SessionClass::Instance->GameMode;
	return mode == GameMode::Skirmish || mode == GameMode::LAN || mode == GameMode::Internet;
}

bool ObserverUIHelpers::IsNonPlayerHouse(HouseClass* pHouse)
{
	return pHouse == HouseExtData::FindNeutral()
		|| pHouse == HouseExtData::FindSpecial()
		|| pHouse == HouseExtData::FindFirstCivilianHouse();
}

const ColorStruct* ObserverUIHelpers::PlayerColorPalette()
{
	// Curated high-contrast palette (Hot Pink is 7th)
	static constexpr ColorStruct palette[PlayerColorCount] {
		{ 0, 102, 255 },   // Player 1 = Blue
		{ 255, 0, 0 },     // Player 2 = Red
		{ 0, 255, 0 },     // Player 3 = Neon Green
		{ 160, 32, 240 },  // Player 4 = Violet
		{ 255, 140, 0 },   // Player 5 = Orange
		{ 255, 255, 0 },   // Player 6 = Yellow
		{ 255, 105, 180 }, // Player 7 = Hot Pink
		{ 0, 255, 255 }    // Player 8 = Cyan
	};

	return palette;
}

ColorStruct ObserverUIHelpers::PaletteColor(size_t index)
{
	return PlayerColorPalette()[index % PlayerColorCount];
}

ColorStruct ObserverUIHelpers::GetHouseColor(HouseClass* pHouse, int fallbackIdx)
{
	if (pHouse && pHouse->ColorSchemeIndex >= 0 && pHouse->ColorSchemeIndex < ColorScheme::Array->Count)
	{
		if (auto const pScheme = ColorScheme::Array->get_or_default(pHouse->ColorSchemeIndex))
		{
			// Convert HSV BaseColor (H, S, V in 0..255) to 8-bit RGB
			return pScheme->BaseColor.operator ColorStruct();
		}
	}

	return PaletteColor(static_cast<size_t>(std::max(0, fallbackIdx)));
}

bool ObserverUIHelpers::IsValidEnemy(HouseClass* pHouse, HouseClass* pCandidate)
{
	return pCandidate
		&& pCandidate != pHouse
		&& !pCandidate->Defeated
		&& !pHouse->IsAlliedWith(pCandidate)
		&& !IsNonPlayerHouse(pCandidate);
}

HouseClass* ObserverUIHelpers::GetTargetEnemy(HouseClass* pHouse)
{
	if (!pHouse)
		return nullptr;

	int const candidates[] { pHouse->WhoLastHurtMe, pHouse->LAEnemy };

	for (int const index : candidates)
	{
		if (index < 0 || index >= HouseClass::Array->Count)
			continue;

		auto const pCandidate = HouseClass::Array->get_or_default(index);
		if (IsValidEnemy(pHouse, pCandidate))
			return pCandidate;
	}

	return nullptr;
}

bool ObserverUIHelpers::HouseHasGrantedSuperWeapon(HouseClass* pHouse)
{
	for (int i = 0; i < pHouse->Supers.Count; ++i)
	{
		auto const pSuper = pHouse->Supers.get_or_default(i);
		if (pSuper && pSuper->Type && pSuper->Granted)
			return true;
	}

	return false;
}

bool ObserverUIHelpers::HouseHasActiveFactory(HouseClass* pHouse)
{
	for (auto const pFactory : *FactoryClass::Array)
	{
		if (pFactory && pFactory->Owner == pHouse && pFactory->Object)
			return true;
	}

	return false;
}

bool ObserverUIHelpers::HouseHasContent(HouseClass* pHouse)
{
	return HasLiveObject(*BuildingClass::Array, pHouse)
		|| HasLiveObject(*InfantryClass::Array, pHouse)
		|| HasLiveObject(*UnitClass::Array, pHouse)
		|| HasLiveObject(*AircraftClass::Array, pHouse)
		|| HouseHasGrantedSuperWeapon(pHouse)
		|| HouseHasActiveFactory(pHouse);
}

bool ObserverUIHelpers::IsHouseHiddenInMultiplayer(HouseClass* pHouse)
{
	return pHouse->Type->MultiplayPassive || IsNonPlayerHouse(pHouse);
}

ObserverBuildingCounts ObserverUIHelpers::CountSupportBuildings(HouseClass* pHouse)
{
	ObserverBuildingCounts counts;

	for (auto const pBuilding : pHouse->Buildings)
	{
		if (!pBuilding || !pBuilding->IsAlive || pBuilding->InLimbo || !pBuilding->Type)
			continue;

		auto const pType = pBuilding->Type;

		if (pType->Factory == AbstractType::InfantryType || pType->GDIBarracks || pType->NODBarracks || pType->YuriBarracks)
			++counts.Barracks;

		if (pType->Helipad || pType->Factory == AbstractType::AircraftType || pType->UnitReload)
			++counts.Helipads;
	}

	return counts;
}

int ObserverUIHelpers::GetKilledUnitCount(HouseClass* pHouse)
{
	return pHouse->TrackedKilledUnitTypes.GetTrackerptr<PhobosUnitTrackerClass>()->GetAll()
		+ pHouse->TrackedKilledInfantryTypes.GetTrackerptr<PhobosUnitTrackerClass>()->GetAll()
		+ pHouse->TrackedKilledAircraftTypes.GetTrackerptr<PhobosUnitTrackerClass>()->GetAll();
}

int ObserverUIHelpers::GetKilledBuildingCount(HouseClass* pHouse)
{
	return pHouse->TrackedKilledBuildingTypes.GetTrackerptr<PhobosUnitTrackerClass>()->GetAll();
}

CoordStruct ObserverUIHelpers::GetPlayerStartCoords(HouseClass* pHouse)
{
	if (!pHouse)
		return CoordStruct::Empty;

	// 1. Prefer the Construction Yard, otherwise remember the first alive building
	BuildingClass* pFallback = nullptr;

	for (auto const pBuilding : *BuildingClass::Array)
	{
		if (!pBuilding || pBuilding->Owner != pHouse || !pBuilding->IsAlive || pBuilding->InLimbo || !pBuilding->Type)
			continue;

		if (pBuilding->Type->ConstructionYard)
			return pBuilding->GetCenterCoords();

		if (!pFallback)
			pFallback = pBuilding;
	}

	if (pFallback)
		return pFallback->GetCenterCoords();

	// 2. Fall back to the AI base anchors
	if (pHouse->BaseSpawnCell != CellStruct::Empty && pHouse->BaseSpawnCell.X > 0)
		return CellClass::Cell2Coord(pHouse->BaseSpawnCell);

	if (pHouse->BaseCenter != CellStruct::Empty && pHouse->BaseCenter.X > 0)
		return CellClass::Cell2Coord(pHouse->BaseCenter);

	return CoordStruct::Empty;
}

// OPTIMIZATION: this used to call TechnoClass::Array->find(), a linear scan over every techno
// in the game, purely to detect a dangling pointer - and it ran for every open card every
// frame. ObserverUIClass::CleanInvalidPointer now purges stale pointers on detach, so a plain
// liveness test is enough.
bool ObserverUIHelpers::IsTechnoValidAndAlive(TechnoClass* pTechno)
{
	return pTechno && pTechno->IsAlive && !pTechno->InLimbo;
}

bool ObserverUIHelpers::IsBuildingValidAndAlive(BuildingClass* pBuilding)
{
	return pBuilding && pBuilding->IsAlive && !pBuilding->InLimbo;
}

int ObserverUIHelpers::GetFactoryProgressPercent(FactoryClass* pFactory)
{
	if (!pFactory || !pFactory->Object)
		return 0;

	int const rate = pFactory->Production.Timer.Rate;
	if (rate > 0)
	{
		int const step = pFactory->Production.Stage; // 0 to 54
		int const timeLeftInStep = pFactory->Production.Timer.GetTimeLeft();
		int const elapsedInStep = (timeLeftInStep >= 0 && timeLeftInStep <= rate) ? (rate - timeLeftInStep) : 0;
		int const totalElapsedFrames = (step * rate) + elapsedInStep;
		int const totalFrames = 54 * rate;
		return std::clamp((totalElapsedFrames * 100) / totalFrames, 0, 100);
	}

	return std::clamp((pFactory->GetProgress() * 100) / 54, 0, 100);
}

BuildingClass* ObserverUIHelpers::FindFactoryBuilding(HouseClass* pHouse, FactoryClass* pFactory)
{
	if (!pHouse || !pFactory)
		return nullptr;

	for (auto const pBuilding : pHouse->Buildings)
	{
		// DIFF: the vanilla-side lookups disagreed on liveness checks; the strictest
		// variant is used everywhere now so a dead factory never anchors a card.
		if (pBuilding && pBuilding->IsAlive && !pBuilding->InLimbo && pBuilding->Factory == pFactory)
			return pBuilding;
	}

	return nullptr;
}

bool ObserverUIHelpers::BuildingGrantsSuperWeapon(BuildingClass* pBuilding, SuperWeaponTypeClass* pSWType)
{
	if (!pBuilding || !pBuilding->Type || !pSWType)
		return false;

	if (pBuilding->Type->SuperWeapon == pSWType->ArrayIndex || pBuilding->Type->SuperWeapon2 == pSWType->ArrayIndex)
		return true;

	if (auto const pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type))
	{
		for (int const swIndex : pExt->SuperWeapons)
		{
			if (swIndex == pSWType->ArrayIndex)
				return true;
		}
	}

	return false;
}

std::vector<BuildingClass*> ObserverUIHelpers::CollectSuperWeaponBuildings(HouseClass* pHouse, SuperWeaponTypeClass* pSWType)
{
	std::vector<BuildingClass*> result;

	if (!pHouse || !pSWType)
		return result;

	for (auto const pBuilding : *BuildingClass::Array)
	{
		if (!pBuilding || pBuilding->Owner != pHouse || !pBuilding->IsAlive || pBuilding->InLimbo)
			continue;

		if (BuildingGrantsSuperWeapon(pBuilding, pSWType))
			result.push_back(pBuilding);
	}

	return result;
}

bool ObserverUIHelpers::IsConsideredVehicle(TechnoClass* pTechno, TechnoTypeClass* pType)
{
	if (!pTechno || !pType || pTechno->WhatAmI() != AbstractType::Building)
		return false;

	auto const pExt = BuildingTypeExtContainer::Instance.Find(static_cast<BuildingTypeClass*>(pType));
	return pExt && pExt->ConsideredVehicle.Get(false);
}

bool ObserverUIHelpers::MatchesFilterCategory(TechnoClass* pTechno, TechnoTypeClass* pType, ObserverFilterCategory category)
{
	AbstractType const absType = pTechno->WhatAmI();
	bool const isConsideredVehicle = IsConsideredVehicle(pTechno, pType);
	bool const isStructure = (absType == AbstractType::Building) && !isConsideredVehicle;

	switch (category)
	{
	case ObserverFilterCategory::Defenses:
		return isStructure && static_cast<BuildingTypeClass*>(pType)->IsBaseDefense;

	case ObserverFilterCategory::Structures:
		return isStructure && !static_cast<BuildingTypeClass*>(pType)->IsBaseDefense;

	case ObserverFilterCategory::AllStructures:
		return isStructure;

	case ObserverFilterCategory::Infantry:
		return absType == AbstractType::Infantry;

	case ObserverFilterCategory::Vehicles:
		return (isConsideredVehicle || absType == AbstractType::Unit) && !pType->Naval && !pType->ConsideredAircraft;

	case ObserverFilterCategory::Naval:
		return pType->Naval;

	case ObserverFilterCategory::Aircraft:
		return absType == AbstractType::Aircraft || pType->ConsideredAircraft;

	case ObserverFilterCategory::AllUnits:
		return isConsideredVehicle || absType != AbstractType::Building;

	case ObserverFilterCategory::Everything:
		return true;

	default:
		// Superweapons (and anything new) are collected separately.
		return false;
	}
}

std::wstring ObserverUIHelpers::ToWide(const char* text)
{
	if (!text)
		return {};

	std::string const narrow(text);
	return std::wstring(narrow.begin(), narrow.end());
}

std::wstring ObserverUIHelpers::ToWide(const std::string& text)
{
	return std::wstring(text.begin(), text.end());
}

const wchar_t* ObserverUIHelpers::PlayerPrefix()
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_PLAYER_PREFIX", L"P");
}

std::wstring ObserverUIHelpers::GetHousePlainName(HouseClass* pHouse)
{
	if (!pHouse)
		return {};

	std::string name = pHouse->PlainName;
	if (name.empty())
		name = pHouse->get_ID();

	return ToWide(name);
}

std::wstring ObserverUIHelpers::GetHouseControlSuffix(HouseClass* pHouse)
{
	if (!pHouse || pHouse->IsControlledByHuman())
		return {};

	switch (pHouse->AIDifficulty)
	{
	case AIDifficulty::Easy: return L" [AI Easy]";
	case AIDifficulty::Normal: return L" [AI Normal]";
	case AIDifficulty::Hard: return L" [AI Hard]";
	default: return L" [AI]";
	}
}

std::wstring ObserverUIHelpers::FormatObjectNameWithDebug(int playerNum, const char* pID, const wchar_t* pUIName, bool isDebugEnabled)
{
	int const effectivePlayerNum = IsMultiplayerSession() ? playerNum : 0;

	std::wstring const wID = ToWide(pID);
	std::wstring const wName = (pUIName && pUIName[0] != L'\0') ? std::wstring(pUIName) : wID;

	std::wostringstream oss;

	if (isDebugEnabled)
	{
		if (effectivePlayerNum > 0)
			oss << PlayerPrefix() << effectivePlayerNum << L" [" << wID << L"]";
		else
			oss << L"[" << wID << L"]";

		if (!wName.empty() && wName != wID)
			oss << L" (" << wName << L")";
	}
	else
	{
		if (effectivePlayerNum > 0)
			oss << L"[" << PlayerPrefix() << effectivePlayerNum << L"] " << wName;
		else
			oss << wName;
	}

	return oss.str();
}

std::wstring ObserverUIHelpers::FormatIDName(AbstractTypeClass* pType)
{
	if (!pType)
		return {};

	std::wstring const wID = ToWide(pType->get_ID());
	std::wstring wName = (pType->UIName && pType->UIName[0] != L'\0') ? std::wstring(pType->UIName) : std::wstring();

	if (wName.empty() && pType->Name[0] != '\0')
		wName = ToWide(pType->Name);

	if (!wName.empty() && wName != wID)
		return wID + L" (" + wName + L")";

	return wID;
}

std::wstring ObserverUIHelpers::GetMissionNameString(Mission mission)
{
	const char* const pName = MissionControlClass::GetMissionName(mission);
	if (pName && pName[0] != '\0')
		return ToWide(pName);

	return L"Idle";
}

void ObserverUIHelpers::FramesToMinutesSeconds(int frames, int& minutes, int& seconds)
{
	int const totalSeconds = (frames + LogicFramesPerSecond - 1) / LogicFramesPerSecond;
	minutes = totalSeconds / 60;
	seconds = totalSeconds % 60;
}

std::wstring ObserverUIHelpers::FormatTimerPair(int framesLeft, int totalFrames)
{
	int minutesLeft = 0;
	int secondsLeft = 0;
	FramesToMinutesSeconds(framesLeft, minutesLeft, secondsLeft);

	int minutesTotal = 0;
	int secondsTotal = 0;
	FramesToMinutesSeconds(totalFrames, minutesTotal, secondsTotal);

	wchar_t buffer[64];
	swprintf_s(buffer, L"%02d:%02d / %02d:%02d", minutesLeft, secondsLeft, minutesTotal, secondsTotal);
	return buffer;
}

std::wstring ObserverUIHelpers::FormatDuration(int frames)
{
	int minutes = 0;
	int seconds = 0;
	FramesToMinutesSeconds(frames, minutes, seconds);

	wchar_t buffer[32];
	swprintf_s(buffer, L"%02d:%02d", minutes, seconds);
	return buffer;
}

double ObserverUIHelpers::CellDistance(const CellStruct& from, const CellStruct& to)
{
	double const dx = static_cast<double>(from.X) - static_cast<double>(to.X);
	double const dy = static_cast<double>(from.Y) - static_cast<double>(to.Y);
	return std::sqrt(dx * dx + dy * dy);
}

std::wstring ObserverUIHelpers::FormatDistance(double distanceInCells)
{
	wchar_t buffer[32];
	swprintf_s(buffer, L"%.1f", distanceInCells);
	return buffer;
}

std::wstring ObserverUIHelpers::FormatTypeCountList(const std::vector<TechnoTypeClass*>& types)
{
	std::vector<TechnoTypeClass*> order;
	std::map<TechnoTypeClass*, int> counts;

	for (auto const pType : types)
	{
		if (!pType)
			continue;

		if (counts[pType] == 0)
			order.push_back(pType);

		++counts[pType];
	}

	std::wostringstream oss;
	bool isFirst = true;

	for (auto const pType : order)
	{
		if (!isFirst)
			oss << L", ";

		isFirst = false;

		std::wstring const wID = ToWide(pType->get_ID());
		std::wstring const wName = (pType->UIName && pType->UIName[0] != L'\0') ? std::wstring(pType->UIName) : wID;

		oss << counts[pType] << L"x [" << wID << L"] " << wName;
	}

	return oss.str();
}

ObserverSearchKey ObserverUIHelpers::PollSearchKey()
{
	static constexpr ObserverPunctuationKey punctuation[] {
		{ VK_SPACE, L' ', L' ' },
		{ 0xDE, L'\'', L'"' }, // Quotes / apostrophe
		{ 0xBD, L'-', L'_' },  // Hyphen / minus
		{ 0xBE, L'.', L'>' },  // Period / dot
		{ 0xBC, L',', L'<' }   // Comma
	};

	ObserverSearchKey result;

	if (IsKeyDown(VK_BACK))
	{
		result.VirtualKey = VK_BACK;
		return result;
	}

	for (auto const& entry : punctuation)
	{
		if (IsKeyDown(entry.VirtualKey))
			return ObserverSearchKey { entry.VirtualKey, entry.Normal, entry.Shifted };
	}

	for (int vk = 'A'; vk <= 'Z'; ++vk)
	{
		if (IsKeyDown(vk))
		{
			return ObserverSearchKey {
				vk,
				static_cast<wchar_t>(L'a' + (vk - 'A')),
				static_cast<wchar_t>(L'A' + (vk - 'A'))
			};
		}
	}

	for (int vk = '0'; vk <= '9'; ++vk)
	{
		if (IsKeyDown(vk))
			return ObserverSearchKey { vk, static_cast<wchar_t>(vk), static_cast<wchar_t>(vk) };
	}

	return result;
}

bool ObserverUIHelpers::ApplySearchKey(std::wstring& text, const ObserverSearchKey& key, bool isShiftHeld)
{
	if (key.VirtualKey == VK_BACK)
	{
		if (text.empty())
			return false;

		text.pop_back();
		return true;
	}

	wchar_t const character = isShiftHeld ? key.Shifted : key.Normal;
	if (character == L'\0')
		return false;

	text += character;
	return true;
}

bool ObserverUIHelpers::IsHotkeyBound(const char* commandName)
{
	for (int idx = 0; idx < CommandClass::Hotkeys->IndexCount; ++idx)
	{
		auto const& entry = CommandClass::Hotkeys->IndexTable[idx];
		if (!entry.Data || !entry.Data->GetName())
			continue;

		if (_stricmp(entry.Data->GetName(), commandName) != 0)
			continue;

		if (entry.ID != 0 && entry.ID != 0xFFFF)
			return true;
	}

	return false;
}

std::vector<ObserverPlayerRow>::const_iterator ObserverUIHelpers::FindRow(const std::vector<ObserverPlayerRow>& rows, HouseClass* pHouse)
{
	return std::find_if(rows.begin(), rows.end(), [pHouse](const ObserverPlayerRow& row)
 {
	 return row.pHouse == pHouse;
	});
}

int ObserverUIHelpers::GetPlayerNumber(const std::vector<ObserverPlayerRow>& rows, HouseClass* pHouse)
{
	auto const it = FindRow(rows, pHouse);
	return (it != rows.end() && it->PlayerNumber > 0) ? it->PlayerNumber : 0;
}

ColorStruct ObserverUIHelpers::GetRowColor(const std::vector<ObserverPlayerRow>& rows, HouseClass* pHouse)
{
	auto const it = FindRow(rows, pHouse);
	return (it != rows.end()) ? it->PlayerColor : ObserverRGB::Muted;
}

DWORD ObserverUIHelpers::GetRowColorValue(const std::vector<ObserverPlayerRow>& rows, HouseClass* pHouse)
{
	auto const it = FindRow(rows, pHouse);
	return (it != rows.end()) ? it->PlayerColorValue : UIColors().TextMuted;
}

Point2D ObserverUIHelpers::CascadePosition(size_t openWindowCount, int cardWidth)
{
	int const cascadeOffset = static_cast<int>(openWindowCount * 24) % 140;
	return Point2D { (ScreenWidth() - cardWidth) / 2 + cascadeOffset, 40 + cascadeOffset };
}

void ObserverUIHelpers::DeselectAll()
{
	while (ObjectClass::CurrentObjects->Count > 0)
	{
		ObjectClass::CurrentObjects->get_or_default(0)->Deselect();
	}
}

void ObserverUIHelpers::CenterOnCoords(const CoordStruct& coords)
{
	if (!TacticalClass::Instance() || coords == CoordStruct::Empty)
		return;

	CoordStruct target = coords;
	TacticalClass::Instance->SetTacticalPosition(&target);
	MapClass::Instance->Redraws = TRUE;
}

void ObserverUIHelpers::CenterAndSelect(TechnoClass* pTarget)
{
	if (!pTarget || !TacticalClass::Instance())
		return;

	CoordStruct coords = pTarget->GetCenterCoords();
	TacticalClass::Instance->SetTacticalPosition(&coords);
	pTarget->Select();
	MapClass::Instance->Redraws = TRUE;
}

BSurface* ObserverUIHelpers::ValidSurface(BSurface* pSurface)
{
	if (pSurface && (pSurface->Get_Width() <= 0 || pSurface->Get_Height() <= 0))
		return nullptr;

	return pSurface;
}

BSurface* ObserverUIHelpers::LoadPCXSurface(const char* fileName)
{
	if (!fileName || fileName[0] == '\0')
		return nullptr;

	PhobosPCXFile exact(fileName);
	if (exact.Exists())
		return exact.GetSurface();

	char buffer[64];
	sprintf_s(buffer, "%s.pcx", fileName);
	_strlwr_s(buffer);

	PhobosPCXFile lowered(buffer);
	return lowered.Exists() ? lowered.GetSurface() : nullptr;
}

BSurface* ObserverUIHelpers::LoadPCXSurfaceForID(const char* id)
{
	if (!id || id[0] == '\0')
		return nullptr;

	char buffer[64];
	sprintf_s(buffer, "%sicon.pcx", id);
	_strlwr_s(buffer);

	PhobosPCXFile iconFile(buffer);
	if (iconFile.Exists())
		return iconFile.GetSurface();

	sprintf_s(buffer, "%s.pcx", id);
	_strlwr_s(buffer);

	PhobosPCXFile plainFile(buffer);
	return plainFile.Exists() ? plainFile.GetSurface() : nullptr;
}

SHPCaches* ObserverUIHelpers::LoadSHPFile(const char* fileName)
{
	if (!fileName || fileName[0] == '\0')
		return nullptr;

	if (auto const pFile = FileSystem::LoadSHPFile(fileName))
		return pFile;

	char buffer[64];
	sprintf_s(buffer, "%s.shp", fileName);
	_strlwr_s(buffer);

	return FileSystem::LoadSHPFile(buffer);
}

SHPCaches* ObserverUIHelpers::LoadPlaceholderCameo()
{
	static const char* const candidates[] { "XXICON.SHP", "xxicon.shp", "XXICON", "xxicon" };

	for (auto const pName : candidates)
	{
		if (auto const pFile = FileSystem::LoadSHPFile(pName))
			return pFile;
	}

	return nullptr;
}

bool ObserverUIHelpers::IsPlaceholderCameo(SHPCaches* pCameo)
{
	if (!pCameo || !pCameo->IsReference() || !pCameo->Filename)
		return false;

	const char* const fileName = pCameo->Filename;
	return _stricmp(fileName, "XXICON.SHP") == 0 || _stricmp(fileName, "XXICON") == 0;
}

void ObserverUIHelpers::DrawPanel(DSurface* pSurface, RectangleStruct rect, ColorStruct fillColor, int opacity, COLORREF borderColor)
{
	if (!pSurface)
		return;

	pSurface->Fill_Rect_Trans(&rect, &fillColor, opacity);
	pSurface->Draw_Rect(rect, borderColor);
}

void ObserverUIHelpers::DrawCenteredText(DSurface* pSurface, RectangleStruct rect, const wchar_t* text, COLORREF color)
{
	if (!pSurface || !text || !BitFont::Instance() || !BitText::Instance())
		return;

	int textWidth = 0;
	int textHeight = 0;
	BitFont::Instance->GetTextDimension(text, &textWidth, &textHeight, rect.Width);

	BitFontStateGuard const guard;
	guard.Clip(rect);
	BitFont::Instance->Color = static_cast<WORD>(color);

	BitText::Instance->DrawText(
		BitFont::Instance,
		pSurface,
		text,
		rect.X + (rect.Width - textWidth) / 2,
		rect.Y + (rect.Height - textHeight) / 2,
		textWidth,
		textHeight,
		0, 0, 0
	);
}

void ObserverUIHelpers::DrawShadowText(DSurface* pSurface, RectangleStruct clipRect, const wchar_t* text, Point2D position, COLORREF color)
{
	if (!pSurface || !text || !BitFont::Instance())
		return;

	BitFontStateGuard const guard;
	guard.Clip(clipRect);

	pSurface->DSurfaceDrawText(text, DSurface::ViewBounds.operator->(), &position, color, 0, TextPrintType::FullShadow | TextPrintType::Point8);
}

void ObserverUIHelpers::DrawCloseButton(DSurface* pSurface, RectangleStruct rect, bool isHovered)
{
	if (!pSurface || !BitFont::Instance())
		return;

	pSurface->Fill_Rect(rect, isHovered ? UIColors().CloseHover : UIColors().BorderPanel);
	pSurface->Draw_Rect(rect, UIColors().BorderLight);

	int textWidth = 0;
	int textHeight = 0;
	BitFont::Instance->GetTextDimension(L"X", &textWidth, &textHeight, rect.Width);

	Point2D position { rect.X + (rect.Width - textWidth) / 2, rect.Y + (rect.Height - textHeight) / 2 };
	pSurface->DSurfaceDrawText(L"X", DSurface::ViewBounds.operator->(), &position, UIColors().White, 0, TextPrintType::Point8);
}

void ObserverUIHelpers::DrawCameoOverlayText(DSurface* pSurface, const RectangleStruct& displayRect, const RectangleStruct& clipRect, const std::wstring& text, COLORREF color)
{
	if (!pSurface || text.empty() || !BitFont::Instance() || !BitText::Instance())
		return;

	int textWidth = 0;
	int textHeight = 0;
	BitFont::Instance->GetTextDimension(text.c_str(), &textWidth, &textHeight, displayRect.Width);

	Point2D const textPoint {
		displayRect.X + (displayRect.Width - textWidth) / 2,
		displayRect.Y + (displayRect.Height - textHeight) / 2
	};

	// Translucent 30% opacity dark background behind text for readability (opacity = 75)
	RectangleStruct const backgroundRect { textPoint.X - 3, textPoint.Y - 1, textWidth + 6, textHeight + 2 };
	RectangleStruct clippedBackground;
	if (IntersectRects(backgroundRect, clipRect, clippedBackground))
	{
		ColorStruct backgroundColor { 0, 0, 0 };
		pSurface->Fill_Rect_Trans(&clippedBackground, &backgroundColor, 75);
	}

	BitFontStateGuard const guard;
	guard.Clip(clipRect);
	BitFont::Instance->Color = static_cast<WORD>(color);

	BitText::Instance->DrawText(
		BitFont::Instance,
		pSurface,
		text.c_str(),
		textPoint.X,
		textPoint.Y,
		textWidth,
		textHeight,
		0, 0, 0
	);
}

void ObserverUIHelpers::DrawSimpleTooltip(DSurface* pSurface, const wchar_t* text, const Point2D& mousePos)
{
	if (!pSurface || !text || !BitFont::Instance())
		return;

	int textWidth = 0;
	int textHeight = 0;
	BitFont::Instance->GetTextDimension(text, &textWidth, &textHeight, 300);

	int const tipX = std::min(mousePos.X + 12, pSurface->Width - textWidth - 16);
	int const tipY = std::max(10, mousePos.Y - textHeight - 12);

	DrawPanel(pSurface, RectangleStruct { tipX - 4, tipY - 4, textWidth + 8, textHeight + 8 },
		ObserverRGB::Black, 200, UIColors().BorderLight);

	Point2D tipPoint { tipX, tipY };
	pSurface->DSurfaceDrawText(text, DSurface::ViewBounds.operator->(), &tipPoint, UIColors().White, 0, TextPrintType::Point8);
}

bool ObserverUIHelpers::DrawCameoImage(
	DSurface* pSurface,
	RectangleStruct destinationRect,
	BSurface* pPCXSurface,
	SHPCaches* pFileSHP,
	ConvertClass* pPalette,
	int frameIndex,
	int zAdjust)
{
	if (!pSurface || (!pPCXSurface && !pFileSHP))
		return false;

	// Prioritize drawing the PCX file if it's provided and valid
	if (ValidSurface(pPCXSurface))
	{
		PCXImages::Instance->BlitToSurface(&destinationRect, pSurface, pPCXSurface);
		return true;
	}

	if (pFileSHP)
	{
		ConvertClass* pUsedPalette = pPalette ? pPalette : FileSystem::CAMEO_PAL;
		if (!pUsedPalette)
			pUsedPalette = FileSystem::UNITx_PAL;

		Point2D location { destinationRect.X, destinationRect.Y };
		pSurface->DrawSHP(pUsedPalette, pFileSHP, frameIndex, &location, DSurface::ViewBounds.operator->(),
			BlitterFlags::bf_400, 0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		return true;
	}

	return false;
}

// ---- ObserverTextBlock ----------------------------------------------------------------

ObserverTextBlock::ObserverTextBlock(int maxWidth)
	: MaxWidth(maxWidth)
{}

void ObserverTextBlock::AddSegments(std::vector<ObserverTextSegment> segments)
{
	if (segments.empty() || !BitFont::Instance())
		return;

	int lineWidth = 0;
	int lineHeight = 0;

	for (auto& segment : segments)
	{
		BitFont::Instance->GetTextDimension(segment.Text.c_str(), &segment.Width, &segment.Height, this->MaxWidth);
		lineWidth += segment.Width;
		lineHeight = std::max(lineHeight, segment.Height);
	}

	this->Width = std::max(this->Width, lineWidth);
	this->Height += lineHeight + LineGap;
	this->Lines.push_back(ObserverTextLine { std::move(segments), lineWidth, lineHeight });
}

void ObserverTextBlock::Add(const std::wstring& text, DWORD color)
{
	if (text.empty())
		return;

	this->AddSegments({ ObserverTextSegment { text, color } });
}

void ObserverTextBlock::Add(const std::wstring& label, DWORD labelColor, const std::wstring& value, DWORD valueColor)
{
	this->AddSegments({
		ObserverTextSegment { label, labelColor },
		ObserverTextSegment { value, valueColor }
	});
}

void ObserverTextBlock::Render(DSurface* pSurface, const RectangleStruct& clipRect, int left, int top) const
{
	if (!pSurface || !BitFont::Instance() || !BitText::Instance())
		return;

	BitFontStateGuard const guard;
	guard.Clip(clipRect);

	int currentY = top;

	for (auto const& line : this->Lines)
	{
		int currentX = left;

		for (auto const& segment : line.Segments)
		{
			// OPTIMIZATION: width/height were measured in AddSegments, no re-measure here.
			BitFont::Instance->Color = static_cast<WORD>(segment.Color);
			BitText::Instance->DrawText(
				BitFont::Instance,
				pSurface,
				segment.Text.c_str(),
				currentX,
				currentY,
				segment.Width,
				line.Height,
				0, 0, 0
			);

			currentX += segment.Width;
		}

		currentY += line.Height + LineGap;
	}
}

// ---- ObserverTooltipBox ----------------------------------------------------------------

RectangleStruct ObserverTooltipBox::Compute(const ObserverTextBlock& block, const Point2D& mousePos, int padding)
{
	int const boxWidth = block.Width + padding * 2;
	int const boxHeight = block.Height + padding * 2;

	int boxX = mousePos.X + 15;
	int boxY = mousePos.Y + 15;

	int const maxX = DSurface::ViewBounds->Width;
	int const maxY = DSurface::ViewBounds->Height;

	if (boxX + boxWidth > maxX)
		boxX = mousePos.X - boxWidth - 5;

	if (boxY + boxHeight > maxY)
		boxY = maxY - boxHeight - 5;

	return RectangleStruct { std::max(0, boxX), std::max(0, boxY), boxWidth, boxHeight };
}

void ObserverTooltipBox::Render(DSurface* pSurface, const ObserverTextBlock& block, const Point2D& mousePos, ColorStruct borderColor, int padding)
{
	if (!pSurface)
		return;

	RectangleStruct const boxRect = Compute(block, mousePos, padding);

	ObserverUIHelpers::DrawPanel(pSurface, boxRect, ObserverRGB::Black, 75,
		Drawing::RGB2DWORD(borderColor.R, borderColor.G, borderColor.B));

	block.Render(pSurface, boxRect, boxRect.X + padding, boxRect.Y + padding);
}

// ---- ObserverHouseSummary ----------------------------------------------------------------

DWORD ObserverHouseSummary::White()
{ return UIColors().White; }

DWORD ObserverHouseSummary::Label()
{ return UIColors().TextLabel; }

DWORD ObserverHouseSummary::Muted()
{ return UIColors().TextMuted; }

DWORD ObserverHouseSummary::Good()
{ return UIColors().Good; }

DWORD ObserverHouseSummary::Bad()
{ return UIColors().Bad; }

DWORD ObserverHouseSummary::SoftBad()
{ return UIColors().SoftBad; }

DWORD ObserverHouseSummary::Accent()
{ return UIColors().Accent; }

bool ObserverHouseSummary::UsePlayerNumber(int playerNumber, bool isMultiplayer, const ObserverHouseSummaryOptions& options)
{
	return playerNumber > 0 && (!options.PlayerNumberNeedsMultiplayer || isMultiplayer);
}

std::wstring ObserverHouseSummary::FormatHouseName(HouseClass* pHouse, int playerNumber, bool isMultiplayer, const ObserverHouseSummaryOptions& options, bool bracketPlayerNumber)
{
	std::wostringstream oss;

	std::wstring const houseID = ObserverUIHelpers::ToWide(pHouse->get_ID());

	if (UsePlayerNumber(playerNumber, isMultiplayer, options))
	{
		if (options.DebugFormatting)
			oss << ObserverUIHelpers::PlayerPrefix() << playerNumber << L" [" << houseID << L"] ";
		else if (bracketPlayerNumber)
			oss << L"[" << ObserverUIHelpers::PlayerPrefix() << playerNumber << L"] ";
		else
			oss << ObserverUIHelpers::PlayerPrefix() << playerNumber << L" ";
	}
	else if (options.DebugFormatting)
	{
		oss << L"[" << houseID << L"] ";
	}

	oss << ObserverUIHelpers::GetHousePlainName(pHouse);

	if (pHouse->Type)
		oss << L" (" << pHouse->Type->UIName << L")";

	return oss.str();
}

void ObserverHouseSummary::AddEconomyLines(ObserverTextBlock& block, HouseClass* pHouse, const std::vector<ObserverPlayerRow>& rows)
{
	std::wostringstream moneyOss;
	moneyOss << L"Credits: $" << pHouse->Available_Money();
	block.Add(moneyOss.str(), Label());

	auto const itRow = ObserverUIHelpers::FindRow(rows, pHouse);
	if (itRow == rows.end())
		return;

	std::wostringstream rateOss;
	DWORD rateColor = Muted();

	if (itRow->IncomeRatePerMin > 0)
	{
		rateOss << L"+$" << itRow->IncomeRatePerMin;
		rateColor = Good();
	}
	else if (itRow->IncomeRatePerMin < 0)
	{
		rateOss << L"-$" << std::abs(itRow->IncomeRatePerMin);
		rateColor = SoftBad();
	}
	else
	{
		rateOss << L"+$0";
	}

	block.Add(L"Economy/min: ", Label(), rateOss.str(), rateColor);
}

void ObserverHouseSummary::AddPowerLine(ObserverTextBlock& block, HouseClass* pHouse)
{
	int const powerOutput = pHouse->PowerOutput;
	int const powerDrain = pHouse->PowerDrain;
	int const balance = powerOutput - powerDrain;

	std::wostringstream powerOss;
	powerOss << powerDrain << L" / " << powerOutput << L" (";

	std::wostringstream balanceOss;
	DWORD balanceColor = Muted();

	if (balance > 0)
	{
		balanceOss << L"+" << balance;
		balanceColor = Good();
	}
	else if (balance < 0)
	{
		balanceOss << balance;
		balanceColor = Bad();
	}
	else
	{
		balanceOss << L"+0";
	}

	block.AddSegments({
		ObserverTextSegment { L"Power: ", Label() },
		ObserverTextSegment { powerOss.str(), Label() },
		ObserverTextSegment { balanceOss.str(), balanceColor },
		ObserverTextSegment { L")", Label() }
	});
}

void ObserverHouseSummary::AddDebugLines(ObserverTextBlock& block, HouseClass* pHouse)
{
	if (!Phobos::Config::DevelopmentCommands)
		return;

	bool const isAI = !pHouse->IsControlledByHuman();

	if (isAI)
		block.Add(L"AI's IQ Level: " + std::to_wstring(pHouse->IQLevel2), Label());

	block.Add(L"Tech Level: " + std::to_wstring(pHouse->StaticData.TechLevel), Label());

	if (isAI && !pHouse->Production)
		block.Add(L"AI Production: ", Label(), L"Disabled", SoftBad());

	if (isAI && !pHouse->AITriggersActive)
		block.Add(L"AI Triggers: ", Label(), L"Disabled", SoftBad());

	if (isAI && !pHouse->AutoBaseBuilding)
		block.Add(L"Auto Base Building: ", Label(), L"Disabled", SoftBad());

	if (!isAI)
		return;

	int activeAITeams = 0;
	for (int k = 0; k < TeamClass::Array->Count; ++k)
	{
		auto const pTeam = TeamClass::Array->get_or_default(k);
		if (pTeam && pTeam->OwnerHouse == pHouse)
			++activeAITeams;
	}

	if (activeAITeams > 0)
		block.Add(L"Active AI Teams: " + std::to_wstring(activeAITeams), Label());
}

void ObserverHouseSummary::AddObjectCountLines(ObserverTextBlock& block, HouseClass* pHouse, const ObserverHouseSummaryOptions& options)
{
	if (!options.HideZeroFactoryCounts || pHouse->CountResourceDestinations > 0)
		block.Add(L"Refineries: " + std::to_wstring(pHouse->CountResourceDestinations), Label());

	if (!options.HideZeroFactoryCounts || pHouse->CountWarfactories > 0)
		block.Add(L"War Factories: " + std::to_wstring(pHouse->CountWarfactories), Label());

	auto const counts = ObserverUIHelpers::CountSupportBuildings(pHouse);

	if (counts.Barracks > 0)
		block.Add(L"Barracks: " + std::to_wstring(counts.Barracks), Label());

	if (counts.Helipads > 0 || pHouse->AirportDocks > 0)
	{
		std::wstring line = L"Helipads: " + std::to_wstring(counts.Helipads);
		if (pHouse->AirportDocks > 0)
			line += L" (Docks: " + std::to_wstring(pHouse->AirportDocks) + L")";

		block.Add(line, Label());
	}

	struct OwnedEntry { const wchar_t* Label; int Count; };
	OwnedEntry const owned[] {
		{ L"Total Buildings: ", pHouse->OwnedBuildings },
		{ L"Total Infantry: ", pHouse->OwnedInfantry },
		{ L"Total Units: ", pHouse->OwnedUnits },
		{ L"Total Aircraft: ", pHouse->OwnedAircraft },
		{ L"Total Navy: ", pHouse->OwnedNavy }
	};

	for (auto const& entry : owned)
	{
		if (entry.Count > 0)
			block.Add(entry.Label + std::to_wstring(entry.Count), Label());
	}

	int const killedUnits = ObserverUIHelpers::GetKilledUnitCount(pHouse);
	if (killedUnits > 0)
		block.Add(L"Killed Units: " + std::to_wstring(killedUnits), Label());

	int const killedBuildings = ObserverUIHelpers::GetKilledBuildingCount(pHouse);
	if (killedBuildings > 0)
		block.Add(L"Killed Buildings: " + std::to_wstring(killedBuildings), Label());
}

void ObserverHouseSummary::AddAlliesLine(ObserverTextBlock& block, HouseClass* pHouse, const std::vector<ObserverPlayerRow>& rows, bool isMultiplayer, const ObserverHouseSummaryOptions& options)
{
	std::wstring allies;

	for (auto const& row : rows)
	{
		if (!row.pHouse || row.pHouse == pHouse || !pHouse->IsAlliedWith(row.pHouse))
			continue;

		if (!allies.empty())
			allies += L", ";

		std::wstring const allyID = ObserverUIHelpers::ToWide(row.pHouse->get_ID());

		if (UsePlayerNumber(row.PlayerNumber, isMultiplayer, options))
		{
			allies += ObserverUIHelpers::PlayerPrefix() + std::to_wstring(row.PlayerNumber);
			if (options.DebugFormatting)
				allies += L" [" + allyID + L"]";
		}
		else
		{
			// Singleplayer / Campaign mode: use House Name (and [ID] in front if debug mode)
			if (options.DebugFormatting)
				allies += L"[" + allyID + L"] ";

			allies += ObserverUIHelpers::GetHousePlainName(row.pHouse);
		}
	}

	if (!allies.empty())
		block.Add(L"Allies: ", Label(), allies, Accent());
}

void ObserverHouseSummary::AddTargetEnemyLine(ObserverTextBlock& block, HouseClass* pHouse, const std::vector<ObserverPlayerRow>& rows, bool isMultiplayer, const ObserverHouseSummaryOptions& options)
{
	if (pHouse->IsControlledByHuman())
		return;

	auto const pEnemy = ObserverUIHelpers::GetTargetEnemy(pHouse);
	if (!pEnemy || !pEnemy->Type)
	{
		block.Add(L"Target Enemy: ", Label(), L"None", Muted());
		return;
	}

	ColorStruct const enemyColor = ObserverUIHelpers::GetHouseColor(pEnemy);
	int const enemyNumber = ObserverUIHelpers::GetPlayerNumber(rows, pEnemy);

	std::wstring const enemyName = FormatHouseName(pEnemy, enemyNumber, isMultiplayer, options, true);

	block.Add(L"Target Enemy: ", Label(), enemyName, Drawing::RGB2DWORD(enemyColor.R, enemyColor.G, enemyColor.B));
}

void ObserverHouseSummary::Build(ObserverTextBlock& block, HouseClass* pHouse, const std::vector<ObserverPlayerRow>& rows, const ObserverHouseSummaryOptions& options)
{
	if (!pHouse || !pHouse->Type)
		return;

	bool const isMultiplayer = ObserverUIHelpers::IsMultiplayerSession();
	int const playerNumber = ObserverUIHelpers::GetPlayerNumber(rows, pHouse);

	// Title Line: Player Number, Player Name & Country Name
	block.Add(FormatHouseName(pHouse, playerNumber, isMultiplayer, options, true)
		+ ObserverUIHelpers::GetHouseControlSuffix(pHouse), White());

	AddEconomyLines(block, pHouse, rows);
	AddPowerLine(block, pHouse);
	AddDebugLines(block, pHouse);

	if (options.ShowDefeatedStatus && pHouse->Defeated)
		block.Add(L"Status: ", Label(), L"Defeated", Bad());

	AddObjectCountLines(block, pHouse, options);
	AddAlliesLine(block, pHouse, rows, isMultiplayer, options);
	AddTargetEnemyLine(block, pHouse, rows, isMultiplayer, options);
}

// ---- ObserverUnitCard ----------------------------------------------------------------

const wchar_t* ObserverUnitCard::Text(const char* label, const wchar_t* fallback)
{
	return GeneralUtils::LoadStringUnlessMissing(label, fallback);
}

void ObserverUnitCard::AddOwnerLine(ObserverTextBlock& block, HouseClass* pOwner)
{
	if (!pOwner || !pOwner->Type || ObserverUIHelpers::IsMultiplayerSession())
		return;

	block.Add(L"Owner: " + ObserverUIHelpers::GetHousePlainName(pOwner) + L" (" + pOwner->Type->UIName + L")", Colors::Label());
}

void ObserverUnitCard::AddCoordsLine(ObserverTextBlock& block, const CellStruct& cell, const wchar_t* label, DWORD color)
{
	std::wostringstream oss;
	oss << label << L"(" << cell.X << L", " << cell.Y << L")";
	block.Add(oss.str(), color);
}

void ObserverUnitCard::AddDistanceLine(ObserverTextBlock& block, const wchar_t* label, const CellStruct& from, const CellStruct& to, DWORD color)
{
	std::wostringstream oss;
	oss << label << L"(" << to.X << L", " << to.Y << L")   "
		<< Text("TXT_OBSERVER_CARD_DISTANCE", L"Distance: ")
		<< ObserverUIHelpers::FormatDistance(ObserverUIHelpers::CellDistance(from, to))
		<< Text("TXT_OBSERVER_CARD_CELLS", L" cells");

	block.Add(oss.str(), color);
}

void ObserverUnitCard::AddSuperWeaponLines(ObserverTextBlock& block, ObserverFloatingUnitWindow& win, int playerNumber, bool isDebug)
{
	auto const pSWType = win.pSuperType;
	auto const pSuper = win.pSuper;
	auto const pOwner = win.pOwner;

	// Title Line: Superweapon Name
	block.Add(ObserverUIHelpers::FormatObjectNameWithDebug(
		playerNumber,
		pSWType ? pSWType->get_ID() : "",
		pSWType ? pSWType->UIName : nullptr,
		isDebug), Colors::White());

	AddOwnerLine(block, pOwner);

	// Cooldown Line: Cooldown: 01:45 / 05:00
	int const totalFrames = pSuper ? pSuper->GetRechargeTime() : (pSWType ? pSWType->RechargeTime : 0);
	int const framesLeft = pSuper ? pSuper->RechargeTimer.GetTimeLeft() : 0;

	block.Add(L"Cooldown: " + ObserverUIHelpers::FormatTimerPair(framesLeft, totalFrames),
		(framesLeft == 0) ? Colors::Good() : Colors::Accent());

	// Power Line (ONLY shown when superweapon requires power AND owner is in low power state)
	if (pSWType && pSWType->IsPowered && pOwner && pOwner->PowerOutput < pOwner->PowerDrain)
		block.Add(L"Power: Low Power", Colors::Bad());

	// Coords Line
	BuildingClass* pSWBuilding = ObserverUIHelpers::IsBuildingValidAndAlive(win.pTargetBuilding) ? win.pTargetBuilding : nullptr;
	if (!pSWBuilding)
	{
		auto const buildings = ObserverUIHelpers::CollectSuperWeaponBuildings(pOwner, pSWType);
		if (!buildings.empty())
			pSWBuilding = buildings.front();
	}

	if (pSWBuilding)
		AddCoordsLine(block, CellClass::Coord2Cell(pSWBuilding->GetCenterCoords()), L"Coords: ", Colors::Label());
}

void ObserverUnitCard::UpdateSnapshot(ObserverUnitCardState& state)
{
	auto& win = *state.Window;

	// Live snapshot tracking while object is alive
	if (state.Techno)
	{
		win.IsDestroyed = false;
		win.LastHP = state.Techno->Health;
		win.LastMaxHP = state.BaseType ? state.BaseType->Strength : state.Techno->Health;
		win.LastCoords = CellClass::Coord2Cell(state.Techno->GetCenterCoords());
		win.LastMission = ObserverUIHelpers::GetMissionNameString(state.Techno->GetCurrentMission());
		win.LastVeterancy = state.Techno->Veterancy.Veterancy;
	}
	else if (state.Building)
	{
		win.IsDestroyed = false;
		win.LastHP = state.Building->Health;
		win.LastMaxHP = state.Building->Type ? state.Building->Type->Strength : (state.BaseType ? state.BaseType->Strength : state.Building->Health);
		win.LastCoords = CellClass::Coord2Cell(state.Building->GetCenterCoords());
		win.LastMission = L"";
		win.LastVeterancy = state.Building->Veterancy.Veterancy;
	}
	else if (win.pTargetTechno || win.pTargetBuilding)
	{
		win.IsDestroyed = true;
	}
}

BuildingTypeClass* ObserverUnitCard::FindFactoryBuildingType(const ObserverUnitCardState& state)
{
	if (state.Building && state.Building->Type)
		return state.Building->Type;

	if (state.Factory && state.Factory->Object && state.Factory->Object->GetTechnoType())
		return type_cast<BuildingTypeClass*>(state.Factory->Object->GetTechnoType());

	if (auto const pBuilding = ObserverUIHelpers::FindFactoryBuilding(state.Owner, state.Factory))
		return pBuilding->Type;

	return nullptr;
}

void ObserverUnitCard::AddTitleLine(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	std::wstring title;

	if (state.IsProductionView)
	{
		// First Line of Production Card = Factory Name!
		auto const pFactoryType = FindFactoryBuildingType(state);
		auto const pFallback = state.TitleType;

		title = ObserverUIHelpers::FormatObjectNameWithDebug(
			state.PlayerNumber,
			pFactoryType ? pFactoryType->get_ID() : (pFallback ? pFallback->get_ID() : ""),
			pFactoryType ? pFactoryType->UIName : (pFallback ? pFallback->UIName : nullptr),
			state.IsDebug);
	}
	else
	{
		title = ObserverUIHelpers::FormatObjectNameWithDebug(
			state.PlayerNumber,
			state.TitleType ? state.TitleType->get_ID() : "",
			state.TitleType ? state.TitleType->UIName : nullptr,
			state.IsDebug);

		if (state.Window->InstanceNumber > 1 && !state.Factory)
			title += L" #" + std::to_wstring(state.Window->InstanceNumber);
	}

	block.Add(title, Colors::White());
}

void ObserverUnitCard::AddProductionLines(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	bool const isProducing = state.CurrentProduct && state.Factory && state.Factory->Object;

	if (isProducing)
	{
		int const percent = ObserverUIHelpers::GetFactoryProgressPercent(state.Factory);
		std::wstring const productName = ObserverUIHelpers::FormatObjectNameWithDebug(
			0, state.CurrentProduct->get_ID(), state.CurrentProduct->UIName, state.IsDebug);

		std::wostringstream oss;

		// Production card: bare product name. Building card on map: "Production: <name>".
		if (!state.IsProductionView)
			oss << L"Production: ";

		oss << productName << L" (" << percent << L"%)";
		block.Add(oss.str(), Colors::Accent());
		return;
	}

	if (state.IsProductionView)
		return;

	if (state.Factory || (state.Building && state.Building->Type && state.Building->Type->Factory != AbstractType::None))
		block.Add(L"Production: None", UIColors().TextDim);
}

void ObserverUnitCard::AddHealthLine(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	auto const& win = *state.Window;

	if (win.IsDestroyed)
	{
		int const maxHP = (win.LastMaxHP > 0) ? win.LastMaxHP : (state.BaseType ? state.BaseType->Strength : 1);

		std::wostringstream oss;
		oss << L"0 / " << maxHP << L" (Destroyed)";
		block.Add(L"HP: ", Colors::Label(), oss.str(), Colors::Bad());
		return;
	}

	if (state.IsProductionView || (!state.Techno && !state.Building))
		return;

	int const currentHP = state.Techno ? state.Techno->Health : state.Building->Health;
	int maxHP = currentHP;

	if (state.Building && state.Building->Type)
		maxHP = state.Building->Type->Strength;
	else if (state.BaseType)
		maxHP = state.BaseType->Strength;

	DWORD hpColor = Colors::Good();
	if (currentHP < maxHP / 2)
		hpColor = UIColors().Warning;
	if (currentHP < maxHP / 4)
		hpColor = Colors::Bad();

	std::wostringstream oss;
	oss << currentHP << L" / " << maxHP;
	block.Add(Text("TXT_OBSERVER_CARD_HP", L"HP: "), Colors::Label(), oss.str(), hpColor);
}

void ObserverUnitCard::AddShieldLine(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	int currentShield = 0;
	int maxShield = 0;

	auto const pTechnoExt = state.Techno ? TechnoExtContainer::Instance.Find(state.Techno) : nullptr;
	if (pTechnoExt && pTechnoExt->ShieldEntity && pTechnoExt->ShieldEntity->GetType())
	{
		currentShield = pTechnoExt->ShieldEntity->GetHP();
		maxShield = pTechnoExt->ShieldEntity->GetType()->Strength.Get();
	}
	else if (auto const pTypeExt = state.BaseType ? TechnoTypeExtContainer::Instance.Find(state.BaseType) : nullptr)
	{
		if (pTypeExt->ShieldType)
		{
			maxShield = pTypeExt->ShieldType->Strength.Get();
			currentShield = maxShield;
		}
	}

	if (maxShield <= 0)
		return;

	std::wostringstream oss;
	oss << currentShield << L" / " << maxShield;

	block.Add(Text("TXT_OBSERVER_CARD_SHIELD", L"Shield: "), Colors::Label(), oss.str(),
		(currentShield <= 0) ? UIColors().TextDim : UIColors().Shield);
}

void ObserverUnitCard::AddTargetLines(ObserverTextBlock& block, const ObserverUnitCardState& state, const CellStruct& currentCell)
{
	AbstractClass* pRawTarget = state.Techno ? state.Techno->Target : (state.Building ? state.Building->Target : nullptr);

	TechnoClass* pTargetTechno = flag_cast_to<TechnoClass*>(pRawTarget);
	if (!pTargetTechno && state.Foot)
		pTargetTechno = flag_cast_to<TechnoClass*>(state.Foot->Destination);

	if (pTargetTechno && ObserverUIHelpers::IsTechnoValidAndAlive(pTargetTechno))
	{
		int const targetPlayerNumber = ObserverUIHelpers::GetPlayerNumber(*state.Rows, pTargetTechno->Owner);

		auto const pTargetType = pTargetTechno->GetTechnoType();
		std::wstring targetName;
		if (pTargetType)
		{
			targetName = (pTargetType->UIName && pTargetType->UIName[0] != L'\0')
				? std::wstring(pTargetType->UIName)
				: ObserverUIHelpers::ToWide(pTargetType->get_ID());
		}

		std::wostringstream oss;
		oss << Text("TXT_OBSERVER_CARD_TARGET", L"Target: ");
		if (targetPlayerNumber > 0)
			oss << L"[" << ObserverUIHelpers::PlayerPrefix() << targetPlayerNumber << L"] ";

		oss << targetName;
		block.Add(oss.str(), UIColors().Target);

		CellStruct const destinationCell = CellClass::Coord2Cell(pTargetTechno->GetCenterCoords());
		if (destinationCell != CellStruct::Empty && (destinationCell.X != currentCell.X || destinationCell.Y != currentCell.Y))
		{
			AddDistanceLine(block, Text("TXT_OBSERVER_CARD_DESTINATION", L"Destination: "),
				currentCell, destinationCell, UIColors().Target);
		}

		return;
	}

	if (pRawTarget)
	{
		if (auto const pCellTarget = cast_to<CellClass*>(pRawTarget))
		{
			AddDistanceLine(block, Text("TXT_OBSERVER_CARD_TARGET", L"Target: "),
				currentCell, pCellTarget->MapCoords, UIColors().Target);
		}

		return;
	}

	if (state.Foot && state.Foot->WaypointCell != CellStruct::Empty && state.Foot->WaypointCell.X > 0)
	{
		CellStruct const destinationCell = state.Foot->WaypointCell;
		if (destinationCell.X != currentCell.X || destinationCell.Y != currentCell.Y)
		{
			AddDistanceLine(block, Text("TXT_OBSERVER_CARD_DESTINATION", L"Destination: "),
				currentCell, destinationCell, UIColors().Destination);
		}
	}
}

void ObserverUnitCard::AddLocationLines(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	auto const& win = *state.Window;

	if (win.IsDestroyed)
	{
		std::wostringstream oss;
		oss << Text("TXT_OBSERVER_CARD_COORDS", L"Coords: ") << L"(" << win.LastCoords.X << L", " << win.LastCoords.Y << L")";

		if (!win.LastMission.empty())
			oss << L"   " << Text("TXT_OBSERVER_CARD_MISSION", L"Mission: ") << win.LastMission;

		block.Add(oss.str(), Colors::Muted());
		return;
	}

	if (state.IsProductionView || (!state.Techno && !state.Building))
		return;

	CellStruct const currentCell = CellClass::Coord2Cell(
		state.Techno ? state.Techno->GetCenterCoords() : state.Building->GetCenterCoords());

	std::wostringstream oss;
	oss << Text("TXT_OBSERVER_CARD_COORDS", L"Coords: ") << L"(" << currentCell.X << L", " << currentCell.Y << L")";

	bool showMission = false;
	if (state.Techno && state.Techno->WhatAmI() != AbstractType::Building)
		showMission = true;
	else if (state.Building && state.Building->Type && (state.Building->Type->Weapon[0].WeaponType || state.Building->Type->Weapon[1].WeaponType))
		showMission = true;

	if (showMission)
	{
		Mission const currentMission = state.Techno ? state.Techno->GetCurrentMission() : state.Building->GetCurrentMission();
		oss << L"   " << Text("TXT_OBSERVER_CARD_MISSION", L"Mission: ") << ObserverUIHelpers::GetMissionNameString(currentMission);

		if (state.IsDebug)
		{
			int const missionStatus = state.Techno ? state.Techno->MissionStatus : state.Building->MissionStatus;
			oss << L" (Status: " << missionStatus << L")";
		}
	}

	block.Add(oss.str(), Colors::Label());

	AddTargetLines(block, state, currentCell);
}

void ObserverUnitCard::AddAmmoLine(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	if (state.IsProductionView || !state.BaseType || state.BaseType->Ammo <= 0)
		return;

	int const maxAmmo = state.BaseType->Ammo;
	int currentAmmo = maxAmmo;

	if (state.Techno)
		currentAmmo = state.Techno->Ammo;
	else if (state.Building)
		currentAmmo = state.Building->Ammo;

	currentAmmo = std::clamp(currentAmmo, 0, maxAmmo);

	DWORD ammoColor = Colors::Accent();
	if (currentAmmo == 0)
		ammoColor = Colors::Bad();
	else if (currentAmmo < maxAmmo)
		ammoColor = UIColors().Warning;

	std::wostringstream oss;
	oss << currentAmmo << L" / " << maxAmmo;
	block.Add(Text("TXT_OBSERVER_CARD_AMMO", L"Ammo: "), Colors::Label(), oss.str(), ammoColor);
}

void ObserverUnitCard::AddVeterancyLine(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	if (state.IsProductionView || !state.BaseType || !state.BaseType->Trainable)
		return;

	auto const& win = *state.Window;

	float veterancy = 0.0f;
	if (win.IsDestroyed)
		veterancy = win.LastVeterancy;
	else if (state.Techno)
		veterancy = state.Techno->Veterancy.Veterancy;
	else if (state.Building)
		veterancy = state.Building->Veterancy.Veterancy;

	DWORD veterancyColor = Colors::Label();
	if (veterancy >= 2.0f)
		veterancyColor = UIColors().Veteran;
	else if (veterancy >= 1.0f)
		veterancyColor = Colors::Accent();

	wchar_t scoreBuffer[32];
	swprintf_s(scoreBuffer, L"%.2f", veterancy);

	std::wostringstream oss;
	oss << Text("TXT_OBSERVER_CARD_VETERANCY", L"Veterancy: ") << scoreBuffer;
	block.Add(oss.str(), veterancyColor);
}

void ObserverUnitCard::AddOccupantLines(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	if (state.IsProductionView)
		return;

	std::vector<TechnoTypeClass*> occupants;
	const wchar_t* label = nullptr;

	if (state.Foot)
	{
		if (state.Foot->Passengers.NumPassengers <= 0)
			return;

		label = Text("TXT_OBSERVER_CARD_PASSENGERS", L"Passengers");

		for (FootClass* pPassenger = state.Foot->Passengers.GetFirstPassenger(); pPassenger; pPassenger = flag_cast_to<FootClass*>(pPassenger->NextObject))
		{
			if (auto const pType = pPassenger->GetTechnoType())
				occupants.push_back(pType);
		}
	}
	else if (state.Building)
	{
		int const occupantCount = state.Building->Occupants.Count;
		if (occupantCount <= 0)
			return;

		label = Text("TXT_OBSERVER_CARD_GARRISONED", L"Garrisoned");

		for (int i = 0; i < occupantCount; ++i)
		{
			auto const pInfantry = state.Building->Occupants.get_or_default(i);
			if (pInfantry && pInfantry->Type)
				occupants.push_back(pInfantry->Type);
		}
	}
	else
	{
		return;
	}

	// The raw count is authoritative: types that could not be resolved still count.
	int const totalCount = state.Foot ? state.Foot->Passengers.NumPassengers : state.Building->Occupants.Count;

	if (state.IsDebug)
	{
		std::wostringstream oss;
		oss << label << L" (" << totalCount << L"): " << ObserverUIHelpers::FormatTypeCountList(occupants);
		block.Add(oss.str(), Colors::Label());
	}
	else
	{
		block.Add(std::wstring(label) + L": " + std::to_wstring(totalCount), Colors::Label());
	}
}

void ObserverUnitCard::AddBuildTimeAndCostLines(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	bool const isProducing = state.CurrentProduct && state.Factory && state.Factory->Object;
	if (!state.IsProductionView || !isProducing)
		return;

	int totalBuildFrames = 0;
	if (state.Factory)
		totalBuildFrames = state.Factory->GetBuildTimeFrames();
	else
		totalBuildFrames = PhobosToolTip::GetBuildTime(state.CurrentProduct, state.Owner);

	if (totalBuildFrames > 0)
	{
		std::wostringstream oss;
		oss << Text("TXT_OBSERVER_CARD_BUILD_TIME", L"Build Time: ") << ObserverUIHelpers::FormatDuration(totalBuildFrames);
		block.Add(oss.str(), Colors::Label());
	}

	std::wostringstream costOss;
	costOss << Text("TXT_OBSERVER_CARD_COST", L"Cost: $") << state.CurrentProduct->Cost;
	block.Add(costOss.str(), Colors::Label());
}

void ObserverUnitCard::AddDebugTeamLines(ObserverTextBlock& block, const ObserverUnitCardState& state)
{
	if (!state.IsDebug || !state.Foot || !state.Foot->BelongsToATeam() || !state.Foot->Team)
		return;

	auto const pTeam = state.Foot->Team;
	auto const pTeamType = pTeam->Type;

	if (pTeamType)
	{
		std::wstring const teamInfo = ObserverUIHelpers::FormatIDName(pTeamType);
		if (!teamInfo.empty())
			block.Add(std::wstring(Text("TXT_OBSERVER_CARD_TEAM", L"Team: ")) + teamInfo, Colors::Label());
	}

	if (pTeamType && pTeamType->TaskForce)
	{
		std::wstring const taskForceInfo = ObserverUIHelpers::FormatIDName(pTeamType->TaskForce);
		if (!taskForceInfo.empty())
			block.Add(std::wstring(Text("TXT_OBSERVER_CARD_TASKFORCE", L"Taskforce: ")) + taskForceInfo, Colors::Label());
	}

	if (!pTeam->CurrentScript || !pTeam->CurrentScript->Type)
		return;

	std::wstring const scriptInfo = ObserverUIHelpers::FormatIDName(pTeam->CurrentScript->Type);
	if (!scriptInfo.empty())
		block.Add(std::wstring(Text("TXT_OBSERVER_CARD_SCRIPT", L"Script: ")) + scriptInfo, Colors::Label());

	int const lineNumber = pTeam->CurrentScript->CurrentMission;
	if (lineNumber < 0)
		return;

	std::wostringstream oss;
	oss << Text("TXT_OBSERVER_CARD_SCRIPT_DATA", L"Script Data: Line ") << lineNumber;

	if (lineNumber < pTeam->CurrentScript->Type->ActionsCount)
	{
		auto const& action = pTeam->CurrentScript->Type->ScriptActions[lineNumber];
		oss << L" (" << static_cast<int>(action.Action) << L", " << action.Argument << L")";
	}

	block.Add(oss.str(), Colors::Label());
}

ObserverUnitCardContext ObserverUnitCard::Build(ObserverTextBlock& block, ObserverFloatingUnitWindow& win, const std::vector<ObserverPlayerRow>& rows)
{
	ObserverUnitCardContext context;

	bool const isDebug = Phobos::Config::DevelopmentCommands;
	int const playerNumber = ObserverUIHelpers::GetPlayerNumber(rows, win.pOwner);

	BuildingClass* pBuilding = ObserverUIHelpers::IsBuildingValidAndAlive(win.pTargetBuilding) ? win.pTargetBuilding : nullptr;
	TechnoClass* pTechno = ObserverUIHelpers::IsTechnoValidAndAlive(win.pTargetTechno) ? win.pTargetTechno : nullptr;

	TechnoTypeClass* pTargetType = win.pType;
	TechnoTypeClass* pBaseType = pBuilding ? pBuilding->Type : (pTechno ? pTechno->GetTechnoType() : pTargetType);

	FactoryClass* pFactory = (pBuilding && pBuilding->Factory) ? pBuilding->Factory : nullptr;
	if (!pFactory && win.IsProductionItem && pTargetType)
		pFactory = FactoryClass::FindByOwnerAndProduct(win.pOwner, pTargetType);

	if (!pBuilding && pFactory)
		pBuilding = ObserverUIHelpers::FindFactoryBuilding(win.pOwner, pFactory);

	TechnoTypeClass* pCurrentProduct = (pFactory && pFactory->Object) ? pFactory->Object->GetTechnoType() : nullptr;

	// A production card always follows whatever the factory is building right now.
	if (win.IsProductionItem && pFactory && pCurrentProduct)
	{
		win.pType = pCurrentProduct;
		pTargetType = pCurrentProduct;
	}

	bool const isProductionView = win.IsProductionItem;

	context.Factory = pFactory;
	context.CurrentProduct = pCurrentProduct;
	context.IsProductionView = isProductionView;
	context.CameoType = isProductionView ? (pTargetType ? pTargetType : pCurrentProduct) : pBaseType;

	if (win.IsSuperweapon || win.pSuperType)
	{
		AddSuperWeaponLines(block, win, playerNumber, isDebug);
		return context;
	}

	if (!pBaseType)
		return context;

	TechnoTypeClass* pTitleType = nullptr;
	if (isProductionView)
		pTitleType = pTargetType ? pTargetType : pCurrentProduct;
	else if (pBuilding)
		pTitleType = pBuilding->Type;
	else
		pTitleType = pBaseType;

	if (!pTitleType)
		pTitleType = pBaseType;

	ObserverUnitCardState state;
	state.Window = &win;
	state.Rows = &rows;
	state.Owner = win.pOwner;
	state.Building = pBuilding;
	state.Techno = pTechno;
	state.Foot = pTechno ? flag_cast_to<FootClass*>(pTechno) : nullptr;
	state.Factory = pFactory;
	state.BaseType = pBaseType;
	state.TitleType = pTitleType;
	state.TargetType = pTargetType;
	state.CurrentProduct = pCurrentProduct;
	state.IsProductionView = isProductionView;
	state.IsDebug = isDebug;
	state.PlayerNumber = playerNumber;

	UpdateSnapshot(state);

	AddTitleLine(block, state);
	AddOwnerLine(block, state.Owner);
	AddProductionLines(block, state);
	AddHealthLine(block, state);
	AddShieldLine(block, state);
	AddLocationLines(block, state);
	AddAmmoLine(block, state);
	AddVeterancyLine(block, state);
	AddOccupantLines(block, state);
	AddBuildTimeAndCostLines(block, state);
	AddDebugTeamLines(block, state);

	return context;
}

// ---- ObserverWindowFactory ----------------------------------------------------------------

bool ObserverWindowFactory::OpenCameoWindow(
	std::vector<ObserverFloatingUnitWindow>& unitWindows,
	size_t otherWindowCount,
	ObserverCycleIndexMap& cycleIndices,
	const ObserverCameoItem& item,
	bool isFromProductionPanel)
{
	size_t const openWindowCount = unitWindows.size() + otherWindowCount;

	if (item.IsSuperweapon && item.pSuperType)
	{
		bool const focused = ObserverUIHelpers::BringToFrontIf(unitWindows, [&item](const ObserverFloatingUnitWindow& window)
 {
	 return window.IsSuperweapon && window.pSuperType == item.pSuperType && window.pOwner == item.pOwner;
		});

		if (!focused)
		{
			ObserverFloatingUnitWindow newWindow;
			newWindow.IsSuperweapon = true;
			newWindow.pSuperType = item.pSuperType;
			newWindow.pSuper = item.pSuper;
			newWindow.pOwner = item.pOwner;

			if (!item.Buildings.empty())
				newWindow.pTargetBuilding = item.Buildings.front();

			newWindow.Position = ObserverUIHelpers::CascadePosition(openWindowCount, UnitCardWidth);
			unitWindows.push_back(newWindow);
		}

		return true;
	}

	TechnoClass* pTargetTechno = nullptr;
	BuildingClass* pTargetBuilding = nullptr;
	int instanceNumber = 1;

	if (isFromProductionPanel)
	{
		if (!item.Buildings.empty())
			pTargetBuilding = item.Buildings.front();
	}
	else
	{
		auto const key = std::make_pair(item.pOwner, reinterpret_cast<uintptr_t>(item.pType));

		if (!item.Buildings.empty())
		{
			size_t const cycleIndex = cycleIndices[key];
			size_t const index = (cycleIndex > 0 ? cycleIndex - 1 : 0) % item.Buildings.size();
			pTargetBuilding = item.Buildings[index];
			instanceNumber = static_cast<int>(index + 1);
		}
		else if (!item.Technos.empty())
		{
			size_t const cycleIndex = cycleIndices[key];
			size_t const index = (cycleIndex > 0 ? cycleIndex - 1 : 0) % item.Technos.size();
			pTargetTechno = item.Technos[index];
			instanceNumber = static_cast<int>(index + 1);
		}
	}

	if (!pTargetTechno && !pTargetBuilding && !isFromProductionPanel)
		return true;

	// Reuse the window if one already exists for this exact instance or factory.
	bool const focused = ObserverUIHelpers::BringToFrontIf(unitWindows,
		[&item, pTargetTechno, pTargetBuilding, isFromProductionPanel](const ObserverFloatingUnitWindow& window)
 {
	 if (pTargetBuilding && window.pTargetBuilding == pTargetBuilding)
		 return true;

	 if (pTargetTechno && window.pTargetTechno == pTargetTechno)
		 return true;

	 return isFromProductionPanel && window.IsProductionItem && window.pType == item.pType && window.pOwner == item.pOwner;
		});

	if (!focused)
	{
		ObserverFloatingUnitWindow newWindow;
		newWindow.pType = isFromProductionPanel
			? item.pType
			: (pTargetTechno ? pTargetTechno->GetTechnoType() : (pTargetBuilding ? pTargetBuilding->Type : item.pType));
		newWindow.pOwner = item.pOwner;
		newWindow.pTargetTechno = pTargetTechno;
		newWindow.pTargetBuilding = pTargetBuilding;
		newWindow.IsProductionItem = isFromProductionPanel;
		newWindow.InstanceNumber = instanceNumber;
		newWindow.Position = ObserverUIHelpers::CascadePosition(openWindowCount, UnitCardWidth);
		unitWindows.push_back(newWindow);
	}

	return true;
}

void ObserverWindowFactory::OpenObjectWindow(
	std::vector<ObserverFloatingUnitWindow>& unitWindows,
	size_t otherWindowCount,
	TechnoClass* pTechno,
	BuildingClass* pBuilding)
{
	size_t const openWindowCount = unitWindows.size() + otherWindowCount;

	bool const focused = ObserverUIHelpers::BringToFrontIf(unitWindows, [pTechno, pBuilding](const ObserverFloatingUnitWindow& window)
 {
	 if (pBuilding && window.pTargetBuilding == pBuilding)
		 return true;

	 return pTechno && window.pTargetTechno == pTechno;
	});

	if (focused)
		return;

	ObserverFloatingUnitWindow newWindow;
	newWindow.pType = pTechno ? pTechno->GetTechnoType() : (pBuilding ? pBuilding->Type : nullptr);
	newWindow.pOwner = pTechno ? pTechno->Owner : (pBuilding ? pBuilding->Owner : nullptr);
	newWindow.pTargetTechno = pTechno;
	newWindow.pTargetBuilding = pBuilding;
	newWindow.IsProductionItem = false;
	newWindow.InstanceNumber = 1;
	newWindow.Position = ObserverUIHelpers::CascadePosition(openWindowCount, UnitCardWidth);
	unitWindows.push_back(newWindow);
}

void ObserverWindowFactory::OpenPlayerWindow(std::vector<ObserverFloatingWindow>& windows, size_t otherWindowCount, HouseClass* pHouse)
{
	size_t const openWindowCount = windows.size() + otherWindowCount;

	bool const focused = ObserverUIHelpers::BringToFrontIf(windows, [pHouse](const ObserverFloatingWindow& window)
 {
	 return window.pHouse == pHouse;
	});

	if (focused)
		return;

	ObserverFloatingWindow newWindow;
	newWindow.pHouse = pHouse;
	newWindow.Position = ObserverUIHelpers::CascadePosition(openWindowCount, PlayerCardWidth);
	windows.push_back(newWindow);
}

// ---- ObserverUIState ----------------------------------------------------------------

bool ObserverUIState::IsInteractive(ObserverUIDisplayMode mode, bool hasFloatingWindows)
{
	return ObserverUIClass::IsActive()
		|| (Phobos::Config::DevelopmentCommands && (mode != ObserverUIDisplayMode::Hidden || hasFloatingWindows));
}

bool ObserverUIClass::IsActive()
{
	if (!ScenarioClass::Instance() || HouseClass::Array->Count == 0 || !HouseClass::CurrentPlayer())
		return false;

	return HouseClass::IsCurrentPlayerObserver()
		|| (HouseClass::Observer() && HouseClass::CurrentPlayer->IsObserver());
}

// Called from the engine-wide detach event (PointerGotInvalid / DetachAll). Every pointer the
// UI keeps across frames is dropped here, which is what lets IsTechnoValidAndAlive and
// IsBuildingValidAndAlive be plain liveness tests instead of linear array scans.
//
// NOTE: floating unit cards are deliberately NOT erased when their target dies - they keep the
// last known snapshot and switch to the "Destroyed" display. Only the raw pointers are nulled.
void ObserverUIClass::CleanInvalidPointer(AbstractClass* pInvalid)
{
	if (!pInvalid)
		return;

	const void* const pTarget = pInvalid;

	// Unit / production / superweapon cards
	for (size_t i = this->FloatingUnitWindows.size(); i > 0; --i)
	{
		auto& win = this->FloatingUnitWindows[i - 1];

		if (static_cast<const void*>(win.pTargetTechno) == pTarget)
			win.pTargetTechno = nullptr;

		if (static_cast<const void*>(win.pTargetBuilding) == pTarget)
			win.pTargetBuilding = nullptr;

		if (static_cast<const void*>(win.pType) == pTarget)
			win.pType = nullptr;

		if (static_cast<const void*>(win.pSuperType) == pTarget)
			win.pSuperType = nullptr;

		// The house owns its SuperClass instances, so both go away together and the card has
		// nothing left to describe.
		if (static_cast<const void*>(win.pOwner) == pTarget)
			this->FloatingUnitWindows.erase(this->FloatingUnitWindows.begin() + (i - 1));
	}

	// Player cards
	for (size_t i = this->FloatingWindows.size(); i > 0; --i)
	{
		if (static_cast<const void*>(this->FloatingWindows[i - 1].pHouse) == pTarget)
			this->FloatingWindows.erase(this->FloatingWindows.begin() + (i - 1));
	}

	// Hover state
	if (static_cast<const void*>(this->pHoveredPlayer) == pTarget)
	{
		this->pHoveredPlayer = nullptr;
		this->HasHoveredPlayer = false;
	}

	if (this->HasHoveredItem
		&& (static_cast<const void*>(this->HoveredItem.pOwner) == pTarget
			|| static_cast<const void*>(this->HoveredItem.pType) == pTarget
			|| static_cast<const void*>(this->HoveredItem.pSuperType) == pTarget
			|| static_cast<const void*>(this->HoveredItem.pSuper) == pTarget))
	{
		this->HoveredItem = {};
		this->HasHoveredItem = false;
	}

	// Rows are rebuilt every frame, but a detach can land between the rebuild and the draw.
	for (size_t i = this->PlayerRows.size(); i > 0; --i)
	{
		auto& row = this->PlayerRows[i - 1];

		if (static_cast<const void*>(row.TargetEnemy) == pTarget)
			row.TargetEnemy = nullptr;

		if (static_cast<const void*>(row.pHouse) == pTarget)
		{
			this->PlayerRows.erase(this->PlayerRows.begin() + (i - 1));
			continue;
		}

		auto const purgeItems = [pTarget](std::vector<ObserverCameoItem>& items)
			{
				for (auto& item : items)
				{
					if (static_cast<const void*>(item.pSuper) == pTarget)
						item.pSuper = nullptr;

					item.Buildings.erase(
						std::remove_if(item.Buildings.begin(), item.Buildings.end(), [pTarget](BuildingClass* pBuilding)
							{
								return static_cast<const void*>(pBuilding) == pTarget;
							}),
						item.Buildings.end());

					item.Technos.erase(
						std::remove_if(item.Technos.begin(), item.Technos.end(), [pTarget](TechnoClass* pTechno)
							{
								return static_cast<const void*>(pTechno) == pTarget;
							}),
						item.Technos.end());

					item.Count = static_cast<int>(item.Technos.size());
				}
			};

		purgeItems(row.ProductionItems);
		purgeItems(row.StructureItems);
	}

	// Per-house caches
	for (auto it = this->EconomyHistory.begin(); it != this->EconomyHistory.end();)
	{
		it = (static_cast<const void*>(it->first) == pTarget) ? this->EconomyHistory.erase(it) : std::next(it);
	}

	for (auto it = this->CycleIndices.begin(); it != this->CycleIndices.end();)
	{
		bool const isStale = static_cast<const void*>(it->first.first) == pTarget
			|| it->first.second == reinterpret_cast<uintptr_t>(pTarget);

		it = isStale ? this->CycleIndices.erase(it) : std::next(it);
	}

	// The rows just changed under the throttle, so force the next rebuild.
	this->LastCollectFrame = -1;
}

void ObserverUIClass::ClearData()
{
	this->DisplayMode = ObserverUIDisplayMode::Hidden;
	this->PlayerRows.clear();

	if (!Phobos::Config::DevelopmentCommands)
	{
		this->FloatingWindows.clear();
		this->FloatingUnitWindows.clear();
	}

	this->EconomyHistory.clear();
	this->CycleIndices.clear();
	this->TabButtons.clear();
	this->SearchFilterText.clear();
	this->SearchTerms.clear();
	this->LastCollectFrame = -1;
	this->IsSearchInputFocused = false;
	this->HoveredItem = {};
	this->HasHoveredItem = false;
	this->pHoveredPlayer = nullptr;
	this->HasHoveredPlayer = false;
	this->WasEnterPressed = false;
	this->VerticalScrollOffset = 0;
	this->MaxVerticalScrollOffset = 0;
}

std::vector<std::wstring> ObserverUIClass::ParseSearchTerms(const std::wstring& query) const
{
	std::vector<std::wstring> terms;
	std::wstring currentTerm;
	bool inQuotes = false;

	auto const flushTerm = [&terms, &currentTerm]()
		{
			if (!currentTerm.empty())
			{
				terms.push_back(currentTerm);
				currentTerm.clear();
			}
		};

	for (wchar_t const character : query)
	{
		if (character == L'"')
		{
			flushTerm();
			inQuotes = !inQuotes;
		}
		else if (character == L' ' && !inQuotes)
		{
			flushTerm();
		}
		else
		{
			currentTerm += character;
		}
	}

	flushTerm();

	for (auto& term : terms)
	{
		std::transform(term.begin(), term.end(), term.begin(), ::towlower);
	}

	return terms;
}

bool ObserverUIClass::MatchesSearchFilter(AbstractTypeClass* pType) const
{
	if (!pType)
		return false;

	switch (pType->WhatAmI())
	{
	case AbstractType::BuildingType:
	{
		auto const pBuildingType = static_cast<BuildingTypeClass*>(pType);
		if (pBuildingType->InvisibleInGame || pBuildingType->Invisible)
			return false;

		break;
	}
	case AbstractType::UnitType:
	case AbstractType::InfantryType:
	case AbstractType::AircraftType:
	{
		if (static_cast<TechnoTypeClass*>(pType)->Invisible)
			return false;

		break;
	}
	default:
		break;
	}

	// OPTIMIZATION: SearchTerms is parsed once per CollectPlayerData rebuild instead of once
	// per candidate object (this runs thousands of times per frame).
	if (this->SearchTerms.empty())
		return true;

	std::wstring name;
	if (pType->UIName && *pType->UIName)
		name = pType->UIName;
	else if (pType->ID)
		name = ObserverUIHelpers::ToWide(pType->ID);

	std::transform(name.begin(), name.end(), name.begin(), ::towlower);

	for (auto const& term : this->SearchTerms)
	{
		if (name.find(term) == std::wstring::npos)
			return false;
	}

	return true;
}

// OPTIMIZATION: this used to be O(houses x every object in the game) every single frame -
// each house re-scanned BuildingClass / InfantryClass / UnitClass / AircraftClass /
// FactoryClass::Array in full, once for HouseHasContent and once more for the item grouping.
// The scans are inverted now: every global array is walked exactly once and each object is
// dispatched to its owner's row through a hash lookup.
void ObserverUIClass::CollectPlayerData(bool force)
{
	// OPTIMIZATION: Update() can run more than once per frame (and Render() calls Update()),
	// but the grouped item lists only change with the simulation. Rebuild once per frame
	// unless a filter tab / search text change explicitly forces it.
	int const currentFrame = Unsorted::CurrentFrame();
	if (!force && currentFrame == this->LastCollectFrame)
		return;

	this->LastCollectFrame = currentFrame;

	// OPTIMIZATION: the search query used to be re-parsed - allocating a fresh vector of
	// wstrings - inside MatchesSearchFilter for every object of every array of every house.
	// It is parsed once per rebuild now and cached in SearchTerms.
	this->SearchTerms = this->ParseSearchTerms(this->SearchFilterText);

	// Preserve existing scroll offsets when refreshing player rows
	std::unordered_map<HouseClass*, int> previousScrollOffsets;
	previousScrollOffsets.reserve(this->PlayerRows.size());
	for (auto const& existingRow : this->PlayerRows)
	{
		previousScrollOffsets[existingRow.pHouse] = existingRow.ScrollOffset;
	}

	this->PlayerRows.clear();

	if (!HouseClass::Array->Count)
		return;

	bool const isMultiplayer = ObserverUIHelpers::IsMultiplayerSession();
	bool const isDevMode = Phobos::Config::DevelopmentCommands;

	// In standard multiplayer every listed house is shown unconditionally; everywhere else a
	// house only earns a row once it actually owns something (see HasContent below).
	bool const requiresContent = !(isMultiplayer && !isDevMode);

	// -------------------------------------------------------------------------------------
	// Phase 1: candidate rows, in HouseClass::Array order
	// -------------------------------------------------------------------------------------
	std::vector<ObserverPlayerRow> candidateRows;
	std::vector<bool> hasContent;
	std::vector<std::map<TechnoTypeClass*, std::vector<TechnoClass*>>> filterGroups;
	std::vector<std::map<TechnoTypeClass*, std::vector<BuildingClass*>>> productionGroups;
	std::vector<std::map<TechnoTypeClass*, int>> productionProgress;
	std::unordered_map<HouseClass*, size_t> rowLookup;

	candidateRows.reserve(static_cast<size_t>(HouseClass::Array->Count));
	rowLookup.reserve(static_cast<size_t>(HouseClass::Array->Count));

	for (int i = 0; i < HouseClass::Array->Count; ++i)
	{
		auto const pHouse = HouseClass::Array->get_or_default(i);
		if (!pHouse || pHouse->IsObserver() || pHouse->Defeated || !pHouse->Type)
			continue;

		// In standard multiplayer show only real skirmish/MP players
		// (exclude MultiplayPassive, Neutral, Special, Civilian).
		if (!requiresContent && ObserverUIHelpers::IsHouseHiddenInMultiplayer(pHouse))
			continue;

		ObserverPlayerRow row;
		row.pHouse = pHouse;
		row.PlayerNumber = pHouse->ArrayIndex + 1; // Direct slot in HouseClass::Array (Base 1)

		std::wstring const plainName = ObserverUIHelpers::GetHousePlainName(pHouse);
		row.PlayerName = isMultiplayer
			? (ObserverUIHelpers::PlayerPrefix() + std::to_wstring(row.PlayerNumber) + L": " + plainName)
			: plainName;
		row.CountryName = pHouse->Type->UIName;

		// Calculate economy rate per minute (+- $X/min) based on rolling sample history
		row.IncomeRatePerMin = 0;
		auto const itHistory = this->EconomyHistory.find(pHouse);
		if (itHistory != this->EconomyHistory.end() && !itHistory->second.empty())
		{
			auto const& latest = itHistory->second.back();
			auto const& oldest = itHistory->second.front();

			int const frameDiff = latest.Frame - oldest.Frame;
			if (frameDiff > 0)
			{
				float const secondsDiff = frameDiff / static_cast<float>(ObserverUIHelpers::LogicFramesPerSecond);
				if (secondsDiff >= 1.0f)
					row.IncomeRatePerMin = static_cast<int>(((latest.Money - oldest.Money) / secondsDiff) * 60.0f);
			}
		}

		row.TargetEnemy = ObserverUIHelpers::GetTargetEnemy(pHouse);
		row.TargetEnemyName = row.TargetEnemy ? ObserverUIHelpers::GetHousePlainName(row.TargetEnemy) : std::wstring(L"None");

		// Restore previous scroll offset if valid
		auto const itScroll = previousScrollOffsets.find(pHouse);
		if (itScroll != previousScrollOffsets.end())
			row.ScrollOffset = itScroll->second;

		// Assign actual player house ColorScheme BaseColor
		row.PlayerColor = ObserverUIHelpers::GetHouseColor(pHouse, row.PlayerNumber - 1);
		row.PlayerColorValue = Drawing::RGB2DWORD(row.PlayerColor.R, row.PlayerColor.G, row.PlayerColor.B);

		rowLookup.emplace(pHouse, candidateRows.size());
		candidateRows.emplace_back(std::move(row));
	}

	if (candidateRows.empty())
		return;

	hasContent.assign(candidateRows.size(), !requiresContent);
	filterGroups.resize(candidateRows.size());
	productionGroups.resize(candidateRows.size());
	productionProgress.resize(candidateRows.size());

	auto const tab = this->ActiveFilterTab;

	// The array scans below always run (HasContent has to see every category), but the item
	// grouping is still limited to the categories the active tab can actually display.
	bool const wantsBuildings = tab == ObserverFilterCategory::Defenses
		|| tab == ObserverFilterCategory::Structures
		|| tab == ObserverFilterCategory::AllStructures
		|| tab == ObserverFilterCategory::Vehicles
		|| tab == ObserverFilterCategory::Naval
		|| tab == ObserverFilterCategory::AllUnits
		|| tab == ObserverFilterCategory::Everything;

	bool const wantsInfantry = tab == ObserverFilterCategory::Infantry
		|| tab == ObserverFilterCategory::Naval
		|| tab == ObserverFilterCategory::AllUnits
		|| tab == ObserverFilterCategory::Everything;

	bool const wantsUnits = tab == ObserverFilterCategory::Vehicles
		|| tab == ObserverFilterCategory::Naval
		|| tab == ObserverFilterCategory::Aircraft
		|| tab == ObserverFilterCategory::AllUnits
		|| tab == ObserverFilterCategory::Everything;

	bool const wantsAircraft = tab == ObserverFilterCategory::Aircraft
		|| tab == ObserverFilterCategory::AllUnits
		|| tab == ObserverFilterCategory::Everything;

	// -------------------------------------------------------------------------------------
	// Phase 2: one pass over every object array, dispatched by owner
	// -------------------------------------------------------------------------------------
	auto const scanArray = [this, &rowLookup, &hasContent, &filterGroups, tab](auto& array, bool wantsCategory)
		{
			for (auto const pObject : array)
			{
				if (!pObject || !pObject->IsAlive || pObject->InLimbo)
					continue;

				auto const itRow = rowLookup.find(pObject->Owner);
				if (itRow == rowLookup.end())
					continue;

				size_t const rowIndex = itRow->second;
				hasContent[rowIndex] = true;

				if (!wantsCategory)
					continue;

				auto const pType = pObject->GetTechnoType();
				if (!pType || !this->MatchesSearchFilter(pType))
					continue;

				if (ObserverUIHelpers::MatchesFilterCategory(pObject, pType, tab))
					filterGroups[rowIndex][pType].push_back(pObject);
			}
		};

	scanArray(*BuildingClass::Array, wantsBuildings);
	scanArray(*InfantryClass::Array, wantsInfantry);
	scanArray(*UnitClass::Array, wantsUnits);
	scanArray(*AircraftClass::Array, wantsAircraft);

	// -------------------------------------------------------------------------------------
	// Phase 3: one pass over the factories, grouped by produced TechnoType
	// -------------------------------------------------------------------------------------
	for (auto const pFactory : *FactoryClass::Array)
	{
		if (!pFactory || !pFactory->Object)
			continue;

		auto const itRow = rowLookup.find(pFactory->Owner);
		if (itRow == rowLookup.end())
			continue;

		size_t const rowIndex = itRow->second;
		hasContent[rowIndex] = true;

		auto const pProducingType = pFactory->Object->GetTechnoType();
		if (!pProducingType || !this->MatchesSearchFilter(pProducingType))
			continue;

		// BUGFIX: FindFactoryBuilding returns nullptr for factories with no owning building
		// (base placement, dead factory). Pushing that null made the cameo cycle stop on an
		// empty slot and anchored unit cards to nothing.
		auto& factoryBuildings = productionGroups[rowIndex][pProducingType]; // creates the group either way
		if (auto const pFactoryBuilding = ObserverUIHelpers::FindFactoryBuilding(pFactory->Owner, pFactory))
			factoryBuildings.push_back(pFactoryBuilding);

		auto& progress = productionProgress[rowIndex][pProducingType];
		progress = std::max(progress, ObserverUIHelpers::GetFactoryProgressPercent(pFactory));
	}

	// -------------------------------------------------------------------------------------
	// Phase 4: superweapons and final row assembly
	// -------------------------------------------------------------------------------------
	for (size_t rowIndex = 0; rowIndex < candidateRows.size(); ++rowIndex)
	{
		auto& row = candidateRows[rowIndex];
		auto const pHouse = row.pHouse;

		for (int s = 0; s < pHouse->Supers.Count; ++s)
		{
			auto const pSuper = pHouse->Supers.get_or_default(s);
			if (!pSuper || !pSuper->Type || !pSuper->Granted)
				continue;

			hasContent[rowIndex] = true;

			if (tab != ObserverFilterCategory::Superweapons || !this->MatchesSearchFilter(pSuper->Type))
				continue;

			// Check Phobos SWTypeExt visibility settings
			auto const pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
			if (pSWExt && !pSWExt->SW_ShowCameo && pSWExt->SW_AutoFire)
				continue;

			ObserverCameoItem item;
			item.pSuperType = pSuper->Type;
			item.pSuper = pSuper;
			item.IsSuperweapon = true;
			item.pOwner = pHouse;
			item.Count = 1;
			item.Buildings = ObserverUIHelpers::CollectSuperWeaponBuildings(pHouse, pSuper->Type);

			int const totalFrames = pSuper->GetRechargeTime();
			int const framesLeft = pSuper->RechargeTimer.GetTimeLeft();
			item.ProgressPercent = (totalFrames > 0 && framesLeft > 0)
				? std::clamp(((totalFrames - framesLeft) * 100) / totalFrames, 0, 100)
				: 100;

			row.StructureItems.emplace_back(std::move(item));
		}

		if (!hasContent[rowIndex])
			continue;

		for (auto& [pType, factories] : productionGroups[rowIndex])
		{
			ObserverCameoItem productionItem;
			productionItem.pType = pType;
			productionItem.ProgressPercent = productionProgress[rowIndex][pType];
			productionItem.IsProduction = true;
			productionItem.pOwner = pHouse;
			productionItem.Buildings = std::move(factories);

			row.ProductionItems.emplace_back(std::move(productionItem));
		}

		for (auto& [pType, technos] : filterGroups[rowIndex])
		{
			ObserverCameoItem item;
			item.pType = pType;
			item.Count = static_cast<int>(technos.size());
			item.IsProduction = false;
			item.pOwner = pHouse;
			item.Technos.reserve(technos.size());

			for (auto const pTechno : technos)
			{
				// BUGFIX: instances report AbstractType::Building, never AbstractType::BuildingType,
				// so the building list used to stay empty and cameo cycling fell back to Technos.
				if (pTechno->WhatAmI() == AbstractType::Building)
					item.Buildings.push_back(static_cast<BuildingClass*>(pTechno));

				item.Technos.push_back(pTechno);
			}

			row.StructureItems.emplace_back(std::move(item));
		}

		this->PlayerRows.emplace_back(std::move(row));
	}

	// Detect mutual alliance teams ONLY for 2 or more mutually allied players
	std::vector<int> assignedTeams(this->PlayerRows.size(), -1);
	std::map<int, ColorStruct> teamColors;
	std::map<int, int> teamCounts;
	int nextTeamID = 0;

	for (size_t i = 0; i < this->PlayerRows.size(); ++i)
	{
		if (assignedTeams[i] != -1)
			continue;

		std::vector<size_t> teamMembers { i };
		for (size_t j = i + 1; j < this->PlayerRows.size(); ++j)
		{
			if (assignedTeams[j] == -1 && this->PlayerRows[i].pHouse->IsMutualAlly(this->PlayerRows[j].pHouse))
				teamMembers.push_back(j);
		}

		// The team alliance line is rendered ONLY if 2 or more players are mutually allied!
		if (teamMembers.size() < 2)
			continue;

		int const teamID = nextTeamID++;
		teamColors[teamID] = ObserverUIHelpers::PaletteColor(static_cast<size_t>(teamID));
		teamCounts[teamID] = static_cast<int>(teamMembers.size());

		for (size_t const index : teamMembers)
		{
			assignedTeams[index] = teamID;
		}
	}

	for (size_t i = 0; i < this->PlayerRows.size(); ++i)
	{
		auto& row = this->PlayerRows[i];
		row.TeamID = assignedTeams[i];

		if (row.TeamID != -1)
		{
			row.TeamColor = teamColors[row.TeamID];
			row.TeamColorValue = Drawing::RGB2DWORD(row.TeamColor.R, row.TeamColor.G, row.TeamColor.B);
			row.TeamMemberCount = teamCounts[row.TeamID];
		}
		else
		{
			row.TeamMemberCount = 1;
		}
	}

	// Stable sort rows so allied team members are grouped adjacently
	std::stable_sort(this->PlayerRows.begin(), this->PlayerRows.end(), [](const ObserverPlayerRow& first, const ObserverPlayerRow& second)
 {
	 if (first.TeamID == second.TeamID)
		 return false;

	 if (first.TeamID == -1)
		 return false;

	 if (second.TeamID == -1)
		 return true;

	 return first.TeamID < second.TeamID;
	});
}

void ObserverUIClass::Update()
{
	if (!ObserverUIState::IsInteractive(this->DisplayMode, this->HasFloatingWindows()))
	{
		if (!this->PlayerRows.empty() || !this->EconomyHistory.empty())
			this->ClearData();

		return;
	}

	// ENTER acts as the default observer key when no hotkey is assigned in the key options menu.
	bool const isEnterPressed = ObserverUIHelpers::IsKeyDown(VK_RETURN);
	if (isEnterPressed && !this->WasEnterPressed)
	{
		if (this->IsSearchInputFocused)
		{
			// Defocus the search box ONLY, do NOT toggle the card window!
			this->IsSearchInputFocused = false;
		}
		else
		{
			// If Show Object Card is unassigned, ENTER opens the card for the hovered/selected object.
			bool const cardOpened = !IsShowObjectCardHotkeyBound() && this->OpenFloatingWindowForSelectedObject();

			// If no card was opened and Toggle Observer UI is unassigned, ENTER toggles the display mode.
			if (!cardOpened && !IsToggleObserverUIHotkeyBound())
				this->ToggleDisplayMode();
		}
	}
	this->WasEnterPressed = isEnterPressed;

	if (this->DisplayMode == ObserverUIDisplayMode::Hidden && !this->HasFloatingWindows())
		return;

	if (this->IsMouseHoveringUI() || this->IsSearchFocused())
		MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);

	// Update dragging position of floating card windows
	Point2D const mousePos = ObserverUIHelpers::MousePosition();
	bool const isLeftPressed = ObserverUIHelpers::IsKeyDown(VK_LBUTTON);

	bool const draggingPlayerCards = ObserverUIHelpers::UpdateWindowDrag(this->FloatingWindows, mousePos, isLeftPressed);
	bool const draggingUnitCards = ObserverUIHelpers::UpdateWindowDrag(this->FloatingUnitWindows, mousePos, isLeftPressed);

	if (draggingPlayerCards || draggingUnitCards)
		DisplayClass::Instance->ClearDragBand();

	// Handle keyboard input & real-time typematic auto-repeat if the search box is focused
	static int heldVK = -1;
	static unsigned long long firstPressTimeMs = 0;
	static unsigned long long lastRepeatTimeMs = 0;

	unsigned long long const nowMs = GetTickCount64();

	if (!this->IsSearchInputFocused)
	{
		heldVK = -1;
		firstPressTimeMs = 0;
		lastRepeatTimeMs = 0;
	}
	else if (ObserverUIHelpers::IsKeyDown(VK_ESCAPE) || ObserverUIHelpers::IsKeyDown(VK_RETURN))
	{
		// Escape or Enter -> Unfocus search box
		this->IsSearchInputFocused = false;
		heldVK = -1;
		firstPressTimeMs = 0;
		lastRepeatTimeMs = 0;
	}
	else
	{
		ObserverSearchKey const key = ObserverUIHelpers::PollSearchKey();
		bool const isShiftHeld = ObserverUIHelpers::IsKeyDown(VK_SHIFT);
		bool textChanged = false;

		if (key.VirtualKey == -1)
		{
			heldVK = -1;
			firstPressTimeMs = 0;
			lastRepeatTimeMs = 0;
		}
		else if (key.VirtualKey != heldVK)
		{
			// First press event: trigger immediately!
			heldVK = key.VirtualKey;
			firstPressTimeMs = nowMs;
			lastRepeatTimeMs = nowMs;
			textChanged = ObserverUIHelpers::ApplySearchKey(this->SearchFilterText, key, isShiftHeld);
		}
		else if ((nowMs - firstPressTimeMs) >= 450 && (nowMs - lastRepeatTimeMs) >= 60)
		{
			// Key is held down: initial delay 450 ms, repeat rate 60 ms
			lastRepeatTimeMs = nowMs;
			textChanged = ObserverUIHelpers::ApplySearchKey(this->SearchFilterText, key, isShiftHeld);
		}

		if (textChanged)
			this->CollectPlayerData(true);
	}

	int const currentFrame = Unsorted::CurrentFrame();

	// Sample money for all active houses once every 15 frames (~1 second)
	static int lastSampleFrame = -1;
	if (lastSampleFrame == -1 || (currentFrame - lastSampleFrame) >= ObserverUIHelpers::LogicFramesPerSecond)
	{
		lastSampleFrame = currentFrame;

		bool const isMultiplayer = ObserverUIHelpers::IsMultiplayerSession();
		bool const isDevMode = Phobos::Config::DevelopmentCommands;

		for (int i = 0; i < HouseClass::Array->Count; ++i)
		{
			auto const pHouse = HouseClass::Array->get_or_default(i);
			if (!pHouse || pHouse->Defeated || !pHouse->Type || pHouse->IsObserver())
				continue;

			if (isMultiplayer && !isDevMode && ObserverUIHelpers::IsHouseHiddenInMultiplayer(pHouse))
				continue;

			auto& history = this->EconomyHistory[pHouse];
			history.push_back({ currentFrame, pHouse->Available_Money() });

			// Keep at most 60 samples (60 seconds rolling history window)
			while (history.size() > 60)
			{
				history.pop_front();
			}
		}
	}

	// Skip player data collection ONLY if in Minimal mode with no open floating windows (saves CPU)
	bool const isMinimalEmpty = this->DisplayMode == ObserverUIDisplayMode::Minimal && !this->HasFloatingWindows();
	if (!isMinimalEmpty)
		this->CollectPlayerData();
}


void ObserverUIClass::Render(DSurface* pSurface)
{
	if (!pSurface || !ObserverUIState::IsInteractive(this->DisplayMode, this->HasFloatingWindows()))
		return;

	this->Update();

	if (this->DisplayMode == ObserverUIDisplayMode::Hidden)
		return;

	Point2D const mousePos = ObserverUIHelpers::MousePosition();

	this->HasHoveredItem = false;
	this->HasHoveredPlayer = false;

	if (this->DisplayMode == ObserverUIDisplayMode::Minimal)
	{
		// 1. Render Floating Player Windows & Floating Unit Windows
		this->RenderFloatingWindows(pSurface);
		this->RenderFloatingUnitWindows(pSurface);

		// 2. Render Inspect Button at bottom-left (54x40, 18px bottom padding so the
		//    bottom info text stays visible)
		this->InspectBtnRect = RectangleStruct { 16, DSurface::ViewBounds->Height - 58, 54, 40 };
		this->IsHoveringInspectBtn = ObserverUIHelpers::HitTest(this->InspectBtnRect, mousePos);

		ObserverUIHelpers::DrawPanel(pSurface, this->InspectBtnRect,
			this->IsHoveringInspectBtn ? ObserverRGB::Accent : ObserverRGB::Black,
			this->IsHoveringInspectBtn ? 180 : 120,
			this->IsHoveringInspectBtn ? UIColors().White : UIColors().BorderLight);

		ObserverUIHelpers::DrawCenteredText(pSurface, this->InspectBtnRect, L"-> [] <-",
			this->IsHoveringInspectBtn ? UIColors().White : UIColors().TextLabel);

		// 3. Render tooltip for the Inspect Button (floating windows own their own hover state)
		this->DrawHoverTooltip(pSurface, mousePos, mousePos);

		return;
	}

	if (this->PlayerRows.empty())
		return;

	int const screenWidth = DSurface::ViewBounds->Width;
	int const screenHeight = DSurface::ViewBounds->Height;

	int const cameoWidth = 60;
	int const cameoHeight = 48;
	int const padding = 4;
	int const sectionGap = 3;
	int const rowHeight = cameoHeight + padding * 2;
	int const startX = 20;

	// Capture mouse clicks using GetKeyState
	bool const isLButtonDown = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
	static bool wasLButtonDown = false;
	bool const isLeftClick = isLButtonDown && !wasLButtonDown;
	wasLButtonDown = isLButtonDown;

	// Max production items across all rows, so the structure sections align across players
	size_t maxProdCount = 0;
	for (auto const& row : this->PlayerRows)
	{
		maxProdCount = std::max(maxProdCount, row.ProductionItems.size());
	}

	int const infoBoxWidth = 120;
	int const playerColorBarWidth = 5;
	int const teamColorBarWidth = 10;
	int const rightMargin = 20;

	int const structStartX = startX + infoBoxWidth + playerColorBarWidth + sectionGap;

	int const prodSectionWidth = (maxProdCount > 0) ? (static_cast<int>(maxProdCount) * (cameoWidth + padding) + padding) : 0;
	int const prodEndX = screenWidth - rightMargin;
	int const prodStartX = (maxProdCount > 0) ? (prodEndX - prodSectionWidth) : prodEndX;

	int const maxStructEndX = (maxProdCount > 0) ? (prodStartX - playerColorBarWidth - sectionGap) : prodEndX;
	int const availableStructWidth = std::max(0, maxStructEndX - structStartX - 30); // Reserve room for scroll buttons

	// Build Tab Buttons positioned top-center, directly above Section 2 (middle objects panel)
	std::vector<std::pair<ObserverFilterCategory, std::wstring>> const tabDefs {
		{ ObserverFilterCategory::Defenses, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_DEFENSES", L"Defenses") },
		{ ObserverFilterCategory::Structures, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_STRUCTURES", L"Structures") },
		{ ObserverFilterCategory::AllStructures, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_ALL_STRUCTURES", L"All Structures") },
		{ ObserverFilterCategory::Infantry, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_INFANTRY", L"Infantry") },
		{ ObserverFilterCategory::Vehicles, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_VEHICLES", L"Vehicles") },
		{ ObserverFilterCategory::Naval, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_NAVAL", L"Naval") },
		{ ObserverFilterCategory::Aircraft, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_AIRCRAFT", L"Aircraft") },
		{ ObserverFilterCategory::AllUnits, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_ALL_UNITS", L"All Units") },
		{ ObserverFilterCategory::Superweapons, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_SUPERWEAPONS", L"Superweapons") },
		{ ObserverFilterCategory::Everything, GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TAB_EVERYTHING", L"Everything") }
	};

	this->TabButtons.clear();

	int const tabHeight = 20;
	int const tabPadding = 8;
	int const tabGap = 3;
	int const tabRowGap = 3;

	// Measure individual tab widths
	std::vector<int> tabWidths;
	tabWidths.reserve(tabDefs.size());
	for (auto const& [category, label] : tabDefs)
	{
		int width = 0;
		int height = 0;

		if (BitFont::Instance())
			BitFont::Instance->GetTextDimension(label.c_str(), &width, &height, 200);
		else
			width = static_cast<int>(label.length() * 8);

		tabWidths.push_back(width + tabPadding * 2);
	}

	int const maxTabsAvailableWidth = std::max(100, maxStructEndX - structStartX);

	// Automatically wrap tabs into multiple rows if the resolution is low or space is constrained
	std::vector<std::vector<size_t>> tabRows;
	std::vector<size_t> currentLine;
	int currentLineWidth = 0;

	for (size_t i = 0; i < tabDefs.size(); ++i)
	{
		int const neededWidth = currentLine.empty() ? tabWidths[i] : (currentLineWidth + tabGap + tabWidths[i]);

		if (!currentLine.empty() && neededWidth > maxTabsAvailableWidth)
		{
			tabRows.push_back(currentLine);
			currentLine.clear();
			currentLine.push_back(i);
			currentLineWidth = tabWidths[i];
		}
		else
		{
			currentLine.push_back(i);
			currentLineWidth = neededWidth;
		}
	}

	if (!currentLine.empty())
		tabRows.push_back(currentLine);

	int const totalRows = static_cast<int>(this->PlayerRows.size());
	int const tabRowsCount = static_cast<int>(tabRows.size());
	int const searchH = 24;
	int const inspectBtnW = 54;
	int const clearW = 24;
	int const searchW = std::max(80, (maxStructEndX - structStartX) - inspectBtnW - clearW - 6);
	int const inspectX = structStartX;
	int const searchX = inspectX + inspectBtnW + 3;

	int const maxAllowedRows = 8;
	int const maxVisibleRows = std::min(maxAllowedRows, totalRows);
	bool const needsScroll = totalRows > maxAllowedRows;

	if (needsScroll)
	{
		this->MaxVerticalScrollOffset = std::max(0, totalRows - maxVisibleRows);
		this->VerticalScrollOffset = std::clamp(this->VerticalScrollOffset, 0, this->MaxVerticalScrollOffset);
	}
	else
	{
		this->VerticalScrollOffset = 0;
		this->MaxVerticalScrollOffset = 0;
	}

	// The panel is ALWAYS bottom-anchored based on the visible rows (fixed position, max 8 rows!)
	int const startY = screenHeight - (maxVisibleRows * rowHeight) - 18;

	// Scroll buttons centered directly above the first player/house info box
	int const scrollBtnW = 40;
	int const scrollBtnH = 20;
	int const scrollBtnGap = 4;
	int const totalScrollBtnsW = (scrollBtnW * 2) + scrollBtnGap;
	int const centerBtnX = std::max(startX, startX + ((infoBoxWidth + playerColorBarWidth) - totalScrollBtnsW) / 2);
	int const scrollBtnY = needsScroll ? (startY - scrollBtnH - 3) : startY;

	int const tabsBaseY = (needsScroll ? scrollBtnY : startY) - 4;
	int const highestTabY = tabsBaseY - (tabRowsCount * tabHeight) - ((tabRowsCount - 1) * tabRowGap);
	int const searchY = highestTabY - searchH - 4;

	if (needsScroll)
	{
		this->VertScrollUpBtnRect = RectangleStruct { centerBtnX, scrollBtnY, scrollBtnW, scrollBtnH };
		this->VertScrollDownBtnRect = RectangleStruct { centerBtnX + scrollBtnW + scrollBtnGap, scrollBtnY, scrollBtnW, scrollBtnH };

		this->IsHoveringVertScrollUp = ObserverUIHelpers::HitTest(this->VertScrollUpBtnRect, mousePos);
		this->IsHoveringVertScrollDown = ObserverUIHelpers::HitTest(this->VertScrollDownBtnRect, mousePos);

		// Render Scroll Up / Down Buttons (centered above the first player/house)
		ObserverUIHelpers::DrawPanel(pSurface, this->VertScrollUpBtnRect,
			this->IsHoveringVertScrollUp ? ObserverRGB::Teal : ObserverRGB::Panel, 95,
			this->IsHoveringVertScrollUp ? UIColors().Highlight : UIColors().BorderNeutral);

		ObserverUIHelpers::DrawPanel(pSurface, this->VertScrollDownBtnRect,
			this->IsHoveringVertScrollDown ? ObserverRGB::Teal : ObserverRGB::Panel, 95,
			this->IsHoveringVertScrollDown ? UIColors().Highlight : UIColors().BorderNeutral);

		if (BitFont::Instance() && BitText::Instance())
		{
			ObserverUIHelpers::DrawShadowText(pSurface, this->VertScrollUpBtnRect, L"^",
				Point2D { this->VertScrollUpBtnRect.X + 16, this->VertScrollUpBtnRect.Y + 2 },
				(this->VerticalScrollOffset > 0) ? UIColors().White : UIColors().Disabled);

			ObserverUIHelpers::DrawShadowText(pSurface, this->VertScrollDownBtnRect, L"v",
				Point2D { this->VertScrollDownBtnRect.X + 16, this->VertScrollDownBtnRect.Y + 2 },
				(this->VerticalScrollOffset < this->MaxVerticalScrollOffset) ? UIColors().White : UIColors().Disabled);
		}
	}
	else
	{
		this->VertScrollUpBtnRect = RectangleStruct { 0, 0, 0, 0 };
		this->VertScrollDownBtnRect = RectangleStruct { 0, 0, 0, 0 };
	}

	int const visibleStart = std::clamp(this->VerticalScrollOffset, 0, std::max(0, totalRows - 1));
	int const visibleEnd = std::min(totalRows, visibleStart + maxVisibleRows);

	// Calculate team Y extents ONLY for visible player rows!
	std::map<int, std::pair<int, int>> teamYExtents;
	for (int rowIndex = visibleStart; rowIndex < visibleEnd; ++rowIndex)
	{
		auto const& row = this->PlayerRows[rowIndex];
		if (row.TeamID < 0 || row.TeamMemberCount < 2)
			continue;

		int const rowY = startY + (rowIndex - visibleStart) * rowHeight;
		auto const itExtent = teamYExtents.find(row.TeamID);

		if (itExtent == teamYExtents.end())
			teamYExtents[row.TeamID] = { rowY, rowY + rowHeight };
		else
			itExtent->second.second = rowY + rowHeight;
	}

	this->InspectBtnRect = RectangleStruct { inspectX, searchY, inspectBtnW, searchH };
	this->SearchBoxRect = RectangleStruct { searchX, searchY, searchW, searchH };
	this->ClearBtnRect = RectangleStruct { searchX + searchW + 3, searchY, clearW, searchH };

	this->IsHoveringInspectBtn = ObserverUIHelpers::HitTest(this->InspectBtnRect, mousePos);
	this->IsHoveringClearBtn = ObserverUIHelpers::HitTest(this->ClearBtnRect, mousePos);

	// Build Tab Buttons
	for (size_t r = 0; r < tabRows.size(); ++r)
	{
		int const rowY = needsScroll
			? (highestTabY + (static_cast<int>(r) * (tabHeight + tabRowGap)))
			: (tabsBaseY - (static_cast<int>(r + 1) * tabHeight) - (static_cast<int>(r) * tabRowGap));

		auto const& lineIndices = tabRows[r];
		int lineTotalW = static_cast<int>(lineIndices.size() - 1) * tabGap;
		for (size_t const index : lineIndices)
		{
			lineTotalW += tabWidths[index];
		}

		int lineStartX = structStartX;
		if (lineStartX + lineTotalW > maxStructEndX)
			lineStartX = std::max(structStartX, maxStructEndX - lineTotalW);

		int currentX = lineStartX;
		for (size_t const index : lineIndices)
		{
			ObserverTabButton button;
			button.Category = tabDefs[index].first;
			button.Label = tabDefs[index].second;
			button.Rect = RectangleStruct { currentX, rowY, tabWidths[index], tabHeight };
			button.IsHovered = ObserverUIHelpers::HitTest(button.Rect, mousePos);
			this->TabButtons.push_back(button);

			if (isLeftClick && button.IsHovered)
			{
				this->ActiveFilterTab = button.Category;
				this->CollectPlayerData(true);
			}

			currentX += tabWidths[index] + tabGap;
		}
	}

	// Render Category Filter Tab Buttons
	for (auto const& button : this->TabButtons)
	{
		bool const isTabActive = button.Category == this->ActiveFilterTab;

		// Border: Neon Cyan when active, Soft White when hovered, Dark Gray otherwise
		COLORREF borderColor = UIColors().BorderPanel;
		COLORREF textColor = UIColors().TextDim;

		if (isTabActive)
		{
			borderColor = UIColors().Highlight;
			textColor = UIColors().White;
		}
		else if (button.IsHovered)
		{
			borderColor = UIColors().TextMuted;
			textColor = UIColors().TextBright;
		}

		ObserverUIHelpers::DrawPanel(pSurface, button.Rect, ObserverRGB::Black, isTabActive ? 95 : 60, borderColor);
		ObserverUIHelpers::DrawCenteredText(pSurface, button.Rect, button.Label.c_str(), textColor);
	}

	// Render Inspect Selected Button [-> [] <-]
	ObserverUIHelpers::DrawPanel(pSurface, this->InspectBtnRect,
		this->IsHoveringInspectBtn ? ObserverRGB::Teal : ObserverRGB::Panel, 80,
		this->IsHoveringInspectBtn ? UIColors().Highlight : UIColors().BorderIdle);

	if (BitFont::Instance())
	{
		int textWidth = 0;
		int textHeight = 0;
		BitFont::Instance->GetTextDimension(L"-> [] <-", &textWidth, &textHeight, inspectBtnW);

		ObserverUIHelpers::DrawShadowText(pSurface, this->InspectBtnRect, L"-> [] <-",
			Point2D { this->InspectBtnRect.X + (inspectBtnW - textWidth) / 2, this->InspectBtnRect.Y + 3 },
			this->IsHoveringInspectBtn ? UIColors().Highlight : UIColors().TextBright);
	}

	// Render Search Input Box
	ObserverUIHelpers::DrawPanel(pSurface, this->SearchBoxRect, ObserverRGB::PanelDark,
		this->IsSearchInputFocused ? 90 : 60,
		this->IsSearchInputFocused ? UIColors().Highlight : UIColors().BorderPanel);

	if (BitFont::Instance())
	{
		RectangleStruct const searchTextRect {
			this->SearchBoxRect.X + 2,
			this->SearchBoxRect.Y + 2,
			this->SearchBoxRect.Width - 4,
			this->SearchBoxRect.Height - 4
		};

		Point2D const textPoint { this->SearchBoxRect.X + 8, this->SearchBoxRect.Y + 4 };

		if (this->SearchFilterText.empty() && !this->IsSearchInputFocused)
		{
			ObserverUIHelpers::DrawShadowText(pSurface, searchTextRect,
				GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_FILTER_PLACEHOLDER", L"Filter..."),
				textPoint, UIColors().TextFaint);
		}
		else
		{
			std::wstring displayText = this->SearchFilterText;
			if (this->IsSearchInputFocused && (Unsorted::CurrentFrame() / 15) % 2 == 0)
				displayText += L"|";

			// Keep the visible text within the box width (auto-scroll the tail of long queries)
			size_t const maxVisibleChars = static_cast<size_t>(std::max(4, (this->SearchBoxRect.Width - 18) / 8));
			if (displayText.length() > maxVisibleChars)
				displayText = displayText.substr(displayText.length() - maxVisibleChars);

			ObserverUIHelpers::DrawShadowText(pSurface, searchTextRect, displayText.c_str(), textPoint, UIColors().White);
		}
	}

	// Render Clear Button [X]
	ObserverUIHelpers::DrawPanel(pSurface, this->ClearBtnRect,
		this->IsHoveringClearBtn ? ObserverRGB::Danger : ObserverRGB::Panel, 80,
		UIColors().BorderIdle);

	ObserverUIHelpers::DrawShadowText(pSurface, this->ClearBtnRect, L"X",
		Point2D { this->ClearBtnRect.X + 8, this->ClearBtnRect.Y + 3 }, UIColors().White);

	// Render team alliance vertical bars attached to the left edge of Section 1
	// (ONLY for alliances of 2+ players)
	for (auto const& [teamID, extent] : teamYExtents)
	{
		auto const itFirstRow = std::find_if(this->PlayerRows.begin(), this->PlayerRows.end(), [teamID](const ObserverPlayerRow& row)
 {
	 return row.TeamID == teamID;
		});

		if (itFirstRow == this->PlayerRows.end())
			continue;

		RectangleStruct teamLineRect { startX - teamColorBarWidth, extent.first, teamColorBarWidth, extent.second - extent.first };
		pSurface->Fill_Rect(teamLineRect, itFirstRow->TeamColorValue);
	}

	ColorStruct const bgPanelColor = ObserverRGB::Black;

	for (int rowIndex = visibleStart; rowIndex < visibleEnd; ++rowIndex)
	{
		auto& row = this->PlayerRows[rowIndex];
		int const currentY = startY + (rowIndex - visibleStart) * rowHeight;
		COLORREF const rowColor = row.PlayerColorValue;

		// Section 1: Player Info Box + Player Color Bar (translucent 30% background)
		row.InfoRect = RectangleStruct { startX, currentY, infoBoxWidth, rowHeight };
		ObserverUIHelpers::DrawPanel(pSurface, row.InfoRect, bgPanelColor, 75, UIColors().BorderPanel);

		RectangleStruct playerColorBarRect { startX + infoBoxWidth, currentY, playerColorBarWidth, rowHeight };
		pSurface->Fill_Rect(playerColorBarRect, rowColor);

		RectangleStruct const infoHoverRect {
			startX - teamColorBarWidth,
			row.InfoRect.Y,
			infoBoxWidth + playerColorBarWidth + teamColorBarWidth,
			row.InfoRect.Height
		};

		if (ObserverUIHelpers::HitTest(infoHoverRect, mousePos))
		{
			this->pHoveredPlayer = row.pHouse;
			this->HasHoveredPlayer = true;
			this->HoveredMousePos = mousePos;
			pSurface->Draw_Rect(row.InfoRect, rowColor);
		}

		// Render Player Name (Line 1) and Country Name (Line 2)
		if (BitFont::Instance() && BitText::Instance())
		{
			ObserverUIHelpers::DrawShadowText(pSurface, row.InfoRect, row.PlayerName.c_str(),
				Point2D { row.InfoRect.X + 6, row.InfoRect.Y + 6 }, UIColors().White);

			ObserverUIHelpers::DrawShadowText(pSurface, row.InfoRect, row.CountryName.c_str(),
				Point2D { row.InfoRect.X + 6, row.InfoRect.Y + 24 }, UIColors().TextMuted);
		}

		// Section 3: Production Items (anchored to the far right) + left Player Color Bar
		if (prodSectionWidth > 0)
		{
			RectangleStruct prodColorBarRect { prodStartX - playerColorBarWidth, currentY, playerColorBarWidth, rowHeight };
			pSurface->Fill_Rect(prodColorBarRect, rowColor);

			row.ProdPanelRect = RectangleStruct { prodStartX, currentY, prodSectionWidth, rowHeight };
			ObserverUIHelpers::DrawPanel(pSurface, row.ProdPanelRect, bgPanelColor, 75, UIColors().BorderPanel);

			int currentProdX = prodEndX - padding - cameoWidth;
			for (auto& item : row.ProductionItems)
			{
				item.DisplayRect = RectangleStruct { currentProdX, currentY + padding, cameoWidth, cameoHeight };
				currentProdX -= (cameoWidth + padding);

				// Strictly render ONLY if the item box fits completely inside ProdPanelRect
				if (item.DisplayRect.X < row.ProdPanelRect.X
					|| (item.DisplayRect.X + item.DisplayRect.Width) >(row.ProdPanelRect.X + row.ProdPanelRect.Width))
				{
					continue;
				}

				bool const isHovered = ObserverUIHelpers::HitTest(item.DisplayRect, mousePos);
				if (isHovered)
				{
					this->HoveredItem = item;
					this->HasHoveredItem = true;
					this->HoveredMousePos = mousePos;
				}

				this->DrawCameoItem(pSurface, item, isHovered, row.ProdPanelRect, rowColor);
			}
		}

		// Section 2: Filtered Objects (middle section, expands up to the production panel)
		int const totalStructWidth = row.StructureItems.empty()
			? 0
			: (static_cast<int>(row.StructureItems.size()) * (cameoWidth + padding) + padding);

		row.MaxScrollOffset = std::max(0, totalStructWidth - availableStructWidth);
		row.ScrollOffset = std::clamp(row.ScrollOffset, 0, row.MaxScrollOffset);

		int const structPanelWidth = std::max(0, std::min(totalStructWidth, availableStructWidth));
		row.StructPanelRect = RectangleStruct { structStartX, currentY, structPanelWidth, rowHeight };

		if (structPanelWidth > 0)
		{
			ObserverUIHelpers::DrawPanel(pSurface, row.StructPanelRect, bgPanelColor, 75, UIColors().BorderPanel);

			int currentStructX = structStartX + padding - row.ScrollOffset;
			for (auto& item : row.StructureItems)
			{
				item.DisplayRect = RectangleStruct { currentStructX, currentY + padding, cameoWidth, cameoHeight };
				currentStructX += cameoWidth + padding;

				// Strictly render ONLY if the item box fits completely inside StructPanelRect
				// (so it never overlaps the scroll buttons)
				if (item.DisplayRect.X < row.StructPanelRect.X
					|| (item.DisplayRect.X + item.DisplayRect.Width) >(row.StructPanelRect.X + row.StructPanelRect.Width))
				{
					continue;
				}

				bool const isHovered = ObserverUIHelpers::HitTest(item.DisplayRect, mousePos);
				if (isHovered)
				{
					this->HoveredItem = item;
					this->HasHoveredItem = true;
					this->HoveredMousePos = mousePos;
				}

				this->DrawCameoItem(pSurface, item, isHovered, row.StructPanelRect, rowColor);
			}
		}

		// Per-row scroll buttons, if the structure list exceeds the available width
		if (row.MaxScrollOffset > 0)
		{
			int const buttonX = structStartX + structPanelWidth + 4;
			row.ScrollLeftBtnRect = RectangleStruct { buttonX, currentY + padding, 20, cameoHeight / 2 - 1 };
			row.ScrollRightBtnRect = RectangleStruct { buttonX, currentY + padding + cameoHeight / 2 + 1, 20, cameoHeight / 2 - 1 };

			row.IsHoveringLeftScroll = ObserverUIHelpers::HitTest(row.ScrollLeftBtnRect, mousePos);
			row.IsHoveringRightScroll = ObserverUIHelpers::HitTest(row.ScrollRightBtnRect, mousePos);

			ObserverUIHelpers::DrawPanel(pSurface, row.ScrollLeftBtnRect,
				row.IsHoveringLeftScroll ? row.PlayerColor : ObserverRGB::Muted, 200, UIColors().White);

			ObserverUIHelpers::DrawPanel(pSurface, row.ScrollRightBtnRect,
				row.IsHoveringRightScroll ? row.PlayerColor : ObserverRGB::Muted, 200, UIColors().White);

			if (BitFont::Instance())
			{
				Point2D leftPoint { row.ScrollLeftBtnRect.X + 5, row.ScrollLeftBtnRect.Y + 2 };
				Point2D rightPoint { row.ScrollRightBtnRect.X + 5, row.ScrollRightBtnRect.Y + 2 };
				pSurface->DSurfaceDrawText(L"<", DSurface::ViewBounds.operator->(), &leftPoint, UIColors().Black, 0, TextPrintType::Point8);
				pSurface->DSurfaceDrawText(L">", DSurface::ViewBounds.operator->(), &rightPoint, UIColors().Black, 0, TextPrintType::Point8);
			}
		}
	}

	// Trigger the click action after all layout geometry rects have been updated
	if (isLeftClick)
		this->HandleMouseClick(mousePos, false);

	// Render tooltip for inspect button, player info or cameo item
	this->DrawHoverTooltip(pSurface, mousePos, this->HoveredMousePos);

	// Render Floating Windows on top of the UI
	this->RenderFloatingWindows(pSurface);
	this->RenderFloatingUnitWindows(pSurface);
}

// Single dispatch for the three mutually exclusive hover tooltips. Used by both the
// Minimal and the Full render paths, which only differ in the anchor point they pass.
void ObserverUIClass::DrawHoverTooltip(DSurface* pSurface, Point2D cursorPos, Point2D tooltipPos)
{
	if (this->IsHoveringInspectBtn)
	{
		ObserverUIHelpers::DrawSimpleTooltip(pSurface,
			GeneralUtils::LoadStringUnlessMissing("TXT_OBSERVER_TOOLTIP_INSPECT", L"Inspect Selected Object (Create Card)"),
			cursorPos);
		return;
	}

	if (this->HasHoveredPlayer && this->pHoveredPlayer)
	{
		this->DrawPlayerTooltip(pSurface, this->pHoveredPlayer, tooltipPos);
		return;
	}

	// DIFF: the Minimal path used to require HoveredItem.pType, which would have dropped
	// superweapon tooltips. DrawTooltip handles both cases, so both paths now share it.
	if (this->HasHoveredItem && (this->HoveredItem.pType || this->HoveredItem.IsSuperweapon))
		this->DrawTooltip(pSurface, this->HoveredItem, tooltipPos);
}

void ObserverUIClass::RenderFloatingUnitWindows(DSurface* pSurface)
{
	if (!pSurface || !BitFont::Instance() || !BitText::Instance())
		return;

	if (!IsActive() && !Phobos::Config::DevelopmentCommands)
		return;

	Point2D const mousePos = ObserverUIHelpers::MousePosition();

	int const maxCardWidth = 360;
	int const cameoBoxW = 60;
	int const cameoBoxH = 48;
	int const boxPadding = 8;
	int const contentLeftMargin = cameoBoxW + 16;

	for (auto& win : this->FloatingUnitWindows)
	{
		if (!win.pOwner || (!win.pType && !win.IsSuperweapon && !win.pSuperType))
			continue;

		COLORREF const playerColor = ObserverUIHelpers::GetRowColorValue(this->PlayerRows, win.pOwner);

		ObserverTextBlock block(maxCardWidth);
		ObserverUnitCardContext const context = ObserverUnitCard::Build(block, win, this->PlayerRows);

		// Calculate total layout width & height (20px reserved for the close button)
		int const boxWidth = std::max(260, block.Width + contentLeftMargin + boxPadding + 20);
		int const boxHeight = std::max(cameoBoxH + boxPadding * 2 + 4, block.Height + boxPadding * 2 + 4);

		win.WindowRect = RectangleStruct { win.Position.X, win.Position.Y, boxWidth, boxHeight };
		win.CloseBtnRect = RectangleStruct { win.Position.X + boxWidth - 20, win.Position.Y + 4, 16, 16 };
		win.CameoClickRect = RectangleStruct { win.Position.X + boxPadding, win.Position.Y + boxPadding, cameoBoxW, cameoBoxH };

		// Translucent panel background with an outer border in the player's color
		ObserverUIHelpers::DrawPanel(pSurface, win.WindowRect, ObserverRGB::Black, 75,
			playerColor);

		// Top-left cameo
		ObserverCameoItem cameoItem;
		if (win.IsSuperweapon || win.pSuperType)
		{
			cameoItem.pSuperType = win.pSuperType;
			cameoItem.pSuper = win.pSuper;
			cameoItem.IsSuperweapon = true;
		}
		else if (context.IsProductionView)
		{
			bool const isProducing = context.Factory && context.Factory->Object && context.CurrentProduct;

			// No cameo while the factory is idle!
			cameoItem.pType = isProducing ? context.CurrentProduct : nullptr;
			cameoItem.IsProduction = isProducing;
			cameoItem.ProgressPercent = isProducing ? ObserverUIHelpers::GetFactoryProgressPercent(context.Factory) : -1;
		}
		else
		{
			cameoItem.pType = context.CameoType;
		}

		cameoItem.pOwner = win.pOwner;
		cameoItem.Count = 1;
		cameoItem.DisplayRect = win.CameoClickRect;

		this->DrawCameoItem(pSurface, cameoItem, ObserverUIHelpers::HitTest(win.CameoClickRect, mousePos), win.CameoClickRect, playerColor);

		ObserverUIHelpers::DrawCloseButton(pSurface, win.CloseBtnRect, ObserverUIHelpers::HitTest(win.CloseBtnRect, mousePos));

		// Render text lines inside the window (offset by the cameo box width)
		block.Render(pSurface, win.WindowRect, win.WindowRect.X + contentLeftMargin, win.WindowRect.Y + boxPadding);
	}
}

void ObserverUIClass::RenderFloatingWindows(DSurface* pSurface)
{
	if (!pSurface || !BitFont::Instance() || !BitText::Instance())
		return;

	if (!IsActive() && !Phobos::Config::DevelopmentCommands)
		return;

	Point2D const mousePos = ObserverUIHelpers::MousePosition();

	int const maxCardWidth = 350;
	int const boxPadding = 8;

	ObserverHouseSummaryOptions options;
	options.DebugFormatting = Phobos::Config::DevelopmentCommands;
	options.ShowDefeatedStatus = true;
	options.HideZeroFactoryCounts = true;
	options.PlayerNumberNeedsMultiplayer = true;

	for (auto& win : this->FloatingWindows)
	{
		if (!win.pHouse || !win.pHouse->Type)
			continue;

		COLORREF const playerColor = ObserverUIHelpers::GetRowColorValue(this->PlayerRows, win.pHouse);

		ObserverTextBlock block(maxCardWidth);
		ObserverHouseSummary::Build(block, win.pHouse, this->PlayerRows, options);

		// 20px extra space for the top-right [X] close button
		int const boxWidth = block.Width + boxPadding * 2 + 20;
		int const boxHeight = block.Height + boxPadding * 2 + 4;

		win.WindowRect = RectangleStruct { win.Position.X, win.Position.Y, boxWidth, boxHeight };
		win.CloseBtnRect = RectangleStruct { win.Position.X + boxWidth - 20, win.Position.Y + 4, 16, 16 };

		ObserverUIHelpers::DrawPanel(pSurface, win.WindowRect, ObserverRGB::Black, 75,
			playerColor);

		ObserverUIHelpers::DrawCloseButton(pSurface, win.CloseBtnRect, ObserverUIHelpers::HitTest(win.CloseBtnRect, mousePos));

		block.Render(pSurface, win.WindowRect, win.WindowRect.X + boxPadding, win.WindowRect.Y + boxPadding);
	}
}

void ObserverUIClass::DrawCameoItem(DSurface* pSurface, const ObserverCameoItem& item, bool isHovered, const RectangleStruct& clipRect, COLORREF playerColor)
{
	if (!pSurface)
		return;

	// Intersection of the item display area and the owning section panel
	RectangleStruct drawRect;
	if (!ObserverUIHelpers::IntersectRects(item.DisplayRect, clipRect, drawRect))
		return;

	BSurface* pPCXSurface = nullptr;
	SHPCaches* pFileSHP = nullptr;

	if (item.IsSuperweapon && item.pSuperType)
	{
		if (auto const pSWExt = SWTypeExtContainer::Instance.Find(item.pSuperType))
		{
			if (pSWExt->SidebarPCX.Exists())
				pPCXSurface = pSWExt->SidebarPCX.GetSurface();
		}

		const char* const imageFile = item.pSuperType->SidebarImageFile;
		if (!pPCXSurface)
		{
			pPCXSurface = ObserverUIHelpers::LoadPCXSurface(imageFile);

			if (!pPCXSurface)
				pFileSHP = ObserverUIHelpers::LoadSHPFile(imageFile);
		}

		if (!pPCXSurface && !pFileSHP)
			pFileSHP = item.pSuperType->SidebarImage;
	}
	else if (item.pType)
	{
		// 1. TechnoTypeExt PCX cameos
		if (auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(item.pType))
		{
			if (pTypeExt->CameoPCX.Exists())
				pPCXSurface = pTypeExt->CameoPCX.GetSurface();
			else if (pTypeExt->AltCameoPCX.Exists())
				pPCXSurface = pTypeExt->AltCameoPCX.GetSurface();
		}

		// 2. CameoFile PCX
		if (!pPCXSurface)
			pPCXSurface = ObserverUIHelpers::LoadPCXSurface(item.pType->CameoFile);

		// 3. ID PCX variants (e.g. TSGACNSTicon.pcx, TSGACNST.pcx, GDIPOWRicon.pcx)
		if (!pPCXSurface)
			pPCXSurface = ObserverUIHelpers::LoadPCXSurfaceForID(item.pType->ID);

		// 4. Fall back to SHP cameos, ONLY if the SHP is not the XXICON.SHP placeholder
		if (!pPCXSurface)
		{
			SHPCaches* pCandidateSHP = item.pType->GetCameo();
			if (!pCandidateSHP)
				pCandidateSHP = item.pType->Cameo;
			if (!pCandidateSHP)
				pCandidateSHP = item.pType->AltCameo;

			if (pCandidateSHP && !ObserverUIHelpers::IsPlaceholderCameo(pCandidateSHP))
				pFileSHP = pCandidateSHP;

			if (!pFileSHP)
				pFileSHP = ObserverUIHelpers::LoadSHPFile(item.pType->CameoFile);
		}
	}

	pPCXSurface = ObserverUIHelpers::ValidSurface(pPCXSurface);

	if (!pFileSHP && !pPCXSurface)
		pFileSHP = ObserverUIHelpers::LoadPlaceholderCameo();

	bool const painted = ObserverUIHelpers::DrawCameoImage(pSurface, drawRect, pPCXSurface, pFileSHP, FileSystem::CAMEO_PAL);

	if (!painted)
	{
		// Draw a missing-cameo outline and placeholder text
		ObserverUIHelpers::DrawPanel(pSurface, drawRect, ObserverRGB::Black, 75, UIColors().BorderNeutral);

		if (BitFont::Instance())
		{
			RectangleStruct textClipRect = clipRect;
			Point2D textPoint { drawRect.X + 4, drawRect.Y + drawRect.Height / 2 - 4 };
			pSurface->DSurfaceDrawText(L"NO CAMEO", &textClipRect, &textPoint, UIColors().Danger, 0, TextPrintType::Point6);
		}
	}

	// Draw hover outline matching the player's color
	if (isHovered)
		pSurface->Draw_Rect(drawRect, playerColor);

	// Superweapon readiness percentage overlay (0% -> 100%)
	if (item.IsSuperweapon && item.pSuper)
	{
		int const totalFrames = item.pSuper->GetRechargeTime();
		int const framesLeft = item.pSuper->RechargeTimer.GetTimeLeft();
		int const percentReady = (totalFrames > 0 && framesLeft > 0)
			? std::clamp(((totalFrames - framesLeft) * 100) / totalFrames, 0, 100)
			: 100;

		ObserverUIHelpers::DrawCameoOverlayText(pSurface, item.DisplayRect, clipRect,
			std::to_wstring(percentReady) + L"%",
			(percentReady == 100) ? UIColors().Good : UIColors().White);
	}
	// Production percentage overlay
	else if (item.IsProduction && item.ProgressPercent >= 0)
	{
		ObserverUIHelpers::DrawCameoOverlayText(pSurface, item.DisplayRect, clipRect,
			std::to_wstring(item.ProgressPercent) + L"%",
			UIColors().Good); // Neon Green
	}

	// Instance count overlay
	if (!item.IsProduction && item.Count > 1)
	{
		ObserverUIHelpers::DrawCameoOverlayText(pSurface, item.DisplayRect, clipRect,
			std::to_wstring(item.Count),
			UIColors().White);
	}
}

void ObserverUIClass::DrawPlayerTooltip(DSurface* pSurface, HouseClass* pHouse, Point2D mousePos)
{
	if (!pSurface || !pHouse || !pHouse->Type || !BitFont::Instance() || !BitText::Instance())
		return;

	ObserverHouseSummaryOptions options;
	options.DebugFormatting = false;
	options.ShowDefeatedStatus = false;
	options.HideZeroFactoryCounts = false;
	options.PlayerNumberNeedsMultiplayer = false;

	ObserverTextBlock block(240);
	ObserverHouseSummary::Build(block, pHouse, this->PlayerRows, options);

	// Tooltip border outline matches the player's color
	ObserverTooltipBox::Render(pSurface, block, mousePos, ObserverUIHelpers::GetRowColor(this->PlayerRows, pHouse));
}

void ObserverUIClass::DrawTooltip(DSurface* pSurface, const ObserverCameoItem& item, Point2D mousePos)
{
	if (!pSurface || (!item.pType && !item.pSuperType) || !BitFont::Instance() || !BitText::Instance())
		return;

	using Colors = ObserverHouseSummary;

	bool const isDebugKeysEnabled = Phobos::Config::DevelopmentCommands;

	ObserverTextBlock block(220);

	if (item.IsSuperweapon && item.pSuperType)
	{
		block.Add(ObserverUIHelpers::FormatObjectNameWithDebug(0, item.pSuperType->get_ID(), item.pSuperType->UIName, isDebugKeysEnabled), Colors::White());

		int const totalFrames = item.pSuper ? item.pSuper->GetRechargeTime() : item.pSuperType->RechargeTime;
		int const framesLeft = item.pSuper ? item.pSuper->RechargeTimer.GetTimeLeft() : 0;

		block.Add(L"Cooldown: " + ObserverUIHelpers::FormatTimerPair(framesLeft, totalFrames),
			(framesLeft == 0) ? Colors::Good() : Colors::Accent());

		if (item.pSuperType->IsPowered && item.pOwner && item.pOwner->PowerOutput < item.pOwner->PowerDrain)
			block.Add(L"Power: Low Power", Colors::Bad());
	}
	else if (item.pType)
	{
		// Top line: Structure / Techno Name
		block.Add(ObserverUIHelpers::FormatObjectNameWithDebug(0, item.pType->get_ID(), item.pType->UIName, isDebugKeysEnabled), Colors::White());

		if (item.IsProduction)
		{
			std::wostringstream costOss;
			costOss << L"Cost: $" << item.pType->GetActualCost(item.pOwner ? item.pOwner : HouseClass::CurrentPlayer);
			block.Add(costOss.str(), Colors::Label());
		}
		else if (!item.Buildings.empty())
		{
			// Individual stats (HP, Shield, Veterancy) ONLY make sense for a single instance
			if (item.Buildings.size() == 1 && item.Buildings.front())
			{
				auto const pBuilding = item.Buildings.front();

				std::wostringstream hpOss;
				hpOss << L"HP: " << pBuilding->Health << L"/" << item.pType->Strength;
				block.Add(hpOss.str(), (pBuilding->Health < item.pType->Strength / 4) ? Colors::SoftBad() : Colors::Label());

				// Shield Status Line
				auto const pExt = TechnoExtContainer::Instance.Find(pBuilding);
				if (pExt && pExt->ShieldEntity && pExt->ShieldEntity->IsAvailable())
				{
					std::wostringstream shieldOss;
					shieldOss << L"Shield: " << pExt->ShieldEntity->GetHP() << L"/" << pExt->ShieldEntity->GetType()->Strength.Get();
					block.Add(shieldOss.str(), Colors::Label());
				}

				// Experience / Veterancy Line
				if (item.pType->Trainable)
				{
					int const vetPercent = static_cast<int>((pBuilding->Veterancy.Veterancy / 2.0f) * 100.0f);
					std::wostringstream vetOss;
					vetOss << L"Veterancy: " << std::clamp(vetPercent, 0, 100) << L"%";
					block.Add(vetOss.str(), Colors::Label());
				}
			}

			// Low Power warning line (applies regardless of the instance count)
			if (item.pType->WhatAmI() == AbstractType::BuildingType)
			{
				auto const pBuildingType = static_cast<BuildingTypeClass*>(item.pType);
				if (pBuildingType->Powered && item.pOwner && item.pOwner->HasLowPower())
					block.Add(L"Low Power", Colors::SoftBad());
			}
		}

		// Description line (wrapped at the tooltip width)
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(item.pType);
		if (Phobos::Config::ToolTipDescriptions && pTypeExt && !pTypeExt->UIDescription.Get().empty())
			block.Add(pTypeExt->UIDescription.Get().Text, Colors::Muted());
	}

	// Tooltip border outline matches the player's color
	ObserverTooltipBox::Render(pSurface, block, mousePos, ObserverUIHelpers::GetRowColor(this->PlayerRows, item.pOwner));
}

bool ObserverUIClass::HandleMouseClick(Point2D mousePos, bool isRightClick)
{
	if (!ObserverUIState::IsInteractive(this->DisplayMode, this->HasFloatingWindows()))
		return false;

	if (this->DisplayMode == ObserverUIDisplayMode::Hidden && !this->HasFloatingWindows())
		return false;

	// BUGFIX: one physical click can reach this twice in the same frame - Render() does its
	// own VK_LBUTTON edge detection and the input hook calls this as well. Every duplicate
	// call advanced CycleIndices a second time, so a cameo with 2 instances always centered
	// back on the first one. Swallow repeats of the same click within the same frame.
	{
		static int lastClickFrame = -1;
		static int lastClickX = -1;
		static int lastClickY = -1;
		static bool lastClickWasRight = false;

		int const currentFrame = Unsorted::CurrentFrame();
		bool const isRepeat = currentFrame == lastClickFrame
			&& mousePos.X == lastClickX
			&& mousePos.Y == lastClickY
			&& isRightClick == lastClickWasRight;

		if (isRepeat)
			return true; // already consumed by the first call this frame

		lastClickFrame = currentFrame;
		lastClickX = mousePos.X;
		lastClickY = mousePos.Y;
		lastClickWasRight = isRightClick;
	}

	// 1. Floating Unit Status Windows (reverse order, top-most first)
	for (int i = static_cast<int>(this->FloatingUnitWindows.size()) - 1; i >= 0; --i)
	{
		auto& win = this->FloatingUnitWindows[i];

		if (ObserverUIHelpers::HitTestActive(win.CloseBtnRect, mousePos))
		{
			this->FloatingUnitWindows.erase(this->FloatingUnitWindows.begin() + i);
			return true;
		}

		if (!isRightClick && ObserverUIHelpers::HitTestActive(win.CameoClickRect, mousePos))
		{
			ObserverUIHelpers::DeselectAll();

			if (ObserverUIHelpers::IsTechnoValidAndAlive(win.pTargetTechno))
			{
				ObserverUIHelpers::CenterAndSelect(win.pTargetTechno);
			}
			else if (ObserverUIHelpers::IsBuildingValidAndAlive(win.pTargetBuilding))
			{
				ObserverUIHelpers::CenterAndSelect(win.pTargetBuilding);
			}
			else if (win.pType && win.pOwner)
			{
				TechnoClass* pFound = nullptr;

				for (auto const pTechno : *TechnoClass::Array)
				{
					if (pTechno && pTechno->IsAlive && !pTechno->InLimbo && pTechno->Owner == win.pOwner && pTechno->GetTechnoType() == win.pType)
					{
						pFound = pTechno;
						break;
					}
				}

				if (!pFound)
				{
					for (auto const pBuilding : *BuildingClass::Array)
					{
						if (pBuilding && pBuilding->IsAlive && !pBuilding->InLimbo && pBuilding->Owner == win.pOwner && pBuilding->Type == win.pType)
						{
							pFound = pBuilding;
							break;
						}
					}
				}

				ObserverUIHelpers::CenterAndSelect(pFound);
			}

			return true;
		}

		if (ObserverUIHelpers::HitTestActive(win.WindowRect, mousePos))
		{
			if (!isRightClick)
			{
				// Left click starts dragging & brings the window to the front
				win.IsDragging = true;
				win.DragOffset = Point2D { mousePos.X - win.Position.X, mousePos.Y - win.Position.Y };

				ObserverUIHelpers::BringToFrontIf(this->FloatingUnitWindows, [&win](const ObserverFloatingUnitWindow& other)
 {
	 return &other == &win;
				});
			}

			return true;
		}
	}

	// 2. Floating Player Status Windows (reverse order, top-most first)
	for (int i = static_cast<int>(this->FloatingWindows.size()) - 1; i >= 0; --i)
	{
		auto& win = this->FloatingWindows[i];

		if (ObserverUIHelpers::HitTestActive(win.CloseBtnRect, mousePos))
		{
			this->FloatingWindows.erase(this->FloatingWindows.begin() + i);
			return true;
		}

		if (ObserverUIHelpers::HitTestActive(win.WindowRect, mousePos))
		{
			if (!isRightClick)
			{
				win.IsDragging = true;
				win.DragOffset = Point2D { mousePos.X - win.Position.X, mousePos.Y - win.Position.Y };

				ObserverUIHelpers::BringToFrontIf(this->FloatingWindows, [&win](const ObserverFloatingWindow& other)
 {
	 return &other == &win;
				});
			}

			return true;
		}
	}

	// 3. Inspect Selected Button [-> [] <-]
	if (!isRightClick && ObserverUIHelpers::HitTestActive(this->InspectBtnRect, mousePos))
	{
		this->OpenFloatingWindowForSelectedObject();
		return true;
	}

	// 3b. Vertical player rows scroll buttons (one row per click, with debounce)
	if (!isRightClick && this->MaxVerticalScrollOffset > 0)
	{
		static unsigned long long lastVertScrollTimeMs = 0;
		unsigned long long const nowMs = GetTickCount64();

		bool const isScrollUp = ObserverUIHelpers::HitTestActive(this->VertScrollUpBtnRect, mousePos);
		bool const isScrollDown = !isScrollUp && ObserverUIHelpers::HitTestActive(this->VertScrollDownBtnRect, mousePos);

		if (isScrollUp || isScrollDown)
		{
			if ((nowMs - lastVertScrollTimeMs) >= 150)
			{
				lastVertScrollTimeMs = nowMs;
				this->VerticalScrollOffset = std::clamp(
					this->VerticalScrollOffset + (isScrollUp ? -1 : 1),
					0,
					this->MaxVerticalScrollOffset);
			}

			return true;
		}
	}

	if (this->DisplayMode == ObserverUIDisplayMode::Minimal)
		return false;

	// 4. Clear Button [X]
	if (!isRightClick && ObserverUIHelpers::HitTestActive(this->ClearBtnRect, mousePos))
	{
		this->SearchFilterText.clear();
		this->IsSearchInputFocused = false;
		this->CollectPlayerData(true);
		return true;
	}

	// 5. Search Input Box
	if (!isRightClick && ObserverUIHelpers::HitTestActive(this->SearchBoxRect, mousePos))
	{
		this->IsSearchInputFocused = true;
		return true;
	}

	// Clicking anywhere else unfocuses the search input
	if (!isRightClick)
		this->IsSearchInputFocused = false;

	// 6. Category Filter Tab Buttons
	if (!isRightClick)
	{
		for (auto const& button : this->TabButtons)
		{
			if (!ObserverUIHelpers::HitTest(button.Rect, mousePos))
				continue;

			this->ActiveFilterTab = button.Category;
			this->CollectPlayerData(true);
			return true;
		}
	}

	int const playerColorBarWidth = 5;
	int const teamColorBarWidth = 10;

	for (auto& row : this->PlayerRows)
	{
		// Section 1: Player Info Box (including the team bar and the player color bar)
		RectangleStruct const infoClickRect {
			row.InfoRect.X - teamColorBarWidth,
			row.InfoRect.Y,
			row.InfoRect.Width + teamColorBarWidth + playerColorBarWidth,
			row.InfoRect.Height
		};

		if (ObserverUIHelpers::HitTest(infoClickRect, mousePos))
		{
			if (isRightClick)
			{
				// Right click opens / focuses a floating window for this player
				ObserverWindowFactory::OpenPlayerWindow(this->FloatingWindows, this->FloatingUnitWindows.size(), row.pHouse);
			}
			else
			{
				// Left click jumps the camera to the player's start point / base
				ObserverUIHelpers::CenterOnCoords(ObserverUIHelpers::GetPlayerStartCoords(row.pHouse));
			}

			return true;
		}

		// Per-row scroll buttons (one cameo width per click)
		if (!isRightClick && row.MaxScrollOffset > 0)
		{
			int const scrollStep = 64; // 1 cameo width (60 + 4 padding)

			if (ObserverUIHelpers::HitTest(row.ScrollLeftBtnRect, mousePos))
			{
				row.ScrollOffset = std::max(0, row.ScrollOffset - scrollStep);
				return true;
			}

			if (ObserverUIHelpers::HitTest(row.ScrollRightBtnRect, mousePos))
			{
				row.ScrollOffset = std::min(row.MaxScrollOffset, row.ScrollOffset + scrollStep);
				return true;
			}
		}

		// Production cameos
		for (auto& item : row.ProductionItems)
		{
			if (!ObserverUIHelpers::HitTest(item.DisplayRect, mousePos))
				continue;

			if (mousePos.X < row.ProdPanelRect.X || mousePos.X >(row.ProdPanelRect.X + row.ProdPanelRect.Width))
				continue;

			if (isRightClick)
			{
				return ObserverWindowFactory::OpenCameoWindow(
					this->FloatingUnitWindows, this->FloatingWindows.size(), this->CycleIndices, item, true);
			}

			this->CenterOnNextBuilding(item);
			return true;
		}

		// Structure / unit cameos
		for (auto& item : row.StructureItems)
		{
			if (!ObserverUIHelpers::HitTest(item.DisplayRect, mousePos))
				continue;

			if (mousePos.X < row.StructPanelRect.X || mousePos.X >(row.StructPanelRect.X + row.StructPanelRect.Width))
				continue;

			if (isRightClick)
			{
				return ObserverWindowFactory::OpenCameoWindow(
					this->FloatingUnitWindows, this->FloatingWindows.size(), this->CycleIndices, item, false);
			}

			// Left clicking a superweapon also opens its card, since it has no map instance to cycle
			if (item.IsSuperweapon)
			{
				ObserverWindowFactory::OpenCameoWindow(
					this->FloatingUnitWindows, this->FloatingWindows.size(), this->CycleIndices, item, false);
			}

			this->CenterOnNextBuilding(item);
			return true;
		}
	}

	return false;
}

bool ObserverUIClass::HandleKeyPress(int keyVal)
{
	if (!this->IsSearchInputFocused)
		return false;

	// Ignore key release events (WWKey::Release = 0x800)
	if ((keyVal & 0x800) != 0)
		return true; // Swallow the release so the game does not process it

	int const vk = keyVal & 0xFF;
	bool const isShift = ObserverUIHelpers::IsKeyDown(VK_SHIFT) || (keyVal & 0x100) != 0;

	// Enter or Escape -> Unfocus search box
	if (vk == VK_RETURN || vk == VK_ESCAPE)
	{
		this->IsSearchInputFocused = false;
		return true;
	}

	bool textChanged = false;

	if (vk == VK_BACK)
	{
		if (!this->SearchFilterText.empty())
		{
			this->SearchFilterText.pop_back();
			textChanged = true;
		}
	}
	else
	{
		wchar_t character = L'\0';

		switch (vk)
		{
		case VK_SPACE:
			character = L' ';
			break;

		case VK_OEM_MINUS:
			character = isShift ? L'_' : L'-';
			break;

		case VK_OEM_PERIOD:
			character = isShift ? L'>' : L'.';
			break;

		case VK_OEM_COMMA:
			character = isShift ? L'<' : L',';
			break;

		default:
			if (vk >= 'A' && vk <= 'Z')
				character = static_cast<wchar_t>(isShift ? vk : std::towlower(vk));
			else if (vk >= 32 && vk <= 126)
				character = static_cast<wchar_t>(vk);

			break;
		}

		if (character != L'\0')
		{
			this->SearchFilterText += character;
			textChanged = true;
		}
	}

	if (textChanged)
		this->CollectPlayerData(true);

	return true; // Swallow all other keys while the search box is focused
}

void ObserverUIClass::CenterOnNextBuilding(ObserverCameoItem& item)
{
	if (!TacticalClass::Instance())
		return;

	// Deselect the current in-game selection
	ObserverUIHelpers::DeselectAll();

	uintptr_t const typeKey = item.IsSuperweapon
		? reinterpret_cast<uintptr_t>(item.pSuperType)
		: reinterpret_cast<uintptr_t>(item.pType);

	auto const key = std::make_pair(item.pOwner, typeKey);

	// BUGFIX: the old code centered on whatever sat at the current index and advanced once,
	// so a dead / limbo instance (or a null factory building) silently ate the click and the
	// camera never moved. Walk forward until a live instance is found instead.
	if (!item.Buildings.empty())
	{
		size_t const count = item.Buildings.size();
		size_t index = this->CycleIndices[key] % count;

		for (size_t attempt = 0; attempt < count; ++attempt)
		{
			auto const pTarget = item.Buildings[index];
			index = (index + 1) % count;

			if (!ObserverUIHelpers::IsBuildingValidAndAlive(pTarget))
				continue;

			ObserverUIHelpers::CenterAndSelect(pTarget);
			break;
		}

		this->CycleIndices[key] = index;
	}
	else if (!item.Technos.empty())
	{
		size_t const count = item.Technos.size();
		size_t index = this->CycleIndices[key] % count;

		for (size_t attempt = 0; attempt < count; ++attempt)
		{
			auto const pTarget = item.Technos[index];
			index = (index + 1) % count;

			if (!ObserverUIHelpers::IsTechnoValidAndAlive(pTarget))
				continue;

			ObserverUIHelpers::CenterAndSelect(pTarget);
			break;
		}

		this->CycleIndices[key] = index;
	}
}

bool ObserverUIClass::HandleMouseWheel(bool isUp)
{
	bool const isActive = IsActive() || (Phobos::Config::DevelopmentCommands && this->DisplayMode != ObserverUIDisplayMode::Hidden);
	if (!isActive || !WWMouseClass::Instance())
		return false;

	Point2D const mousePos = ObserverUIHelpers::MousePosition();

	// 1. Horizontal cameo scrolling while hovering a specific player's cameo bar
	if (this->DisplayMode == ObserverUIDisplayMode::Full)
	{
		int const scrollStep = 64; // 1 cameo width (60 + 4 padding)

		for (auto& row : this->PlayerRows)
		{
			if (row.MaxScrollOffset <= 0 || !ObserverUIHelpers::HitTest(row.StructPanelRect, mousePos))
				continue;

			row.ScrollOffset = std::clamp(row.ScrollOffset + (isUp ? -scrollStep : scrollStep), 0, row.MaxScrollOffset);
			return true;
		}
	}

	// 2. Vertical player row scrolling (one row per wheel notch)
	if (this->MaxVerticalScrollOffset > 0)
	{
		this->VerticalScrollOffset = std::clamp(this->VerticalScrollOffset + (isUp ? -1 : 1), 0, this->MaxVerticalScrollOffset);
		return true;
	}

	return false;
}

bool ObserverUIClass::IsMouseHoveringUI() const
{
	if (!WWMouseClass::Instance())
		return false;

	bool const isUIOpen = IsActive() || (Phobos::Config::DevelopmentCommands && this->DisplayMode != ObserverUIDisplayMode::Hidden);

	if (!isUIOpen && !Phobos::Config::DevelopmentCommands)
		return false;

	if (this->DisplayMode == ObserverUIDisplayMode::Hidden && !this->HasFloatingWindows())
		return false;

	Point2D const mousePos = ObserverUIHelpers::MousePosition();

	// 0. A window currently being dragged always counts as hovering the UI
	if (ObserverUIHelpers::IsAnyWindowDragging(this->FloatingUnitWindows)
		|| ObserverUIHelpers::IsAnyWindowDragging(this->FloatingWindows))
	{
		return true;
	}

	// 1. Floating windows (player & unit cards)
	if (ObserverUIHelpers::IsMouseOverAnyWindow(this->FloatingUnitWindows, mousePos)
		|| ObserverUIHelpers::IsMouseOverAnyWindow(this->FloatingWindows, mousePos))
	{
		return true;
	}

	if (!isUIOpen)
		return false;

	// 2. Inspect button
	if (ObserverUIHelpers::HitTestActive(this->InspectBtnRect, mousePos))
		return true;

	if (this->DisplayMode == ObserverUIDisplayMode::Minimal)
		return false;

	RectangleStruct const globalRects[] {
		this->SearchBoxRect,
		this->ClearBtnRect,
		this->VertScrollUpBtnRect,
		this->VertScrollDownBtnRect
	};

	for (auto const& rect : globalRects)
	{
		if (ObserverUIHelpers::HitTestActive(rect, mousePos))
			return true;
	}

	// 3. Category filter tab buttons
	for (auto const& button : this->TabButtons)
	{
		if (ObserverUIHelpers::HitTest(button.Rect, mousePos))
			return true;
	}

	// 4. Player row panels
	for (auto const& row : this->PlayerRows)
	{
		// Section 1: Player Info Box (+ team bar on the left, player color bar on the right)
		if (row.InfoRect.Width > 0 && mousePos.Y >= row.InfoRect.Y && mousePos.Y <= (row.InfoRect.Y + row.InfoRect.Height))
		{
			if (mousePos.X >= (row.InfoRect.X - 10) && mousePos.X <= (row.InfoRect.X + row.InfoRect.Width + 5))
				return true;
		}

		// Section 2: Filtered objects panel & scroll buttons
		if (row.StructPanelRect.Width > 0 && mousePos.Y >= row.StructPanelRect.Y && mousePos.Y <= (row.StructPanelRect.Y + row.StructPanelRect.Height))
		{
			if (mousePos.X >= row.StructPanelRect.X && mousePos.X <= (row.StructPanelRect.X + row.StructPanelRect.Width))
				return true;

			if (row.ScrollLeftBtnRect.Width > 0 && mousePos.X >= row.ScrollLeftBtnRect.X && mousePos.X <= (row.ScrollLeftBtnRect.X + row.ScrollLeftBtnRect.Width))
				return true;

			if (row.ScrollRightBtnRect.Width > 0 && mousePos.X >= row.ScrollRightBtnRect.X && mousePos.X <= (row.ScrollRightBtnRect.X + row.ScrollRightBtnRect.Width))
				return true;
		}

		// Section 3: Production panel (+ left player color bar)
		if (row.ProdPanelRect.Width > 0 && mousePos.Y >= row.ProdPanelRect.Y && mousePos.Y <= (row.ProdPanelRect.Y + row.ProdPanelRect.Height))
		{
			if (mousePos.X >= (row.ProdPanelRect.X - 5) && mousePos.X <= (row.ProdPanelRect.X + row.ProdPanelRect.Width))
				return true;
		}
	}

	return false;
}

void ObserverUIClass::ClearFloatingWindows()
{
	this->FloatingWindows.clear();
	this->FloatingUnitWindows.clear();
}

void ObserverUIClass::ToggleDisplayMode()
{
	this->DisplayMode = static_cast<ObserverUIDisplayMode>(
		(static_cast<int>(this->DisplayMode) + 1) % static_cast<int>(ObserverUIDisplayMode::Count));
}

bool ObserverUIClass::IsToggleObserverUIHotkeyBound()
{
	return ObserverUIHelpers::IsHotkeyBound("ToggleObserverUI");
}

bool ObserverUIClass::IsShowObjectCardHotkeyBound()
{
	return ObserverUIHelpers::IsHotkeyBound("ShowObjectCard");
}

bool ObserverUIClass::OpenFloatingWindowForSelectedObject()
{
	std::vector<ObjectClass*> targets;

	// 1. First priority: the object currently under the mouse cursor
	for (auto const pTechno : *TechnoClass::Array)
	{
		if (pTechno && pTechno->IsMouseHovering)
		{
			targets.push_back(pTechno);
			break;
		}
	}

	// 2. Second priority: the current selection
	if (targets.empty())
	{
		for (int k = 0; k < ObjectClass::CurrentObjects->Count; ++k)
		{
			if (auto const pObject = ObjectClass::CurrentObjects->get_or_default(k))
				targets.push_back(pObject);
		}
	}

	bool openedAny = false;

	for (auto const pObject : targets)
	{
		if (!pObject)
			continue;

		auto const pBuilding = cast_to<BuildingClass*>(pObject);
		auto const pTechno = pBuilding ? nullptr : flag_cast_to<TechnoClass*>(pObject);

		auto const pValidBuilding = ObserverUIHelpers::IsBuildingValidAndAlive(pBuilding) ? pBuilding : nullptr;
		auto const pValidTechno = pValidBuilding ? nullptr : (ObserverUIHelpers::IsTechnoValidAndAlive(pTechno) ? pTechno : nullptr);

		if (!pValidTechno && !pValidBuilding)
			continue;

		ObserverWindowFactory::OpenObjectWindow(this->FloatingUnitWindows, this->FloatingWindows.size(), pValidTechno, pValidBuilding);
		openedAny = true;
	}

	return openedAny;
}