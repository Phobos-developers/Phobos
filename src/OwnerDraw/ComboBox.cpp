#include "OwnerDraw.Internal.h"

static COLORREF ComboBoxTextColor(bool disabled, bool alternatePalette)
{
	if (alternatePalette)
		return disabled ? OwnerDraw::AltDisabledTextColor : OwnerDraw::AltComboTextColor;

	return disabled ? Phobos::UI::ColorDisabledCombobox : Phobos::UI::ColorTextCombobox;
}

static void SyncComboDropSelectionColor()
{
	OwnerDraw::ListSelectionFillColor = Phobos::UI::ColorSelectionCombobox;
}

constexpr int ComboBoxArrowWidth = 20;
constexpr int ComboBoxArrowLeftOffset = 19;
constexpr int ComboBoxVisibleHeight = 24;
constexpr int ComboBoxDefaultMaxVisibleDropItems = 9;
constexpr int ComboBoxMaxColorItems = 50;
constexpr int ComboBoxTextEntryInlineBytes = 0;
constexpr int ComboBoxEditListNotificationCode = 0x300;
constexpr int ComboBoxParentEditChangeNotificationCode = 5;

static bool IsComboBoxDropDownList(HWND hWnd)
{
	return (::GetWindowLongA(hWnd, GWL_STYLE) & 3) == CBS_DROPDOWNLIST;
}

static bool IsComboBoxDropDown(HWND hWnd)
{
	return (::GetWindowLongA(hWnd, GWL_STYLE) & 3) == CBS_DROPDOWN;
}

int BitFontHeight(BitFont* pFont)
{
	if (!pFont)
		pFont = BitFont::Instance;

	if (!pFont)
		return 10;

	return pFont->field_1C;
}

static void TrimComboTextToWidth(wchar_t* pText, size_t capacity, BitFont* pFont, int maxWidth)
{
	if (!pText || !capacity || maxWidth <= 0)
		return;

	pText[capacity - 1] = L'\0';
	size_t length = std::wcslen(pText);
	if (!length)
		return;

	int textWidth = 0;
	int textHeight = 0;
	if (!pFont)
		pFont = BitFont::Instance;

	while (length > 0 && pFont)
	{
		pFont->GetTextDimension(pText, &textWidth, &textHeight, 0);
		if (textWidth <= maxWidth)
			break;

		--length;
		pText[length] = L'\0';
		if (length + 3 < capacity)
			std::wcscat(pText, L"...");
	}
}

static WWUIComboBoxItem* AllocateComboBoxItem(OwnerDrawDialogElement& data, const wchar_t* pText, bool isWide)
{
	if (!pText)
		pText = L"";

	const size_t length = std::wcslen(pText);
	const size_t bytes = sizeof(WWUIComboBoxItem) + (length + 1) * sizeof(wchar_t) + ComboBoxTextEntryInlineBytes;
	auto pEntry = static_cast<WWUIComboBoxItem*>(YRMemory::Allocate(bytes));
	if (!pEntry)
		return nullptr;

	pEntry->Next = data.AsComboBox().TextEntries();
	pEntry->ItemData = 0;
	pEntry->Text = reinterpret_cast<wchar_t*>(reinterpret_cast<char*>(pEntry) + sizeof(WWUIComboBoxItem));
	pEntry->IsWideText = isWide ? 1 : 0;
	std::wcscpy(pEntry->Text, pText);
	data.AsComboBox().TextEntries() = pEntry;
	return pEntry;
}

static void RemoveComboBoxItem(OwnerDrawDialogElement& data, WWUIComboBoxItem* pEntry)
{
	if (!pEntry)
		return;

	WWUIComboBoxItem* pPrevious = nullptr;
	for (auto pCurrent = data.AsComboBox().TextEntries(); pCurrent; pCurrent = pCurrent->Next)
	{
		if (pCurrent != pEntry)
		{
			pPrevious = pCurrent;
			continue;
		}

		if (pPrevious)
			pPrevious->Next = pCurrent->Next;
		else
			data.AsComboBox().TextEntries() = pCurrent->Next;

		YRMemory::Deallocate(pCurrent);
		return;
	}
}

