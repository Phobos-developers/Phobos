#include "ZoomManager.h"

#include <TacticalClass.h>
#include <Surface.h>
#include <MapClass.h>
#include <ScenarioClass.h>
#include <Unsorted.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <emmintrin.h>
#include <vector>

bool ZoomManager::Enabled = false;
bool ZoomManager::ScrollEnabled = true;
bool ZoomManager::KeyEnabled = true;
double ZoomManager::CurrentZoom = 1.0;
double ZoomManager::TargetZoom = 1.0;
double ZoomManager::MinZoom = 1.0;
double ZoomManager::MaxZoom = 3.6;
double ZoomManager::Step = 0.2;
bool ZoomManager::Smooth = true;
double ZoomManager::SmoothRate = 0.25;
double ZoomManager::ActiveSmoothRate = 0.25;
bool ZoomManager::BlitAppliedThisFrame = false;

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
		if (ClampTacticalPos(&currentPos))
		{
			TacticalClass::Instance->SetTacticalPosition(&currentPos);
			MapClass::Instance.MarkNeedsRedraw(2);
		}
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
		if (ClampTacticalPos(&currentPos))
		{
			TacticalClass::Instance->SetTacticalPosition(&currentPos);
			MapClass::Instance.MarkNeedsRedraw(2);
		}
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
		if (ClampTacticalPos(&currentPos))
		{
			TacticalClass::Instance->SetTacticalPosition(&currentPos);
			MapClass::Instance.MarkNeedsRedraw(2);
		}
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
		if (ClampTacticalPos(&currentPos))
			TacticalClass::Instance->SetTacticalPosition(&currentPos);

		MapClass::Instance.MarkNeedsRedraw(2);
	}
}

// Smoothly interpolates current zoom toward target zoom each frame
void ZoomManager::Update()
{
	BlitAppliedThisFrame = false;

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

	static bool wasZoomed = false;

	if (Smooth && std::abs(CurrentZoom - TargetZoom) > 0.0001)
	{
		CurrentZoom += (TargetZoom - CurrentZoom) * effectiveRate;

		if (std::abs(CurrentZoom - TargetZoom) <= 0.0001)
		{
			CurrentZoom = TargetZoom;
			ActiveSmoothRate = SmoothRate;
		}

		Point2D currentPos = TacticalClass::Instance->TacticalCoord1;
		if (ClampTacticalPos(&currentPos))
		{
			TacticalClass::Instance->SetTacticalPosition(&currentPos);
			MapClass::Instance.MarkNeedsRedraw(2);
		}
	}
	else if (CurrentZoom != TargetZoom)
	{
		CurrentZoom = TargetZoom;
		ActiveSmoothRate = SmoothRate;
		Point2D currentPos = TacticalClass::Instance->TacticalCoord1;
		if (ClampTacticalPos(&currentPos))
		{
			TacticalClass::Instance->SetTacticalPosition(&currentPos);
			MapClass::Instance.MarkNeedsRedraw(2);
		}
	}

	const bool isZoomedNow = IsZoomed();
	if (wasZoomed && !isZoomedNow)
		MapClass::Instance.MarkNeedsRedraw(2);

	wasZoomed = isZoomedNow;
}

// Calculates source crop and destination blit rectangles maintaining strict center parity
void ZoomManager::GetBlitRects(RectangleStruct& srcRect, RectangleStruct& dstRect)
{
	const RectangleStruct& vb = DSurface::ViewBounds;
	dstRect = vb;

	if (vb.Width <= 0 || vb.Height <= 0)
	{
		srcRect = vb;
		return;
	}

	if (!IsZoomed())
	{
		srcRect = vb;
		return;
	}

	const double zoom = CurrentZoom;
	int srcW = static_cast<int>(std::round(vb.Width / zoom));
	srcW = std::clamp(srcW, 2, vb.Width);
	if ((srcW % 2) != (vb.Width % 2))
	{
		if (vb.Width / zoom > srcW && srcW + 1 <= vb.Width)
			srcW += 1;
		else if (srcW - 1 >= 2)
			srcW -= 1;
		else
			srcW += 1;
	}

	int srcH = static_cast<int>(std::round(vb.Height / zoom));
	srcH = std::clamp(srcH, 2, vb.Height);
	if ((srcH % 2) != (vb.Height % 2))
	{
		if (vb.Height / zoom > srcH && srcH + 1 <= vb.Height)
			srcH += 1;
		else if (srcH - 1 >= 2)
			srcH -= 1;
		else
			srcH += 1;
	}

	const int cropX = vb.X + (vb.Width - srcW) / 2;
	const int cropY = vb.Y + (vb.Height - srcH) / 2;

	srcRect = { cropX, cropY, srcW, srcH };
}

