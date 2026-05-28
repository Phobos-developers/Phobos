#include "Functions.h"

#include <Utilities/Debug.h>

#include "Surface.h"
#include "Renderer.h"
#include "Mouse.h"
#include "Options.h"
#include "../OwnerDraw/OwnerDraw.h"

#include <Unsorted.h>
#include <Drawing.h>
#include <GameOptionsClass.h>
#include <MouseClass.h>

#include <windowsx.h>
#include <ShellScalingApi.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

DEFINE_REFERENCE(RectangleStruct, DisplayVisibleRect, 0x886FB0u)

static bool OwnerDrawRectsAlreadyCaptured = false;

void __fastcall RenderDX::SetHighDPIAwareness()
{
	Debug::Log("[RenderDX] Setting high DPI awareness\n");

	using SetProcessDpiAwarenessContextFunc = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
	using SetProcessDpiAwarenessFunc = HRESULT(WINAPI*)(PROCESS_DPI_AWARENESS);
	using SetProcessDPIAwareFunc = BOOL(WINAPI*)();

	const HMODULE hUser32 = ::GetModuleHandleA("user32.dll");
	const HMODULE hShcore = ::GetModuleHandleA("shcore.dll");

	// Try to set the highest level of DPI awareness available, but don't fail if it's not supported (e.g. on Windows 7)
	if (hUser32)
	{
		const auto FnSetProcessDpiAwarenessContext = reinterpret_cast<SetProcessDpiAwarenessContextFunc>(
			::GetProcAddress(hUser32, "SetProcessDpiAwarenessContext"));

		if (FnSetProcessDpiAwarenessContext)
		{
			Debug::Log("[RenderDX] Setting DPI awareness context to PER_MONITOR_AWARE_V2\n");
			if (!FnSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
			{
				DWORD error = ::GetLastError();
				if (error == ERROR_ACCESS_DENIED)
					Debug::Log("[RenderDX] SetProcessDpiAwarenessContext failed: Access denied (already set to a different context)\n");
				else
					Debug::Log("[RenderDX] SetProcessDpiAwarenessContext failed: %d\n", error);
			}
			else
			{

				Debug::Log("[RenderDX] SetProcessDpiAwarenessContext succeeded\n");
				return;
			}
		}
	}

	// If SetProcessDpiAwarenessContext is not available or failed, try SetProcessDpiAwareness
	if (hShcore)
	{
		const auto FnSetProcessDpiAwareness = reinterpret_cast<SetProcessDpiAwarenessFunc>(
			::GetProcAddress(hShcore, "SetProcessDpiAwareness"));

		if (FnSetProcessDpiAwareness)
		{
			Debug::Log("[RenderDX] Setting process DPI awareness to PROCESS_PER_MONITOR_DPI_AWARE\n");
			const HRESULT result = FnSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			if (FAILED(result))
			{
				if (result == E_ACCESSDENIED)
					Debug::Log("[RenderDX] SetProcessDpiAwareness failed: Access denied (already set to a different level of awareness)\n");
				else
					Debug::Log("[RenderDX] SetProcessDpiAwareness failed: 0x%08X\n", static_cast<DWORD>(result));
			}
			else
			{
				Debug::Log("[RenderDX] SetProcessDpiAwareness succeeded\n");
				return;
			}
		}
	}

	// If neither of the above are available, fall back to SetProcessDPIAware (Windows 7 and earlier)
	if (hUser32)
	{
		const auto FnSetProcessDPIAware = reinterpret_cast<SetProcessDPIAwareFunc>(
			::GetProcAddress(hUser32, "SetProcessDPIAware"));

		if (FnSetProcessDPIAware)
		{
			Debug::Log("[RenderDX] Setting process DPI awareness with SetProcessDPIAware\n");
			if (!FnSetProcessDPIAware())
				Debug::Log("[RenderDX] SetProcessDPIAware failed: %d\n", ::GetLastError());
			else
				Debug::Log("[RenderDX] SetProcessDPIAware succeeded\n");
		}
	}

	Debug::Log("[RenderDX] Failed to set high DPI awareness. The application may not scale correctly on high DPI displays.\n");
}

static void ReleasePrimarySurface()
{
	auto pPrimary = DSurface::Primary;
	if (!pPrimary)
		return;

	DSurface::Primary = nullptr;
	Debug::Log("[RenderDX] Deleting old primary surface\n");
	GameDelete(pPrimary);
}

bool __fastcall RenderDX::AllocateSurfaces(const RectangleStruct& hiddenRect, const RectangleStruct& compositeRect, const RectangleStruct& tileRect, const RectangleStruct& sidebarRect, bool hiddenFirst)
{
	Debug::Log("[RenderDX] Allocating new surfaces\n");

	if (DSurface::Alternate)
	{
		Debug::Log("[RenderDX] Deleting AlternateSurface\n");
		GameDelete(DSurface::Alternate);
		DSurface::Alternate = nullptr;
	}

	if (DSurface::Hidden)
	{
		Debug::Log("[RenderDX] Deleting HiddenSurface\n");
		GameDelete(DSurface::Hidden);
		DSurface::Hidden = nullptr;
	}

	if (DSurface::Composite)
	{
		Debug::Log("[RenderDX] Deleting CompositeSurface\n");
		GameDelete(DSurface::Composite);
		DSurface::Composite = nullptr;
	}

	if (DSurface::Tile)
	{
		Debug::Log("[RenderDX] Deleting TileSurface\n");
		GameDelete(DSurface::Tile);
		DSurface::Tile = nullptr;
	}

	if (DSurface::Sidebar)
	{
		Debug::Log("[RenderDX] Deleting SidebarSurface\n");
		GameDelete(DSurface::Sidebar);
		DSurface::Sidebar = nullptr;
	}

	if (hiddenFirst && hiddenRect.Width > 0 && hiddenRect.Height > 0)
	{
		DSurface::Hidden = GameCreate<DXSurface>(hiddenRect.Width, hiddenRect.Height);
		DSurface::Hidden->Fill(0);
		Debug::Log("[RenderDX] HiddenSurface (%dx%d)\n", hiddenRect.Width, hiddenRect.Height);
	}

	if (compositeRect.Width > 0 && compositeRect.Height > 0)
	{
		DSurface::Composite = GameCreate<DXSurface>(compositeRect.Width, compositeRect.Height);
		DSurface::Composite->Fill(0);
		Debug::Log("[RenderDX] CompositeSurface (%dx%d)\n", compositeRect.Width, compositeRect.Height);
	}

	if (tileRect.Width > 0 && tileRect.Height > 0)
	{
		DSurface::Tile = GameCreate<DXSurface>(tileRect.Width, tileRect.Height);
		DSurface::Tile->Fill(0);
		Debug::Log("[RenderDX] TileSurface (%dx%d)\n", tileRect.Width, tileRect.Height);
	}

	if (sidebarRect.Width > 0 && sidebarRect.Height > 0)
	{
		DSurface::Sidebar = GameCreate<DXSurface>(sidebarRect.Width, sidebarRect.Height);
		DSurface::Sidebar->Fill(0);
		Debug::Log("[RenderDX] SidebarSurface (%dx%d)\n", sidebarRect.Width, sidebarRect.Height);
	}

	if (!hiddenFirst && hiddenRect.Width > 0 && hiddenRect.Height > 0)
	{
		DSurface::Hidden = GameCreate<DXSurface>(hiddenRect.Width, hiddenRect.Height);
		DSurface::Hidden->Fill(0);
		Debug::Log("[RenderDX] HiddenSurface (%dx%d)\n", hiddenRect.Width, hiddenRect.Height);
	}

	if (hiddenRect.Width > 0 && hiddenRect.Height > 0)
	{
		DSurface::Alternate = GameCreate<DXSurface>(hiddenRect.Width, hiddenRect.Height);
		DSurface::Alternate->Fill(0);
		Debug::Log("[RenderDX] AlternateSurface (%dx%d)\n", hiddenRect.Width, hiddenRect.Height);
	}

	return true;
}

bool __fastcall RenderDX::SetVideoMode(HWND, int width, int height, int bitsPerPixel)
{
	Debug::Log("[RenderDX] Setting video mode to %dx%d@%d\n", width, height, bitsPerPixel);

	if (!DXRenderer::Instance().IsRendererReady())
	{
		Debug::Log("[RenderDX] Renderer is not ready\n");
		return false;
	}

	WWUI::CaptureOwnerDrawWindowRects();
	OwnerDrawRectsAlreadyCaptured = true;

	ResetVideoMode();
	if (!DXRenderer::Instance().CreateRenderer(width, height, bitsPerPixel))
	{
		OwnerDrawRectsAlreadyCaptured = false;
		Debug::Log("[RenderDX] Failed to create renderer\n");
		return false;
	}

	Drawing::RenderWidth = width;
	Drawing::RenderHeight = height;
	Drawing::RenderBitsPerPixel = bitsPerPixel;
	DSurface::ViewBounds = RectangleStruct { 0, 0, width, height };
	DisplayVisibleRect = DSurface::ViewBounds;

	RenderDX::UpdateScale();
	OwnerDrawRectsAlreadyCaptured = false;

	return true;
}

void __fastcall RenderDX::ResetVideoMode()
{
	Debug::Log("[RenderDX] Resetting video mode\n");

	ReleasePrimarySurface();
	DXRenderer::Instance().DestroyRenderer();

	Drawing::RenderWidth = 0;
	Drawing::RenderHeight = 0;
	Drawing::RenderBitsPerPixel = 0;

	RenderDX::ResetScale();
}

static bool WindowResizeInProgress = false;
static bool DeferredWindowResize = false;
static int DeferredWindowWidth = 0;
static int DeferredWindowHeight = 0;

static void RecalcMouseWindowRegion(bool rebuildCursor)
{
	if (!DXMouse::Instance)
		return;

	DXMouse::Instance->RecalcCaptureRegion();
	if (rebuildCursor)
		DXMouse::Instance->RebuildCursorImage();
}

bool __fastcall RenderDX::HandleFullscreenToggleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (wParam != VK_RETURN || !(lParam & (1 << 29)))
		return false;

	if (message == WM_SYSKEYDOWN)
	{
		DXRenderer::Instance().ToggleFullscreen();
		RecalcMouseWindowRegion(true);
		return true;
	}

	return message == WM_SYSKEYUP || message == WM_SYSCHAR;
}