static WWUIComboBoxItem* GetComboBoxItem(WNDPROC pOriginalWndProc, HWND hWnd, int index)
{
	const auto result = CallSelectedHandler(pOriginalWndProc, hWnd, CB_GETITEMDATA, index, 0);
	if (result == CB_ERR || !result)
		return nullptr;

	return reinterpret_cast<WWUIComboBoxItem*>(result);
}

static LRESULT ForwardComboTextMessageToEditList(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (IsComboBoxDropDownList(hWnd))
		return CallSelectedHandler(FindWindowProc(OwnerDraw::DialogProcs, hWnd), hWnd, message, wParam, lParam);

	LRESULT result = 0;
	for (HWND child = ::GetWindow(hWnd, GW_CHILD); child; child = ::GetWindow(child, GW_HWNDNEXT))
	{
		char className[16] {};
		::GetClassNameA(child, className, sizeof(className));
		if (_strcmpi(className, "listbox"))
			continue;

		if (message == WM_SETFOCUS)
		{
			::SetFocus(child);
			result = 0;
		}
		else
		{
			const UINT forwardedMessage = message == CB_LIMITTEXT ? EM_LIMITTEXT : message;
			result = ::SendMessageA(child, forwardedMessage, wParam, lParam);
		}
	}

	return result;
}

static int ResolveComboBorderColor(COLORREF color, bool disabledColor)
{
	if (color == static_cast<COLORREF>(-1))
		return disabledColor ? 0 : -1;

	return ConvertRGBToSurfaceColor(color);
}

static void DrawComboDropButton(const RectangleStruct& rect, bool dropped, bool alternatePalette)
{
	DrawScrollArrow(DSurface::Alternate, rect, dropped, dropped, alternatePalette);
}

static bool GetComboBoxRenderLocalRect(HWND hWnd, const RECT& ownerRect, RECT& localRect)
{
	localRect = ownerRect;

	const HWND parentHwnd = ::GetParent(hWnd);
	if (!parentHwnd || parentHwnd == Game::hWnd)
		return true;

	RECT parentRect {};
	if (!OwnerDraw::GetRectangle(parentHwnd, &parentRect))
		return false;

	localRect.left -= parentRect.left;
	localRect.right -= parentRect.left;
	localRect.top -= parentRect.top;
	localRect.bottom -= parentRect.top;
	return true;
}

static void EnsureComboBoxWindowHeight(HWND hWnd, OwnerDrawDialogElement& data, RECT& clientRect, RECT& ownerRect)
{
	const int width = ownerRect.right - ownerRect.left;
	if (width <= 0 || ownerRect.bottom - ownerRect.top == ComboBoxVisibleHeight)
		return;

	RECT localRect {};
	if (!GetComboBoxRenderLocalRect(hWnd, ownerRect, localRect))
		return;

	const BOOL moved = RenderDX::IsOwnerDrawUsingRawWindowCoordinates()
		? ::MoveWindow(hWnd, localRect.left, localRect.top, width, ComboBoxVisibleHeight, FALSE)
		: RenderDX::MoveWindowInRender(hWnd, localRect.left, localRect.top, width, ComboBoxVisibleHeight, FALSE);

	if (!moved)
		return;

	ResetOwnerDrawCachedSurface(data);
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates() || !RenderDX::GetClientRectInRender(hWnd, &clientRect))
		::GetClientRect(hWnd, &clientRect);
}

