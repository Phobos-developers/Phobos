#include "OwnerDraw.Internal.h"

constexpr int SliderValueLabelWidth = 50;
constexpr int SliderTrackRightPadding = 13;
constexpr int SliderGripCenterOffset = 6;
constexpr int SliderGripHitWidth = 12;
constexpr int SliderMouseHitBottomInset = 18;

static int SliderTrackTravel(const RECT& clientRect, int valueLabelWidth)
{
	const int travel = clientRect.right - clientRect.left - valueLabelWidth - SliderTrackRightPadding;
	return travel > 1 ? travel : 1;
}

static int SliderThumbOffsetFromPosition(int positionOffset, int trackTravel, int rangeSpan)
{
	if (!rangeSpan)
		return 0;

	return positionOffset * trackTravel / rangeSpan;
}

static int SliderClampedGripX(int x, const RECT& clientRect, int valueLabelWidth)
{
	int gripX = x - SliderGripCenterOffset;
	if (gripX < 1)
		gripX = 1;

	const int maxGripX = clientRect.right - valueLabelWidth - SliderGripHitWidth;
	if (maxGripX < gripX)
		gripX = maxGripX;

	return gripX;
}

static void SliderUpdateFromGripX(
	int gripX,
	int rangeSpan,
	int rangeMin,
	int stepValue,
	int trackTravel,
	int& positionOffset,
	int& thumbOffsetPixels)
{
	if (!trackTravel)
		trackTravel = 1;

	int offset = (rangeSpan + 1) * (gripX - 1) / trackTravel;
	if (offset >= rangeSpan)
		offset = rangeSpan;

	if (!stepValue)
		stepValue = 1;

	positionOffset = stepValue * ((rangeMin + offset) / stepValue) - rangeMin;
	thumbOffsetPixels = SliderThumbOffsetFromPosition(positionOffset, trackTravel, rangeSpan);
}

static void EnsureSliderCache(HWND hWnd, OwnerDrawDialogElement& data, const RECT& clientRect, const RECT& ownerRect)
{
	if (data.CacheSurface || !DSurface::Alternate)
		return;

	const int width = clientRect.right + 1;
	const int height = clientRect.bottom + 1;
	if (width <= 0 || height <= 0)
		return;

	data.CacheSurface = GameCreate<BSurface>(width, height);
	++OwnerDraw::CachedSurfaceCount;

	RectangleStruct destRect { 0, 0, width, height };
	RectangleStruct sourceRect { ownerRect.left, ownerRect.top, width, height };
	CopySurfacePart(data.CacheSurface, destRect, DSurface::Alternate, sourceRect);
	BlendFillRect(destRect, data.CacheSurface, 0, static_cast<unsigned char>(data.Alpha));
}

static int SliderBorderColor(bool disabled)
{
	const COLORREF color = disabled ? OwnerDraw::DisabledBorderColor : OwnerDraw::DefaultBorderColor;
	if (color == static_cast<COLORREF>(-1))
		return -1;

	return ConvertRGBToSurfaceColor(color);
}

