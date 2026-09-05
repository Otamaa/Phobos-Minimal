#include "Body.h"

#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>

#include <TacticalClass.h>

ASMJIT_PATCH(0x423855, AnimClass_DrawIt_ShadowLocation, 0x7)
{
	enum { SkipGameCode = 0x42385D };

	GET(AnimClass*, pThis, ESI);
	GET(Point2D*, pLocation, EDI);

	int zCoord = pThis->GetZ();

	if (auto const pUnit = cast_to<UnitClass*>(pThis->OwnerObject)) {
		// If deploying anim is played in air, cast shadow on ground.
		if (pUnit->DeployAnim == pThis && pUnit->GetHeight() > 0) {
			auto const pCell = pUnit->GetCell();
			auto const coords = pCell->GetCenterCoords();
			*pLocation = TacticalClass::Instance->CoordsToClient(coords);
			zCoord = coords.Z;
		}
	}

	R->EAX(zCoord);
	return SkipGameCode;
}

ASMJIT_PATCH(0x423654, AnimClass_DrawIt_Tiled_Interval, 0x5)
{
	GET(FakeAnimClass*, pThis, ESI);
	GET(RectangleStruct*, pBounds, EAX);
	GET(int*, pValue, EDI);

	int height = pBounds->Height;

	auto const pTypeExt = pThis->_GetTypeExtData();

	if (pTypeExt->Tiled_Interval > 0)
		height = pTypeExt->Tiled_Interval;

	R->EAX(height);
	R->ECX(*pValue);
	return 0x423659;
}


// 0x422CD8 is in an alternate code path only used by anims with ID RING1, unused normally but covering it just because

ASMJIT_PATCH(0x423122, AnimClass_DrawIt_DrawOffset, 0x6)
{
	GET(FakeAnimClass* const, pThis, ESI);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x110, 0x4));

	auto const pTypeExt = pThis->_GetTypeExtData();
	pLocation->X += pTypeExt->XDrawOffset;

	bool const applyX = pTypeExt->XDrawOffset_ApplyBracketWidth;
	bool const applyY = pTypeExt->YDrawOffset_ApplyBracketHeight;

	if ((applyX || applyY) && pThis->OwnerObject && pThis->OwnerObject->AbstractFlags & AbstractFlags::Techno)
	{
		// Hardcoded in shield healthbar code as well.
		constexpr int SHIELD_HEALTHBAR_OFFSET = -3;
		auto const pTechno = static_cast<TechnoClass*>(pThis->OwnerObject);
		bool const invertX = pTypeExt->XDrawOffset_InvertBracketShift;
		bool const invertY = pTypeExt->YDrawOffset_InvertBracketShift;

		if (auto const pBuilding = cast_to<BuildingClass*>(pTechno))
		{
			auto const pType = pBuilding->Type;
			auto const pos = TechnoExtData::GetBuildingSelectBracketPosition(pBuilding, BuildingSelectBracketPosition::Top);

			if (applyY && ((pType->Height >= 0 && !invertY) || (pType->Height < 0 && invertY)))
				pLocation->Y = pos.Y + pTypeExt->YDrawOffset_BracketAdjust_Buildings.Get(pTypeExt->YDrawOffset_BracketAdjust);

			if (applyX)
			{
				int const width = pBuilding->Type->GetFoundationWidth();
				int const shift = static_cast<int>(Unsorted::CellWidthInPixels * (width / 2.0) * (invertX ? -1 : 1));
				pLocation->X = pos.X + shift + pTypeExt->XDrawOffset_BracketAdjust_Buildings.Get(pTypeExt->XDrawOffset_BracketAdjust);
			}
		}
		else
		{
			auto const pType = pTechno->GetTechnoType();
			auto const horizontalPos = invertX ? HorizontalPosition::Left : HorizontalPosition::Right;
			auto const pos = TechnoExtData::GetFootSelectBracketPosition(pTechno, Anchor(horizontalPos, VerticalPosition::Top));

			if (applyY && ((pType->PixelSelectionBracketDelta <= 0 && !invertY) || (pType->PixelSelectionBracketDelta > 0 && invertY)))
				pLocation->Y = pos.Y + pType->PixelSelectionBracketDelta + pTypeExt->YDrawOffset_BracketAdjust;

			if (applyX)
				pLocation->X = pos.X + pTypeExt->XDrawOffset_BracketAdjust;
		}

		if (applyY)
		{
			if (auto const pShield = TechnoExtContainer::Instance.Find(pTechno)->ShieldEntity.get())
			{
				auto const pShieldType = pShield->GetType();

				if (pShield->IsAvailable() && !pShield->IsBrokenAndNonRespawning() && (pShield->GetHealthRatio() > 0.0 || !pShieldType->Pips_HideIfNoStrength))
				{
					if ((pShieldType->BracketDelta <= 0 && !invertY) || (pShieldType->BracketDelta > 0 && invertY))
						pLocation->Y += pShieldType->BracketDelta + SHIELD_HEALTHBAR_OFFSET;
				}
			}
		}
	}

	*pLocation += pThis->_GetExtData()->AEDrawOffset;

	return 0;
}ASMJIT_PATCH_AGAIN(0x422CD8, AnimClass_DrawIt_DrawOffset, 0x6)