static void PaintComboBox(HWND hWnd, OwnerDrawDialogElement& data, const RECT& ownerRect, WNDPROC pOriginalWndProc)
{
	if (!DSurface::Alternate)
		return;

	auto pFont = data.AsComboBox().Font() ? data.AsComboBox().Font() : BitFont::Instance;
	const bool dropped = ::SendMessageA(hWnd, CB_GETDROPPEDSTATE, 0, 0) != 0;
	const int width = ownerRect.right - ownerRect.left;
	const int height = ownerRect.bottom - ownerRect.top;

	RectangleStruct comboRect
	{
		ownerRect.left,
		ownerRect.top,
		width,
		ComboBoxVisibleHeight
	};

	RectangleStruct localRect { 0, 0, width, height };
	RectangleStruct parentSourceRect = localRect;
	OwnerDrawDialogElement* pParentData = nullptr;
	if (const HWND parentHwnd = ::GetParent(hWnd))
	{
		pParentData = FindOwnerDrawData(parentHwnd);
		if (pParentData)
		{
			RECT parentRect {};
			OwnerDraw::GetRectangle(parentHwnd, &parentRect);

			if (pParentData->CacheSurface)
			{
				parentSourceRect.X = ownerRect.left - parentRect.left;
				parentSourceRect.Y = ownerRect.top - parentRect.top;
			}
		}
	}
	EnsureScrollBarCache(data, pParentData, localRect, parentSourceRect);

	OwnerDraw::CopyDimmedBackground(&comboRect, hWnd, static_cast<unsigned char>(data.Alpha));
	BlendFillRect(comboRect, DSurface::Alternate, 0, static_cast<unsigned char>(data.Alpha));

	const LONG style = ::GetWindowLongA(hWnd, GWL_STYLE);
	const bool disabled = (style & WS_DISABLED) != 0;
	const bool alternatePalette = data.AsComboBox().UseAlternatePalette();
	const COLORREF borderColor = alternatePalette
		? (disabled ? OwnerDraw::AltDisabledBorderColor : OwnerDraw::AltBorderColor)
		: (disabled ? OwnerDraw::DisabledBorderColor : OwnerDraw::DefaultBorderColor);

	DrawBeveledBorder(DSurface::Alternate, comboRect, 2, ResolveComboBorderColor(borderColor, disabled));

	RectangleStruct textAreaRect { comboRect.X, comboRect.Y, comboRect.Width - ComboBoxArrowWidth, comboRect.Height };
	RectangleStruct buttonRect { ownerRect.right - ComboBoxArrowLeftOffset, comboRect.Y + 1, comboRect.Width, comboRect.Height };
	DrawComboDropButton(buttonRect, dropped, alternatePalette);

	if (disabled)
		BlendFillRect(comboRect, DSurface::Alternate, 0, static_cast<unsigned char>(data.Alpha));

	if ((style & 3) != CBS_DROPDOWNLIST)
	{
		::ValidateRect(hWnd, nullptr);
		return;
	}

	const int selectedIndex = static_cast<int>(CallSelectedHandler(pOriginalWndProc, hWnd, CB_GETCURSEL, 0, 0));
	wchar_t textBuffer[0x100] {};
	if (auto pItem = GetComboBoxItem(pOriginalWndProc, hWnd, selectedIndex))
	{
		std::wcsncpy(textBuffer, pItem->Text ? pItem->Text : L"", std::size(textBuffer) - 1);
	}

	COLORREF textColor = ComboBoxTextColor(disabled, alternatePalette);
	if (data.AsComboBox().UseItemColorOverrides()
		&& selectedIndex >= 0
		&& selectedIndex < ComboBoxMaxColorItems
		&& data.AsComboBox().ItemColorOverrides()[selectedIndex] >= 0)
	{
		textColor = static_cast<COLORREF>(data.AsComboBox().ItemColorOverrides()[selectedIndex]);
		auto fillRect = textAreaRect;
		InsetSurfaceRect(fillRect, 2, 2);
		DSurface::Alternate->FillRect(&fillRect, ConvertRGBToSurfaceColor(textColor));
	}

	const int textMaxWidth = std::max(textAreaRect.Width - 4, 0);
	TrimComboTextToWidth(textBuffer, std::size(textBuffer), pFont, textMaxWidth);

	RECT textRect
	{
		textAreaRect.X + 2,
		textAreaRect.Y,
		textAreaRect.X + textAreaRect.Width,
		textAreaRect.Y + textAreaRect.Height
	};

	OwnerDraw::DrawWideText(DSurface::Alternate, textBuffer, &textRect, pFont, textColor, 4, 12, 0, 0, 0);
	::ValidateRect(hWnd, nullptr);
}

