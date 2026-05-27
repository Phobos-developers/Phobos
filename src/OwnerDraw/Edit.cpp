#include "OwnerDraw.Internal.h"

void CharToWideString(wchar_t* pBuffer, int capacity, const char* pText)
{
	if (!pBuffer || capacity <= 0)
		return;

	pBuffer[0] = L'\0';
	if (!pText)
		return;

	::MultiByteToWideChar(CP_ACP, 0, pText, -1, pBuffer, capacity);
	pBuffer[capacity - 1] = L'\0';
}

void WideToCharString(char* pBuffer, int capacity, const wchar_t* pText)
{
	if (!pBuffer || capacity <= 0)
		return;

	pBuffer[0] = '\0';
	if (!pText)
		return;

	::WideCharToMultiByte(CP_ACP, 0, pText, -1, pBuffer, capacity, nullptr, nullptr);
	pBuffer[capacity - 1] = '\0';
}

static UINT GetCurrentKeyboardCodePage()
{
	char buffer[7] {};
	const WORD language = LOWORD(::GetKeyboardLayout(0));
	const LCID locale = MAKELCID(language, SORT_DEFAULT);

	if (!::GetLocaleInfoA(locale, LOCALE_IDEFAULTANSICODEPAGE, buffer, static_cast<int>(std::size(buffer))))
		return CP_ACP;

	const int codePage = std::atoi(buffer);
	return codePage > 0 ? static_cast<UINT>(codePage) : CP_ACP;
}

static wchar_t LocalizeCharacter(char character)
{
	wchar_t result {};
	::MultiByteToWideChar(GetCurrentKeyboardCodePage(), MB_USEGLYPHCHARS, &character, 1, &result, 1);
	return result;
}

static WideWstring* EnsureNewEditText(OwnerDrawDialogElement& data)
{
	if (!data.AsNewEdit().Text())
	{
		auto pMemory = YRMemory::Allocate(sizeof(WideWstring));
		if (!pMemory)
			return nullptr;

		data.AsNewEdit().Text() = new (pMemory) WideWstring();
	}

	return data.AsNewEdit().Text();
}

static const wchar_t* NewEditTextBuffer(OwnerDrawDialogElement& data)
{
	const auto pText = EnsureNewEditText(data);
	return pText && pText->Buffer ? pText->Buffer : L"";
}

static int NewEditTextLength(OwnerDrawDialogElement& data)
{
	const auto pText = EnsureNewEditText(data);
	return pText ? static_cast<int>(pText->GetLength()) : 0;
}

static void SetNewEditText(OwnerDrawDialogElement& data, const wchar_t* pText)
{
	if (auto pTarget = EnsureNewEditText(data))
		*pTarget = pText ? pText : L"";
}

static void TrimNewEditTextToLimit(OwnerDrawDialogElement& data)
{
	const int limit = data.AsNewEdit().TextLimit();
	if (limit <= 0)
		return;

	if (NewEditTextLength(data) <= limit)
		return;

	std::wstring value(NewEditTextBuffer(data), limit);
	SetNewEditText(data, value.c_str());

	if (data.AsNewEdit().CaretIndex() > limit)
		data.AsNewEdit().CaretIndex() = limit;
}

static bool RemoveNewEditTextRange(OwnerDrawDialogElement& data, int index, int length)
{
	std::wstring value(NewEditTextBuffer(data));
	if (index < 0 || length <= 0 || index >= static_cast<int>(value.size()))
		return false;

	length = std::min(length, static_cast<int>(value.size()) - index);
	value.erase(static_cast<size_t>(index), static_cast<size_t>(length));
	SetNewEditText(data, value.c_str());
	data.AsNewEdit().CaretIndex() = std::clamp(data.AsNewEdit().CaretIndex(), 0, static_cast<int>(value.size()));
	return true;
}

