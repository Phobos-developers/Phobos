#include "OwnerDraw.Internal.h"

static void DrawGroupBoxLine(int startX, int startY, int endX, int endY, int color)
{
	Point2D start { startX, startY };
	Point2D end { endX, endY };
	DSurface::Alternate->DrawLine(&start, &end, color);
}

static void DrawGroupBoxPixel(int x, int y, int color)
{
	Point2D point { x, y };
	DSurface::Alternate->SetPixel(&point, color);
}

static int MeasureGroupBoxCaption(BitFont* pFont, const wchar_t* pCaption, int maxWidth)
{
	if (!pFont)
		pFont = BitFont::Instance;

	int width = 0;
	int height = 0;
	if (pFont)
		pFont->GetTextDimension(pCaption, &width, &height, maxWidth);

	return width;
}

static void DrawGroupBoxCaption(const RECT& groupRect, const wchar_t* pCaption)
{
	RectangleStruct captionRect
	{
		groupRect.left + 10,
		groupRect.top,
		groupRect.right - groupRect.left,
		groupRect.bottom - groupRect.top
	};

	OwnerDraw::PrintTextFixedLength(
		Phobos::UI::ColorTextGroupbox,
		nullptr,
		&captionRect,
		pCaption,
		static_cast<int>(std::wcslen(pCaption)),
		0,
		0,
		DSurface::Alternate,
		0);
}

static void DrawGroupBoxFramePass(
	const RECT& groupRect,
	int topY,
	int inset,
	bool hasCaption,
	int captionWidth,
	int nearColor,
	int farColor,
	int cornerColor)
{
	const int left = groupRect.left + inset;
	const int right = groupRect.right - inset - 1;
	const int y = topY + inset;
	const int bottom = groupRect.bottom - inset - 1;

	if (hasCaption)
	{
		DrawGroupBoxLine(left, y, groupRect.left + 8, y, nearColor);
		DrawGroupBoxLine(groupRect.left + captionWidth + 12 + inset, y, right, y, nearColor);
	}
	else
	{
		DrawGroupBoxLine(left, y, right, y, nearColor);
	}

	DrawGroupBoxLine(left, y + 1, left, bottom, nearColor);
	DrawGroupBoxLine(left, bottom, right, bottom, farColor);
	DrawGroupBoxLine(right, y, right, bottom - 1, farColor);
	DrawGroupBoxPixel(right, y, cornerColor);
	DrawGroupBoxPixel(left, bottom, cornerColor);
}

static LRESULT PaintGroupBoxCtrl(HWND hWnd, OwnerDrawDialogElement& data)
{
	const wchar_t* const pCaption = data.TextBuffer ? data.TextBuffer : L"";

	RECT groupRect {};
	OwnerDraw::GetRectangle(hWnd, &groupRect);

	const int groupWidth = groupRect.right - groupRect.left;
	const auto pFont = data.AsGroupBox().Font() ? data.AsGroupBox().Font() : BitFont::Instance;
	const int topY = groupRect.top + BitFontHeight(pFont) / 2;
	const int captionWidth = MeasureGroupBoxCaption(pFont, pCaption, groupWidth);

	DrawGroupBoxCaption(groupRect, pCaption);

	const int lightColor = ConvertRGBToSurfaceColor(Phobos::UI::ColorBorder1);
	const int shadowColor = ConvertRGBToSurfaceColor(Phobos::UI::ColorBorder2);
	const int averageColor = ConvertRGBToSurfaceColor(AverageColor(Phobos::UI::ColorBorder1, Phobos::UI::ColorBorder2));
	const bool hasCaption = std::wcslen(pCaption) != 0;

	DrawGroupBoxFramePass(groupRect, topY, 0, hasCaption, captionWidth, lightColor, shadowColor, averageColor);
	DrawGroupBoxFramePass(groupRect, topY, 1, hasCaption, captionWidth, shadowColor, lightColor, averageColor);

	::ValidateRect(hWnd, nullptr);
	return 0;
}

LRESULT CALLBACK WWUI::GroupBoxCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		if (auto pData = FindOwnerDrawData(hWnd))
			return PaintGroupBoxCtrl(hWnd, *pData);

		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_NCPAINT:
		return 0;

	default:
		return CallSelectedHandler(FindWindowProc(OwnerDraw::DialogProcs, hWnd), hWnd, message, wParam, lParam);
	}
}
