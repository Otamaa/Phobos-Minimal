#pragma once

#include <Utilities/Template.h>
#include <Timers.h>
#include <CDTimer.h>
#include <TransitionTimer.h>
#include <ProgressTimer.h>
#include <GeneralStructures.h>

// Forward declarations
struct SHPStruct;
class BSurface;
class ConvertClass;
class TechnoTypeClass;
class ShapeButtonClass;
class ToggleClass;
class HouseTypeExtData;
class DSurface;
class SuperWeaponTypeClass;
class SWTypeExtData;
class ScenarioExtData;
class HouseTypeExtData;
class INI_EX;

void ConfigureTemporarySWClass(int index, TechnoTypeClass* pTransporterType, const CellStruct& cell, const CellStruct& spawnCell);

class DropshipLoadoutClass
{
public:
	DropshipLoadoutClass() = default;
	~DropshipLoadoutClass();

	bool Initialize(bool bIgnoreFixedUnits = false, bool bPreloadCargo = false, int allowableUnitsIndex = 0, int startingMoney = 0, Nullable<bool> bAddUnusedMoneyToPlayer = {}, Nullable<bool> bRememberPurchasedCargo = {}, SuperWeaponTypeClass* pSWType = nullptr);
	void Run();

	static void OpenInGameWindow(bool bIgnoreFixedUnits = false, bool bPreloadCargo = false, int allowableUnitsIndex = 0, int startingMoney = 0, Nullable<bool> bAddUnusedMoneyToPlayer = {}, Nullable<bool> bRememberPurchasedCargo = {}, SuperWeaponTypeClass* pSWType = nullptr);
	static bool IsDropshipLoadoutActive();
	static void DropshipLoadout_OnMouseWheelUp();
	static void DropshipLoadout_OnMouseWheelDown();

	static void ParseHouse(INI_EX& exINI, const char* pSection, HouseTypeExtData* pData);
	static void ParseScenario(INI_EX& exINI, const char* pSection, ScenarioExtData* pData);
	static void ParseSWType(INI_EX& exINI, const char* pSection, SWTypeExtData* pData);

private:
	void LoadAssets();
	void CalculateLayout(DSurface* pSurface);
	void CreateControls();
	void HandleInput(int command, int buttonID);
	void UpdateAnimations();
	void Render(DSurface* pSurface);
	void DrawTooltip(DSurface* pSurface);
	void SaveCargo();
	int GetCarrierSizeLimit(int carrierIdx);
	bool CanCarrierHoldUnit(int carrierIdx, TechnoTypeClass* pUnitType);

	// Extensions
	HouseTypeExtData* pHouseTypeExt { nullptr };
	SuperWeaponTypeClass* pSWType { nullptr };
	class SWTypeExtData* pSWTypeExt { nullptr };

	// Config & state
	int nStartingDropships { 0 };
	long initialMoney { 0 };
	long currentMoney { 0 };
	int nSidebarCameos { 8 };
	int nDropshipBayCameos { 5 };
	int nDropshipBayTotalSlots { 0 };
	int firstBrowsableCameo { 0 };
	bool pressedSpaceKey { false };
	bool repaintAll { true };
	bool lastTimeWasOverCameos { false };
	bool freeDropshipSlots { false };
	bool bIgnoreFixedUnits { false };
	bool bPreloadCargo { false };
	Nullable<bool> bAddUnusedMoneyToPlayer {};
	Nullable<bool> bRememberPurchasedCargo {};
	int allowableUnitsIndex { 0 };
	int startingMoney { 0 };

	// Assets (Palette, surfaces, SHPs)
	ConvertClass* dropshipLoadout_Palette { nullptr };
	SHPStruct* dropshipLoadout_Background { nullptr };
	SHPStruct* dropshipLoadout_UpArrow { nullptr };
	SHPStruct* dropshipLoadout_DownArrow { nullptr };
	SHPStruct* dropshipLoadout_Loadout { nullptr };
	SHPStruct* dropshipLoadout_PilotLit { nullptr };
	std::vector<SHPStruct*> dropshipLoadout_DGreenList {};

	BSurface* dropshipLoadout_BackgroundPCX { nullptr };
	BSurface* dropshipLoadout_UpArrowPCX { nullptr };
	BSurface* dropshipLoadout_DownArrowPCX { nullptr };
	std::vector<BSurface*> dropshipLoadout_LoadoutPCX {};
	std::vector<BSurface*> dropshipLoadout_PilotLitPCX {};
	std::vector<std::vector<BSurface*>> dropshipLoadout_DGreenListPCX {};

	// Unit lists
	std::vector<TechnoTypeClass*> availableUnits {};
	std::vector<int> availableUnitsMaximums {};
	std::vector<std::vector<TechnoTypeClass*>> dropshipBayChosenUnitsLists {};
	std::vector<std::vector<bool>> dropshipBayFixedUnitsLists {};
	std::map<TechnoTypeClass*, int> dropshipBayChosenUnitsCount {};
	TechnoTypeClass* lastSelected { nullptr };
	TechnoTypeClass* pHoveredUnitType { nullptr };
	int hoveredDropshipIdx { -1 };
	int hoveredSlotIdx { -1 };

