#pragma once

#include <GeneralStructures.h>

// Tactical zoom management for dynamic magnification of the battlefield view.
// Integrates with Westwood's native ZoomInFactor pipeline (0x4F4780) and provides
// bidirectional coordinate transformation between screen space and cropped tactical composite space.
class ZoomManager
{
public:
	static bool Enabled;
	static double CurrentZoom;
	static double TargetZoom;
	static double MinZoom;
	static double MaxZoom;
	static double Step;
	static bool Smooth;
	static double SmoothRate;

	// Adjusts target magnification factor inward
	static void ZoomIn();

	// Adjusts target magnification factor outward, clamped to MinZoom (1.0x)
	static void ZoomOut();

	// Resets magnification to standard 1.0x view
	static void ResetZoom();

	// Per-render-frame interpolation update for smooth zoom transitions
	static void Update();

	// Returns true when magnification is active and exceeds base 1.0x scale
	static bool IsZoomed();

	// Transforms client mouse screen coordinates to the cropped tactical composite surface
	static Point2D ScreenToTactical(const Point2D& screenPoint);

	// Transforms cropped tactical composite surface coordinates to client mouse screen coordinates
	static Point2D TacticalToScreen(const Point2D& virtualPoint);
};

