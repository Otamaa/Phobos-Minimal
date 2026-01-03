#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Tiberium/Body.h>

#include <Utilities/Macro.h>

#include <CaptureManagerClass.h>
#include <SpawnManagerClass.h>
#include <InfantryClass.h>
#include <InfantryTypeClass.h>

#include <TacticalClass.h>

#include <Misc/Ares/Hooks/Header.h>

#ifdef _OLDHOOKS

ASMJIT_PATCH(0x6F6637, TechnoClass_DrawHealthBar_HideBuildingsPips, 0x5)
{
	enum { SkipDrawPips = 0x6F677D, Continue = 0x0 };

	GET(TechnoClass*, pThis, ESI);

	return TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())
		->HealthBar_HidePips ? SkipDrawPips : Continue;
}


// destroying a building (no health left) resulted in a single green pip shown
// in the health bar for a split second. this makes the last pip red.
ASMJIT_PATCH(0x6F661D, TechnoClass_DrawHealthBar_DestroyedBuilding_RedPip, 0x7)
{
	GET(BuildingClass*, pBld, ESI);
	return (pBld->Health <= 0 || pBld->IsRedHP()) ? 0x6F6628 : 0x6F6630;
}


ASMJIT_PATCH(0x6F64A0, TechnoClass_DrawHealthBar_Hide, 0x5)
{
	enum
	{
		Draw = 0x0,
		DoNotDraw = 0x6F6ABD
	};

	GET(TechnoClass*, pThis, ECX);

	const auto what = pThis->WhatAmI();

	if (what == UnitClass::AbsID)
	{
		const auto pUnit = (UnitClass*)pThis;

		if (pUnit->DeathFrameCounter > 0)
			return DoNotDraw;
	}

	if (what == BuildingClass::AbsID)
	{
		const auto pBld = (BuildingClass*)pThis;

		if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->Firestorm_Wall)
			return DoNotDraw;
	}

	if ((TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->HealthBar_Hide.Get())
		|| pThis->TemporalTargetingMe
		|| pThis->IsSinking
	)
		return DoNotDraw;

	return Draw;
}

#endif

ASMJIT_PATCH(0x6F5E37, TechnoClass_DrawExtras_DrawHealthBar, 0x6)
{
	enum { Permanent = 0x6F5E41, Continue = 0x0 };

	GET(TechnoClass*, pThis, EBP);

	if ((pThis->IsMouseHovering
		|| TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->HealthBar_Permanent)
		&& !MapClass::Instance->IsLocationShrouded(pThis->GetCoords()))
	{
		return Permanent;
	}

	return Continue;
}

ASMJIT_PATCH(0x6D9076, TacticalClass_RenderLayers_DrawBefore, 0x5)// FootClass
{
	GET(TechnoClass*, pTechno, ESI);
	GET(Point2D*, pLocation, EAX);

	if (pTechno->IsSelected && Phobos::Config::EnableSelectBox)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

		if (!pTypeExt->HealthBar_Hide && !pTypeExt->HideSelectBox)
			TechnoExtData::DrawSelectBox(pTechno, pLocation, &DSurface::ViewBounds, true);
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x6D9134, TacticalClass_RenderLayers_DrawBefore, 0x5)// BuildingClass

#ifndef _DrawHP

static bool DrawHPBar(TechnoClass* pThis)
{
	const auto what = pThis->WhatAmI();
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->Is_DriverKilled)
		return false;


	if (what == UnitClass::AbsID)
	{
		const auto pUnit = (UnitClass*)pThis;

		if (pUnit->DeathFrameCounter > 0)
			return false;
	}

	if (what == BuildingClass::AbsID)
	{
		const auto pBld = (BuildingClass*)pThis;

		if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->Firestorm_Wall)
			return false;
	}

	if ((TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->HealthBar_Hide.Get())
		|| pThis->TemporalTargetingMe
		|| pThis->IsSinking
	)
		return false;

	return true;
}

// Helper to draw a single pip
static void DrawSingleHPPip(Point2D* position, ConvertClass* pConvert, SHPStruct* shape, int frameIndex, RectangleStruct* clipRect, BlitterFlags flags)
{
	DSurface::Temp->DrawSHP(
		pConvert,
		shape,
		frameIndex,
		position,
		clipRect,
		flags,
		0, 0, 0,
		1000,
		0, 0, 0, 0, 0
	);
}

