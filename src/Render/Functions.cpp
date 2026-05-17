#include "Functions.h"

#include <Utilities/Debug.h>

#include "Surface.h"
#include "Renderer.h"
#include "Mouse.h"
#include "Options.h"

#include <Unsorted.h>
#include <Drawing.h>
#include <GameOptionsClass.h>
#include <MouseClass.h>

#include <windowsx.h>

#include <algorithm>
#include <cstring>
#include <vector>

bool __fastcall RenderDX::AllocateSurfaces(const RectangleStruct& hiddenRect, const RectangleStruct& compositeRect, const RectangleStruct& tileRect, const RectangleStruct& sidebarRect, bool hiddenFirst) {
	Debug::Log("[RenderDX] Allocating new surfaces\n");

	if (DSurface::Alternate) {
		Debug::Log("[RenderDX] Deleting AlternateSurface\n");
		GameDelete(DSurface::Alternate);
		DSurface::Alternate = nullptr;
	}

	if (DSurface::Hidden) {
		Debug::Log("[RenderDX] Deleting HiddenSurface\n");
		GameDelete(DSurface::Hidden);
		DSurface::Hidden = nullptr;
	}

	if (DSurface::Composite) {
		Debug::Log("[RenderDX] Deleting CompositeSurface\n");
		GameDelete(DSurface::Composite);
		DSurface::Composite = nullptr;
	}

	if (DSurface::Tile) {
		Debug::Log("[RenderDX] Deleting TileSurface\n");
		GameDelete(DSurface::Tile);
		DSurface::Tile = nullptr;
	}

	if (DSurface::Sidebar) {
		Debug::Log("[RenderDX] Deleting SidebarSurface\n");
		GameDelete(DSurface::Sidebar);
		DSurface::Sidebar = nullptr;
	}

	if (hiddenFirst && hiddenRect.Width > 0 && hiddenRect.Height > 0) {
		DSurface::Hidden = GameCreate<DXSurface>(hiddenRect.Width, hiddenRect.Height);
		DSurface::Hidden->Fill(0);
		Debug::Log("[RenderDX] HiddenSurface (%dx%d)\n", hiddenRect.Width, hiddenRect.Height);
	}

	if (compositeRect.Width > 0 && compositeRect.Height > 0) {
		DSurface::Composite = GameCreate<DXSurface>(compositeRect.Width, compositeRect.Height);
		DSurface::Composite->Fill(0);
		Debug::Log("[RenderDX] CompositeSurface (%dx%d)\n", compositeRect.Width, compositeRect.Height);
	}

	if (tileRect.Width > 0 && tileRect.Height > 0) {
		DSurface::Tile = GameCreate<DXSurface>(tileRect.Width, tileRect.Height);
		DSurface::Tile->Fill(0);
		Debug::Log("[RenderDX] TileSurface (%dx%d)\n", tileRect.Width, tileRect.Height);
	}

	if (sidebarRect.Width > 0 && sidebarRect.Height > 0) {
		DSurface::Sidebar = GameCreate<DXSurface>(sidebarRect.Width, sidebarRect.Height);
		DSurface::Sidebar->Fill(0);
		Debug::Log("[RenderDX] SidebarSurface (%dx%d)\n", sidebarRect.Width, sidebarRect.Height);
	}

	if (!hiddenFirst && hiddenRect.Width > 0 && hiddenRect.Height > 0) {
		DSurface::Hidden = GameCreate<DXSurface>(hiddenRect.Width, hiddenRect.Height);
		DSurface::Hidden->Fill(0);
		Debug::Log("[RenderDX] HiddenSurface (%dx%d)\n", hiddenRect.Width, hiddenRect.Height);
	}

	if (hiddenRect.Width > 0 && hiddenRect.Height > 0) {
		DSurface::Alternate = GameCreate<DXSurface>(hiddenRect.Width, hiddenRect.Height);
		DSurface::Alternate->Fill(0);
		Debug::Log("[RenderDX] AlternateSurface (%dx%d)\n", hiddenRect.Width, hiddenRect.Height);
	}

	return true;
}

