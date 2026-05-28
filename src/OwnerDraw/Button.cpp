#include "OwnerDraw.Internal.h"

constexpr UINT OwnerDrawButtonTimerId = 0;
constexpr UINT OwnerDrawButtonTimerInterval = 1000;
constexpr int OwnerDrawButtonTextStyle = 5;
constexpr int OwnerDrawButtonTextAlign = 12;
constexpr BYTE OwnerDrawButtonDisabledOverlayAlpha = 0x80;

static COLORREF MakeOwnerDrawButtonSideTextColor(BYTE red, WORD greenBlue)
{
	const BYTE green = static_cast<BYTE>(greenBlue & 0xFF);
	const BYTE blue = static_cast<BYTE>((greenBlue >> 8) & 0xFF);
	const int surfaceColor = Drawing::RGB_To_Int(red, green, blue);

	BYTE outputRed = 0;
	BYTE outputGreen = 0;
	BYTE outputBlue = 0;
	Drawing::Int_To_RGB(surfaceColor, outputRed, outputGreen, outputBlue);

	return static_cast<COLORREF>(
		outputRed
		| (static_cast<DWORD>(outputGreen) << 8)
		| ((static_cast<DWORD>(outputBlue) | 0x200) << 16));
}

static COLORREF GetDisabledOwnerDrawButtonTextColor()
{
	if (!SessionClass::Instance.CurrentlyInGame || !ScenarioClass::Instance)
		return Phobos::UI::ColorDisabledButton;

	switch (ScenarioClass::Instance->PlayerSideIndex)
	{
	case 0:
		return MakeOwnerDrawButtonSideTextColor(
			OwnerDraw::ButtonDisabledSide0Red,
			OwnerDraw::ButtonDisabledSide0GreenBlue);

	case 1:
		return MakeOwnerDrawButtonSideTextColor(
			OwnerDraw::ButtonDisabledSide1Red,
			OwnerDraw::ButtonDisabledSide1GreenBlue);

	default:
		return MakeOwnerDrawButtonSideTextColor(
			OwnerDraw::ButtonDisabledSideOtherRed,
			OwnerDraw::ButtonDisabledSideOtherGreenBlue);
	}
}

static void EnsureOwnerDrawButtonCache(OwnerDrawDialogElement& data, const RECT& clientRect, const RECT& ownerRect)
{
	if (data.CacheSurface || !DSurface::Alternate)
		return;

	const int width = clientRect.right + 1;
	const int height = clientRect.bottom + 1;
	if (width <= 0 || height <= 0)
		return;

	data.CacheSurface = GameCreate<BSurface>(width, height);
	if (!data.CacheSurface)
		return;

	++OwnerDraw::CachedSurfaceCount;

	RectangleStruct destRect { 0, 0, width, height };
	RectangleStruct sourceRect { ownerRect.left, ownerRect.top, width, height };
	CopySurfacePart(data.CacheSurface, destRect, DSurface::Alternate, sourceRect);
}

static void RestoreOwnerDrawButtonCache(HWND hWnd, OwnerDrawDialogElement& data, const RECT& clientRect, const RECT& ownerRect)
{
	if (!data.CacheSurface || !DSurface::Alternate)
		return;

	const int width = clientRect.right + 1;
	const int height = clientRect.bottom + 1;
	if (width <= 0 || height <= 0)
		return;

	RectangleStruct destRect { ownerRect.left, ownerRect.top, width, height };
	RectangleStruct sourceRect { 0, 0, width, height };
	CopySurfacePart(DSurface::Alternate, destRect, data.CacheSurface, sourceRect);
	::InvalidateRect(hWnd, nullptr, FALSE);
}