// Helper structure for health bar drawing state
struct HealthBarDrawState
{
	int barLength;
	int spacing;
	int pipBrdFrame;
	Point2D pipDelta;
	Point2D pipBrdDelta;
};

// Helper structure for health bar rendering assets
struct HealthBarAssets
{
	SHPStruct* pipShape;
	SHPStruct* bracketShape;
	ConvertClass* convert;
};

// Constants
constexpr BlitterFlags HEALTHBAR_FLAGS = BlitterFlags::bf_400 | BlitterFlags::Centered;
constexpr BlitterFlags PIPBAR_FLAGS = BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered;

// Forward declarations
static void DrawBuildingHealthBar(TechnoClass* techno, Point2D* position, RectangleStruct* clipRect, const HealthBarAssets* assets);
static void DrawUnitHealthBar(TechnoClass* techno, AbstractType unitType, Point2D* position, RectangleStruct* clipRect, const HealthBarAssets* assets);
static bool ShouldDrawPips(TechnoClass* techno);

void __fastcall FakeTechnoClass::__DrawHealthBar_Selection(TechnoClass* techno, discard_t, Point2D* position, RectangleStruct* clipRect, bool unused)
{
	if (!DrawHPBar(techno))
		return;

	AbstractType technoType = techno->WhatAmI();
	TechnoTypeClass* technoTypeClass = GET_TECHNOTYPE(techno);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(technoTypeClass);
	SHPStruct* pips_SHP = pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP());
	pips_SHP = pTypeExt->HealthBarSHP.Get(pips_SHP);
	SHPStruct* pPipsShapeSelected = pTypeExt->HealthBarSHP_Selected.Get(FileSystem::PIPBRD_SHP());
	ConvertClass* pPalette = FileSystem::PALETTE_PAL();

	if (pTypeExt->HealthbarRemap.Get())
		pPalette = techno->GetRemapColour();
	else if (const auto pConvertData = pTypeExt->HealthBarSHP_Palette.GetConvert())
		pPalette = pConvertData;

	// Setup default assets
	HealthBarAssets assets {
		.pipShape = pips_SHP,
		.bracketShape = pPipsShapeSelected,
		.convert = pPalette
	};

	if (techno->IsSelected && Phobos::Config::EnableSelectBox && !pTypeExt->HideSelectBox)
		TechnoExtData::DrawSelectBox(techno, position, clipRect);

	if (technoType == AbstractType::Building) {
		DrawBuildingHealthBar(techno, position, clipRect, &assets);
	} else {
		DrawUnitHealthBar(techno, technoType, position, clipRect, &assets);
	}
}

