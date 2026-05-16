#pragma once

#include <GeneralStructures.h>

class Surface;

class RenderDX {
public:
	static bool __fastcall AllocateSurfaces(const RectangleStruct& hidden_rect, const RectangleStruct& composite_rect, const RectangleStruct& tile_rect, const RectangleStruct& sidebar_rect, bool hidden_first);
	static bool __fastcall SetVideoMode(HWND, int width, int height, int bits_per_pixel);
	static void __fastcall ResetVideoMode();
	static void __fastcall CreateMainWindow(HINSTANCE instance, int cmd_show, int width, int height);
	static void __fastcall DestroyMainWindow();
	static bool __fastcall UpdateScreen(Surface* surface);
	static bool __fastcall ShouldScale();
	static bool __fastcall ChangeDisplayMode(int width, int height);
	static float __fastcall GetXScale();
	static float __fastcall GetYScale();
	static int __fastcall ClientToRenderX(int x);
	static int __fastcall ClientToRenderY(int y);
	static void __fastcall UpdateScale();
	static void __fastcall ResetScale();
	static int* __fastcall EnumDisplayModes(DWORD minw, DWORD minh, DWORD maxw, DWORD maxh, DWORD bitdepth);
	static void __fastcall MainProcHandlePaint();
};
