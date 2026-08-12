#pragma once

// =====================================================================================
// Include policy
//
// Only the headers whose *definitions* are actually required here are included:
//  - GeneralDefinitions.h : Mission / AbstractType / AIDifficulty enums (an enum cannot be
//                           forward declared without repeating its underlying type)
//  - GeneralStructures.h  : Point2D, RectangleStruct, ColorStruct, CellStruct, CoordStruct,
//                           LTRBStruct - all held by value below
//  - BitFont.h            : BitFontStateGuard stores the font state through decltype
//  - Windows.h            : DWORD / COLORREF used by the helper signatures
//
// Everything else is only ever used through a pointer or a reference and is forward
// declared instead. The heavy engine headers that used to live here (HouseClass.h,
// BuildingClass.h, TacticalClass.h, ScenarioClass.h, SuperClass.h, FactoryClass.h, ...)
// moved into ObserverUI.cpp.
// =====================================================================================

#include <Windows.h>

#include <GeneralDefinitions.h>
#include <GeneralStructures.h>
#include <BitFont.h>

#include <algorithm>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

class AbstractTypeClass;
class BSurface;
class BuildingClass;
class BuildingTypeClass;
class ConvertClass;
class DSurface;
class FactoryClass;
class FootClass;
class HouseClass;
class SuperClass;
class SuperWeaponTypeClass;
class TechnoClass;
class TechnoTypeClass;

// VERIFY: SHPCaches is declared as a struct in this fork; swap the class-key here if the
// definition uses `class` (otherwise MSVC emits C4099).
struct SHPCaches;

// Camera cycling index per (House, TechnoType) pair.
using ObserverCycleIndexMap = std::map<std::pair<HouseClass*, uintptr_t>, size_t>;

struct PlayerEconomySample
{
	int Frame { 0 };
	int Money { 0 };
};

enum class ObserverUIDisplayMode : int
{
	Full = 0,      // Phase 1: Full UI (All panels + floating windows + inspect button)
	Minimal,       // Phase 2: Minimal View (Only floating windows + bottom-left inspect button)
	Hidden,        // Phase 3: Hidden (Everything hidden)
	Count
};

enum class ObserverFilterCategory : int
{
	Defenses = 0,
	Structures,
	AllStructures,
	Infantry,
	Vehicles,
	Naval,
	Aircraft,
	AllUnits,
	Superweapons,
	Everything,
	Count
};

struct ObserverTabButton
{
	ObserverFilterCategory Category;
	std::wstring Label;
	RectangleStruct Rect { 0, 0, 0, 0 };
	bool IsHovered { false };
};

// Structure representing an individual cameo item (structure, unit or production item)
struct ObserverCameoItem
{
	TechnoTypeClass* pType { nullptr };
	SuperWeaponTypeClass* pSuperType { nullptr };
	SuperClass* pSuper { nullptr };
	bool IsSuperweapon { false };
	int Count { 0 };               // Quantity count
	int ProgressPercent { -1 };    // 0..100 for items currently in production
	bool IsProduction { false };
	HouseClass* pOwner { nullptr };
	std::vector<BuildingClass*> Buildings {}; // List of building instances for camera cycling
	std::vector<TechnoClass*> Technos {};     // List of techno instances for camera cycling
	RectangleStruct DisplayRect { 0, 0, 0, 0 }; // Screen area occupied by this cameo
};

// Structure representing an active player row in the Observer UI bar
struct ObserverPlayerRow
{
	HouseClass* pHouse { nullptr };
	int PlayerNumber { 0 }; // 1 for P1, 2 for P2, 3 for P3...
	ColorStruct PlayerColor { 0, 0, 0 };
	std::wstring PlayerName {};
	std::wstring CountryName {};
	HouseClass* TargetEnemy { nullptr };
	std::wstring TargetEnemyName {};
	int IncomeRatePerMin { 0 }; // Net credits per minute (+- $X/min)
	std::vector<ObserverCameoItem> ProductionItems {};
	std::vector<ObserverCameoItem> StructureItems {};

	RectangleStruct InfoRect { 0, 0, 0, 0 };
	RectangleStruct ProdPanelRect { 0, 0, 0, 0 };
	RectangleStruct StructPanelRect { 0, 0, 0, 0 };

