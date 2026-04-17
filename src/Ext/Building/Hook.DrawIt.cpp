#include "Body.h"

#include <Ext/BuildingType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <TacticalClass.h>

//// ─── Tint color override ─────────────────────────────────────────────────────
//// 0x43D386 — replaces the vanilla tint-color block; applies extension palette.
//// Skips 0x43D386 → 0x43D4EB (vanilla airstrike / iron-curtain colour logic).
//ASMJIT_PATCH(0x43D386, BuildingClass_Draw_TintColor, 0x6)
//{
//	enum { SkipGameCode = 0x43D4EB };
//
//	GET(BuildingClass*, pThis, ESI);
//
//	int color = TechnoExtData::ApplyTintColor(pThis, true, true, false);
//	TechnoExtData::ApplyCustomTint(pThis, &color, nullptr);
//	R->EDI(color);
//
//	return SkipGameCode;
//}
//
//// ─── Bib shape hidden when not placed on map ──────────────────────────────────
//// 0x43D874 — skips drawing the bib (foundation shadow) for limboed buildings.
//ASMJIT_PATCH(0x43D874, BuildingClass_Draw_BuildupBibShape, 0x6)
//{
//	enum { DontDrawBib = 0x43D8EE };
//
//	GET(BuildingClass* const, pThis, ESI);
//	return !pThis->ActuallyPlacedOnMap ? DontDrawBib : 0x0;
//}
//
//// ─── ZShapePointMove buildup gate ────────────────────────────────────────────
//// 0x43D6E5 — honours ZShapePointMove_OnBuildup flag from the type extension.
//// Vanilla always applies the Z-shape point offset; extension can suppress it
//// during Construction / Selling missions.
//ASMJIT_PATCH(0x43D6E5, BuildingClass_Draw_ZShapePointMove, 0x5)
//{
//	enum { Apply = 0x43D6EF, Skip = 0x43D712 };
//
//	GET(FakeBuildingClass*, pThis, ESI);
//	GET(Mission, mission, EAX);
//
//	if ((mission != Mission::Selling && mission != Mission::Construction) ||
//		pThis->_GetTypeExtData()->ZShapePointMove_OnBuildup)
//		return Apply;
//
//	return Skip;
//}
//
//// ─── LimboID / IsHideDuringSpecialAnim early-out ─────────────────────────────
//// 0x43D290 — skips rendering for buildings that are limbo-delivered OR whose
////            type has IsHideDuringSpecialAnim set while a special anim plays.
//// Both redirect to 0x43D9D5 (end of draw body before voxel helper call).
//ASMJIT_PATCH(0x43D290, BuildingClass_Draw_LimboDelivered, 0x5)
//{
//	GET(BuildingClass* const, pBuilding, ECX);
//
//	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
//
//	if (pTypeExt->IsHideDuringSpecialAnim &&
//		(pBuilding->Anims[(int)BuildingAnimSlot::Special] ||
//			pBuilding->Anims[(int)BuildingAnimSlot::SpecialTwo] ||
//			pBuilding->Anims[(int)BuildingAnimSlot::SpecialThree]))
//		return 0x43D9D5;
//
//	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID >= 0
//		? 0x43D9D5
//		: 0x0;
//}

// FakeBuildingClass_Draw_It.cpp
// Backport of BuildingClass::Draw_It (0x43D290 – 0x43DA73)
// Draws: gate anim, main body shape, bib, under-door anim.
//
// Hook integrations (all 4 are inlined — no ASMJIT_PATCH blocks needed):
//   0x43D290 | LimboDelivered    | early exit guard        (len=5)
//   0x43D386 | TintColor         | replaces full color block (len=6)
//   0x43D6E5 | ZShapePointMove   | conditional ZShape gate  (len=5)
//   0x43D874 | BuildupBibShape   | ActuallyPlacedOnMap gate (len=6)

