#include "OwnerDraw.Internal.h"

WWWinData* FindOwnerDrawData(HWND hWnd)
{
	if (!OwnerDraw::Dialogs.size())
		return nullptr;

	return OwnerDraw::Dialogs.try_get(hWnd);
}

WNDPROC FindWindowProc(OwnerDraw::HwndProcDict& procs, HWND hWnd)
{
	if (!procs.size())
		return nullptr;

	if (const auto pProc = procs.try_get(hWnd))
		return *pProc;

	return nullptr;
}

static WNDPROC GetComboDropWindowProc()
{
	return reinterpret_cast<WNDPROC>(0x60D540);
}

bool IsEmpty(const WideWstring& text)
{
	return text.GetLength() == 0;
}

const wchar_t* GetWideTextBuffer(const WideWstring& text)
{
	return text.Buffer ? text.Buffer : L"";
}

static WideWstring QueryTooltipText(HWND parentHwnd, HWND controlHwnd, LPARAM hitCode)
{
	OwnerDrawTooltipRequest request;
	request.ControlHwnd = controlHwnd;
	request.HitCode = hitCode;

	::SendMessageA(parentHwnd, WW_GETTOOLTIPTEXT, 0, reinterpret_cast<LPARAM>(&request));

	return request.Text;
}

static bool IsTooltipTextCurrent(HWND tooltipHwnd, const WideWstring& text)
{
	const auto pTooltipData = FindOwnerDrawData(tooltipHwnd);
	if (!pTooltipData)
		return false;

	const wchar_t* currentText = pTooltipData->TextBuffer ? pTooltipData->TextBuffer : L"";
	return std::wcscmp(currentText, GetWideTextBuffer(text)) == 0;
}

static void SetTooltipTextIfChanged(HWND tooltipHwnd, const WideWstring& text)
{
	if (IsTooltipTextCurrent(tooltipHwnd, text))
		return;

	::SendMessageA(tooltipHwnd, WW_SETTEXTW, 0, reinterpret_cast<LPARAM>(GetWideTextBuffer(text)));
}

static std::vector<OwnerDrawWindowMessageKey>& ActiveWindowMessages()
{
	static std::vector<OwnerDrawWindowMessageKey> messages;
	return messages;
}

static bool AllowsRecursiveMessage(UINT message)
{
	return message == WM_COMMAND
		|| message == WM_SYSKEYDOWN
		|| message == WM_SYSKEYUP
		|| message == WM_SYSCOMMAND
		|| message == WM_SYSCHAR;
}

class WindowMessageGuardScope
{
public:
	WindowMessageGuardScope(HWND hWnd, UINT message) :
		Key { message, hWnd }
	{
	}

	bool Enter()
	{
		auto& messages = ActiveWindowMessages();
		const auto it = std::find(messages.begin(), messages.end(), this->Key);

		if (it != messages.end())
		{
			if (!AllowsRecursiveMessage(this->Key.Message))
				return false;

			messages.erase(it);
		}

		messages.push_back(this->Key);
		this->Active = true;
		return true;
	}

	void Release()
	{
		if (!this->Active)
			return;

		auto& messages = ActiveWindowMessages();
		const auto it = std::find(messages.begin(), messages.end(), this->Key);
		if (it != messages.end())
			messages.erase(it);

		this->Active = false;
	}

	~WindowMessageGuardScope()
	{
		this->Release();
	}

private:
	OwnerDrawWindowMessageKey Key;
	bool Active { false };
};

static HWND GetActiveWindowStackTop()
{
	const int count = OwnerDraw::ActiveWindowStackCount;
	if (count <= 0 || !OwnerDraw::ActiveWindowStack)
		return nullptr;

	return OwnerDraw::ActiveWindowStack[count - 1];
}

static bool IsOwnerDrawDialogRoot(HWND hWnd)
{
	return hWnd
		&& ::IsWindow(hWnd)
		&& ::GetWindowLongA(hWnd, DialogProcWindowLongIndex);
}

bool WWUI::HasActiveOwnerDrawDialog()
{
	if (OwnerDraw::ActiveWindowStackCount > 0 && OwnerDraw::ActiveWindowStack)
	{
		for (int i = 0; i < OwnerDraw::ActiveWindowStackCount; ++i)
		{
			if (IsOwnerDrawDialogRoot(OwnerDraw::ActiveWindowStack[i]))
				return true;
		}
	}

	if (!OwnerDraw::Dialogs.size())
		return false;

	for (auto it = OwnerDraw::Dialogs.begin(); it != OwnerDraw::Dialogs.end(); ++it)
	{
		if (IsOwnerDrawDialogRoot(it->Key))
			return true;
	}

	return false;
}

static void ResizeActiveWindowStack(int capacity)
{
	if (capacity < 10)
		capacity = 10;

	auto pItems = static_cast<HWND*>(YRMemory::Allocate(sizeof(HWND) * capacity));
	std::memset(pItems, 0, sizeof(HWND) * capacity);

	const int copyCount = std::min(OwnerDraw::ActiveWindowStackCount, capacity);
	if (OwnerDraw::ActiveWindowStack && copyCount > 0)
		std::memcpy(pItems, OwnerDraw::ActiveWindowStack, sizeof(HWND) * copyCount);

	if (OwnerDraw::ActiveWindowStack)
		YRMemory::Deallocate(OwnerDraw::ActiveWindowStack);

	OwnerDraw::ActiveWindowStack = pItems;
	OwnerDraw::ActiveWindowStackCapacity = capacity;

	if (OwnerDraw::ActiveWindowStackCount > capacity)
		OwnerDraw::ActiveWindowStackCount = capacity;
}

static void EnsureActiveWindowStackCapacity(int required)
{
	if (required <= OwnerDraw::ActiveWindowStackCapacity)
		return;

	int capacity = OwnerDraw::ActiveWindowStackCapacity * 2;
	if (capacity < required)
		capacity = required;

	ResizeActiveWindowStack(capacity);
}

static void MaybeShrinkActiveWindowStack()
{
	const int capacity = OwnerDraw::ActiveWindowStackCapacity;
	const int count = OwnerDraw::ActiveWindowStackCount;

	if (capacity <= 10 || count * 3 > capacity)
		return;

	ResizeActiveWindowStack(std::max(capacity / 2, 10));
}

static void RemoveActiveWindow(HWND hWnd)
{
	for (int index = 0; index < OwnerDraw::ActiveWindowStackCount; )
	{
		if (OwnerDraw::ActiveWindowStack[index] != hWnd)
		{
			++index;
			continue;
		}

		const int last = OwnerDraw::ActiveWindowStackCount - 1;
		if (index < last)
		{
			std::memmove(
				&OwnerDraw::ActiveWindowStack[index],
				&OwnerDraw::ActiveWindowStack[index + 1],
				sizeof(HWND) * (last - index));
		}

		OwnerDraw::ActiveWindowStackCount = last;
		MaybeShrinkActiveWindowStack();
	}
}