static bool InsertNewEditCharacter(OwnerDrawDialogElement& data, wchar_t character)
{
	if (!character || character <= 0x1F)
		return false;

	if (data.AsNewEdit().AsciiOnly() && character >= 0x100)
		return false;

	if (data.AsNewEdit().RejectChars() && std::wcschr(data.AsNewEdit().RejectChars(), character))
		return false;

	std::wstring value(NewEditTextBuffer(data));
	if (data.AsNewEdit().TextLimit() > 0 && static_cast<int>(value.size()) >= data.AsNewEdit().TextLimit())
		return false;

	int caretIndex = std::clamp(data.AsNewEdit().CaretIndex(), 0, static_cast<int>(value.size()));
	value.insert(value.begin() + caretIndex, character);
	SetNewEditText(data, value.c_str());
	data.AsNewEdit().CaretIndex() = caretIndex + 1;
	return true;
}

static void NotifyNewEditTextChanged(HWND hWnd, HWND parentHwnd)
{
	if (!parentHwnd)
		return;

	const int controlId = ::GetWindowLongA(hWnd, GWL_ID) & 0xFFFF;
	::SendMessageA(parentHwnd, WM_COMMAND, controlId | 0x03000000, reinterpret_cast<LPARAM>(hWnd));
	::SendMessageA(parentHwnd, WM_COMMAND, controlId | 0x04000000, reinterpret_cast<LPARAM>(hWnd));
}

static void NotifyNewEditEnterPressed(HWND hWnd, HWND parentHwnd)
{
	if (parentHwnd)
		::SendMessageA(parentHwnd, WW_EDIT_ENTERPRESSED, 0, reinterpret_cast<LPARAM>(hWnd));
}

static void NotifyNewEditMultilineEnter(HWND hWnd, HWND parentHwnd)
{
	if (!parentHwnd)
		return;

	const int controlId = ::GetWindowLongA(hWnd, GWL_ID) & 0xFFFF;
	::SendMessageA(parentHwnd, WM_COMMAND, controlId | 0x05010000, reinterpret_cast<LPARAM>(hWnd));
}

static bool IsComboBoxParent(HWND parentHwnd)
{
	if (!parentHwnd)
		return false;

	char className[32] {};
	::GetClassNameA(parentHwnd, className, static_cast<int>(std::size(className)));
	return std::strcmp(className, "ComboBox") == 0;
}

static void InvalidateNewEdit(HWND hWnd, HWND parentHwnd)
{
	if (IsComboBoxParent(parentHwnd))
		::InvalidateRect(parentHwnd, nullptr, FALSE);

	::InvalidateRect(hWnd, nullptr, FALSE);
}

static int NewEditTextWidth(BitFont* pFont, const wchar_t* pText)
{
	if (!pText || !pText[0])
		return 0;

	if (!pFont)
		pFont = BitFont::Instance;

	if (!pFont)
		return static_cast<int>(std::wcslen(pText)) * 8;

	int textWidth = 0;
	int textHeight = 0;
	pFont->GetTextDimension(pText, &textWidth, &textHeight, 0);
	return textWidth;
}

static int NewEditFitCharacterCount(BitFont* pFont, const wchar_t* pText, int maxWidth)
{
	if (!pText || maxWidth <= 0)
		return 0;

	const int length = static_cast<int>(std::wcslen(pText));
	int fitCount = 0;

	for (int count = 1; count <= length; ++count)
	{
		std::wstring candidate(pText, pText + count);
		if (NewEditTextWidth(pFont, candidate.c_str()) > maxWidth)
			break;

		fitCount = count;
	}

	return fitCount;
}

static int PrintNewEditTextSegment(
	DSurface* pSurface,
	RectangleStruct& rect,
	BitFont* pFont,
	const std::wstring& text,
	int start,
	int end,
	COLORREF color,
	int animationPos)
{
	if (end <= start || rect.Width <= 0)
		return 0;

	const std::wstring segment(text.begin() + start, text.begin() + end);
	const int width = NewEditTextWidth(pFont, segment.c_str());

	OwnerDraw::PrintTextFixedLength(
		color,
		pFont,
		&rect,
		segment.c_str(),
		static_cast<int>(segment.size()),
		0,
		0,
		pSurface,
		animationPos);

	rect.X += width;
	rect.Width = std::max(rect.Width - width, 0);
	return width;
}

