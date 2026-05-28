#include "Renderer.h"

#include <Utilities/Debug.h>

#include <CommCtrl.h>

#include <Unsorted.h>

#include "Functions.h"
#include "Options.h"
#include "Renderer.SurfacePS.h"
#include "Renderer.SurfaceVS.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

DXRenderer& DXRenderer::Instance() {
	static DXRenderer instance;
	return instance;
}

static LONG_PTR GetConfiguredWindowedStyle(LONG_PTR style, bool visible) {
	if (RenderOptions::Config().WindowedBorder)
		style = (style & ~WS_POPUP) | WS_OVERLAPPEDWINDOW;
	else
		style = (style & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)) | WS_POPUP;

	if (visible)
		style |= WS_VISIBLE;
	else
		style &= ~WS_VISIBLE;

	return style;
}

static LONG_PTR GetConfiguredWindowedExStyle(LONG_PTR exStyle) {
	if (RenderOptions::Config().WindowedBorder)
		return exStyle;

	return exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
}

static bool GetMonitorRect(HMONITOR monitor, RECT& monitorRect) {
	if (!monitor)
		return false;

	MONITORINFO monitorInfo {};
	monitorInfo.cbSize = sizeof(monitorInfo);
	if (!::GetMonitorInfoA(monitor, &monitorInfo))
		return false;

	monitorRect = monitorInfo.rcMonitor;
	return true;
}

static bool GetPrimaryMonitorRect(RECT& monitorRect) {
	POINT point { 0, 0 };
	return GetMonitorRect(::MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY), monitorRect);
}

static bool GetNearestMonitorRect(HWND hWnd, RECT& monitorRect) {
	return GetMonitorRect(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), monitorRect);
}