	// Per-player independent scroll state & button rects
	int ScrollOffset { 0 };
	int MaxScrollOffset { 0 };
	RectangleStruct ScrollLeftBtnRect { 0, 0, 0, 0 };
	RectangleStruct ScrollRightBtnRect { 0, 0, 0, 0 };
	bool IsHoveringLeftScroll { false };
	bool IsHoveringRightScroll { false };

	int TeamID { -1 };
	int TeamMemberCount { 0 };
	ColorStruct TeamColor { 0, 0, 0 };
};

struct ObserverFloatingWindow
{
	HouseClass* pHouse { nullptr };
	Point2D Position { 150, 150 };
	RectangleStruct WindowRect { 0, 0, 0, 0 };
	RectangleStruct CloseBtnRect { 0, 0, 0, 0 };
	bool IsDragging { false };
	Point2D DragOffset { 0, 0 };
};

struct ObserverFloatingUnitWindow
{
	TechnoTypeClass* pType { nullptr };
	SuperWeaponTypeClass* pSuperType { nullptr };
	SuperClass* pSuper { nullptr };
	bool IsSuperweapon { false };
	HouseClass* pOwner { nullptr };
	BuildingClass* pTargetBuilding { nullptr };
	TechnoClass* pTargetTechno { nullptr };
	bool IsProductionItem { false };
	int InstanceNumber { 1 };
	Point2D Position { 200, 200 };
	RectangleStruct WindowRect { 0, 0, 0, 0 };
	RectangleStruct CloseBtnRect { 0, 0, 0, 0 };
	RectangleStruct CameoClickRect { 0, 0, 0, 0 };
	bool IsDragging { false };
	Point2D DragOffset { 0, 0 };

	// Last known snapshot before object death
	bool IsDestroyed { false };
	int LastHP { 0 };
	int LastMaxHP { 0 };
	CellStruct LastCoords { CellStruct::Empty };
	std::wstring LastMission { L"Destroyed" };
	float LastVeterancy { 0.0f };
};

// =====================================================================================
// Shared helper types
//
// Support code for ObserverUIClass. No unnamed namespace is used; helpers are grouped
// into small structs so they stay named and greppable. Only the templates are defined
// inline here - every other body lives in ObserverUI.cpp.
// =====================================================================================

struct BitFontStateGuard
{
	BitFontStateGuard();

	~BitFontStateGuard();

	BitFontStateGuard(const BitFontStateGuard&) = delete;
	BitFontStateGuard& operator=(const BitFontStateGuard&) = delete;
	// Clip all following text draws to `rect`.
	void Clip(const RectangleStruct& rect, bool enableClipping = true) const;

	decltype(BitFont::Instance->Bounds) Bounds;
	decltype(BitFont::Instance->Color) Color;
	decltype(BitFont::Instance->field_41) Field41;
};

struct ObserverPunctuationKey
{
	int VirtualKey;
	wchar_t Normal;
	wchar_t Shifted;
};

struct ObserverSearchKey
{
	int VirtualKey = -1;
	wchar_t Normal = L'\0';
	wchar_t Shifted = L'\0';
};

struct ObserverBuildingCounts
{
	int Barracks = 0;
	int Helipads = 0;
};

struct ObserverUIHelpers
{
	static constexpr int LogicFramesPerSecond = 15;
	static constexpr size_t PlayerColorCount = 8;
	// ---------------------------------------------------------------------------------
	// Geometry / input
	// ---------------------------------------------------------------------------------
	// NOTE: named IntersectRects (not IntersectRect) so it can never collide with the
	// Win32 IntersectRect() that <windows.h> drags in.
	static bool IntersectRects(const RectangleStruct& first, const RectangleStruct& second, RectangleStruct& out);

	static bool HitTest(const RectangleStruct& rect, const Point2D& point);

	// Same as HitTest but ignores rects that were never laid out this frame.
	static bool HitTestActive(const RectangleStruct& rect, const Point2D& point);

	static Point2D MousePosition();

	static bool IsKeyDown(int virtualKey);

	static int ScreenWidth();

	static int ScreenHeight();

	// ---------------------------------------------------------------------------------
	// Session / house queries
	// ---------------------------------------------------------------------------------
	static bool IsMultiplayerSession();