static LRESULT BringOwnerDrawWindowToTop(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const HWND previousTop = GetActiveWindowStackTop();
	const HWND target = wParam ? reinterpret_cast<HWND>(wParam) : hWnd;

	RemoveActiveWindow(target);

	if (lParam)
	{
		EnsureActiveWindowStackCapacity(OwnerDraw::ActiveWindowStackCount + 1);
		OwnerDraw::ActiveWindowStack[OwnerDraw::ActiveWindowStackCount++] = target;

		OwnerDraw::AboutToCallSetWindowPos = 1;
		::SetWindowPos(target, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		OwnerDraw::AboutToCallSetWindowPos = 0;
	}

	return reinterpret_cast<LRESULT>(previousTop);
}

static bool IsActiveWindowMessageBlocked(HWND hWnd, UINT message)
{
	if (OwnerDraw::ActiveWindowStackCount <= 0)
		return false;

	const HWND activeTop = GetActiveWindowStackTop();

	bool belongsToActiveWindow = ::GetParent(hWnd) == nullptr;
	if (::GetWindowLongA(hWnd, GWL_ID) <= 0)
		belongsToActiveWindow = true;

	for (HWND walker = hWnd; walker; walker = ::GetParent(walker))
	{
		if (walker == activeTop)
		{
			belongsToActiveWindow = true;
			break;
		}
	}

	bool allowedOutsideActiveWindow = false;
	if (message < WM_MOUSEFIRST || message > WM_MBUTTONDBLCLK)
		allowedOutsideActiveWindow = true;
	if (message >= WM_NCMOUSEMOVE && message <= WM_NCMBUTTONDBLCLK)
		allowedOutsideActiveWindow = false;
	if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
		allowedOutsideActiveWindow = false;
	if (message == WM_SYSKEYUP || message == WM_SYSKEYDOWN || message == WM_SYSCOMMAND || message == WM_SYSCHAR)
		allowedOutsideActiveWindow = true;
	if (message == WW_UNKNOWN49B || message == WM_TIMER || message == WW_LB_GETCELLTEXT)
		allowedOutsideActiveWindow = false;

	return !belongsToActiveWindow && !allowedOutsideActiveWindow;
}

static bool HandleWindowPosChanging(HWND hWnd, LPARAM lParam, LRESULT& result)
{
	if (OwnerDraw::AboutToCallSetWindowPos == 1 || OwnerDraw::ActiveWindowStackCount <= 0)
		return false;

	int index = -1;
	for (int i = 0; i < OwnerDraw::ActiveWindowStackCount; ++i)
	{
		if (OwnerDraw::ActiveWindowStack[i] == hWnd)
		{
			index = i;
			break;
		}
	}

	if (index < 0)
		return false;

	auto pPosition = reinterpret_cast<WINDOWPOS*>(lParam);
	if (!pPosition)
	{
		result = 0;
		return true;
	}

	if (index == OwnerDraw::ActiveWindowStackCount - 1
		&& pPosition->hwndInsertAfter
		&& !(pPosition->flags & SWP_NOZORDER))
	{
		pPosition->hwndInsertAfter = nullptr;
	}
	else
	{
		pPosition->flags |= SWP_NOZORDER | SWP_NOOWNERZORDER;
	}

	::InvalidateRect(hWnd, nullptr, FALSE);
	result = 0;
	return true;
}

using ScalarDeletingDestructor = void* (__thiscall*)(Surface*, unsigned int);
using GenericDeletingDestructor = void* (__thiscall*)(void*, unsigned int);

void DeleteSurfaceObject(Surface*& pSurface)
{
	if (!pSurface)
		return;

	const auto pDestructor = (*reinterpret_cast<ScalarDeletingDestructor**>(pSurface))[0];
	pDestructor(pSurface, 1);
	pSurface = nullptr;
}

void ResetOwnerDrawCachedSurface(OwnerDrawDialogElement& data)
{
	if (!data.CacheSurface)
		return;

	DeleteSurfaceObject(data.CacheSurface);
	if (OwnerDraw::CachedSurfaceCount > 0)
		--OwnerDraw::CachedSurfaceCount;
}

void ResetOwnerDrawCachedSurfaceTree(HWND rootHwnd)
{
	if (!rootHwnd)
	{
		for (auto it = OwnerDraw::Dialogs.begin(); it != OwnerDraw::Dialogs.end(); ++it)
		{
			ResetOwnerDrawCachedSurface(it->Value);
			if (::IsWindow(it->Key))
				::InvalidateRect(it->Key, nullptr, FALSE);
		}

		return;
	}

	if (auto pData = FindOwnerDrawData(rootHwnd))
		ResetOwnerDrawCachedSurface(*pData);

	::InvalidateRect(rootHwnd, nullptr, FALSE);

	for (HWND child = ::GetWindow(rootHwnd, GW_CHILD); child; child = ::GetWindow(child, GW_HWNDNEXT))
		ResetOwnerDrawCachedSurfaceTree(child);
}

void DeleteUnknownGameObject(void*& pObject)
{
	if (!pObject)
		return;

	const auto pDestructor = (*reinterpret_cast<GenericDeletingDestructor**>(pObject))[0];
	pDestructor(pObject, 1);
	pObject = nullptr;
}

static void RestoreAndClearTooltipIfNeeded(HWND hWnd, UINT message)
{
	auto& tooltip = OwnerDraw::TooltipBlitState;

	if (hWnd != tooltip.OwnerHwnd || !tooltip.Active)
		return;

	if (message != WM_NCDESTROY && message != WM_SHOWWINDOW && message != WM_KILLFOCUS)
		return;

	const bool restoredBackground = !tooltip.BackgroundRestored && tooltip.BackingSurface;
	if (!tooltip.BackgroundRestored)
	{
		OwnerDraw::RestoreTooltipBackground();
		if (restoredBackground)
			RenderDX::UpdateScreen(DSurface::Primary);
	}

	DeleteSurfaceObject(tooltip.BackingSurface);
	tooltip.Active = 0;
	tooltip.BackgroundRestored = 0;
}

void InsetSurfaceRect(RectangleStruct& rect, int x, int y)
{
	rect.X += x;
	rect.Y += y;
	rect.Width -= 2 * x;
	rect.Height -= 2 * y;
}

static void CopyAlternateToPrimary(const RectangleStruct& destRect, const RectangleStruct& sourceRect)
{
	if (!DSurface::Primary || !DSurface::Alternate)
		return;

	DSurface::Primary->Lock(0, 0);
	DSurface::Alternate->Lock(0, 0);
	DSurface::Primary->CopyFromPart(
		const_cast<RectangleStruct*>(&destRect),
		DSurface::Alternate,
		const_cast<RectangleStruct*>(&sourceRect),
		false,
		true);
	DSurface::Alternate->Unlock();
	DSurface::Primary->Unlock();
}

int ConvertRGBToSurfaceColor(COLORREF color)
{
	if (color == static_cast<COLORREF>(-1))
		return -1;

	return Drawing::RGB_To_Int(GetRValue(color), GetGValue(color), GetBValue(color));
}

COLORREF AverageColor(COLORREF first, COLORREF second)
{
	return RGB(
		(GetRValue(first) + GetRValue(second)) / 2,
		(GetGValue(first) + GetGValue(second)) / 2,
		(GetBValue(first) + GetBValue(second)) / 2);
}

static WORD BlendSurfacePixel(WORD destination, WORD source, int alpha)
{
	const int inverseAlpha = 255 - alpha;
	const WORD redMask = OwnerDraw::ColorShiftRed;
	const WORD greenMask = OwnerDraw::ColorShiftGreen;
	const WORD blueMask = OwnerDraw::ColorShiftBlue;

	return static_cast<WORD>(
		((((source & redMask) * alpha + (destination & redMask) * inverseAlpha) >> 8) & redMask)
		| ((((source & greenMask) * alpha + (destination & greenMask) * inverseAlpha) >> 8) & greenMask)
		| ((((source & blueMask) * alpha + (destination & blueMask) * inverseAlpha) >> 8) & blueMask));
}

WORD BlendSurfacePixelTowardMasks(WORD destination, int alpha)
{
	const int inverseAlpha = 255 - alpha;
	const WORD redMask = OwnerDraw::ColorShiftRed;
	const WORD greenMask = OwnerDraw::ColorShiftGreen;
	const WORD blueMask = OwnerDraw::ColorShiftBlue;

	return static_cast<WORD>(
		(((redMask * alpha + (destination & redMask) * inverseAlpha) >> 8) & redMask)
		| (((greenMask * alpha + (destination & greenMask) * inverseAlpha) >> 8) & greenMask)
		| (((blueMask * alpha + (destination & blueMask) * inverseAlpha) >> 8) & blueMask));
}

void BlendFillRect(const RectangleStruct& rect, Surface* pSurface, WORD color, int alpha)
{
	if (!pSurface || alpha <= 0 || rect.Width <= 0 || rect.Height <= 0)
		return;

	auto pPixels = static_cast<WORD*>(pSurface->Lock(0, 0));
	if (!pPixels)
		return;

	const int pitch = pSurface->GetPitch() / 2;
	const int left = std::max(rect.X, 0);
	const int top = std::max(rect.Y, 0);
	const int right = std::min(rect.X + rect.Width, pSurface->GetWidth());
	const int bottom = std::min(rect.Y + rect.Height, pSurface->GetHeight());

	for (int y = top; y < bottom; ++y)
	{
		auto pLine = &pPixels[y * pitch + left];
		for (int x = left; x < right; ++x)
		{
			*pLine = BlendSurfacePixel(*pLine, color, alpha);
			++pLine;
		}
	}

	pSurface->Unlock();
}

void BlendGradientRect(const RectangleStruct& rect, Surface* pSurface, WORD color, int widthScale)
{
	if (!pSurface || rect.Width <= 0 || rect.Height <= 0)
		return;

	int fillWidth = static_cast<int>((static_cast<long long>(rect.Width) * widthScale) >> 16);
	if (fillWidth < 0)
		return;

	if (!fillWidth)
		fillWidth = 1;

	auto pPixels = static_cast<WORD*>(pSurface->Lock(0, 0));
	if (!pPixels)
		return;

	const int pitch = pSurface->GetPitch() / 2;
	const int quarterHeight = rect.Height / 4;
	bool useGradient = true;

	for (int y = 0; y < rect.Height; ++y)
	{
		if (y == 3 * quarterHeight)
			useGradient = true;

		if (y == quarterHeight)
			useGradient = false;

		auto pLine = &pPixels[(rect.Y + y) * pitch + rect.X];
		int alphaNumerator = 255;

		for (int x = 0; x < fillWidth; ++x)
		{
			if (useGradient)
			{
				const int alpha = (alphaNumerator / rect.Width) & 0xFF;
				*pLine = BlendSurfacePixel(*pLine, color, alpha);
			}
			else
			{
				*pLine = color;
			}

			++pLine;
			alphaNumerator += 255;
		}
	}

	pSurface->Unlock();
}

bool DrawAlphaLine(DSurface* pSurface, Point2D start, Point2D end, WORD color, BYTE alpha)
{
	if (!pSurface)
		return false;

	if (start.Y == end.Y)
	{
		if (start.X > end.X)
			std::swap(start.X, end.X);

		auto pPixels = static_cast<WORD*>(pSurface->Lock(start.X, start.Y));
		if (!pPixels)
			return false;

		for (int x = start.X; x <= end.X; ++x)
		{
			*pPixels = BlendSurfacePixel(*pPixels, color, alpha);
			++pPixels;
		}

		pSurface->Unlock();
		return true;
	}

	if (start.X == end.X)
	{
		const int step = start.Y <= end.Y ? pSurface->GetPitch() : -pSurface->GetPitch();
		const int count = std::abs(end.Y - start.Y) + 1;
		auto pPixelBytes = static_cast<BYTE*>(pSurface->Lock(start.X, start.Y));
		if (!pPixelBytes)
			return false;

		for (int i = 0; i < count; ++i)
		{
			auto pPixel = reinterpret_cast<WORD*>(pPixelBytes);
			*pPixel = BlendSurfacePixel(*pPixel, color, alpha);
			pPixelBytes += step;
		}

		pSurface->Unlock();
		return true;
	}

	return false;
}

bool DrawAlphaBeveledRect(
	DSurface* pSurface,
	const RectangleStruct& rect,
	bool raised,
	int thickness,
	BYTE leftAlpha,
	BYTE topAlpha,
	BYTE rightAlpha,
	BYTE bottomAlpha)
{
	if (!pSurface || thickness <= 0)
		return false;

	const WORD topLeftColor = raised ? 0xFFFF : 0;
	const WORD bottomRightColor = raised ? 0 : 0xFFFF;
	bool result = raised;

	for (int layer = 0; layer < thickness; ++layer)
	{
		Point2D start { rect.X + layer, rect.Y + layer };
		Point2D end { rect.X + rect.Width - layer - 2, rect.Y + layer };
		result = DrawAlphaLine(pSurface, start, end, topLeftColor, topAlpha);

		start = { rect.X + layer, rect.Y + rect.Height - layer - 1 };
		end = { rect.X + rect.Width - layer - 1, start.Y };
		result = DrawAlphaLine(pSurface, start, end, bottomRightColor, bottomAlpha);

		start = { rect.X + layer, rect.Y + layer + 1 };
		end = { start.X, rect.Y + rect.Height - layer - 1 };
		result = DrawAlphaLine(pSurface, start, end, topLeftColor, leftAlpha);

		start = { rect.X + rect.Width - layer - 1, rect.Y + layer };
		end = { start.X, rect.Y + rect.Height - layer - 2 };
		result = DrawAlphaLine(pSurface, start, end, bottomRightColor, rightAlpha);
	}

	return result;
}

int DrawBeveledBorder(Surface* pSurface, const RectangleStruct& rect, int thickness, int color)
{
	if (!pSurface || thickness <= 0)
		return thickness - 1;

	int lineColor = color;
	if (lineColor == -1 && OwnerDraw::DefaultBorderColor != static_cast<COLORREF>(-1))
		lineColor = ConvertRGBToSurfaceColor(OwnerDraw::DefaultBorderColor);

	const int lightColor = ConvertRGBToSurfaceColor(OwnerDraw::BevelLightColor);
	const int shadowColor = ConvertRGBToSurfaceColor(OwnerDraw::BevelShadowColor);
	const int averageColor = ConvertRGBToSurfaceColor(AverageColor(OwnerDraw::BevelLightColor, OwnerDraw::BevelShadowColor));

	const int leftBase = rect.X - thickness;
	const int topBase = rect.Y - thickness;
	const int rightBase = leftBase + rect.Width + 2 * thickness - 1;
	const int bottomBase = topBase + rect.Height + 2 * thickness - 1;

	for (int layer = 0; layer < thickness; ++layer)
	{
		int topLeftColor = lineColor;
		int bottomRightColor = lineColor;

		if (thickness == 2)
		{
			topLeftColor = layer == 0 ? lightColor : shadowColor;
			bottomRightColor = layer == 0 ? shadowColor : lightColor;
		}

		const int left = leftBase + layer;
		const int top = topBase + layer;
		const int right = rightBase - layer;
		const int bottom = bottomBase - layer;

		Point2D start { left, top };
		Point2D end { right - 1, top };
		pSurface->DrawLine(&start, &end, topLeftColor);

		start = { left, top + 1 };
		end = { left, bottom };
		pSurface->DrawLine(&start, &end, topLeftColor);

		start = { left, bottom };
		end = { right, bottom };
		pSurface->DrawLine(&start, &end, bottomRightColor);

		start = { right, top };
		end = { right, bottom - 1 };
		pSurface->DrawLine(&start, &end, bottomRightColor);

		if (thickness == 2)
		{
			Point2D corner { right, top };
			pSurface->SetPixel(&corner, averageColor);
			corner = { left, bottom };
			pSurface->SetPixel(&corner, averageColor);
		}
	}

	return 0;
}

BSurface* GetPCXSurface(const char* pFilename)
{
	return PCX::Instance.GetSurface(pFilename, nullptr);
}

bool BlitTiledPCX(const RectangleStruct& rect, Surface* pDestination, Surface* pSource, int offsetX, int offsetY)
{
	if (!pDestination || !pSource || rect.Width <= 0 || rect.Height <= 0)
		return false;

	auto pDestPixels = static_cast<WORD*>(pDestination->Lock(0, 0));
	if (!pDestPixels)
		return false;

	auto pSourcePixels = static_cast<WORD*>(pSource->Lock(0, 0));
	if (!pSourcePixels)
	{
		pDestination->Unlock();
		return false;
	}

	const int destPitch = pDestination->GetPitch() / 2;
	const int sourcePitch = pSource->GetPitch() / 2;
	const int sourceWidth = pSource->GetWidth();
	const int sourceHeight = pSource->GetHeight();
	const int sourceStartX = offsetX + std::max((sourceWidth - rect.Width) / 2, 0);
	int sourceY = offsetY + std::max((sourceHeight - rect.Height) / 2, 0);

	for (int y = 0; y < rect.Height; ++y)
	{
		int sourceX = sourceStartX;
		auto pDestLine = &pDestPixels[rect.X + destPitch * (rect.Y + y)];
		for (int x = 0; x < rect.Width; ++x)
		{
			*pDestLine++ = pSourcePixels[sourcePitch * (sourceY % sourceHeight) + (sourceX % sourceWidth)];
			++sourceX;
		}

		++sourceY;
	}

	pSource->Unlock();
	pDestination->Unlock();
	return true;
}

bool CopySurfacePart(Surface* pDestination, const RectangleStruct& toRect, Surface* pSource, const RectangleStruct& fromRect)
{
	if (!pDestination || !pSource)
		return false;

	auto dest = toRect;
	auto source = fromRect;
	return pDestination->CopyFromPart(&dest, pSource, &source, false, true);
}

void DrawPCXCopy(Surface* pDestination, const RectangleStruct& rect, BSurface* pPCX)
{
	if (!pPCX)
		return;

	RectangleStruct sourceRect { 0, 0, pPCX->GetWidth(), pPCX->GetHeight() };
	RectangleStruct destRect { rect.X, rect.Y, pPCX->GetWidth(), pPCX->GetHeight() };
	CopySurfacePart(pDestination, destRect, pPCX, sourceRect);
}

static void NotifyChildren(HWND hWnd, UINT message)
{
	OwnerDrawHWNDVector children {};
	::EnumChildWindows(hWnd, OwnerDraw::CollectChildHwndProc, reinterpret_cast<LPARAM>(&children));

	for (int i = 0; i < children.Count; ++i)
		::SendMessageA(children.Items[i], message, 0, 0);

	if (children.Items)
		YRMemory::Deallocate(children.Items);
}

static void RepaintChildWindows(HWND hWnd, HWND ownerHwnd, const RECT& ownerDrawRect)
{
	OwnerDrawHWNDVector children {};
	::EnumChildWindows(hWnd, OwnerDraw::CollectChildHwndProc, reinterpret_cast<LPARAM>(&children));

	HWND comboDropHwnd = nullptr;

	for (int i = 0; i < children.Count; ++i)
	{
		const HWND childHwnd = children.Items[i];
		const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, childHwnd);

		if (pOriginalWndProc == GetComboDropWindowProc())
		{
			comboDropHwnd = childHwnd;
			continue;
		}

		::InvalidateRect(childHwnd, nullptr, FALSE);
		::UpdateWindow(childHwnd);
	}

	if (comboDropHwnd)
	{
		::InvalidateRect(comboDropHwnd, nullptr, FALSE);
		::UpdateWindow(comboDropHwnd);
	}
	else if (OwnerDraw::ComboDropActiveDropHwnd && OwnerDraw::ComboDropActiveParentHwnd == ownerHwnd)
	{
		RECT dropRect {};
		OwnerDraw::GetRectangle(OwnerDraw::ComboDropActiveDropHwnd, &dropRect);

		RECT intersect {};
		if (::IntersectRect(&intersect, &ownerDrawRect, &dropRect))
		{
			::InvalidateRect(OwnerDraw::ComboDropActiveDropHwnd, nullptr, FALSE);
			::UpdateWindow(OwnerDraw::ComboDropActiveDropHwnd);
		}
	}

	if (children.Items)
		YRMemory::Deallocate(children.Items);
}

