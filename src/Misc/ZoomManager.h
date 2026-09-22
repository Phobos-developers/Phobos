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

	static void ZoomIn();
	static void ZoomOut();
	static void ResetZoom();
	static void Update();
	static void ApplyTacticalBlit();

	static bool IsZoomed();
	static Point2D ScreenToTactical(const Point2D& screenPoint);
	static Point2D TacticalToScreen(const Point2D& virtualPoint);
};