	static bool IsNonPlayerHouse(HouseClass* pHouse);

	static const ColorStruct* PlayerColorPalette();

	static ColorStruct PaletteColor(size_t index);

	static ColorStruct GetHouseColor(HouseClass* pHouse, int fallbackIdx = 0);

	static bool IsValidEnemy(HouseClass* pHouse, HouseClass* pCandidate);

	static HouseClass* GetTargetEnemy(HouseClass* pHouse);

	template<typename TArray>
	static bool HasLiveObject(TArray& array, HouseClass* pHouse)
	{
		for (auto const pObject : array)
		{
			if (pObject && pObject->Owner == pHouse && pObject->IsAlive && !pObject->InLimbo)
				return true;
		}

		return false;
	}
	static bool HouseHasGrantedSuperWeapon(HouseClass* pHouse);

	static bool HouseHasActiveFactory(HouseClass* pHouse);

	// Any house that owns something worth displaying (used for singleplayer / debug mode).
	static bool HouseHasContent(HouseClass* pHouse);

	// In standard multiplayer (DevelopmentCommands=false) only real MP players are listed.
	static bool IsHouseHiddenInMultiplayer(HouseClass* pHouse);

	static ObserverBuildingCounts CountSupportBuildings(HouseClass* pHouse);

	static int GetKilledUnitCount(HouseClass* pHouse);

	static int GetKilledBuildingCount(HouseClass* pHouse);

	static CoordStruct GetPlayerStartCoords(HouseClass* pHouse);

	// ---------------------------------------------------------------------------------
	// Object queries
	// ---------------------------------------------------------------------------------
	static bool IsTechnoValidAndAlive(TechnoClass* pTechno);

	static bool IsBuildingValidAndAlive(BuildingClass* pBuilding);

	static int GetFactoryProgressPercent(FactoryClass* pFactory);

	static BuildingClass* FindFactoryBuilding(HouseClass* pHouse, FactoryClass* pFactory);

	static bool BuildingGrantsSuperWeapon(BuildingClass* pBuilding, SuperWeaponTypeClass* pSWType);

	static std::vector<BuildingClass*> CollectSuperWeaponBuildings(HouseClass* pHouse, SuperWeaponTypeClass* pSWType);

	static bool IsConsideredVehicle(TechnoClass* pTechno, TechnoTypeClass* pType);

	static bool MatchesFilterCategory(TechnoClass* pTechno, TechnoTypeClass* pType, ObserverFilterCategory category);

	// ---------------------------------------------------------------------------------
	// Text formatting
	// ---------------------------------------------------------------------------------
	static std::wstring ToWide(const char* text);

	static std::wstring ToWide(const std::string& text);

	static const wchar_t* PlayerPrefix();

	static std::wstring GetHousePlainName(HouseClass* pHouse);

	static std::wstring GetHouseControlSuffix(HouseClass* pHouse);

	static std::wstring FormatObjectNameWithDebug(int playerNum, const char* pID, const wchar_t* pUIName, bool isDebugEnabled);

	// "ID (UIName)" or just "ID" - used by the debug AI team lines.
	static std::wstring FormatIDName(AbstractTypeClass* pType);

	static std::wstring GetMissionNameString(Mission mission);

	static void FramesToMinutesSeconds(int frames, int& minutes, int& seconds);

	// "01:45 / 05:00"
	static std::wstring FormatTimerPair(int framesLeft, int totalFrames);

	static std::wstring FormatDuration(int frames);

	static double CellDistance(const CellStruct& from, const CellStruct& to);

	static std::wstring FormatDistance(double distanceInCells);

	// Groups an ordered list of types into "2x [E1] GI, 1x [E2] Rocketeer".
	static std::wstring FormatTypeCountList(const std::vector<TechnoTypeClass*>& types);

	// ---------------------------------------------------------------------------------
	// Search input
	// ---------------------------------------------------------------------------------
	static ObserverSearchKey PollSearchKey();

	static bool ApplySearchKey(std::wstring& text, const ObserverSearchKey& key, bool isShiftHeld);

	static bool IsHotkeyBound(const char* commandName);

