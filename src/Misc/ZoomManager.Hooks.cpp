#include "ZoomManager.h"

#include <Surface.h>
#include <Utilities/Macro.h>

// Intercepts tactical mouse input messages at the root dispatcher (ScrollClass::Message_Handler).
// By transforming the LPARAM coordinate payload for mouse clicks, all downstream unit selection,
// move orders, attack commands, and rubberband drag start points receive accurate tactical coordinates.
// Middle-click (WM_MBUTTONDOWN) resets magnification back to standard 1.0x view.
DEFINE_HOOK(0x6930A0, ScrollClass_MessageHandler_TranslateCoordinates, 0x5)
{
	GET_STACK(const UINT*, pMessage, 0x8);
	GET_STACK(LPARAM*, pLParam, 0x10);

	if (!pMessage || !pLParam)
	{
		return 0;
	}

	const UINT msg = *pMessage;

	if (msg == WM_MBUTTONDOWN && ZoomManager::IsZoomed())
	{
		ZoomManager::ResetZoom();
		return 0;
	}

	if (ZoomManager::IsZoomed())
	{
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
		{
			const short rawX = static_cast<short>(LOWORD(*pLParam));
			const short rawY = static_cast<short>(HIWORD(*pLParam));

			Point2D screenPoint { rawX - DSurface::ViewBounds.X, rawY - DSurface::ViewBounds.Y };
			Point2D virtualPoint = ZoomManager::ScreenToTactical(screenPoint);

			const short newX = static_cast<short>(virtualPoint.X + DSurface::ViewBounds.X);
			const short newY = static_cast<short>(virtualPoint.Y + DSurface::ViewBounds.Y);

			*pLParam = MAKELPARAM(newX, newY);
		}
	}

	return 0;
}

// Translates per-frame cursor coordinates in ScrollClass::Scroll_AI.
// Ensures hover selection detection, dynamic action cursor changes (e.g. attack, enter, move),
// and active rubberband drag-selection box tracking align precisely with the magnified tactical display.
DEFINE_HOOK(0x4AE55B, ScrollClass_ScrollAI_TranslateCoordinates, 0x5)
{
	if (ZoomManager::IsZoomed())
	{
		auto pPoint = reinterpret_cast<Point2D*>(R->ESP() + 0x1C);
		*pPoint = ZoomManager::ScreenToTactical(*pPoint);
	}

	return 0;
}

// Translates coordinates during building placement preview in HouseClass::PlaceObject.
// Keeps the building blueprint grid snapping accurately aligned under the cursor while zoomed.
DEFINE_HOOK(0x4FB44D, HouseClass_PlaceObject_TranslateCoordinates, 0x5)
{
	if (ZoomManager::IsZoomed())
	{
		auto pPoint = reinterpret_cast<Point2D*>(R->ESP() + 0x24);
		*pPoint = ZoomManager::ScreenToTactical(*pPoint);
	}

	return 0;
}

// Translates coordinates in DisplayClass::Action when resolving tactical context actions.
DEFINE_HOOK(0x4AACB5, DisplayClass_Action_TranslateCoordinates, 0x5)
{
	if (ZoomManager::IsZoomed())
	{
		auto pPoint = reinterpret_cast<Point2D*>(R->ESP() + 0x1C);
		*pPoint = ZoomManager::ScreenToTactical(*pPoint);
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