// ─── Main function ────────────────────────────────────────────────────────────
void  FakeBuildingClass::_Draw_It(
    Point2D* pPixel,    // arg0
	RectangleStruct* pBound)    // a6
{
	auto* pTypeExt = this->_GetTypeExtData();
	auto* pExt = this->_GetExtData();
    // ═════════════════════════════════════════════════════════════════════════
    // HOOK 0x43D290 | BuildingClass_Draw_LimboDelivered | len=5 → 0x43D9D5
    // Fires at function entry. Exits early for:
    //   (a) buildings whose type hides them when a special anim is playing, OR
    //   (b) limbo-delivered buildings (LimboID >= 0 = placed in limbo by trigger)
    // Both cases redirect to the function epilogue (return without drawing).
    // ═════════════════════════════════════════════════════════════════════════
    {
     
		if (pTypeExt->IsHideDuringSpecialAnim &&
			(this->Anims[(int)BuildingAnimSlot::Special] ||
				this->Anims[(int)BuildingAnimSlot::SpecialTwo] ||
				this->Anims[(int)BuildingAnimSlot::SpecialThree]))
			return;

        if (pExt->LimboID >= 0)
            return;
    }

    // ── 1. Cell coordinate at building origin ────────────────────────────────
    CellStruct cellIdx = this->GetMapCoords();
	BuildingTypeClass* const type = this->Type;

    // ── 2. Shape pointer — bail if no image data ──────────────────────────────
    SHPStruct* shape = this->GetImage();
    if (!shape)
        return;

    // ── 3. Deconstruction: make all anims invisible while selling ─────────────
	auto curMission = this->GetCurrentMission();

	if (this->BState == BStateType::Construction && curMission == Mission::Selling) {
		for (auto& anim : this->Anims) {
			if (anim) {
				anim->Invisible = true;
			}
		}
	}

    // ── 4. Type marked invisible in-game → skip draw entirely ────────────────
    if (type->InvisibleInGame)
        return;

    // ── 5. Locals: z-adjust, z-shape defaults, depth shape ───────────────────
    // NormalZAdjust: per-type vertical offset applied to every draw call height.
    // zShapeX / zShapeY: screen-space anchor for the depth (Z) shadow shape.
    //   Default values (0xC6=198, 0x1BE=446) are overridden by ZShapePointMove.
    int   zAdjust = type->NormalZAdjust;
    SHPStruct* depthShape = FileSystem::BUILDINGZ_SHA();
    bool  hasJetOcc = false;   // JumpJet or BalloonHover occupant flag (v64)

    // ── 6. Check first radio contact for helicopter-pad logic ────────────────
    // If the unit exiting is a JumpJet or BalloonHover type, a different set of
    // deploy / under-door anims is used (roof variant instead of ground variant).
	if (curMission == Mission::Unload){
		if (TechnoClass* contactUnit = this->GetRadioContact()) {
			TechnoTypeClass* unitType = GET_TECHNOTYPE(contactUnit);
			if (unitType->JumpJet || unitType->BalloonHover) {
				hasJetOcc = true;
			}
        }
    }

    // ── 7. Tint / overlay colour ──────────────────────────────────────────────
    // ═════════════════════════════════════════════════════════════════════════
    // HOOK 0x43D386 | BuildingClass_Draw_TintColor | len=6 → 0x43D4EB
    // Completely replaces the vanilla airstrike + iron-curtain colour-packing
    // block (the LABEL_21/22/28/29 goto tangle in raw pseudocode — same pattern
    // as the 43DA80 helper).  Extension handles both tint sources in one call.
    // ═════════════════════════════════════════════════════════════════════════
    int tintColor = TechnoExtData::ApplyTintColor(this, true, true, false);
    TechnoExtData::ApplyCustomTint(this, &tintColor, nullptr);
	CoordStruct center = this->GetCoords();

    // ── 8. Cell-mapped visibility: clear tint if in revealed/mapped area ──────
    {//at building center cell
        const CellStruct centerCell = {
            static_cast<short>(center.X >> 8),
            static_cast<short>(center.Y >> 8)
        };

		auto pCenterCell = MapClass::Instance->GetCellAt(centerCell);

        if (pCenterCell->IsShrouded())
            tintColor = 0;
    }

	auto pBuildingCell = MapClass::Instance->GetCellAt(cellIdx);
    // ── 9. Gate animation path (Mission::Open = 0x18) ────────────────────────
    // War Factory / gated building: draw only the gate shape and return early.
    // Does NOT draw the normal building body or bib.
	// Handle gate/door animations
	auto& door = this->UnloadTimer;
	if (curMission == Mission::Open && (door.IsOpening() || door.IsClosing() || door.IsOpen()))
	{
		// Calculate gate frame based on door state
		int gateFrame = (int)(door.GetCompletePercent()
					* this->Type->GateStages);

		if (door.IsClosing())
		{
			gateFrame = this->Type->GateStages - gateFrame;
		}

		if (door.IsClosed())
		{
			gateFrame = 0;
		}

		if (door.IsOpen())
		{
			gateFrame = this->Type->GateStages - 1;
		}

		// Clamp frame to valid range
		gateFrame = MaxImpl(0, MinImpl(gateFrame, this->Type->GateStages - 1));

		// Add damage frame offset if building is damaged
		if (this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow)
		{
			gateFrame += this->Type->GateStages + 1;
		}

		ZGradient zgrad = ZGradient::Ground;

		if (gateFrame < this->Type->GateStages / 2 || pTypeExt->IsBarGate)
		{
			zgrad = ZGradient::Deg90;
		}

		const int lightLevel = pBuildingCell->Color1.Red;
		const int depthAdjust = Game::AdjustHeight(this->GetZ());

		this->Draw_Object(shape,
			gateFrame,
			pPixel,
			pBound,
			DirType::North,  // rotation
			256,  // scale
			zAdjust - depthAdjust,  // height adjust
			zgrad,  // ZGradient
			1,  // useZBuffer
			lightLevel,
			tintColor,
			0, 0, 0, 0, BlitterFlags::None  // z-shape params
		);

		return;
	}


    // ─────────────────────────────────────────────────────────────────────────
    // ── 10. Main draw path ────────────────────────────────────────────────────
    // ─────────────────────────────────────────────────────────────────────────

    // Optional deploy-anim override while factory is unloading a unit
	if (curMission == Mission::Unload)
	{
		if (hasJetOcc && type->RoofDeployingAnim)
		{
			if (this->Type->RoofDeployingAnim)
			{
				shape = this->Type->RoofDeployingAnim;
				zAdjust = -40;
			}
		}
		else
		{
			if (this->Type->DeployingAnim)
			{
				shape = this->Type->DeployingAnim;
				zAdjust = -20;
			}
		}
	}

    // ═════════════════════════════════════════════════════════════════════════
    // HOOK 0x43D6E5 | BuildingClass_Draw_ZShapePointMove | len=5 → Apply/Skip
    // Vanilla: skip ZShapePointMove for Mission::Construction and Mission::Selling.
    // Extension: ALSO honour ZShapePointMove_OnBuildup flag — when set, the offset
    //            is applied even during construction/selling animations.
    //         _GetTypeExtData() returns FakeBuildingTypeClass extension data.
    // ═════════════════════════════════════════════════════════════════════════
	int zShapeX = 198;
	int zShapeY = 446;

    {
        const bool applyZShape = (curMission != Mission::Construction && curMission != Mission::Selling) || pTypeExt->ZShapePointMove_OnBuildup;

        if (applyZShape)
        {
            zShapeX += type->ZShapePointMove.X;
            zShapeY += type->ZShapePointMove.Y;
        }
    }

    // Compute screen-space depth shadow placement from building tile dimensions
    //         The cell-coord pair from Center_Coord / 256 is passed as input.
    {
        // Reinterpret cell position as the XY input expected by the screen helper
        Point2D screenInput = {
            static_cast<int>(static_cast<short>(center.X >> 8)),
            static_cast<int>(static_cast<short>(center.Y >> 8))
        };

        const int buildH = this->Type->GetFoundationWidth();
        const int buildW = this->Type->GetFoundationHeight(0);

		Point2D screenSize {
			(buildW << 8) - 256 , (buildH << 8) - 256
		};
        
		Point2D screenOffset = TacticalClass::Instance->
			AdjustForZShapeMove(screenSize.X, screenSize.Y);

        zShapeX -= screenOffset.X;
        zShapeY -= screenOffset.Y;

        // Wide buildings (>= 8 tiles) don't use the depth shape
        if (buildW >= 8)
            depthShape = nullptr;
    }

	const int shapeNum = this->GetShapeNumber();

    // ── 11. Main shape draw ───────────────────────────────────────────────────
    // Only draws if the clipping rect has positive height (visible on screen).
    if (pBound->Height > 0)
    {
		const int lightLevel = (int16)this->Type->ExtraLight + pBuildingCell->Color1.Red;
		const int depthAdjust = Game::AdjustHeight(this->GetZ());

		if (!pTypeExt->Firestorm_Wall)
		{
			// Frame selection: min(Shape_Number(), frameCount/2)
			// Shape frame count is stored as a signed word at shape+6.
			const int frameCount = shape->Frames;

			const int displayFrame = (shapeNum >= frameCount / 2)
				? (frameCount / 2)
				: shapeNum;

			this->Draw_Object(
				shape, displayFrame,
				pPixel, pBound,
				DirType::North,
				256,
				zAdjust - depthAdjust,
				ZGradient::Deg90,
				1,
				lightLevel, tintColor,
				depthShape,
				0,
				zShapeX,
				zShapeY,
				BlitterFlags::None);
		}
		else
		{
			this->Draw_Object(shape,
				shapeNum,
				pPixel,
				pBound,
				DirType::North,  // rotation
				256,  // scale
				-1 - depthAdjust,  // height adjust
				ZGradient::Ground,  // ZGradient
				1,  // useZBuffer
				lightLevel,
				tintColor,
				depthShape,
				0,
				zShapeX,
				zShapeY,
				BlitterFlags::None  // flags
			);
		}
    }

    // ── 12. Bib shape (foundation overlay beneath the building) ───────────────
    if (type->BibShape && this->BState != BStateType::Construction)
    {
        // ═══════════════════════════════════════════════════════════════════
        // HOOK 0x43D874 | BuildingClass_Draw_BuildupBibShape | len=6 → 0x43D8EE
        // Vanilla reads pThis->BState (non-zero = placed) to gate bib drawing.
        // Extension replaces that check with ActuallyPlacedOnMap so bib is
        // suppressed for preview / limbo placements that still have BState set.
        // SUSPECT: ActuallyPlacedOnMap may alias BState at +0x534 or be a
        //          separate flag — confirm against YRpp field definitions.
        // ═══════════════════════════════════════════════════════════════════
        if (this->ActuallyPlacedOnMap)
        {
            // Bib uses drawflag1=0 (no depth sort), NOT 2 like the main shape.
            // Height = -1 - Adjust_For_Height (note: more negative than under-door).
			const int lightLevel = (int)(this->Type->ExtraLight + pBuildingCell->Color1.Red);
			const int heightZ = -1 - Game::AdjustHeight(this->GetZ());

            this->Draw_Object(
                type->BibShape, shapeNum,
                pPixel, pBound,
				DirType::North,
				256,
				heightZ,
				ZGradient::Ground,
                1,
				lightLevel, tintColor,
                nullptr, 0, 0, 0, BlitterFlags::None);
        }
    }

    // ── 13. Under-door anim (Mission::Unload only) ────────────────────────────
    // Draws a secondary shape visible under the bay door during unit transit.
    // Uses a different set of anims for JumpJet (roof) vs ground-level units.
    // Height = -Adjust_For_Height (simpler negation; no -1 offset unlike bib).
    if (curMission != Mission::Unload)
        return;

	if (const auto RoofAnim = hasJetOcc ? this->Type->UnderRoofDoorAnim : this->Type->UnderDoorAnim)
	{
		this->Draw_Object(
			RoofAnim,
			this->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow ? 1 : 0,
			pPixel,
			pBound,
			DirType::North, 256,  // rotation, scale
			-Game::AdjustHeight(this->GetZ()),  // height adjust
			ZGradient::Ground, 1,  // ZGradient, useZBuffer
			 (int16)this->Type->ExtraLight + pBuildingCell->Color1.Red,
			tintColor,
			0, 0, 0, 0, BlitterFlags::None  // z-shape params
		);
	}

    return;
}
