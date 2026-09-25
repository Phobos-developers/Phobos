#include "ZoomManager.h"

#include <GScreenClass.h>
#include <RadarClass.h>
#include <Surface.h>
#include <TacticalClass.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <Unsorted.h>

#include <algorithm>

static Point2D radarCenterPixel = { 0, 0 };

// Translate screen click coordinates into tactical space for mouse hover and selection targeting
DEFINE_HOOK(0x692300, DisplayClass_ProcessClickCoords_TranslateCoordinates, 0x7)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(Point2D*, pPoint, 0x4);

	if (pPoint)
	{
		static Point2D translatedPoint;
		translatedPoint = ZoomManager::ScreenToTactical(*pPoint);
		R->Stack<Point2D*>(0x4, &translatedPoint);
	}

	return 0;
}

// Translate mouse coordinates on initial mouse button press for the unit selection box (rubberband)
DEFINE_HOOK(0x4AC310, DisplayClass_MouseLeftPress_TranslateCoordinates, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(Point2D*, pPoint, 0x4);

	if (pPoint)
	{
		static Point2D pressPoint;
		pressPoint = ZoomManager::ScreenToTactical(*pPoint);
		R->Stack<Point2D*>(0x4, &pressPoint);
	}

	return 0;
}

// Translate mouse coordinates for the unit selection box (rubberband)
DEFINE_HOOK(0x4AC380, DisplayClass_UpdateDragBand_TranslateCoordinates, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(Point2D*, pPoint, 0x4);

	if (pPoint)
	{
		static Point2D dragPoint;
		dragPoint = ZoomManager::ScreenToTactical(*pPoint);
		R->Stack<Point2D*>(0x4, &dragPoint);
	}

	return 0;
}

// Reset tactical zoom to 1.0x on middle mouse button click
DEFINE_HOOK(0x6930A0, ScrollClass_MessageHandler_MiddleClickReset, 0x5)
{
	GET_STACK(const UINT*, pMessage, 0x8);

	if (ZoomManager::IsZoomed() && ZoomManager::ScrollEnabled && pMessage && *pMessage == WM_MBUTTONDOWN)
		ZoomManager::ResetZoom();

	return 0;
}

// Advance smooth zoom interpolation at the beginning of each frame
DEFINE_HOOK(0x4F4480, GScreenClass_Render_ZoomUpdate, 0x9)
{
	ZoomManager::ApplySurfacePatches();
	ZoomManager::Update();

	return 0;
}

// Render tactical terrain and units onto Alternate surface, apply scaled blit to Composite, and preserve unscaled UI
DEFINE_HOOK(0x4F44CB, GScreenClass_Render_TacticalAndCommandBar, 0x5)
{
	if (!ZoomManager::IsZoomed() || !TacticalClass::Instance || !DSurface::Alternate || !DSurface::Composite)
		return 0;

	GET(GScreenClass*, pGScreen, ESI);
	GET_STACK(bool, flag, 0x08);
	GET_STACK(bool, bDraw, 0x0C);

	TacticalClass::Instance->Render(DSurface::Composite, flag, 0);

	DSurface* pComposite = DSurface::Composite;
	DSurface::Temp = DSurface::Alternate;

	TacticalClass::Instance->Render(DSurface::Alternate, flag, 1);
	TacticalClass::Instance->Render(DSurface::Alternate, flag, 2);

	DSurface::Temp = pComposite;

	if (!ZoomManager::BlitAppliedThisFrame)
		ZoomManager::ApplyTacticalBlit();

	pGScreen->Draw(bDraw ? 1 : 0);

	return 0x4F451B;
}

// Blit zoomed tactical map before on-screen timers (superweapon countdowns, mission timer) are rendered
DEFINE_HOOK(0x6D4941, TacticalClass_Render_Pass2_WorldEnd, 0x6)
{
	if (ZoomManager::IsZoomed())
		ZoomManager::ApplyTacticalBlit();

	return 0;
}

// Evaluate camera scrolling boundary in Scroll_Not_Really with zoom-scaled limits
DEFINE_HOOK(0x6DA2FE, TacticalClass_Scroll_Not_Really_ClampCandidate, 0x6)
{
	GET(int, candX, ESI);
	GET(int, candY, EDI);

	Point2D candidate = { candX, candY };
	if (!Unsorted::ArmageddonMode)
		ZoomManager::ClampTacticalPos(&candidate);

	R->ESI(candidate.X);
	R->EDI(candidate.Y);

	return 0x6DA327;
}

// Update camera coordinates in TacticalClass::Update with zoom-aware boundary clamping
DEFINE_HOOK(0x6D2727, TacticalClass_Update_ClampDesiredPos, 0x8)
{
	GET(TacticalClass*, pTactical, ESI);

	Point2D desiredPos = pTactical->TacticalCoord2;
	if (!Unsorted::ArmageddonMode)
		ZoomManager::ClampTacticalPos(&desiredPos);

	R->EAX(desiredPos.X);
	R->ECX(desiredPos.Y);

	return 0x6D275E;
}

// Restrict camera position in TacticalClass::SetTacticalPosition(Point2D*) with zoom-scaled limits
DEFINE_HOOK(0x6D600B, TacticalClass_SetTacticalPosition_ClampPos, 0x7)
{
	GET(Point2D*, pPoint, EDI);

	Point2D clamped = *pPoint;
	if (!Unsorted::ArmageddonMode)
		ZoomManager::ClampTacticalPos(&clamped);

	R->EAX(clamped.X);
	R->ECX(clamped.Y);

	return 0x6D6040;
}

// Restrict camera position in TacticalClass::SetTacticalPosition(CoordStruct*) with zoom-scaled limits
DEFINE_HOOK(0x6D611A, TacticalClass_SetTacticalPosition_Coord_ClampPos, 0xB)
{
	GET(int, candX, ESI);
	GET(int, candY, EBX);

	Point2D candidate = { candX, candY };
	if (!Unsorted::ArmageddonMode)
		ZoomManager::ClampTacticalPos(&candidate);

	R->ESI(candidate.X);
	R->EBX(candidate.Y);

	return 0x6D613F;
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
	const int width = DSurface::ViewBounds.Width;
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
	const int height = DSurface::ViewBounds.Height;
	const int effectiveHeight = static_cast<int>(height / zoom + 0.5);

	R->EBX(effectiveHeight);
	return 0x653DB2;
}

