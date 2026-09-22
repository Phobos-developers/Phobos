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
bool ZoomManager::ScrollEnabled = true;
bool ZoomManager::KeyEnabled = true;
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

	if (Unsorted::UserInputLocked)
		return false;

	if (ScenarioClass::Instance && ScenarioClass::Instance->UserInputLocked)
		return false;

	return true;
}

// Applies scripted tactical zoom from map triggers with resolution clamping and transition rate
void ZoomManager::SetScriptZoom(double targetZoom, int transitionRate, int minWidth, int minHeight)
{
	if (!TacticalClass::Instance)
		return;

	double clampedZoom = targetZoom;

	// Clamp zoom level to preserve minimum visible tactical viewport dimensions
	if (minWidth > 0 && DSurface::ViewBounds.Width > 0)
	{
		const double maxByWidth = static_cast<double>(DSurface::ViewBounds.Width) / static_cast<double>(minWidth);
		clampedZoom = std::min(clampedZoom, maxByWidth);
	}

	if (minHeight > 0 && DSurface::ViewBounds.Height > 0)
	{
		const double maxByHeight = static_cast<double>(DSurface::ViewBounds.Height) / static_cast<double>(minHeight);
		clampedZoom = std::min(clampedZoom, maxByHeight);
	}

	TargetZoom = std::max(1.0, clampedZoom);

	// Synchronize input lock state to prevent cutscene reset from overriding scripted target
	LastInputLockedState = Unsorted::UserInputLocked || (ScenarioClass::Instance && ScenarioClass::Instance->UserInputLocked);

	if (transitionRate <= 0)
	{
		CurrentZoom = TargetZoom;
		ActiveSmoothRate = SmoothRate;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord1;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance.MarkNeedsRedraw(2);
	}
	else
	{
		ActiveSmoothRate = std::clamp(static_cast<double>(transitionRate) / 100.0, 0.01, 1.0);
	}
}

// Steps target zoom inward toward maximum magnification
void ZoomManager::ZoomIn()
{
	if (!CanPlayerZoom() || !TacticalClass::Instance)
		return;

	ActiveSmoothRate = SmoothRate;
	TargetZoom = std::clamp(TargetZoom + Step, MinZoom, MaxZoom);

	if (!Smooth)
	{
		CurrentZoom = TargetZoom;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord1;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance.MarkNeedsRedraw(2);
	}
}

// Steps target zoom outward toward default 1.0x
void ZoomManager::ZoomOut()
{
	if (!CanPlayerZoom() || !TacticalClass::Instance)
		return;

	ActiveSmoothRate = SmoothRate;
	TargetZoom = std::clamp(TargetZoom - Step, MinZoom, MaxZoom);

	if (!Smooth)
	{
		CurrentZoom = TargetZoom;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord1;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance.MarkNeedsRedraw(2);
	}
}

// Resets target zoom immediately back to 1.0x scale
void ZoomManager::ResetZoom()
{
	if (!CanPlayerZoom() || !TacticalClass::Instance)
		return;

	ActiveSmoothRate = SmoothRate;
	TargetZoom = 1.0;

	if (!Smooth)
	{
		CurrentZoom = 1.0;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord1;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance.MarkNeedsRedraw(2);
	}
}

// Smoothly interpolates current zoom toward target zoom each frame
void ZoomManager::Update()
{
	if (!TacticalClass::Instance)
		return;

	const bool currentLocked = Unsorted::UserInputLocked || (ScenarioClass::Instance && ScenarioClass::Instance->UserInputLocked);

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

		Point2D currentPos = TacticalClass::Instance->TacticalCoord1;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance.MarkNeedsRedraw(2);
	}
	else if (CurrentZoom != TargetZoom)
	{
		CurrentZoom = TargetZoom;
		ActiveSmoothRate = SmoothRate;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord1;
		TacticalClass::Instance->SetTacticalPosition(&currentPos);
		MapClass::Instance.MarkNeedsRedraw(2);
	}
}

// Restricts camera coordinates to playable map boundaries, scaled by current zoom
bool ZoomManager::ClampTacticalPos(Point2D* pPoint)
{
	if (!pPoint)
		return false;

	const auto& viewBounds = DSurface::ViewBounds;
	const double zoom = IsZoomed() ? CurrentZoom : 1.0;

	const int effectiveWidth = static_cast<int>(viewBounds.Width / zoom + 0.5);
	const int effectiveHeight = static_cast<int>(viewBounds.Height / zoom + 0.5);

	const int v1 = Make_Global<int>(0x87F8E4);
	const int v2 = Make_Global<int>(0x87F8DC);
	const int v3 = Make_Global<int>(0x87F8E8);
	const int v4 = Make_Global<int>(0x87F8EC);
	const int v5 = Make_Global<int>(0x87F8F0);

	const int minX = 30 * (2 * v1 - v2) + (effectiveWidth / 2);
	const int maxX = std::max(minX, minX + (60 * v4) - effectiveWidth);

	const int minY = 15 * (v2 + 2 * v3 - 5) + (effectiveHeight / 2);
	const int maxY = std::max(minY, minY + ((60 * v5 + 270) / 2) - effectiveHeight);

	bool bClamped = false;

	if (pPoint->Y < minY)
	{
		pPoint->Y = minY;
		bClamped = true;
	}
	else if (pPoint->Y > maxY)
	{
		pPoint->Y = maxY;
		bClamped = true;
	}

	if (pPoint->X < minX)
	{
		pPoint->X = minX;
		bClamped = true;
	}
	else if (pPoint->X > maxX)
	{
		pPoint->X = maxX;
		bClamped = true;
	}

	return bClamped;
}

// Maps screen pixel coordinates into tactical surface space under zoom
Point2D ZoomManager::ScreenToTactical(const Point2D& screenPoint)
{
	if (!IsZoomed())
		return screenPoint;

	const double zoom = CurrentZoom;
	const int surfaceWidth = DSurface::Composite ? DSurface::Composite->Width : DSurface::ViewBounds.Width;
	const int surfaceHeight = DSurface::Composite ? DSurface::Composite->Height : DSurface::ViewBounds.Height;

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
	const int surfaceWidth = DSurface::Composite ? DSurface::Composite->Width : DSurface::ViewBounds.Width;
	const int surfaceHeight = DSurface::Composite ? DSurface::Composite->Height : DSurface::ViewBounds.Height;

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
	if (!IsZoomed() || !DSurface::Alternate || !DSurface::Composite)
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

	DSurface::Composite->CopyFrom(&dstRect, &dstRect, DSurface::Alternate, &dstRect, &srcRect, false, false);
}