#ifdef fullbackport 
void FakeAnimClass::_Draw_It(Point2D* arg_0, RectangleStruct* a6)
{
	// === D3D RING1 special path ===
	if (Game::bDirect3DIsUseable() && ZBuffer::Instance() && ObjectClass_SameName(&this->o, "RING1"))
	{
		int started = this->stage.Timer.Started;
		int rate = this->stage.Rate;
		int delay = this->stage.Timer.DelayTime;

		if (started != -1)
			delay = (Unsorted::CurrentFrame() - started >= delay) ? 0 : delay - (Unsorted::CurrentFrame() - started);

		int progress = rate + this->stage.Stage * rate - delay;
		int totalDuration = rate * ((int(__thiscall*)(AnimClass*))this->o.a.vftable->t.r.m.Commence)(this);
		int radius = progress + 8;

		int rightX = arg_0->X + 2 * radius;
		int leftX = arg_0->X - 2 * radius;
		int bottomY = arg_0->Y + radius;
		int topY = arg_0->Y - radius;

		int szFull = (totalDuration - progress) << 8;

		// Alpha: fades to zero at end of animation
		int alphaRaw = szFull / totalDuration;
		int alpha = std::clamp(alphaRaw, 0, 255);

		// Brightness: ramps up during first third, ramps down after
		int brightnessRaw;
		if (progress >= totalDuration / 3)
			brightnessRaw = szFull / (totalDuration - totalDuration / 3);
		else
			brightnessRaw = (progress << 8) / (totalDuration / 3);
		int brightness = std::clamp(brightnessRaw, 0, 255);
		int green = 2 * brightness / 3;

		// Blue: similar envelope but over two thirds
		int twoThirds = 2 * totalDuration / 3;
		int blueRaw = (progress >= twoThirds)
			? szFull / (totalDuration - twoThirds)
			: (progress << 8) / twoThirds;
		auto blue = (uint8_t)std::clamp(blueRaw, 0, 255);

		// Z-depth
		int   zCoord = this->o.a.vftable->t.r.m.o.Get_Z_Coord((FootClass*)this);
		auto  zBase = (int16_t)(ZBufferPTR->Bounds.Y + ZBufferPTR->MaxValue
					   - arg_0->Y - ViewportBounds_TacPixel.Y
					   + this->_ZAdjust + this->Class->YDrawOffset
					   - Adjust_For_Height(zCoord) - 2);
		float szTop = (float)(uint16_t)(zBase + radius) * 0.000015259022f;
		float szBottom = (float)(uint16_t)(zBase - radius) * 0.000015259022f;

		CD3DTriangle in1, in2;	
		in1.Set_Color((uint8_t)alpha, (uint8_t)green, blue);
		in2.Set_Color((uint8_t)alpha, (uint8_t)green, blue);

		float fLeftX = (float)leftX;
		float fTopY = (float)topY;
		float fBottomY = (float)bottomY;
		float fRightX = (float)rightX;

		// Triangle fan forming a quad
		in1.Set_Coords(0, fLeftX, fTopY, szTop, 0.0f, 0.0f);
		in1.Set_Coords(1, fLeftX, fBottomY, szBottom, 0.0f, 1.0f);
		in1.Set_Coords(2, fRightX, fBottomY, szBottom, 1.0f, 1.0f);
		in2.Set_Coords(0, fLeftX, fTopY, szTop, 0.0f, 0.0f);
		in2.Set_Coords(1, fRightX, fBottomY, szBottom, 1.0f, 1.0f);
		in2.Set_Coords( 2, fRightX, fTopY, szTop, 1.0f, 0.0f);

	
		DSurface::CD3DTriangleInstance->Add(&in1);
		DSurface::CD3DTriangleInstance->Add(&in2);
		return;
	}

	// === Standard 2D rendering ===

	if ((unsigned int)CurrentFPS < GetMinFrameRate() && (int)this->Class->DetailLevel > 1)
		return;
	if (this->IsInvisible)
		return;

	AnimTypeClass* pType = this->Class;
	if (pType->DetailLevel > Options.DetailLevel || (this->_IsFogged && pType->ShouldFogRemove))
		return;

	auto* shape = (ShapeCache*)this->o.a.vftable->t.r.m.o.Get_Image_Data(&this->o);
	if (!shape)
		return;

	int    animFlags = this->__AnimFlags;
	int    currentStage = this->stage.Stage;
	int    frameIdx = currentStage + pType->Start;

	// Temporal transparency
	if (this->__UnderTemporal)
		animFlags |= pType->DoubleThick ? (BF_TRANS_50 | BF_TRANS_25) : BF_TRANS_50;

	// --- Translucency (was goto LABEL_46 / LABEL_47) ---
	if (pType->TranslucencyDetailLevel <= Options.DetailLevel)
	{
		if (pType->Translucent)
		{
			if (this->__Translucency_timing_178 >= 15)
				return;
			double stageD = (double)currentStage;
			double endD = (double)(int)pType->End;
			if (stageD > endD * 0.6) animFlags |= BF_TRANS_50 | BF_TRANS_25;
			else if (stageD > endD * 0.4) animFlags |= BF_TRANS_50;
			else if (stageD > endD * 0.2) animFlags |= BF_TRANS_25;
		}
		else
		{
			int translucency = pType->Translucency;
			if (translucency > 0)
			{
				if (this->__Translucency_timing_178 >= 15)
					return;
				if (translucency == 25) animFlags |= BF_TRANS_25;
				else if (translucency == 50) animFlags |= BF_TRANS_50;
				else if (translucency == 75) animFlags |= BF_TRANS_50 | BF_TRANS_25;
				// other non-zero values: no blending flag
			}
			else
			{
				// Dynamic timing-based fade (e.g. during death sequence)
				int timing = this->__Translucency_timing_178;
				if (timing != 0)
				{
					if (timing > 15) return;
					if (timing > 5)  animFlags |= BF_TRANS_50;  // 6-15 → 50%
					else             animFlags |= BF_TRANS_25;  // 1-5  → 25%
				}
			}
		}
	}

	// Warp flag (applied after translucency is decided)
	if (!(animFlags & BF_DARKEN))
		BYTE1(animFlags) |= BF_TRANS_WARP;

	// --- Drawer / tint color selection (was goto LABEL_77 / LABEL_78) ---
	int height = this->o.a.vftable->t.r.m.o.Get_Height((TechnoClass*)&this->o);
	int tintColor = 1000;  // default: full brightness
	int drawer;

	if (pType->IsVeins)
	{
		drawer = (int)ColorSchemes.Vector[PlayerPtr->RemapColor]->Drawer;
		if (!pType->UseNormalLight)
		{
			CoordStruct* coord = this->o.a.vftable->t.r.m.o.mcoord_41BE00(this, &a3_scratch);
			tintColor = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, coord)->Color1.g;
		}
	}
	else if (this->__lighting__celldraw_196)
	{
		CoordStruct* coord = this->o.a.vftable->t.r.m.o.mcoord_41BE00(this, &a3_scratch);
		CellStruct   cell = { (int16_t)(coord->X / 256), (int16_t)(coord->Y / 256) };
		CellClass* pCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cell);

		if (!pCell->TileDrawer)
			CellClass::Init_Drawer(pCell, 0, 0x10000, 0, 1000, 1000, 1000);

		drawer = (int)pCell->TileDrawer;
		if (!pType->UseNormalLight)
			tintColor = pCell->Color1.g;
	}
	else if (this->_CellDrawer)
	{
		drawer = this->_CellDrawer;
		if (!pType->UseNormalLight)
			tintColor = this->__TintColor;
	}
	else
	{
		drawer = (int)AnimDrawer;
		if (pType->AltPalette)
			drawer = (int)(*ColorSchemes.Vector)->Drawer;
		if (!pType->UseNormalLight)
		{
			CoordStruct* coord = this->o.a.vftable->t.r.m.o.mcoord_41BE00((AbstractClass*)this, (CoordStruct*)&a3_scratch);
			CellStruct   cell = { (int16_t)(coord->X / 256), (int16_t)(coord->Y / 256) };
			tintColor = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &cell)->Color1.r;
		}
	}

	// --- HasExtras overlay (drawn before main sprite) ---
	if (this->__HasExtras)
	{
		int      yAdj = Adjust_For_Height(height);
		Point2D  ePos = { arg_0->X, pType->YDrawOffset + yAdj + arg_0->Y };
		int      zCoord = this->o.a.vftable->t.r.m.o.Get_Z_Coord((FootClass*)this);
		int      zAdjust = Adjust_For_Height(zCoord);
		CC_Draw_Shape(&TempSurface->xs.s, (ConvertClass*)drawer, shape, frameIdx,
					  &ePos, a6, BF_2000 | BF_400 | BF_CENTER | BF_DARKEN,
					  0, pType->YDrawOffset - zAdjust, 0, 1000, 0, 0, 0, 0, 0);
	}

	// --- Building anim color overlay (was goto LABEL_99) ---
	int overlayColor = 0;
	if (this->__IsBuildingAnim)
	{
		CoordStruct* centerCoord = ((CoordStruct * (__thiscall*)(void*))this->o.a.vftable->t.r.m.o.a.Center_Coord)(this);
		BuildingClass* pBuilding = CellClass::Cell_Building(MapClass::operator[](&Map.sc.t.sb.p.r.d.m, centerCoord));
		if (pBuilding)
		{
			if (pBuilding->t.__Airstrike)
				overlayColor |= PackDSurfaceColor(Rule->LaserTargetColor);

			if (pBuilding->t.r.m.o.a.vftable->t.r.m.o.mAbstractClass_IsIronCurtained(&pBuilding->t)
				&& pBuilding->t.__ForceShielded == 1)
			{
				overlayColor |= PackDSurfaceColor(Rule->ForceShieldColor);
			}
		}
	}

	// Fog-of-war: suppress overlay if cell is mapped (player can see it)
	{
		CoordStruct* centerCoord = ((CoordStruct * (__thiscall*)(void*))this->o.a.vftable->t.r.m.o.a.Center_Coord)(this);
		CellStruct   fogCell = { (int16_t)(centerCoord->X / 256), (int16_t)(centerCoord->Y / 256) };
		CellClass* pFogCell = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &fogCell);
		if (CellClass::Is_Mapped(pFogCell))
			overlayColor = 0;
	}

	// --- Shape drawing ---
	if (pType->Tiled)
	{
		// Repeating tile strip (walks up the screen until past the top)
		Rect frameBounds;
		ShapeCache::Get_Frame_Bounds(shape, &frameBounds, 0);

		int  tileSize = frameBounds.Height;
		int  halfTile = tileSize / 2;
		int  drawX = arg_0->X;
		int  drawY = arg_0->Y - halfTile;

		int  zCoord = this->o.a.vftable->t.r.m.o.Get_Z_Coord((FootClass*)this);
		int  zOffset = this->_ZAdjust + pType->YDrawOffset - Adjust_For_Height(zCoord) - 50;
		BYTE1(animFlags) |= ShapeFlags_Type_20;

		bool exitAfterThis = false;
		do
		{
			Point2D tilePos = { drawX, drawY + pType->YDrawOffset };
			CC_Draw_Shape(&TempSurface->xs.s, AnimDrawer, shape, frameIdx,
						  &tilePos, &ViewportBounds_TacPixel, (BlitterFlags)animFlags,
						  0, zOffset, 2, tintColor, overlayColor, 0, 0, 0, 0);

			if (drawY < 0)
				exitAfterThis = true;

			drawY -= tileSize;
			zOffset -= tileSize + halfTile;
		}
		while (!exitAfterThis);
	}
	else if (pType->Flat)
	{
		Point2D flatPos = { arg_0->X, pType->YDrawOffset + arg_0->Y };
		int zCoord = this->o.a.vftable->t.r.m.o.Get_Z_Coord((FootClass*)this);
		int zOffset = this->_ZAdjust + pType->YDrawOffset - Adjust_For_Height(zCoord) - 3;
		BYTE1(animFlags) |= ShapeFlags_Type_20;
		CC_Draw_Shape(&TempSurface->xs.s, (ConvertClass*)drawer, shape, frameIdx,
					  &flatPos, a6, (BlitterFlags)animFlags,
					  0, zOffset, 0, tintColor, overlayColor, 0, 0, 0, 0);
	}
	else
	{
		Point2D drawPos = { arg_0->X, pType->YDrawOffset + arg_0->Y };
		int zCoord = this->o.a.vftable->t.r.m.o.Get_Z_Coord((FootClass*)this);
		int zOffset = this->_ZAdjust + pType->YDrawOffset - Adjust_For_Height(zCoord);
		BYTE1(animFlags) |= ShapeFlags_Type_20;

		CC_Draw_Shape(&TempSurface->xs.s, (ConvertClass*)drawer, shape, frameIdx,
					  &drawPos, a6, (BlitterFlags)animFlags,
					  0, zOffset - 2, 2, tintColor, overlayColor, 0, 0, 0, 0);

		if (pType->Shadow)
		{
			int frameCount = shape->CurrentHeader.FrameCount;
			int shadowZ = this->o.a.vftable->t.r.m.o.Get_Z_Coord((FootClass*)this);
			int shadowOff = -2 - Adjust_For_Height(shadowZ);
			BlitterFlags shadowFlags = (BlitterFlags)
				((animFlags & ~(ShapeFlags_Type_4 | ShapeFlags_2))
				 | ShapeFlags_Type_400 | SHAPE_CENTER | ShapeFlags_1);
			CC_Draw_Shape(&TempSurface->xs.s, (ConvertClass*)drawer, shape,
						  frameIdx + frameCount / 2,
						  (Point2D*)arg_0, a6, shadowFlags,
						  0, shadowOff, 0, 1000, 0, 0, 0, 0, 0);
		}
	}
}
#endif