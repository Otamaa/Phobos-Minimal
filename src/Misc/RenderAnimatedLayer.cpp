// Helper function: Determine if redraw is needed and calculate offsets
struct RedrawContext
{
	int offsetX;
	int offsetY;
	bool needsFullRedraw;
	bool shouldSkipToSurface;
	bool shouldSkipToRedraws;
};

RedrawContext Tactical::DetermineRedrawMode(Tactical* this, bool& drawbool, TacticalRenderMode mode, bool boundsChanged)
{
	RedrawContext ctx = { 0, 0, false, false, false };

	ctx.offsetX = this->__LastTacticalPos.X - this->__TacticalPos.X;
	ctx.offsetY = this->__LastTacticalPos.Y - this->__TacticalPos.Y;

	if (mode != TacticalRenderMode_0_ALL)
	{
		if (!tacticaldrawflag_B0E63C)
		{
			// Continue to check drawbool
		}
		else
		{
			drawbool = true;
			ctx.needsFullRedraw = true;
		}
	}
	else
	{
		if (abs32(ctx.offsetX) < ViewportBounds_TacPixel.Width &&
			abs32(ctx.offsetY) < ViewportBounds_TacPixel.Height)
		{
			tacticaldrawflag_B0E63C = 0;
		}
		else
		{
			drawbool = true;
			tacticaldrawflag_B0E63C = 1;
			ctx.needsFullRedraw = true;
		}
	}

	if (ctx.needsFullRedraw)
	{
		ctx.offsetX = 0;
		ctx.offsetY = 0;
	}

	// Determine skip conditions
	if (mode == TacticalRenderMode_MOVING_ANIMATING)
	{
		ctx.shouldSkipToSurface = true;
		return ctx;
	}

	if (drawbool)
	{
		return ctx; // Will do full redraw
	}

	if (this->drawbool_byteD7C)
	{
		if (!boundsChanged)
		{
			if (!this->__TacticalPosUpdated__IsToRedraw &&
				DirtyAreaStructsVector.ActiveCount == 0 &&
				this->__TacticalPos.X == this->__LastTacticalPos.X &&
				this->__TacticalPos.Y == this->__LastTacticalPos.Y)
			{
				ctx.shouldSkipToSurface = true;
			}
		}
		return ctx;
	}

	if (boundsChanged)
	{
		return ctx; // Will do full redraw
	}

	if (DirtyAreaStructsVector.ActiveCount > 0)
	{
		if (!this->__TacticalPosUpdated__IsToRedraw &&
			DirtyAreaStructsVector.ActiveCount == 0 &&
			this->__TacticalPos.X == this->__LastTacticalPos.X &&
			this->__TacticalPos.Y == this->__LastTacticalPos.Y)
		{
			ctx.shouldSkipToSurface = true;
		}
		return ctx;
	}

	if (!this->__TacticalPosUpdated__IsToRedraw)
	{
		if (this->_TacticalCoord.X == this->_LastTacticalCoord.X &&
			this->_TacticalCoord.Y == this->_LastTacticalCoord.Y)
		{
			ctx.shouldSkipToSurface = true;
		}
	}

	return ctx;
}

// Helper function: Handle surface blitting and buffer operations
DSurface* Tactical::HandleSurfaceOperations(Tactical* this, DSurface* surface,
											 int offsetX, int offsetY,
											 bool drawbool, TacticalRenderMode mode)
{
	int absOffsetX = abs32(offsetX);
	int absOffsetY = abs32(offsetY);

	RectangleStruct srcRect;
	srcRect.X = ViewDimensions.X + (offsetX < 0 ? -offsetX : 0);
	srcRect.Y = ViewDimensions.Y + (offsetY < 0 ? -offsetY : 0);
	srcRect.Width = ViewDimensions.Width - absOffsetX;
	srcRect.Height = ViewDimensions.Height - absOffsetY;

	RectangleStruct dstRect;
	dstRect.X = ViewDimensions.X + (offsetX > 0 ? offsetX : 0);
	dstRect.Y = ViewDimensions.Y + (offsetY > 0 ? offsetY : 0);
	dstRect.Width = ViewDimensions.Width - absOffsetX;
	dstRect.Height = ViewDimensions.Height - absOffsetY;

	if (offsetX || offsetY)
	{
		if (mode == TacticalRenderMode_0_ALL || mode == TacticalRenderMode_3_ALL)
		{
			CompositeSurface->xs.s.vftble->Copy_From__RSR(CompositeSurface, &dstRect, TileSurface, &srcRect, 0, 1);
			ZBuffer_7BCB50(ZBufferPTR, -offsetX, -offsetY, 65535);
			ABuffer::Blit_At(ABufferPTR, -offsetX, -offsetY, 127);

			// Swap surfaces
			DSurface* temp = CompositeSurface;
			CompositeSurface = TileSurface;
			TileSurface = temp;
			surface = CompositeSurface;
			TempSurface = CompositeSurface;
		}
	}
	else
	{
		if (drawbool && (mode == TacticalRenderMode_0_ALL || mode == TacticalRenderMode_3_ALL))
		{
			ZBuffer::Fill(ZBufferPTR, 0xFFFF);
			ABuffer::Fill(ABufferPTR, 127);
		}
	}

	return surface;
}

