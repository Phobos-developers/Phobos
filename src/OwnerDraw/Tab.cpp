#include "OwnerDraw.Internal.h"

static void DrawTabFrameTiledPCX(const char* pFilename, const RectangleStruct& rect)
{
	if (auto pPCX = GetPCXSurface(pFilename))
		BlitTiledPCX(rect, DSurface::Alternate, pPCX, 0, 0);
}

static void DrawTabFrameCornerPCX(const char* pFilename, int x, int y)
{
	auto pPCX = GetPCXSurface(pFilename);
	if (!pPCX)
		return;

	RectangleStruct destRect { x, y, pPCX->GetWidth(), pPCX->GetHeight() };
	RectangleStruct sourceRect { 0, 0, pPCX->GetWidth(), pPCX->GetHeight() };
	CopySurfacePart(DSurface::Alternate, destRect, pPCX, sourceRect);
}

static void DrawTabFrame(const RECT& panelRect)
{
	const int width = panelRect.right - panelRect.left;
	const int height = panelRect.bottom - panelRect.top;
	if (width <= 0 || height <= 0)
		return;

	if (auto pLeft = GetPCXSurface("tab_fml.pcx"))
	{
		RectangleStruct rect { panelRect.left, panelRect.top, pLeft->GetWidth(), height };
		DrawTabFrameTiledPCX("tab_fml.pcx", rect);
	}

	if (auto pRight = GetPCXSurface("tab_fmr.pcx"))
	{
		const int rightWidth = pRight->GetWidth();
		RectangleStruct rect { panelRect.right - rightWidth, panelRect.top, rightWidth, height };
		DrawTabFrameTiledPCX("tab_fmr.pcx", rect);
	}

	if (auto pTop = GetPCXSurface("tab_ftm.pcx"))
	{
		RectangleStruct rect { panelRect.left, panelRect.top, width, pTop->GetHeight() };
		DrawTabFrameTiledPCX("tab_ftm.pcx", rect);
	}

	if (auto pBottom = GetPCXSurface("tab_fbm.pcx"))
	{
		const int bottomHeight = pBottom->GetHeight();
		RectangleStruct rect { panelRect.left, panelRect.bottom - bottomHeight, width, bottomHeight };
		DrawTabFrameTiledPCX("tab_fbm.pcx", rect);
	}

	DrawTabFrameCornerPCX("tab_ftl.pcx", panelRect.left, panelRect.top);

	if (auto pTopRight = GetPCXSurface("tab_ftr.pcx"))
		DrawTabFrameCornerPCX("tab_ftr.pcx", panelRect.right - pTopRight->GetWidth(), panelRect.top);

	if (auto pBottomLeft = GetPCXSurface("tab_fbl.pcx"))
		DrawTabFrameCornerPCX("tab_fbl.pcx", panelRect.left, panelRect.bottom - pBottomLeft->GetHeight());

	if (auto pBottomRight = GetPCXSurface("tab_fbr.pcx"))
	{
		DrawTabFrameCornerPCX(
			"tab_fbr.pcx",
			panelRect.right - pBottomRight->GetWidth(),
			panelRect.bottom - pBottomRight->GetHeight());
	}
}

static void DrawTabTransparentPCX(const char* pFilename, const RectangleStruct& rect)
{
	auto pPCX = GetPCXSurface(pFilename);
	if (!pPCX)
		return;

	RectangleStruct blitRect { rect.X, rect.Y, pPCX->GetWidth(), pPCX->GetHeight() };
	PCX::Instance.BlitToSurface(
		&blitRect,
		DSurface::Alternate,
		pPCX,
		static_cast<WORD>(ConvertRGBToSurfaceColor(RGB(255, 0, 255))));
}

static void DrawTabItemMiddle(char variant, int drawLeft, int drawTop, int drawWidth)
{
	auto pLeftCapBase = GetPCXSurface("tab_tlu.pcx");
	if (!pLeftCapBase)
		return;

	char filename[32] {};
	std::snprintf(filename, sizeof(filename), "tab_tm%c.pcx", variant);

	auto pMiddle = GetPCXSurface(filename);
	if (!pMiddle)
		return;

	const int capWidth = pLeftCapBase->GetWidth();
	RectangleStruct rect
	{
		drawLeft + capWidth,
		drawTop,
		drawWidth - 2 * capWidth,
		pMiddle->GetHeight()
	};

	DrawTabFrameTiledPCX(filename, rect);
}

static void DrawTabItemCaps(char variant, int drawLeft, int drawTop, int drawWidth)
{
	char filename[32] {};
	std::snprintf(filename, sizeof(filename), "tab_tl%c.pcx", variant);
	DrawTabTransparentPCX(filename, { drawLeft, drawTop, 0, 0 });

	std::snprintf(filename, sizeof(filename), "tab_tr%c.pcx", variant);
	if (auto pRightCap = GetPCXSurface(filename))
	{
		DrawTabTransparentPCX(
			filename,
			{ drawLeft + drawWidth - pRightCap->GetWidth(), drawTop, 0, 0 });
	}
}