static void RepaintOverlappingPreviousSibling(HWND hWnd, HWND ownerHwnd, const RECT& ownerDrawRect, int paintCopyMode)
{
	if (paintCopyMode <= 0 || (hWnd != ownerHwnd && OwnerDraw::PaintDepth != 1))
		return;

	for (HWND sibling = ownerHwnd; sibling; )
	{
		sibling = ::GetWindow(sibling, GW_HWNDPREV);
		if (!sibling)
			break;

		if (!::GetWindowLongA(sibling, DialogProcWindowLongIndex))
			continue;

		RECT siblingRect {};
		OwnerDraw::GetRectangle(sibling, &siblingRect);

		RECT intersect {};
		if (::IntersectRect(&intersect, &ownerDrawRect, &siblingRect))
		{
			::InvalidateRect(sibling, nullptr, FALSE);
			::UpdateWindow(sibling);
			break;
		}
	}
}

static void RestoreTooltipBackgroundForPaint(const RECT& ownerDrawRect, bool& redrawTooltip)
{
	auto& tooltip = OwnerDraw::TooltipBlitState;

	const RECT tooltipRect
	{
		tooltip.Rect.X,
		tooltip.Rect.Y,
		tooltip.Rect.X + tooltip.Rect.Width + 1,
		tooltip.Rect.Y + tooltip.Rect.Height + 1
	};

	RECT intersect {};
	if (OwnerDraw::PaintDepth != 1
		|| !::IntersectRect(&intersect, &ownerDrawRect, &tooltipRect)
		|| !tooltip.Active
		|| tooltip.BackgroundRestored
		|| !tooltip.BackingSurface
		|| !DSurface::Primary)
	{
		return;
	}

	RectangleStruct targetRect { tooltip.Rect.X, tooltip.Rect.Y, tooltip.Rect.Width, tooltip.Rect.Height };
	RectangleStruct sourceRect { 0, 0, tooltip.Rect.Width, tooltip.Rect.Height };
	DSurface::Primary->CopyFromPart(&targetRect, tooltip.BackingSurface, &sourceRect, false, true);
	RenderDX::UpdateScreen(DSurface::Primary);
	tooltip.BackgroundRestored = 1;
	redrawTooltip = true;
}