static void CenterRectInMonitor(RECT& rect, const RECT& monitorRect) {
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;

	rect.left = monitorRect.left + (monitorRect.right - monitorRect.left - width) / 2;
	rect.top = monitorRect.top + (monitorRect.bottom - monitorRect.top - height) / 2;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

bool DXRenderer::CreateMainWindow(HINSTANCE instance, int cmdShow, int width, int height, WNDPROC proc) {
	::InitCommonControls();

	WNDCLASSA wc {};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = proc;
	wc.hInstance = instance;
	wc.hIcon = ::LoadIconA(instance, MAKEINTRESOURCEA(93));
	wc.hCursor = ::LoadCursorA(nullptr, IDC_ARROW);
	wc.lpszClassName = reinterpret_cast<const char*>(0x849F48);
	if (!::RegisterClassA(&wc)) {
		Debug::Log("[RenderDX] Failed to register window class\n");
		return false;
	}

	LONG_PTR style = GetConfiguredWindowedStyle(WS_OVERLAPPEDWINDOW, false);
	LONG_PTR exStyle = GetConfiguredWindowedExStyle(0);
	RECT rect = { 0, 0, width, height };
	::AdjustWindowRectEx(&rect, static_cast<DWORD>(style), FALSE, static_cast<DWORD>(exStyle));
	bool centerWindow = false;
	RECT monitorRect {};
	if (GetPrimaryMonitorRect(monitorRect)) {
		CenterRectInMonitor(rect, monitorRect);
		centerWindow = true;
	}

	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;
	int windowX = centerWindow ? rect.left : CW_USEDEFAULT;
	int windowY = centerWindow ? rect.top : CW_USEDEFAULT;

	Game::hWnd = ::CreateWindowExA(static_cast<DWORD>(exStyle), wc.lpszClassName, wc.lpszClassName, static_cast<DWORD>(style), windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, instance, nullptr);

	if (!Game::hWnd) {
		Debug::Log("[RenderDX] Failed to create main window\n");
		return false;
	}

	// Disable clipping because we draw Win32 child windows as part of the main window
	style = GetWindowLongPtrA(Game::hWnd, GWL_STYLE);
	style &= ~(WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	SetWindowLongPtrA(Game::hWnd, GWL_STYLE, style);

	Hwnd = Game::hWnd;
	WindowWidth = windowWidth;
	WindowHeight = windowHeight;

	if (RenderOptions::Config().StartFullscreen)
		ToggleFullscreen();

	::ShowWindow(Game::hWnd, cmdShow);
	::UpdateWindow(Game::hWnd);

	Game::hIMC = ::ImmGetContext(Game::hWnd);
	::ImmAssociateContext(Game::hWnd, nullptr);

	::RegisterHotKey(Game::hWnd, 1, MOD_ALT | MOD_CONTROL | MOD_SHIFT, 'M');

	// Gain focus for the game window to ensure it receives input
	::SetForegroundWindow(Game::hWnd);
	Unsorted::GameInFocus = true;

	if (!DXRenderer::Instance().LoadImports()) {
		Debug::Log("[RenderDX] Failed to load required libraries\n");
		return false;
	}

	return true;
}

void DXRenderer::DestroyMainWindow() {
	if (!Hwnd)
		return;

	::DestroyWindow(Hwnd);
	Hwnd = nullptr;

	DXRenderer::Instance().UnloadImports();
}

bool DXRenderer::IsRendererReady() {
	return true;
}

bool DXRenderer::CreateRenderer(int width, int height, int bitsPerPixel) {
	if (bitsPerPixel != 16) {
		Debug::Log("[RenderDX] Unsupported bits per pixel: %d\n", bitsPerPixel);
		return false;
	}

	RenderWidth = width;
	RenderHeight = height;
	if (WindowWidth <= 0)
		WindowWidth = width;
	if (WindowHeight <= 0)
		WindowHeight = height;

	UpdateViewportAndScissor();

	if (!CreateDevice()) {
		Debug::Log("[RenderDX] Failed to create D3D11 device\n");
		return false;
	}

	if (!CreateSwapChain()) {
		Debug::Log("[RenderDX] Failed to create swap chain\n");
		return false;
	}

	if (!CreateRenderTargetViews()) {
		Debug::Log("[RenderDX] Failed to create render target views\n");
		return false;
	}

	if (!CreateSurfacePipeline()) {
		Debug::Log("[RenderDX] Failed to create surface pipeline\n");
		return false;
	}

	if (!CreateFixedSurfaceGpuResources()) {
		Debug::Log("[RenderDX] Failed to create fixed surface GPU resources\n");
		return false;
	}

	return true;
}

void DXRenderer::DestroyRenderer() {
	if (DeviceContext) {
		DeviceContext->ClearState();
		DeviceContext->Flush();
	}

	DepthStencilState.Reset();
	BlendState.Reset();
	RasterizerState.Reset();
	SamplerState.Reset();
	PixelShader.Reset();
	VertexShader.Reset();
	SurfaceShaderResourceView.Reset();
	SurfaceTexture.Reset();

	RenderTargetView.Reset();

	if (SwapChain) {
		BOOL fullscreenState = FALSE;
		Microsoft::WRL::ComPtr<IDXGIOutput> pTarget;
		if (SUCCEEDED(SwapChain->GetFullscreenState(&fullscreenState, pTarget.GetAddressOf())) && fullscreenState)
			SwapChain->SetFullscreenState(FALSE, nullptr);

		SwapChain.Reset();
	}
	SwapChainBufferCount = 0;

	DeviceContext.Reset();
	Device.Reset();
	Factory.Reset();
}

bool DXRenderer::ResizeWindow(int width, int height) {
	WindowWidth = width;
	WindowHeight = height;

	UpdateViewportAndScissor();
	RenderDX::UpdateScale();

	if (!Device || !SwapChain) {
		return true; // No swap chain to resize, not an error.
	}

	if (!WaitForGpu()) {
		Debug::Log("[RenderDX] Failed to wait for GPU before resizing swap chain.\n");
		return false;
	}

	RenderTargetView.Reset();

	const UINT bufferCount = SwapChainBufferCount ? SwapChainBufferCount : 1;
	if (FAILED(SwapChain->ResizeBuffers(bufferCount, WindowWidth, WindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0))) {
		Debug::Log("[RenderDX] Failed to resize swap chain buffers.\n");
		return false;
	}

	if (!CreateRenderTargetViews())
		return false;

	Debug::Log("[RenderDX] Swap chain resized successfully to %ux%u.\n", WindowWidth, WindowHeight);

	return true;
}

void DXRenderer::ToggleFullscreen() {
	Debug::Log("[RenderDX] Toggling fullscreen mode.\n");

	if (!Hwnd)
		return;

	if (!Windowed) {
		if (!HasWindowedState) {
			Debug::Log("[RenderDX] Cannot restore windowed mode, no saved window state.\n");
			return;
		}

		Windowed = true;

		::SetWindowLongPtrA(Hwnd, GWL_STYLE, GetConfiguredWindowedStyle(WindowedStyle, true));
		::SetWindowLongPtrA(Hwnd, GWL_EXSTYLE, GetConfiguredWindowedExStyle(WindowedExStyle));

		const int width = WindowedRect.right - WindowedRect.left;
		const int height = WindowedRect.bottom - WindowedRect.top;
		int windowX = WindowedRect.left;
		int windowY = WindowedRect.top;

		if (!RenderOptions::Config().WindowedBorder) {
			RECT monitorRect {};
			if (GetNearestMonitorRect(Hwnd, monitorRect)) {
				RECT centeredRect = { 0, 0, width, height };
				CenterRectInMonitor(centeredRect, monitorRect);
				WindowedRect = centeredRect;
				windowX = centeredRect.left;
				windowY = centeredRect.top;
			}
		}

		::SetWindowPos(Hwnd, nullptr, windowX, windowY, width, height, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		Debug::Log("[RenderDX] Borderless fullscreen disabled.\n");
		return;
	}

	if (!::GetWindowRect(Hwnd, &WindowedRect)) {
		Debug::Log("[RenderDX] Failed to save window rectangle before entering borderless fullscreen.\n");
		return;
	}

	WindowedStyle = GetConfiguredWindowedStyle(::GetWindowLongPtrA(Hwnd, GWL_STYLE), true);
	WindowedExStyle = GetConfiguredWindowedExStyle(::GetWindowLongPtrA(Hwnd, GWL_EXSTYLE));
	HasWindowedState = true;

	RECT monitorRect {};
	if (!GetNearestMonitorRect(Hwnd, monitorRect)) {
		Debug::Log("[RenderDX] Failed to get monitor rectangle for borderless fullscreen.\n");
		return;
	}

	const LONG_PTR borderlessStyle = (WindowedStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU)) | WS_POPUP | WS_VISIBLE;
	const LONG_PTR borderlessExStyle = WindowedExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);

	Windowed = false;

	::SetWindowLongPtrA(Hwnd, GWL_STYLE, borderlessStyle);
	::SetWindowLongPtrA(Hwnd, GWL_EXSTYLE, borderlessExStyle);

	const int width = monitorRect.right - monitorRect.left;
	const int height = monitorRect.bottom - monitorRect.top;
	::SetWindowPos(Hwnd, HWND_TOP, monitorRect.left, monitorRect.top, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

	Debug::Log("[RenderDX] Borderless fullscreen enabled.\n");
}

bool DXRenderer::UploadSurfaceToTexture(void* pSurfaceData, int sourcePitch) {
	const int sourceRowBytes = RenderWidth * static_cast<int>(sizeof(std::uint16_t));
	if (sourcePitch < sourceRowBytes) {
		Debug::Log("[RenderDX] Source pitch %d is smaller than required row bytes %d.\n", sourcePitch, sourceRowBytes);
		return false;
	}

	if (!UploadSurfaceToGpu(pSurfaceData, sourcePitch))
		return false;

	return RenderSurface();
}

void DXRenderer::SetRenderScale(bool scale) {
	if (ScaleRender == scale)
		return;

	ScaleRender = scale;
	UpdateViewportAndScissor();
	RenderDX::UpdateScale();
}

bool DXRenderer::Present() {
	if (FAILED(SwapChain->Present(0, 0))) {
		Debug::Log("[RenderDX] Failed to present swap chain.\n");
		return false;
	}

	return true;
}

void DXRenderer::MoveWindow(int x, int y, int width, int height) {
	if (Windowed && !RenderOptions::Config().WindowedBorder) {
		RECT monitorRect {};
		if (GetNearestMonitorRect(Hwnd, monitorRect)) {
			RECT centeredRect = { 0, 0, width, height };
			CenterRectInMonitor(centeredRect, monitorRect);
			x = centeredRect.left;
			y = centeredRect.top;
		}
	}

	::MoveWindow(Hwnd, x, y, width, height, TRUE);
	WindowWidth = width;
	WindowHeight = height;
	UpdateViewportAndScissor();
	RenderDX::UpdateScale();
}

bool DXRenderer::IsWindowed() const {
	return Windowed;
}

int DXRenderer::GetWindowWidth() const {
	return WindowWidth;
}

int DXRenderer::GetWindowHeight() const {
	return WindowHeight;
}

float DXRenderer::GetViewportX() const {
	return RenderViewportX;
}

float DXRenderer::GetViewportY() const {
	return RenderViewportY;
}

float DXRenderer::GetViewportWidth() const {
	return RenderViewportWidth;
}

float DXRenderer::GetViewportHeight() const {
	return RenderViewportHeight;
}

DXRenderer::DXRenderer() {}

DXRenderer::~DXRenderer() {}

bool DXRenderer::LoadImports() {
	UnloadImports();

	D3D11Lib = ::LoadLibraryW(L"d3d11.dll");
	if (!D3D11Lib) {
		Debug::Log("[RenderDX] Failed to load d3d11.dll.\n");
		return false;
	}

	D3D11CreateDeviceProc = reinterpret_cast<decltype(&D3D11CreateDevice)>(::GetProcAddress(D3D11Lib, "D3D11CreateDevice"));
	if (!D3D11CreateDeviceProc) {
		Debug::Log("[RenderDX] Failed to get address of D3D11CreateDevice.\n");
		UnloadImports();
		return false;
	}

	DXGILib = ::LoadLibraryW(L"dxgi.dll");
	if (!DXGILib) {
		Debug::Log("[RenderDX] Failed to load dxgi.dll.\n");
		UnloadImports();
		return false;
	}
	CreateDXGIFactory2Proc = reinterpret_cast<decltype(&CreateDXGIFactory2)>(::GetProcAddress(DXGILib, "CreateDXGIFactory2"));
	CreateDXGIFactory1Proc = reinterpret_cast<decltype(&CreateDXGIFactory1)>(::GetProcAddress(DXGILib, "CreateDXGIFactory1"));
	CreateDXGIFactoryProc = reinterpret_cast<decltype(&CreateDXGIFactory)>(::GetProcAddress(DXGILib, "CreateDXGIFactory"));
	if (!CreateDXGIFactory2Proc && !CreateDXGIFactory1Proc && !CreateDXGIFactoryProc) {
		Debug::Log("[RenderDX] Failed to get a DXGI factory creation entry point.\n");
		UnloadImports();
		return false;
	}

	return true;
}

void DXRenderer::UnloadImports() {
	if (DXGILib) {
		::FreeLibrary(DXGILib);
		DXGILib = nullptr;
		CreateDXGIFactory2Proc = nullptr;
		CreateDXGIFactory1Proc = nullptr;
		CreateDXGIFactoryProc = nullptr;
	}
	if (D3D11Lib) {
		::FreeLibrary(D3D11Lib);
		D3D11Lib = nullptr;
		D3D11CreateDeviceProc = nullptr;
	}
}

bool DXRenderer::CreateFactory(UINT dxgiFactoryFlags) {
	HRESULT hr = S_OK;
	Factory.Reset();

	if (CreateDXGIFactory2Proc) {
		Microsoft::WRL::ComPtr<IDXGIFactory2> factory2;
		hr = CreateDXGIFactory2Proc(dxgiFactoryFlags, IID_PPV_ARGS(factory2.GetAddressOf()));
		if (SUCCEEDED(hr) && SUCCEEDED(hr = factory2.As(&Factory))) {
			Debug::Log("[RenderDX] DXGI factory created with CreateDXGIFactory2.\n");
			return true;
		}

		Debug::Log("[RenderDX] CreateDXGIFactory2 failed: 0x%08X. Falling back to older factory entry points.\n", static_cast<DWORD>(hr));
	}

	if (CreateDXGIFactory1Proc) {
		Microsoft::WRL::ComPtr<IDXGIFactory1> factory1;
		hr = CreateDXGIFactory1Proc(IID_PPV_ARGS(factory1.GetAddressOf()));
		if (SUCCEEDED(hr) && SUCCEEDED(hr = factory1.As(&Factory))) {
			Debug::Log("[RenderDX] DXGI factory created with CreateDXGIFactory1.\n");
			return true;
		}

		Debug::Log("[RenderDX] CreateDXGIFactory1 failed: 0x%08X.\n", static_cast<DWORD>(hr));
	}

	if (CreateDXGIFactoryProc) {
		hr = CreateDXGIFactoryProc(IID_PPV_ARGS(Factory.ReleaseAndGetAddressOf()));
		if (SUCCEEDED(hr)) {
			Debug::Log("[RenderDX] DXGI factory created with CreateDXGIFactory.\n");
			return true;
		}

		Debug::Log("[RenderDX] CreateDXGIFactory failed: 0x%08X.\n", static_cast<DWORD>(hr));
	}

	return false;
}

bool DXRenderer::CreateDevice() {
	HRESULT hr = S_OK;
	UINT dxgiFactoryFlags = 0;
	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if DXRENDER_DEBUG
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	if (!CreateFactory(dxgiFactoryFlags)) {
		Debug::Log("[RenderDX] Failed to create DXGI factory.\n");
		return false;
	}

	const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL createdLevel = D3D_FEATURE_LEVEL_11_0;

	Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(hr = Factory.As(&factory6))) {
		Debug::Log("[RenderDX] IDXGIFactory6 interface is available. Using EnumAdapterByGpuPreference to select the adapter.\n");
		for (UINT adapterIndex = 0; !Device; ++adapterIndex) {
			Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
			hr = factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(hardwareAdapter.GetAddressOf()));
			if (hr == DXGI_ERROR_NOT_FOUND)
				break;
			if (FAILED(hr)) {
				Debug::Log("[RenderDX] EnumAdapterByGpuPreference failed: 0x%08X\n", static_cast<DWORD>(hr));
				break;
			}

			DXGI_ADAPTER_DESC1 desc {};
			hardwareAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (SUCCEEDED(hr = D3D11CreateDeviceProc(hardwareAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, deviceFlags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, Device.ReleaseAndGetAddressOf(), &createdLevel, DeviceContext.ReleaseAndGetAddressOf()))) {
				Debug::Log("[RenderDX] D3D11 device created successfully on adapter: %ls\n", desc.Description);
				break;
			}
		}
	}
	else {
		Debug::Log("[RenderDX] IDXGIFactory6 is unavailable: 0x%08X. Falling back to older adapter enumeration.\n", static_cast<DWORD>(hr));
	}

	if (!Device) {
		Microsoft::WRL::ComPtr<IDXGIFactory1> factory1;
		if (SUCCEEDED(hr = Factory.As(&factory1))) {
			for (UINT adapterIndex = 0; !Device; ++adapterIndex) {
				Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
				hr = factory1->EnumAdapters1(adapterIndex, hardwareAdapter.ReleaseAndGetAddressOf());
				if (hr == DXGI_ERROR_NOT_FOUND)
					break;
				if (FAILED(hr)) {
					Debug::Log("[RenderDX] EnumAdapters1 failed: 0x%08X\n", static_cast<DWORD>(hr));
					break;
				}

				DXGI_ADAPTER_DESC1 desc {};
				hardwareAdapter->GetDesc1(&desc);
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;

				if (SUCCEEDED(hr = D3D11CreateDeviceProc(hardwareAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, deviceFlags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, Device.ReleaseAndGetAddressOf(), &createdLevel, DeviceContext.ReleaseAndGetAddressOf()))) {
					Debug::Log("[RenderDX] D3D11 device created successfully on adapter: %ls\n", desc.Description);
					break;
				}
			}
		}
		else {
			Debug::Log("[RenderDX] IDXGIFactory1 is unavailable: 0x%08X. Falling back to EnumAdapters().\n", static_cast<DWORD>(hr));
		}
	}

	if (!Device) {
		for (UINT adapterIndex = 0; !Device; ++adapterIndex) {
			Microsoft::WRL::ComPtr<IDXGIAdapter> hardwareAdapter;
			hr = Factory->EnumAdapters(adapterIndex, hardwareAdapter.ReleaseAndGetAddressOf());
			if (hr == DXGI_ERROR_NOT_FOUND)
				break;
			if (FAILED(hr)) {
				Debug::Log("[RenderDX] EnumAdapters failed: 0x%08X\n", static_cast<DWORD>(hr));
				break;
			}

			DXGI_ADAPTER_DESC desc {};
			hardwareAdapter->GetDesc(&desc);

			if (SUCCEEDED(hr = D3D11CreateDeviceProc(hardwareAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, deviceFlags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, Device.ReleaseAndGetAddressOf(), &createdLevel, DeviceContext.ReleaseAndGetAddressOf()))) {
				Debug::Log("[RenderDX] D3D11 device created successfully on adapter: %ls\n", desc.Description);
				break;
			}
		}
	}

	if (!Device) {
		Debug::Log("[RenderDX] Failed to create D3D11 device on enumerated adapters. Attempting default hardware device.\n");

		if (SUCCEEDED(hr = D3D11CreateDeviceProc(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, Device.ReleaseAndGetAddressOf(), &createdLevel, DeviceContext.ReleaseAndGetAddressOf())))
			Debug::Log("[RenderDX] Default D3D11 hardware device created successfully.\n");
	}

	if (!Device) {
		Debug::Log("[RenderDX] Failed to create D3D11 device on a hardware adapter. Attempting to create WARP device.\n");

		if (FAILED(hr = D3D11CreateDeviceProc(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, deviceFlags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, Device.ReleaseAndGetAddressOf(), &createdLevel, DeviceContext.ReleaseAndGetAddressOf()))) {
			Debug::Log("[RenderDX] Failed to create D3D11 WARP device: 0x%08X\n", static_cast<DWORD>(hr));
			return false;
		}

		Debug::Log("[RenderDX] D3D11 WARP device created successfully.\n");
	}

	return Device != nullptr;
}

