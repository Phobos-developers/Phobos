#include "OwnerDraw.Internal.h"

static void EnsureProgressCache(OwnerDrawDialogElement& data, const RectangleStruct& cacheRect, const RectangleStruct& screenRect)
{
	if (data.CacheSurface || !DSurface::Alternate || cacheRect.Width <= 0 || cacheRect.Height <= 0)
		return;

	data.CacheSurface = GameCreate<BSurface>(cacheRect.Width, cacheRect.Height);
	if (!data.CacheSurface)
		return;

	++OwnerDraw::CachedSurfaceCount;
	CopySurfacePart(data.CacheSurface, cacheRect, DSurface::Alternate, screenRect);
}

static void PaintProgress(OwnerDrawDialogElement& data, const RECT& ownerRect)
{
	if (!DSurface::Alternate)
		return;

	const int width = ownerRect.right - ownerRect.left + 1;
	const int height = ownerRect.bottom - ownerRect.top + 1;
	if (width <= 0 || height <= 0)
		return;

	RectangleStruct cacheRect { 0, 0, width, height };
	RectangleStruct screenRect { ownerRect.left, ownerRect.top, width, height };

	EnsureProgressCache(data, cacheRect, screenRect);

	if (data.CacheSurface)
		CopySurfacePart(DSurface::Alternate, screenRect, data.CacheSurface, cacheRect);

	const int rangeSpan = data.AsProgress().MaxValue() - data.AsProgress().MinValue();
	const int widthScale = rangeSpan
		? static_cast<int>((static_cast<long long>(data.AsProgress().Position()) << 16) / rangeSpan)
		: 0;

	const WORD color = static_cast<WORD>(ConvertRGBToSurfaceColor(RGB(255, 0, 0)));
	BlendGradientRect(screenRect, DSurface::Alternate, color, widthScale);
}

LRESULT CALLBACK WWUI::ProgressCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return 0;

	auto& data = *pData;

	switch (message)
	{
	case WW_PROGRESS_SETRANGE:
		data.AsProgress().MinValue() = LOWORD(lParam);
		data.AsProgress().MaxValue() = HIWORD(lParam);
		return 0;

	case WW_PROGRESS_SETPOS:
	{
		int position = static_cast<int>(wParam);
		if (position < data.AsProgress().MinValue())
			position = data.AsProgress().MinValue();

		if (position > data.AsProgress().MaxValue())
			position = data.AsProgress().MaxValue();

		data.AsProgress().Position() = position;
		::InvalidateRect(hWnd, nullptr, FALSE);
		return 0;
	}

	case WM_PAINT:
		PaintProgress(data, ownerRect);
		::ValidateRect(hWnd, nullptr);
		return 0;

	case WW_INITDIALOG:
		data.AsProgress().MaxValue() = 100;
		return 0;

	default:
		return 0;
	}
}