static bool DrawOwnerDrawButtonShape(
	OwnerDrawDialogElement& data,
	const RectangleStruct& controlRect,
	int drawItemState,
	LONG windowStyle,
	COLORREF& textColor)
{
	ConvertClass* pConvert = nullptr;
	SHPStruct* pShape = nullptr;
	int frame = 0;

	switch (data.LayoutBand)
	{
	case 1:
		pConvert = OwnerDraw::GetSmallButtonAnimConvert();
		pShape = OwnerDraw::SmallButtonAnimShape;
		frame = 2;
		if (drawItemState & 1)
			frame = 4;
		else if (data.AsButton().AlternateFrame())
			frame = 3;
		break;

	case 2:
		pConvert = OwnerDraw::GetSideButtonConvert();
		pShape = OwnerDraw::SideButtonShape;
		if (drawItemState & 1)
			frame = 1;
		else if (data.AsButton().AlternateFrame())
			frame = 2;
		break;

	case 3:
		pConvert = OwnerDraw::GetCloseButtonConvert();
		pShape = OwnerDraw::CloseButtonShape;
		if (drawItemState & 1)
			frame = 1;
		else if (data.AsButton().AlternateFrame())
			frame = 2;
		break;

	default:
		break;
	}

	if (windowStyle & WS_DISABLED)
		textColor = GetDisabledOwnerDrawButtonTextColor();

	if (!pConvert || !pShape || !DSurface::Alternate)
		return false;

	Point2D position { controlRect.X, controlRect.Y };
	RectangleStruct bounds = DSurface::Alternate->GetRect();
	CC_Draw_Shape(
		DSurface::Alternate,
		pConvert,
		pShape,
		frame,
		&position,
		&bounds,
		BlitterFlags::bf_400,
		0,
		0,
		ZGradient::Ground,
		1000,
		0,
		nullptr,
		0,
		0,
		0);

	return true;
}

static void DrawOwnerDrawButtonImage(
	OwnerDrawDialogElement& data,
	const RectangleStruct& controlRect,
	int drawItemState)
{
	auto pImage = data.ControlImage;
	if (!pImage)
		return;

	if ((drawItemState & 1) && data.StateImageSurface)
		pImage = data.StateImageSurface;

	RectangleStruct sourceRect { 0, 0, controlRect.Width, controlRect.Height };
	CopySurfacePart(DSurface::Alternate, controlRect, pImage, sourceRect);
}

static int SelectOwnerDrawButtonSliceIndex(int height)
{
	return height >= 30 ? 1 : 0;
}

static void DrawOwnerDrawButtonSlices(
	HWND hWnd,
	OwnerDrawDialogElement& data,
	const RECT& clientRect,
	const RECT& ownerRect,
	RectangleStruct& drawRect,
	int drawItemState,
	LONG windowStyle)
{
	const bool pressed = (drawItemState & 1) != 0;
	char variant = pressed ? 'd' : 'u';

	if (windowStyle & WS_DISABLED)
	{
		variant = 'u';
	}
	else if (variant == 'd' && OwnerDraw::ButtonSliceVariant == 'u')
	{
		VocClass::PlayGlobal(RulesClass::Instance->GenericClick, 0x2000, 1.0f);
	}

	OwnerDraw::ButtonSliceVariant = variant;

	const int sliceHeights[2] { 24, 30 };
	const int leftSliceWidths[2] { 7, 10 };
	const int rightSliceWidths[2] { 7, 10 };
	const int sliceIndex = SelectOwnerDrawButtonSliceIndex(drawRect.Height);
	const int sliceHeight = sliceHeights[sliceIndex];
	const int leftSliceWidth = leftSliceWidths[sliceIndex];
	const int rightSliceWidth = rightSliceWidths[sliceIndex];

	RestoreOwnerDrawButtonCache(hWnd, data, clientRect, ownerRect);

	drawRect.Y += (drawRect.Height - sliceHeight) / 2;
	if (pressed)
		drawRect.Y += 2;

	char filename[32] {};

	std::snprintf(filename, sizeof(filename), "b%c%c_li%d.pcx", variant, 'e', sliceHeight);
	if (auto pLeft = GetPCXSurface(filename))
	{
		drawRect.Height = pLeft->GetHeight();
		RectangleStruct destRect { drawRect.X, drawRect.Y, leftSliceWidth, sliceHeight };
		RectangleStruct sourceRect { 0, 0, leftSliceWidth, sliceHeight };
		CopySurfacePart(DSurface::Alternate, destRect, pLeft, sourceRect);
	}
	else
	{
		drawRect.Height = sliceHeight;
	}

	std::snprintf(filename, sizeof(filename), "b%c%c_mi%d.pcx", variant, 'e', sliceHeight);
	if (auto pMiddle = GetPCXSurface(filename))
	{
		RectangleStruct middleRect
		{
			drawRect.X + leftSliceWidth,
			drawRect.Y,
			drawRect.Width - leftSliceWidth - rightSliceWidth,
			pMiddle->GetHeight()
		};

		if (middleRect.Width > 0 && middleRect.Height > 0)
			BlitTiledPCX(middleRect, DSurface::Alternate, pMiddle, 0, 0);
	}

	std::snprintf(filename, sizeof(filename), "b%c%c_ri%d.pcx", variant, 'e', sliceHeight);
	if (auto pRight = GetPCXSurface(filename))
	{
		const int rightHeight = pRight->GetHeight();
		RectangleStruct destRect
		{
			drawRect.X + drawRect.Width - rightSliceWidth,
			drawRect.Y,
			rightSliceWidth,
			rightHeight
		};
		RectangleStruct sourceRect { 0, 0, rightSliceWidth, rightHeight };
		CopySurfacePart(DSurface::Alternate, destRect, pRight, sourceRect);
	}
}