static void PaintSlider(
	HWND hWnd,
	OwnerDrawDialogElement& data,
	const RECT& clientRect,
	const RECT& ownerRect,
	int thumbHitLeftX,
	int thumbHitRightX,
	int rangeMin,
	int positionOffset,
	int stepValue,
	int valueLabelWidth,
	bool showValueLabel)
{
	if (!DSurface::Alternate)
		return;

	const int controlLeft = ownerRect.left;
	const int controlTop = ownerRect.top;
	const int controlWidth = ownerRect.right - ownerRect.left;
	const int controlHeight = ownerRect.bottom - ownerRect.top;

	const LONG style = ::GetWindowLongA(hWnd, GWL_STYLE);
	const bool disabled = (style & WS_DISABLED) != 0;

	EnsureSliderCache(hWnd, data, clientRect, ownerRect);

	if (data.CacheSurface)
	{
		RectangleStruct destRect { ownerRect.left, ownerRect.top, clientRect.right + 1, clientRect.bottom + 1 };
		RectangleStruct sourceRect { 0, 0, clientRect.right + 1, clientRect.bottom + 1 };
		CopySurfacePart(DSurface::Alternate, destRect, data.CacheSurface, sourceRect);
	}

	if (showValueLabel)
	{
		const int labelPanelX = controlWidth - valueLabelWidth + controlLeft + 1;
		const int labelPanelTop = controlTop - 1;

		if (auto pMiddle = GetPCXSurface("trofm.pcx"))
		{
			RectangleStruct middleRect { labelPanelX, labelPanelTop, valueLabelWidth, pMiddle->GetHeight() };
			BlitTiledPCX(middleRect, DSurface::Alternate, pMiddle, 0, 0);
		}

		if (auto pLeft = GetPCXSurface("trofl.pcx"))
		{
			RectangleStruct leftRect { labelPanelX, labelPanelTop, pLeft->GetWidth(), pLeft->GetHeight() };
			DrawPCXCopy(DSurface::Alternate, leftRect, pLeft);
		}

		if (auto pRight = GetPCXSurface("trofr.pcx"))
		{
			RectangleStruct rightRect
			{
				controlLeft + controlWidth - pRight->GetWidth() + 1,
				labelPanelTop,
				pRight->GetWidth(),
				pRight->GetHeight()
			};
			DrawPCXCopy(DSurface::Alternate, rightRect, pRight);
		}
	}

	RectangleStruct gripRect
	{
		controlLeft + thumbHitLeftX,
		controlTop,
		thumbHitRightX - thumbHitLeftX,
		controlHeight
	};

	if (auto pGrip = GetPCXSurface("trakgrip.pcx"))
		DrawPCXCopy(DSurface::Alternate, gripRect, pGrip);

	if (disabled)
		BlendFillRect(gripRect, DSurface::Alternate, 0, OwnerDraw::DisabledOverlayAlpha);

	const int borderColor = SliderBorderColor(disabled);
	RectangleStruct trackBorderRect
	{
		controlLeft,
		controlTop,
		controlWidth - valueLabelWidth,
		controlHeight
	};
	DrawBeveledBorder(DSurface::Alternate, trackBorderRect, 2, borderColor);

	const int labelInset = (showValueLabel ? 1 : 0) + 1;
	RectangleStruct labelBorderRect
	{
		controlLeft + controlWidth + labelInset - valueLabelWidth,
		controlTop,
		valueLabelWidth - labelInset,
		controlHeight
	};
	DrawBeveledBorder(DSurface::Alternate, labelBorderRect, 2, borderColor);

	if (showValueLabel)
	{
		if (!stepValue)
			stepValue = 1;

		wchar_t text[0x100] {};
		std::swprintf(text, std::size(text), L"%d", stepValue * ((rangeMin + positionOffset) / stepValue));

		RECT textRect
		{
			ownerRect.right - 49,
			ownerRect.top,
			ownerRect.right,
			ownerRect.bottom
		};

		const COLORREF textColor = disabled ? Phobos::UI::ColorDisabledSlider : Phobos::UI::ColorTextSlider;
		OwnerDraw::DrawWideText(DSurface::Alternate, text, &textRect, data.AsSlider().Font(), textColor, 5, 12, 0, 0, 0);
	}
}