// Restricts camera coordinates to playable map boundaries, scaled by current zoom
bool ZoomManager::ClampTacticalPos(Point2D* pPoint)
{
	if (!pPoint)
		return false;

	const auto& viewBounds = DSurface::ViewBounds;
	if (viewBounds.Width <= 0 || viewBounds.Height <= 0)
		return false;

	RectangleStruct srcRect, dstRect;
	GetBlitRects(srcRect, dstRect);

	const int effectiveWidth = srcRect.Width;
	const int effectiveHeight = srcRect.Height;

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

	RectangleStruct srcRect, dstRect;
	GetBlitRects(srcRect, dstRect);

	if (dstRect.Width <= 0 || dstRect.Height <= 0)
		return screenPoint;

	if (screenPoint.X < dstRect.X || screenPoint.X >= dstRect.X + dstRect.Width ||
	    screenPoint.Y < dstRect.Y || screenPoint.Y >= dstRect.Y + dstRect.Height)
	{
		return screenPoint;
	}

	Point2D virtualPoint;
	virtualPoint.X = static_cast<int>(srcRect.X + (static_cast<double>(screenPoint.X - dstRect.X) * srcRect.Width / dstRect.Width) + 0.5);
	virtualPoint.Y = static_cast<int>(srcRect.Y + (static_cast<double>(screenPoint.Y - dstRect.Y) * srcRect.Height / dstRect.Height) + 0.5);

	virtualPoint.X = std::clamp(virtualPoint.X, srcRect.X, srcRect.X + srcRect.Width - 1);
	virtualPoint.Y = std::clamp(virtualPoint.Y, srcRect.Y, srcRect.Y + srcRect.Height - 1);

	return virtualPoint;
}

// Maps tactical surface coordinates back to screen pixel space
Point2D ZoomManager::TacticalToScreen(const Point2D& virtualPoint)
{
	if (!IsZoomed())
		return virtualPoint;

	RectangleStruct srcRect, dstRect;
	GetBlitRects(srcRect, dstRect);

	if (srcRect.Width <= 0 || srcRect.Height <= 0)
		return virtualPoint;

	Point2D screenPoint;
	screenPoint.X = static_cast<int>(dstRect.X + (static_cast<double>(virtualPoint.X - srcRect.X) * dstRect.Width / srcRect.Width) + 0.5);
	screenPoint.Y = static_cast<int>(dstRect.Y + (static_cast<double>(virtualPoint.Y - srcRect.Y) * dstRect.Height / srcRect.Height) + 0.5);

	return screenPoint;
}