static void DrawOwnerDrawButtonText(
	OwnerDrawDialogElement& data,
	const RectangleStruct& drawRect,
	int drawItemState,
	COLORREF textColor)
{
	if (data.ControlImage || !data.TextBuffer)
		return;

	RECT textRect
	{
		drawRect.X,
		drawRect.Y + 1,
		drawRect.X + drawRect.Width - 2,
		drawRect.Y + drawRect.Height
	};

	if (drawItemState & 1)
	{
		textRect.left = drawRect.X + 2;
		textRect.top += 4;
	}

	OwnerDraw::DrawWideText(
		DSurface::Alternate,
		data.TextBuffer,
		&textRect,
		data.AsButton().Font(),
		textColor,
		OwnerDrawButtonTextStyle,
		OwnerDrawButtonTextAlign,
		0,
		0,
		0);
}

static LRESULT PaintOwnerDrawButton(HWND hWnd, OwnerDrawDialogElement& data, LONG windowStyle)
{
	if (data.SkipDraw)
	{
		::ValidateRect(hWnd, nullptr);
		return 0;
	}

	RECT ownerRect {};
	RECT clientRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates() || !RenderDX::GetClientRectInRender(hWnd, &clientRect))
		::GetClientRect(hWnd, &clientRect);

	const int width = ownerRect.right - ownerRect.left;
	const int height = ownerRect.bottom - ownerRect.top;
	RectangleStruct controlRect { ownerRect.left, ownerRect.top, width, height };
	RectangleStruct drawRect = controlRect;

	if (DSurface::Alternate)
	{
		EnsureOwnerDrawButtonCache(data, clientRect, ownerRect);

		COLORREF textColor = Phobos::UI::ColorTextButton;
		const int drawItemState = data.AsButton().DrawItemState();
		if (data.LayoutBand)
		{
			DrawOwnerDrawButtonShape(data, controlRect, drawItemState, windowStyle, textColor);
		}
		else if (data.ControlImage)
		{
			DrawOwnerDrawButtonImage(data, controlRect, drawItemState);
		}
		else
		{
			DrawOwnerDrawButtonSlices(hWnd, data, clientRect, ownerRect, drawRect, drawItemState, windowStyle);
		}

		DrawOwnerDrawButtonText(data, drawRect, drawItemState, textColor);

		if (!data.LayoutBand && (windowStyle & WS_DISABLED))
			BlendFillRect(controlRect, DSurface::Alternate, 0, OwnerDrawButtonDisabledOverlayAlpha);
	}

	::ValidateRect(hWnd, nullptr);
	return 0;
}

constexpr int CheckboxArtSize = 18;
constexpr int CheckboxTextOffset = 26;
constexpr int CheckboxTextStyle = 4;
constexpr int CheckboxTextAlign = 12;

static const char* SelectCheckboxArtName(OwnerDrawDialogElement& data)
{
	const bool checked = data.AsCheckbox().CheckState() == BST_CHECKED;

	if (data.AsCheckbox().UseExtendedArt())
	{
		if (checked)
			return data.AsCheckbox().ArtVariant() ? "cce_i.pcx" : "cce_il.pcx";

		return data.AsCheckbox().ArtVariant() ? "cce_ir.pcx" : "cue_i.pcx";
	}

	return checked ? "cce_i.pcx" : "cue_i.pcx";
}