using ReinitMenuLayoutRectsFunc = void(__fastcall*)(int width, int height);

static void ReinitMenuLayoutRects(int width, int height)
{
	reinterpret_cast<ReinitMenuLayoutRectsFunc>(0x72E1B0)(width, height);
}

static void ApplyWindowResize(int width, int height)
{
	DXRenderer::Instance().ResizeWindow(width, height);
	RecalcMouseWindowRegion(true);
}

static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	{
		// Scale mouse inputs before they are processed by SDL or the game.
		if (RenderDX::ShouldScale())
		{
			POINT point { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			if (message == WM_MOUSEWHEEL)
				RenderDX::ScreenToRenderPoint(&point, true);
			else
				point = RenderDX::ClientToRenderPoint(point, true);

			lParam = MAKELPARAM(static_cast<WORD>(point.x), static_cast<WORD>(point.y));
		}
		break;
	}

	case WM_MOVE:
	{
		if (DXMouse::Instance)
		{
			DXMouse::Instance->RecalcCaptureRegion();
		}
		return 0; // handled
	}

	case WM_ENTERSIZEMOVE:
	{
		WindowResizeInProgress = true;
		DeferredWindowResize = false;
		DeferredWindowWidth = 0;
		DeferredWindowHeight = 0;
		return 0; // handled
	}

	case WM_EXITSIZEMOVE:
	{
		WindowResizeInProgress = false;

		if (DeferredWindowResize)
		{
			int width = DeferredWindowWidth;
			int height = DeferredWindowHeight;

			RECT clientRect {};
			if (::GetClientRect(hWnd, &clientRect))
			{
				width = clientRect.right - clientRect.left;
				height = clientRect.bottom - clientRect.top;
			}

			if (width > 0 && height > 0)
				ApplyWindowResize(width, height);

			DeferredWindowResize = false;
			DeferredWindowWidth = 0;
			DeferredWindowHeight = 0;
		}

		return 0; // handled
	}

	case WM_SIZE:
	{
		const int width = LOWORD(lParam);
		const int height = HIWORD(lParam);
		if (wParam == SIZE_MINIMIZED || width == 0 || height == 0)
		{
			DeferredWindowResize = false;
			RecalcMouseWindowRegion(false);
		}
		else if (WindowResizeInProgress)
		{
			DeferredWindowResize = true;
			DeferredWindowWidth = width;
			DeferredWindowHeight = height;
			RecalcMouseWindowRegion(false);
		}
		else
		{
			ApplyWindowResize(width, height);
		}

		return 0; // handled
	}

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	{
		if (RenderDX::HandleFullscreenToggleMessage(message, wParam, lParam))
			return 0; // handled
		break;
	}

	case WM_SETCURSOR:
	{
		// Prevent the system from setting the cursor when it's over our window, since we handle it ourselves.
		if (LOWORD(lParam) == HTCLIENT)
		{
			if (DXMouse::Instance)
				DXMouse::Instance->SetCachedCursor();
			return TRUE; // handled
		}
		break;
	}

	case WM_ACTIVATEAPP:
	{
		if (RenderOptions::Config().PauseGameWhenLoseFocus)
			break; // goto the original window procedure to allow the game to pause when losing focus
		if (hWnd == Game::hWnd)
		{
			Unsorted::GameInFocus = true; // game is always active
			if (wParam)
			{
				Debug::Log("[RenderDX] Game window activated\n");
				if (DXMouse::Instance)
					DXMouse::Instance->CaptureMouse();
			}
			else
			{
				Debug::Log("[RenderDX] Game window deactivated\n");
				if (DXMouse::Instance)
					DXMouse::Instance->ReleaseMouse();
			}
		}
		return 0; // handled - prevent the game from pausing when the window is deactivated
	}
	}

	// Call original window procedure for default processing
	return reinterpret_cast<LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM)>(0x7775C0)(hWnd, message, wParam, lParam);
}