	// ---------------------------------------------------------------------------------
	// Player rows
	// ---------------------------------------------------------------------------------
	static std::vector<ObserverPlayerRow>::const_iterator FindRow(const std::vector<ObserverPlayerRow>& rows, HouseClass* pHouse);

	static int GetPlayerNumber(const std::vector<ObserverPlayerRow>& rows, HouseClass* pHouse);

	static ColorStruct GetRowColor(const std::vector<ObserverPlayerRow>& rows, HouseClass* pHouse);

	// ---------------------------------------------------------------------------------
	// Floating window management
	// ---------------------------------------------------------------------------------
	template<typename T, typename TPredicate>
	static bool BringToFrontIf(std::vector<T>& windows, TPredicate predicate)
	{
		auto const it = std::find_if(windows.begin(), windows.end(), predicate);
		if (it == windows.end())
			return false;

		if (it + 1 != windows.end())
		{
			T const found = *it;
			windows.erase(it);
			windows.push_back(found);
		}

		return true;
	}
	static Point2D CascadePosition(size_t openWindowCount, int cardWidth);

	template<typename T>
	static bool UpdateWindowDrag(std::vector<T>& windows, const Point2D& mousePos, bool isLeftPressed)
	{
		bool anyDragging = false;
		int const screenW = ScreenWidth();
		int const screenH = ScreenHeight();

		for (auto& window : windows)
		{
			if (!window.IsDragging)
				continue;

			if (!isLeftPressed)
			{
				window.IsDragging = false;
				continue;
			}

			window.Position.X = std::clamp(mousePos.X - window.DragOffset.X, 0, std::max(0, screenW - window.WindowRect.Width));
			window.Position.Y = std::clamp(mousePos.Y - window.DragOffset.Y, 0, std::max(0, screenH - window.WindowRect.Height));
			anyDragging = true;
		}

		return anyDragging;
	}
	template<typename T>
	static bool IsMouseOverAnyWindow(const std::vector<T>& windows, const Point2D& mousePos)
	{
		for (auto const& window : windows)
		{
			if (HitTestActive(window.WindowRect, mousePos))
				return true;
		}

		return false;
	}
	template<typename T>
	static bool IsAnyWindowDragging(const std::vector<T>& windows)
	{
		for (auto const& window : windows)
		{
			if (window.IsDragging)
				return true;
		}

		return false;
	}
	// ---------------------------------------------------------------------------------
	// Map interaction
	// ---------------------------------------------------------------------------------
	static void DeselectAll();

	static void CenterOnCoords(const CoordStruct& coords);

	static void CenterAndSelect(TechnoClass* pTarget);

	// ---------------------------------------------------------------------------------
	// Cameo asset lookup
	// ---------------------------------------------------------------------------------
	static BSurface* ValidSurface(BSurface* pSurface);

	// Tries "<name>" first, then a lowercased "<name>.pcx".
	static BSurface* LoadPCXSurface(const char* fileName);

	// Tries "<id>icon.pcx", then "<id>.pcx" (both lowercased).
	static BSurface* LoadPCXSurfaceForID(const char* id);

	// Tries "<name>" first, then a lowercased "<name>.shp".
	static SHPCaches* LoadSHPFile(const char* fileName);

	static SHPCaches* LoadPlaceholderCameo();

	static bool IsPlaceholderCameo(SHPCaches* pCameo);

	// ---------------------------------------------------------------------------------
	// Drawing primitives
	// ---------------------------------------------------------------------------------
	static void DrawPanel(DSurface* pSurface, RectangleStruct rect, ColorStruct fillColor, int opacity, COLORREF borderColor);

	static void DrawCenteredText(DSurface* pSurface, RectangleStruct rect, const wchar_t* text, COLORREF color);

	static void DrawShadowText(DSurface* pSurface, RectangleStruct clipRect, const wchar_t* text, Point2D position, COLORREF color);

	static void DrawCloseButton(DSurface* pSurface, RectangleStruct rect, bool isHovered);

	// Centered overlay label on a cameo (production %, superweapon %, instance count).
	static void DrawCameoOverlayText(DSurface* pSurface, const RectangleStruct& displayRect, const RectangleStruct& clipRect, const std::wstring& text, COLORREF color);

	static void DrawSimpleTooltip(DSurface* pSurface, const wchar_t* text, const Point2D& mousePos);

