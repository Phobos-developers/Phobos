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
#include <vector>

bool __fastcall RenderDX::AllocateSurfaces(const RectangleStruct& hidden_rect, const RectangleStruct& composite_rect, const RectangleStruct& tile_rect, const RectangleStruct& sidebar_rect, bool hidden_first) {
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

	if (hidden_first && hidden_rect.Width > 0 && hidden_rect.Height > 0) {
		DSurface::Hidden = GameCreate<DXSurface>(hidden_rect.Width, hidden_rect.Height);
		DSurface::Hidden->Fill(0);
		Debug::Log("[RenderDX] HiddenSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
	}

	if (composite_rect.Width > 0 && composite_rect.Height > 0) {
		DSurface::Composite = GameCreate<DXSurface>(composite_rect.Width, composite_rect.Height);
		DSurface::Composite->Fill(0);
		Debug::Log("[RenderDX] CompositeSurface (%dx%d)\n", composite_rect.Width, composite_rect.Height);
	}

	if (tile_rect.Width > 0 && tile_rect.Height > 0) {
		DSurface::Tile = GameCreate<DXSurface>(tile_rect.Width, tile_rect.Height);
		DSurface::Tile->Fill(0);
		Debug::Log("[RenderDX] TileSurface (%dx%d)\n", tile_rect.Width, tile_rect.Height);
	}

	if (sidebar_rect.Width > 0 && sidebar_rect.Height > 0) {
		DSurface::Sidebar = GameCreate<DXSurface>(sidebar_rect.Width, sidebar_rect.Height);
		DSurface::Sidebar->Fill(0);
		Debug::Log("[RenderDX] SidebarSurface (%dx%d)\n", sidebar_rect.Width, sidebar_rect.Height);
	}

	if (!hidden_first && hidden_rect.Width > 0 && hidden_rect.Height > 0) {
		DSurface::Hidden = GameCreate<DXSurface>(hidden_rect.Width, hidden_rect.Height);
		DSurface::Hidden->Fill(0);
		Debug::Log("[RenderDX] HiddenSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
	}

	if (hidden_rect.Width > 0 && hidden_rect.Height > 0) {
		DSurface::Alternate = GameCreate<DXSurface>(hidden_rect.Width, hidden_rect.Height);
		DSurface::Alternate->Fill(0);
		Debug::Log("[RenderDX] AlternateSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
	}

	return true;
}

bool __fastcall RenderDX::SetVideoMode(HWND, int width, int height, int bits_per_pixel) {
	Debug::Log("[RenderDX] Setting video mode to %dx%d@%d\n", width, height, bits_per_pixel);

	if (!DXRenderer::Instance().IsRendererReady()) {
		Debug::Log("[RenderDX] Renderer is not ready\n");
		return false;
	}

	ResetVideoMode();
	if (!DXRenderer::Instance().CreateRenderer(width, height, bits_per_pixel)) {
		Debug::Log("[RenderDX] Failed to create renderer\n");
		return false;
	}

	Drawing::RenderWidth = width;
	Drawing::RenderHeight = height;
	Drawing::RenderBitsPerPixel = bits_per_pixel;

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

	DXMouse::Instance->Recalc_Capture_Region();
	if (rebuildCursor)
		DXMouse::Instance->Rebuild_Cursor_Image();
}

static void ApplyWindowResize(int width, int height) {
	DXRenderer::Instance().ResizeWindow(width, height);
	RecalcMouseWindowRegion(true);
}

static LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
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
			int x = GET_X_LPARAM(lparam);
			int y = GET_Y_LPARAM(lparam);

			x = RenderDX::ClientToRenderX(x);
			y = RenderDX::ClientToRenderY(y);

			lparam = MAKELPARAM(x, y);
		}
		break;
	}

	case WM_MOVE:
	{
		if (DXMouse::Instance) {
			DXMouse::Instance->Recalc_Capture_Region();
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
			if (::GetClientRect(hwnd, &clientRect)) {
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
		const int width = LOWORD(lparam);
		const int height = HIWORD(lparam);
		if (wparam == SIZE_MINIMIZED || width == 0 || height == 0) {
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
		if (wparam == VK_RETURN && (lparam & (1 << 29))) {
			DXRenderer::Instance().ToggleFullscreen();
			if (DXMouse::Instance) {
				DXMouse::Instance->Recalc_Capture_Region();
				DXMouse::Instance->Rebuild_Cursor_Image();
			}
			return 0; // handled
		}
		break;
	}

	case WM_SETCURSOR:
	{
		// Prevent the system from setting the cursor when it's over our window, since we handle it ourselves.
		if (LOWORD(lparam) == HTCLIENT) {
			if (DXMouse::Instance)
				DXMouse::Instance->Set_Cached_Cursor();
			return TRUE; // handled
		}
		break;
	}

	case WM_ACTIVATEAPP:
	{
		if (DXRenderOptions::Config().PauseGameWhenLoseFocus)
			break; // goto the original window procedure to allow the game to pause when losing focus
		if (hwnd == Game::hWnd) {
			Unsorted::ScenarioInit = true; // game is always active
			if (wparam) {
				Debug::Log("[RenderDX] Game window activated\n");
				if (DXMouse::Instance)
					DXMouse::Instance->Capture_Mouse();
			}
			else {
				Debug::Log("[RenderDX] Game window deactivated\n");
				if (DXMouse::Instance)
					DXMouse::Instance->Release_Mouse();
			}
		}
		return 0; // handled - prevent the game from pausing when the window is deactivated
	}
	}

	// Call original window procedure for default processing
	return reinterpret_cast<LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM)>(0x7775C0)(hwnd, msg, wparam, lparam);
}

void __fastcall RenderDX::CreateMainWindow(HINSTANCE instance, int cmd_show, int width, int height) {
	Debug::Log("[RenderDX] Creating main window\n");
	if (!DXRenderer::Instance().CreateMainWindow(instance, cmd_show, width, height, MainWindowProc)) {
		Debug::Log("[RenderDX] Failed to create main window\n");
		::MessageBoxA(nullptr, "Failed to create main window", "Error", MB_ICONERROR);
		::ExitProcess(0xC0DEBEEF);
	}
}

void __fastcall RenderDX::DestroyMainWindow() {
	Debug::Log("[RenderDX] Destroying main window\n");
	DXRenderer::Instance().DestroyMainWindow();
}

bool __fastcall RenderDX::UpdateScreen(Surface* surface) {
	if (!surface) {
		Debug::Log("[RenderDX] UpdateScreen called with null surface\n");
		return false;
	}

	const bool shouldScale = ShouldScale();
	DXRenderer::Instance().SetRenderScale(shouldScale);

	// Retrieve the game surface data
	if (void* pixels = surface->Lock(0, 0)) {
		if (!DXRenderer::Instance().UploadSurfaceToTexture(pixels, surface->GetPitch())) {
			Debug::Log("[RenderDX] Failed to upload surface to texture\n");
			surface->Unlock();
			return false;
		}
		surface->Unlock();
	}

	static bool scaled = ShouldScale();

	// Extra process on scaling change
	if (scaled != shouldScale) {
		scaled = shouldScale;
		if (DXMouse::Instance)
			DXMouse::Instance->Rebuild_Cursor_Image();
	}

	DXRenderer::Instance().Present();

	return true;
}

bool __fastcall RenderDX::ShouldScale() {
	return Unsorted::SpecialDialog == 0 && Unsorted::WSDialogCount == 0;
}

static void RebuildDisplayState(const RectangleStruct& view_rect) {
	auto temp = view_rect;
	temp.X = GameOptionsClass::Instance.SidebarMode ? 0 : 168;
	temp.Y = 16;
	temp.Width -= 168;
	temp.Height -= 16;

	DSurface::ViewBounds = view_rect;
	Drawing::RenderWidth = view_rect.Width;
	Drawing::RenderHeight = view_rect.Height;

	DSurface::Primary = DXSurface::CreatePrimary();

	RenderDX::AllocateSurfaces(
		view_rect,
		RectangleStruct { 0,0,temp.Width,view_rect.Height },
		RectangleStruct { 0,0,temp.Width,view_rect.Height },
		RectangleStruct { 0,0,168,view_rect.Height },
		false
	);
	DSurface::Temp = DSurface::Hidden;

	if (DXMouse::Instance) {
		DXMouse::Instance->Rebuild_Cursor_Image();
	}

	SidebarClass::Instance.Set_View_Dimensions(temp);
	SidebarClass::Instance.Init_IO();
	SidebarClass::Instance.Activate(1);
	SidebarClass::Instance.InitGUI();
	SidebarClass::Instance.MarkNeedsRedraw(2); // REDRAW_ALL
	DXMouse::Instance->Show_Mouse();
}

bool __fastcall RenderDX::ChangeDisplayMode(int width, int height) {
	Debug::Log("[RenderDX] Changing display mode to %dx%d\n", width, height);

	// Save current window position
	RectangleStruct old_rect = DSurface::ViewBounds;
	if (old_rect.Width <= 0 || old_rect.Height <= 0) {
		if (Drawing::RenderWidth > 0 && Drawing::RenderHeight > 0) {
			Debug::Log("[RenderDX] Current view bounds are invalid, using RenderWidth/RenderHeight\n");
			old_rect = RectangleStruct { 0, 0, Drawing::RenderWidth, Drawing::RenderHeight };
		}
	}

	const int old_render_width = Drawing::RenderWidth;
	const int old_render_height = Drawing::RenderHeight;
	const int old_render_bpp = Drawing::RenderBitsPerPixel;

	int old_window_x = 0;
	int old_window_y = 0;
	int old_window_width = DXRenderer::Instance().GetWindowWidth();
	int old_window_height = DXRenderer::Instance().GetWindowHeight();

	DXMouse::Instance->Hide_Mouse();

	// Delete the old primary surface
	if (DSurface::Primary) {
		Debug::Log("[RenderDX] Deleting old primary surface\n");
		GameDelete(DSurface::Primary);
		DSurface::Primary = nullptr;
	}

	if (DXRenderer::Instance().IsWindowed()) {
		int window_width = width;
		int window_height = height;

		RECT temp;
		::GetWindowRect(Game::hWnd, &temp);
		old_window_x = temp.left;
		old_window_y = temp.top;
		old_window_width = temp.right - temp.left;
		old_window_height = temp.bottom - temp.top;

		int center_x = old_window_x + old_window_width / 2;
		int center_y = old_window_y + old_window_height / 2;
		
		int new_x = center_x - window_width / 2;
		int new_y = center_y - window_height / 2;

		DXRenderer::Instance().MoveWindow(new_x, new_y, window_width, window_height);

		Debug::Log("[RenderDX] Moved window to (%d, %d) with size %dx%d\n", new_x, new_y, window_width, window_height);
	}

	// Recreate all intermediates
	if (!SetVideoMode(Game::hWnd, width, height, 16)) {
		if (DXRenderer::Instance().IsWindowed()) {
			DXRenderer::Instance().MoveWindow(old_window_x, old_window_y, old_window_width, old_window_height);
			Debug::Log("[RenderDX] Restore window to (%d, %d) with size %dx%d\n", old_window_x, old_window_y, old_window_width, old_window_height);
		}

		if (old_rect.X > 0 && old_rect.Y > 0 && old_render_width > 0 && old_render_height > 0) {
			Debug::Log("[RenderDX] Restoring old display mode.\n");
			if (!SetVideoMode(Game::hWnd, old_render_width, old_render_height, old_render_bpp)) {
				Debug::Log("[RenderDX] Failed to restore old display mode.\n");
				DXMouse::Instance->Show_Mouse();
				return false;
			}
			RebuildDisplayState(old_rect);
		}
		else {
			Debug::Log("[RenderDX] Old view bounds are invalid, cannot restore\n");
		}

		DXMouse::Instance->Show_Mouse();
		return false;
	}

	RectangleStruct new_view_rect = { 0, 0, width, height };
	RebuildDisplayState(new_view_rect);
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

int* __fastcall RenderDX::EnumDisplayModes(DWORD minw, DWORD minh, DWORD maxw, DWORD maxh, DWORD) {
	std::vector<std::pair<int, int>> modes;
	DEVMODE devmode{};
	DWORD mode_index = 0;

	while (::EnumDisplaySettingsA(nullptr, mode_index++, &devmode)) {
		const DWORD w = devmode.dmPelsWidth;
		const DWORD h = devmode.dmPelsHeight;
		const DWORD bpp = devmode.dmBitsPerPel;

		if (w >= minw && h >= minh && w <= maxw && h <= maxh && bpp == 32) {
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
			GScreenClass::UpdatePrimarySurface(DXMouse::Instance->Is_Captured(), DSurface::Composite, nullptr);
			SidebarClass::Instance.BlitSidebar(true);
		}
		else if (Game::IsMoviePlaying()) {
			Game::BlitMovie();
		}
		else {
			GScreenClass::UpdatePrimarySurface(DXMouse::Instance->Is_Captured(), DSurface::Hidden, nullptr);
		}
	}
}
