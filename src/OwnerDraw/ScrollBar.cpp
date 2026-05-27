#include "OwnerDraw.Internal.h"

constexpr int ScrollBarButtonHeight = 22;
constexpr int ScrollBarMinimumThumbHeight = 14;
constexpr int ScrollBarInitialRepeatMs = 500;
constexpr int ScrollBarRepeatMs = 25;

void DrawScrollArrow(Surface* pSurface, const RectangleStruct& rect, bool isUp, bool pressed, bool useGreyArt)
{
	char filename[32] {};
	std::snprintf(filename, sizeof(filename), isUp ? "guparrow%c.pcx" : "gdnarrow%c.pcx", pressed ? 'p' : 'r');

	const char* pFilename = useGreyArt ? filename : filename + 1;
	DrawPCXCopy(pSurface, rect, GetPCXSurface(pFilename));
}

static void BlendScrollBarCache(OwnerDrawDialogElement& data, int width, int height)
{
	auto pPixels = static_cast<WORD*>(data.CacheSurface->Lock(0, 0));
	if (!pPixels)
		return;

	const int pixelCount = width * height;
	const int alpha = static_cast<unsigned char>(data.Alpha);

	for (int i = 0; i < pixelCount; ++i)
	{
		pPixels[i] = BlendSurfacePixelTowardMasks(pPixels[i], alpha);
	}

	data.CacheSurface->Unlock();
}

void EnsureScrollBarCache(
	OwnerDrawDialogElement& data,
	OwnerDrawDialogElement* pParentData,
	const RectangleStruct& localRect,
	const RectangleStruct& parentSourceRect)
{
	if (data.CacheSurface)
	{
		if (data.CacheSurface->GetWidth() != localRect.Width || data.CacheSurface->GetHeight() != localRect.Height)
			DeleteSurfaceObject(data.CacheSurface);
	}

	if (data.CacheSurface || localRect.Width <= 0 || localRect.Height <= 0)
		return;

	data.CacheSurface = GameCreate<BSurface>(localRect.Width, localRect.Height);
	++OwnerDraw::CachedSurfaceCount;

	if (pParentData && pParentData->CacheSurface)
		CopySurfacePart(data.CacheSurface, localRect, pParentData->CacheSurface, parentSourceRect);

	BlendScrollBarCache(data, localRect.Width, localRect.Height);
}