	static bool DrawCameoImage(DSurface* pSurface, RectangleStruct destinationRect, BSurface* pPCXSurface, SHPCaches* pFileSHP, ConvertClass* pPalette, int frameIndex = 0, int zAdjust = -2);
};

// =====================================================================================
// Text block - replaces the four hand-rolled "addLine / addLineSegments" lambda pairs
// =====================================================================================

struct ObserverTextSegment
{
	std::wstring Text;
	DWORD Color;
};

struct ObserverTextLine
{
	std::vector<ObserverTextSegment> Segments;
	int Width;
	int Height;
};

struct ObserverTextBlock
{
	static constexpr int LineGap = 2;
	explicit ObserverTextBlock(int maxWidth);

	void AddSegments(std::vector<ObserverTextSegment> segments);

	void Add(const std::wstring& text, DWORD color);

	void Add(const std::wstring& label, DWORD labelColor, const std::wstring& value, DWORD valueColor);

	void Render(DSurface* pSurface, const RectangleStruct& clipRect, int left, int top) const;

	std::vector<ObserverTextLine> Lines;
	int MaxWidth;
	int Width = 0;
	int Height = 0;
};

struct ObserverTooltipBox
{
	static RectangleStruct Compute(const ObserverTextBlock& block, const Point2D& mousePos, int padding);

	static void Render(DSurface* pSurface, const ObserverTextBlock& block, const Point2D& mousePos, ColorStruct borderColor, int padding = 6);
};

// =====================================================================================
// House summary - single source of truth for the player card AND the player tooltip
// =====================================================================================

struct ObserverHouseSummaryOptions
{
	// Prefix names with their INI [ID] (DevelopmentCommands / debug display).
	bool DebugFormatting = false;
	// Card-only "Status: Defeated" line.
	bool ShowDefeatedStatus = false;
	// Card hides zeroed refinery / war factory counts, the tooltip always shows them.
	bool HideZeroFactoryCounts = false;
	// Card only uses "P<n>" prefixes in multiplayer, the tooltip always uses them.
	bool PlayerNumberNeedsMultiplayer = false;
};

struct ObserverHouseSummary
{
	static DWORD White();

	static DWORD Label();

	static DWORD Muted();

	static DWORD Good();

	static DWORD Bad();

	static DWORD SoftBad();

	static DWORD Accent();

	static bool UsePlayerNumber(int playerNumber, bool isMultiplayer, const ObserverHouseSummaryOptions& options);

	// "P1 [GDI] Player (Allied) [AI Hard]" in all its debug / non-debug variants.
	static std::wstring FormatHouseName(HouseClass* pHouse, int playerNumber, bool isMultiplayer, const ObserverHouseSummaryOptions& options, bool bracketPlayerNumber);

	static void AddEconomyLines(ObserverTextBlock& block, HouseClass* pHouse, const std::vector<ObserverPlayerRow>& rows);

	static void AddPowerLine(ObserverTextBlock& block, HouseClass* pHouse);

	// Debug-only AI / Tech lines (ONLY if DebugKeysEnabled=yes in rulesmd.ini)
	static void AddDebugLines(ObserverTextBlock& block, HouseClass* pHouse);

	static void AddObjectCountLines(ObserverTextBlock& block, HouseClass* pHouse, const ObserverHouseSummaryOptions& options);

	static void AddAlliesLine(ObserverTextBlock& block, HouseClass* pHouse, const std::vector<ObserverPlayerRow>& rows, bool isMultiplayer, const ObserverHouseSummaryOptions& options);

	static void AddTargetEnemyLine(ObserverTextBlock& block, HouseClass* pHouse, const std::vector<ObserverPlayerRow>& rows, bool isMultiplayer, const ObserverHouseSummaryOptions& options);

	static void Build(ObserverTextBlock& block, HouseClass* pHouse, const std::vector<ObserverPlayerRow>& rows, const ObserverHouseSummaryOptions& options);
};

// =====================================================================================
// Unit / production / superweapon card content
// =====================================================================================

struct ObserverUnitCardContext
{
	FactoryClass* Factory = nullptr;
	TechnoTypeClass* CurrentProduct = nullptr;
	TechnoTypeClass* CameoType = nullptr;
	bool IsProductionView = false;
};