static void DrawTabItemText(
	HWND hWnd,
	OwnerDrawDialogElement& data,
	int tabIndex,
	bool selected,
	int drawLeft,
	int drawTop,
	int drawWidth,
	int drawHeight)
{
	char title[64] {};
	std::snprintf(title, sizeof(title), "Title");

	TCITEMA item {};
	item.mask = TCIF_TEXT;
	item.pszText = title;
	item.cchTextMax = static_cast<int>(std::size(title));
	::SendMessageA(hWnd, TCM_GETITEMA, tabIndex, reinterpret_cast<LPARAM>(&item));
	title[std::size(title) - 1] = '\0';

	wchar_t wideTitle[64] {};
	std::swprintf(wideTitle, std::size(wideTitle), L"%hs", item.pszText ? item.pszText : title);

	RECT textRect
	{
		drawLeft,
		drawTop + 6,
		drawLeft + drawWidth,
		drawTop + drawHeight
	};

	OwnerDraw::DrawWideText(
		DSurface::Alternate,
		wideTitle,
		&textRect,
		data.AsTab().Font(),
		selected ? Phobos::UI::ColorText : OwnerDraw::SelectedTabTextColor,
		5,
		12,
		0,
		0,
		0);
}

static void DrawTabItem(HWND hWnd, OwnerDrawDialogElement& data, const RECT& controlRect, int tabIndex, int selectedIndex, RECT& itemRect)
{
	const char variant = tabIndex == selectedIndex ? 'u' : 'd';
	const int leftOverlap = itemRect.left < 6 ? itemRect.left : 6;
	const int drawLeft = controlRect.left + itemRect.left - leftOverlap;
	const int drawTop = controlRect.top + itemRect.top;
	const int drawWidth = itemRect.right + leftOverlap - itemRect.left;
	const int drawHeight = itemRect.bottom - itemRect.top;

	DrawTabItemMiddle(variant, drawLeft, drawTop, drawWidth);
	DrawTabItemCaps(variant, drawLeft, drawTop, drawWidth);
	DrawTabItemText(hWnd, data, tabIndex, tabIndex == selectedIndex, drawLeft, drawTop, drawWidth, drawHeight);
	::ValidateRect(hWnd, &itemRect);
}

static void InitializeTabCtrl(HWND hWnd)
{
	RECT controlRect {};
	OwnerDraw::GetRectangle(hWnd, &controlRect);

	int tabHeight = 0;
	if (auto pLeftUp = GetPCXSurface("tab_tlu.pcx"))
		tabHeight = std::max(pLeftUp->GetHeight() - 1, 0);

	::SendMessageA(hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(0x59, tabHeight));
}

static LRESULT PaintTabCtrl(HWND hWnd, OwnerDrawDialogElement& data)
{
	RECT controlRect {};
	OwnerDraw::GetRectangle(hWnd, &controlRect);

	const int itemCount = static_cast<int>(::SendMessageA(hWnd, TCM_GETITEMCOUNT, 0, 0));

	RECT firstItemRect {};
	::SendMessageA(hWnd, TCM_GETITEMRECT, 0, reinterpret_cast<LPARAM>(&firstItemRect));
	::SendMessageA(hWnd, TCM_GETITEMRECT, 0, reinterpret_cast<LPARAM>(&firstItemRect));

	RECT panelRect = controlRect;
	panelRect.top += firstItemRect.bottom - firstItemRect.top + 3;

	RectangleStruct dimRect
	{
		panelRect.left,
		panelRect.top,
		panelRect.right - panelRect.left,
		panelRect.bottom - panelRect.top
	};
	OwnerDraw::CopyDimmedBackground(&dimRect, hWnd, 0);
	::ValidateRect(hWnd, nullptr);

	DrawTabFrame(panelRect);

	if (itemCount <= 0)
	{
		::ValidateRect(hWnd, nullptr);
		return 0;
	}

	int selectedIndex = static_cast<int>(::SendMessageA(hWnd, TCM_GETCURSEL, 0, 0));
	if (selectedIndex < 0 || selectedIndex >= itemCount)
		selectedIndex = 0;

	OwnerDraw::GetRectangle(hWnd, &controlRect);

	int tabIndex = selectedIndex + 1;
	if (tabIndex >= itemCount)
		tabIndex = 0;

	for (int drawnCount = 0; drawnCount < itemCount; ++drawnCount)
	{
		RECT itemRect {};
		if (!::SendMessageA(hWnd, TCM_GETITEMRECT, tabIndex, reinterpret_cast<LPARAM>(&itemRect)))
		{
			tabIndex = 0;
			if (!::SendMessageA(hWnd, TCM_GETITEMRECT, tabIndex, reinterpret_cast<LPARAM>(&itemRect)))
				break;
		}

		DrawTabItem(hWnd, data, controlRect, tabIndex, selectedIndex, itemRect);

		if (tabIndex == selectedIndex)
			break;

		++tabIndex;
		if (tabIndex >= itemCount)
			tabIndex = 0;
	}

	::ValidateRect(hWnd, nullptr);
	return 0;
}

LRESULT CALLBACK WWUI::TabCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	switch (message)
	{
	case WM_ERASEBKGND:
		return 1;

	case WM_NCPAINT:
		return 0;

	case WW_INITDIALOG:
		InitializeTabCtrl(hWnd);
		return forwardOriginal();

	case WM_PAINT:
		if (auto pData = FindOwnerDrawData(hWnd))
			return PaintTabCtrl(hWnd, *pData);

		::ValidateRect(hWnd, nullptr);
		return 0;

	default:
		return forwardOriginal();
	}
}