bool __fastcall RenderDX::SetVideoMode(HWND, int width, int height, int bitsPerPixel) {
	Debug::Log("[RenderDX] Setting video mode to %dx%d@%d\n", width, height, bitsPerPixel);

	if (!DXRenderer::Instance().IsRendererReady()) {
		Debug::Log("[RenderDX] Renderer is not ready\n");
		return false;
	}

	ResetVideoMode();
	if (!DXRenderer::Instance().CreateRenderer(width, height, bitsPerPixel)) {
		Debug::Log("[RenderDX] Failed to create renderer\n");
		return false;
	}

	Drawing::RenderWidth = width;
	Drawing::RenderHeight = height;
	Drawing::RenderBitsPerPixel = bitsPerPixel;

	RenderDX::UpdateScale();

	return true;
}

void __fastcall RenderDX::ResetVideoMode() {
	Debug::Log("[RenderDX] Resetting video mode\n");

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

static void RecalcMouseWindowRegion(bool rebuildCursor) {
	if (!DXMouse::Instance)
		return;

	DXMouse::Instance->RecalcCaptureRegion();
	if (rebuildCursor)
		DXMouse::Instance->RebuildCursorImage();
}

static void ApplyWindowResize(int width, int height) {
	DXRenderer::Instance().ResizeWindow(width, height);
	RecalcMouseWindowRegion(true);
}

static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
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
		if (RenderDX::ShouldScale()) {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			x = RenderDX::ClientToRenderX(x);
			y = RenderDX::ClientToRenderY(y);

			lParam = MAKELPARAM(x, y);
		}
		break;
	}

	case WM_MOVE:
	{
		if (DXMouse::Instance) {
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

		if (DeferredWindowResize) {
			int width = DeferredWindowWidth;
			int height = DeferredWindowHeight;

			RECT clientRect {};
			if (::GetClientRect(hWnd, &clientRect)) {
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
		if (wParam == SIZE_MINIMIZED || width == 0 || height == 0) {
			DeferredWindowResize = false;
			RecalcMouseWindowRegion(false);
		}
		else if (WindowResizeInProgress) {
			DeferredWindowResize = true;
			DeferredWindowWidth = width;
			DeferredWindowHeight = height;
			RecalcMouseWindowRegion(false);
		}
		else {
			ApplyWindowResize(width, height);
		}

		return 0; // handled
	}

	case WM_SYSKEYDOWN:
	{
		// Handle Alt+Enter for fullscreen toggle
		if (wParam == VK_RETURN && (lParam & (1 << 29))) {
			DXRenderer::Instance().ToggleFullscreen();
			if (DXMouse::Instance) {
				DXMouse::Instance->RecalcCaptureRegion();
				DXMouse::Instance->RebuildCursorImage();
			}
			return 0; // handled
		}
		break;
	}

	case WM_SETCURSOR:
	{
		// Prevent the system from setting the cursor when it's over our window, since we handle it ourselves.
		if (LOWORD(lParam) == HTCLIENT) {
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
		if (hWnd == Game::hWnd) {
			Unsorted::GameInFocus = true; // game is always active
			if (wParam) {
				Debug::Log("[RenderDX] Game window activated\n");
				if (DXMouse::Instance)
					DXMouse::Instance->CaptureMouse();
			}
			else {
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

void __fastcall RenderDX::CreateMainWindow(HINSTANCE instance, int cmdShow, int width, int height) {
	Debug::Log("[RenderDX] Creating main window\n");
	if (!DXRenderer::Instance().CreateMainWindow(instance, cmdShow, width, height, MainWindowProc)) {
		Debug::Log("[RenderDX] Failed to create main window\n");
		::MessageBoxA(nullptr, "Failed to create main window", "Error", MB_ICONERROR);
		::ExitProcess(0xC0DEBEEF);
	}
}

void __fastcall RenderDX::DestroyMainWindow() {
	Debug::Log("[RenderDX] Destroying main window\n");
	DXRenderer::Instance().DestroyMainWindow();
}

bool __fastcall RenderDX::UpdateScreen(Surface* pSurface) {
	if (!pSurface) {
		Debug::Log("[RenderDX] UpdateScreen called with null surface\n");
		return false;
	}

	const bool shouldScale = ShouldScale();
	DXRenderer::Instance().SetRenderScale(shouldScale);

	// Retrieve the game surface data
	if (void* pPixels = pSurface->Lock(0, 0)) {
		if (!DXRenderer::Instance().UploadSurfaceToTexture(pPixels, pSurface->GetPitch())) {
			Debug::Log("[RenderDX] Failed to upload surface to texture\n");
			pSurface->Unlock();
			return false;
		}
		pSurface->Unlock();
	}

	static bool scaled = ShouldScale();

	// Extra process on scaling change
	if (scaled != shouldScale) {
		scaled = shouldScale;
		if (DXMouse::Instance)
			DXMouse::Instance->RebuildCursorImage();
	}

	DXRenderer::Instance().Present();

	return true;
}

bool __fastcall RenderDX::ShouldScale() {
	return Unsorted::SpecialDialog == 0 && Unsorted::WSDialogCount == 0;
}

static void RebuildDisplayState(const RectangleStruct& viewRect) {
	auto sidebarRect = viewRect;
	sidebarRect.X = GameOptionsClass::Instance.SidebarMode ? 0 : 168;
	sidebarRect.Y = 16;
	sidebarRect.Width -= 168;
	sidebarRect.Height -= 16;

	DSurface::ViewBounds = viewRect;
	Drawing::RenderWidth = viewRect.Width;
	Drawing::RenderHeight = viewRect.Height;

	DSurface::Primary = DXSurface::CreatePrimary();

	RenderDX::AllocateSurfaces(
		viewRect,
		RectangleStruct { 0, 0, sidebarRect.Width, viewRect.Height },
		RectangleStruct { 0, 0, sidebarRect.Width, viewRect.Height },
		RectangleStruct { 0, 0, 168, viewRect.Height },
		false
	);
	DSurface::Temp = DSurface::Hidden;

	if (DXMouse::Instance) {
		DXMouse::Instance->RebuildCursorImage();
	}

	SidebarClass::Instance.Set_View_Dimensions(sidebarRect);
	SidebarClass::Instance.Init_IO();
	SidebarClass::Instance.Activate(1);
	SidebarClass::Instance.InitGUI();
	SidebarClass::Instance.MarkNeedsRedraw(2); // REDRAW_ALL
	DXMouse::Instance->ShowMouse();
}

bool __fastcall RenderDX::ChangeDisplayMode(int width, int height) {
	Debug::Log("[RenderDX] Changing display mode to %dx%d\n", width, height);

	// Save current window position
	RectangleStruct oldRect = DSurface::ViewBounds;
	if (oldRect.Width <= 0 || oldRect.Height <= 0) {
		if (Drawing::RenderWidth > 0 && Drawing::RenderHeight > 0) {
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

	// Delete the old primary surface
	if (DSurface::Primary) {
		Debug::Log("[RenderDX] Deleting old primary surface\n");
		GameDelete(DSurface::Primary);
		DSurface::Primary = nullptr;
	}

	if (DXRenderer::Instance().IsWindowed()) {
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
	if (!SetVideoMode(Game::hWnd, width, height, 16)) {
		if (DXRenderer::Instance().IsWindowed()) {
			DXRenderer::Instance().MoveWindow(oldWindowX, oldWindowY, oldWindowWidth, oldWindowHeight);
			Debug::Log("[RenderDX] Restore window to (%d, %d) with size %dx%d\n", oldWindowX, oldWindowY, oldWindowWidth, oldWindowHeight);
		}

		if (oldRect.X > 0 && oldRect.Y > 0 && oldRenderWidth > 0 && oldRenderHeight > 0) {
			Debug::Log("[RenderDX] Restoring old display mode.\n");
			if (!SetVideoMode(Game::hWnd, oldRenderWidth, oldRenderHeight, oldRenderBpp)) {
				Debug::Log("[RenderDX] Failed to restore old display mode.\n");
				DXMouse::Instance->ShowMouse();
				return false;
			}
			RebuildDisplayState(oldRect);
		}
		else {
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

float __fastcall RenderDX::GetXScale() {
	return ScaleX;
}

float __fastcall RenderDX::GetYScale() {
	return ScaleY;
}

int __fastcall RenderDX::ClientToRenderX(int x) {
	if (Drawing::RenderWidth <= 0)
		return x;

	return std::clamp(static_cast<int>((x - ViewportX) * ScaleX), 0, Drawing::RenderWidth - 1);
}

int __fastcall RenderDX::ClientToRenderY(int y) {
	if (Drawing::RenderHeight <= 0)
		return y;

	return std::clamp(static_cast<int>((y - ViewportY) * ScaleY), 0, Drawing::RenderHeight - 1);
}

void __fastcall RenderDX::UpdateScale() {
	const float viewportWidth = DXRenderer::Instance().GetViewportWidth();
	const float viewportHeight = DXRenderer::Instance().GetViewportHeight();
	ViewportX = DXRenderer::Instance().GetViewportX();
	ViewportY = DXRenderer::Instance().GetViewportY();

	if (Drawing::RenderWidth <= 0 || Drawing::RenderHeight <= 0 || viewportWidth <= 0.0f || viewportHeight <= 0.0f) {
		ResetScale();
		return;
	}

	ScaleX = static_cast<float>(Drawing::RenderWidth) / viewportWidth;
	ScaleY = static_cast<float>(Drawing::RenderHeight) / viewportHeight;
}

void __fastcall RenderDX::ResetScale() {
	ScaleX = 1.0f;
	ScaleY = 1.0f;
	ViewportX = 0.0f;
	ViewportY = 0.0f;
}

int* __fastcall RenderDX::EnumDisplayModes(DWORD minWidth, DWORD minHeight, DWORD maxWidth, DWORD maxHeight, DWORD) {
	std::vector<std::pair<int, int>> modes;
	DEVMODE devmode{};
	DWORD modeIndex = 0;

	while (::EnumDisplaySettingsA(nullptr, modeIndex++, &devmode)) {
		const DWORD w = devmode.dmPelsWidth;
		const DWORD h = devmode.dmPelsHeight;
		const DWORD bpp = devmode.dmBitsPerPel;

		if (w >= minWidth && h >= minHeight && w <= maxWidth && h <= maxHeight && bpp == 32) {
			modes.emplace_back(static_cast<int>(w), static_cast<int>(h));
		}
	}

	if (modes.empty()) {
		return nullptr;
	}

	std::sort(modes.begin(), modes.end());
	modes.erase(std::unique(modes.begin(), modes.end()), modes.end());

	const size_t count = modes.size();
	const size_t bytes = sizeof(int) * (count * 2 + 1);

	int* list = static_cast<int*>(YRMemory::Allocate(bytes));
	std::memset(list, 0, bytes);

	int* ptr = list;
	for (const auto& mode : modes) {
		*ptr++ = mode.first;
		*ptr++ = mode.second;
	}

	return list;
}

void __fastcall RenderDX::MainProcHandlePaint() {
	if (DXMouse::Instance && DSurface::Primary && DSurface::Hidden && DSurface::Composite) {
		if (Unsorted::ScenarioStarted) {
			GScreenClass::UpdatePrimarySurface(DXMouse::Instance->IsCaptured(), DSurface::Composite, nullptr);
			SidebarClass::Instance.BlitSidebar(true);
		}
		else if (Game::IsMoviePlaying()) {
			Game::BlitMovie();
		}
		else {
			GScreenClass::UpdatePrimarySurface(DXMouse::Instance->IsCaptured(), DSurface::Hidden, nullptr);
		}
	}
}