static void AnimatedNewEditTextPrint(
	DSurface* pSurface,
	RectangleStruct textRect,
	const wchar_t* pText,
	int caretIndex,
	BitFont* pFont,
	COLORREF textColor,
	int& scrollStart,
	bool hasFocus,
	bool maskText,
	bool fillBackground,
	int animationPos,
	int caretBlinkState)
{
	if (!pSurface || textRect.Width <= 0 || textRect.Height <= 0)
		return;

	if (!pFont)
		pFont = BitFont::Instance;

	const wchar_t* pSource = pText ? pText : L"";
	const int sourceLength = static_cast<int>(std::wcslen(pSource));
	caretIndex = std::clamp(caretIndex, 0, sourceLength);

	int compositionLength = 0;
	int compositionCursor = 0;
	bool composing = false;
	if (hasFocus)
	{
		OwnerDraw::UpdateIMECompositionString();
		compositionLength = std::clamp(OwnerDraw::IMECompositionStringLength, 0, 0x100);
		compositionCursor = std::clamp(OwnerDraw::IMECompositionCursorPos, 0, compositionLength);
		composing = OwnerDraw::IMEComposing != 0;
	}

	constexpr size_t DisplayBufferCapacity = 0x800;
	std::wstring displayText(pSource);
	if (displayText.size() >= DisplayBufferCapacity)
		displayText.resize(DisplayBufferCapacity - 1);

	const int compositionStart = std::min(caretIndex, static_cast<int>(displayText.size()));
	if (compositionLength > 0)
	{
		displayText.insert(
			displayText.begin() + compositionStart,
			OwnerDraw::IMECompositionString,
			OwnerDraw::IMECompositionString + compositionLength);

		if (displayText.size() >= DisplayBufferCapacity)
			displayText.resize(DisplayBufferCapacity - 1);
	}

	if (maskText)
		std::fill(displayText.begin(), displayText.end(), L'*');

	const int displayLength = static_cast<int>(displayText.size());
	const int compositionEnd = std::min(compositionStart + compositionLength, displayLength);
	int displayCaret = composing || compositionLength
		? compositionStart + compositionCursor
		: std::min(caretIndex, displayLength);
	displayCaret = std::clamp(displayCaret, 0, displayLength);

	scrollStart = std::clamp(scrollStart, 0, displayLength);
	if (displayCaret < scrollStart + 5)
		scrollStart = std::max(displayCaret - 5, 0);

	while (scrollStart < displayLength)
	{
		const int visibleCount = NewEditFitCharacterCount(pFont, displayText.c_str() + scrollStart, textRect.Width - 5);
		if (visibleCount >= displayCaret - scrollStart)
			break;

		++scrollStart;
	}

	if (fillBackground)
	{
		RectangleStruct fillRect
		{
			textRect.X - 1,
			textRect.Y - 1,
			NewEditTextWidth(pFont, displayText.c_str() + scrollStart) + 5,
			textRect.Height + 2
		};
		pSurface->FillRect(&fillRect, 0);
	}

	RectangleStruct drawRect = textRect;
	int caretX = -1;

	auto drawRange = [&](int rangeStart, int rangeEnd, COLORREF color)
	{
		int visibleStart = std::max(rangeStart, scrollStart);
		int visibleEnd = std::min(rangeEnd, displayLength);
		if (visibleEnd <= visibleStart)
			return;

		if (caretX < 0 && displayCaret >= visibleStart && displayCaret <= visibleEnd)
		{
			PrintNewEditTextSegment(pSurface, drawRect, pFont, displayText, visibleStart, displayCaret, color, animationPos);
			caretX = drawRect.X;
			PrintNewEditTextSegment(pSurface, drawRect, pFont, displayText, displayCaret, visibleEnd, color, animationPos);
		}
		else
		{
			PrintNewEditTextSegment(pSurface, drawRect, pFont, displayText, visibleStart, visibleEnd, color, animationPos);
		}
	};

	drawRange(0, compositionStart, textColor);
	drawRange(compositionStart, compositionEnd, OwnerDraw::ImeCompositionTextColor);
	drawRange(compositionEnd, displayLength, textColor);

	if (caretX < 0)
	{
		if (displayCaret <= scrollStart)
			caretX = textRect.X;
		else if (displayCaret >= displayLength)
			caretX = drawRect.X;
	}

	if (hasFocus && caretX >= 0 && !caretBlinkState)
	{
		const WORD caretColor = static_cast<WORD>(ConvertRGBToSurfaceColor(Phobos::UI::ColorCaret));
		Point2D start { caretX, textRect.Y };
		Point2D end { caretX, textRect.Y + textRect.Height - 2 };
		DrawAlphaLine(pSurface, start, end, caretColor, 0xFF);

		++start.X;
		++end.X;
		DrawAlphaLine(pSurface, start, end, caretColor, 0xFF);
	}
}

