#include "ZoomManager.h"

#include <TacticalClass.h>
#include <Surface.h>
#include <MapClass.h>
#include <ScenarioClass.h>
#include <Unsorted.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <algorithm>
#include <cmath>

bool ZoomManager::Enabled = false;
bool ZoomManager::WheelEnabled = true;
bool ZoomManager::HotkeysEnabled = true;
double ZoomManager::CurrentZoom = 1.0;
double ZoomManager::TargetZoom = 1.0;
double ZoomManager::MinZoom = 1.0;
double ZoomManager::MaxZoom = 2.5;
double ZoomManager::Step = 0.15;
bool ZoomManager::Smooth = true;
double ZoomManager::SmoothRate = 0.25;
double ZoomManager::ActiveSmoothRate = 0.25;

static bool LastInputLockedState = false;

// Checks whether tactical zoom is currently magnifying the view
bool ZoomManager::IsZoomed()
{
	return CurrentZoom > 1.0001;
}

// Determines if the human player is currently permitted to interact with tactical zoom
bool ZoomManager::CanPlayerZoom()
{
	if (!Enabled)
		return false;

	if (Game::UserInputLocked())
		return false;

	if (ScenarioClass::Instance->UserInputLocked)
		return false;

	return true;
}

// Applies scripted tactical zoom from map triggers with resolution clamping and transition rate
void ZoomManager::SetScriptZoom(double targetZoom, int transitionRate, int minWidth, int minHeight)
{
	double clampedZoom = targetZoom;

	// Clamp zoom level to preserve minimum visible tactical viewport dimensions
	if (minWidth > 0 && DSurface::ViewBounds->Width > 0)
	{
		const double maxByWidth = static_cast<double>(DSurface::ViewBounds->Width) / static_cast<double>(minWidth);
		clampedZoom = std::min(clampedZoom, maxByWidth);
	}

	if (minHeight > 0 && DSurface::ViewBounds->Height > 0)
	{
		const double maxByHeight = static_cast<double>(DSurface::ViewBounds->Height) / static_cast<double>(minHeight);
		clampedZoom = std::min(clampedZoom, maxByHeight);
	}

	TargetZoom = std::max(1.0, clampedZoom);

	// Synchronize input lock state to prevent cutscene reset from overriding scripted target
	LastInputLockedState = Game::UserInputLocked() || ScenarioClass::Instance->UserInputLocked;

	if (transitionRate <= 0)
	{
		CurrentZoom = TargetZoom;
		ActiveSmoothRate = SmoothRate;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance->MarkNeedsRedraw(2);
	}
	else
	{
		ActiveSmoothRate = std::clamp(static_cast<double>(transitionRate) / 100.0, 0.01, 1.0);
	}
}

// Steps target zoom inward toward maximum magnification
void ZoomManager::ZoomIn()
{
	if (!CanPlayerZoom() )
		return;

	ActiveSmoothRate = SmoothRate;
	TargetZoom = std::clamp(TargetZoom + Step, MinZoom, MaxZoom);

	if (!Smooth)
	{
		CurrentZoom = TargetZoom;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance->MarkNeedsRedraw(2);
	}
}

// Steps target zoom outward toward default 1.0x
void ZoomManager::ZoomOut()
{
	if (!CanPlayerZoom())
		return;

	ActiveSmoothRate = SmoothRate;
	TargetZoom = std::clamp(TargetZoom - Step, MinZoom, MaxZoom);

	if (!Smooth)
	{
		CurrentZoom = TargetZoom;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance->MarkNeedsRedraw(2);
	}
}

// Resets target zoom immediately back to 1.0x scale
void ZoomManager::ResetZoom()
{
	if (!CanPlayerZoom())
		return;

	ActiveSmoothRate = SmoothRate;
	TargetZoom = 1.0;

	if (!Smooth)
	{
		CurrentZoom = 1.0;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance->MarkNeedsRedraw(2);
	}
}