static LRESULT AddOrInsertComboString(
	OwnerDrawDialogElement& data,
	WNDPROC pOriginalWndProc,
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam,
	bool wideText)
{
	char narrowText[2048] {};
	wchar_t wideBuffer[2048] {};

	const LPARAM nativeTextParam = [&]() -> LPARAM
	{
		if (wideText)
		{
			const auto pWideText = reinterpret_cast<const wchar_t*>(lParam);
			std::wcsncpy(wideBuffer, pWideText ? pWideText : L"", std::size(wideBuffer) - 1);
			WideToCharString(narrowText, std::size(narrowText), wideBuffer);
			return reinterpret_cast<LPARAM>(narrowText);
		}

		const auto pText = reinterpret_cast<const char*>(lParam);
		std::strncpy(narrowText, pText ? pText : "", std::size(narrowText) - 1);
		CharToWideString(wideBuffer, std::size(wideBuffer), narrowText);
		return reinterpret_cast<LPARAM>(narrowText);
	}();

	const bool add = message == WW_CB_ADDSTRINGA || message == WW_CB_ADDSTRINGW;
	const UINT nativeMessage = add ? CB_ADDSTRING : CB_INSERTSTRING;
	const WPARAM nativeIndex = add ? 0 : wParam;
	const auto nativeResult = CallSelectedHandler(pOriginalWndProc, hWnd, nativeMessage, nativeIndex, nativeTextParam);
	if (nativeResult == CB_ERR || nativeResult == CB_ERRSPACE)
		return nativeResult;

	const int itemIndex = static_cast<int>(nativeResult);
	auto pEntry = AllocateComboBoxItem(data, wideBuffer, wideText);
	if (!pEntry)
	{
		CallSelectedHandler(pOriginalWndProc, hWnd, CB_DELETESTRING, itemIndex, 0);
		return CB_ERRSPACE;
	}

	const auto setDataResult = CallSelectedHandler(
		pOriginalWndProc,
		hWnd,
		CB_SETITEMDATA,
		itemIndex,
		reinterpret_cast<LPARAM>(pEntry));

	if (setDataResult == CB_ERR || setDataResult == CB_ERRSPACE)
	{
		CallSelectedHandler(pOriginalWndProc, hWnd, CB_DELETESTRING, itemIndex, 0);
		RemoveComboBoxItem(data, pEntry);
		return setDataResult;
	}

	return itemIndex;
}

static LRESULT FindComboString(OwnerDrawDialogElement& data, WNDPROC pOriginalWndProc, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool wideText, bool exact, bool select)
{
	(void)data;
	(void)message;

	wchar_t needle[2048] {};
	if (wideText)
	{
		const auto pText = reinterpret_cast<const wchar_t*>(lParam);
		std::wcsncpy(needle, pText ? pText : L"", std::size(needle) - 1);
	}
	else
	{
		CharToWideString(needle, std::size(needle), reinterpret_cast<const char*>(lParam));
	}

	const int count = static_cast<int>(CallSelectedHandler(pOriginalWndProc, hWnd, CB_GETCOUNT, 0, 0));
	if (count == CB_ERR)
		return 0;

	int index = static_cast<int>(wParam);
	if (index < 0)
		index = 0;

	if (index >= count)
		return CB_ERR;

	const size_t needleLength = std::wcslen(needle);
	for (; index < count; ++index)
	{
		const auto pEntry = GetComboBoxItem(pOriginalWndProc, hWnd, index);
		const wchar_t* pText = pEntry && pEntry->Text ? pEntry->Text : L"";
		const bool match = exact
			? _wcsicmp(needle, pText) == 0
			: _wcsnicmp(needle, pText, needleLength) == 0;

		if (!match)
			continue;

		if (select)
			return ::SendMessageA(hWnd, CB_SETCURSEL, index, 0);

		return index;
	}

	return CB_ERR;
}

