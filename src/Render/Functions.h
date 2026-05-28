#pragma once

#include <GeneralStructures.h>

class Surface;

class RenderDX {
public:
	static bool __fastcall AllocateSurfaces(const RectangleStruct& hiddenRect, const RectangleStruct& compositeRect, const RectangleStruct& tileRect, const RectangleStruct& sidebarRect, bool hiddenFirst);
	static bool __fastcall SetVideoMode(HWND, int width, int height, int bitsPerPixel);
	static void __fastcall SetHighDPIAwareness();
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
	static int __fastcall ClientToRenderXUnclamped(int x);
	static int __fastcall ClientToRenderYUnclamped(int y);
	static int __fastcall RenderToClientX(int x);
	static int __fastcall RenderToClientY(int y);
	static POINT __fastcall ClientToRenderPoint(POINT point, bool clamp);
	static POINT __fastcall RenderToClientPoint(POINT point);
	static RECT __fastcall ClientToRenderRect(const RECT& rect, bool clamp);
	static RECT __fastcall RenderToClientRect(const RECT& rect);
	static bool __fastcall ScreenToRenderPoint(LPPOINT pPoint, bool clamp);
	static bool __fastcall RenderToScreenPoint(LPPOINT pPoint);
	static bool __fastcall GetWindowRectInRender(HWND hWnd, LPRECT pRect);
	static bool __fastcall GetClientRectInRender(HWND hWnd, LPRECT pRect);
	static POINT __fastcall MouseLParamToRenderLocalPoint(HWND hWnd, LPARAM lParam);
	static LPARAM __fastcall MouseLParamToRenderLocal(HWND hWnd, LPARAM lParam);
	static POINT __fastcall ScreenToRenderLocalPoint(HWND hWnd, POINT point);
	static RECT __fastcall ClientToRenderLocalRect(HWND hWnd, const RECT& rect);
	static RECT __fastcall RenderLocalToClientRect(HWND hWnd, const RECT& rect);
	static bool __fastcall RenderRectToClient(HWND referenceHwnd, const RECT& renderRect, LPRECT pClientRect);
	static BOOL __fastcall MoveWindowInRender(HWND hWnd, int x, int y, int width, int height, BOOL repaint);
	static BOOL __fastcall SetWindowPosInRender(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT flags);
	static bool __fastcall IsOwnerDrawUsingRawWindowCoordinates();
	static void __fastcall SetOwnerDrawRawWindowCoordinates(bool enabled);
	static bool __fastcall HandleFullscreenToggleMessage(UINT message, WPARAM wParam, LPARAM lParam);
	static void __fastcall UpdateScale();
	static void __fastcall ResetScale();
	static int* __fastcall EnumDisplayModes(DWORD minWidth, DWORD minHeight, DWORD maxWidth, DWORD maxHeight, DWORD bitDepth);
	static void __fastcall MainProcHandlePaint();
};