// Smoothly interpolates current zoom toward target zoom each frame
void ZoomManager::Update()
{
	const bool currentLocked = Game::UserInputLocked() || ScenarioClass::Instance->UserInputLocked;

	if (currentLocked && !LastInputLockedState)
	{
		// Smoothly restore default view when input is locked for cutscenes without an active script override
		if (std::abs(TargetZoom - 1.0) > 0.0001)
		{
			TargetZoom = 1.0;
			ActiveSmoothRate = SmoothRate;
		}
	}
	LastInputLockedState = currentLocked;

	const double effectiveRate = (ActiveSmoothRate > 0.0) ? ActiveSmoothRate : SmoothRate;

	if (Smooth && std::abs(CurrentZoom - TargetZoom) > 0.0001)
	{
		CurrentZoom += (TargetZoom - CurrentZoom) * effectiveRate;

		if (std::abs(CurrentZoom - TargetZoom) <= 0.0001)
		{
			CurrentZoom = TargetZoom;
			ActiveSmoothRate = SmoothRate;
		}

		Point2D currentPos = TacticalClass::Instance->TacticalCoord;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance->MarkNeedsRedraw(2);
	}
	else if (CurrentZoom != TargetZoom)
	{
		CurrentZoom = TargetZoom;
		ActiveSmoothRate = SmoothRate;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance->MarkNeedsRedraw(2);
	}
}

// Maps screen pixel coordinates into tactical surface space under zoom
Point2D ZoomManager::ScreenToTactical(const Point2D& screenPoint)
{
	if (!IsZoomed())
		return screenPoint;

	const double zoom = CurrentZoom;
	const int surfaceWidth = DSurface::Composite() ? DSurface::Composite->Width : DSurface::ViewBounds->Width;
	const int surfaceHeight = DSurface::Composite() ? DSurface::Composite->Height : DSurface::ViewBounds->Height;

	if (surfaceWidth <= 0 || surfaceHeight <= 0)
		return screenPoint;

	if (screenPoint.X < 0 || screenPoint.X >= surfaceWidth || screenPoint.Y < 0 || screenPoint.Y >= surfaceHeight)
		return screenPoint;

	const double zoomedWidth = static_cast<double>(surfaceWidth) / zoom;
	const double zoomedHeight = static_cast<double>(surfaceHeight) / zoom;

	const double cropX = (static_cast<double>(surfaceWidth) - zoomedWidth) * 0.5;
	const double cropY = (static_cast<double>(surfaceHeight) - zoomedHeight) * 0.5;

	Point2D virtualPoint;
	virtualPoint.X = static_cast<int>(cropX + (static_cast<double>(screenPoint.X) / zoom) + 0.5);
	virtualPoint.Y = static_cast<int>(cropY + (static_cast<double>(screenPoint.Y) / zoom) + 0.5);

	virtualPoint.X = std::clamp(virtualPoint.X, 0, surfaceWidth - 1);
	virtualPoint.Y = std::clamp(virtualPoint.Y, 0, surfaceHeight - 1);

	return virtualPoint;
}

// Maps tactical surface coordinates back to screen pixel space
Point2D ZoomManager::TacticalToScreen(const Point2D& virtualPoint)
{
	if (!IsZoomed())
		return virtualPoint;

	const double zoom = CurrentZoom;
	const int surfaceWidth = DSurface::Composite() ? DSurface::Composite->Width : DSurface::ViewBounds->Width;
	const int surfaceHeight = DSurface::Composite() ? DSurface::Composite->Height : DSurface::ViewBounds->Height;

	if (surfaceWidth <= 0 || surfaceHeight <= 0)
		return virtualPoint;

	const double zoomedWidth = static_cast<double>(surfaceWidth) / zoom;
	const double zoomedHeight = static_cast<double>(surfaceHeight) / zoom;

	const double cropX = (static_cast<double>(surfaceWidth) - zoomedWidth) * 0.5;
	const double cropY = (static_cast<double>(surfaceHeight) - zoomedHeight) * 0.5;

	Point2D screenPoint;
	screenPoint.X = static_cast<int>((static_cast<double>(virtualPoint.X) - cropX) * zoom + 0.5);
	screenPoint.Y = static_cast<int>((static_cast<double>(virtualPoint.Y) - cropY) * zoom + 0.5);

	return screenPoint;
}

// Blits centered viewport crop from Alternate surface onto Composite surface
void ZoomManager::ApplyTacticalBlit()
{
	if (!IsZoomed() || !DSurface::Alternate() || !DSurface::Composite())
		return;

	const RectangleStruct& vb = DSurface::ViewBounds;
	const double zoom = CurrentZoom;

	const double zoomedWidth = static_cast<double>(vb.Width) / zoom;
	const double zoomedHeight = static_cast<double>(vb.Height) / zoom;

	const double cropX = vb.X + (static_cast<double>(vb.Width) - zoomedWidth) * 0.5;
	const double cropY = vb.Y + (static_cast<double>(vb.Height) - zoomedHeight) * 0.5;

	RectangleStruct srcRect
	{
		static_cast<int>(cropX + 0.5),
		static_cast<int>(cropY + 0.5),
		static_cast<int>(zoomedWidth + 0.5),
		static_cast<int>(zoomedHeight + 0.5)
	};
	RectangleStruct dstRect = vb;

	DSurface::Composite->Copy_From(dstRect, dstRect, DSurface::Alternate(), dstRect, srcRect, false, false);
}

