#include "Functions.h"

#include <Utilities/Macro.h>

#include <Unsorted.h>
#include <Drawing.h>
#include <Surface.h>
#include <SessionClass.h>

DEFINE_FUNCTION_JUMP(LJMP, 0x777C30, RenderDX::CreateMainWindow);

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

DEFINE_NAKED_HOOK(0x5C0477, DXRenderUpdateScreenVQMovieBlitToScreen) {
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

DEFINE_HOOK(0x432FF1, DXRenderUpdateScreenBinkMovieBlitToScreen, 0x7)
{
	RenderDX::UpdateScreen(DSurface::Primary);
	return 0;
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