static void PaintNewEdit(HWND hWnd, OwnerDrawDialogElement& data, HWND parentHwnd)
{
	if (!DSurface::Alternate)
		return;

	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	const int width = ownerRect.right - ownerRect.left + 1;
	const int height = ownerRect.bottom - ownerRect.top + 1;
	if (width <= 0 || height <= 0)
		return;

	RectangleStruct drawRect { ownerRect.left, ownerRect.top, width, height };
	OwnerDraw::CopyDimmedBackground(&drawRect, hWnd, static_cast<unsigned int>(data.Alpha));

	if (!IsComboBoxParent(parentHwnd))
		DrawBeveledBorder(DSurface::Alternate, drawRect, 2, -1);

	RectangleStruct textRect
	{
		ownerRect.left + 2,
		ownerRect.top,
		ownerRect.right - ownerRect.left + 1,
		ownerRect.bottom - ownerRect.top + 1
	};

	AnimatedNewEditTextPrint(
		DSurface::Alternate,
		textRect,
		NewEditTextBuffer(data),
		data.AsNewEdit().CaretIndex(),
		data.AsNewEdit().Font(),
		Phobos::UI::ColorTextEdit,
		data.AsNewEdit().ScrollStart(),
		data.HasFocus != 0,
		((data.AsNewEdit().StyleFlags() >> 5) & 1) != 0,
		false,
		0,
		data.AsNewEdit().CaretBlinkState());

	::ValidateRect(hWnd, nullptr);
}

static size_t GetEditWideText(HWND hWnd, wchar_t* pWideText, int capacity, int* pSelectionEndChars)
{
	if (pSelectionEndChars)
		*pSelectionEndChars = 0;

	if (!pWideText || capacity <= 0)
		return 0;

	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);

	char ansiText[0x400] {};
	CallSelectedHandler(
		pOriginalWndProc,
		hWnd,
		WM_GETTEXT,
		static_cast<WPARAM>(std::size(ansiText)),
		reinterpret_cast<LPARAM>(ansiText));

	if (pSelectionEndChars)
	{
		DWORD selectionStart = 0;
		DWORD selectionEnd = 0;
		const auto selection = static_cast<DWORD>(CallSelectedHandler(
			pOriginalWndProc,
			hWnd,
			EM_GETSEL,
			reinterpret_cast<WPARAM>(&selectionStart),
			reinterpret_cast<LPARAM>(&selectionEnd)));
		const auto selectionEndBytes = static_cast<size_t>(HIWORD(selection));
		*pSelectionEndChars = static_cast<int>(_mbsnccnt(
			reinterpret_cast<const unsigned char*>(ansiText),
			selectionEndBytes));
	}

	pWideText[0] = L'\0';
	::MultiByteToWideChar(CP_ACP, 0, ansiText, -1, pWideText, capacity);
	pWideText[capacity - 1] = L'\0';

	const size_t wideLength = std::wcslen(pWideText);
	std::wstring normalized;
	normalized.reserve(wideLength + 2);

	bool removedNewLine = false;
	for (size_t i = 0; i < wideLength; ++i)
	{
		if (pWideText[i] == L'\r' || pWideText[i] == L'\n')
		{
			removedNewLine = true;
			continue;
		}

		normalized.push_back(pWideText[i]);
	}

	if (removedNewLine)
		normalized += L"\r\n";

	std::wcsncpy(pWideText, normalized.c_str(), static_cast<size_t>(capacity - 1));
	pWideText[capacity - 1] = L'\0';
	return std::wcslen(pWideText);
}