LRESULT CALLBACK WWUI::SliderCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT clientRect {};
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates() || !RenderDX::GetClientRectInRender(hWnd, &clientRect))
		::GetClientRect(hWnd, &clientRect);

	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return 0;

	auto& data = *pData;

	int playClickSound = 1;
	int isMouseTracking = data.AsSlider().IsMouseTracking();
	int isThumbDragging = data.AsSlider().IsThumbDragging();
	int rangeSpan = data.AsSlider().RangeSpan();
	int positionOffset = data.AsSlider().PositionOffset();
	int rangeMin = data.AsSlider().RangeMin();
	int thumbOffsetPixels = data.AsSlider().ThumbOffsetPixels();
	int stepValue = data.AsSlider().StepValue();
	int showValueLabel = data.AsSlider().ShowValueLabel();

	int valueLabelWidth = 0;
	int trackTravel = 1;
	auto updateTrackMetrics = [&]()
	{
		if (!stepValue)
		{
			valueLabelWidth = SliderValueLabelWidth;
			stepValue = 1;
			showValueLabel = 1;
		}
		else
		{
			valueLabelWidth = showValueLabel ? SliderValueLabelWidth : 0;
		}

		trackTravel = SliderTrackTravel(clientRect, valueLabelWidth);
	};

	auto updateThumbFromPosition = [&]()
	{
		thumbOffsetPixels = SliderThumbOffsetFromPosition(positionOffset, trackTravel, rangeSpan);
	};

	updateTrackMetrics();

	if (!rangeSpan)
	{
		const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
		const int rangeMax = static_cast<int>(CallSelectedHandler(pOriginalWndProc, hWnd, WW_SLIDER_GETRANGEMAX, 0, 0));
		rangeMin = static_cast<int>(CallSelectedHandler(pOriginalWndProc, hWnd, WW_SLIDER_GETRANGEMIN, 0, 0));
		rangeSpan = rangeMax - rangeMin;
		if (!rangeSpan)
			rangeSpan = 100;

		positionOffset = static_cast<int>(CallSelectedHandler(pOriginalWndProc, hWnd, WW_SLIDER_GETPOS, 0, 0)) - rangeMin;
		updateThumbFromPosition();

		data.AsSlider().ThumbOffsetPixels() = thumbOffsetPixels;
		data.AsSlider().RangeSpan() = rangeSpan;
		data.AsSlider().PositionOffset() = positionOffset;
		data.AsSlider().RangeMin() = rangeMin;
		data.AsSlider().StepValue() = stepValue;
		data.AsSlider().ShowValueLabel() = showValueLabel;
	}

	if (!isThumbDragging)
		updateThumbFromPosition();

	int thumbHitLeftX = clientRect.left + thumbOffsetPixels + 1;
	int thumbHitRightX = thumbHitLeftX + SliderGripHitWidth;

	if (isThumbDragging)
	{
		POINT point {};
		::GetCursorPos(&point);
		point = RenderDX::ScreenToRenderLocalPoint(hWnd, point);

		SliderUpdateFromGripX(
			SliderClampedGripX(point.x, clientRect, valueLabelWidth),
			rangeSpan,
			rangeMin,
			stepValue,
			trackTravel,
			positionOffset,
			thumbOffsetPixels);

		thumbHitLeftX = thumbOffsetPixels + 1;
		thumbHitRightX = thumbHitLeftX + SliderGripHitWidth;
	}

	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(FindWindowProc(OwnerDraw::DialogProcs, hWnd), hWnd, message, wParam, lParam);
	};

	auto writeBack = [&]() -> LRESULT
	{
		const bool notifyChanged =
			positionOffset != data.AsSlider().PositionOffset()
			|| rangeSpan != data.AsSlider().RangeSpan()
			|| rangeMin != data.AsSlider().RangeMin();
		const bool visualChanged =
			notifyChanged
			|| thumbOffsetPixels != data.AsSlider().ThumbOffsetPixels()
			|| stepValue != data.AsSlider().StepValue()
			|| showValueLabel != data.AsSlider().ShowValueLabel();

		data.AsSlider().IsMouseTracking() = isMouseTracking;
		data.AsSlider().IsThumbDragging() = isThumbDragging;
		data.AsSlider().ThumbOffsetPixels() = thumbOffsetPixels;
		data.AsSlider().RangeSpan() = rangeSpan;
		data.AsSlider().PositionOffset() = positionOffset;
		data.AsSlider().RangeMin() = rangeMin;
		data.AsSlider().StepValue() = stepValue;
		data.AsSlider().ShowValueLabel() = showValueLabel;

		if (visualChanged)
			::InvalidateRect(hWnd, nullptr, FALSE);

		if (notifyChanged)
		{
			::SendMessageA(::GetParent(hWnd), WM_HSCROLL, MAKELONG(SB_THUMBTRACK, positionOffset + rangeMin), reinterpret_cast<LPARAM>(hWnd));

			if (playClickSound == 1 && !data.AsSlider().SuppressClickSound() && RulesClass::Instance)
				VocClass::PlayGlobal(RulesClass::Instance->GenericClick, 0x2000, 1.0f);
		}

		return 0;
	};

	switch (message)
	{
	case WM_ERASEBKGND:
		return 0;

	case WM_NCHITTEST:
	case WM_GETDLGCODE:
		return forwardOriginal();

	case WM_ENABLE:
		::InvalidateRect(hWnd, nullptr, FALSE);
		return writeBack();

	case WM_PAINT:
		PaintSlider(
			hWnd,
			data,
			clientRect,
			ownerRect,
			thumbHitLeftX,
			thumbHitRightX,
			rangeMin,
			positionOffset,
			stepValue,
			valueLabelWidth,
			showValueLabel != 0);
		::ValidateRect(hWnd, nullptr);
		return writeBack();

	case WM_MOUSEMOVE:
		if (isThumbDragging)
			::InvalidateRect(hWnd, nullptr, FALSE);

		if (wParam & MK_LBUTTON)
			return writeBack();

		[[fallthrough]];

	case WM_LBUTTONUP:
		isMouseTracking = 0;
		isThumbDragging = 0;
		::ReleaseCapture();
		return writeBack();

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (message == WM_LBUTTONDOWN)
		{
			isMouseTracking = 1;
			::SetCapture(hWnd);
		}
		else
		{
			isMouseTracking = 0;
			::ReleaseCapture();
		}

		{
			const POINT point = RenderDX::MouseLParamToRenderLocalPoint(hWnd, lParam);
			if (point.y > clientRect.bottom - SliderMouseHitBottomInset)
			{
				const int clickX = point.x;
				if (clickX < thumbHitLeftX || clickX >= thumbHitRightX)
				{
					SliderUpdateFromGripX(
						SliderClampedGripX(clickX, clientRect, valueLabelWidth),
						rangeSpan,
						rangeMin,
						stepValue,
						trackTravel,
						positionOffset,
						thumbOffsetPixels);
				}
				else if (message == WM_LBUTTONDOWN)
				{
					isThumbDragging = 1;
				}
			}
		}
		return writeBack();

	case WW_SLIDER_GETPOS:
		return stepValue * ((rangeMin + positionOffset) / stepValue);

	case WW_SLIDER_SETPOS:
	{
		const int requestedOffset = static_cast<int>(lParam) - rangeMin;
		if (requestedOffset <= rangeSpan && requestedOffset >= 0)
			positionOffset = requestedOffset;

		playClickSound = 0;
		updateThumbFromPosition();
		return writeBack();
	}

	case WW_SLIDER_SETRANGE:
		rangeMin = LOWORD(lParam);
		rangeSpan = HIWORD(lParam) - LOWORD(lParam);
		if (positionOffset > rangeSpan)
			positionOffset = rangeSpan;

		if (positionOffset < 0)
			positionOffset = 0;

		playClickSound = 0;
		updateThumbFromPosition();
		return writeBack();

	case WW_SLIDER_SETSTEP:
		stepValue = static_cast<int>(lParam);
		updateTrackMetrics();
		updateThumbFromPosition();
		return writeBack();

	case WW_SLIDER_SHOWVALUE:
		showValueLabel = static_cast<int>(lParam);
		updateTrackMetrics();
		updateThumbFromPosition();
		return writeBack();

	case WW_SLIDER_SUPPRESSCLICK:
		data.AsSlider().SuppressClickSound() = wParam == 0;
		return writeBack();

	default:
		return writeBack();
	}
}