bool DXRenderer::CreateSwapChain() {
	HRESULT hr = S_OK;

	SwapChain.Reset();
	SwapChainBufferCount = 0;

	Microsoft::WRL::ComPtr<IDXGIFactory2> factory2;
	if (SUCCEEDED(hr = Factory.As(&factory2))) {
		if (!CreateFlipSwapChain(factory2.Get(), DXGI_SWAP_EFFECT_FLIP_DISCARD))
			CreateFlipSwapChain(factory2.Get(), DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL);
	}
	else {
		Debug::Log("[RenderDX] IDXGIFactory2 is unavailable: 0x%08X. Falling back to legacy swap chain.\n", static_cast<DWORD>(hr));
	}

	if (!SwapChain && !CreateLegacySwapChain())
		return false;

	const UINT fullAssociationFlags = DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN;
	const UINT reducedAssociationFlags = DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER;
	hr = Factory->MakeWindowAssociation(Hwnd, fullAssociationFlags);
	if (FAILED(hr))
		hr = Factory->MakeWindowAssociation(Hwnd, reducedAssociationFlags);
	if (FAILED(hr))
		Debug::Log("[RenderDX] Failed to set window association: 0x%08X\n", static_cast<DWORD>(hr));

	Debug::Log("[RenderDX] Swap chain created successfully.\n");
	return true;
}