static LRESULT PaintCheckboxCtrl(HWND hWnd, OwnerDrawDialogElement& data, LONG windowStyle)
{
	if (!DSurface::Alternate)
	{
		::ValidateRect(hWnd, nullptr);
		return 0;
	}

	RECT clientRect {};
	RECT textRect {};
	RECT ownerRect {};
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates() || !RenderDX::GetClientRectInRender(hWnd, &clientRect))
		::GetClientRect(hWnd, &clientRect);
	OwnerDraw::GetRectangle(hWnd, &textRect);
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	const RectangleStruct artDest
	{
		ownerRect.left,
		ownerRect.top,
		CheckboxArtSize,
		CheckboxArtSize
	};

	if (auto pArt = GetPCXSurface(SelectCheckboxArtName(data)))
	{
		RectangleStruct sourceRect { 0, 0, pArt->GetWidth(), pArt->GetHeight() };
		CopySurfacePart(DSurface::Alternate, artDest, pArt, sourceRect);
	}

	if (windowStyle & WS_DISABLED)
		BlendFillRect(artDest, DSurface::Alternate, 0, OwnerDraw::DisabledOverlayAlpha);

	if (data.TextBuffer)
	{
		textRect.left += CheckboxTextOffset;
		const COLORREF textColor = (windowStyle & WS_DISABLED)
			? Phobos::UI::ColorDisabledCheckbox
			: Phobos::UI::ColorTextCheckbox;

		OwnerDraw::DrawWideText(
			DSurface::Alternate,
			data.TextBuffer,
			&textRect,
			data.AsCheckbox().Font(),
			textColor,
			CheckboxTextStyle,
			CheckboxTextAlign,
			0,
			0,
			0);
	}

	::ValidateRect(hWnd, nullptr);
	return 0;
}

static bool IsInsideCheckboxArt(HWND hWnd, LPARAM lParam)
{
	const POINT point = RenderDX::MouseLParamToRenderLocalPoint(hWnd, lParam);
	return point.x >= 0 && point.y >= 0 && point.x < CheckboxArtSize && point.y < CheckboxArtSize;
}

static void NotifyCheckboxClicked(HWND hWnd, int checkState)
{
	if (RulesClass::Instance)
		VocClass::PlayGlobal(RulesClass::Instance->GUICheckboxSound, 0x2000, 1.0f);

	if (const HWND parentHwnd = ::GetParent(hWnd))
	{
		const WPARAM command = static_cast<WPARAM>(
			(::GetWindowLongA(hWnd, GWL_ID) & 0xFFFF)
			| ((checkState & 0xFFFF) << 16));

		::SendMessageA(parentHwnd, WM_COMMAND, command, reinterpret_cast<LPARAM>(hWnd));
	}
}

constexpr int RadioTextStyle = 5;
constexpr int RadioTextAlign = 12;
constexpr BYTE RadioDisabledOverlayAlpha = 0x80;

static void EnsureRadioCache(OwnerDrawDialogElement& data, const RECT& clientRect, const RECT& ownerRect)
{
	if (data.CacheSurface || !DSurface::Alternate)
		return;

	const int width = clientRect.right + 1;
	const int height = clientRect.bottom + 1;
	if (width <= 0 || height <= 0)
		return;

	data.CacheSurface = GameCreate<BSurface>(width, height);
	if (!data.CacheSurface)
		return;

	++OwnerDraw::CachedSurfaceCount;

	RectangleStruct destRect { 0, 0, width, height };
	RectangleStruct sourceRect { ownerRect.left, ownerRect.top, width, height };
	CopySurfacePart(data.CacheSurface, destRect, DSurface::Alternate, sourceRect);
}

static void RestoreRadioCache(HWND hWnd, OwnerDrawDialogElement& data, const RECT& clientRect, const RECT& ownerRect)
{
	if (!data.CacheSurface || !DSurface::Alternate)
		return;

	const int width = clientRect.right + 1;
	const int height = clientRect.bottom + 1;
	if (width <= 0 || height <= 0)
		return;

	RectangleStruct destRect { ownerRect.left, ownerRect.top, width, height };
	RectangleStruct sourceRect { 0, 0, width, height };
	CopySurfacePart(DSurface::Alternate, destRect, data.CacheSurface, sourceRect);
	::InvalidateRect(hWnd, nullptr, FALSE);
}

static int SelectRadioSliceHeight(int controlHeight)
{
	return controlHeight >= 30 ? 30 : 24;
}

static void DrawRadioImage(OwnerDrawDialogElement& data, const RectangleStruct& controlRect, int selected)
{
	auto pImage = data.ControlImage;
	if (selected && data.StateImageSurface)
		pImage = data.StateImageSurface;

	if (!pImage)
		return;

	RectangleStruct sourceRect { 0, 0, controlRect.Width, controlRect.Height };
	CopySurfacePart(DSurface::Alternate, controlRect, pImage, sourceRect);
}