void __fastcall RenderDX::CreateMainWindow(HINSTANCE instance, int cmdShow, int width, int height)
{
	Debug::Log("[RenderDX] Creating main window\n");
	SetHighDPIAwareness();
	if (!DXRenderer::Instance().CreateMainWindow(instance, cmdShow, width, height, MainWindowProc))
	{
		Debug::Log("[RenderDX] Failed to create main window\n");
		::MessageBoxA(nullptr, "Failed to create main window", "Error", MB_ICONERROR);
		::ExitProcess(0xC0DEBEEF);
	}
}

void __fastcall RenderDX::DestroyMainWindow()
{
	Debug::Log("[RenderDX] Destroying main window\n");
	DXRenderer::Instance().DestroyMainWindow();
}

bool __fastcall RenderDX::UpdateScreen(Surface* pSurface)
{
	if (!pSurface)
	{
		Debug::Log("[RenderDX] UpdateScreen called with null surface\n");
		return false;
	}

	const bool shouldScale = ShouldScale();
	DXRenderer::Instance().SetRenderScale(shouldScale);

	// Retrieve the game surface data
	if (void* pPixels = pSurface->Lock(0, 0))
	{
		if (!DXRenderer::Instance().UploadSurfaceToTexture(pPixels, pSurface->GetPitch()))
		{
			Debug::Log("[RenderDX] Failed to upload surface to texture\n");
			pSurface->Unlock();
			return false;
		}
		pSurface->Unlock();
	}

	static bool scaled = ShouldScale();

	// Extra process on scaling change
	if (scaled != shouldScale)
	{
		scaled = shouldScale;
		if (DXMouse::Instance)
			DXMouse::Instance->RebuildCursorImage();
	}

	DXRenderer::Instance().Present();

	return true;
}