static LRESULT ForwardEditSetText(OwnerDrawDialogElement& data, HWND hWnd, WNDPROC pOriginalWndProc)
{
	char ansiText[0x800] {};
	if (data.TextBuffer)
		WideToCharString(ansiText, static_cast<int>(std::size(ansiText)), data.TextBuffer);

	return CallSelectedHandler(
		pOriginalWndProc,
		hWnd,
		WM_SETTEXT,
		0,
		reinterpret_cast<LPARAM>(ansiText));
}

static LRESULT CopyEditTextW(HWND hWnd, WPARAM capacityParam, LPARAM lParam)
{
	auto pBuffer = reinterpret_cast<wchar_t*>(lParam);
	const int capacity = static_cast<int>(capacityParam);
	if (!pBuffer || capacity <= 0)
		return 0;

	std::vector<wchar_t> text(0x800);
	GetEditWideText(hWnd, text.data(), static_cast<int>(text.size()), nullptr);

	std::wcsncpy(pBuffer, text.data(), static_cast<size_t>(capacity - 1));
	pBuffer[capacity - 1] = L'\0';
	return static_cast<LRESULT>(std::wcslen(pBuffer));
}

static LRESULT CopyEditTextA(HWND hWnd, WPARAM capacityParam, LPARAM lParam)
{
	auto pBuffer = reinterpret_cast<char*>(lParam);
	const int capacity = static_cast<int>(capacityParam);
	if (!pBuffer || capacity <= 0)
		return 0;

	std::vector<wchar_t> text(0x800);
	GetEditWideText(hWnd, text.data(), static_cast<int>(text.size()), nullptr);
	WideToCharString(pBuffer, capacity, text.data());
	return static_cast<LRESULT>(std::strlen(pBuffer));
}

static LRESULT AppendEditNewLine(HWND hWnd, WNDPROC pOriginalWndProc)
{
	const int textLength = static_cast<int>(CallSelectedHandler(pOriginalWndProc, hWnd, WM_GETTEXTLENGTH, 0, 0));
	std::vector<char> text(static_cast<size_t>(std::max(textLength + 3, 3)), '\0');

	CallSelectedHandler(
		pOriginalWndProc,
		hWnd,
		WM_GETTEXT,
		static_cast<WPARAM>(text.size()),
		reinterpret_cast<LPARAM>(text.data()));

	const size_t copiedLength = std::strlen(text.data());
	if (copiedLength + 2 < text.size())
		std::memcpy(text.data() + copiedLength, "\r\n", 3);

	CallSelectedHandler(pOriginalWndProc, hWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(text.data()));

	if (const HWND parentHwnd = ::GetParent(hWnd))
	{
		const int controlId = ::GetWindowLongA(hWnd, GWL_ID) & 0xFFFF;
		::SendMessageA(parentHwnd, WM_COMMAND, controlId | 0x05010000, reinterpret_cast<LPARAM>(hWnd));
	}

	return 0;
}