static void AccumulatePaintBounds(const RECT& ownerDrawRect)
{
	if (OwnerDraw::PaintLeft >= ownerDrawRect.left)
		OwnerDraw::PaintLeft = ownerDrawRect.left;
	if (OwnerDraw::PaintTop >= ownerDrawRect.top)
		OwnerDraw::PaintTop = ownerDrawRect.top;
	if (OwnerDraw::PaintRight <= ownerDrawRect.right)
		OwnerDraw::PaintRight = ownerDrawRect.right;
	if (OwnerDraw::PaintBottom <= ownerDrawRect.bottom)
		OwnerDraw::PaintBottom = ownerDrawRect.bottom;
}

struct PaintRoot
{
	HWND ProbeHwnd {};
	HWND OwnerHwnd {};
	OwnerDrawDialogElement* Data {};
};

static PaintRoot FindPaintRoot(HWND hWnd)
{
	HWND ownerHwnd = hWnd;
	HWND probeHwnd = hWnd;

	while (probeHwnd)
	{
		ownerHwnd = probeHwnd;
		if (::GetWindowLongA(probeHwnd, DialogProcWindowLongIndex))
			break;

		probeHwnd = ::GetParent(probeHwnd);
	}

	return PaintRoot
	{
		probeHwnd,
		ownerHwnd,
		probeHwnd ? FindOwnerDrawData(ownerHwnd) : nullptr
	};
}

LRESULT CallSelectedHandler(WNDPROC pSelectedWndProc, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!pSelectedWndProc)
		return 0;

	return ::CallWindowProcA(pSelectedWndProc, hWnd, message, wParam, lParam);
}

static LRESULT DispatchPaintMessage(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam,
	WNDPROC pSelectedWndProc,
	const RECT& ownerDrawRect,
	int& paintCopyMode,
	bool& redrawTooltip)
{
	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return CallSelectedHandler(pSelectedWndProc, hWnd, message, wParam, lParam);

	if (pData->NeedsControlImage)
	{
		::ValidateRect(hWnd, nullptr);
		return CallSelectedHandler(pSelectedWndProc, hWnd, WW_SCROLLBAR_UPDATETHUMB, wParam, lParam);
	}

	RestoreTooltipBackgroundForPaint(ownerDrawRect, redrawTooltip);
	AccumulatePaintBounds(ownerDrawRect);

	const auto root = FindPaintRoot(hWnd);
	int rootPaintState = 0;

	if (root.ProbeHwnd == hWnd)
	{
		if (!root.Data)
		{
			::ValidateRect(hWnd, nullptr);
			return 1;
		}

		rootPaintState = root.Data->Extra[PaintStateExtraIndex] < 1 ? 1 : root.Data->Extra[PaintStateExtraIndex];
	}
	else
	{
		if (!root.Data)
		{
			::ValidateRect(hWnd, nullptr);
			RepaintOverlappingPreviousSibling(hWnd, root.OwnerHwnd, ownerDrawRect, paintCopyMode);
			return 1;
		}

		rootPaintState = root.Data->Extra[PaintStateExtraIndex];
	}

	paintCopyMode = rootPaintState;
	LRESULT result = 1;

	if (rootPaintState < 1)
	{
		::ValidateRect(hWnd, nullptr);
	}
	else
	{
		result = CallSelectedHandler(pSelectedWndProc, hWnd, message, wParam, lParam);
		if (root.Data)
			root.Data->Extra[PaintStateExtraIndex] = rootPaintState;

		RepaintChildWindows(hWnd, root.OwnerHwnd, ownerDrawRect);
	}

	RepaintOverlappingPreviousSibling(hWnd, root.OwnerHwnd, ownerDrawRect, rootPaintState);
	return result;
}