// Helper function: Calculate scroll regions for terrain rendering
struct ScrollRegions
{
	RectangleStruct leftRegion;
	RectangleStruct rightRegion;
	RectangleStruct topRegion;
	RectangleStruct bottomRegion;
	RectangleStruct centerRegion;
	RectangleStruct horizontalRegion;
	RectangleStruct verticalRegion;
};

ScrollRegions Tactical::CalculateScrollRegions(Tactical* this, int absOffsetX, int absOffsetY)
{
	ScrollRegions regions;
	memset(&regions, 0, sizeof(regions));

	bool scrolledRight = this->__LastTacticalPos.X > this->__TacticalPos.X;
	bool scrolledLeft = this->__LastTacticalPos.X < this->__TacticalPos.X;
	bool scrolledDown = this->__LastTacticalPos.Y > this->__TacticalPos.Y;
	bool scrolledUp = this->__LastTacticalPos.Y < this->__TacticalPos.Y;

	// Left edge region
	RectangleStruct leftRect;
	leftRect.X = ViewportBounds_TacPixel.X;
	leftRect.Y = ViewportBounds_TacPixel.Y;
	leftRect.Width = absOffsetX;
	leftRect.Height = ViewportBounds_TacPixel.Height;
	regions.leftRegion = *Intersect(&regions.leftRegion, &leftRect, &ViewportBounds_TacPixel, 0, 0);

	// Right edge region
	RectangleStruct rightRect;
	rightRect.X = ViewportBounds_TacPixel.Width + ViewportBounds_TacPixel.X - absOffsetX;
	rightRect.Y = ViewportBounds_TacPixel.Y;
	rightRect.Width = absOffsetX;
	rightRect.Height = ViewportBounds_TacPixel.Height;
	regions.rightRegion = *Intersect(&regions.rightRegion, &rightRect, &ViewportBounds_TacPixel, 0, 0);

	// Calculate horizontal offset for top/bottom regions
	int horizX;
	if (scrolledRight)
	{
		horizX = absOffsetX + ViewportBounds_TacPixel.X;
	}
	else if (scrolledLeft)
	{
		horizX = ViewportBounds_TacPixel.X;
	}
	else
	{
		horizX = ViewportBounds_TacPixel.X;
	}

	int horizWidth = (scrolledRight || scrolledLeft) ?
		(ViewportBounds_TacPixel.Width - absOffsetX) :
		ViewportBounds_TacPixel.Width;

	// Top region
	RectangleStruct topRect;
	topRect.X = horizX;
	topRect.Y = ViewportBounds_TacPixel.Y;
	topRect.Width = horizWidth;
	topRect.Height = absOffsetY;
	regions.topRegion = *Intersect(&regions.topRegion, &topRect, &ViewportBounds_TacPixel, 0, 0);

	// Bottom region
	RectangleStruct bottomRect;
	bottomRect.X = horizX;
	bottomRect.Y = ViewportBounds_TacPixel.Height + ViewportBounds_TacPixel.Y - absOffsetY;
	bottomRect.Width = horizWidth;
	bottomRect.Height = absOffsetY;
	regions.bottomRegion = *Intersect(&regions.bottomRegion, &bottomRect, &ViewportBounds_TacPixel, 0, 0);

	// Center region (area that doesn't need redraw)
	regions.centerRegion.X = ViewportBounds_TacPixel.X + (scrolledRight ? absOffsetX : 0);
	regions.centerRegion.Y = ViewportBounds_TacPixel.Y + (scrolledDown ? absOffsetY : 0);
	regions.centerRegion.Width = ViewportBounds_TacPixel.Width - absOffsetX;
	regions.centerRegion.Height = ViewportBounds_TacPixel.Height - absOffsetY;

	// Select which regions to use based on scroll direction
	if (scrolledRight)
	{
		regions.horizontalRegion = regions.leftRegion;
	}
	else if (scrolledLeft)
	{
		regions.horizontalRegion = regions.rightRegion;
	}

	if (scrolledDown)
	{
		regions.verticalRegion = regions.topRegion;
	}
	else if (scrolledUp)
	{
		regions.verticalRegion = regions.bottomRegion;
	}

	return regions;
}