bool __fastcall RenderDX::ShouldScale()
{
	return true;
}

static RectangleStruct GetSidebarClipBounds(const RectangleStruct& viewRect)
{
	constexpr int sidebarWidth = 168;
	constexpr int bottomBarHeight = 32;

	return RectangleStruct
	{
		GameOptionsClass::Instance.SidebarMode ? 0 : sidebarWidth,
		0,
		viewRect.Width - sidebarWidth,
		viewRect.Height - bottomBarHeight
	};
}

static void RebuildDisplayState(const RectangleStruct& viewRect)
{
	constexpr int sidebarWidth = 168;
	const auto sidebarRect = GetSidebarClipBounds(viewRect);
	const RectangleStruct tacticalSurfaceRect { 0, 0, sidebarRect.Width, viewRect.Height };
	const RectangleStruct sidebarSurfaceRect { 0, 0, sidebarWidth, viewRect.Height };

	DSurface::ViewBounds = viewRect;
	DisplayVisibleRect = viewRect;
	Drawing::RenderWidth = viewRect.Width;
	Drawing::RenderHeight = viewRect.Height;

	DSurface::Primary = DXSurface::CreatePrimary();

	RenderDX::AllocateSurfaces(
		viewRect,
		tacticalSurfaceRect,
		tacticalSurfaceRect,
		sidebarSurfaceRect,
		false
	);
	DSurface::Temp = DSurface::Hidden;

	if (DXMouse::Instance)
	{
		DXMouse::Instance->RebuildCursorImage();
	}

	ReinitMenuLayoutRects(viewRect.Width, viewRect.Height);

	SidebarClass::Instance.Set_View_Dimensions(sidebarRect);
	SidebarClass::Instance.Init_IO();
	SidebarClass::Instance.Activate(1);
	SidebarClass::Instance.InitGUI();
	SidebarClass::Instance.MarkNeedsRedraw(2); // REDRAW_ALL
	DXMouse::Instance->ShowMouse();

	WWUI::RelayoutWindowsAfterDisplayModeChange();
}