struct ObserverUnitCardState
{
	ObserverFloatingUnitWindow* Window = nullptr;
	const std::vector<ObserverPlayerRow>* Rows = nullptr;
	HouseClass* Owner = nullptr;
	BuildingClass* Building = nullptr;
	TechnoClass* Techno = nullptr;
	FootClass* Foot = nullptr;
	FactoryClass* Factory = nullptr;
	TechnoTypeClass* BaseType = nullptr;
	TechnoTypeClass* TitleType = nullptr;
	TechnoTypeClass* TargetType = nullptr;
	TechnoTypeClass* CurrentProduct = nullptr;
	bool IsProductionView = false;
	bool IsDebug = false;
	int PlayerNumber = 0;
};

struct ObserverUnitCard
{
	using Colors = ObserverHouseSummary;
	static const wchar_t* Text(const char* label, const wchar_t* fallback);

	// Owner Line in Singleplayer / Campaign
	static void AddOwnerLine(ObserverTextBlock& block, HouseClass* pOwner);

	static void AddCoordsLine(ObserverTextBlock& block, const CellStruct& cell, const wchar_t* label, DWORD color);

	static void AddDistanceLine(ObserverTextBlock& block, const wchar_t* label, const CellStruct& from, const CellStruct& to, DWORD color);

	// ---------------------------------------------------------------------------------
	// Superweapon card
	// ---------------------------------------------------------------------------------
	static void AddSuperWeaponLines(ObserverTextBlock& block, ObserverFloatingUnitWindow& win, int playerNumber, bool isDebug);

	// ---------------------------------------------------------------------------------
	// Techno / production card
	// ---------------------------------------------------------------------------------
	static void UpdateSnapshot(ObserverUnitCardState& state);

	static BuildingTypeClass* FindFactoryBuildingType(const ObserverUnitCardState& state);

	static void AddTitleLine(ObserverTextBlock& block, const ObserverUnitCardState& state);

	static void AddProductionLines(ObserverTextBlock& block, const ObserverUnitCardState& state);

	static void AddHealthLine(ObserverTextBlock& block, const ObserverUnitCardState& state);

	static void AddShieldLine(ObserverTextBlock& block, const ObserverUnitCardState& state);

	static void AddTargetLines(ObserverTextBlock& block, const ObserverUnitCardState& state, const CellStruct& currentCell);

	static void AddLocationLines(ObserverTextBlock& block, const ObserverUnitCardState& state);

	static void AddAmmoLine(ObserverTextBlock& block, const ObserverUnitCardState& state);

	static void AddVeterancyLine(ObserverTextBlock& block, const ObserverUnitCardState& state);

	static void AddOccupantLines(ObserverTextBlock& block, const ObserverUnitCardState& state);

	// Total Build Time (MM:SS) & Cost - ONLY for production cards while actually producing.
	static void AddBuildTimeAndCostLines(ObserverTextBlock& block, const ObserverUnitCardState& state);

	// Debug AI Team Info - ONLY when DebugKeysEnabled=yes, ALWAYS at the very end of the card.
	static void AddDebugTeamLines(ObserverTextBlock& block, const ObserverUnitCardState& state);

	// ---------------------------------------------------------------------------------
	// Entry point
	// ---------------------------------------------------------------------------------
	static ObserverUnitCardContext Build(ObserverTextBlock& block, ObserverFloatingUnitWindow& win, const std::vector<ObserverPlayerRow>& rows);
};

// =====================================================================================
// Floating window creation
// =====================================================================================

struct ObserverWindowFactory
{
	static constexpr int UnitCardWidth = 320;
	static constexpr int PlayerCardWidth = 340;
	// NOTE: takes the concrete ObserverCycleIndexMap (declared in the header) instead of
	// being a template, so the body can live in the .cpp and the header does not need the
	// full TechnoClass / BuildingClass definitions.
	static bool OpenCameoWindow(std::vector<ObserverFloatingUnitWindow>& unitWindows, size_t otherWindowCount, ObserverCycleIndexMap& cycleIndices, const ObserverCameoItem& item, bool isFromProductionPanel);

	static void OpenObjectWindow(std::vector<ObserverFloatingUnitWindow>& unitWindows, size_t otherWindowCount, TechnoClass* pTechno, BuildingClass* pBuilding);