static void FinishPaint(HWND hWnd, OwnerDrawDialogElement* pData, int paintCopyMode, int windowOffsetX, int windowOffsetY)
{
	if (!pData)
		return;

	if (::GetWindowLongA(hWnd, DialogProcWindowLongIndex) && OwnerDraw::PaintDepth > 1)
		pData->Extra[PaintStateExtraIndex] = 2;

	if (--OwnerDraw::PaintDepth != 0)
		return;

	if (!pData->NeedsControlImage && paintCopyMode >= 1 && !OwnerDraw::IsWebBrowserVisible())
	{
		const int paintLeft = OwnerDraw::PaintLeft;
		const int paintTop = OwnerDraw::PaintTop;
		const int paintWidth = OwnerDraw::PaintRight - OwnerDraw::PaintLeft;
		const int paintHeight = OwnerDraw::PaintBottom - OwnerDraw::PaintTop;

		if (paintWidth > 0 && paintHeight > 0)
		{
			RectangleStruct sourceRect { paintLeft, paintTop, paintWidth, paintHeight };
			RectangleStruct destRect { paintLeft + windowOffsetX, paintTop + windowOffsetY, paintWidth, paintHeight };

			if (pData->Extra[PaintStateExtraIndex] == 1)
			{
				pData->Extra[PaintStateExtraIndex] = 2;

				if (::GetWindowLongA(hWnd, DialogProcWindowLongIndex) && OwnerDraw::RunOpenAnimationIfNeeded(hWnd))
					pData->Extra[PaintStateExtraIndex] = 3;

				CopyAlternateToPrimary(destRect, sourceRect);
				NotifyChildren(hWnd, WW_EDIT_RESTOREFOCUS);
			}
			else
			{
				char className[0x80] {};
				::GetClassNameA(hWnd, className, sizeof(className));

				if (!std::strcmp(className, "ComboBox"))
				{
					InsetSurfaceRect(sourceRect, -1, -1);
					InsetSurfaceRect(destRect, -1, -1);
				}

				CopyAlternateToPrimary(destRect, sourceRect);
			}
		}
	}

	OwnerDraw::PaintRight = 0;
	OwnerDraw::PaintLeft = 0xFFFFFF;
	OwnerDraw::PaintTop = 0xFFFFFF;
	OwnerDraw::PaintBottom = 0;
}

static void ReleaseElementText(OwnerDrawDialogElement& data)
{
	if (!data.TextBuffer)
		return;

	YRMemory::Deallocate(data.TextBuffer);
	data.TextBuffer = nullptr;
}

static void SetElementTextA(OwnerDrawDialogElement& data, const char* pText)
{
	ReleaseElementText(data);

	if (pText && *pText)
	{
		const auto length = std::strlen(pText);
		data.TextBuffer = static_cast<wchar_t*>(YRMemory::Allocate(sizeof(wchar_t) * (length + 1)));
		std::swprintf(data.TextBuffer, length + 1, L"%hs", pText);
	}

	data.HasText = 1;
}

static bool SetElementTextW(OwnerDrawDialogElement& data, const wchar_t* pText)
{
	const bool changed = (!data.TextBuffer && pText)
		|| (data.TextBuffer && !pText)
		|| (data.TextBuffer && pText && std::wcscmp(data.TextBuffer, pText));

	ReleaseElementText(data);

	if (pText && *pText)
	{
		const auto length = std::wcslen(pText);
		data.TextBuffer = static_cast<wchar_t*>(YRMemory::Allocate(sizeof(wchar_t) * (length + 1)));
		std::wcscpy(data.TextBuffer, pText);
	}

	data.HasText = 0;
	return changed;
}

static void CopyTextA(const OwnerDrawDialogElement& data, WPARAM length, LPARAM lParam)
{
	if (!lParam)
		return;

	auto pBuffer = reinterpret_cast<char*>(lParam);
	*pBuffer = '\0';

	if (data.TextBuffer)
	{
		OwnerDraw::WideToCharString(pBuffer, data.TextBuffer, length);
		if (length)
			pBuffer[length - 1] = '\0';
	}
}

static void CopyTextW(const OwnerDrawDialogElement& data, WPARAM length, LPARAM lParam)
{
	if (!lParam)
		return;

	auto pBuffer = reinterpret_cast<wchar_t*>(lParam);
	*pBuffer = L'\0';

	if (data.TextBuffer)
	{
		std::wcsncpy(pBuffer, data.TextBuffer, length);
		if (length)
			pBuffer[length - 1] = L'\0';
	}
}

static bool IsNativeTextMessage(UINT message, LRESULT& result)
{
	switch (message)
	{
	case WM_SETTEXT:
		result = 0;
		return true;

	case WM_GETTEXT:
	case LB_GETTEXT:
		result = 0;
		return true;

	case CB_ADDSTRING:
	case CB_FINDSTRING:
	case CB_FINDSTRINGEXACT:
	case CB_SELECTSTRING:
	case CB_INSERTSTRING:
	case CB_GETLBTEXT:
	case LB_ADDSTRING:
	case LB_FINDSTRING:
	case LB_FINDSTRINGEXACT:
	case LB_SELECTSTRING:
	case LB_INSERTSTRING:
		result = -1;
		return true;

	default:
		return false;
	}
}

static bool HandleElementTextMessage(
	OwnerDrawDialogElement& data,
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam,
	LRESULT& result,
	bool& callSelectedHandler)
{
	switch (message)
	{
	case WW_SETHASTEXT:
		result = data.HasText == 0;
		callSelectedHandler = false;
		return true;

	case WW_GETTEXTA:
		CopyTextA(data, wParam, lParam);
		return true;

	case WW_GETTEXTW:
		CopyTextW(data, wParam, lParam);
		return true;

	case WW_SETUNKNOWNPROP50:
		data.AsNewEdit().RejectChars() = reinterpret_cast<wchar_t*>(lParam);
		return true;

	case WW_SETTEXTA:
		SetElementTextA(data, reinterpret_cast<const char*>(lParam));
		return true;

	case WW_SETUNKNOWNPROP30:
		result = data.AsNewEdit().AsciiOnly();
		data.AsNewEdit().AsciiOnly() = static_cast<int>(wParam);
		callSelectedHandler = false;
		return true;

	case WW_SETTEXTW:
	{
		const bool changed = SetElementTextW(data, reinterpret_cast<const wchar_t*>(lParam));
		if (changed && data.AsStatic().DrawMode() == WWUIStaticDrawMode::TypewriterText && data.AsStatic().AnimationRunning())
		{
			::KillTimer(hWnd, 0);
			data.AsStatic().AnimationRunning() = false;
			::SendMessageA(hWnd, WW_STATIC_REVEALTEXTS, 0, 0);
		}
		return true;
	}

	default:
		return false;
	}
}

static void UpdateTooltipTextOnMouseMove(HWND hWnd, LPARAM lParam)
{
	const HWND parentHwnd = ::GetParent(hWnd);
	const HWND tooltipHwnd = parentHwnd ? ::GetDlgItem(parentHwnd, OwnerDraw::TooltipText) : nullptr;
	if (!tooltipHwnd)
		return;

	WideWstring tooltipText;

	LPARAM pointParam = MAKELPARAM(LOWORD(lParam), HIWORD(lParam));
	if (!RenderDX::IsOwnerDrawUsingRawWindowCoordinates()
		&& FindWindowProc(OwnerDraw::DialogProcs, hWnd) == GetComboDropWindowProc())
	{
		pointParam = RenderDX::MouseLParamToRenderLocal(hWnd, lParam);
	}

	const auto hitCode = ::SendMessageA(hWnd, WW_QUERYTOOLTIPHIT, 0, pointParam);
	tooltipText = QueryTooltipText(parentHwnd, hWnd, hitCode);

	if (IsEmpty(tooltipText))
	{
		tooltipText = QueryTooltipText(parentHwnd, hWnd, -1);

		if (IsEmpty(tooltipText))
		{
			if (const auto label = OwnerDraw::GetTooltipStringLabel(parentHwnd, hWnd))
				tooltipText = StringTable::LoadString(label);
			else
				tooltipText = L"";
		}
	}

	SetTooltipTextIfChanged(tooltipHwnd, tooltipText);
}

static bool IsComboDropMousePointMessage(UINT message)
{
	return message >= WM_MOUSEFIRST && message <= WM_MBUTTONDBLCLK;
}