bool __fastcall RenderDX::ChangeDisplayMode(int width, int height)
{
	Debug::Log("[RenderDX] Changing display mode to %dx%d\n", width, height);

	// Save current window position
	RectangleStruct oldRect = DSurface::ViewBounds;
	if (oldRect.Width <= 0 || oldRect.Height <= 0)
	{
		if (Drawing::RenderWidth > 0 && Drawing::RenderHeight > 0)
		{
			Debug::Log("[RenderDX] Current view bounds are invalid, using RenderWidth/RenderHeight\n");
			oldRect = RectangleStruct { 0, 0, Drawing::RenderWidth, Drawing::RenderHeight };
		}
	}

	const int oldRenderWidth = Drawing::RenderWidth;
	const int oldRenderHeight = Drawing::RenderHeight;
	const int oldRenderBpp = Drawing::RenderBitsPerPixel;

	int oldWindowX = 0;
	int oldWindowY = 0;
	int oldWindowWidth = DXRenderer::Instance().GetWindowWidth();
	int oldWindowHeight = DXRenderer::Instance().GetWindowHeight();

	DXMouse::Instance->HideMouse();

	ReleasePrimarySurface();

	if (DXRenderer::Instance().IsWindowed())
	{
		int windowWidth = width;
		int windowHeight = height;

		RECT temp;
		::GetWindowRect(Game::hWnd, &temp);
		oldWindowX = temp.left;
		oldWindowY = temp.top;
		oldWindowWidth = temp.right - temp.left;
		oldWindowHeight = temp.bottom - temp.top;

		int centerX = oldWindowX + oldWindowWidth / 2;
		int centerY = oldWindowY + oldWindowHeight / 2;

		int newX = centerX - windowWidth / 2;
		int newY = centerY - windowHeight / 2;

		DXRenderer::Instance().MoveWindow(newX, newY, windowWidth, windowHeight);

		Debug::Log("[RenderDX] Moved window to (%d, %d) with size %dx%d\n", newX, newY, windowWidth, windowHeight);
	}

	// Recreate all intermediates
	if (!SetVideoMode(Game::hWnd, width, height, 16))
	{
		if (DXRenderer::Instance().IsWindowed())
		{
			DXRenderer::Instance().MoveWindow(oldWindowX, oldWindowY, oldWindowWidth, oldWindowHeight);
			Debug::Log("[RenderDX] Restore window to (%d, %d) with size %dx%d\n", oldWindowX, oldWindowY, oldWindowWidth, oldWindowHeight);
		}

		if (oldRect.Width > 0 && oldRect.Height > 0 && oldRenderWidth > 0 && oldRenderHeight > 0)
		{
			Debug::Log("[RenderDX] Restoring old display mode.\n");
			if (!SetVideoMode(Game::hWnd, oldRenderWidth, oldRenderHeight, oldRenderBpp))
			{
				Debug::Log("[RenderDX] Failed to restore old display mode.\n");
				DXMouse::Instance->ShowMouse();
				return false;
			}
			RebuildDisplayState(oldRect);
		}
		else
		{
			Debug::Log("[RenderDX] Old view bounds are invalid, cannot restore\n");
		}

		DXMouse::Instance->ShowMouse();
		return false;
	}

	RectangleStruct newViewRect = { 0, 0, width, height };
	RebuildDisplayState(newViewRect);
	Debug::Log("[RenderDX]: ViewBounds: %dx%d\n", width, height);
	Debug::Log("[RenderDX] Mode change complete.\n");

	return true;
}

static float ScaleX = 1.0f;
static float ScaleY = 1.0f;
static float ViewportX = 0.0f;
static float ViewportY = 0.0f;
static bool OwnerDrawRawWindowCoordinates = false;

float __fastcall RenderDX::GetXScale()
{
	return ScaleX;
}

float __fastcall RenderDX::GetYScale()
{
	return ScaleY;
}

static int ClampRenderX(int x)
{
	if (Drawing::RenderWidth <= 0)
		return x;

	return std::clamp(x, 0, Drawing::RenderWidth - 1);
}

static int ClampRenderY(int y)
{
	if (Drawing::RenderHeight <= 0)
		return y;

	return std::clamp(y, 0, Drawing::RenderHeight - 1);
}

static RECT ClampRenderRect(const RECT& rect)
{
	RECT result = rect;
	if (Drawing::RenderWidth > 0)
	{
		result.left = std::clamp<LONG>(result.left, 0, static_cast<LONG>(Drawing::RenderWidth));
		result.right = std::clamp<LONG>(result.right, 0, static_cast<LONG>(Drawing::RenderWidth));
	}
	if (Drawing::RenderHeight > 0)
	{
		result.top = std::clamp<LONG>(result.top, 0, static_cast<LONG>(Drawing::RenderHeight));
		result.bottom = std::clamp<LONG>(result.bottom, 0, static_cast<LONG>(Drawing::RenderHeight));
	}
	return result;
}

static int ClientToRenderXRounded(int x)
{
	if (Drawing::RenderWidth <= 0)
		return x;

	return static_cast<int>(std::lround((static_cast<float>(x) - ViewportX) * ScaleX));
}

static int ClientToRenderYRounded(int y)
{
	if (Drawing::RenderHeight <= 0)
		return y;

	return static_cast<int>(std::lround((static_cast<float>(y) - ViewportY) * ScaleY));
}

static RECT ClientToRenderRectRounded(const RECT& rect)
{
	return RECT
	{
		ClientToRenderXRounded(rect.left),
		ClientToRenderYRounded(rect.top),
		ClientToRenderXRounded(rect.right),
		ClientToRenderYRounded(rect.bottom)
	};
}

int __fastcall RenderDX::ClientToRenderX(int x)
{
	if (Drawing::RenderWidth <= 0)
		return x;

	return ClampRenderX(ClientToRenderXUnclamped(x));
}

int __fastcall RenderDX::ClientToRenderY(int y)
{
	if (Drawing::RenderHeight <= 0)
		return y;

	return ClampRenderY(ClientToRenderYUnclamped(y));
}

int __fastcall RenderDX::ClientToRenderXUnclamped(int x)
{
	if (Drawing::RenderWidth <= 0)
		return x;

	return static_cast<int>((static_cast<float>(x) - ViewportX) * ScaleX);
}

int __fastcall RenderDX::ClientToRenderYUnclamped(int y)
{
	if (Drawing::RenderHeight <= 0)
		return y;

	return static_cast<int>((static_cast<float>(y) - ViewportY) * ScaleY);
}