// Performs high-performance CPU software stretch blit directly in RAM, bypassing DirectDraw Blt stalls
void ZoomManager::FastStretchBlit(DSurface* pDst, const RectangleStruct& dstRect, DSurface* pSrc, const RectangleStruct& srcRect)
{
	if (!pDst || !pSrc || dstRect.Width <= 0 || dstRect.Height <= 0 || srcRect.Width <= 0 || srcRect.Height <= 0)
		return;

	WORD* pSrcBase = static_cast<WORD*>(pSrc->Lock(0, 0));
	WORD* pDstBase = static_cast<WORD*>(pDst->Lock(0, 0));

	if (!pSrcBase || !pDstBase)
	{
		if (pDstBase)
			pDst->Unlock();

		if (pSrcBase)
			pSrc->Unlock();

		RectangleStruct clippedDst = dstRect;
		RectangleStruct clippedSrc = srcRect;
		pDst->CopyFrom(&clippedDst, &clippedDst, pSrc, &clippedDst, &clippedSrc, false, false);
		return;
	}

	const int srcPitch = pSrc->GetPitch();
	const int dstPitch = pDst->GetPitch();
	const BYTE* pSrcBytes = reinterpret_cast<const BYTE*>(pSrcBase);
	BYTE* pDstBytes = reinterpret_cast<BYTE*>(pDstBase);

	const int dw = dstRect.Width;
	const int dh = dstRect.Height;
	const int sw = srcRect.Width;
	const int sh = srcRect.Height;
	const int srcMaxX = pSrc->Width - 1;
	const int srcMaxY = pSrc->Height - 1;

	thread_local std::vector<WORD> xMap;
	if (static_cast<int>(xMap.size()) < dw)
		xMap.resize(dw);

	for (int dx = 0; dx < dw; ++dx)
	{
		const int sx = srcRect.X + (dx * sw) / dw;
		xMap[dx] = static_cast<WORD>(std::clamp(sx, 0, srcMaxX));
	}

	const int hexadecs = dw / 16;
	const size_t rowBytes = static_cast<size_t>(dw) * sizeof(WORD);
	int lastSy = -1;
	const WORD* pPrevDstRow = nullptr;

	for (int dy = 0; dy < dh; ++dy)
	{
		int sy = srcRect.Y + (dy * sh) / dh;
		sy = std::clamp(sy, 0, srcMaxY);

		WORD* pDstRow = reinterpret_cast<WORD*>(pDstBytes + (dstRect.Y + dy) * dstPitch) + dstRect.X;

		if (sy == lastSy && pPrevDstRow)
		{
			std::memcpy(pDstRow, pPrevDstRow, rowBytes);
			continue;
		}

		pPrevDstRow = pDstRow;
		lastSy = sy;

		const WORD* pSrcRow = reinterpret_cast<const WORD*>(pSrcBytes + sy * srcPitch);
		int dx = 0;

		for (int h = 0; h < hexadecs; ++h, dx += 16)
		{
			const WORD p0 = pSrcRow[xMap[dx + 0]];
			const WORD p1 = pSrcRow[xMap[dx + 1]];
			const WORD p2 = pSrcRow[xMap[dx + 2]];
			const WORD p3 = pSrcRow[xMap[dx + 3]];
			const WORD p4 = pSrcRow[xMap[dx + 4]];
			const WORD p5 = pSrcRow[xMap[dx + 5]];
			const WORD p6 = pSrcRow[xMap[dx + 6]];
			const WORD p7 = pSrcRow[xMap[dx + 7]];

			const __m128i val0 = _mm_setr_epi16(p0, p1, p2, p3, p4, p5, p6, p7);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(pDstRow + dx + 0), val0);

			const WORD p8 = pSrcRow[xMap[dx + 8]];
			const WORD p9 = pSrcRow[xMap[dx + 9]];
			const WORD p10 = pSrcRow[xMap[dx + 10]];
			const WORD p11 = pSrcRow[xMap[dx + 11]];
			const WORD p12 = pSrcRow[xMap[dx + 12]];
			const WORD p13 = pSrcRow[xMap[dx + 13]];
			const WORD p14 = pSrcRow[xMap[dx + 14]];
			const WORD p15 = pSrcRow[xMap[dx + 15]];

			const __m128i val1 = _mm_setr_epi16(p8, p9, p10, p11, p12, p13, p14, p15);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(pDstRow + dx + 8), val1);
		}

		for (; dx + 8 <= dw; dx += 8)
		{
			const WORD p0 = pSrcRow[xMap[dx + 0]];
			const WORD p1 = pSrcRow[xMap[dx + 1]];
			const WORD p2 = pSrcRow[xMap[dx + 2]];
			const WORD p3 = pSrcRow[xMap[dx + 3]];
			const WORD p4 = pSrcRow[xMap[dx + 4]];
			const WORD p5 = pSrcRow[xMap[dx + 5]];
			const WORD p6 = pSrcRow[xMap[dx + 6]];
			const WORD p7 = pSrcRow[xMap[dx + 7]];

			const __m128i val = _mm_setr_epi16(p0, p1, p2, p3, p4, p5, p6, p7);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(pDstRow + dx), val);
		}

		for (; dx < dw; ++dx)
			pDstRow[dx] = pSrcRow[xMap[dx]];
	}

	pDst->Unlock();
	pSrc->Unlock();
}

// Blits centered viewport crop from Alternate surface onto Composite surface
void ZoomManager::ApplyTacticalBlit()
{
	if (!IsZoomed() || !DSurface::Alternate || !DSurface::Composite)
		return;

	BlitAppliedThisFrame = true;

	RectangleStruct srcRect, dstRect;
	GetBlitRects(srcRect, dstRect);

	FastStretchBlit(DSurface::Composite, dstRect, DSurface::Alternate, srcRect);
}