static LRESULT GetComboText(WNDPROC pOriginalWndProc, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool wideOutput)
{
	if (lParam && message != WW_CB_GETITEMTEXTFORMAT && message != CB_GETLBTEXTLEN)
	{
		if (wideOutput)
			*reinterpret_cast<wchar_t*>(lParam) = L'\0';
		else
			*reinterpret_cast<char*>(lParam) = '\0';
	}

	if (static_cast<int>(wParam) == -1)
		return 0;

	auto pEntry = GetComboBoxItem(pOriginalWndProc, hWnd, static_cast<int>(wParam));
	if (!pEntry)
		return 0;

	if (message == WW_CB_GETITEMTEXTFORMAT)
		return pEntry->IsWideText;

	const wchar_t* pText = pEntry->Text ? pEntry->Text : L"";
	const auto length = static_cast<LRESULT>(std::wcslen(pText));

	if (message == WW_CB_GETLBTEXTA || message == WW_CB_GETLBTEXTW)
	{
		if (lParam)
		{
			if (wideOutput)
				std::wcscpy(reinterpret_cast<wchar_t*>(lParam), pText);
			else
				WideToCharString(reinterpret_cast<char*>(lParam), static_cast<int>(length + 1), pText);
		}
	}

	return length < 0 ? 0 : length;
}

static LRESULT SetComboSelection(OwnerDrawDialogElement& data, WNDPROC pOriginalWndProc, HWND hWnd, WPARAM wParam)
{
	const int selection = static_cast<int>(wParam);
	data.AsComboBox().CurrentSelection() = selection;

	if (selection == -1)
	{
		::SendMessageA(hWnd, WW_SETTEXTA, 0, reinterpret_cast<LPARAM>(""));
	}
	else if (auto pEntry = GetComboBoxItem(pOriginalWndProc, hWnd, selection))
	{
		if (pEntry->IsWideText)
		{
			::SendMessageA(hWnd, WW_SETTEXTW, 0, reinterpret_cast<LPARAM>(pEntry->Text ? pEntry->Text : L""));
		}
		else
		{
			char buffer[2048] {};
			WideToCharString(buffer, std::size(buffer), pEntry->Text ? pEntry->Text : L"");
			::SendMessageA(hWnd, WW_SETTEXTA, 0, reinterpret_cast<LPARAM>(buffer));
		}
	}

	if (IsComboBoxDropDown(hWnd))
		return 0;

	::InvalidateRect(hWnd, nullptr, FALSE);
	return CallSelectedHandler(pOriginalWndProc, hWnd, CB_SETCURSEL, wParam, 0);
}

static void CloseComboDropDown(OwnerDrawDialogElement& data, HWND hWnd)
{
	const HWND dropHwnd = data.AsComboBox().DropDownHwnd();
	if (!dropHwnd)
		return;

	::ReleaseCapture();
	if (const HWND parentHwnd = ::GetParent(hWnd))
		::SendMessageA(parentHwnd, WW_BRINGTOTOP, reinterpret_cast<WPARAM>(dropHwnd), 0);

	::DestroyWindow(dropHwnd);
	CleanupDestroyedWindow(dropHwnd);
	data.AsComboBox().DropDownHwnd() = nullptr;
}

static bool GetDroppedControlRectInRender(HWND hWnd, RECT& rect)
{
	::SendMessageA(hWnd, CB_GETDROPPEDCONTROLRECT, 0, reinterpret_cast<LPARAM>(&rect));

	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
		return true;

	POINT topLeft { rect.left, rect.top };
	POINT bottomRight { rect.right, rect.bottom };
	if (!RenderDX::ScreenToRenderPoint(&topLeft, false) || !RenderDX::ScreenToRenderPoint(&bottomRight, false))
		return false;

	rect.left = topLeft.x;
	rect.top = topLeft.y;
	rect.right = bottomRight.x;
	rect.bottom = bottomRight.y;
	return true;
}