int __fastcall RenderDX::RenderToClientX(int x)
{
	if (Drawing::RenderWidth <= 0 || ScaleX == 0.0f)
		return x;

	return static_cast<int>(std::lround(ViewportX + static_cast<float>(x) / ScaleX));
}

int __fastcall RenderDX::RenderToClientY(int y)
{
	if (Drawing::RenderHeight <= 0 || ScaleY == 0.0f)
		return y;

	return static_cast<int>(std::lround(ViewportY + static_cast<float>(y) / ScaleY));
}

POINT __fastcall RenderDX::ClientToRenderPoint(POINT point, bool clamp)
{
	point.x = clamp ? ClientToRenderX(point.x) : ClientToRenderXUnclamped(point.x);
	point.y = clamp ? ClientToRenderY(point.y) : ClientToRenderYUnclamped(point.y);
	return point;
}

POINT __fastcall RenderDX::RenderToClientPoint(POINT point)
{
	point.x = RenderToClientX(point.x);
	point.y = RenderToClientY(point.y);
	return point;
}

RECT __fastcall RenderDX::ClientToRenderRect(const RECT& rect, bool clamp)
{
	RECT result
	{
		ClientToRenderXUnclamped(rect.left),
		ClientToRenderYUnclamped(rect.top),
		ClientToRenderXUnclamped(rect.right),
		ClientToRenderYUnclamped(rect.bottom)
	};

	return clamp ? ClampRenderRect(result) : result;
}

RECT __fastcall RenderDX::RenderToClientRect(const RECT& rect)
{
	if (Drawing::RenderWidth <= 0 || Drawing::RenderHeight <= 0 || ScaleX == 0.0f || ScaleY == 0.0f)
		return rect;

	return RECT
	{
		static_cast<LONG>(std::floor(ViewportX + static_cast<float>(rect.left) / ScaleX)),
		static_cast<LONG>(std::floor(ViewportY + static_cast<float>(rect.top) / ScaleY)),
		static_cast<LONG>(std::ceil(ViewportX + static_cast<float>(rect.right) / ScaleX)),
		static_cast<LONG>(std::ceil(ViewportY + static_cast<float>(rect.bottom) / ScaleY))
	};
}

bool __fastcall RenderDX::ScreenToRenderPoint(LPPOINT pPoint, bool clamp)
{
	if (!pPoint)
		return false;

	if (!::ScreenToClient(Game::hWnd, pPoint))
		return false;

	*pPoint = ClientToRenderPoint(*pPoint, clamp);
	return true;
}

bool __fastcall RenderDX::RenderToScreenPoint(LPPOINT pPoint)
{
	if (!pPoint)
		return false;

	*pPoint = RenderToClientPoint(*pPoint);
	return ::ClientToScreen(Game::hWnd, pPoint) != FALSE;
}

