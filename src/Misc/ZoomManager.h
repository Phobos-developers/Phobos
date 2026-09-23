#pragma once

#include <GeneralStructures.h>

class ZoomManager
{
public:
	static bool Enabled;
	static bool ScrollEnabled;
	static bool KeyEnabled;
	static double CurrentZoom;
	static double TargetZoom;
	static double MinZoom;
	static double MaxZoom;
	static double Step;
	static bool Smooth;
	static double SmoothRate;
	static double ActiveSmoothRate;
	static bool BlitAppliedThisFrame;

	// Redirects hardcoded DSurface::Composite references to DSurface::Temp for tactical rendering
	static void ApplySurfacePatches();

	// Checks whether player manual zoom interaction is allowed
	static bool CanPlayerZoom();

	// Applies scripted tactical zoom from map triggers with optional resolution clamping
	static void SetScriptZoom(double targetZoom, int transitionRate, int minWidth, int minHeight);

	// Adjusts target zoom level incrementally
	static void ZoomIn();
	static void ZoomOut();

	// Resets zoom to default 1.0x scale
	static void ResetZoom();

	// Smoothly interpolates current zoom toward target zoom each frame
	static void Update();

	// Calculates source crop and destination blit rectangles maintaining strict center parity
	static void GetBlitRects(RectangleStruct& srcRect, RectangleStruct& dstRect);

	// Blits tactical surface to composite buffer with centering and scaling
	static void ApplyTacticalBlit();

	// Checks whether zoom level is currently greater than 1.0x
	static bool IsZoomed();

	// Restricts camera coordinates to playable map boundaries, scaled by current zoom
	static bool ClampTacticalPos(Point2D* pPoint);

	// Transforms screen coordinates to tactical composite coordinates for mouse interactions
	static Point2D ScreenToTactical(const Point2D& screenPoint);

	// Transforms tactical coordinates back to screen coordinates
	static Point2D TacticalToScreen(const Point2D& virtualPoint);
};

