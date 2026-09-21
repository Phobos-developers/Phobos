#include "ZoomManager.h"

#include <TacticalClass.h>
#include <Surface.h>
#include <MapClass.h>

#include <algorithm>
#include <cmath>

bool ZoomManager::Enabled = true;
bool ZoomManager::WheelEnabled = true;
bool ZoomManager::HotkeysEnabled = true;
double ZoomManager::CurrentZoom = 1.0;
double ZoomManager::TargetZoom = 1.0;
double ZoomManager::MinZoom = 1.0;
double ZoomManager::MaxZoom = 2.5;
double ZoomManager::Step = 0.15;
bool ZoomManager::Smooth = true;
double ZoomManager::SmoothRate = 0.25;

bool ZoomManager::IsZoomed()
{
	return Enabled && TacticalClass::Instance && TacticalClass::Instance->ZoomInFactor > 1.0;
}

void ZoomManager::ZoomIn()
{
	if (!Enabled || !TacticalClass::Instance)
		return;

	TargetZoom = std::clamp(TargetZoom + Step, MinZoom, MaxZoom);

	if (!Smooth)
	{
		CurrentZoom = TargetZoom;
		TacticalClass::Instance->ZoomInFactor = CurrentZoom;
		MapClass::Instance.MarkNeedsRedraw(2);
	}
}

void ZoomManager::ZoomOut()
{
	if (!Enabled || !TacticalClass::Instance)
		return;

	TargetZoom = std::clamp(TargetZoom - Step, MinZoom, MaxZoom);

	if (!Smooth)
	{
		CurrentZoom = TargetZoom;
		TacticalClass::Instance->ZoomInFactor = CurrentZoom;
		MapClass::Instance.MarkNeedsRedraw(2);
	}
}

void ZoomManager::ResetZoom()
{
	if (!TacticalClass::Instance)
		return;

	TargetZoom = 1.0;

	if (!Smooth)
	{
		CurrentZoom = 1.0;
		TacticalClass::Instance->ZoomInFactor = 1.0;
		MapClass::Instance.MarkNeedsRedraw(2);
	}
}

void ZoomManager::Update()
{
	if (!TacticalClass::Instance)
		return;

	// Exponential lerp decay towards the target zoom factor
	if (Smooth)
	{
		if (std::abs(CurrentZoom - TargetZoom) > 0.0001)
		{
			CurrentZoom += (TargetZoom - CurrentZoom) * SmoothRate;

			if (std::abs(CurrentZoom - TargetZoom) <= 0.0001)
				CurrentZoom = TargetZoom;

			TacticalClass::Instance->ZoomInFactor = CurrentZoom;
			MapClass::Instance.MarkNeedsRedraw(2);
		}
		else if (CurrentZoom != TargetZoom)
		{
			CurrentZoom = TargetZoom;
			TacticalClass::Instance->ZoomInFactor = CurrentZoom;
			MapClass::Instance.MarkNeedsRedraw(2);
		}
	}
	else
	{
		if (CurrentZoom != TargetZoom)
		{
			CurrentZoom = TargetZoom;
			TacticalClass::Instance->ZoomInFactor = CurrentZoom;
			MapClass::Instance.MarkNeedsRedraw(2);
		}
	}
}

// Inverts the stretch-blit transformation performed in GScreenClass::UpdatePrimarySurface (0x4F4780).
// In that function, Westwood calculates a centered crop of DSurface::Composite:
//   src_width = surfaceWidth / zoom
//   src_height = surfaceHeight / zoom
//   src_x = (surfaceWidth - src_width) / 2
//   src_y = (surfaceHeight - src_height) / 2
// and stretch-blits that source rectangle to fill the entire primary tactical viewport.
// This function maps mouse cursor screen coordinates back to the unscaled composite surface pixels.
Point2D ZoomManager::ScreenToTactical(const Point2D& screenPoint)
{
	if (!IsZoomed())
		return screenPoint;

	const RectangleStruct& vb = DSurface::ViewBounds;

	if (screenPoint.X < vb.X || screenPoint.X >= vb.X + vb.Width ||
		screenPoint.Y < vb.Y || screenPoint.Y >= vb.Y + vb.Height)
		return screenPoint;

	const double zoom = TacticalClass::Instance->ZoomInFactor;
	const int surfaceWidth = DSurface::Composite ? DSurface::Composite->Width : vb.Width;
	const int surfaceHeight = DSurface::Composite ? DSurface::Composite->Height : vb.Height;

	if (surfaceWidth <= 0 || surfaceHeight <= 0)
		return screenPoint;

	const double zoomedWidth = static_cast<double>(surfaceWidth) / zoom;
	const double zoomedHeight = static_cast<double>(surfaceHeight) / zoom;

	const double cropX = (static_cast<double>(surfaceWidth) - zoomedWidth) * 0.5;
	const double cropY = (static_cast<double>(surfaceHeight) - zoomedHeight) * 0.5;

	Point2D virtualPoint;
	virtualPoint.X = static_cast<int>(cropX + (static_cast<double>(screenPoint.X - vb.X) / zoom) + 0.5);
	virtualPoint.Y = static_cast<int>(cropY + (static_cast<double>(screenPoint.Y - vb.Y) / zoom) + 0.5);

	virtualPoint.X = std::clamp(virtualPoint.X, 0, surfaceWidth - 1);
	virtualPoint.Y = std::clamp(virtualPoint.Y, 0, surfaceHeight - 1);

	return virtualPoint;
}

// Forward transformation mapping tactical composite surface coordinates into screen viewport space.
Point2D ZoomManager::TacticalToScreen(const Point2D& virtualPoint)
{
	if (!IsZoomed())
		return virtualPoint;

	const double zoom = TacticalClass::Instance->ZoomInFactor;
	const int surfaceWidth = DSurface::Composite ? DSurface::Composite->Width : DSurface::ViewBounds.Width;
	const int surfaceHeight = DSurface::Composite ? DSurface::Composite->Height : DSurface::ViewBounds.Height;

	if (surfaceWidth <= 0 || surfaceHeight <= 0)
		return virtualPoint;

	const double zoomedWidth = static_cast<double>(surfaceWidth) / zoom;
	const double zoomedHeight = static_cast<double>(surfaceHeight) / zoom;

	const double cropX = (static_cast<double>(surfaceWidth) - zoomedWidth) * 0.5;
	const double cropY = (static_cast<double>(surfaceHeight) - zoomedHeight) * 0.5;

	Point2D screenPoint;
	screenPoint.X = static_cast<int>((static_cast<double>(virtualPoint.X) - cropX) * zoom + 0.5) + DSurface::ViewBounds.X;
	screenPoint.Y = static_cast<int>((static_cast<double>(virtualPoint.Y) - cropY) * zoom + 0.5) + DSurface::ViewBounds.Y;

	return screenPoint;
}