bool __fastcall RenderDX::GetWindowRectInRender(HWND hWnd, LPRECT pRect)
{
	if (!hWnd || !pRect)
		return false;

	if (hWnd == Game::hWnd)
	{
		pRect->left = 0;
		pRect->top = 0;
		pRect->right = Drawing::RenderWidth;
		pRect->bottom = Drawing::RenderHeight;
		return true;
	}

	RECT windowRect {};
	if (!::GetWindowRect(hWnd, &windowRect))
		return false;

	POINT topLeft { windowRect.left, windowRect.top };
	POINT bottomRight { windowRect.right, windowRect.bottom };
	if (!::ScreenToClient(Game::hWnd, &topLeft) || !::ScreenToClient(Game::hWnd, &bottomRight))
		return false;

	const RECT clientRect { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
	*pRect = ClientToRenderRectRounded(clientRect);
	return true;
}

bool __fastcall RenderDX::GetClientRectInRender(HWND hWnd, LPRECT pRect)
{
	if (!hWnd || !pRect)
		return false;

	if (hWnd == Game::hWnd)
	{
		pRect->left = 0;
		pRect->top = 0;
		pRect->right = Drawing::RenderWidth;
		pRect->bottom = Drawing::RenderHeight;
		return true;
	}

	RECT clientRect {};
	if (!::GetClientRect(hWnd, &clientRect))
		return false;

	POINT topLeft { clientRect.left, clientRect.top };
	POINT bottomRight { clientRect.right, clientRect.bottom };
	if (!::ClientToScreen(hWnd, &topLeft) || !::ClientToScreen(hWnd, &bottomRight))
		return false;

	if (!::ScreenToClient(Game::hWnd, &topLeft) || !::ScreenToClient(Game::hWnd, &bottomRight))
		return false;

	const RECT renderRect = ClientToRenderRectRounded(RECT { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y });
	pRect->left = 0;
	pRect->top = 0;
	pRect->right = renderRect.right - renderRect.left;
	pRect->bottom = renderRect.bottom - renderRect.top;
	return true;
}

POINT __fastcall RenderDX::ScreenToRenderLocalPoint(HWND hWnd, POINT point)
{
	if (!ScreenToRenderPoint(&point, false))
		return point;

	RECT windowRect {};
	if (GetWindowRectInRender(hWnd, &windowRect))
	{
		point.x -= windowRect.left;
		point.y -= windowRect.top;
	}

	return point;
}

RECT __fastcall RenderDX::ClientToRenderLocalRect(HWND hWnd, const RECT& rect)
{
	POINT topLeft { rect.left, rect.top };
	POINT bottomRight { rect.right, rect.bottom };
	if (!::ClientToScreen(hWnd, &topLeft) || !::ClientToScreen(hWnd, &bottomRight))
		return rect;

	topLeft = ScreenToRenderLocalPoint(hWnd, topLeft);
	bottomRight = ScreenToRenderLocalPoint(hWnd, bottomRight);

	return RECT { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
}

RECT __fastcall RenderDX::RenderLocalToClientRect(HWND hWnd, const RECT& rect)
{
	RECT windowRect {};
	if (!GetWindowRectInRender(hWnd, &windowRect))
		return rect;

	const RECT renderRect
	{
		windowRect.left + rect.left,
		windowRect.top + rect.top,
		windowRect.left + rect.right,
		windowRect.top + rect.bottom
	};

	RECT clientRect {};
	if (!RenderRectToClient(hWnd, renderRect, &clientRect))
		return rect;

	return clientRect;
}

POINT __fastcall RenderDX::MouseLParamToRenderLocalPoint(HWND hWnd, LPARAM lParam)
{
	POINT point { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if (!::ClientToScreen(hWnd, &point))
		return point;

	return ScreenToRenderLocalPoint(hWnd, point);
}

LPARAM __fastcall RenderDX::MouseLParamToRenderLocal(HWND hWnd, LPARAM lParam)
{
	const POINT point = MouseLParamToRenderLocalPoint(hWnd, lParam);
	return MAKELPARAM(static_cast<WORD>(point.x), static_cast<WORD>(point.y));
}

bool __fastcall RenderDX::RenderRectToClient(HWND referenceHwnd, const RECT& renderRect, LPRECT pClientRect)
{
	if (!pClientRect)
		return false;

	POINT topLeft { renderRect.left, renderRect.top };
	POINT bottomRight { renderRect.right, renderRect.bottom };
	if (!RenderToScreenPoint(&topLeft) || !RenderToScreenPoint(&bottomRight))
		return false;

	if (referenceHwnd)
	{
		if (!::ScreenToClient(referenceHwnd, &topLeft) || !::ScreenToClient(referenceHwnd, &bottomRight))
			return false;
	}

	pClientRect->left = topLeft.x;
	pClientRect->top = topLeft.y;
	pClientRect->right = bottomRight.x;
	pClientRect->bottom = bottomRight.y;
	return true;
}

static bool GetParentRenderOrigin(HWND hWnd, POINT& origin)
{
	origin = { 0, 0 };

	if (!hWnd || hWnd == Game::hWnd)
		return true;

	RECT parentRect {};
	if (!RenderDX::GetWindowRectInRender(hWnd, &parentRect))
		return false;

	origin.x = parentRect.left;
	origin.y = parentRect.top;
	return true;
}

static bool GetWindowRectInParentRender(HWND hWnd, LPRECT pRect)
{
	if (!RenderDX::GetWindowRectInRender(hWnd, pRect))
		return false;

	POINT parentOrigin {};
	if (!GetParentRenderOrigin(::GetParent(hWnd), parentOrigin))
		return false;

	pRect->left -= parentOrigin.x;
	pRect->right -= parentOrigin.x;
	pRect->top -= parentOrigin.y;
	pRect->bottom -= parentOrigin.y;
	return true;
}

static bool RenderLocalRectToWindowRect(HWND hWnd, const RECT& localRect, LPRECT pWindowRect)
{
	const HWND parentHwnd = ::GetParent(hWnd);
	POINT parentOrigin {};
	if (!GetParentRenderOrigin(parentHwnd, parentOrigin))
		return false;

	const RECT renderRect
	{
		parentOrigin.x + localRect.left,
		parentOrigin.y + localRect.top,
		parentOrigin.x + localRect.right,
		parentOrigin.y + localRect.bottom
	};

	return RenderDX::RenderRectToClient(parentHwnd, renderRect, pWindowRect);
}

BOOL __fastcall RenderDX::MoveWindowInRender(HWND hWnd, int x, int y, int width, int height, BOOL repaint)
{
	if (!hWnd)
		return FALSE;

	RECT clientRect {};
	const RECT renderRect { x, y, x + width, y + height };
	if (!RenderLocalRectToWindowRect(hWnd, renderRect, &clientRect))
		return FALSE;

	return ::MoveWindow(
		hWnd,
		clientRect.left,
		clientRect.top,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		repaint);
}

BOOL __fastcall RenderDX::SetWindowPosInRender(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT flags)
{
	if (!hWnd)
		return FALSE;

	RECT renderRect {};
	if (!GetWindowRectInParentRender(hWnd, &renderRect))
		return FALSE;

	if (!(flags & SWP_NOMOVE))
	{
		const int width = renderRect.right - renderRect.left;
		const int height = renderRect.bottom - renderRect.top;
		renderRect.left = x;
		renderRect.top = y;
		renderRect.right = x + width;
		renderRect.bottom = y + height;
	}

	if (!(flags & SWP_NOSIZE))
	{
		renderRect.right = renderRect.left + cx;
		renderRect.bottom = renderRect.top + cy;
	}

	RECT clientRect {};
	if (!RenderLocalRectToWindowRect(hWnd, renderRect, &clientRect))
		return FALSE;

	const int clientWidth = clientRect.right - clientRect.left;
	const int clientHeight = clientRect.bottom - clientRect.top;
	return ::SetWindowPos(
		hWnd,
		hWndInsertAfter,
		clientRect.left,
		clientRect.top,
		clientWidth,
		clientHeight,
		flags);
}

bool __fastcall RenderDX::IsOwnerDrawUsingRawWindowCoordinates()
{
	return OwnerDrawRawWindowCoordinates;
}

void __fastcall RenderDX::SetOwnerDrawRawWindowCoordinates(bool enabled)
{
	OwnerDrawRawWindowCoordinates = enabled;
}

void __fastcall RenderDX::UpdateScale()
{
	if (!OwnerDrawRectsAlreadyCaptured)
		WWUI::CaptureOwnerDrawWindowRects();

	const float viewportWidth = DXRenderer::Instance().GetViewportWidth();
	const float viewportHeight = DXRenderer::Instance().GetViewportHeight();
	const float viewportX = DXRenderer::Instance().GetViewportX();
	const float viewportY = DXRenderer::Instance().GetViewportY();

	if (Drawing::RenderWidth <= 0 || Drawing::RenderHeight <= 0 || viewportWidth <= 0.0f || viewportHeight <= 0.0f)
	{
		ResetScale();
		WWUI::ApplyOwnerDrawWindowRects();
		return;
	}

	ViewportX = viewportX;
	ViewportY = viewportY;
	ScaleX = static_cast<float>(Drawing::RenderWidth) / viewportWidth;
	ScaleY = static_cast<float>(Drawing::RenderHeight) / viewportHeight;
	WWUI::ApplyOwnerDrawWindowRects();
}

void __fastcall RenderDX::ResetScale()
{
	ScaleX = 1.0f;
	ScaleY = 1.0f;
	ViewportX = 0.0f;
	ViewportY = 0.0f;
}

int* __fastcall RenderDX::EnumDisplayModes(DWORD minWidth, DWORD minHeight, DWORD maxWidth, DWORD maxHeight, DWORD)
{
	std::vector<std::pair<int, int>> modes;
	DEVMODE devmode {};
	DWORD modeIndex = 0;

	while (::EnumDisplaySettingsA(nullptr, modeIndex++, &devmode))
	{
		const DWORD w = devmode.dmPelsWidth;
		const DWORD h = devmode.dmPelsHeight;
		const DWORD bpp = devmode.dmBitsPerPel;

		if (w >= minWidth && h >= minHeight && w <= maxWidth && h <= maxHeight && bpp == 32)
		{
			modes.emplace_back(static_cast<int>(w), static_cast<int>(h));
		}
	}

	if (modes.empty())
	{
		return nullptr;
	}

	std::sort(modes.begin(), modes.end());
	modes.erase(std::unique(modes.begin(), modes.end()), modes.end());

	const size_t count = modes.size();
	const size_t bytes = sizeof(int) * (count * 2 + 1);

	int* list = static_cast<int*>(YRMemory::Allocate(bytes));
	std::memset(list, 0, bytes);

	int* ptr = list;
	for (const auto& mode : modes)
	{
		*ptr++ = mode.first;
		*ptr++ = mode.second;
	}

	return list;
}

void __fastcall RenderDX::MainProcHandlePaint()
{
	if (DXMouse::Instance && DSurface::Primary && DSurface::Hidden && DSurface::Composite)
	{
		if (Unsorted::ScenarioStarted)
		{
			if (WWUI::HasActiveOwnerDrawDialog())
			{
				RenderDX::UpdateScreen(DSurface::Primary);
				return;
			}

			GScreenClass::UpdatePrimarySurface(DXMouse::Instance->IsCaptured(), DSurface::Composite, nullptr);
			SidebarClass::Instance.BlitSidebar(true);
		}
		else if (Game::IsMoviePlaying())
		{
			Game::BlitMovie();
		}
		else
		{
			GScreenClass::UpdatePrimarySurface(DXMouse::Instance->IsCaptured(), DSurface::Hidden, nullptr);
		}
	}
}
