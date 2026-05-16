#include "Functions.h"

#include <Syringe.h>
#include <Utilities/Macro.h>

#include <Unsorted.h>
#include <Surface.h>
#include <SessionClass.h>
#include <Drawing.h>

#ifdef CALL
#undef CALL
#endif

// Main window creation
DEFINE_FUNCTION_JUMP(LJMP, 0x777C30, RenderDX::CreateMainWindow);

static BOOL WINAPI _ClientToScreen(HWND hWnd, LPPOINT lpPoint) {
	return TRUE;
}

// Disable ClientToScreen
DEFINE_PATCH_TYPED(void*, 0x7E14B8, _ClientToScreen);

// But these 3 need to use the real ClientToScreen so that dialogs are where they should be.
static void __fastcall CenterWindowIn(HWND window, HWND parent) {
	RECT rcl;
	::GetClientRect(parent, &rcl);

	if (parent == Game::hWnd) {
		rcl.right = Drawing::RenderWidth;
		rcl.bottom = Drawing::RenderHeight;
	}

	::ClientToScreen(parent, reinterpret_cast<LPPOINT>(&rcl));
	::ClientToScreen(parent, reinterpret_cast<LPPOINT>(&rcl.right));
	rcl.right -= rcl.left;
	rcl.bottom -= rcl.top;

	RECT rect;
	::GetClientRect(window, &rect);
	::ClientToScreen(window, reinterpret_cast<LPPOINT>(&rect));
	::ClientToScreen(window, reinterpret_cast<LPPOINT>(&rect.right));
	rect.right -= rect.left;
	rect.bottom -= rect.top;
	int x = (rcl.right - rect.right + 1) / 2;
	int y = (rcl.bottom - rect.bottom + 1) / 2;

	x = std::max(x, 0);
	y = std::max(y, 0);

	::SetWindowPos(window, nullptr, x, y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x777080, CenterWindowIn);

static BOOL __fastcall ODMoveDialog(HWND window, int x, int y) {
	int xpos;
	int ypos;

	RECT rect1;
	rect1.left = 0;
	rect1.top = 0;
	rect1.right = *reinterpret_cast<LONG*>(0x8A00A4);
	rect1.bottom = *reinterpret_cast<LONG*>(0x8A00A8);

	::ClientToScreen(Game::hWnd, reinterpret_cast<LPPOINT>(&rect1));
	::ClientToScreen(Game::hWnd, reinterpret_cast<LPPOINT>(&rect1.right));

	RECT rect2;
	::GetWindowRect(window, &rect2);

	rect2.right -= rect2.left;
	rect2.bottom -= rect2.top;

	if (x == -1) {
		xpos = rect2.left - rect1.left;
	}
	else {
		xpos = x;
	}
	rect2.left = xpos;

	if (y == -1) {
		ypos = rect2.top - rect1.top;
	}
	else {
		ypos = y;
	}
	rect2.top = ypos;

	return ::MoveWindow(window, rect2.left, rect2.top, rect2.right, rect2.bottom, FALSE);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x623170, ODMoveDialog);

static BOOL __fastcall WinDialog_GetRectangle(HWND hWnd, LPRECT rect) {
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
DEFINE_FUNCTION_JUMP(LJMP, 0x775690, WinDialog_GetRectangle);

// However this WinDialog_GetRectangle is used for drawing offset, so we need to use the original window rect
static BOOL __fastcall _GetWindowRect(HWND hWnd, LPRECT rect) {
	return ::GetWindowRect(hWnd, rect);
}
DEFINE_FUNCTION_JUMP(CALL, 0x610E77, _GetWindowRect);

// All controls inside the window is repositioned by this function, fix it up too
static BOOL __fastcall OD_MoveIngameWindowControls(HWND hWnd) {
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
DEFINE_FUNCTION_JUMP(LJMP, 0x60B7A0, OD_MoveIngameWindowControls);

DEFINE_JUMP(LJMP, 0x4A4830, 0x4A4848); // Skip Wait_Blit

DEFINE_JUMP(LJMP, 0x4A4780, 0x4A4825); // Skip Set_DD_Palette

// Rendering preps.
DEFINE_FUNCTION_JUMP(LJMP, 0x533FD0, RenderDX::AllocateSurfaces);
DEFINE_FUNCTION_JUMP(LJMP, 0x4A42F0, RenderDX::SetVideoMode);
DEFINE_FUNCTION_JUMP(LJMP, 0x4A44F0, RenderDX::ResetVideoMode);
DEFINE_FUNCTION_JUMP(LJMP, 0x560BF0, RenderDX::ChangeDisplayMode);

// Update the window surface when the game updates its PrimarySurface
DEFINE_HOOK(0x4F4B7E, DXRender_UpdateScreen_GScreenClass_Blit, 0x5) {
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_HOOK(0x5D233A, DXRender_UpdateScreen_MSEngine_Blit_Rects, 0x5) {
	const auto eflags = R->EFLAGS();
	// not JLE
	const auto zf = eflags & 0x40;
	const auto sf = eflags & 0x80;
	const auto of = eflags & 0x800;
	if (!(zf || sf != of)) {
		RenderDX::UpdateScreen(DSurface::Primary);
	}
	return 0;
}

DEFINE_HOOK(0x5D1F15, DXRender_UpdateScreen_MSEngine_Frame_Update, 0x5) 	{
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_HOOK(0x690228, DXRender_UpdateScreen_ScoreClass_Call_Back_Delay, 0x6) {
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_NAKED_HOOK(0x5C0477, DXRender_UpdateScreen_Movie_Blit_To_Screen) {
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

// Windows controls
static LRESULT CALLBACK OwnerDraw_Window_Procedure_(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	auto result = reinterpret_cast<LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM)>(0x610CA0)(hwnd, msg, wparam, lparam);
	if (msg == WM_PAINT) {
		RenderDX::UpdateScreen(DSurface::Primary);
	}
	return result;
}
DEFINE_PATCH_TYPED(void*, 0x60FF06, OwnerDraw_Window_Procedure_);

DEFINE_HOOK_AGAIN(0x611FB0, DXRender_UpdateScreen_OwnerDraw_Window, 0x6);
DEFINE_HOOK(0x61187D, DXRender_UpdateScreen_OwnerDraw_Window, 0xA) {
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
}

DEFINE_HOOK(0x7776B5, MainWindowProc_WMPAINT, 0x6) {
	RenderDX::MainProcHandlePaint();
	return 0x7779B5;
}

// Call Set_Video_Mode even when windowed.
DEFINE_JUMP(LJMP, 0x6BD9D9, 0x6BDA61);
DEFINE_JUMP(LJMP, 0x6BDB16, 0x6BDB6D);

// Disable DirectDraw.
DEFINE_JUMP(LJMP, 0x4A3FD0, 0x4A4019); // Skip Prep_Direct_Draw
DEFINE_FUNCTION_JUMP(LJMP, 0x4A4900, RenderDX::EnumDisplayModes);