// Helper function: Render terrain layers
void Tactical::RenderTerrainLayers(Tactical* this, bool drawbool, int offsetX, int offsetY)
{
	int absOffsetX = abs32(offsetX);
	int absOffsetY = abs32(offsetY);

	ScrollRegions regions = CalculateScrollRegions(this, absOffsetX, absOffsetY);

	Tactical::Render_Objects_Near_Shroud(this, drawbool, offsetX, offsetY, &regions.centerRegion);
	Tactical::Render_Shroud(this, &regions.horizontalRegion, &regions.verticalRegion, &regions.centerRegion, drawbool);
	Tactical::Render_Tiles(this, &regions.horizontalRegion, &regions.verticalRegion, drawbool);
	Tactical::Render_Fog(this, &regions.horizontalRegion, &regions.verticalRegion, drawbool);
	Tactical::Render_Overlays(this, &regions.horizontalRegion, &regions.verticalRegion, drawbool);
	Tactical::Render_Terrain_Objects(this, &regions.horizontalRegion, &regions.verticalRegion, drawbool);
	Tactical::Render_Cell_Shadows(this, &regions.horizontalRegion, &regions.verticalRegion, drawbool);
	Tactical::Render_Buildings(this, &regions.horizontalRegion, &regions.verticalRegion, drawbool);

	// Clear dirty areas
	ClearDirtyAreas();
}

// Helper function: Clear dirty area vector
void Tactical::ClearDirtyAreas()
{
	int activeCount = DirtyAreaStructsVector.ActiveCount;

	for (int i = activeCount - 1; i >= 0; --i)
	{
		if (i < activeCount)
		{
			--activeCount;
			DirtyAreaStructsVector.ActiveCount = activeCount;

			// Shift remaining elements
			for (int j = i; j < activeCount; ++j)
			{
				void* src = DirtyAreaStructsVector.Vector_Item + (j + 1) * 20;
				void* dst = DirtyAreaStructsVector.Vector_Item + j * 20;
				qmemcpy(dst, src, 0x14);
			}
		}
	}
}

// Helper function: Draw techno action lines and control links
void Tactical::DrawTechnoOverlays(Tactical* this, bool inPlanningMode)
{
	if (TechnoVector.ActiveCount <= 0)
		return;

	for (int i = 0; i < TechnoVector.ActiveCount; ++i)
	{
		TechnoClass* techno = TechnoVector.Vector_Item[i];

		if (!HouseClass::Player_Has_Control(techno->House))
		{
			// Check for psychic detection
			if (TechnoClass_Psychic_Detection(techno) &&
				techno &&
				(techno->r.m.o.a.TargetBitfield[0] & 4) != 0 &&
				!techno->r.m.o.IsInLimbo)
			{
				FootClass_draw_dashed_4DC340(techno);
			}

			if (inPlanningMode)
				continue;
		}
		else if (inPlanningMode)
		{
			continue;
		}

		// Draw action lines for selected units
		if (techno->r.m.o.IsSelected && ActionLinesEnabled)
		{
			techno->r.m.o.a.vftable->t.Draw_Action_Lines(techno, 0, 0);
		}

		// Draw capture manager links
		if (techno->r.m.o.Strength)
		{
			CaptureManagerClass* captureManager = techno->__CaptureManager;
			if (captureManager && CaptureManagerClass::Should_Draw_Link(captureManager))
			{
				CaptureManagerClass::Draw_Control_Links(captureManager);
			}

			if (techno->r.m.o.Strength)
			{
				TechnoClass* originalOwner = techno->__OriginalOwner;
				if (originalOwner)
				{
					CaptureManagerClass* ownerCapture = originalOwner->__CaptureManager;
					if (ownerCapture && CaptureManagerClass::Should_Draw_Link(ownerCapture))
					{
						CaptureManagerClass::Draw_Control_Links(ownerCapture);
					}
				}
			}
		}

		// Draw airstrike target indicator
		DrawAirstrikeIndicator(techno);
	}
}