bool DXRenderer::CreateFlipSwapChain(IDXGIFactory2* pFactory2, DXGI_SWAP_EFFECT swapEffect) {
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC1 desc {};
	desc.Width = WindowWidth;
	desc.Height = WindowHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FrameCount;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = swapEffect;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	if (FAILED(hr = pFactory2->CreateSwapChainForHwnd(Device.Get(), Hwnd, &desc, nullptr, nullptr, swapChain1.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create flip swap chain with effect %u: 0x%08X\n", static_cast<UINT>(swapEffect), static_cast<DWORD>(hr));
		return false;
	}

	if (FAILED(hr = swapChain1.As(&SwapChain))) {
		Debug::Log("[RenderDX] Failed to query base swap chain interface: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	SwapChainBufferCount = FrameCount;
	Debug::Log("[RenderDX] Flip swap chain created with effect %u.\n", static_cast<UINT>(swapEffect));
	return true;
}

bool DXRenderer::CreateLegacySwapChain() {
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC desc {};
	desc.BufferDesc.Width = WindowWidth;
	desc.BufferDesc.Height = WindowHeight;
	desc.BufferDesc.RefreshRate.Numerator = 0;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 1;
	desc.OutputWindow = Hwnd;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.Flags = 0;

	if (FAILED(hr = Factory->CreateSwapChain(Device.Get(), &desc, SwapChain.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create legacy swap chain: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	SwapChainBufferCount = 1;
	Debug::Log("[RenderDX] Legacy swap chain created.\n");
	return true;
}

bool DXRenderer::CreateRenderTargetViews() {
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(hr = SwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())))) {
		Debug::Log("[RenderDX] Failed to get back buffer: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}
	if (FAILED(hr = Device->CreateRenderTargetView(backBuffer.Get(), nullptr, RenderTargetView.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create render target view: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	Debug::Log("[RenderDX] Render target view created successfully.\n");
	return true;
}

bool DXRenderer::CreateSurfacePipeline() {
	HRESULT hr = S_OK;
	if (FAILED(hr = Device->CreateVertexShader(SurfaceVertexShaderBytecode, sizeof(SurfaceVertexShaderBytecode), nullptr, VertexShader.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create vertex shader: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	if (FAILED(hr = Device->CreatePixelShader(SurfacePixelShaderBytecode, sizeof(SurfacePixelShaderBytecode), nullptr, PixelShader.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create pixel shader: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(hr = Device->CreateSamplerState(&samplerDesc, SamplerState.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create sampler state: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	D3D11_RASTERIZER_DESC rasterizerDesc {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = TRUE;
	if (FAILED(hr = Device->CreateRasterizerState(&rasterizerDesc, RasterizerState.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create rasterizer state: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	D3D11_BLEND_DESC blendDesc {};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED(hr = Device->CreateBlendState(&blendDesc, BlendState.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create blend state: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc {};
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.StencilEnable = FALSE;
	if (FAILED(hr = Device->CreateDepthStencilState(&depthStencilDesc, DepthStencilState.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create depth stencil state: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	Debug::Log("[RenderDX] Surface pipeline created successfully.\n");
	return true;
}

bool DXRenderer::CreateFixedSurfaceGpuResources() {
	HRESULT hr = S_OK;

	UINT formatSupport = 0;
	if (FAILED(hr = Device->CheckFormatSupport(DXGI_FORMAT_B5G6R5_UNORM, &formatSupport)) || !(formatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) || !(formatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE)) {
		Debug::Log("[RenderDX] B5G6R5 texture sampling is not supported by the D3D11 device: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	D3D11_TEXTURE2D_DESC textureDesc {};
	textureDesc.Width = RenderWidth;
	textureDesc.Height = RenderHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B5G6R5_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(hr = Device->CreateTexture2D(&textureDesc, nullptr, SurfaceTexture.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create surface texture: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
	srvDesc.Format = DXGI_FORMAT_B5G6R5_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	if (FAILED(hr = Device->CreateShaderResourceView(SurfaceTexture.Get(), &srvDesc, SurfaceShaderResourceView.ReleaseAndGetAddressOf()))) {
		Debug::Log("[RenderDX] Failed to create surface shader resource view: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	return true;
}

void DXRenderer::UpdateViewportAndScissor() {
	if (ScaleRender) {
		RenderViewportX = 0.0f;
		RenderViewportY = 0.0f;
		RenderViewportWidth = static_cast<float>(WindowWidth);
		RenderViewportHeight = static_cast<float>(WindowHeight);

		if (RenderOptions::Config().PreserveAspectRatio && RenderWidth > 0 && RenderHeight > 0 && WindowWidth > 0 && WindowHeight > 0) {
			const float scale = std::min(
				static_cast<float>(WindowWidth) / static_cast<float>(RenderWidth),
				static_cast<float>(WindowHeight) / static_cast<float>(RenderHeight)
			);

			RenderViewportWidth = static_cast<float>(RenderWidth) * scale;
			RenderViewportHeight = static_cast<float>(RenderHeight) * scale;
			RenderViewportX = (static_cast<float>(WindowWidth) - RenderViewportWidth) * 0.5f;
			RenderViewportY = (static_cast<float>(WindowHeight) - RenderViewportHeight) * 0.5f;
		}

		Viewport.TopLeftX = RenderViewportX;
		Viewport.TopLeftY = RenderViewportY;
		Viewport.Width = RenderViewportWidth;
		Viewport.Height = RenderViewportHeight;
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;

		ScissorRect.left = static_cast<LONG>(RenderViewportX);
		ScissorRect.top = static_cast<LONG>(RenderViewportY);
		ScissorRect.right = static_cast<LONG>(RenderViewportX + RenderViewportWidth);
		ScissorRect.bottom = static_cast<LONG>(RenderViewportY + RenderViewportHeight);
	}
	else {
		// Just render the surface at its native resolution in the top-left corner of the viewport.
		RenderViewportX = 0.0f;
		RenderViewportY = 0.0f;
		RenderViewportWidth = static_cast<float>(RenderWidth);
		RenderViewportHeight = static_cast<float>(RenderHeight);

		Viewport.TopLeftX = RenderViewportX;
		Viewport.TopLeftY = RenderViewportY;
		Viewport.Width = RenderViewportWidth;
		Viewport.Height = RenderViewportHeight;
		Viewport.MinDepth = 0.0f;
		Viewport.MaxDepth = 1.0f;

		ScissorRect.left = 0;
		ScissorRect.top = 0;
		ScissorRect.right = static_cast<LONG>(RenderWidth);
		ScissorRect.bottom = static_cast<LONG>(RenderHeight);
	}
}

bool DXRenderer::WaitForGpu() {
	if (!DeviceContext)
		return true;

	DeviceContext->ClearState();
	DeviceContext->Flush();
	return true;
}

bool DXRenderer::RenderSurface() {
	if (!DeviceContext || !RenderTargetView || !SurfaceShaderResourceView)
		return false;

	auto pRenderTargetView = RenderTargetView.Get();
	DeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	DeviceContext->ClearRenderTargetView(pRenderTargetView, clearColor);

	DeviceContext->RSSetViewports(1, &Viewport);
	DeviceContext->RSSetScissorRects(1, &ScissorRect);
	DeviceContext->RSSetState(RasterizerState.Get());

	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	DeviceContext->OMSetBlendState(BlendState.Get(), blendFactor, 0xFFFFFFFF);
	DeviceContext->OMSetDepthStencilState(DepthStencilState.Get(), 0);

	DeviceContext->IASetInputLayout(nullptr);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DeviceContext->VSSetShader(VertexShader.Get(), nullptr, 0);
	DeviceContext->PSSetShader(PixelShader.Get(), nullptr, 0);

	auto pSampler = SamplerState.Get();
	DeviceContext->PSSetSamplers(0, 1, &pSampler);

	auto pShaderResourceView = SurfaceShaderResourceView.Get();
	DeviceContext->PSSetShaderResources(0, 1, &pShaderResourceView);
	DeviceContext->Draw(3, 0);

	ID3D11ShaderResourceView* pNullShaderResourceView = nullptr;
	DeviceContext->PSSetShaderResources(0, 1, &pNullShaderResourceView);

	return true;
}

bool DXRenderer::UploadSurfaceToGpu(const void* pPixels, int sourcePitch) {
	if (!DeviceContext || !SurfaceTexture)
		return false;

	D3D11_MAPPED_SUBRESOURCE mapped {};
	HRESULT hr = DeviceContext->Map(SurfaceTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr)) {
		Debug::Log("[RenderDX] Failed to map surface texture: 0x%08X\n", static_cast<DWORD>(hr));
		return false;
	}

	auto dstBase = static_cast<std::uint8_t*>(mapped.pData);
	const auto* srcBase = static_cast<const std::uint8_t*>(pPixels);
	const UINT sourceRowBytes = RenderWidth * static_cast<UINT>(sizeof(std::uint16_t));
	for (int y = 0; y < RenderHeight; ++y) {
		auto* dstRow = dstBase + static_cast<size_t>(y) * mapped.RowPitch;
		const auto* srcRow = srcBase + static_cast<size_t>(y) * sourcePitch;
		std::memcpy(dstRow, srcRow, sourceRowBytes);
	}

	DeviceContext->Unmap(SurfaceTexture.Get(), 0);
	return true;
}
