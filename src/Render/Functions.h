#pragma once

#include <GeneralStructures.h>

class Surface;

class RenderDX {
public:
	static bool __fastcall AllocateSurfaces(const RectangleStruct& hiddenRect, const RectangleStruct& compositeRect, const RectangleStruct& tileRect, const RectangleStruct& sidebarRect, bool hiddenFirst);
	static bool __fastcall SetVideoMode(HWND, int width, int height, int bitsPerPixel);
	static void __fastcall ResetVideoMode();
	static void __fastcall CreateMainWindow(HINSTANCE instance, int cmdShow, int width, int height);
	static void __fastcall DestroyMainWindow();
	static bool __fastcall UpdateScreen(Surface* pSurface);
	static bool __fastcall ShouldScale();
	static bool __fastcall ChangeDisplayMode(int width, int height);
	static float __fastcall GetXScale();
	static float __fastcall GetYScale();
	static int __fastcall ClientToRenderX(int x);
	static int __fastcall ClientToRenderY(int y);
	static void __fastcall UpdateScale();
	static void __fastcall ResetScale();
	static int* __fastcall EnumDisplayModes(DWORD minWidth, DWORD minHeight, DWORD maxWidth, DWORD maxHeight, DWORD bitDepth);
	static void __fastcall MainProcHandlePaint();
};
