#include "ZoomManager.h"

#include <Surface.h>
#include <TacticalClass.h>
#include <WWMouseClass.h>
#include <Utilities/Macro.h>

#include <algorithm>

// Translate mouse coordinates to scaled tactical space for map interactions without mutating caller memory
DEFINE_HOOK(0x692300, DisplayClass_ProcessClickCoords_TranslateCoordinates, 0x7)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(const Point2D*, pPoint, 0x4);

	if (pPoint)
	{
		static thread_local Point2D translatedPoint;
		const RectangleStruct& vb = DSurface::ViewBounds;
		Point2D screenPt = *pPoint;
		screenPt.X += vb.X;
		screenPt.Y += vb.Y;

		translatedPoint = ZoomManager::ScreenToTactical(screenPt);
		translatedPoint.X -= vb.X;
		translatedPoint.Y -= vb.Y;

		R->Stack<const Point2D*>(0x4, &translatedPoint);
	}

	return 0;
}

// Translate active rubberband selection box coordinates to scaled tactical space without mutating caller memory
DEFINE_HOOK(0x4AC380, DisplayClass_UpdateDragBand_TranslateCoordinates, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(const Point2D*, pPoint, 0x4);

	if (pPoint)
	{
		static thread_local Point2D translatedPoint;
		const RectangleStruct& vb = DSurface::ViewBounds;
		Point2D screenPt = *pPoint;
		screenPt.X += vb.X;
		screenPt.Y += vb.Y;

		translatedPoint = ZoomManager::ScreenToTactical(screenPt);
		translatedPoint.X -= vb.X;
		translatedPoint.Y -= vb.Y;

		R->Stack<const Point2D*>(0x4, &translatedPoint);
	}

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

// Advance smooth zoom interpolation per render frame
DEFINE_HOOK(0x4F4480, GScreenClass_Render_ZoomUpdate, 0x9)
{
	ZoomManager::Update();
	return 0;
}

// Translate cursor background save/restore rectangle to scaled tactical space
DEFINE_HOOK(0x7B91B8, WWMouseClass_func_3C_TranslateRect, 0x16)
{
	GET(const RectangleStruct*, pSrcRect, ECX);
	GET(RectangleStruct*, pDstRect, EDX);
	GET(const DSurface*, pSurface, EBP);

	*pDstRect = *pSrcRect;

	if (ZoomManager::IsZoomed() && (pSurface == DSurface::Composite || pSurface == nullptr))
	{
		const RectangleStruct& vb = DSurface::ViewBounds;

		if (pDstRect->X >= vb.X && pDstRect->X < vb.X + vb.Width &&
			pDstRect->Y >= vb.Y && pDstRect->Y < vb.Y + vb.Height)
		{
			Point2D screenPt { pDstRect->X, pDstRect->Y };
			Point2D virtPt = ZoomManager::ScreenToTactical(screenPt);
			pDstRect->X = virtPt.X;
			pDstRect->Y = virtPt.Y;
		}
	}

	return 0x7B91CE;
}

// Translate cursor draw coordinates to scaled tactical space within the tactical viewport
DEFINE_HOOK(0x7B9445, WWMouseClass_Draw_TranslateCoordinates, 0x6)
{
	if (!ZoomManager::IsZoomed())
		return 0;

	GET_STACK(const DSurface*, pSurface, 0x5C);

	if (pSurface == DSurface::Composite || pSurface == nullptr)
	{
		GET(int, mouseX, ESI);
		GET(int, mouseY, EDI);

		const RectangleStruct& vb = DSurface::ViewBounds;

		if (mouseX >= vb.X && mouseX < vb.X + vb.Width &&
			mouseY >= vb.Y && mouseY < vb.Y + vb.Height)
		{
			Point2D screenPt { mouseX, mouseY };
			Point2D virtPt = ZoomManager::ScreenToTactical(screenPt);

			R->ESI(virtPt.X);
			R->EDI(virtPt.Y);
		}
	}

	return 0;
}

// Scale right-click pan scroll distance inversely by zoom factor to maintain consistent screen speed
DEFINE_HOOK(0x693791, ScrollClass_RightclickScroll_ScaleDistance, 0x6)
{
	if (!ZoomManager::IsZoomed() || !TacticalClass::Instance)
		return 0;

	const double zoom = TacticalClass::Instance->ZoomInFactor;
	REF_STACK(int, distX, 0x18);
	REF_STACK(int, distY, 0x1C);

	if (distX > 0)
		distX = std::max(1, static_cast<int>(distX / zoom));

	if (distY > 0)
		distY = std::max(1, static_cast<int>(distY / zoom));

	return 0;
}