// Redirects hardcoded DSurface::Composite references in world-space rendering to DSurface::Temp
void ZoomManager::ApplySurfacePatches()
{
	static bool applied = false;
	if (applied)
		return;

	applied = true;

	// EBolt::Draw: redirect target surface from DSurface::Composite to DSurface::Temp
	Patch::Apply_RAW(0x4C1EF4 + 2, { 0x14 });
	Patch::Apply_RAW(0x4C24EC + 2, { 0x14 });
	Patch::Apply_RAW(0x4C2601 + 2, { 0x14 });
	Patch::Apply_RAW(0x4C26EE + 2, { 0x14 });

	// LineTrail::Draw: redirect target surface from DSurface::Composite to DSurface::Temp
	Patch::Apply_RAW(0x556CDA + 2, { 0x14 });

	// FootClass::Draw_Action_Line (non-player): redirect target surface from DSurface::Composite to DSurface::Temp
	Patch::Apply_RAW(0x4DC5BD + 2, { 0x14 });
	Patch::Apply_RAW(0x4DC64A + 2, { 0x14 });
	Patch::Apply_RAW(0x4DC6FE + 1, { 0x14 });

	// TechnoClass order and target action lines (0x007049C0, called by FootClass::DrawActionLines for player units):
	// redirect target surface from DSurface::Composite to DSurface::Temp
	Patch::Apply_RAW(0x704AE6 + 2, { 0x14 });
	Patch::Apply_RAW(0x704B93 + 2, { 0x14 });
	Patch::Apply_RAW(0x704C13 + 2, { 0x14 });
	Patch::Apply_RAW(0x704CFD + 2, { 0x14 });
	Patch::Apply_RAW(0x704D87 + 2, { 0x14 });
	Patch::Apply_RAW(0x704DDD + 2, { 0x14 });
	Patch::Apply_RAW(0x704E1D + 2, { 0x14 });

	// TechnoClass mind control, slave, and spawn action lines (0x00704E40):
	// redirect target surface from DSurface::Composite to DSurface::Temp
	Patch::Apply_RAW(0x704F76 + 2, { 0x14 });
	Patch::Apply_RAW(0x705000 + 2, { 0x14 });
	Patch::Apply_RAW(0x7051BB + 2, { 0x14 });
	Patch::Apply_RAW(0x70521F + 2, { 0x14 });

	// Planning Mode waypoint nodes and lines: redirect target surface from DSurface::Composite to DSurface::Temp
	Patch::Apply_RAW(0x6353CF + 2, { 0x14 });
	Patch::Apply_RAW(0x635530 + 2, { 0x14 });
	Patch::Apply_RAW(0x63560D + 2, { 0x14 });
	Patch::Apply_RAW(0x635758 + 2, { 0x14 });
	Patch::Apply_RAW(0x63B28D + 2, { 0x14 });
	Patch::Apply_RAW(0x63B3CA + 2, { 0x14 });
	Patch::Apply_RAW(0x63B4E6 + 2, { 0x14 });
	Patch::Apply_RAW(0x63B792 + 2, { 0x14 });
	Patch::Apply_RAW(0x63B88F + 2, { 0x14 });
	Patch::Apply_RAW(0x63B8F8 + 2, { 0x14 });
	Patch::Apply_RAW(0x63B9F4 + 2, { 0x14 });
	Patch::Apply_RAW(0x63BA68 + 2, { 0x14 });
	Patch::Apply_RAW(0x63BBA5 + 2, { 0x14 });
	Patch::Apply_RAW(0x63BC1B + 2, { 0x14 });
	Patch::Apply_RAW(0x63C14D + 2, { 0x14 });
	Patch::Apply_RAW(0x63C208 + 2, { 0x14 });
	Patch::Apply_RAW(0x63C311 + 2, { 0x14 });
	Patch::Apply_RAW(0x63C395 + 2, { 0x14 });
	Patch::Apply_RAW(0x63C476 + 2, { 0x14 });
	Patch::Apply_RAW(0x63C948 + 2, { 0x14 });
	Patch::Apply_RAW(0x63CEE3 + 2, { 0x14 });
	Patch::Apply_RAW(0x63D44A + 2, { 0x14 });
	Patch::Apply_RAW(0x63D4D5 + 2, { 0x14 });
	Patch::Apply_RAW(0x63D5F5 + 2, { 0x14 });
	Patch::Apply_RAW(0x63D6E2 + 2, { 0x14 });
	Patch::Apply_RAW(0x63D758 + 2, { 0x14 });
	Patch::Apply_RAW(0x63D7E7 + 2, { 0x14 });
	Patch::Apply_RAW(0x63D8C3 + 2, { 0x14 });
	Patch::Apply_RAW(0x63D8F0 + 2, { 0x14 });
}