// Helper function: Draw airstrike target indicator
void Tactical::DrawAirstrikeIndicator(TechnoClass* techno)
{
	AirstrikeClass* airstrike = techno->__Airstrike;
	if (!airstrike)
		return;

	if (airstrike->StrikeOwner != techno)
		return;

	TechnoClass* target = airstrike->AirstrikeTarget;
	if (!target)
		return;

	if (target->r.m.o.a.vftable->t.r.m.o.a.Kind_Of(&target->r.m.o.a) != RTTI_BUILDING)
		return;

	DWORD time = timeGetTime() >> timeShiftValue;
	CoordStruct* targetCenter = target->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(target);

	int baseX = targetCenter->X;
	int baseY = targetCenter->Y;
	int baseZ = targetCenter->Z;

	// Calculate oscillating position
	int oscillatingZ = (int)(FastMath::Sin((float)time) * 15.0f + baseZ);
	int oscillatingX = (int)(FastMath::Cos((float)time * 0.37f) * 15.0f + baseX);

	CoordStruct* strikeCoord = techno->r.m.o.a.vftable->t.r.m.o.mcoord_4263D0(techno);

	CoordStruct targetCoord;
	targetCoord.X = oscillatingX;
	targetCoord.Y = baseY;
	targetCoord.Z = oscillatingZ;

	TechnoClass_draw_705860(techno, strikeCoord->X, strikeCoord->Y, strikeCoord->Z, &targetCoord);
}

// Helper function: Render timers (mission timer, superweapons, blackout)
void Tactical::RenderTimers(Tactical* this)
{
	int timerIndex = 0;

	// Mission timer
	if (Scen->MissionTimer.Started != -1)
	{
		int remaining = CalculateTimerRemaining(&Scen->MissionTimer);
		ColorScheme* color = ColorSchemes.Vector_Item[PlayerPtr->RemapColor];
		Print_Timer_On_Tactical(timerIndex++, color, remaining / 15, Scen->MissionTimerTextCSF, 0, 0);
	}

	// Superweapon timers
	for (int i = 0; i < ShownSupers.ActiveCount; ++i)
	{
		SuperClass* super = ShownSupers.Vector_Item[i];

		if (super->IsSuspended && !Session.Type)
		{
			int remaining = CalculateTimerRemaining(&super->Control);
			if (remaining == SuperClass::Get_Recharge_Time(super))
				continue;
		}

		int remaining = CalculateTimerRemaining(&super->Control);
		ColorScheme* color = ColorSchemes.Vector_Item[super->House->RemapColor];
		Print_Timer_On_Tactical(timerIndex++, color, remaining / 15,
								super->Class->at.UIName, &super->text_48, &super->__BlinkState);
	}

	// Blackout timers
	for (int i = 0; i < Houses.ActiveCount; ++i)
	{
		HouseClass* house = Houses.Vector_Item[i];

		int accumulated = house->_ForceShieldBlackoutTime.Accumulated;
		int started = house->_ForceShieldBlackoutTime.Started;

		if (started != -1 && Frame - started < accumulated)
		{
			accumulated -= Frame - started;
		}

		if (accumulated > 0)
		{
			ColorScheme* color = ColorSchemes.Vector_Item[house->RemapColor];
			const char* text = TextManager::Fetch("MSG:BlackoutTimer", 0, "D:\\ra2mdpost\\Tactical.CPP", 452);
			Print_Timer_On_Tactical(timerIndex++, color, accumulated / 15, text, 0, 0);
		}
	}
}

// Helper function: Calculate remaining time from timer
int Tactical::CalculateTimerRemaining(TimerStruct* timer)
{
	if (timer->Started == -1)
		return timer->DelayTime;

	if (Frame - timer->Started >= timer->DelayTime)
		return 0;

	return timer->DelayTime - (Frame - timer->Started);
}