	static void OpenPlayerWindow(std::vector<ObserverFloatingWindow>& windows, size_t otherWindowCount, HouseClass* pHouse);
};

// =====================================================================================
// ObserverUIClass
// =====================================================================================

struct ObserverUIState
{
	static bool IsInteractive(ObserverUIDisplayMode mode, bool hasFloatingWindows);
};

class ObserverUIClass
{
public:
	static ObserverUIClass Instance;

	void Update();
	void Render(DSurface* pSurface);
	bool HandleMouseClick(Point2D mousePos, bool isRightClick);
	bool HandleKeyPress(int keyVal);
	bool HandleMouseWheel(bool isUp);
	bool IsMouseHoveringUI() const;
	bool IsSearchFocused() const { return this->IsSearchInputFocused; }
	ObserverUIDisplayMode GetDisplayMode() const { return this->DisplayMode; }
	void ClearData();

	void RenderFloatingWindows(DSurface* pSurface);
	void RenderFloatingUnitWindows(DSurface* pSurface);
	bool OpenFloatingWindowForSelectedObject();
	void ClearFloatingWindows();
	void ToggleDisplayMode();
	static bool IsToggleObserverUIHotkeyBound();
	static bool IsShowObjectCardHotkeyBound();

	bool HasFloatingWindows() const { return !this->FloatingWindows.empty() || !this->FloatingUnitWindows.empty(); }

	static bool IsActive();

private:
	ObserverUIDisplayMode DisplayMode { ObserverUIDisplayMode::Hidden };
	bool WasEnterPressed { false };
	void CollectPlayerData();
	void DrawCameoItem(DSurface* pSurface, const ObserverCameoItem& item, bool isHovered, const RectangleStruct& clipRect, ColorStruct playerColor);
	void DrawTooltip(DSurface* pSurface, const ObserverCameoItem& item, Point2D mousePos);
	void DrawPlayerTooltip(DSurface* pSurface, HouseClass* pHouse, Point2D mousePos);
	// EXTENSION: single dispatch for the inspect-button / player / cameo tooltips, shared
	// by the Minimal and the Full render paths.
	void DrawHoverTooltip(DSurface* pSurface, Point2D cursorPos, Point2D tooltipPos);
	void CenterOnNextBuilding(ObserverCameoItem& item);

	ObserverFilterCategory ActiveFilterTab { ObserverFilterCategory::AllStructures };
	std::vector<ObserverTabButton> TabButtons {};

	// Search filter box state
	std::wstring SearchFilterText {};
	bool IsSearchInputFocused { false };
	RectangleStruct InspectBtnRect { 0, 0, 0, 0 };
	RectangleStruct SearchBoxRect { 0, 0, 0, 0 };
	RectangleStruct ClearBtnRect { 0, 0, 0, 0 };
	bool IsHoveringInspectBtn { false };
	bool IsHoveringClearBtn { false };

	// Vertical player rows scrolling state
	int VerticalScrollOffset { 0 };
	int MaxVerticalScrollOffset { 0 };
	RectangleStruct VertScrollUpBtnRect { 0, 0, 0, 0 };
	RectangleStruct VertScrollDownBtnRect { 0, 0, 0, 0 };
	bool IsHoveringVertScrollUp { false };
	bool IsHoveringVertScrollDown { false };

	std::vector<std::wstring> ParseSearchTerms(const std::wstring& query) const;
	bool MatchesSearchFilter(AbstractTypeClass* pType) const;

	std::vector<ObserverPlayerRow> PlayerRows {};
	std::vector<ObserverFloatingWindow> FloatingWindows {};
	std::vector<ObserverFloatingUnitWindow> FloatingUnitWindows {};
	std::map<HouseClass*, std::deque<PlayerEconomySample>> EconomyHistory {};

	// Tooltip tracking
	ObserverCameoItem HoveredItem {};
	bool HasHoveredItem { false };

	HouseClass* pHoveredPlayer { nullptr };
	bool HasHoveredPlayer { false };

	Point2D HoveredMousePos { 0, 0 };

	// Tracks camera cycling index per (House, BuildingTypeArrayIndex) pair
	ObserverCycleIndexMap CycleIndices {};
};