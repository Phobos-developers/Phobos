#pragma once

#include <GeneralStructures.h>

class ZoomManager
{
public:
	static bool Enabled;
	static bool WheelEnabled;
	static bool HotkeysEnabled;
	static double CurrentZoom;
	static double TargetZoom;
	static double MinZoom;
	static double MaxZoom;
	static double Step;
	static bool Smooth;
	static double SmoothRate;

	// Adjusts target zoom level incrementally
	static void ZoomIn();
	static void ZoomOut();

	// Resets zoom to default 1.0x scale
	static void ResetZoom();

	// Smoothly interpolates current zoom toward target zoom each frame
	static void Update();

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

