#include "ZoomManager.h"

#include <Surface.h>
#include <Utilities/Macro.h>

// Intercepts coordinate conversion at DisplayClass::ProcessClickCoords.
// This is the single engine bottleneck where tactical viewport mouse coordinates map to map cells and objects.
// Translates coordinates for:
// - Unit hover and contextual action cursors (ScrollClass::Scroll_AI)
// - Unit click selection (ScrollClass::Message_Handler WM_LBUTTONDOWN)
// - Unit move and attack commands (ScrollClass::Message_Handler WM_RBUTTONDOWN)
// - Drag selection band anchor coordinate (DisplayClass::StartDragBand)
// - Multi-unit rubberband box drag selection (ScrollClass::Select_Boxes)
// - Building placement foundation grid snapping (HouseClass::PlaceObject)
// - Direct tactical context actions (DisplayClass::Action)
// Because this hook modifies only the stack-allocated local Point2D passed into ProcessClickCoords
// without mutating Windows LPARAM messages or WWMouseClass, the Windows OS cursor and the in-game cursor
// remain 100% synchronized with zero offset, drift, or jumping.
DEFINE_HOOK(0x692300, DisplayClass_ProcessClickCoords_TranslateCoordinates, 0x7)
{
	if (ZoomManager::IsZoomed())
	{
		GET_STACK(Point2D*, pPoint, 0x4);
		if (pPoint)
		{
			*pPoint = ZoomManager::ScreenToTactical(*pPoint);
		}
	}

	return 0;
}

// Intercepts active rubberband rectangle coordinate updates in DisplayClass::UpdateDragBand.
// Ensures that while dragging, the visual selection box rendered to DSurface::Composite tracks
// the zoomed mouse position with subpixel precision.
DEFINE_HOOK(0x4AC380, DisplayClass_UpdateDragBand_TranslateCoordinates, 0x6)
{
	if (ZoomManager::IsZoomed())
	{
		GET_STACK(Point2D*, pPoint, 0x4);
		if (pPoint)
		{
			*pPoint = ZoomManager::ScreenToTactical(*pPoint);
		}
	}

	return 0;
}

// Resets tactical magnification back to standard 1.0x view when middle mouse button is pressed.
// Does NOT modify *pLParam so Windows message dispatching remains pristine.
DEFINE_HOOK(0x6930A0, ScrollClass_MessageHandler_MiddleClickReset, 0x5)
{
	GET_STACK(const UINT*, pMessage, 0x8);

	if (ZoomManager::IsZoomed() && ZoomManager::WheelEnabled && pMessage && *pMessage == WM_MBUTTONDOWN)
	{
		ZoomManager::ResetZoom();
	}

	return 0;
}

// Advances the smooth zoom interpolation once per render frame in GScreenClass::Render
// prior to composite rendering and blitting to the primary display surface.
DEFINE_HOOK(0x4F4480, GScreenClass_Render_ZoomUpdate, 0x9)
{
	ZoomManager::Update();
	return 0;
}