	// Layout/Locations
	RectangleStruct windowRectangle;
	int upArrowX { 0 }, upArrowY { 0 };
	int downArrowX { 0 }, downArrowY { 0 };
	RectangleStruct upArrowLocation {};
	RectangleStruct downArrowLocation {};
	std::vector<RectangleStruct> sidebarCameLocations {};
	std::vector<std::vector<RectangleStruct>> dropshipBayCameLocations {};
	RectangleStruct loadoutLocation {};
	RectangleStruct pilotLitLocation {};
	std::vector<RectangleStruct> dGreenLocation {};

	// Interactive Buttons
	std::vector<ShapeButtonClass*> buttonsList {};
	ToggleClass* commandManager { nullptr };

	// Animations & Timers
	int currentLoadoutFrame { -1 };
	int currentPilotLitFrame { -1 };
	int loadoutFrameDelay { 11 };
	int pilotLitFrameDelay { 15 };
	int loadoutTotalFrames { 0 };
	int pilotLitTotalFrames { 0 };
	int animTimer_StartValue { 15 };
	int animTimer_DelayedStartValue_Loadout { 0 };
	int animTimer_DelayedStartValue_PilotLit { 0 };

	SysTimerClass animTimer_UpdateFrameTimer {};
	SysTimerClass animTimer_DelayedStartTimer_Loadout {};
	SysTimerClass animTimer_UpdateFrameTimer_Loadout {};
	SysTimerClass animTimer_DelayedStartTimer_PilotLit {};
	SysTimerClass animTimer_UpdateFrameTimer_PilotLit {};

	int sidebarRowAnimationIndex { -1 };
	int currentSidebarRowAnimationFrame { 0 };
	int sidebarRowAnimationFrameDelay { 5 };
	int sidebarRowAnimationTotalFrames { 0 };
	SysTimerClass animTimer_UpdateFrameTimer_SidebarRowAnimation {};

	// Sounds
	int buyClickSoundIdx { -1 };
	int sellClickSoundIdx { -1 };
	int arrowsClickSoundIdx { -1 };
	int startingDragDropSoundIdx { -1 };
	int endingDragDropSoundIdx { -1 };

	// Drag & Drop state
	bool bIsDragging { false };
	bool bDragPending { false };
	TechnoTypeClass* pDraggedUnitType { nullptr };
	int nSourceDropshipIdx { -1 };
	int nSourceSlotIdx { -1 };
	bool bDraggedIsFixed { false };
	Point2D dragStartMousePos { 0, 0 };

private:

	// -----------------------------------------------------------------------
	// Config sources
	// -----------------------------------------------------------------------

	// Was duplicated in: Initialize(), GetCarrierSizeLimit(), SaveCargo() x2.
	std::vector<TechnoTypeClass*> GatherCarriers() const;

	// Was duplicated in: GetCarrierSizeLimit() only for the size list, but the
	// pattern (house-type list, else scenario-global list) is the same shape.
	std::vector<int> GatherCarrierSizeLimits() const;

	// -----------------------------------------------------------------------
	// Purchase / limit queries
	// -----------------------------------------------------------------------

	// `dropshipBayChosenUnitsCount.count(p) > 0 ? [p] : 0`
	// Was duplicated in: Run(), HandleInput() x2, Render() x2, DrawTooltip().
	int GetInstanceCount(TechnoTypeClass* pType) const;

	// Linear scan of availableUnits/availableUnitsMaximums for the cap.
	// Was duplicated in: Run(), HandleInput(), Render(), DrawTooltip().
	// Returns INT_MAX when uncapped.
	int GetMaxInstances(TechnoTypeClass* pType) const;

	// Raw configured maximum (-1 = uncapped, 0 = hidden). Used by DrawTooltip.
	int GetConfiguredMaximum(TechnoTypeClass* pType) const;

	// "is there any carrier that can hold this type and has an empty slot"
	// Was duplicated in: Run(), HandleInput(), Render() x2.
	bool HasCompatibleFreeSlot(TechnoTypeClass* pType);

	// "does carrier N have any empty slot" - Was duplicated in: Run(), Render().
	bool CarrierHasFreeSlot(int carrierIdx) const;

	// -----------------------------------------------------------------------
	// Slot mutation
	// -----------------------------------------------------------------------

	// Refund + compact list + decrement count + play sell sound + repaint.
	// Was duplicated in: HandleInput() x2 (sidebar right-click, bay right-click).
	bool SellUnitAt(int carrierIdx, int slotIdx);

	// Charge + occupy + increment count + lastSelected + play buy sound.
	// Was duplicated in: Run() (quick-click purchase) and HandleInput()
	// (sidebar left-click purchase).
	bool BuyIntoFirstFreeSlot(TechnoTypeClass* pType);

	// -----------------------------------------------------------------------
	// Geometry
	// -----------------------------------------------------------------------

	// Bounding box of the sidebar cameo column plus the two scroll arrows,
	// padded by 10px left/top/bottom and extended to the window's right edge.
	// Was duplicated verbatim in: Run() (drop target) and Render() (sell hint).
	RectangleStruct GetSidebarArea() const;

