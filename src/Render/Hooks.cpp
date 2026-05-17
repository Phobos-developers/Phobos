#include "Functions.h"

#include <Utilities/Macro.h>

#include <Unsorted.h>
#include <Drawing.h>
#include <Surface.h>
#include <SessionClass.h>

#ifdef CALL
#undef CALL
#endif

DEFINE_FUNCTION_JUMP(LJMP, 0x777C30, RenderDX::CreateMainWindow);

static BOOL WINAPI ClientToScreenHook(HWND hWnd, LPPOINT lpPoint) {
	return TRUE;
}

DEFINE_PATCH_TYPED(void*, 0x7E14B8, ClientToScreenHook);

static void __fastcall CenterWindowIn(HWND window, HWND parent) {
	RECT parentRect;
	::GetClientRect(parent, &parentRect);

	if (parent == Game::hWnd) {
		parentRect.right = Drawing::RenderWidth;
		parentRect.bottom = Drawing::RenderHeight;
	}

	::ClientToScreen(parent, reinterpret_cast<LPPOINT>(&parentRect));
	::ClientToScreen(parent, reinterpret_cast<LPPOINT>(&parentRect.right));
	parentRect.right -= parentRect.left;
	parentRect.bottom -= parentRect.top;

	RECT rect;
	::GetClientRect(window, &rect);
	::ClientToScreen(window, reinterpret_cast<LPPOINT>(&rect));
	::ClientToScreen(window, reinterpret_cast<LPPOINT>(&rect.right));
	rect.right -= rect.left;
	rect.bottom -= rect.top;
	int x = (parentRect.right - rect.right + 1) / 2;
	int y = (parentRect.bottom - rect.bottom + 1) / 2;

	x = std::max(x, 0);
	y = std::max(y, 0);

	::SetWindowPos(window, nullptr, x, y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x777080, CenterWindowIn);

static BOOL __fastcall MoveDialog(HWND window, int x, int y) {
	int xPos;
	int yPos;

	RECT screenRect;
	screenRect.left = 0;
	screenRect.top = 0;
	screenRect.right = Drawing::RenderWidth;
	screenRect.bottom = Drawing::RenderHeight;

	::ClientToScreen(Game::hWnd, reinterpret_cast<LPPOINT>(&screenRect));
	::ClientToScreen(Game::hWnd, reinterpret_cast<LPPOINT>(&screenRect.right));

	RECT windowRect;
	::GetWindowRect(window, &windowRect);

	windowRect.right -= windowRect.left;
	windowRect.bottom -= windowRect.top;

	if (x == -1)
		xPos = windowRect.left - screenRect.left;
	else
		xPos = x;
	windowRect.left = xPos;

	if (y == -1)
		yPos = windowRect.top - screenRect.top;
	else
		yPos = y;
	windowRect.top = yPos;

	return ::MoveWindow(window, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom, FALSE);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x623170, MoveDialog);

static BOOL __fastcall WinDialogGetRectangle(HWND hWnd, LPRECT rect) {
	BOOL result = ::GetWindowRect(hWnd, rect);
	if (result) {
		RECT client;
		::GetClientRect(Game::hWnd, &client);
		::ClientToScreen(Game::hWnd, reinterpret_cast<LPPOINT>(&client));
		rect->left -= client.left;
		rect->right -= client.left;
		rect->top -= client.top;
		rect->bottom -= client.top;
	}
	return result;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x775690, WinDialogGetRectangle);

static BOOL __fastcall GetWindowRectHook(HWND hWnd, LPRECT rect) {
	return ::GetWindowRect(hWnd, rect);
}
DEFINE_FUNCTION_JUMP(CALL, 0x610E77, GetWindowRectHook);

static BOOL __fastcall MoveIngameWindowControls(HWND hWnd) {
	if (!SessionClass::Instance.CurrentlyInGame)
		return FALSE;

	auto parent = ::GetParent(hWnd);

	RECT rect;
	RECT parentRect;
	if (!parent || !::GetWindowRect(hWnd, &rect) || !::GetWindowRect(parent, &parentRect))
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

DEFINE_JUMP(LJMP, 0x4A4830, 0x4A4848); // Skip Wait_Blit

DEFINE_JUMP(LJMP, 0x4A4780, 0x4A4825); // Skip Set_DD_Palette

DEFINE_FUNCTION_JUMP(LJMP, 0x533FD0, RenderDX::AllocateSurfaces);
DEFINE_FUNCTION_JUMP(LJMP, 0x4A42F0, RenderDX::SetVideoMode);
DEFINE_FUNCTION_JUMP(LJMP, 0x4A44F0, RenderDX::ResetVideoMode);
DEFINE_FUNCTION_JUMP(LJMP, 0x560BF0, RenderDX::ChangeDisplayMode);

DEFINE_HOOK(0x4F4B7E, DXRenderUpdateScreenGScreenClassBlit, 0x5) {
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_HOOK(0x5D2320, DXRenderUpdateScreenMSEngineBlitRects, 0x7) {
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_HOOK(0x5D1F15, DXRenderUpdateScreenMSEngineFrameUpdate, 0x5) {
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_HOOK(0x690228, DXRenderUpdateScreenScoreClassCallBackDelay, 0x6) {
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_NAKED_HOOK(0x5C0477, DXRenderUpdateScreenMovieBlitToScreen) {
	__asm {
		call dword ptr[edx + 8]
		mov ecx, dword ptr ds:[0x887308]
		call RenderDX::UpdateScreen
		pop edi
		pop esi
		pop ebx
		add esp, 0x20
		ret
	}
}

static LRESULT CALLBACK OwnerDrawWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	auto result = reinterpret_cast<LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM)>(0x610CA0)(hWnd, message, wParam, lParam);
	if (message == WM_PAINT)
		RenderDX::UpdateScreen(DSurface::Primary);
	return result;
}
DEFINE_PATCH_TYPED(void*, 0x60FF06, OwnerDrawWindowProcedure);

DEFINE_HOOK_AGAIN(0x611FB0, DXRenderUpdateScreenOwnerDrawWindow, 0x6);
DEFINE_HOOK(0x61187D, DXRenderUpdateScreenOwnerDrawWindow, 0xA) {
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_HOOK(0x7776B5, MainWindowProcWMPaint, 0x6) {
	RenderDX::MainProcHandlePaint();
	return 0x7779B5;
}

DEFINE_JUMP(LJMP, 0x6BD9D9, 0x6BDA61);
DEFINE_JUMP(LJMP, 0x6BDB16, 0x6BDB6D);

DEFINE_JUMP(LJMP, 0x4A3FD0, 0x4A4019); // Skip Prep_Direct_Draw
DEFINE_FUNCTION_JUMP(LJMP, 0x4A4900, RenderDX::EnumDisplayModes);