static void DrawRadioSlices(
	HWND hWnd,
	OwnerDrawDialogElement& data,
	const RECT& clientRect,
	const RECT& ownerRect,
	const RectangleStruct& controlRect,
	LONG windowStyle,
	int selected)
{
	char variant = selected ? 'd' : 'u';
	if (windowStyle & WS_DISABLED)
		variant = 'u';

	const int sliceHeight = SelectRadioSliceHeight(controlRect.Height);
	const int leftSliceWidth = 7;
	const int rightSliceWidth = 10;

	RestoreRadioCache(hWnd, data, clientRect, ownerRect);

	const int sliceY = controlRect.Y + (controlRect.Height - sliceHeight) / 2 + (selected ? 2 : 0);
	char filename[32] {};

	std::snprintf(filename, sizeof(filename), "b%c%c_li%d.pcx", variant, 'e', sliceHeight);
	if (auto pLeft = GetPCXSurface(filename))
	{
		RectangleStruct destRect { controlRect.X, sliceY, leftSliceWidth, sliceHeight };
		RectangleStruct sourceRect { 0, 0, leftSliceWidth, sliceHeight };
		CopySurfacePart(DSurface::Alternate, destRect, pLeft, sourceRect);
	}

	std::snprintf(filename, sizeof(filename), "b%c%c_mi%d.pcx", variant, 'e', sliceHeight);
	if (auto pMiddle = GetPCXSurface(filename))
	{
		RectangleStruct middleRect
		{
			controlRect.X + leftSliceWidth,
			sliceY,
			controlRect.Width - rightSliceWidth,
			pMiddle->GetHeight()
		};

		BlitTiledPCX(middleRect, DSurface::Alternate, pMiddle, 0, 0);
	}

	std::snprintf(filename, sizeof(filename), "b%c%c_ri%d.pcx", variant, 'e', sliceHeight);
	if (auto pRight = GetPCXSurface(filename))
	{
		const int rightHeight = pRight->GetHeight();
		RectangleStruct destRect
		{
			controlRect.X + controlRect.Width - rightSliceWidth,
			sliceY,
			rightSliceWidth,
			rightHeight
		};
		RectangleStruct sourceRect { 0, 0, rightSliceWidth, rightHeight };
		CopySurfacePart(DSurface::Alternate, destRect, pRight, sourceRect);
	}

	if (data.TextBuffer)
	{
		RECT textRect
		{
			controlRect.X,
			sliceY + 1,
			controlRect.X + controlRect.Width - 2,
			sliceY + controlRect.Height - 2
		};

		if (selected)
		{
			textRect.left += 2;
			textRect.top += 4;
		}

		OwnerDraw::DrawWideText(
			DSurface::Alternate,
			data.TextBuffer,
			&textRect,
			data.AsRadio().Font(),
			Phobos::UI::ColorTextRadio,
			RadioTextStyle,
			RadioTextAlign,
			0,
			0,
			0);
	}
}

static LRESULT PaintRadioCtrl(HWND hWnd, OwnerDrawDialogElement& data, LONG windowStyle)
{
	if (!DSurface::Alternate)
	{
		::ValidateRect(hWnd, nullptr);
		return 0;
	}

	RECT ownerRect {};
	RECT clientRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates() || !RenderDX::GetClientRectInRender(hWnd, &clientRect))
		::GetClientRect(hWnd, &clientRect);

	const int width = ownerRect.right - ownerRect.left;
	const int height = ownerRect.bottom - ownerRect.top;
	RectangleStruct controlRect { ownerRect.left, ownerRect.top, width, height };

	EnsureRadioCache(data, clientRect, ownerRect);

	const int selected = data.AsRadio().CheckState() & 1;
	if (data.ControlImage)
	{
		DrawRadioImage(data, controlRect, selected);
	}
	else
	{
		DrawRadioSlices(hWnd, data, clientRect, ownerRect, controlRect, windowStyle, selected);
	}

	if (windowStyle & WS_DISABLED)
		BlendFillRect(controlRect, DSurface::Alternate, 0, RadioDisabledOverlayAlpha);

	::ValidateRect(hWnd, nullptr);
	return 0;
}