static void PaintEdit(HWND hWnd, OwnerDrawDialogElement& data, HWND parentHwnd, UINT message)
{
	if (!DSurface::Alternate)
		return;

	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	if (message == WM_PAINT)
	{
		RECT updateRect {};
		if (::GetUpdateRect(hWnd, &updateRect, FALSE))
		{
			updateRect.left += ownerRect.left;
			updateRect.top += ownerRect.top;
			updateRect.right += ownerRect.left;
			updateRect.bottom += ownerRect.top;
		}
	}

	const int width = ownerRect.right - ownerRect.left + 1;
	const int height = ownerRect.bottom - ownerRect.top + 1;
	if (width <= 0 || height <= 0)
		return;

	RectangleStruct drawRect { ownerRect.left, ownerRect.top, width, height };
	OwnerDraw::CopyDimmedBackground(&drawRect, hWnd, static_cast<unsigned int>(data.Alpha));

	if (!IsComboBoxParent(parentHwnd))
		DrawBeveledBorder(DSurface::Alternate, drawRect, 2, -1);

	std::vector<wchar_t> text(0x1400);
	int caretIndex = 0;
	GetEditWideText(hWnd, text.data(), static_cast<int>(text.size()), &caretIndex);

	RectangleStruct textRect { ownerRect.left, ownerRect.top, width, height };
	const bool maskText = ((::GetWindowLongA(hWnd, GWL_STYLE) >> 5) & 1) != 0;

	AnimatedNewEditTextPrint(
		DSurface::Alternate,
		textRect,
		text.data(),
		caretIndex,
		data.AsEdit().TextFont(),
		Phobos::UI::ColorText,
		data.AsEdit().TextScrollStart(),
		data.HasFocus != 0,
		maskText,
		false,
		0,
		0);

	::ValidateRect(hWnd, nullptr);
}

LRESULT CALLBACK WWUI::EditCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto pData = FindOwnerDrawData(hWnd);
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	if (!pData)
		return forwardOriginal();

	auto& data = *pData;
	if (::GetFocus() == hWnd && !data.AsEdit().FocusRestoreReadyFlag())
	{
		data.AsEdit().FocusRestorePendingFlag() = 1;
		::SetFocus(Game::hWnd);
	}

	const LONG windowStyle = ::GetWindowLongA(hWnd, GWL_STYLE);
	const HWND parentHwnd = ::GetParent(hWnd);

	if ((message == WM_KEYDOWN || message == WM_KEYUP) && wParam == VK_TAB)
		return 0;

	switch (message)
	{
	case WW_INITDIALOG:
	{
		if (!parentHwnd)
			return 0;

		RECT windowRect {};
		RECT clientRect {};
		RECT parentRect {};
		::GetWindowRect(hWnd, &windowRect);
		::GetClientRect(hWnd, &clientRect);
		::GetWindowRect(parentHwnd, &parentRect);

		::MoveWindow(
			hWnd,
			windowRect.left - parentRect.left + 1,
			windowRect.top - parentRect.top + 1,
			clientRect.right - 2,
			clientRect.bottom - 2,
			FALSE);

		if (::GetFocus() == hWnd)
		{
			data.AsEdit().FocusRestorePendingFlag() = 1;
			::SetFocus(Game::hWnd);
		}

		if (windowStyle & WS_TABSTOP)
		{
			data.AsEdit().RestoreTabStopFlag() = 1;
			::SetWindowLongA(hWnd, GWL_STYLE, windowStyle & ~static_cast<LONG>(WS_TABSTOP));
		}

		return 0;
	}

	case WM_SETFOCUS:
		::SendMessageA(hWnd, EM_SETSEL, static_cast<WPARAM>(-1), static_cast<LPARAM>(-1));
		if (!data.AsEdit().FocusRestoreReadyFlag())
			::PostMessageA(hWnd, WW_EDIT_DEFERFOCUSRESTORE, 0, 0);

		InvalidateNewEdit(hWnd, parentHwnd);
		return forwardOriginal();

	case WW_GETTEXTW:
		return CopyEditTextW(hWnd, wParam, lParam);

	case WW_GETTEXTA:
		return CopyEditTextA(hWnd, wParam, lParam);

	case WM_GETTEXTLENGTH:
	{
		std::vector<wchar_t> text(0x800);
		return static_cast<LRESULT>(GetEditWideText(hWnd, text.data(), static_cast<int>(text.size()), nullptr));
	}

	case WW_SETTEXTW:
	case WW_SETTEXTA:
		return ForwardEditSetText(data, hWnd, pOriginalWndProc);

	case WM_CHAR:
		if (wParam == VK_RETURN)
		{
			if (windowStyle & ES_MULTILINE)
				return AppendEditNewLine(hWnd, pOriginalWndProc);

			return forwardOriginal();
		}

		if (wParam == VK_TAB)
		{
			if (const HWND nextHwnd = ::GetNextDlgTabItem(parentHwnd, hWnd, FALSE))
				::SetFocus(nextHwnd);
			else
				::SetFocus(hWnd);

			return 0;
		}

		return forwardOriginal();

	case WW_EDIT_RESTOREFOCUS:
		data.AsEdit().FocusRestoreReadyFlag() = 1;
		if (data.AsEdit().FocusRestorePendingFlag())
		{
			::SetFocus(hWnd);
			data.AsEdit().FocusRestorePendingFlag() = 0;
		}

		if (data.AsEdit().RestoreTabStopFlag())
			::SetWindowLongA(hWnd, GWL_STYLE, windowStyle | WS_TABSTOP);

		return 0;

	case WM_PAINT:
	case WM_ERASEBKGND:
		PaintEdit(hWnd, data, parentHwnd, message);
		break;

	case WM_CONTEXTMENU:
		return 1;

	case WM_MOUSEMOVE:
		return 1;

	default:
		break;
	}

	switch (message)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	case WM_SYSDEADCHAR:
	case WM_KILLFOCUS:
	case WM_LBUTTONDOWN:
		InvalidateNewEdit(hWnd, parentHwnd);
		break;

	default:
		break;
	}

	return forwardOriginal();
}