	// Returns the ID of the button under the cursor, or -1.
	// Was inline in Run(); extracted so Render()/HandleInput() can share it.
	int HitTestButton() const;

	// Creates a ShapeButtonClass, positions it, registers it in buttonsList and
	// wires it to commandManager. Was duplicated 4x in CreateControls().
	//
	// SUSPECT: the original made the FIRST successfully-created button the
	// command manager and never Add()ed it to itself. If that first button
	// (the scroll-up arrow) ever fails to allocate, commandManager stays null
	// and every later Add() is silently skipped - the whole screen becomes
	// non-interactive with no diagnostic. Preserved, but worth a Debug::Log.
	ShapeButtonClass* AddButton(int id, const RectangleStruct& rect);

	// -----------------------------------------------------------------------
	// LoadAssets sub-steps
	// -----------------------------------------------------------------------

	// Was duplicated in: LoadAssets() x2 (SW branch and country branch).
	// `configuredMoney` is -1 when no Money= was set anywhere.
	long ResolveInitialMoney(long configuredMoney, bool& usesPlayerWalletOut) const;

	// Cost of the saved cargo that is neither a fixed unit nor a free initial
	// unit. Consumes matches out of `initialUnitsRemaining`.
	// Was duplicated in: LoadAssets() x2.
	static long AccountPreloadedUnits(
		const std::vector<TechnoTypeClass*>& cargo,
		const std::vector<TechnoTypeClass*>* pFixedList,
		std::vector<TechnoTypeClass*>& initialUnitsRemaining);

	// Fills availableUnits/availableUnitsMaximums, skipping maximum == 0.
	// Was duplicated in: LoadAssets() x2.
	void BuildAvailableUnits(const std::vector<TechnoTypeClass*>& allowable, std::vector<int> maximums);

	// Appends a type to availableUnits if absent, uncapped.
	// Was duplicated in: LoadAssets() x2.
	void EnsureUnitAvailable(TechnoTypeClass* pUnit);

	// Loads DGREEN1..4 defaults into dropshipLoadout_DGreenList.
	// Was duplicated in: LoadAssets() x2.
	void LoadDefaultDGreenList();

	// -----------------------------------------------------------------------
	// CreateControls sub-step
	// -----------------------------------------------------------------------

	// One bay slot's occupant, resolved from saved cargo (preload) or from the
	// fixed/initial lists. The SW and country branches of CreateControls() had
	// this same ~45-line block twice with only the container shape differing.
	struct BaySlotSource
	{
		const std::vector<TechnoTypeClass*>* pSavedCargo = nullptr; // null => no preload
		const std::vector<TechnoTypeClass*>* pFixedUnits = nullptr;
		const std::vector<TechnoTypeClass*>* pInitialUnits = nullptr;
	};

	TechnoTypeClass* ResolveBaySlot(
		const BaySlotSource& source,
		int slotIdx,
		std::vector<TechnoTypeClass*>& fixedRemaining,
		std::vector<TechnoTypeClass*>& initialRemaining,
		bool& isFixedOut);

	// -----------------------------------------------------------------------
	// CalculateLayout sub-step
	// -----------------------------------------------------------------------

	// The SW and country branches of CalculateLayout() were ~90% identical -
	// only the SOURCE of each value differed. Resolving the sources up front
	// collapses ~450 lines into ~180.
	struct LayoutConfig
	{
		Point2D UpArrow = Point2D::Empty;
		Point2D DownArrow = Point2D::Empty;
		Point2D Loadout { 45, 2 };
		Point2D PilotLit { 284, 151 };

		int SidebarCameosCount = 0;                            // 0 => default 2-wide grid
		const std::vector<Point2D>* pSidebarCameoLocations = nullptr;

		int DGreenAnimationsCount = 0;                         // 0 => default column
		const std::vector<Point2D>* pDGreenLocations = nullptr;

		// Already flattened to per-carrier rows; empty => built-in defaults.
		std::vector<std::vector<Point2D>> BayCameoLocations;
	};

	LayoutConfig ResolveLayoutConfig() const;
	void BuildDefaultBayGrids(int backgroundX, int backgroundY, int cameoWidth, int cameoHeight);

	// -----------------------------------------------------------------------
	// Run / SaveCargo sub-steps
	// -----------------------------------------------------------------------

	// Decomposition of Run(), single call site. Owns the pending->active
	// transition, the quick-click case and the whole drop resolution.
	void UpdateDragState(int buttonID);

	// Final wallet reconciliation. Was duplicated in: SaveCargo() x2.
	// `hasConfiguredMoney` is "a Money= was set on the SW / country / global".
	void SettleMoney(bool addUnusedMoneyToPlayer, bool hasConfiguredMoney);

	// Keeps only the initial units still present in the saved cargo.
	// Was duplicated in: SaveCargo() x2.
	static std::vector<TechnoTypeClass*> RetainInitialUnitsPresentInCargo(
		const std::vector<TechnoTypeClass*>& initialUnits,
		std::vector<TechnoTypeClass*> cargoCopy);
};