// Draw health bar for buildings (diagonal bar)
static void DrawBuildingHealthBar(TechnoClass* techno, Point2D* position, RectangleStruct* clipRect, const HealthBarAssets* assets)
{
	BuildingClass* building = (BuildingClass*)techno;
	TechnoTypeClass* technoType = building->Type;

	// Get building dimensions
	CoordStruct dimensions {};
	technoType->Dimension2(&dimensions);

	// Check for custom health bar settings
	int customBarLength = 0;        // Read from technoType->CustomHealthBarLength

	// Calculate corner offsets for isometric projection
	CoordStruct halfDimensions {
		.X = dimensions.X / 2,
		.Y = dimensions.Y / 2,
		.Z = dimensions.Z / 2
	};

	// Top-left corner of building
	CoordStruct cornerCoord {
		.X = halfDimensions.X - dimensions.X,  // -X/2
		.Y = dimensions.Y - halfDimensions.Y,   // Y/2
		.Z = dimensions.Z
	};

	// Calculate the appropriate corner based on health bar side

	Point2D screenCornerTop = TacticalClass::CoordsToScreen(&cornerCoord);

	cornerCoord.Y = -cornerCoord.Y;
	Point2D screenCornerBottom = TacticalClass::CoordsToScreen(&cornerCoord);

	cornerCoord.Z = 0;
	cornerCoord.Y = -cornerCoord.Y;
	Point2D screenBottomRight = TacticalClass::CoordsToScreen(&cornerCoord);

	const int barHeight = customBarLength > 0 ? customBarLength : (screenCornerTop.Y - screenCornerBottom.Y) / 2;

	{ // these are draw before HP itself drawn
		const auto pExt = TechnoExtContainer::Instance.Find(techno);

		if (const auto pShieldData = pExt->Shield.get())
		{
			if (pShieldData->IsAvailable() && !pShieldData->IsBrokenAndNonRespawning())
				pShieldData->DrawShieldBar_Building(barHeight, position, clipRect);
		}

		TechnoExtData::ProcessDigitalDisplays(techno);
	}

	int baseXPosition = screenCornerTop.X + 4 * barHeight + 3;
	int diagonalDirection = 1; // Pips move left as they go down
	// 1 for left (moving left), -1 for right (moving right)

// Calculate how much of the bar should be filled based on health
	double healthRatio = techno->GetHealthPercentage();
	int filledPips = (int)(healthRatio * barHeight);

	// Clamp to valid range
	if (filledPips < 1) filledPips = 1;
	if (filledPips > barHeight) filledPips = barHeight;

	// Determine health bar color based on health percentage
	int colorIndex = 1; // Green

	if (healthRatio <= RulesClass::Instance->ConditionYellow) {
		colorIndex = 2; // Yellow
	}

	if (healthRatio <= RulesClass::Instance->ConditionRed) {
		colorIndex = 4; // Red
	}

	// Base Y offset for diagonal pip placement
	const int baseYOffset = 2 - (2 * barHeight);

	// Draw filled health pips (diagonal from corner)
	if (filledPips > 0)
	{
		int horizontalOffset = 0;  // Moves along horizontal axis (4 pixels per pip)
		int verticalOffset = 0;     // Moves down vertically (2 pixels per pip)

		for (int i = 0; i < filledPips; i++)
		{
			Point2D pipPos {
				.X = position->X + baseXPosition - (diagonalDirection * horizontalOffset),
				.Y = position->Y + screenCornerTop.Y + baseYOffset + 2 - verticalOffset
			};

			DrawSingleHPPip(&pipPos, assets->convert, assets->pipShape, colorIndex, clipRect, HEALTHBAR_FLAGS);
			horizontalOffset += 4;
			verticalOffset -= 2;
		}
	}

	// Draw empty health pips (continuing the diagonal)
	int emptyPips = barHeight - filledPips;
	if (emptyPips > 0)
	{
		int emptyStartOffsetY = -2 * filledPips;
		int emptyStartOffsetX = 4 * filledPips;

		for (int i = 0; i < emptyPips; i++)
		{
			Point2D pipPos {
				.X = position->X + baseXPosition - (diagonalDirection * emptyStartOffsetX),
				.Y = position->Y + screenCornerTop.Y + baseYOffset + 2 - emptyStartOffsetY
			};

			DrawSingleHPPip(&pipPos, assets->convert, assets->pipShape, 0, clipRect, HEALTHBAR_FLAGS);

			emptyStartOffsetY -= 2;
			emptyStartOffsetX += 4;
		}
	}

	// Draw additional pips (ammo, passengers, etc.) if visible
	if (ShouldDrawPips(techno))
	{
		Point2D pipPosition {
			.X = position->X + screenBottomRight.X,
			.Y = position->Y + screenBottomRight.Y
		};

		techno->DrawPipScalePips(&pipPosition, position, clipRect);
	}
}