static LRESULT OpenComboDropDown(OwnerDrawDialogElement& data, HWND hWnd, const RECT& clientRect, const RECT& ownerRect)
{
	if (data.AsComboBox().DropDownHwnd())
		return 1;

	SyncComboDropSelectionColor();

	const HWND parentHwnd = ::GetParent(hWnd);
	if (!parentHwnd)
		return 1;

	RECT parentRect {};
	OwnerDraw::GetRectangle(parentHwnd, &parentRect);

	int itemHeight = static_cast<int>(::SendMessageA(hWnd, CB_GETITEMHEIGHT, 0, 0));
	if (itemHeight <= 0)
		itemHeight = 1;

	int itemCount = static_cast<int>(::SendMessageA(hWnd, CB_GETCOUNT, 0, 0));
	if (itemCount < 1)
		itemCount = 1;

	const int visibleComboHeight = ComboBoxVisibleHeight;
	const int maxVisibleItems = data.AsComboBox().MaxVisibleDropItems();
	int visibleItems = 0;
	if (maxVisibleItems > 0)
	{
		visibleItems = maxVisibleItems;
	}
	else
	{
		RECT dropRect {};
		if (GetDroppedControlRectInRender(hWnd, dropRect))
		{
			int nativeListHeight = dropRect.bottom - dropRect.top;
			if (dropRect.top <= ownerRect.top + visibleComboHeight / 2)
				nativeListHeight -= visibleComboHeight;

			if (nativeListHeight > 0)
				visibleItems = nativeListHeight / itemHeight;
		}

		if (visibleItems <= 1 && itemCount > 1)
			visibleItems = ComboBoxDefaultMaxVisibleDropItems;
	}

	if (visibleItems < 1)
		visibleItems = 1;

	visibleItems = std::min(visibleItems, itemCount);

	const int dropTop = ownerRect.top + visibleComboHeight + 1;
	int maxVisibleByParent = (parentRect.bottom - dropTop) / itemHeight;
	if (maxVisibleByParent < 1)
		maxVisibleByParent = 1;

	visibleItems = std::min(visibleItems, maxVisibleByParent);

	int dropHeight = visibleItems * itemHeight;
	if (dropHeight <= 0)
		dropHeight = itemHeight;

	const int width = clientRect.right - clientRect.left;
	const RECT dropRenderRect
	{
		ownerRect.left,
		dropTop,
		ownerRect.left + width,
		dropTop + dropHeight
	};

	RECT dropClientRect {};
	if (!RenderDX::RenderRectToClient(parentHwnd, dropRenderRect, &dropClientRect))
		return 1;

	const HWND dropHwnd = ::CreateWindowExA(
		0,
		"ComboDropWin",
		nullptr,
		WS_CHILD,
		dropClientRect.left,
		dropClientRect.top,
		dropClientRect.right - dropClientRect.left,
		dropClientRect.bottom - dropClientRect.top,
		parentHwnd,
		nullptr,
		Game::hInstance,
		hWnd);

	if (!dropHwnd)
		return 1;

	if (!FindOwnerDrawData(dropHwnd))
	{
		OwnerDrawDialogElement dropData;
		dropData.AsComboBox().Font() = BitFont::Instance;
		dropData.ControlType = WWControlType::Default;

		OwnerDraw::Dialogs[dropHwnd] = dropData;
	}

	SessionIpb::RegisterHwnd(dropHwnd);
	SessionIpb::RegisterHwnd(dropHwnd);

	::SendMessageA(dropHwnd, WW_DROPDOWN_INITIALIZE, 0, 0);
	::SendMessageA(parentHwnd, WW_BRINGTOTOP, reinterpret_cast<WPARAM>(dropHwnd), 1);
	::SetCapture(dropHwnd);
	::ShowWindow(dropHwnd, SW_SHOWNORMAL);
	data.AsComboBox().DropDownHwnd() = dropHwnd;
	return 1;
}

