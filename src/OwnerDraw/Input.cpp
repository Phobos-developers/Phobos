#include "OwnerDraw.Internal.h"

constexpr WORD InputHotkeyShift = HOTKEYF_SHIFT << 8;
constexpr WORD InputHotkeyControl = HOTKEYF_CONTROL << 8;
constexpr WORD InputHotkeyAlt = HOTKEYF_ALT << 8;
constexpr WORD InputHotkeyNoText = HOTKEYF_EXT << 8;

static void AppendCString(char* pBuffer, size_t bufferSize, const char* pText)
{
	if (!pBuffer || !bufferSize || !pText)
		return;

	const size_t used = std::strlen(pBuffer);
	if (used >= bufferSize - 1)
		return;

	std::strncat(pBuffer, pText, bufferSize - used - 1);
}

static void AppendKeyName(char* pBuffer, size_t bufferSize, UINT virtualKey, bool appendSeparator)
{
	char keyName[32] {};
	const UINT scanCode = ::MapVirtualKeyA(virtualKey, MAPVK_VK_TO_VSC);
	::GetKeyNameTextA(static_cast<LONG>((scanCode << 16) | 0x02000001), keyName, static_cast<int>(std::size(keyName)));

	AppendCString(pBuffer, bufferSize, keyName);
	if (appendSeparator)
		AppendCString(pBuffer, bufferSize, "+");
}

static void BuildKeyboardKeyString(WORD keyCode, wchar_t* pOutText, size_t outTextSize)
{
	if (!pOutText || !outTextSize)
		return;

	pOutText[0] = L'\0';
	if (keyCode & InputHotkeyNoText)
		return;

	char keyText[500] {};
	if (keyCode & InputHotkeyAlt)
		AppendKeyName(keyText, std::size(keyText), VK_MENU, true);

	if (keyCode & InputHotkeyControl)
		AppendKeyName(keyText, std::size(keyText), VK_CONTROL, true);

	if (keyCode & InputHotkeyShift)
		AppendKeyName(keyText, std::size(keyText), VK_SHIFT, true);

	AppendKeyName(keyText, std::size(keyText), LOBYTE(keyCode), false);
	std::swprintf(pOutText, outTextSize, L"%hs", keyText);
	pOutText[outTextSize - 1] = L'\0';
}

static int DrawWideTextBasic(Surface* pSurface, const wchar_t* pText, const RECT& textRect, BitFont* pFont, COLORREF color)
{
	if (!pSurface || !pText)
		return 0;

	auto pDrawFont = pFont ? pFont : BitFont::Instance;
	if (!pDrawFont || !BitText::Instance)
		return 0;

	LTRBStruct bounds { textRect.left, textRect.top, textRect.right, textRect.bottom };
	pDrawFont->SetClipMode(true);
	pDrawFont->SetRectangle(&bounds);
	pDrawFont->SetColor(static_cast<WORD>(ConvertRGBToSurfaceColor(color)));

	BitText::Instance->DrawText(
		pDrawFont,
		pSurface,
		pText,
		textRect.left,
		textRect.top,
		textRect.right - textRect.left,
		textRect.bottom - textRect.top,
		0,
		0,
		0);

	return 0;
}

static void EnsureInputCache(OwnerDrawDialogElement& data, const RectangleStruct& screenRect)
{
	if (data.CacheSurface || !DSurface::Alternate || screenRect.Width <= 0 || screenRect.Height <= 0)
		return;

	data.CacheSurface = GameCreate<BSurface>(screenRect.Width, screenRect.Height);
	if (!data.CacheSurface)
		return;

	++OwnerDraw::CachedSurfaceCount;

	RectangleStruct cacheRect { 0, 0, screenRect.Width, screenRect.Height };
	CopySurfacePart(data.CacheSurface, cacheRect, DSurface::Alternate, screenRect);
}

static LRESULT PaintInputCtrl(HWND hWnd, OwnerDrawDialogElement& data)
{
	if (!DSurface::Alternate)
	{
		::ValidateRect(hWnd, nullptr);
		return 0;
	}

	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	RectangleStruct screenRect
	{
		ownerRect.left,
		ownerRect.top,
		ownerRect.right - ownerRect.left + 1,
		ownerRect.bottom - ownerRect.top + 1
	};

	EnsureInputCache(data, screenRect);

	wchar_t keyText[256] {};
	BuildKeyboardKeyString(static_cast<WORD>(::SendMessageA(hWnd, WW_INPUT_GETKEY, 0, 0)), keyText, std::size(keyText));

	if (data.CacheSurface)
	{
		RectangleStruct cacheRect { 0, 0, screenRect.Width, screenRect.Height };
		CopySurfacePart(DSurface::Alternate, screenRect, data.CacheSurface, cacheRect);
	}

	DrawBeveledBorder(DSurface::Alternate, screenRect, 2, -1);

	if (std::wcslen(keyText))
	{
		RECT textRect
		{
			ownerRect.left + 4,
			ownerRect.top + 4,
			ownerRect.right - 4,
			ownerRect.bottom - 4
		};

		DrawWideTextBasic(DSurface::Alternate, keyText, textRect, data.AsInput().Font(), OwnerDraw::PrimaryTextColor);
	}

	::ValidateRect(hWnd, nullptr);
	return 0;
}

LRESULT CALLBACK WWUI::InputCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	switch (message)
	{
	case WM_PAINT:
		if (auto pData = FindOwnerDrawData(hWnd))
			return PaintInputCtrl(hWnd, *pData);

		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_NCPAINT:
		return 0;

	default:
		return forwardOriginal();
	}
}

LRESULT CALLBACK WWUI::SysListViewCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);

	RECT ownerRect {};
	(void)OwnerDraw::GetRectangle(hWnd, &ownerRect);

	RECT clientRect {};
	(void)::GetClientRect(hWnd, &clientRect);

	if (message == WM_CTLCOLOREDIT)
	{
		const auto hdc = reinterpret_cast<HDC>(wParam);
		::SetTextColor(hdc, OwnerDraw::PrimaryTextColor);
		::SetBkMode(hdc, TRANSPARENT);
		return reinterpret_cast<LRESULT>(::GetStockObject(NULL_BRUSH));
	}

	return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
}