static void PaintScrollBar(
	HWND hWnd,
	OwnerDrawDialogElement& data,
	const RECT& clientRect,
	const RECT& scrollBarRect,
	int thumbTop,
	int thumbBottom,
	bool upPressed,
	bool downPressed)
{
	if (!DSurface::Alternate)
		return;

	RectangleStruct localRect { 0, 0, clientRect.right, clientRect.bottom };
	RectangleStruct destRect { scrollBarRect.left, scrollBarRect.top, clientRect.right, clientRect.bottom };

	const HWND parentHwnd = ::GetParent(hWnd);
	auto pParentData = parentHwnd ? FindOwnerDrawData(parentHwnd) : nullptr;

	RECT parentRect {};
	if (parentHwnd)
		OwnerDraw::GetRectangle(parentHwnd, &parentRect);

	RectangleStruct parentSourceRect = localRect;
	if (pParentData && pParentData->CacheSurface)
	{
		parentSourceRect.X = scrollBarRect.left - parentRect.left;
		parentSourceRect.Y = scrollBarRect.top - parentRect.top;
	}

	EnsureScrollBarCache(data, pParentData, localRect, parentSourceRect);

	if (pParentData && pParentData->CacheSurface)
		CopySurfacePart(DSurface::Alternate, destRect, pParentData->CacheSurface, parentSourceRect);

	const bool disabled = data.AsScrollBar().Disabled();
	int borderColor = ConvertRGBToSurfaceColor(disabled ? OwnerDraw::AltBorderColor : OwnerDraw::DefaultBorderColor);
	if (disabled && OwnerDraw::AltBorderColor == static_cast<COLORREF>(-1))
		borderColor = -1;

	DrawBeveledBorder(DSurface::Alternate, destRect, 2, borderColor);

	RectangleStruct thumbRect
	{
		scrollBarRect.left,
		scrollBarRect.top + thumbTop,
		clientRect.right,
		thumbBottom - thumbTop
	};

	if (auto pGripMiddle = GetPCXSurface(disabled ? "gsbgripm.pcx" : "sbgripm.pcx"))
	{
		auto middleRect = thumbRect;
		middleRect.Width = pGripMiddle->GetWidth();
		BlitTiledPCX(middleRect, DSurface::Alternate, pGripMiddle, 0, 0);
	}

	if (auto pGripTop = GetPCXSurface(disabled ? "gsbgript.pcx" : "sbgript.pcx"))
		DrawPCXCopy(DSurface::Alternate, thumbRect, pGripTop);

	if (auto pGripBottom = GetPCXSurface(disabled ? "gsbgripb.pcx" : "sbgripb.pcx"))
	{
		RectangleStruct bottomRect
		{
			thumbRect.X,
			scrollBarRect.top + thumbBottom - pGripBottom->GetHeight(),
			thumbRect.Width,
			thumbRect.Height
		};

		DrawPCXCopy(DSurface::Alternate, bottomRect, pGripBottom);
	}

	RectangleStruct upButtonRect { scrollBarRect.left, scrollBarRect.top, clientRect.right, ScrollBarButtonHeight };
	RectangleStruct upButtonSource { 0, 0, clientRect.right, ScrollBarButtonHeight };
	CopySurfacePart(DSurface::Alternate, upButtonRect, data.CacheSurface, upButtonSource);

	const BYTE bevelAlpha = static_cast<BYTE>(OwnerDraw::ScrollButtonBevelAlpha);
	DrawAlphaBeveledRect(DSurface::Alternate, upButtonRect, !upPressed, 2, bevelAlpha, bevelAlpha, bevelAlpha, bevelAlpha);
	DrawScrollArrow(DSurface::Alternate, upButtonRect, true, upPressed, disabled);

	RectangleStruct downButtonRect
	{
		scrollBarRect.left,
		scrollBarRect.bottom - ScrollBarButtonHeight,
		clientRect.right,
		ScrollBarButtonHeight
	};
	RectangleStruct downButtonSource { 0, clientRect.bottom - ScrollBarButtonHeight, clientRect.right, ScrollBarButtonHeight };
	CopySurfacePart(DSurface::Alternate, downButtonRect, data.CacheSurface, downButtonSource);
	DrawAlphaBeveledRect(DSurface::Alternate, downButtonRect, !downPressed, 2, bevelAlpha, bevelAlpha, bevelAlpha, bevelAlpha);
	DrawScrollArrow(DSurface::Alternate, downButtonRect, false, downPressed, disabled);
}