LRESULT CALLBACK WWUI::OwnerDrawCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return forwardOriginal();

	auto& data = *pData;

	switch (message)
	{
	case WM_ACTIVATE:
	case WM_KILLFOCUS:
	case WM_MOUSEACTIVATE:
		return 0;

	case WM_PAINT:
		return PaintOwnerDrawButton(hWnd, data, ::GetWindowLongA(hWnd, GWL_STYLE));

	case WM_TIMER:
		data.AsButton().AlternateFrame() = !data.AsButton().AlternateFrame();
		::InvalidateRect(hWnd, nullptr, TRUE);
		return forwardOriginal();

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (data.SkipDraw)
			return 0;

		VocClass::PlayGlobal(RulesClass::Instance->GUIMainButtonSound, 0x2000, 1.0f);
		return forwardOriginal();

	case WW_BUTTON_SETANIMATED:
		if (lParam == 1)
		{
			if (!data.AsButton().TimerActive())
			{
				data.AsButton().TimerActive() = true;
				::SetTimer(hWnd, OwnerDrawButtonTimerId, OwnerDrawButtonTimerInterval, nullptr);
			}
		}
		else if (data.AsButton().TimerActive())
		{
			data.AsButton().TimerActive() = false;
			data.AsButton().AlternateFrame() = false;
			::KillTimer(hWnd, OwnerDrawButtonTimerId);
			::InvalidateRect(hWnd, nullptr, TRUE);
		}

		return forwardOriginal();

	default:
		return forwardOriginal();
	}
}

LRESULT CALLBACK WWUI::CheckboxCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return 0;

	auto& data = *pData;
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	switch (message)
	{
	case BM_GETCHECK:
		return data.AsCheckbox().CheckState();

	case BM_SETCHECK:
		data.AsCheckbox().CheckState() = static_cast<int>(wParam);
		::InvalidateRect(hWnd, nullptr, FALSE);
		return 0;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		::InvalidateRect(hWnd, nullptr, FALSE);
		return forwardOriginal();

	case WM_PAINT:
		if (!data.AsCheckbox().UseNativePaint())
			return PaintCheckboxCtrl(hWnd, data, ::GetWindowLongA(hWnd, GWL_STYLE));

		return forwardOriginal();

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (!IsInsideCheckboxArt(hWnd, lParam))
			return 0;

		data.AsCheckbox().CheckState() = data.AsCheckbox().CheckState() == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED;
		::InvalidateRect(hWnd, nullptr, FALSE);
		NotifyCheckboxClicked(hWnd, data.AsCheckbox().CheckState());
		return 0;

	case WW_INITDIALOG:
		data.AsCheckbox().CheckState() = static_cast<int>(
			CallSelectedHandler(pOriginalWndProc, hWnd, BM_GETCHECK, 0, 0));
		return forwardOriginal();

	case WW_CHECKBOX_ENABLEEXTENDEDART:
	{
		const bool enabled = lParam != 0;
		const bool oldArtVariant = data.AsCheckbox().ArtVariant();
		data.AsCheckbox().UseExtendedArt() = enabled;
		if (oldArtVariant != enabled)
			::InvalidateRect(hWnd, nullptr, FALSE);

		return forwardOriginal();
	}

	case WW_CHECKBOX_SETARTVARIANT:
	{
		const bool variant = lParam != 0;
		const bool oldArtVariant = data.AsCheckbox().ArtVariant();
		data.AsCheckbox().ArtVariant() = variant;
		if (oldArtVariant != variant)
			::InvalidateRect(hWnd, nullptr, FALSE);

		return forwardOriginal();
	}

	case WW_CHECKBOX_GETARTVARIANT:
		return data.AsCheckbox().ArtVariant();

	default:
		return forwardOriginal();
	}
}

LRESULT CALLBACK WWUI::RadioCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return 0;

	auto& data = *pData;
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);
	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	switch (message)
	{
	case BM_GETCHECK:
		return data.AsRadio().CheckState();

	case BM_SETCHECK:
		data.AsRadio().CheckState() = static_cast<int>(wParam);
		::InvalidateRect(hWnd, nullptr, TRUE);
		return forwardOriginal();

	case WM_PAINT:
		return PaintRadioCtrl(hWnd, data, ::GetWindowLongA(hWnd, GWL_STYLE));

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (data.AsRadio().CheckState())
			return 0;

		data.AsRadio().CheckState() = BST_CHECKED;
		::InvalidateRect(hWnd, nullptr, TRUE);

		if (RulesClass::Instance)
			VocClass::PlayGlobal(RulesClass::Instance->GenericClick, 0x2000, 1.0f);

		return forwardOriginal();

	case WM_LBUTTONUP:
		::LockWindowUpdate(::GetParent(hWnd));
		{
			const LRESULT result = forwardOriginal();
			::LockWindowUpdate(nullptr);
			return result;
		}

	case WW_INITDIALOG:
		data.AsRadio().CheckState() = static_cast<int>(
			CallSelectedHandler(pOriginalWndProc, hWnd, BM_GETCHECK, 0, 0));
		return forwardOriginal();

	default:
		return forwardOriginal();
	}
}
