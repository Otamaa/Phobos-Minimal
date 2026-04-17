// FakeBuildingClass_Draw_It.cpp
// Backport of BuildingClass::Draw_It (0x43D290 – 0x43DA73)
// Draws: gate anim, main body shape, bib, under-door anim.
//
// Hook integrations (all 4 are inlined — no ASMJIT_PATCH blocks needed):
//   0x43D290 | LimboDelivered    | early exit guard        (len=5)
//   0x43D386 | TintColor         | replaces full color block (len=6)
//   0x43D6E5 | ZShapePointMove   | conditional ZShape gate  (len=5)
//   0x43D874 | BuildupBibShape   | ActuallyPlacedOnMap gate (len=6)

// ─── Shared: cell altitude helper (used in every Techno_Draw_Object call) ────
// Reads the signed word at CellClass+0x10A (Color1.r lighting value).
// VERIFY: Color1.r field identity and sign-extend behaviour.
static FORCEINLINE int CellAlt(const CellClass& cell)
{
	return static_cast<int>(static_cast<short>(cell.Color1.r));
}

// ─── Main function ────────────────────────────────────────────────────────────
static char __fastcall FakeBuildingClass__Draw_It(
	BuildingClass* pThis, void* /*_edx*/,
	Point2D* pPixel,    // arg0
	Rect* pBound)    // a6
{
	// ═════════════════════════════════════════════════════════════════════════
	// HOOK 0x43D290 | BuildingClass_Draw_LimboDelivered | len=5 → 0x43D9D5
	// Fires at function entry. Exits early for:
	//   (a) buildings whose type hides them when a special anim is playing, OR
	//   (b) limbo-delivered buildings (LimboID >= 0 = placed in limbo by trigger)
	// Both cases redirect to the function epilogue (return without drawing).
	// ═════════════════════════════════════════════════════════════════════════
	{
		auto* pTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

		if (pTypeExt->IsHideDuringSpecialAnim &&
			(pThis->Anims[(int)BuildingAnimSlot::Special] ||
				pThis->Anims[(int)BuildingAnimSlot::SpecialTwo] ||
				pThis->Anims[(int)BuildingAnimSlot::SpecialThree]))
			return 0;

		if (BuildingExtContainer::Instance.Find(pThis)->LimboID >= 0)
			return 0;
	}

	// ── 1. Cell coordinate at building origin ────────────────────────────────
	CellStruct cellIdx;
	pThis->Coord_Cell(&cellIdx);  // VERIFY: vtable +0x1B8

	BuildingTypeClass* const type = pThis->Class;  // VERIFY: +0x520

	// ── 2. Shape pointer — bail if no image data ──────────────────────────────
	void* shape = pThis->Get_Image_Data();  // VERIFY: vtable +0x6C
	if (!shape)
		return 0;

	// ── 3. Deconstruction: make all anims invisible while selling ─────────────
	// VERIFY: Kind_Of vtable +0x2C; BState at +0x534; BSTATE_CONSTRUCTION value;
	//         Mission::Selling = 0x13 (MISSION_DECONSTRUCTION in vanilla enum).
	if (pThis &&
		pThis->WhatAmI() == AbstractType::Building &&
		pThis->BState == BSTATE_CONSTRUCTION &&
		pThis->GetCurrentMission() == Mission::Selling)
	{
		for (int i = 0; i < 21; ++i)
		{
			if (pThis->Anims[i])
				pThis->Anims[i]->IsInvisible = true;  // VERIFY: IsInvisible at +0x19D
		}
	}

	// ── 4. Type marked invisible in-game → skip draw entirely ────────────────
	if (type->InvisibleInGame)   // VERIFY: +0x1701
		return 0;

	// ── 5. Locals: z-adjust, z-shape defaults, depth shape ───────────────────
	// NormalZAdjust: per-type vertical offset applied to every draw call height.
	// zShapeX / zShapeY: screen-space anchor for the depth (Z) shadow shape.
	//   Default values (0xC6=198, 0x1BE=446) are overridden by ZShapePointMove.
	// VERIFY: NormalZAdjust at type+0x1520.
	int   zAdjust = type->NormalZAdjust;
	int   zShapeX = 198;
	int   zShapeY = 446;
	void* depthShape = BuildingClass::BuildingDepthShape;
	bool  hasJetOcc = false;   // JumpJet or BalloonHover occupant flag (v64)

	// ── 6. Check first radio contact for helicopter-pad logic ────────────────
	// If the unit exiting is a JumpJet or BalloonHover type, a different set of
	// deploy / under-door anims is used (roof variant instead of ground variant).
	if (pThis->GetCurrentMission() == Mission::Unload)  // 0x10
	{
		auto* occupant = static_cast<TechnoClass*>(pThis->Contact_With_Whom(0));
		if (occupant)
		{
			auto* occType = occupant->Techno_Type_Class();
			// VERIFY: JumpJet at occType+0xD94, BalloonHover at +0xD6A
			if (occType->JumpJet || occType->BalloonHover)
				hasJetOcc = true;
		}
	}

	// ── 7. Tint / overlay colour ──────────────────────────────────────────────
	// ═════════════════════════════════════════════════════════════════════════
	// HOOK 0x43D386 | BuildingClass_Draw_TintColor | len=6 → 0x43D4EB
	// Completely replaces the vanilla airstrike + iron-curtain colour-packing
	// block (the LABEL_21/22/28/29 goto tangle in raw pseudocode — same pattern
	// as the 43DA80 helper).  Extension handles both tint sources in one call.
	// ═════════════════════════════════════════════════════════════════════════
	int tintColor = TechnoExtData::ApplyTintColor(pThis, true, true, false);
	TechnoExtData::ApplyCustomTint(pThis, &tintColor, nullptr);

	// ── 8. Cell-mapped visibility: clear tint if in revealed/mapped area ──────
	{
		CoordStruct center;
		pThis->Center_Coord(&center);  // VERIFY: vtable +0x48

		const CellStruct centerCell = {
			static_cast<short>(center.X >> 8),
			static_cast<short>(center.Y >> 8)
		};
		if (Map[centerCell].Is_Mapped())
			tintColor = 0;
	}

	// ── 9. Gate animation path (Mission::Open = 0x18) ────────────────────────
	// War Factory / gated building: draw only the gate shape and return early.
	// Does NOT draw the normal building body or bib.
	// VERIFY: Mission::Open = 0x18; GateStages at type+0x16F8.
	if (pThis->GetCurrentMission() == Mission::Open)
	{
		const bool gateActive =
			pThis->Door.Is_Door_Opening() ||
			pThis->Door.Is_Door_Closing() ||
			pThis->Door.Is_Door_Open();

		if (gateActive)
		{
			int gateFrame = static_cast<int>(
				pThis->Door.Get_Percent_Complete() * static_cast<double>(type->GateStages));

			if (pThis->Door.Is_Door_Closing())
				gateFrame = type->GateStages - gateFrame;
			if (pThis->Door.Is_Door_Closed())
				gateFrame = 0;
			if (pThis->Door.Is_Door_Open())
				gateFrame = type->GateStages - 1;

			gateFrame = std::min(gateFrame, type->GateStages - 1);
			gateFrame = std::max(gateFrame, 0);

			// Damaged gate: offset into second half of frames (+GateStages+1)
			// VERIFY: ConditionYellow at Rule+0x1700; Health_Ratio() return type
			const int dmgOffset = (pThis->Health_Ratio() <= Rule->ConditionYellow)
				? (type->GateStages + 1) : 0;

			// Gate does NOT add ExtraLight to cellAlt (confirmed from assembly)
			const int cellAlt = CellAlt(Map[cellIdx]);
			const int height = zAdjust - Adjust_For_Height(pThis->Get_Z_Coord());

			// VERIFY: Techno_Draw_Object param order (17 params incl. shape/frame)
			pThis->Techno_Draw_Object(
				shape, gateFrame + dmgOffset,
				pPixel, pBound,
				/*window=*/    0,
				/*dir=*/       256,
				height,
				/*drawflag1=*/ 2,
				/*drawflag2=*/ 1,
				cellAlt, tintColor,
				/*z_shape=*/      nullptr,
				/*z_shape_frame=*/0,
				/*z_shape_x=*/    0,
				/*z_shape_y=*/    0,
				/*a17=*/          0);

			return 0;   // gate path returns immediately
		}
	}

	// ─────────────────────────────────────────────────────────────────────────
	// ── 10. Main draw path ────────────────────────────────────────────────────
	// ─────────────────────────────────────────────────────────────────────────

	// Optional deploy-anim override while factory is unloading a unit
	if (pThis->GetCurrentMission() == Mission::Unload)
	{
		if (hasJetOcc)
		{
			// VERIFY: RoofDeployingAnim at type+0x14FC
			if (type->RoofDeployingAnim)
			{
				shape = type->RoofDeployingAnim;
				zAdjust = -40;   // 0xFFFFFFD8
			}
		}
		else
		{
			// VERIFY: DeployingAnim at type+0x14E4
			if (type->DeployingAnim)
			{
				shape = type->DeployingAnim;
				zAdjust = -20;   // 0xFFFFFFEC
			}
		}
	}

	// ═════════════════════════════════════════════════════════════════════════
	// HOOK 0x43D6E5 | BuildingClass_Draw_ZShapePointMove | len=5 → Apply/Skip
	// Vanilla: skip ZShapePointMove for Mission::Construction and Mission::Selling.
	// Extension: ALSO honour ZShapePointMove_OnBuildup flag — when set, the offset
	//            is applied even during construction/selling animations.
	// VERIFY: ZShapePointMove at type+0x1530 (Point2D with short X/Y offsets).
	//         _GetTypeExtData() returns FakeBuildingTypeClass extension data.
	// ═════════════════════════════════════════════════════════════════════════
	{
		const Mission mission = pThis->GetCurrentMission();
		const bool applyZShape =
			(mission != Mission::Selling && mission != Mission::Construction) ||
			pThis->_GetTypeExtData()->ZShapePointMove_OnBuildup;

		if (applyZShape)
		{
			zShapeX = type->ZShapePointMove.X + 198;
			zShapeY = type->ZShapePointMove.Y + 446;
		}
	}

	// Compute screen-space depth shadow placement from building tile dimensions
	// VERIFY: Tactical::XY_To_Screen_Pixels takes world-XY (not pixel) coords.
	//         The cell-coord pair from Center_Coord / 256 is passed as input.
	{
		CoordStruct center;
		pThis->Center_Coord(&center);

		// Reinterpret cell position as the XY input expected by the screen helper
		Point2D screenInput = {
			static_cast<int>(static_cast<short>(center.X >> 8)),
			static_cast<int>(static_cast<short>(center.Y >> 8))
		};

		const int buildH = BuildingTypeClass::Height(type, /*bib=*/0);
		const int buildW = BuildingTypeClass::Width(type);

		Point2DStruct screenSize;
		screenSize.X = (buildW << 8) - 256;
		screenSize.Y = (buildH << 8) - 256;

		const Point2D* pScreen = Tactical::XY_To_Screen_Pixels(
			TacticalMap, &screenInput, &screenSize);
		zShapeX -= pScreen->X;
		zShapeY -= pScreen->Y;

		// Wide buildings (>= 8 tiles) don't use the depth shape
		if (buildW >= 8)
			depthShape = nullptr;
	}

	// ── 11. Main shape draw ───────────────────────────────────────────────────
	// Only draws if the clipping rect has positive height (visible on screen).
	if (pBound->Height > 0)
	{
		// Frame selection: min(Shape_Number(), frameCount/2)
		// Shape frame count is stored as a signed word at shape+6.
		// VERIFY: ShapeFile format — NumFrames at byte offset 6.
		const int frameCount = static_cast<int>(*reinterpret_cast<short*>(
			static_cast<char*>(shape) + 6));
		const int shapeNum = pThis->Shape_Number();
		const int displayFrame = (shapeNum >= frameCount / 2)
			? (frameCount / 2)
			: shapeNum;

		// Main shape DOES add ExtraLight (confirmed from assembly at 43D824)
		// VERIFY: ExtraLight at type+0x1548 (signed word)
		const int cellAlt = CellAlt(Map[cellIdx])
			+ static_cast<int>(static_cast<short>(type->ExtraLight));
		const int height = zAdjust - Adjust_For_Height(pThis->Get_Z_Coord());

		pThis->Techno_Draw_Object(
			shape, displayFrame,
			pPixel, pBound,
			/*window=*/    0,
			/*dir=*/       256,
			height,
			/*drawflag1=*/ 2,
			/*drawflag2=*/ 1,
			cellAlt, tintColor,
			depthShape,
			/*z_shape_frame=*/0,
			zShapeX,
			zShapeY,
			/*a17=*/0);
	}

	// ── 12. Bib shape (foundation overlay beneath the building) ───────────────
	// VERIFY: BibShape at type+0x1518; BState (build state) at this+0x534.
	if (type->BibShape && pThis->BState)
	{
		// ═══════════════════════════════════════════════════════════════════
		// HOOK 0x43D874 | BuildingClass_Draw_BuildupBibShape | len=6 → 0x43D8EE
		// Vanilla reads pThis->BState (non-zero = placed) to gate bib drawing.
		// Extension replaces that check with ActuallyPlacedOnMap so bib is
		// suppressed for preview / limbo placements that still have BState set.
		// SUSPECT: ActuallyPlacedOnMap may alias BState at +0x534 or be a
		//          separate flag — confirm against YRpp field definitions.
		// ═══════════════════════════════════════════════════════════════════
		if (pThis->ActuallyPlacedOnMap)
		{
			// Bib uses drawflag1=0 (no depth sort), NOT 2 like the main shape.
			// Height = -1 - Adjust_For_Height (note: more negative than under-door).
			const int cellAlt = CellAlt(Map[cellIdx])
				+ static_cast<int>(static_cast<short>(type->ExtraLight));
			const int height = -1 - Adjust_For_Height(pThis->Get_Z_Coord());

			pThis->Techno_Draw_Object(
				type->BibShape, pThis->Shape_Number(),
				pPixel, pBound,
				/*window=*/    0,
				/*dir=*/       256,
				height,
				/*drawflag1=*/ 0,    // ← bib differs from main shape here
				/*drawflag2=*/ 1,
				cellAlt, tintColor,
				nullptr, 0, 0, 0, 0);
		}
	}

	// ── 13. Under-door anim (Mission::Unload only) ────────────────────────────
	// Draws a secondary shape visible under the bay door during unit transit.
	// Uses a different set of anims for JumpJet (roof) vs ground-level units.
	// Height = -Adjust_For_Height (simpler negation; no -1 offset unlike bib).
	if (pThis->GetCurrentMission() != Mission::Unload)
		return 0;

	const bool isDamaged = (pThis->Health_Ratio() <= Rule->ConditionYellow);
	const int  underFrame = isDamaged ? 1 : 0;
	const int  underCellAlt = CellAlt(Map[cellIdx])
		+ static_cast<int>(static_cast<short>(type->ExtraLight));
	const int  underHeight = -Adjust_For_Height(pThis->Get_Z_Coord());

	if (hasJetOcc)
	{
		// VERIFY: UnderRoofDoorAnim at type+0x1504
		if (!type->UnderRoofDoorAnim)
			return 0;

		pThis->Techno_Draw_Object(
			type->UnderRoofDoorAnim, underFrame,
			pPixel, pBound,
			/*window=*/    0,
			/*dir=*/       256,
			underHeight,
			/*drawflag1=*/ 0,
			/*drawflag2=*/ 1,
			underCellAlt, tintColor,
			nullptr, 0, 0, 0, 0);
	}
	else
	{
		// VERIFY: UnderDoorAnim at type+0x14EC
		if (!type->UnderDoorAnim)
			return 0;

		pThis->Techno_Draw_Object(
			type->UnderDoorAnim, underFrame,
			pPixel, pBound,
			/*window=*/    0,
			/*dir=*/       256,
			underHeight,
			/*drawflag1=*/ 0,
			/*drawflag2=*/ 1,
			underCellAlt, tintColor,
			nullptr, 0, 0, 0, 0);
	}

	return 0;
}