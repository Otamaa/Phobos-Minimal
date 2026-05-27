#include "Body.h"

#include "Body.h"

// FakeBuildingClass_DrawVoxels.cpp
// Backport of BuildingClass_draw_43DA80 (0x43DA80 – 0x43E79F)
// Handles: occupant exit-drawing, door animation, voxel turret/barrel rendering
//
// Goto elimination: LABEL_23 / LABEL_24 replaced by BuildTintColor() helper.

// ─────────────────────────────────────────────────────────────────────────────
// Helper: replicate the DSurface color-mode packing that was hidden inside the
//         LABEL_23 / LABEL_24 goto tangle.
//
// Assembly analysis (0x43DC64–0x43DCCA and 0x43DD22–0x43DD8A):
//   ColorMode == 2  → three OR contributions (RGB565 + two 16-bit steps)
//   ColorMode == 1  → two OR contributions  (            two 16-bit steps)
//   ColorMode >= 3  → one OR contribution   (            single step only )
// ─────────────────────────────────────────────────────────────────────────────
static OPTIONALINLINE int __cdecl BuildTintColor(uint8_t R, uint8_t G, uint8_t B)
{
	const int mode = DSurface::ColorMode();
	int result = 0;

	if (mode == 2)
	{
		// RGB565 contribution  (G<<11 | R<<5 | B)
		result = (((int)G << 6 | (int)R) << 5) | (int)B;
	}

	if (mode <= 2)
	{
		// Mid-bit step — runs for mode 1 and 2
		result |= ((((int)G << 5) | ((int)R >> 1)) << 6) | (int)B;
	}

	// Base step — all modes
	result |= ((((int)G << 5) | ((int)R >> 1)) << 5) | (int)B;

	return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: convert BAM DirStruct → 5-bit facing index used for voxel lookup.
//   Assembly pattern: ((*fc.Current(&tmp) >> 10) + 1) >> 1) & 0x1F
// ─────────────────────────────────────────────────────────────────────────────
static FORCEINLINE unsigned int FacingTo5Bit(FacingClass& fc, DirStruct& tmp)
{
	return ((*reinterpret_cast<DWORD*>(fc.Current(&tmp)) >> 10u) + 1u) >> 1u & 0x1Fu;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: shared "draw a voxel with the default matrix" call pattern that
//         repeats 5 times in the original function.
// ─────────────────────────────────────────────────────────────────────────────
static FORCEINLINE void DrawVoxelWithDefaultMatrix(
	BuildingClass* pThis,
	Matrix3D_Union& matSrc,       // source orientation matrix
	Matrix3D_Union& matResult,    // scratch for operator* result
	Matrix3D_Union& matMul,       // scratch for Get_Default_Matrix result
	VoxelStruct* pVoxel,
	int               frame,
	int               key,
	VoxelIndexKey* pIdxKey,      // VERIFY: exact type name
	Rect* pBound,
	Point2D& drawPt,
	int               cellAlt,
	DWORD             tintColor)
{
	Matrix3D_Union* pDefault = used_Voxel_Get_Default_Matrix(&matMul);
	Matrix3D_Union* pFinal = operator*(&matResult, pDefault, &matSrc);

	// VERIFY: vtable slot 0x444 / Draw_Voxel signature
	pThis->Draw_Voxel(pVoxel, frame, key, pIdxKey, pBound, &drawPt,
					  pFinal, cellAlt, reinterpret_cast<Matrix3D_Union*>(tintColor), 0);
}

/**/
// ─────────────────────────────────────────────────────────────────────────────
// Main function
// ─────────────────────────────────────────────────────────────────────────────
static void __fastcall FakeBuildingClass__DrawVoxels(
	BuildingClass* pThis, void* /*_edx*/,
	Point2D* pPixel,          // [esp+4] = a6
	Rect* pBound)          // [esp+8] = arg4
{
	// ── Locals ───────────────────────────────────────────────────────────────
	CellStruct     cellIdx;
	Point2D        drawOffset;
	DirStruct      facingTmp;
	Vector3        turretTranslation;   // v115 — turret offset extracted from matB
	Vector3        barrelTranslation;   // v116 — barrel save
	float          angle = 0.0f;
	DWORD          tintColor = 0;       // var_110 — accumulated tint (0 = none)

	Matrix3D_Union matB;        // B        — primary working matrix
	Matrix3D_Union matAux;      // var_9C   — secondary / combined matrix
	Matrix3D_Union matResult;   // arg0     — operator* retstr
	Matrix3D_Union matMul;      // a        — Get_Default_Matrix out

	// ── 1. Grab cell coordinate at building origin ────────────────────────────
	// VERIFY: vtable slot Coord_Cell = +0x1B4
	pThis->Coord_Cell(&cellIdx);

	BuildingTypeClass* type = pThis->Class;

	// ── 2. Draw occupants exiting through the factory door ───────────────────
	if (pThis->GetCurrentMission() == MISSION_UNLOAD)
	{
		const bool doorActive =
			pThis->Door.Is_Door_Opening() ||
			pThis->Door.Is_Door_Open() ||
			pThis->Door.Is_Door_Closed() ||
			pThis->Door.Is_Door_Closing();

		if (doorActive && pThis->IsTethered)
		{
			const int contactCount = pThis->Radio.VectorMax;  // +0xE8
			for (int i = 0; i < contactCount; ++i)
			{
				// VERIFY: official name RadioClass::Contact_With_Whom
				TechnoClass* occupant = static_cast<TechnoClass*>(
					pThis->Contact_With_Whom(i));

				if (!occupant)
					continue;
				if (occupant->IsInLimbo)
					continue;
				// VERIFY: vtable +0x2C = Kind_Of / WhatAmI; value 6 = RTTI_BUILDING
				if (occupant->WhatAmI() == AbstractType::Building)
					continue;

				// Retrieve the occupant's target coordinate
				CoordStruct targetCoord;
				// VERIFY: vtable +0x4C = __Target_Coord; second arg 0 = no sub-index
				occupant->Target_Coord(&targetCoord, 0);

				// Fog-of-war check (only in multiplayer scenario mode)
				// Skip entirely when: no window, debug map, or FoW disabled
				const bool skipFogCheck =
					!MainWindow ||
					Debug_Map_DEBUGDEBUG ||
					!(Scen->Specials.Bitfield & 0x1000u);

				if (!skipFogCheck)
				{
					// Check occupant's current world coord
					Vector3 occupantPos;
					occupantPos.X = *reinterpret_cast<float*>(&occupant->Coord.X);
					occupantPos.Y = *reinterpret_cast<float*>(&occupant->Coord.Y);
					occupantPos.Z = *reinterpret_cast<float*>(&occupant->Coord.Z);

					if (Map.IsLocationFogged(occupantPos))
						continue;
					// VERIFY: targetCoord passed as Vector3 — may need explicit cast
					if (Map.IsLocationFogged(*reinterpret_cast<Vector3*>(&targetCoord)))
						continue;
				}

				// Draw the tether / connector line
				CoordStruct lineCoord;
				// VERIFY: vtable +0xAC = Center_Coord / mcoord_41BE00
				Tactical::Coord_To_Pixel_Adjusted_To_Viewport(
					TacticalMap,
					occupant->Center_Coord(&lineCoord),
					pPixel);

				// VERIFY: vtable +0x114 = Draw_It (passing a7.Y and arg4)
				occupant->Draw_It(&drawOffset.Y, pBound);
			}
		}
	}

	// ── 3. Compute tint color ─────────────────────────────────────────────────
	// var_110 starts at 0; non-zero means a colored overlay should be applied.
	tintColor = 0;

	if (pThis->Airstrike)   // +0x294
	{
		// Palette entry: Rule offset arithmetic → Rule + LaserTargetColor*3 + base
		// VERIFY: exact palette struct layout at Rule+0x18A4 and +0x1874/5/6
		const int   idx = Rule->LaserTargetColor; // +0x18A4
		const auto* palette = reinterpret_cast<const uint8_t*>(Rule) + idx * 3;
		const uint8_t R = palette[0x1875u];
		const uint8_t G = palette[0x1874u];
		const uint8_t B = palette[0x1876u];
		tintColor |= (DWORD)BuildTintColor(R, G, B);
	}

	// VERIFY: vtable +0x160 = IsIronCurtained  ;  +0x1C4 = ForceShielded
	if (pThis->IsIronCurtained() && pThis->ForceShielded == 1)
	{
		const int   idx = Rule->ForceShieldColor; // +0x18B0
		const auto* palette = reinterpret_cast<const uint8_t*>(Rule) + idx * 3;
		const uint8_t R = palette[0x1875u];
		// SUSPECT: var_11A / var_119 assignments are SWAPPED relative to the
		//          airstrike block above — check assembly at 0x43DD0E–0x43DD1E.
		const uint8_t B = palette[0x1874u];   // swapped
		const uint8_t G = palette[0x1876u];   // swapped
		tintColor |= (DWORD)BuildTintColor(R, G, B);
	}

	// ── 4. Cell-mapped visibility override ───────────────────────────────────
	// If the building's cell is mapped/visible, clear the tint.
	{
		// VERIFY: vtable +0x48 = Center_Coord; result divided to get CellStruct
		CoordStruct center;
		pThis->Center_Coord(&center);
		const CellStruct cellPos = { (short)(center.X >> 8), (short)(center.Y >> 8) };
		if (Map[cellPos].Is_Mapped())
			tintColor = 0;
	}

	// ── 5. Door animation drawing ─────────────────────────────────────────────
	// VERIFY: vtable +0x184 = GetCurrentMission; +0x520 offset = this->Class
	if (pThis->GetCurrentMission() == MISSION_UNLOAD && type->DoorAnim)
	{
		const bool doorVisible =
			pThis->Door.Is_Door_Closed() ||
			pThis->Door.Is_Door_Opening() ||
			pThis->Door.Is_Door_Closing() ||
			pThis->Door.Is_Door_Open();

		if (doorVisible)
		{
			// Compute animation frame from door completion percentage
			int doorFrame = (int)(pThis->Door.Get_Percent_Complete()
								  * (double)type->DoorStages);  // +0xF00

			if (pThis->Door.Is_Door_Closing())
				doorFrame = type->DoorStages - doorFrame;
			if (pThis->Door.Is_Door_Closed())
				doorFrame = 0;

			doorFrame = std::min(doorFrame, type->DoorStages - 1);
			doorFrame = std::max(doorFrame, 0);

			// Damaged door uses a second set of frames (+DoorStages offset)
			// VERIFY: Health_Ratio() <= ConditionYellow (+0x1700 in Rule)
			if (pThis->Health_Ratio() <= Rule->ConditionYellow && type->DamagedDoor)
				doorFrame += type->DoorStages;

			// VERIFY: vtable +0x1D0 = Get_Z_Coord
			const int zCoord = pThis->Get_Z_Coord();
			const int height = -5 - Adjust_For_Height(zCoord);

			// CellClass::Color1.r at offset +0x10A used as lighting hint
			const int cellAlt = (int)(short)Map[cellIdx].Color1.r; // VERIFY field identity

			// VERIFY: Techno_Draw_Object signature — 17 params
			pThis->Techno_Draw_Object(
				type->DoorAnim, doorFrame,
				pPixel, pBound,
				/*WindowNumberType=*/0,
				/*DirType=*/256,
				height,
				/*aaa=*/0, /*drawflag1=*/0, /*drawflag2=*/0,
				cellAlt,
				0, 0, 0, 0, 0, 0);

			// Signal that the door has fully closed
			if (pThis->Door.Is_Door_Closing() && doorFrame == 0)
				pThis->IsToDisplay = true;  // +0x80
		}
	}

	// ── 6. Voxel turret path ──────────────────────────────────────────────────
	if (type->TurretAnimIsVoxel)   // +0x16C5
	{
		const MissionType mission = pThis->GetCurrentMission();
		const MissionType missionQueue = pThis->MissionQueue;
		const int         stage = pThis->stage.Stage;

		bool inConstruction = false;
		if ((mission == MISSION_CONSTRUCTION || missionQueue == MISSION_CONSTRUCTION) &&
			stage < type->Anims[0].Start + type->Anims[0].Count - 1)
		{
			inConstruction = true;
		}

		// Skip rendering while building is mid-construction unless Artillary flag set
		// VERIFY: +0x16CA = Artillary flag
		const bool skip = (mission == MISSION_DECONSTRUCTION && stage > 0) || inConstruction;

		if (!skip || type->Artillary)
		{
			// Facing indices for voxel lookup keys
			unsigned int turretFacingIdx, barrelFacingIdx;
			if (VPL_Read)
			{
				// Use the same facing for both (barrel shares turret facing in this path)
				turretFacingIdx = FacingTo5Bit(pThis->PrimaryFacing, facingTmp);
				barrelFacingIdx = turretFacingIdx;
			}
			else
			{
				turretFacingIdx = ~0u;
				barrelFacingIdx = ~0u;
			}

			// Animation frame offsets (modded by motion library frame count)
			int turretAnimFrame = 0;
			int barrelAnimFrame = 0;

			if (type->TurretVoxel.motlib)   // +0xB8 voxlib, +0xBC motlib
				turretAnimFrame = pThis->TurretAnimFrame % type->TurretVoxel.motlib->FrameCount; // +0x148

			if (type->BarrelVoxel.motlib)   // +0xC4
				barrelAnimFrame = pThis->TurretAnimFrame % type->BarrelVoxel.motlib->FrameCount;

			// Pack frame + facing index into single int key for caching
			int turretKey = (turretAnimFrame << 16) | (int)turretFacingIdx;
			int barrelKey = (barrelAnimFrame << 16) | (int)barrelFacingIdx;

			Matrix3D::Make_Identity(&matB);

			if (type->TurretVoxel.voxlib)
			{
				// ── Build turret body matrix ──────────────────────────────────
				// Rotate Z by turret facing angle
				// VERIFY: dbl_7E4408 = -0.1963495408493621 (= -π/16)
				const int facingStep = (int)(FacingTo5Bit(pThis->PrimaryFacing, facingTmp)) - 8;
				angle = (float)facingStep * -0.1963495408493621f;
				Matrix3D::Rotate_Z(&matB, angle);

				// HOOK 0x43E0C4 | BuildingClass_Draw_43DA80_TurretMultiOffset | len=5 → 0x43E0E8
				// Vanilla: Translate_X(&matB, TurretOffset / 8)
				// Extension: full 3D translate using TechnoTypeExt::TurretOffset vector.
				// TurretMultiOffsetOneByEightMult = 1/8 equivalent applied per-axis.
				{
					const auto* pTypeExt = TechnoTypeExtContainer::Instance.Find(type);
					const auto& nOffs = pTypeExt->TurretOffset;
					const float ox = static_cast<float>(nOffs->X * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);
					const float oy = static_cast<float>(nOffs->Y * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);
					const float oz = static_cast<float>(nOffs->Z * TechnoTypeExtData::TurretMultiOffsetOneByEightMult);
					matB.Translate(ox, oy, oz);
				}

				// Extract translation column from matB for the auxiliary matrix
				// matAux = copy of matB, then translated by -Row[n].W (negated translation)
				turretTranslation.X = -matB.Row[0].W;
				turretTranslation.Y = -matB.Row[1].W;
				turretTranslation.Z = -matB.Row[2].W;

				qmemcpy(&matAux, &matB, sizeof(matAux));
				Matrix3D::Translate(&matAux, &turretTranslation);

				// ── Recoil adjustments ────────────────────────────────────────
				// VERIFY: TurretRecoil at +0x3F0/3EC ; BarrelRecoil at +0x410/40C
				if (pThis->TurretRecoil.UseMatrixX)   // +0x3F0 (non-zero = active)
				{
					angle = -pThis->TurretRecoil.MatrixXOffset;  // +0x3EC, negated
					Matrix3D::Translate_X(&matB, angle);
					turretKey = -1;  // invalidate voxel cache key
				}
				if (pThis->BarrelRecoil.UseMatrixX)   // +0x410
				{
					angle = -pThis->BarrelRecoil.MatrixXOffset;  // +0x40C, negated
					Matrix3D::Translate_X(&matAux, angle);
					turretKey = -1;  // SUSPECT: should this be barrelKey? mirrors assembly but seems wrong
				}

				// ── Build barrel facing matrix ────────────────────────────────
				// VERIFY: BarrelFacing = pThis+0x370
				const int barrelStep = (int)(FacingTo5Bit(pThis->BarrelFacing, facingTmp)) - 8;
				angle = -(float)(barrelStep) * -0.1963495408493621f;  // double-negation = positive
				Matrix3D::Rotate_Y(&matAux, angle);

				// The translation stored at turretOffset is passed as the vector to Translate
				// VERIFY: v115 holds the 3-component turret translation; filled earlier from matB
				//         but assembly re-reads matB.Row[n].W after the Translate_X calls.
				//         Safest: re-extract after all recoil transforms.
				turretTranslation.X = matB.Row[0].W;
				turretTranslation.Y = matB.Row[1].W;
				turretTranslation.Z = matB.Row[2].W;
				Matrix3D::Translate(&matAux, &turretTranslation);

				// ── Draw position in screen space ────────────────────────────
				// VERIFY: BuildingAnim[9] = slot index 9; Position = Point2D offset
				//         +0x520 = type; +0x11E0/11E4 = BuildingAnim[9].Position.X/Y
				drawOffset.X = pPixel->X + type->BuildingAnim[9].Position.X;
				drawOffset.Y = type->BuildingAnim[9].Position.Y + pPixel->Y;

				// ── Determine barrel draw order relative to turret ────────────
				// 4-direction quadrant from primary facing (3 bits → 2 bits → 0..3)
				//   Quadrant 1 or 2 (side views):  draw barrel AFTER turret (in front)
				//   Quadrant 0 or 3 (front/back):  draw barrel BEFORE turret (behind)
				const unsigned int facingQuad =
					((*reinterpret_cast<DWORD*>(pThis->PrimaryFacing.Current(&facingTmp))
						>> 13u) + 1u) >> 1u & 3u;

				// var_11A in original: 0 = barrel drawn before turret, 1 = after
				bool barrelDrawnAfter = false;

				if (facingQuad > 0u && facingQuad < 3u)
				{
					// Side-facing: check facing again more precisely
					const unsigned int facingQuad2 =
						((*reinterpret_cast<DWORD*>(pThis->PrimaryFacing.Current(&facingTmp))
							>> 13u) + 1u) >> 1u & 3u;

					if (facingQuad2 < 3u)
					{
						barrelDrawnAfter = true;  // barrel drawn in second pass (after turret)
					}
				}

				if (!barrelDrawnAfter)
				{
					// Draw barrel first (behind turret) — only if barrel voxel present
					if (type->BarrelVoxel.voxlib && type->BarrelVoxel.motlib)
					{
						const int cellAlt = (int)(short)Map[cellIdx].Color1.r;
						DrawVoxelWithDefaultMatrix(pThis, matAux, matResult, matMul,
							&type->BarrelVoxel, /*frame=*/0, turretKey,
							&type->VoxelIndex4,         // VERIFY: VoxelIndex4 field
							pBound, drawOffset, cellAlt, tintColor);
					}
				}

				// Draw turret body (always)
				{
					const int cellAlt = (int)(short)Map[cellIdx].Color1.r;
					DrawVoxelWithDefaultMatrix(pThis, matB, matResult, matMul,
						&type->TurretVoxel, turretAnimFrame, turretKey,
						&type->VoxelIndex2,             // VERIFY: VoxelIndex2 field
						pBound, drawOffset, cellAlt, tintColor);
				}

				// Draw barrel after turret if flagged (in front)
				if (barrelDrawnAfter && type->BarrelVoxel.voxlib && type->BarrelVoxel.motlib)
				{
					const int cellAlt = (int)(short)Map[cellIdx].Color1.r;
					DrawVoxelWithDefaultMatrix(pThis, matAux, matResult, matMul,
						&type->BarrelVoxel, /*frame=*/0, turretKey,
						&type->VoxelIndex4,
						pBound, drawOffset, cellAlt, tintColor);
				}
			}
			else if (type->BarrelVoxel.voxlib && type->BarrelVoxel.motlib)
			{
				// ── No turret voxel, barrel-only path (loc_43E41D) ────────────
				// Copy translation out of identity matB, translate by its negation,
				// apply rotations, then translate back.
				barrelTranslation.X = matB.Row[0].W;
				barrelTranslation.Y = matB.Row[1].W;
				barrelTranslation.Z = matB.Row[2].W;

				Vector3 negBarrel = { -barrelTranslation.X,
									  -barrelTranslation.Y,
									  -barrelTranslation.Z };
				Matrix3D::Translate(&matB, &negBarrel);

				const int pStep = (int)(FacingTo5Bit(pThis->PrimaryFacing, facingTmp)) - 8;
				angle = (float)pStep * -0.1963495408493621f;
				Matrix3D::Rotate_Z(&matB, angle);

				const int bStep = (int)(FacingTo5Bit(pThis->BarrelFacing, facingTmp)) - 8;
				angle = -(float)(bStep) * -0.1963495408493621f;
				Matrix3D::Rotate_Y(&matB, angle);

				Matrix3D::Translate(&matB, &barrelTranslation);

				drawOffset.X = pPixel->X + type->BuildingAnim[9].Position.X;
				drawOffset.Y = type->BuildingAnim[9].Position.Y + pPixel->Y;

				const int cellAlt = (int)(short)Map[cellIdx].Color1.r;
				DrawVoxelWithDefaultMatrix(pThis, matB, matResult, matMul,
					&type->BarrelVoxel, barrelAnimFrame, barrelKey,
					&type->VoxelIndex4,
					pBound, drawOffset, cellAlt, tintColor);
			}
		}
	}
	else if (type->BarrelAnimIsVoxel)   // +0x16C6
	{
		// ── 7. Barrel-only voxel (non-turret building, e.g. artillery) ────────
		const MissionType mission = pThis->GetCurrentMission();
		const MissionType missionQueue = pThis->MissionQueue;
		const int         stage = pThis->stage.Stage;

		bool inConstruction = false;
		if ((mission == MISSION_CONSTRUCTION || missionQueue == MISSION_CONSTRUCTION) &&
			stage < type->Anims[0].Start + type->Anims[0].Count - 1)
		{
			inConstruction = true;
		}

		const bool skip = (mission == MISSION_DECONSTRUCTION && stage > 0) || inConstruction;

		if (!skip)
		{
			// Build base matrix from BuildingClass helper
			// VERIFY: BuildingClass_voxel_458810 fills the matrix from turret state
			qmemcpy(&matAux, BuildingClass_voxel_458810(pThis, &matResult), sizeof(matAux));

			// Compute a rotated 5-bit facing index with an additional static offset
			// VERIFY: someoffset_818CB0 = static per-building rotation bias
			// Assembly: (someoffset + facing) & 0x8000001F with signed fixup = smod 32
			const unsigned int rawFacing = FacingTo5Bit(pThis->PrimaryFacing, facingTmp);
			int facingMod = (int)((someoffset_818CB0 + rawFacing) & 0x1Fu);
			if (facingMod < 0) { --facingMod; facingMod |= ~0x1F; ++facingMod; } // asm sign-fixup

			const bool turretInFront = (facingMod <= 16);
			// setle al in assembly: 1 if facingMod <= 0x10, i.e., turret drawn before barrel

			AnimClass* anim9 = pThis->Anims[9];  // VERIFY: Anims[9] = turret overlay anim

			// First render pass: turret anim drawn before barrel (if in front)
			if (turretInFront && anim9)
			{
				if (DisableInvisibleBeforeRender)
					anim9->IsInvisible = false;        // +0x19D
				anim9->Render(&ViewportBounds_TacPixel, /*a2=*/1, /*a3=*/0);
				anim9->IsInvisible = true;
			}

			drawOffset.X = pPixel->X + type->BuildingAnim[9].Position.X;
			drawOffset.Y = type->BuildingAnim[9].Position.Y + pPixel->Y;

			const int cellAlt = (int)(short)Map[cellIdx].Color1.r;
			DrawVoxelWithDefaultMatrix(pThis, matAux, matResult, matMul,
				&type->BarrelVoxel, /*frame=*/0, /*key=*/-1,
				&type->VoxelIndex1,             // VERIFY: VoxelIndex1 field
				pBound, drawOffset, cellAlt, tintColor);

			// Second render pass: turret anim drawn after barrel (if behind)
			if (!turretInFront && anim9)
			{
				if (DisableInvisibleBeforeRender)
					anim9->IsInvisible = false;
				anim9->Render(&ViewportBounds_TacPixel, /*a2=*/1, /*a3=*/0);
				anim9->IsInvisible = true;
			}
		}
	}
}