//================================================= Hooks =================================================

static Point2D radarCenterPixel = { 0, 0 };

// Translate screen click coordinates into tactical space for mouse hover and selection targeting
DEFINE_HOOK(0x692300, DisplayClass_ProcessClickCoords_TranslateCoordinates, 0x7)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(Point2D*, pPoint, 0x4);

	static Point2D translatedPoint;
	translatedPoint = ZoomManager::ScreenToTactical(*pPoint);
	R->Stack<Point2D*>(0x4, &translatedPoint);

	return 0;
}

// Translate mouse coordinates for the unit selection box (rubberband)
DEFINE_HOOK(0x4AC380, DisplayClass_UpdateDragBand_TranslateCoordinates, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(Point2D*, pPoint, 0x4);

	static Point2D dragPoint;
	dragPoint = ZoomManager::ScreenToTactical(*pPoint);
	R->Stack<Point2D*>(0x4, &dragPoint);

	return 0;
}

// Reset tactical zoom to 1.0x on middle mouse button click
DEFINE_HOOK(0x6930A0, ScrollClass_MessageHandler_MiddleClickReset, 0x5)
{
	GET_STACK(const UINT*, pMessage, 0x8);

	if (ZoomManager::IsZoomed() && ZoomManager::WheelEnabled && pMessage && *pMessage == WM_MBUTTONDOWN)
		ZoomManager::ResetZoom();

	return 0;
}

// Capture the true unclamped camera center on the radar minimap before Westwood clamps it
DEFINE_HOOK(0x657013, RadarClass_Render_Radar_RecordCenter, 0x6)
{
	GET(RadarClass*, pRadar, ESI);

	radarCenterPixel.X = pRadar->unknown_rect_14DC.X;
	radarCenterPixel.Y = pRadar->unknown_rect_14DC.Y;

	return 0;
}

// Scale and center the white viewport bounding box on the radar minimap
DEFINE_HOOK(0x657134, RadarClass_Render_Radar_ScaleViewRect, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET(RadarClass*, pRadar, ESI);

	auto& radarViewRect = pRadar->unknown_rect_14DC;
	const auto& radarRect = pRadar->unknown_rect_149C;

	const double zoom = ZoomManager::CurrentZoom;
	const int oldW = radarViewRect.Width;
	const int oldH = radarViewRect.Height;

	const int newW = std::max(2, static_cast<int>(oldW / zoom + 0.5));
	const int newH = std::max(2, static_cast<int>(oldH / zoom + 0.5));

	int newX = radarCenterPixel.X - newW / 2;
	int newY = radarCenterPixel.Y - newH / 2;

	if (newX < radarRect.X)
		newX = radarRect.X;
	else if (newX + newW >= radarRect.X + radarRect.Width)
		newX = radarRect.X + radarRect.Width - newW - 1;

	if (newY < radarRect.Y)
		newY = radarRect.Y;
	else if (newY + newH >= radarRect.Y + radarRect.Height)
		newY = radarRect.Y + radarRect.Height - newH - 1;

	radarViewRect.X = newX;
	radarViewRect.Y = newY;
	radarViewRect.Width = newW;
	radarViewRect.Height = newH;

	return 0;
}

// Scale horizontal radar click boundary to match effective zoomed viewport width
DEFINE_HOOK(0x653D92, RadarClass_RTacticalClass_Action_ClampWidth, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	const double zoom = ZoomManager::CurrentZoom;
	const int width = DSurface::ViewBounds->Width;
	const int effectiveWidth = static_cast<int>(width / zoom + 0.5);

	R->ECX(effectiveWidth);
	return 0x653D98;
}

// Scale vertical radar click boundary to match effective zoomed viewport height
DEFINE_HOOK(0x653DAC, RadarClass_RTacticalClass_Action_ClampHeight, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	const double zoom = ZoomManager::CurrentZoom;
	const int height = DSurface::ViewBounds->Height;
	const int effectiveHeight = static_cast<int>(height / zoom + 0.5);

	R->EBX(effectiveHeight);
	return 0x653DB2;
}