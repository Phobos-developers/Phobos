#include <Utilities/Macro.h>

#include "Functions.h"

#include <Unsorted.h>
#include <Drawing.h>
#include <SessionClass.h>

#include <algorithm>

#ifdef CALL
#undef CALL
#endif

static BOOL WINAPI ClientToScreenHook(HWND, LPPOINT)
{
	// The game also uses this IAT entry while blitting game and movie surfaces.
	// Keep its historical surface-space semantics; explicit Phobos helpers use real Win32 conversion.
	return TRUE;
}
DEFINE_PATCH_TYPED(void*, 0x7E14B8, ClientToScreenHook);

static BOOL WINAPI GetClientRectHook(HWND hWnd, LPRECT rect)
{
	if (hWnd == Game::hWnd && rect && Drawing::RenderWidth > 0 && Drawing::RenderHeight > 0)
	{
		rect->left = 0;
		rect->top = 0;
		rect->right = Drawing::RenderWidth;
		rect->bottom = Drawing::RenderHeight;
		return TRUE;
	}

	return ::GetClientRect(hWnd, rect);
}
DEFINE_PATCH_TYPED(void*, 0x7E14C4, GetClientRectHook);

static void __fastcall CenterWindowIn(HWND window, HWND parent)
{
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
	{
		RECT parentRect {};
		::GetClientRect(parent, &parentRect);

		if (parent == Game::hWnd)
		{
			parentRect.right = Drawing::RenderWidth;
			parentRect.bottom = Drawing::RenderHeight;
		}

		RECT rect {};
		::GetClientRect(window, &rect);
		int x = (parentRect.right - rect.right + 1) / 2;
		int y = (parentRect.bottom - rect.bottom + 1) / 2;

		x = std::max(x, 0);
		y = std::max(y, 0);

		::SetWindowPos(window, nullptr, x, y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
		return;
	}

	RECT parentRect {};
	if (parent == Game::hWnd)
	{
		parentRect.right = Drawing::RenderWidth;
		parentRect.bottom = Drawing::RenderHeight;
	}
	else if (!RenderDX::GetClientRectInRender(parent, &parentRect))
	{
		return;
	}

	RECT rect {};
	if (!RenderDX::GetClientRectInRender(window, &rect))
		return;

	const int parentWidth = parentRect.right - parentRect.left;
	const int parentHeight = parentRect.bottom - parentRect.top;
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;
	int x = (parentWidth - width + 1) / 2;
	int y = (parentHeight - height + 1) / 2;

	x = std::max(x, 0);
	y = std::max(y, 0);

	RenderDX::SetWindowPosInRender(window, nullptr, x, y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x777080, CenterWindowIn);

static BOOL GetRawWindowRectInRenderLayout(HWND hWnd, LPRECT rect);

static BOOL __fastcall MoveDialog(HWND window, int x, int y)
{
	RECT windowRect {};
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
	{
		if (!GetRawWindowRectInRenderLayout(window, &windowRect))
			return FALSE;
	}
	else if (!RenderDX::GetWindowRectInRender(window, &windowRect))
	{
		return FALSE;
	}

	const int width = windowRect.right - windowRect.left;
	const int height = windowRect.bottom - windowRect.top;
	const int xPos = x == -1 ? windowRect.left : x;
	const int yPos = y == -1 ? windowRect.top : y;
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
		return ::MoveWindow(window, xPos, yPos, width, height, FALSE);

	return RenderDX::MoveWindowInRender(window, xPos, yPos, width, height, FALSE);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x623170, MoveDialog);

static BOOL GetRawWindowRectInRenderLayout(HWND hWnd, LPRECT rect)
{
	if (!hWnd || !rect)
		return FALSE;

	if (hWnd == Game::hWnd)
	{
		rect->left = 0;
		rect->top = 0;
		rect->right = Drawing::RenderWidth;
		rect->bottom = Drawing::RenderHeight;
		return TRUE;
	}

	if (!::GetWindowRect(hWnd, rect))
		return FALSE;

	POINT origin { 0, 0 };
	if (!::ClientToScreen(Game::hWnd, &origin))
		return FALSE;

	rect->left -= origin.x;
	rect->right -= origin.x;
	rect->top -= origin.y;
	rect->bottom -= origin.y;
	return TRUE;
}

static BOOL __fastcall WinDialogGetRectangle(HWND hWnd, LPRECT rect)
{
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
		return GetRawWindowRectInRenderLayout(hWnd, rect);

	return RenderDX::GetWindowRectInRender(hWnd, rect);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x775690, WinDialogGetRectangle);

static BOOL __fastcall GetWindowRectHook(HWND hWnd, LPRECT rect)
{
	return ::GetWindowRect(hWnd, rect);
}
DEFINE_FUNCTION_JUMP(CALL, 0x610E77, GetWindowRectHook);

static BOOL WINAPI OwnerDrawPaintGetClientRectHook(HWND hWnd, LPRECT rect)
{
	if (!RenderDX::IsOwnerDrawUsingRawWindowCoordinates() && RenderDX::GetClientRectInRender(hWnd, rect))
		return TRUE;

	return ::GetClientRect(hWnd, rect);
}
DEFINE_FUNCTION_JUMP(CALL6, 0x621EF3, OwnerDrawPaintGetClientRectHook);

static BOOL WINAPI ComboDropGetClientRectHook(HWND hWnd, LPRECT rect)
{
	if (!RenderDX::IsOwnerDrawUsingRawWindowCoordinates() && RenderDX::GetClientRectInRender(hWnd, rect))
		return TRUE;

	return ::GetClientRect(hWnd, rect);
}
DEFINE_FUNCTION_JUMP(CALL6, 0x60D58E, ComboDropGetClientRectHook);

static BOOL WINAPI ComboDropGetWindowRectHook(HWND hWnd, LPRECT rect)
{
	if (!RenderDX::IsOwnerDrawUsingRawWindowCoordinates() && RenderDX::GetWindowRectInRender(hWnd, rect))
		return TRUE;

	return ::GetWindowRect(hWnd, rect);
}
DEFINE_FUNCTION_JUMP(CALL6, 0x60E049, ComboDropGetWindowRectHook);

static BOOL WINAPI ComboDropSetWindowPosHook(
	HWND hWnd,
	HWND hWndInsertAfter,
	int x,
	int y,
	int cx,
	int cy,
	UINT flags)
{
	if (!RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
		return RenderDX::SetWindowPosInRender(hWnd, hWndInsertAfter, x, y, cx, cy, flags);

	return ::SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, flags);
}
DEFINE_FUNCTION_JUMP(CALL6, 0x60E84A, ComboDropSetWindowPosHook);
DEFINE_FUNCTION_JUMP(CALL6, 0x60F1BE, ComboDropSetWindowPosHook);

static bool IsComboDropMousePointMessage(UINT message)
{
	return message >= WM_MOUSEFIRST && message <= WM_MBUTTONDBLCLK;
}

static WNDPROC GetComboDropWindowProc()
{
	return reinterpret_cast<WNDPROC>(0x60D540);
}

static LRESULT CALLBACK ComboDropWindowProcHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (RenderDX::HandleFullscreenToggleMessage(message, wParam, lParam))
		return 0;

	if (!RenderDX::IsOwnerDrawUsingRawWindowCoordinates() && IsComboDropMousePointMessage(message))
		lParam = RenderDX::MouseLParamToRenderLocal(hWnd, lParam);

	return GetComboDropWindowProc()(hWnd, message, wParam, lParam);
}
DEFINE_PATCH_TYPED(void*, 0x60D4A2, ComboDropWindowProcHook);

static HWND WINAPI ComboDropCreateWindowExAHook(
	DWORD exStyle,
	LPCSTR className,
	LPCSTR windowName,
	DWORD style,
	int x,
	int y,
	int width,
	int height,
	HWND parentHwnd,
	HMENU menu,
	HINSTANCE instance,
	LPVOID param)
{
	if (!RenderDX::IsOwnerDrawUsingRawWindowCoordinates() && parentHwnd)
	{
		RECT parentRect {};
		RECT clientRect {};
		const RECT localRect { x, y, x + width, y + height };
		if (RenderDX::GetWindowRectInRender(parentHwnd, &parentRect)
			&& RenderDX::RenderRectToClient(
				parentHwnd,
				RECT {
					parentRect.left + localRect.left,
					parentRect.top + localRect.top,
					parentRect.left + localRect.right,
					parentRect.top + localRect.bottom
				},
				&clientRect))
		{
			x = clientRect.left;
			y = clientRect.top;
			width = clientRect.right - clientRect.left;
			height = clientRect.bottom - clientRect.top;
		}
	}

	return ::CreateWindowExA(
		exStyle,
		className,
		windowName,
		style,
		x,
		y,
		width,
		height,
		parentHwnd,
		menu,
		instance,
		param);
}
DEFINE_FUNCTION_JUMP(CALL6, 0x60E72F, ComboDropCreateWindowExAHook);

static BOOL __fastcall MoveIngameWindowControls(HWND hWnd)
{
	if (!SessionClass::Instance.CurrentlyInGame)
		return FALSE;

	auto parent = ::GetParent(hWnd);

	RECT rect;
	RECT parentRect;
	if (!::GetWindowRect(hWnd, &rect) || !::GetWindowRect(parent, &parentRect))
		return FALSE;

	int x = rect.left - parentRect.left + (parentRect.right - parentRect.left - 800) / 2;
	int y = rect.top - parentRect.top + (parentRect.bottom - parentRect.top - 600) / 2;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	return ::MoveWindow(hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, FALSE);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x60B7A0, MoveIngameWindowControls);