LRESULT CALLBACK WWUI::ComboBoxCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto pData = FindOwnerDrawData(hWnd);
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
	if (!pData)
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);

	auto& data = *pData;

	RECT clientRect {};
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates() || !RenderDX::GetClientRectInRender(hWnd, &clientRect))
		::GetClientRect(hWnd, &clientRect);

	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	auto handleItemData = [&]() -> LRESULT
	{
		auto pEntry = GetComboBoxItem(pOriginalWndProc, hWnd, static_cast<int>(wParam));
		if (!pEntry)
			return CB_ERR;

		if (message == CB_GETITEMDATA || message == WW_GETITEMDATA)
			return pEntry->ItemData;

		pEntry->ItemData = static_cast<int>(lParam);
		return reinterpret_cast<LRESULT>(pEntry);
	};

	switch (message)
	{
	case WM_DESTROY:
		::SendMessageA(hWnd, CB_SHOWDROPDOWN, 0, 0);
		return forwardOriginal();

	case WM_PAINT:
		EnsureComboBoxWindowHeight(hWnd, data, clientRect, ownerRect);
		PaintComboBox(hWnd, data, ownerRect, pOriginalWndProc);
		return 0;

	case WM_ERASEBKGND:
		return 0;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (RulesClass::Instance)
			VocClass::PlayGlobal(RulesClass::Instance->GUIComboOpenSound, 0x2000, 1.0f);

		if (RenderDX::MouseLParamToRenderLocalPoint(hWnd, lParam).x > clientRect.right - ComboBoxArrowWidth)
		{
			const bool dropped = ::SendMessageA(hWnd, CB_GETDROPPEDSTATE, 0, 0) == 1;
			::PostMessageA(hWnd, CB_SHOWDROPDOWN, dropped ? 0 : 1, 0);
		}
		return 0;

	case WM_SETFOCUS:
	case WM_SETTEXT:
	case WM_GETTEXT:
	case WM_GETTEXTLENGTH:
	case CB_LIMITTEXT:
		return ForwardComboTextMessageToEditList(hWnd, message, wParam, lParam);

	case WM_DELETEITEM:
		if (const auto pDeleteItem = reinterpret_cast<DELETEITEMSTRUCT*>(lParam))
		{
			RemoveComboBoxItem(data, reinterpret_cast<WWUIComboBoxItem*>(pDeleteItem->itemData));
			if (data.AsComboBox().CurrentSelection() == static_cast<int>(pDeleteItem->itemID))
				data.AsComboBox().CurrentSelection() = -1;
		}
		return forwardOriginal();

	case WM_COMMAND:
		if (HIWORD(wParam) == ComboBoxEditListNotificationCode && !IsComboBoxDropDownList(hWnd))
		{
			if (const HWND parentHwnd = ::GetParent(hWnd))
			{
				const WPARAM command = static_cast<WPARAM>(
					(::GetDlgCtrlID(hWnd) & 0xFFFF)
					| (ComboBoxParentEditChangeNotificationCode << 16));
				::SendMessageA(parentHwnd, WM_COMMAND, command, reinterpret_cast<LPARAM>(hWnd));
			}
			return 0;
		}
		break;

	case CB_GETCURSEL:
		return data.AsComboBox().CurrentSelection();

	case CB_GETLBTEXTLEN:
		return GetComboText(pOriginalWndProc, hWnd, message, wParam, lParam, true);

	case CB_SETCURSEL:
		return SetComboSelection(data, pOriginalWndProc, hWnd, wParam);

	case CB_SHOWDROPDOWN:
		EnsureComboBoxWindowHeight(hWnd, data, clientRect, ownerRect);
		if (wParam)
			return OpenComboDropDown(data, hWnd, clientRect, ownerRect);

		CloseComboDropDown(data, hWnd);
		return 1;

	case CB_GETITEMDATA:
	case CB_SETITEMDATA:
	case WW_GETITEMDATA:
	case WW_SETITEMDATA:
		return handleItemData();

	case WW_INITDIALOG:
	{
		const int fontHeight = BitFontHeight(data.AsComboBox().Font());
		const int selectionHeight = static_cast<int>(::SendMessageA(hWnd, CB_GETITEMHEIGHT, static_cast<WPARAM>(-1), 0));
		if (!data.AsComboBox().HeightInitialized()
			|| selectionHeight != ComboBoxVisibleHeight
			|| ::SendMessageA(hWnd, CB_GETITEMHEIGHT, 0, 0) != fontHeight + 6)
		{
			::SendMessageA(hWnd, CB_SETITEMHEIGHT, static_cast<WPARAM>(-1), ComboBoxVisibleHeight);
			::SendMessageA(hWnd, CB_SETITEMHEIGHT, 0, fontHeight + 6);
			data.AsComboBox().HeightInitialized() = 1;
		}

		EnsureComboBoxWindowHeight(hWnd, data, clientRect, ownerRect);
		data.AsComboBox().CurrentSelection() = -1;
		std::memset(data.AsComboBox().ItemColorOverrides(), 0xFF, sizeof(int) * ComboBoxMaxColorItems);
		return 0;
	}

	case WW_SETCOLOR:
		if (wParam <= ComboBoxMaxColorItems)
			data.AsComboBox().ItemColorOverrides()[wParam] = static_cast<int>(lParam);
		return forwardOriginal();

	case WW_SETTEXTW:
	case WW_GETTEXTW:
	case WW_SETTEXTA:
	case WW_GETTEXTA:
		return ForwardComboTextMessageToEditList(hWnd, message, wParam, lParam);

	case WW_CB_FINDSTRINGA:
		return FindComboString(data, pOriginalWndProc, hWnd, message, wParam, lParam, false, false, false);

	case WW_CB_FINDSTRINGEXACTA:
		return FindComboString(data, pOriginalWndProc, hWnd, message, wParam, lParam, false, true, false);

	case WW_CB_SELECTSTRINGA:
		return FindComboString(data, pOriginalWndProc, hWnd, message, wParam, lParam, false, false, true);

	case WW_CB_FINDSTRINGW:
		return FindComboString(data, pOriginalWndProc, hWnd, message, wParam, lParam, true, false, false);

	case WW_CB_FINDSTRINGEXACTW:
		return FindComboString(data, pOriginalWndProc, hWnd, message, wParam, lParam, true, true, false);

	case WW_CB_SELECTSTRINGW:
		return FindComboString(data, pOriginalWndProc, hWnd, message, wParam, lParam, true, false, true);

	case WW_CB_INSERTSTRINGA:
	case WW_CB_ADDSTRINGA:
		return AddOrInsertComboString(data, pOriginalWndProc, hWnd, message, wParam, lParam, false);

	case WW_CB_INSERTSTRINGW:
	case WW_CB_ADDSTRINGW:
		return AddOrInsertComboString(data, pOriginalWndProc, hWnd, message, wParam, lParam, true);

	case WW_CB_GETLBTEXTA:
		return GetComboText(pOriginalWndProc, hWnd, message, wParam, lParam, false);

	case WW_CB_GETLBTEXTW:
		return GetComboText(pOriginalWndProc, hWnd, message, wParam, lParam, true);

	case WW_CB_GETITEMTEXTFORMAT:
		return GetComboText(pOriginalWndProc, hWnd, message, wParam, lParam, true);

	case WW_EDIT_ENTERPRESSED:
	case WW_EDIT_TABNAV:
		if (const HWND parentHwnd = ::GetParent(hWnd))
			::SendMessageA(parentHwnd, message, wParam, lParam);
		return forwardOriginal();

	case WW_CB_ENABLEITEMCOLORS:
		data.AsComboBox().UseItemColorOverrides() = lParam == 1;
		return forwardOriginal();

	case WW_CB_SETMAXVISIBLEDROPITEMS:
		data.AsComboBox().MaxVisibleDropItems() = static_cast<int>(lParam);
		return forwardOriginal();

	case WW_QUERYTOOLTIPHIT:
		if (const HWND dropHwnd = data.AsComboBox().DropDownHwnd())
		{
			POINT point { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			::ClientToScreen(hWnd, &point);
			if (!RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
			{
				point = RenderDX::ScreenToRenderLocalPoint(dropHwnd, point);
			}
			else
			{
				::ScreenToClient(dropHwnd, &point);
			}

			return ::SendMessageA(
				dropHwnd,
				WW_QUERYTOOLTIPHIT,
				0,
				MAKELPARAM(static_cast<WORD>(point.x), static_cast<WORD>(point.y)));
		}
		return -1;

	case WW_CB_SETALTERNATEPALETTE:
		data.AsComboBox().UseAlternatePalette() = lParam == 1;
		return forwardOriginal();

	default:
		break;
	}

	return forwardOriginal();
}