// Draw health bar for units and infantry (horizontal bar)
static void DrawUnitHealthBar(TechnoClass* techno, AbstractType unitType, Point2D* position, RectangleStruct* clipRect, const HealthBarAssets* assets)
{
	TechnoTypeClass* technoType = GET_TECHNOTYPE(techno);
	bool isInfantry = (unitType == AbstractType::Infantry);
	const auto pExt = TechnoExtContainer::Instance.Find(techno);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(technoType);

	Point2D forPipBrd {
		.X = isInfantry ? 11 : 1,
		.Y = technoType->PixelSelectionBracketDelta - (isInfantry ? 25 : 26) + pTypeExt->HealthBarSHPBracketOffset
	};

	Point2D forPip {
		.X = isInfantry ? -5 : -15 ,
		.Y = technoType->PixelSelectionBracketDelta - (isInfantry ? 24 : 25) + pTypeExt->HealthBarSHPBracketOffset
	};

	HealthBarDrawState drawState {
		.barLength = isInfantry ? 8 : 17,
		.spacing = 2,
		.pipBrdFrame = isInfantry ? 1 : 0,
		.pipDelta = forPip,
		.pipBrdDelta = forPipBrd,
	};

	{ // these are draw before HP itself drawn


		if (const auto pShieldData = pExt->Shield.get())
		{
			if (pShieldData->IsAvailable() && !pShieldData->IsBrokenAndNonRespawning())
				pShieldData->DrawShieldBar_Other(drawState.barLength, position, clipRect);
		}

		TechnoExtData::ProcessDigitalDisplays(techno);
	}

	// Setup drawing parameters based on unit type
	if (techno->IsSelected) {
		Point2D bracketPos = position->operator+(drawState.pipBrdDelta);
		DrawSingleHPPip(&bracketPos, assets->convert, assets->bracketShape, drawState.pipBrdFrame, clipRect, PIPBAR_FLAGS);
	}

	// Allow override of bar length (future extension point)
	// Could check: if (technoType->CustomHealthBarLength > 0) { drawState.barLength = technoType->CustomHealthBarLength; drawState.useCustomLength = true; }

	// Calculate filled health bar length
	double healthRatio = techno->GetHealthPercentage();
	int filledLength = (int)(healthRatio * drawState.barLength);

	// Clamp health bar length
	if (filledLength < 1) filledLength = 1;
	if (filledLength > drawState.barLength) filledLength = drawState.barLength;

	// Determine health bar color frame
	int healthFrameIndex = pTypeExt->HealthBarSHP_HealthFrame->Y; // Green

	if (healthRatio <= RulesClass::Instance->ConditionYellow) {
		healthFrameIndex = pTypeExt->HealthBarSHP_HealthFrame->Z; // Yellow
	}

	if (techno->Health <= 0 || healthRatio <= RulesClass::Instance->ConditionRed) {
		healthFrameIndex = pTypeExt->HealthBarSHP_HealthFrame->X; // Red
	}

	// Draw health bar pips
	for (int i = 0; i < filledLength; i++)
	{
		Point2D pipPos = position->operator+(drawState.pipDelta);
		pipPos += pTypeExt->HealthBarSHP_PointOffset;
		pipPos.X += i * drawState.spacing;
		DrawSingleHPPip(&pipPos, assets->convert, assets->pipShape, healthFrameIndex, clipRect, HEALTHBAR_FLAGS);
	}

	// Draw pips if allied or visible
	if (ShouldDrawPips(techno))
	{
		COMPILETIMEEVAL Point2D pipPosDelta {
			.X = -10 ,
			.Y = 10
		};

		Point2D pipPosition = position->operator+(pipPosDelta);
		techno->DrawPipScalePips(&pipPosition, position, clipRect);
	}
}

// Check if pips should be drawn
static bool ShouldDrawPips(TechnoClass* techno)
{
	TechnoTypeClass* technoType = GET_TECHNOTYPE(techno);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(technoType);

	if (pTypeExt->HealthBar_HidePips)
		return false;

	if (!HouseClass::IsCurrentPlayerObserver())
	{
		const bool showPipScale = pTypeExt->HealthBar_Permanent_PipScale;
		if (!showPipScale && !techno->IsMouseHovering && !techno->IsSelected)
			return true;

		// Always draw if PipsDrawForAll is set
		if (!technoType->PipsDrawForAll)
		{
			// Draw if allied with player
			if (techno->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
			{
				return true;
			}

			// Draw if player can see (radar/spy satellite)
			int playerBit = 1 << HouseClass::CurrentPlayer->ArrayIndex;
			if (techno->DisplayProductionTo & playerBit)
			{
				return true;
			}

			// Special case for mind-controlled buildings
			if (techno->WhatAmI() == AbstractType::Building)
			{
				BuildingClass* building = (BuildingClass*)techno;
				if (building->IsMindControlled())
				{
					return true;
				}
			}

			return false;
		}
	}

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6F64A0, FakeTechnoClass::__DrawHealthBar_Selection)
#endif