// Helper function: Render animated objects layer
void Tactical::RenderAnimatedLayer(Tactical* this, DSurface* surface, TacticalRenderMode mode)
{
	Tactical::Fill_Building_Selectables(this,
		ViewportBounds_TacPixel.X,
		ViewportBounds_TacPixel.Y,
		ViewportBounds_TacPixel.Width,
		ViewportBounds_TacPixel.Height);

	surface->xs.s.vftble->Lock(&surface->xs.s, 0, 0);

	DSurface* savedTemp = TempSurface;
	TempSurface = surface;

	// Draw waypoints, rally points, placement (pass 0)
	Tactical::Draw_Waypoint_Stuff(this, 0);
	Tactical::Draw_Rally_Points(this, 0);
	Tactical::draw_placement(this, 0);

	// Draw effects
	IonBlastClass::Draw_All();
	Tactical::Render_Layers(this, 1);
	SpotlightClass::Draw_All();
	LaserDrawClass::Draw_All();
	EBolt::Draw_All();
	LineTrail::Draw_All();
	RadBeam::Draw_All();
	Tactical_super_lines_circles(this);

	// Draw beacons
	BeaconPlacement::Set_Alliances(&Beacons, TempSurface,
		ViewportBounds_TacPixel.X,
		ViewportBounds_TacPixel.Y,
		ViewportBounds_TacPixel.Width,
		ViewportBounds_TacPixel.Height);

	// Draw band box and waypoints (pass 1)
	Tactical::Draw_Band_Box(this);
	Tactical::Draw_Waypoint_Stuff(this, 1);
	Tactical::Draw_Rally_Points(this, 1);
	Tactical::draw_placement(this, 1);

	// Planning mode nodes
	bool inPlanningMode = Is_In_Planning_Mode();
	if (inPlanningMode)
	{
		plannodes_63B0A0();
		plannodes_63B150();
	}

	// Draw techno overlays
	bool hadTechnoVector = TechnoVector.ActiveCount > 0;
	DrawTechnoOverlays(this, inPlanningMode);

	if (hadTechnoVector)
	{
		draw_63B2F0();
	}

	// Draw pixel effects
	Tactical::Draw_Pixel_Effects(this, &ViewportBounds_TacPixel, &ViewportBounds_TacPixel);

	TempSurface = savedTemp;
	surface->xs.s.vftble->Unlock(surface);

	// Render timers
	RenderTimers(this);

	// Draw tactical text
	Tactical::Draw_Tactical_Text(this, this->__Text);

	// Reset flags
	this->drawbool_byteD7C = 0;
	this->__TacticalPosUpdated__IsToRedraw = 0;
}

// Main render function - refactored
void __thiscall Tactical::Render(Tactical* this, DSurface* surface, bool drawbool, TacticalRenderMode mode)
{
	this->_ObjectsInViewport = 0;

	bool boundsChanged = Tactical_checkbuildingbounds_6D9B50(this,
		ViewportBounds_TacPixel.X,
		ViewportBounds_TacPixel.Y,
		ViewportBounds_TacPixel.Width,
		ViewportBounds_TacPixel.Height);

	// Determine what needs to be redrawn
	RedrawContext ctx = DetermineRedrawMode(this, drawbool, mode, boundsChanged);

	// Handle surface blitting if needed
	if (!ctx.shouldSkipToSurface)
	{
		surface = HandleSurfaceOperations(this, surface, ctx.offsetX, ctx.offsetY, drawbool, mode);
	}

	// Early exit for mode 0
	if (mode == TacticalRenderMode_0_ALL)
		return;

	// Render buildings in ground layer
	if (!Debug_Map_DEBUGDEBUG)
	{
		Tactical::Render_Buildings_In_Ground_Layer_0(this,
			ViewportBounds_TacPixel.X - 64,
			ViewportBounds_TacPixel.Y - 64,
			ViewportBounds_TacPixel.Width + 128,
			ViewportBounds_TacPixel.Height + 128);
	}

	// Swap temp surface for terrain rendering
	DSurface* savedTemp = TempSurface;
	TempSurface = TileSurface;
	TileSurface->xs.s.vftble->Lock(&TileSurface->xs.s, 0, 0);

	// Render terrain layers
	if (mode == TacticalRenderMode_TERRAIN || mode == TacticalRenderMode_3_ALL)
	{
		RenderTerrainLayers(this, drawbool, ctx.offsetX, ctx.offsetY);

		TempSurface->xs.s.vftble->Unlock(&TempSurface->xs.s);
		TempSurface = savedTemp;
		this->__SomeCellCount = 0;
	}

	// Update last positions
	this->_LastTacticalCoord = this->_TacticalCoord;
	this->__LastTacticalPos = this->__TacticalPos;

	// Copy tile surface to temp if terrain mode
	if (mode == TacticalRenderMode_TERRAIN || mode == TacticalRenderMode_3_ALL)
	{
		TempSurface->xs.s.vftble->Copy_From__RSR(TempSurface,
			&ViewportBounds_TacPixel, TileSurface, &ViewportBounds_TacPixel, 0, 1);

		if (mode == TacticalRenderMode_TERRAIN)
			return;
	}

	// Render animated layer
	if (mode == TacticalRenderMode_MOVING_ANIMATING || mode == TacticalRenderMode_3_ALL)
	{
		RenderAnimatedLayer(this, surface, mode);
	}
}