LRESULT CALLBACK WWUI::ScrollBarCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT clientRect {};
	::GetClientRect(hWnd, &clientRect);

	RECT scrollBarRect {};
	OwnerDraw::GetRectangle(hWnd, &scrollBarRect);

	const int inset = OwnerDraw::ControlInsetPx;
	const int clientWidth = clientRect.right - 2 * inset;
	clientRect.bottom -= 2 * inset;
	clientRect.right = clientWidth;
	scrollBarRect.left += inset;
	scrollBarRect.top += inset;
	scrollBarRect.right -= inset;
	scrollBarRect.bottom -= inset;

	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return 0;

	auto& data = *pData;

	const bool restoreCaptureToNotify = data.AsScrollBar().RestoreCaptureToNotifyHwnd() != 0;
	bool isMouseTracking = data.AsScrollBar().IsMouseTracking() != 0;
	bool isThumbDragging = data.AsScrollBar().IsThumbDragging() != 0;
	int rangeMax = data.AsScrollBar().RangeMax();
	int position = data.AsScrollBar().Position();
	bool upButtonPressed = data.AsScrollBar().UpButtonPressed() != 0;
	bool downButtonPressed = data.AsScrollBar().DownButtonPressed() != 0;

	if (!rangeMax)
		rangeMax = 100;

	const int scrollBarWidth = clientRect.right - clientRect.left;
	const int scrollBarLeft = clientRect.right - scrollBarWidth;
	const int trackHeight = clientRect.bottom - clientRect.top - 2 * ScrollBarButtonHeight;

	int thumbHeight = static_cast<int>(
		static_cast<double>(trackHeight)
		- std::log(static_cast<double>(rangeMax + 1)) * static_cast<double>(trackHeight) * 0.2);

	if (thumbHeight <= ScrollBarMinimumThumbHeight)
		thumbHeight = ScrollBarMinimumThumbHeight;

	int thumbTravel = trackHeight - thumbHeight;
	int thumbTravelDivisor = thumbTravel;
	if (thumbTravelDivisor <= 1)
	{
		thumbTravel = 1;
		thumbTravelDivisor = 1;
	}

	int thumbTop = 0;
	int thumbBottom = 0;
	int notifyCode = SB_LINEUP;

	auto updateThumbFromCursor = [&]()
	{
		POINT point {};
		::GetCursorPos(&point);
		::ScreenToClient(hWnd, &point);

		thumbTop = point.y - thumbHeight / 2;
		if (thumbTop < ScrollBarButtonHeight)
			thumbTop = ScrollBarButtonHeight;

		const int maxThumbTop = clientRect.bottom - thumbHeight - ScrollBarButtonHeight;
		if (maxThumbTop < thumbTop)
			thumbTop = maxThumbTop;

		thumbBottom = thumbTop + thumbHeight;
		notifyCode = SB_THUMBTRACK;
		position = rangeMax * (thumbTop - ScrollBarButtonHeight) / thumbTravelDivisor;
	};

	if (message < WM_USER || message == WW_SCROLLBAR_UPDATETHUMB)
	{
		if (isThumbDragging)
		{
			updateThumbFromCursor();
		}
		else
		{
			thumbTop = position * thumbTravel / rangeMax + clientRect.top + ScrollBarButtonHeight;
			thumbBottom = thumbTop + thumbHeight;
		}
	}

	auto restoreCapture = [&]()
	{
		::KillTimer(hWnd, 0);
		::ReleaseCapture();

		if (restoreCaptureToNotify && data.AsScrollBar().NotifyHwnd())
			::SetCapture(data.AsScrollBar().NotifyHwnd());
	};

	auto writeBack = [&]() -> LRESULT
	{
		const bool shouldNotify =
			(position != data.AsScrollBar().Position() || rangeMax != data.AsScrollBar().RangeMax())
			&& data.AsScrollBar().NotifyHwnd();

		data.AsScrollBar().IsMouseTracking() = isMouseTracking;
		data.AsScrollBar().IsThumbDragging() = isThumbDragging;
		data.AsScrollBar().RangeMax() = rangeMax;
		data.AsScrollBar().Position() = position;
		data.AsScrollBar().UpButtonPressed() = upButtonPressed;
		data.AsScrollBar().DownButtonPressed() = downButtonPressed;

		if (shouldNotify)
		{
			const WPARAM scrollParam = (static_cast<WPARAM>(position & 0xFFFF) << 16)
				| static_cast<WPARAM>(notifyCode & 0xFFFF);

			::SendMessageA(data.AsScrollBar().NotifyHwnd(), WM_VSCROLL, scrollParam, reinterpret_cast<LPARAM>(hWnd));
			::InvalidateRect(hWnd, nullptr, FALSE);
		}

		return 0;
	};

	switch (message)
	{
	case SBM_GETPOS:
		return position;

	case WM_ERASEBKGND:
		return 0;

	case WM_NCHITTEST:
	case WM_GETDLGCODE:
		return CallSelectedHandler(FindWindowProc(OwnerDraw::DialogProcs, hWnd), hWnd, message, wParam, lParam);

	case WM_PAINT:
		if (data.SkipDraw)
		{
			::ValidateRect(hWnd, nullptr);
			return writeBack();
		}

		if (data.NeedsControlImage)
			return 0;

		PaintScrollBar(
			hWnd,
			data,
			clientRect,
			scrollBarRect,
			thumbTop,
			thumbBottom,
			upButtonPressed,
			downButtonPressed);

		::ValidateRect(hWnd, nullptr);
		return writeBack();

	case SBM_SETPOS:
		if (static_cast<int>(wParam) <= rangeMax && static_cast<int>(wParam) > 0)
			position = static_cast<int>(wParam);

		return writeBack();

	case SBM_SETRANGE:
		rangeMax = static_cast<int>(lParam);
		if (position > rangeMax)
			position = rangeMax;

		return writeBack();

	case SBM_SETSCROLLINFO:
	{
		const auto pScrollInfo = reinterpret_cast<const SCROLLINFO*>(lParam);
		rangeMax = pScrollInfo->nMax;
		position = pScrollInfo->nPos;
		return writeBack();
	}

	case WM_TIMER:
	{
		POINT point {};
		::GetCursorPos(&point);
		::ScreenToClient(hWnd, &point);

		upButtonPressed = false;
		downButtonPressed = false;

		if (isMouseTracking && point.x > scrollBarLeft)
		{
			if (point.y < ScrollBarButtonHeight)
			{
				upButtonPressed = !isThumbDragging;
				if (position)
				{
					notifyCode = SB_LINEUP;
					--position;
					::SetTimer(hWnd, 0, ScrollBarRepeatMs, nullptr);
					return writeBack();
				}
			}
			else if (point.y > clientRect.bottom - ScrollBarButtonHeight)
			{
				downButtonPressed = !isThumbDragging;
				if (position + 1 <= rangeMax)
				{
					notifyCode = SB_LINEDOWN;
					++position;
				}
			}
		}

		::SetTimer(hWnd, 0, ScrollBarRepeatMs, nullptr);
		return writeBack();
	}

	case WM_MOUSEMOVE:
		if (isThumbDragging)
		{
			RECT invalidateRect
			{
				scrollBarLeft,
				clientRect.top,
				clientRect.right,
				clientRect.bottom
			};
			::InvalidateRect(hWnd, &invalidateRect, FALSE);
		}

		if (wParam & MK_LBUTTON)
			return writeBack();

		[[fallthrough]];

	case WM_LBUTTONUP:
		isMouseTracking = false;
		isThumbDragging = false;

		if (upButtonPressed || downButtonPressed)
			::InvalidateRect(hWnd, nullptr, FALSE);

		upButtonPressed = false;
		downButtonPressed = false;
		restoreCapture();
		notifyCode = SB_ENDSCROLL;
		return writeBack();

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (message == WM_LBUTTONDOWN)
		{
			isMouseTracking = true;
			::SetCapture(hWnd);
			::SetTimer(hWnd, 0, ScrollBarInitialRepeatMs, nullptr);
		}
		else
		{
			isMouseTracking = false;
			isThumbDragging = false;
			restoreCapture();
		}

		{
			const int clickX = LOWORD(lParam);
			const int clickY = HIWORD(lParam);
			const int repeatCount = message == WM_LBUTTONDBLCLK ? 2 : 1;

			upButtonPressed = false;
			downButtonPressed = false;

			for (int i = 0; i < repeatCount; ++i)
			{
				if (clickX <= scrollBarLeft)
					continue;

				if (clickY < ScrollBarButtonHeight && position)
				{
					upButtonPressed = true;
					notifyCode = SB_LINEUP;
					--position;
				}
				else if (clickY <= clientRect.bottom - ScrollBarButtonHeight || position + 1 > rangeMax)
				{
					if (clickY < thumbTop || clickY >= thumbBottom)
					{
						thumbTop = clickY - thumbHeight / 2;
						if (thumbTop < ScrollBarButtonHeight)
							thumbTop = ScrollBarButtonHeight;

						const int maxThumbTop = clientRect.bottom - thumbHeight - ScrollBarButtonHeight;
						if (maxThumbTop < thumbTop)
							thumbTop = maxThumbTop;

						thumbBottom = thumbTop + thumbHeight;
						notifyCode = SB_THUMBTRACK;
						position = rangeMax * (thumbTop - ScrollBarButtonHeight) / thumbTravelDivisor;
					}
					else if (message == WM_LBUTTONDOWN)
					{
						isThumbDragging = true;
					}
				}
				else
				{
					++position;
					downButtonPressed = true;
					notifyCode = SB_LINEDOWN;
				}
			}
		}

		return writeBack();

	default:
		break;
	}

	if (message == WW_DROPDOWN_SETACTIVE)
	{
		data.AsScrollBar().RestoreCaptureToNotifyHwnd() = lParam != 0;
		return writeBack();
	}

	if (message == WW_CB_SETALTERNATEPALETTE)
	{
		data.AsScrollBar().Disabled() = lParam == 1;
		return writeBack();
	}

	return writeBack();
}