LRESULT CALLBACK WWUI::NewEditCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;

	auto pData = FindOwnerDrawData(hWnd);
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	if (!pData)
		return forwardOriginal();

	auto& data = *pData;
	EnsureNewEditText(data);

	const HWND parentHwnd = ::GetParent(hWnd);

	if ((message == WM_KEYDOWN || message == WM_KEYUP) && wParam == VK_TAB)
	{
		if (message == WM_KEYDOWN && parentHwnd)
		{
			const WPARAM shiftPressed = (::GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
			::SendMessageA(parentHwnd, WW_EDIT_TABNAV, shiftPressed, reinterpret_cast<LPARAM>(hWnd));
		}

		return 0;
	}

	auto copyWideText = [&]() -> LRESULT
	{
		const int capacity = static_cast<int>(wParam);
		auto pBuffer = reinterpret_cast<wchar_t*>(lParam);
		if (!pBuffer || capacity <= 0)
			return 0;

		const auto pText = NewEditTextBuffer(data);
		std::wcsncpy(pBuffer, pText, capacity - 1);
		pBuffer[capacity - 1] = L'\0';
		return static_cast<LRESULT>(std::wcslen(pBuffer));
	};

	auto copyAnsiText = [&]() -> LRESULT
	{
		const int capacity = static_cast<int>(wParam);
		auto pBuffer = reinterpret_cast<char*>(lParam);
		if (!pBuffer || capacity <= 0)
			return 0;

		WideToCharString(pBuffer, capacity, NewEditTextBuffer(data));
		return static_cast<LRESULT>(std::strlen(pBuffer));
	};

	auto handleInputCharacter = [&](wchar_t character, bool consumedInput) -> LRESULT
	{
		if (!character)
			return consumedInput ? 0 : forwardOriginal();

		if (InsertNewEditCharacter(data, character))
			NotifyNewEditTextChanged(hWnd, parentHwnd);

		return 0;
	};

	switch (message)
	{
	case WW_INITDIALOG:
	{
		if (!parentHwnd)
			return 0;

		RECT windowRect {};
		RECT clientRect {};
		RECT parentRect {};
		::GetWindowRect(hWnd, &windowRect);
		::GetClientRect(hWnd, &clientRect);
		::GetWindowRect(parentHwnd, &parentRect);

		::SetWindowPos(
			hWnd,
			nullptr,
			windowRect.left - parentRect.left + 1,
			windowRect.top - parentRect.top + 1,
			clientRect.right - 2,
			clientRect.bottom - 2,
			SWP_SHOWWINDOW);
		return 0;
	}

	case EM_LIMITTEXT:
		data.AsNewEdit().TextLimit() = static_cast<int>(wParam);
		TrimNewEditTextToLimit(data);
		return forwardOriginal();

	case WW_GETTEXTW:
		return copyWideText();

	case WW_GETTEXTA:
		return copyAnsiText();

	case WM_GETTEXTLENGTH:
		return NewEditTextLength(data);

	case WW_SETTEXTW:
	case WW_SETTEXTA:
		SetNewEditText(data, data.TextBuffer ? data.TextBuffer : L"");
		data.AsNewEdit().CaretIndex() = 0;
		data.AsNewEdit().ScrollStart() = 0;
		TrimNewEditTextToLimit(data);
		data.AsNewEdit().CaretIndex() = NewEditTextLength(data);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			NotifyNewEditEnterPressed(hWnd, parentHwnd);
			if (data.AsNewEdit().StyleFlags() & 4)
			{
				if (auto pText = EnsureNewEditText(data))
					*pText += L"\r\n";

				NotifyNewEditMultilineEnter(hWnd, parentHwnd);
			}
			return 0;
		}
		break;

	case WM_SETFOCUS:
		data.AsNewEdit().CaretBlinkState() = 0;
		::SetTimer(hWnd, 0, 1000, nullptr);
		InvalidateNewEdit(hWnd, parentHwnd);
		return forwardOriginal();

	case WM_KILLFOCUS:
		::KillTimer(hWnd, 0);
		InvalidateNewEdit(hWnd, parentHwnd);
		return forwardOriginal();

	case WM_TIMER:
		data.AsNewEdit().CaretBlinkState() ^= 1;
		::InvalidateRect(hWnd, nullptr, FALSE);
		return forwardOriginal();

	case WM_PAINT:
	case WM_ERASEBKGND:
		PaintNewEdit(hWnd, data, parentHwnd);
		break;

	case WM_CONTEXTMENU:
		return 1;

	case WM_MOUSEMOVE:
		return 1;

	default:
		break;
	}

	switch (message)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	case WM_SYSDEADCHAR:
	case WM_LBUTTONDOWN:
		InvalidateNewEdit(hWnd, parentHwnd);
		break;

	default:
		break;
	}

	if (message == WM_CHAR)
	{
		if (wParam <= 0x1F)
			return forwardOriginal();

		return handleInputCharacter(LocalizeCharacter(static_cast<char>(wParam)), true);
	}

	if (message == WM_KEYDOWN)
	{
		bool textChanged = false;
		switch (wParam)
		{
		case VK_BACK:
			if (data.AsNewEdit().CaretIndex() > 0)
			{
				--data.AsNewEdit().CaretIndex();
				textChanged = RemoveNewEditTextRange(data, data.AsNewEdit().CaretIndex(), 1);
			}
			break;

		case VK_DELETE:
			if (data.AsNewEdit().CaretIndex() < NewEditTextLength(data))
				textChanged = RemoveNewEditTextRange(data, data.AsNewEdit().CaretIndex(), 1);
			break;

		case VK_END:
			data.AsNewEdit().CaretIndex() = NewEditTextLength(data);
			return 0;

		case VK_HOME:
			data.AsNewEdit().CaretIndex() = 0;
			return 0;

		case VK_LEFT:
			if (data.AsNewEdit().CaretIndex() > 0)
				--data.AsNewEdit().CaretIndex();
			return 0;

		case VK_RIGHT:
			if (data.AsNewEdit().CaretIndex() < NewEditTextLength(data))
				++data.AsNewEdit().CaretIndex();
			return 0;

		default:
			return forwardOriginal();
		}

		if (textChanged)
			NotifyNewEditTextChanged(hWnd, parentHwnd);

		return 0;
	}

	if (message == WM_IME_CHAR)
		return handleInputCharacter(OwnerDraw::ConvertIMECharToWide(static_cast<UINT>(wParam), lParam), true);

	if (message == WW_EDIT_INPUTCHARW)
		return handleInputCharacter(static_cast<wchar_t>(wParam), true);

	return forwardOriginal();
}