static LPARAM GetSelectedHandlerLParam(HWND hWnd, UINT message, LPARAM lParam, const OwnerDrawDialogElement* pData, WNDPROC pSelectedWndProc)
{
	if (pSelectedWndProc == GetComboDropWindowProc()
		&& IsComboDropMousePointMessage(message)
		&& !RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
	{
		return RenderDX::MouseLParamToRenderLocal(hWnd, lParam);
	}

	if (!pData || pData->DialogID != 148 || !::GetWindowLongA(hWnd, DialogProcWindowLongIndex))
		return lParam;

	if (message != WM_LBUTTONDOWN && message != WM_LBUTTONUP)
		return lParam;

	POINT point { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if (!::ClientToScreen(hWnd, &point))
		return lParam;

	return MAKELPARAM(static_cast<WORD>(point.x), static_cast<WORD>(point.y));
}

void CleanupDestroyedWindow(HWND hWnd)
{
	if (auto pData = FindOwnerDrawData(hWnd))
	{
		ResetOwnerDrawCachedSurface(*pData);
	}

	OwnerDraw::DialogProcs.erase(hWnd);
	OwnerDraw::Dialogs.erase(hWnd);
	OwnerDraw::SubclassProcs.erase(hWnd);

	const auto pUserData = reinterpret_cast<void*>(::GetWindowLongA(hWnd, GWL_USERDATA));
	if (pUserData)
		YRMemory::Deallocate(pUserData);

	::SetWindowLongA(hWnd, GWL_USERDATA, 0);
	SessionIpb::UnregisterHwnd(hWnd);
}

static bool GetRawWindowRectInRenderLayout(HWND hWnd, RECT& rect)
{
	RECT windowRect {};
	if (!::GetWindowRect(hWnd, &windowRect))
		return false;

	const HWND parentHwnd = ::GetParent(hWnd);
	const HWND coordinateHwnd = parentHwnd ? parentHwnd : Game::hWnd;

	POINT origin { 0, 0 };
	if (coordinateHwnd && !::ClientToScreen(coordinateHwnd, &origin))
		return false;

	rect.left = windowRect.left - origin.x;
	rect.top = windowRect.top - origin.y;
	rect.right = windowRect.right - origin.x;
	rect.bottom = windowRect.bottom - origin.y;
	return true;
}

static void ApplyRenderLayoutToWindowTree(HWND hWnd)
{
	RECT rect {};
	if (GetRawWindowRectInRenderLayout(hWnd, rect))
	{
		RenderDX::MoveWindowInRender(
			hWnd,
			rect.left,
			rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top,
			FALSE);
	}

	for (HWND child = ::GetWindow(hWnd, GW_CHILD); child; child = ::GetWindow(child, GW_HWNDNEXT))
		ApplyRenderLayoutToWindowTree(child);
}

struct CapturedOwnerDrawWindowRect
{
	HWND Hwnd {};
	RECT Rect {};
	int Depth {};
};

static std::vector<CapturedOwnerDrawWindowRect>& CapturedOwnerDrawWindowRects()
{
	static std::vector<CapturedOwnerDrawWindowRect> rects;
	return rects;
}

static int GetWindowHierarchyDepth(HWND hWnd)
{
	int depth = 0;
	for (HWND walker = ::GetParent(hWnd); walker; walker = ::GetParent(walker))
		++depth;

	return depth;
}

using WinDialogGetCurrentHandle = HWND(__fastcall*)();
using WinDialogFindHandle = HWND(__fastcall*)(HWND);

static HWND GetCurrentWinDialogHandle()
{
	return reinterpret_cast<WinDialogGetCurrentHandle>(0x775B10)();
}

static HWND FindPreviousWinDialogHandle(HWND hWnd)
{
	return reinterpret_cast<WinDialogFindHandle>(0x7759B0)(hWnd);
}

static bool ContainsHwnd(const std::vector<HWND>& windows, HWND hWnd)
{
	return std::find(windows.begin(), windows.end(), hWnd) != windows.end();
}

static void AddOwnerDrawDialogRoots(std::vector<HWND>& windows)
{
	for (auto it = OwnerDraw::Dialogs.begin(); it != OwnerDraw::Dialogs.end(); ++it)
	{
		const HWND hWnd = it->Key;
		if (!::IsWindow(hWnd) || ContainsHwnd(windows, hWnd))
			continue;

		if (::GetWindowLongA(hWnd, DialogProcWindowLongIndex))
			windows.push_back(hWnd);
	}
}

static std::vector<HWND> GetOwnerDrawDialogRoots()
{
	std::vector<HWND> windows;

	for (HWND hWnd = GetCurrentWinDialogHandle(); hWnd; hWnd = FindPreviousWinDialogHandle(hWnd))
	{
		if (!::IsWindow(hWnd) || ContainsHwnd(windows, hWnd))
			continue;

		if (FindOwnerDrawData(hWnd))
			windows.push_back(hWnd);
	}

	AddOwnerDrawDialogRoots(windows);
	return windows;
}

static bool FindCapturedOwnerDrawRect(
	const std::vector<CapturedOwnerDrawWindowRect>& rects,
	HWND hWnd,
	RECT& rect)
{
	for (const auto& captured : rects)
	{
		if (captured.Hwnd == hWnd)
		{
			rect = captured.Rect;
			return true;
		}
	}

	return false;
}

static std::vector<CapturedOwnerDrawWindowRect> CaptureOwnerDrawRenderRects()
{
	std::vector<CapturedOwnerDrawWindowRect> rects;
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates() || !OwnerDraw::Dialogs.size())
		return rects;

	rects.reserve(OwnerDraw::Dialogs.size());
	for (auto it = OwnerDraw::Dialogs.begin(); it != OwnerDraw::Dialogs.end(); ++it)
	{
		const HWND hWnd = it->Key;
		if (!::IsWindow(hWnd))
			continue;

		RECT rect {};
		if (OwnerDraw::GetRectangle(hWnd, &rect))
			rects.push_back(CapturedOwnerDrawWindowRect { hWnd, rect, GetWindowHierarchyDepth(hWnd) });
	}

	return rects;
}

static void NormalizeOwnerDrawWindowsToRawRenderCoordinates()
{
	auto rects = CaptureOwnerDrawRenderRects();
	if (rects.empty())
		return;

	std::stable_sort(
		rects.begin(),
		rects.end(),
		[](const CapturedOwnerDrawWindowRect& lhs, const CapturedOwnerDrawWindowRect& rhs)
		{
			return lhs.Depth < rhs.Depth;
		});

	const bool restoreRawWindowCoordinates = RenderDX::IsOwnerDrawUsingRawWindowCoordinates();
	RenderDX::SetOwnerDrawRawWindowCoordinates(true);

	for (const auto& captured : rects)
	{
		if (!::IsWindow(captured.Hwnd))
			continue;

		POINT parentOrigin {};
		const HWND parentHwnd = ::GetParent(captured.Hwnd);
		if (parentHwnd && parentHwnd != Game::hWnd)
		{
			RECT parentRect {};
			if (FindCapturedOwnerDrawRect(rects, parentHwnd, parentRect))
			{
				parentOrigin.x = parentRect.left;
				parentOrigin.y = parentRect.top;
			}
		}

		::MoveWindow(
			captured.Hwnd,
			captured.Rect.left - parentOrigin.x,
			captured.Rect.top - parentOrigin.y,
			captured.Rect.right - captured.Rect.left,
			captured.Rect.bottom - captured.Rect.top,
			FALSE);
	}

	RenderDX::SetOwnerDrawRawWindowCoordinates(restoreRawWindowCoordinates);
}

void WWUI::CaptureOwnerDrawWindowRects()
{
	auto& capturedRects = CapturedOwnerDrawWindowRects();
	capturedRects.clear();

	capturedRects = CaptureOwnerDrawRenderRects();
}

void WWUI::ApplyOwnerDrawWindowRects()
{
	auto& capturedRects = CapturedOwnerDrawWindowRects();
	if (capturedRects.empty())
		return;

	std::stable_sort(
		capturedRects.begin(),
		capturedRects.end(),
		[](const CapturedOwnerDrawWindowRect& lhs, const CapturedOwnerDrawWindowRect& rhs)
		{
			return lhs.Depth < rhs.Depth;
		});

	for (const auto& captured : capturedRects)
	{
		if (!::IsWindow(captured.Hwnd))
			continue;

		RECT clientRect {};
		if (!RenderDX::RenderRectToClient(::GetParent(captured.Hwnd), captured.Rect, &clientRect))
			continue;

		::MoveWindow(
			captured.Hwnd,
			clientRect.left,
			clientRect.top,
			clientRect.right - clientRect.left,
			clientRect.bottom - clientRect.top,
			FALSE);
	}

	WWUI::SyncListBoxScrollBarPositions(nullptr);
	WWUI::SyncStaticMoviePositions(nullptr);
	ResetOwnerDrawCachedSurfaceTree(nullptr);
	capturedRects.clear();
}

void WWUI::RelayoutWindowsAfterDisplayModeChange()
{
	if (!OwnerDraw::Dialogs.size())
		return;

	const auto windows = GetOwnerDrawDialogRoots();
	if (windows.empty())
	{
		ResetOwnerDrawCachedSurfaceTree(nullptr);
		WWUI::SyncListBoxScrollBarPositions(nullptr);
		WWUI::SyncStaticMoviePositions(nullptr);
		return;
	}

	const bool restoreRawWindowCoordinates = RenderDX::IsOwnerDrawUsingRawWindowCoordinates();
	if (!restoreRawWindowCoordinates)
		NormalizeOwnerDrawWindowsToRawRenderCoordinates();

	RenderDX::SetOwnerDrawRawWindowCoordinates(true);

	auto baseSize = OwnerDraw::BaseLayoutSize;
	for (const HWND hWnd : windows)
	{
		if (!::IsWindow(hWnd))
			continue;

		ResetOwnerDrawCachedSurfaceTree(hWnd);
		OwnerDraw::UpdateControlPosition(hWnd, &baseSize);
		ApplyRenderLayoutToWindowTree(hWnd);
	}

	RenderDX::SetOwnerDrawRawWindowCoordinates(false);

	for (const HWND hWnd : windows)
	{
		if (!::IsWindow(hWnd))
			continue;

		UI::CenterWindow(hWnd);
		WWUI::SyncListBoxScrollBarPositions(hWnd);
		WWUI::SyncStaticMoviePositions(hWnd);
		ResetOwnerDrawCachedSurfaceTree(hWnd);
	}

	RenderDX::SetOwnerDrawRawWindowCoordinates(restoreRawWindowCoordinates);
}

static void FinishDialogInitialization(HWND hWnd)
{
	if (!SessionClass::Instance.CurrentlyInGame)
		OwnerDraw::LoadNotInGameResources(hWnd);

	OwnerDraw::UpdateTopPanelAnimationFlag(hWnd);
	OwnerDraw::UpdateButtonAnimationFlag(hWnd);
	OwnerDraw::UpdateMainScreenAnimationFlag(hWnd);
	OwnerDraw::UpdateFlagD8FromDialogID(hWnd);

	::EnumChildWindows(hWnd, OwnerDraw::InitCompactDialogControlsProc, 0);

	if (OwnerDraw::TrySetDialogLayoutBand1(hWnd))
	{
		auto baseSize = OwnerDraw::BaseLayoutSize;
		OwnerDraw::UpdateControlPosition(hWnd, &baseSize);
	}
	else
	{
		OwnerDraw::TrySetDialogLayoutBand2(hWnd);
	}

	::EnumChildWindows(hWnd, OwnerDraw::ClassifyLayoutBand, 0);
	::EnumChildWindows(hWnd, OwnerDraw::ResetControlDrawModeAndTimerProc, 0);
	ApplyRenderLayoutToWindowTree(hWnd);
	RenderDX::SetOwnerDrawRawWindowCoordinates(false);
	UI::CenterWindow(hWnd);
	WWUI::SyncListBoxScrollBarPositions(hWnd);
	ResetOwnerDrawCachedSurfaceTree(hWnd);
	WWUI::SyncStaticMoviePositions(hWnd);
	::SetFocus(hWnd);
}

static void RegisterDialogControls(HWND hWnd, int dialogID)
{
	UI::RegisterComboDropAndNewEditClasses();
	OwnerDraw::CurrentDialogHwnd = hWnd;

	::EnumChildWindows(hWnd, OwnerDraw::SetNeedsControlImage, 1);
	OwnerDraw::SetNeedsControlImage(hWnd, 1);

	::EnumChildWindows(hWnd, OwnerDraw::ReplaceEditWithListboxProc, 1);

	::EnumChildWindows(hWnd, OwnerDraw::RegisterChildControlProc, 0);
	OwnerDraw::RegisterChildControlProc(hWnd, 0);

	::EnumChildWindows(hWnd, OwnerDraw::SetNeedsControlImage, 0);
	OwnerDraw::SetNeedsControlImage(hWnd, 0);

	OwnerDraw::ScaleControls(hWnd);
	OwnerDraw::SetDialogID(hWnd, dialogID);
}

static LRESULT HandleInitDialog(HWND hWnd, LPARAM lParam)
{
	++Unsorted::WSDialogCount;
	RenderDX::SetOwnerDrawRawWindowCoordinates(true);

	if (lParam)
	{
		const int dialogID = *reinterpret_cast<const WORD*>(lParam);
		RegisterDialogControls(hWnd, dialogID);
	}
	else
	{
		OwnerDraw::PrepareDialogChildControls(hWnd, 0);
		OwnerDraw::ScaleControls(hWnd);

		if (auto pData = FindOwnerDrawData(hWnd))
			pData->DialogID = 0;
	}

	FinishDialogInitialization(hWnd);
	return 0;
}

HWND __fastcall WWUI::RegisterOwnerDrawWindow(HWND hWnd, int dialogID)
{
	const bool restoreRawWindowCoordinates = RenderDX::IsOwnerDrawUsingRawWindowCoordinates();

	RenderDX::SetOwnerDrawRawWindowCoordinates(true);
	RegisterDialogControls(hWnd, dialogID);
	FinishDialogInitialization(hWnd);
	RenderDX::SetOwnerDrawRawWindowCoordinates(restoreRawWindowCoordinates);

	return hWnd;
}

static LRESULT HandlePaint(HWND hWnd)
{
	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return 0;

	if (pData->SkipDraw)
	{
		::ValidateRect(hWnd, nullptr);
		return 1;
	}

	OwnerDraw::Paint(hWnd);

	pData = FindOwnerDrawData(hWnd);
	if (pData && pData->HasFadeAnimation)
	{
		if (const auto movieHwnd = ::GetDlgItem(hWnd, OwnerDraw::TransitionMovie))
			::SendMessageA(movieHwnd, WW_STATIC_DETACHMOVIE, 0, 0);

		OwnerDraw::DrawCampaignMenuTransition(hWnd, false);
		pData->HasFadeAnimation = false;
	}

	::ValidateRect(hWnd, nullptr);
	return 0;
}

static LRESULT HandleTooltipRefresh(HWND hWnd, LPARAM lParam)
{
	const auto tooltipHwnd = ::GetDlgItem(hWnd, OwnerDraw::TooltipText);
	if (!tooltipHwnd)
		return 0;

	const POINT screenPoint { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	POINT clientPoint = screenPoint;
	::ScreenToClient(hWnd, &clientPoint);

	WideWstring tooltipText;

	if (const auto controlHwnd = ::ChildWindowFromPointEx(hWnd, clientPoint, CWP_SKIPINVISIBLE))
	{
		POINT controlPoint = screenPoint;
		::ScreenToClient(controlHwnd, &controlPoint);
		const LPARAM pointParam = MAKELPARAM(static_cast<WORD>(controlPoint.x), static_cast<WORD>(controlPoint.y));
		const auto hitCode = ::SendMessageA(controlHwnd, WW_QUERYTOOLTIPHIT, 0, pointParam);

		tooltipText = QueryTooltipText(hWnd, controlHwnd, hitCode);

		if (IsEmpty(tooltipText))
		{
			tooltipText = QueryTooltipText(hWnd, controlHwnd, -1);

			if (IsEmpty(tooltipText))
			{
				if (const auto label = OwnerDraw::GetTooltipStringLabel(hWnd, controlHwnd))
					tooltipText = StringTable::LoadString(label);
				else
					tooltipText = L"";
			}
		}
	}

	SetTooltipTextIfChanged(tooltipHwnd, tooltipText);
	return 0;
}

LRESULT __fastcall WWUI::OwnerDrawStandardWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (RenderDX::HandleFullscreenToggleMessage(message, wParam, lParam))
		return 0;

	switch (message)
	{
	case WM_DESTROY:
		UI::RemoveModelessDialog(hWnd);
		--Unsorted::WSDialogCount;
		::SetFocus(Game::hWnd);
		return 0;

	case WM_PAINT:
		return HandlePaint(hWnd);

	case WM_ERASEBKGND:
		return 1;

	case WM_DRAWITEM:
		OwnerDraw::DrawItem(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
		return 1;

	case WM_NCHITTEST:
		return HandleTooltipRefresh(hWnd, lParam);

	case WM_INITDIALOG:
		return HandleInitDialog(hWnd, lParam);

	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		return reinterpret_cast<LRESULT>(::GetStockObject(BLACK_BRUSH));

	case WW_INITDIALOG:
		::SendMessageA(hWnd, WW_BRINGTOTOP, reinterpret_cast<WPARAM>(hWnd), 1);
		return 0;

	case WW_TRANSITION_COMPLETE:
		::EnumChildWindows(hWnd, OwnerDraw::SendTransitionCompleteToCustomTextChildProc, 0);
		return 0;

	default:
		return 0;
	}

	return 0;
}

LRESULT CALLBACK WWUI::OwnerDrawWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_SETCURSOR)
		return 1;

	if (RenderDX::HandleFullscreenToggleMessage(message, wParam, lParam))
		return 0;

	const bool isPaintMessage = message == WM_PAINT;
	const auto updatePrimaryAfterPaint = [isPaintMessage]()
	{
		if (isPaintMessage)
			RenderDX::UpdateScreen(DSurface::Primary);
	};

	if (OwnerDraw::ServiceIMEMessage(hWnd, message, wParam, lParam))
	{
		const auto imeResult = OwnerDraw::GetIMEResult();
		updatePrimaryAfterPaint();
		return imeResult;
	}

	const auto pSelectedWndProc = FindWindowProc(OwnerDraw::SubclassProcs, hWnd);

	if (message == WM_SYSKEYUP && wParam == VK_TAB)
		::SendMessageA(Game::hWnd, WM_SYSKEYUP, VK_TAB, lParam);

	// RenderDX's original 0x610E77 hook made the final copyback use client-surface coordinates.
	constexpr int windowOffsetX = 0;
	constexpr int windowOffsetY = 0;

	if (IsActiveWindowMessageBlocked(hWnd, message))
	{
		updatePrimaryAfterPaint();
		return 0;
	}

	WindowMessageGuardScope guard(hWnd, message);
	if (!guard.Enter())
	{
		updatePrimaryAfterPaint();
		return 0;
	}

	RECT clientRect {};
	::GetClientRect(hWnd, &clientRect);

	RECT ownerDrawClientRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerDrawClientRect);

	if (isPaintMessage && !Unsorted::GameInFocus)
	{
		::ValidateRect(hWnd, nullptr);
		guard.Release();
		updatePrimaryAfterPaint();
		return 0;
	}

	if (isPaintMessage)
	{
		++OwnerDraw::PaintDepth;

		RECT updateRect {};
		::GetUpdateRect(hWnd, &updateRect, FALSE);
		updateRect.left += ownerDrawClientRect.left;
		updateRect.right += ownerDrawClientRect.left;
		updateRect.top += ownerDrawClientRect.top;
		updateRect.bottom += ownerDrawClientRect.top;
	}

	auto pData = FindOwnerDrawData(hWnd);
	int paintCopyMode = 0;
	bool redrawTooltip = false;
	LRESULT result = 0;

	auto complete = [&](LRESULT result) -> LRESULT
	{
		guard.Release();

		if (isPaintMessage)
			FinishPaint(hWnd, pData, paintCopyMode, windowOffsetX, windowOffsetY);

		if (redrawTooltip)
			OwnerDraw::DrawTooltip(true);

		updatePrimaryAfterPaint();

		return message == WM_INITDIALOG ? 0 : result;
	};

	switch (message)
	{
	case WW_GETHWND:
		return complete(pData && pData->LinkedHwnd() ? 1 : 0);

	case WW_GETGDIPROPS:
		if (pData)
		{
			const auto hdc = reinterpret_cast<HDC>(lParam);
			const auto oldFont = ::SelectObject(hdc, ::GetStockObject(SYSTEM_FONT));
			pData->Extra[SavedFontExtraIndex] = reinterpret_cast<int>(oldFont);
			::SelectObject(hdc, oldFont);
			pData->Extra[SavedBkModeExtraIndex] = ::GetBkMode(hdc);
			pData->Extra[SavedBkColorExtraIndex] = ::GetBkColor(hdc);
			pData->Extra[SavedTextColorExtraIndex] = ::GetTextColor(hdc);
			return complete(1);
		}
		break;

	case WW_SETGDIPROPS:
		if (pData)
		{
			const auto hdc = reinterpret_cast<HDC>(lParam);
			::SelectObject(hdc, reinterpret_cast<HGDIOBJ>(pData->Extra[SavedFontExtraIndex]));
			::SetBkMode(hdc, pData->Extra[SavedBkModeExtraIndex]);
			::SetBkColor(hdc, pData->Extra[SavedBkColorExtraIndex]);
			::SetTextColor(hdc, pData->Extra[SavedTextColorExtraIndex]);
			return complete(1);
		}
		break;

	case WW_SETHASIMAGE:
		if (pData)
		{
			const auto previous = pData->NeedsControlImage;
			const HWND linkedHwnd = pData->LinkedHwnd();
			pData->NeedsControlImage = lParam;

			if (linkedHwnd)
			{
				if (auto pLinkedData = FindOwnerDrawData(linkedHwnd))
					pLinkedData->NeedsControlImage = lParam;
			}

			result = previous;
		}
		break;

	case WW_BRINGTOTOP:
		return complete(BringOwnerDrawWindowToTop(hWnd, wParam, lParam));

	default:
		break;
	}

	if (OwnerDraw::ActiveWindowStackCount)
	{
		if (message == WM_WINDOWPOSCHANGING)
		{
			LRESULT windowPosResult = 0;
			if (HandleWindowPosChanging(hWnd, lParam, windowPosResult))
				return complete(windowPosResult);
		}
		else if (message == WM_DESTROY)
		{
			RemoveActiveWindow(hWnd);
		}
	}

	RestoreAndClearTooltipIfNeeded(hWnd, message);

	bool callSelectedHandler = true;

	switch (message)
	{
	case WM_MOVE:
	case WM_SIZE:
	case WM_WINDOWPOSCHANGED:
		ResetOwnerDrawCachedSurfaceTree(hWnd);
		WWUI::SyncListBoxScrollBarPositions(hWnd);
		WWUI::SyncStaticMoviePositions(hWnd);
		break;

	case WM_ERASEBKGND:
		return complete(1);

	case WM_SETFOCUS:
		if (pSelectedWndProc == OwnerDraw::OwnerDrawButtonHandler || pSelectedWndProc == OwnerDraw::ListBoxHandler)
			::SetFocus(reinterpret_cast<HWND>(wParam));

		if (pData && !pData->HasFocus)
		{
			OwnerDraw::CancelIMEComposition();
			pData->HasFocus = 1;
		}
		break;

	case WM_KILLFOCUS:
		if (pData)
			pData->HasFocus = 0;
		break;

	case WM_SHOWWINDOW:
		if (!wParam && pData)
			pData->Extra[PaintStateExtraIndex] = 0;
		break;

	default:
		break;
	}

	if (pData)
	{
		switch (message)
		{
		case WW_SETIMAGE:
			result = reinterpret_cast<LRESULT>(pData->ControlImage);
			pData->ControlImage = reinterpret_cast<Surface*>(lParam);
			return complete(result);

		case WW_SETACTIVEIMAGE:
			result = reinterpret_cast<LRESULT>(pData->StateImageSurface);
			pData->StateImageSurface = reinterpret_cast<Surface*>(lParam);
			return complete(result);

		case WW_SETUNKNOWNPROP24:
			result = pData->UnknownProp24();
			pData->UnknownProp24() = lParam;
			return complete(result);

		default:
			break;
		}

		if (pData->UnknownProp24() && (message == WM_TIMER || (message >= WM_MOUSEFIRST && message <= WM_MBUTTONDBLCLK)))
		{
			RECT rect {};
			::GetWindowRect(hWnd, &rect);
			::WindowFromPoint(POINT { rect.left, rect.top });
		}
	}

	if (IsNativeTextMessage(message, result))
		callSelectedHandler = false;

	if (pData)
		HandleElementTextMessage(*pData, hWnd, message, wParam, lParam, result, callSelectedHandler);

	if (message == WM_MOUSEMOVE)
		UpdateTooltipTextOnMouseMove(hWnd, lParam);

	if (isPaintMessage && pData)
	{
		result = DispatchPaintMessage(
			hWnd,
			message,
			wParam,
			lParam,
			pSelectedWndProc,
			ownerDrawClientRect,
			paintCopyMode,
			redrawTooltip);
	}
	else if (callSelectedHandler && pSelectedWndProc)
	{
		const LPARAM selectedLParam = GetSelectedHandlerLParam(hWnd, message, lParam, pData, pSelectedWndProc);
		result = CallSelectedHandler(pSelectedWndProc, hWnd, message, wParam, selectedLParam);
	}

	if (message == WM_NCDESTROY)
		CleanupDestroyedWindow(hWnd);

	return complete(result);
}
