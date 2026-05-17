#include "Renderer.h"

#include <Utilities/Debug.h>

#include <CommCtrl.h>

#include <Unsorted.h>

#include "Functions.h"
#include "Options.h"

#include <algorithm>

DXRenderer& DXRenderer::Instance() {
	static DXRenderer instance;
	return instance;
}

static LONG_PTR GetConfiguredWindowedStyle(LONG_PTR style, bool visible) {
	if (DXRenderOptions::Config().WindowedBorder)
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
	if (DXRenderOptions::Config().WindowedBorder)
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

static bool GetNearestMonitorRect(HWND hwnd, RECT& monitorRect) {
	return GetMonitorRect(::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), monitorRect);
}

static void CenterRectInMonitor(RECT& rect, const RECT& monitorRect) {
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;

	rect.left = monitorRect.left + (monitorRect.right - monitorRect.left - width) / 2;
	rect.top = monitorRect.top + (monitorRect.bottom - monitorRect.top - height) / 2;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

bool DXRenderer::CreateMainWindow(HINSTANCE instance, int cmd_show, int width, int height, WNDPROC proc) {
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

	int window_width = rect.right - rect.left;
	int window_height = rect.bottom - rect.top;
	int window_x = centerWindow ? rect.left : CW_USEDEFAULT;
	int window_y = centerWindow ? rect.top : CW_USEDEFAULT;

	Game::hWnd = ::CreateWindowExA(static_cast<DWORD>(exStyle), wc.lpszClassName, wc.lpszClassName, static_cast<DWORD>(style), window_x, window_y, window_width, window_height, nullptr, nullptr, instance, nullptr);

	if (!Game::hWnd) {
		Debug::Log("[RenderDX] Failed to create main window\n");
		return false;
	}

	// Disable clipping because we draw Win32 child windows as part of the main window
	style = GetWindowLongPtrA(Game::hWnd, GWL_STYLE);
	style &= ~(WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	SetWindowLongPtrA(Game::hWnd, GWL_STYLE, style);

	Hwnd = Game::hWnd;
	WindowWidth = window_width;
	WindowHeight = window_height;

	if (DXRenderOptions::Config().StartFullscreen)
		ToggleFullscreen();

	::ShowWindow(Game::hWnd, cmd_show);
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

bool DXRenderer::CreateRenderer(int width, int height, int bits_per_pixel) {
	if (bits_per_pixel != 16) {
		Debug::Log("[RenderDX] Unsupported bits per pixel: %d\n", bits_per_pixel);
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
		Debug::Log("[RenderDX] Failed to create D3D12 device\n");
		return false;
	}

	if (!CreateCommandQueue()) {
		Debug::Log("[RenderDX] Failed to create command queue\n");
		return false;
	}

	if (!CreateSwapChain()) {
		Debug::Log("[RenderDX] Failed to create swap chain\n");
		return false;
	}

	if (!CreateRtvHeap()) {
		Debug::Log("[RenderDX] Failed to create RTV descriptor heap\n");
		return false;
	}

	if (!CreateRenderTargetViews()) {
		Debug::Log("[RenderDX] Failed to create render target views\n");
		return false;
	}

	if (!CreateSrvHeap()) {
		Debug::Log("[RenderDX] Failed to create SRV descriptor heap\n");
		return false;
	}

	if (!CreateSurfacePipeline()) {
		Debug::Log("[RenderDX] Failed to create surface pipeline\n");
		return false;
	}

	if (!CreateCommandObjects()) {
		Debug::Log("[RenderDX] Failed to create command objects\n");
		return false;
	}

	if (!CreateFenceObjects()) {
		Debug::Log("[RenderDX] Failed to create fence objects\n");
		return false;
	}

	if (!CreateFixedSurfaceGpuResources()) {
		Debug::Log("[RenderDX] Failed to create fixed surface GPU resources\n");
		return false;
	}

	return true;
}

void DXRenderer::DestroyRenderer() {
	if (SurfaceTexture) {
		SurfaceTexture.Reset();
	}

	for (UINT i = 0; i < kFrameCount; ++i) {
		if (SurfaceUploadBuffers[i] && SurfaceUploadMapped[i]) {
			SurfaceUploadBuffers[i]->Unmap(0, nullptr);
			SurfaceUploadMapped[i] = nullptr;
		}
	}

	for (UINT i = 0; i < kFrameCount; ++i) {
		if (SurfaceUploadBuffers[i]) {
			SurfaceUploadBuffers[i].Reset();
		}
	}

	if (Fence) {
		Fence.Reset();
	}
	if (FenceEvent) {
		::CloseHandle(FenceEvent);
		FenceEvent = nullptr;
	}

	if (CommandList) {
		CommandList.Reset();
	}
	for (UINT i = 0; i < kFrameCount; ++i) {
		if (CommandAllocators[i]) {
			CommandAllocators[i].Reset();
		}
	}

	if (PipelineState) {
		PipelineState.Reset();
	}
	if (RootSignature) {
		RootSignature.Reset();
	}

	if (SrvHeap) {
		SrvHeap.Reset();
	}

	for (UINT i = 0; i < kFrameCount; ++i) {
		if (RenderTargets[i])
			RenderTargets[i].Reset();
	}

	if (RtvHeap) {
		RtvHeap.Reset();
	}
	RtvDescriptorSize = 0;

	if (SwapChain) {
		BOOL fullscreenState = FALSE;
		Microsoft::WRL::ComPtr<IDXGIOutput> pTarget;
		if (SUCCEEDED(SwapChain->GetFullscreenState(&fullscreenState, &pTarget)) && fullscreenState)
			SwapChain->SetFullscreenState(FALSE, nullptr);

		SwapChain.Reset();
	}
	FrameIndex = 0;

	if (CommandQueue) {
		CommandQueue.Reset();
	}

	if (Device) {
		Device.Reset();
	}
	if (Factory) {
		Factory.Reset();
	}
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

	for (auto& target : RenderTargets) {
		target.Reset();
	}

	if (FAILED(SwapChain->ResizeBuffers(kFrameCount, WindowWidth, WindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0))) {
		Debug::Log("[RenderDX] Failed to resize swap chain buffers.\n");
		return false;
	}

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
	if (!CreateRenderTargetViews())
		return false;

	const UINT64 newFenceValue = Fence->GetCompletedValue() + 1;
	FenceValues.fill(newFenceValue);

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

		if (!DXRenderOptions::Config().WindowedBorder) {
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

bool DXRenderer::UploadSurfaceToTexture(void* surface_data, int source_pitch) {
	const int sourceRowBytes = RenderWidth * static_cast<int>(sizeof(std::uint16_t));
	if (source_pitch < sourceRowBytes) {
		Debug::Log("[RenderDX] Source pitch %d is smaller than required row bytes %d.\n", source_pitch, sourceRowBytes);
		return false;
	}

	if (!PopulateCommandListForCPUSurface(surface_data, source_pitch))
		return false;

	ID3D12CommandList* list[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(1, list);
	return true;
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

	return MoveToNextFrame();
}

void DXRenderer::MoveWindow(int x, int y, int width, int height) {
	if (Windowed && !DXRenderOptions::Config().WindowedBorder) {
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
	D3D12Lib = ::LoadLibraryW(L"d3d12.dll");
	if (!D3D12Lib) {
		Debug::Log("[RenderDX] Failed to load d3d12.dll.\n");
		return false;
	}

#if DXRENDER_DEBUG
	FP_D3D12GetDebugInterface = reinterpret_cast<decltype(&D3D12GetDebugInterface)>(::GetProcAddress(D3D12Lib, "D3D12GetDebugInterface"));
	if (!FP_D3D12GetDebugInterface) {
		Debug::Log("[RenderDX] Failed to get address of D3D12GetDebugInterface.\n");
		return false;
	}
#endif
	FP_D3D12CreateDevice = reinterpret_cast<decltype(&D3D12CreateDevice)>(::GetProcAddress(D3D12Lib, "D3D12CreateDevice"));
	if (!FP_D3D12CreateDevice) {
		Debug::Log("[RenderDX] Failed to get address of D3D12CreateDevice.\n");
		return false;
	}
	FP_D3D12SerializeRootSignature = reinterpret_cast<decltype(&D3D12SerializeRootSignature)>(::GetProcAddress(D3D12Lib, "D3D12SerializeRootSignature"));
	if (!FP_D3D12SerializeRootSignature) {
		Debug::Log("[RenderDX] Failed to get address of D3D12SerializeRootSignature.\n");
		return false;
	}

	DXGILib = ::LoadLibraryW(L"dxgi.dll");
	if (!DXGILib) {
		Debug::Log("[RenderDX] Failed to load dxgi.dll.\n");
		return false;
	}
	FP_CreateDXGIFactory2 = reinterpret_cast<decltype(&CreateDXGIFactory2)>(::GetProcAddress(DXGILib, "CreateDXGIFactory2"));
	if (!FP_CreateDXGIFactory2) {
		Debug::Log("[RenderDX] Failed to get address of CreateDXGIFactory2.\n");
		return false;
	}

	D3DCompilerLib = ::LoadLibraryW(L"d3dcompiler_47.dll");
	if (!D3DCompilerLib) {
		Debug::Log("[RenderDX] Failed to load d3dcompiler_47.dll.\n");
		return false;
	}
	FP_D3DCompile = reinterpret_cast<decltype(&D3DCompile)>(::GetProcAddress(D3DCompilerLib, "D3DCompile"));
	if (!FP_D3DCompile) {
		Debug::Log("[RenderDX] Failed to get address of D3DCompile.\n");
		return false;
	}

	return true;
}

void DXRenderer::UnloadImports() {
	if (D3DCompilerLib) {
		::FreeLibrary(D3DCompilerLib);
		FP_D3DCompile = nullptr;
	}
	if (DXGILib) {
		::FreeLibrary(DXGILib);
		FP_CreateDXGIFactory2 = nullptr;
	}
	if (D3D12Lib) {
		::FreeLibrary(D3D12Lib);
#if DXRENDER_DEBUG
		FP_D3D12GetDebugInterface = nullptr;
#endif
		FP_D3D12CreateDevice = nullptr;
		FP_D3D12SerializeRootSignature = nullptr;
	}
}

bool DXRenderer::CreateDevice() {
	HRESULT hr = S_OK;
	UINT dxgiFactoryFlags = 0;
#if DXRENDER_DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(hr = FP_D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		Debug::Log("[RenderDX] D3D12 debug layer enabled.\n");
		debugController->EnableDebugLayer();
	}
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	if (FAILED(hr = FP_CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&Factory)))) {
		Debug::Log("[RenderDX] Failed to create DXGI factory: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
	if (FAILED(hr = Factory.As(&factory6)))
	{
		Debug::Log("[RenderDX] Failed to query IDXGIFactory6 interface: 0x%08X\nFalling back to EnumAdapters1()", static_cast<unsigned int>(hr));
		Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
		for (UINT adapterIndex = 0; Factory->EnumAdapters1(adapterIndex, &hardwareAdapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			hardwareAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}
			if (SUCCEEDED(hr = FP_D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device))))
			{
				Debug::Log("[RenderDX] D3D12 device created successfully on adapter: %ls\n", desc.Description);
				break;
			}
		}
	}
	else
	{
		Debug::Log("[RenderDX] IDXGIFactory6 interface is available. Using EnumAdapterByGpuPreference to select the adapter.\n");
		Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
		for (UINT adapterIndex = 0; factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&hardwareAdapter)) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			hardwareAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}
			if (SUCCEEDED(hr = FP_D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device))))
			{
				Debug::Log("[RenderDX] D3D12 device created successfully on adapter: %ls\n", desc.Description);
				break;
			}
		}
	}
	if (!Device)
	{
		Debug::Log("[RenderDX] Failed to create D3D12 device on a hardware adapter. Attempting to create WARP device.\n");

		Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
		if (FAILED(hr = Factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter))))
		{
			Debug::Log("[RenderDX] Failed to create WARP adapter: 0x%08X\n", static_cast<unsigned int>(hr));
			return false;
		}
		if (FAILED(hr = FP_D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device))))
		{
			Debug::Log("[RenderDX] Failed to create WARP adapter: 0x%08X\n", static_cast<unsigned int>(hr));
			return false;
		}

		Debug::Log("[RenderDX] D3D12 WARP device created successfully.\n");
	}

	return Device != nullptr;
}

bool DXRenderer::CreateCommandQueue() {
	HRESULT hr = S_OK;

	D3D12_COMMAND_QUEUE_DESC queueDesc {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	if (FAILED(hr = Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)))) {
		Debug::Log("[RenderDX] Failed to create command queue: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	Debug::Log("[RenderDX] Command queue created successfully.\n");
	return true;
}

bool DXRenderer::CreateSwapChain() {
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC1 desc {};
	desc.Width = WindowWidth;
	desc.Height = WindowHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = kFrameCount;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	if (FAILED(hr = Factory->CreateSwapChainForHwnd(CommandQueue.Get(), Hwnd, &desc, nullptr, nullptr, &swapChain1))) {
		Debug::Log("[RenderDX] Failed to create swap chain: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	if (FAILED(hr = Factory->MakeWindowAssociation(Hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN))) {
		Debug::Log("[RenderDX] Failed to set window association: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	if (FAILED(hr = swapChain1->QueryInterface(IID_PPV_ARGS(&SwapChain)))) {
		Debug::Log("[RenderDX] Failed to query IDXGISwapChain3 interface: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	Debug::Log("[RenderDX] Swap chain created successfully.\n");
	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
	return true;
}

bool DXRenderer::CreateRtvHeap() {
	HRESULT hr = S_OK;

	D3D12_DESCRIPTOR_HEAP_DESC desc {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = kFrameCount;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	if (FAILED(hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&RtvHeap)))) {
		Debug::Log("[RenderDX] Failed to create RTV descriptor heap: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	Debug::Log("[RenderDX] RTV descriptor heap created successfully.\n");
	return true;
}

bool DXRenderer::CreateRenderTargetViews() {
	HRESULT hr = S_OK;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < kFrameCount; ++i) {
		if (FAILED(hr = SwapChain->GetBuffer(i, IID_PPV_ARGS(&RenderTargets[i])))) {
			Debug::Log("[RenderDX] Failed to get back buffer %u: 0x%08X\n", i, static_cast<unsigned int>(hr));
			return false;
		}
		Device->CreateRenderTargetView(RenderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += RtvDescriptorSize;
	}

	Debug::Log("[RenderDX] Render target views created successfully.\n");
	return true;
}

bool DXRenderer::CreateSrvHeap() {
	HRESULT hr = S_OK;

	D3D12_DESCRIPTOR_HEAP_DESC desc {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1; // For surface texture SRV
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;

	if (FAILED(hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&SrvHeap)))) {
		Debug::Log("[RenderDX] Failed to create SRV descriptor heap: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	Debug::Log("[RenderDX] SRV descriptor heap created successfully.\n");
	return true;
}

bool DXRenderer::CreateSurfacePipeline() {
	D3D12_DESCRIPTOR_RANGE srvRange {};
	srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRange.NumDescriptors = 1;
	srvRange.BaseShaderRegister = 0;
	srvRange.RegisterSpace = 0;
	srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParam {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;
	rootParam.DescriptorTable.pDescriptorRanges = &srvRange;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC sampler {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MipLODBias = 0.0f;
	sampler.MaxAnisotropy = 1;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc {};
	rootSigDesc.NumParameters = 1;
	rootSigDesc.pParameters = &rootParam;
	rootSigDesc.NumStaticSamplers = 1;
	rootSigDesc.pStaticSamplers = &sampler;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	HRESULT hr = FP_D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&rootSigBlob,
		&errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob)
			Debug::Log("[RenderDX] Root signature serialization error: %s\n", static_cast<const char*>(errorBlob->GetBufferPointer()));
		else
			Debug::Log("[RenderDX] Unknown root signature serialization error.\n");
		return false;
	}

	if (FAILED(hr = Device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)))) {
		Debug::Log("[RenderDX] Failed to create root signature: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	static constexpr const char* shaderSource = R"(
Texture2D<float4> gSurface : register(t0);
SamplerState gSampler : register(s0);

struct VSOut
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

VSOut VSMain(uint vertexId : SV_VertexID)
{
    VSOut o;

	// Full-screen triangle vertices and UVs
    float2 positions[3] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f,  3.0f),
        float2( 3.0f, -1.0f)
    };

    float2 uvs[3] =
    {
        float2(0.0f,  1.0f),
        float2(0.0f, -1.0f),
        float2(2.0f,  1.0f)
    };

    o.position = float4(positions[vertexId], 0.0f, 1.0f);
    o.uv = uvs[vertexId];

    return o;
}

float4 PSMain(VSOut input) : SV_Target0
{
    return gSurface.Sample(gSampler, input.uv);
}
)";

	auto vertexShader = CompileShader(shaderSource, "VSMain", "vs_5_0");
	auto pixelShader = CompileShader(shaderSource, "PSMain", "ps_5_0");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc {};
	psoDesc.pRootSignature = RootSignature.Get();
	psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
	psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
	psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
	psoDesc.BlendState.IndependentBlendEnable = FALSE;
	D3D12_RENDER_TARGET_BLEND_DESC rtBlend {};
	rtBlend.BlendEnable = FALSE;
	rtBlend.LogicOpEnable = FALSE;
	rtBlend.SrcBlend = D3D12_BLEND_ONE;
	rtBlend.DestBlend = D3D12_BLEND_ZERO;
	rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
	rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.BlendState.RenderTarget[0] = rtBlend;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
	psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;
	psoDesc.RasterizerState.MultisampleEnable = FALSE;
	psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	psoDesc.DepthStencilState.BackFace = psoDesc.DepthStencilState.FrontFace;
	psoDesc.InputLayout = {};
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;

	if (FAILED(hr = Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PipelineState)))) {
		Debug::Log("[RenderDX] Failed to create pipeline state: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	Debug::Log("[RenderDX] Pipeline state created successfully.\n");
	return true;
}

bool DXRenderer::CreateCommandObjects() {
	HRESULT hr = S_OK;

	for (UINT i = 0; i < kFrameCount; ++i) {
		if (FAILED(hr = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[i])))) {
			Debug::Log("[RenderDX] Failed to create command allocator %u: 0x%08X\n", i, static_cast<unsigned int>(hr));
			return false;
		}
	}

	if (FAILED(hr = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocators[FrameIndex].Get(), PipelineState.Get(), IID_PPV_ARGS(&CommandList)))) {
		Debug::Log("[RenderDX] Failed to create command list: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	Debug::Log("[RenderDX] Command objects created successfully.\n");
	CommandList->Close(); // Command list needs to be closed before reset in the render loop.
	return true;
}

bool DXRenderer::CreateFenceObjects() {
	HRESULT hr = S_OK;

	FenceValues.fill(0);

	if (FAILED(hr = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)))) {
		Debug::Log("[RenderDX] Failed to create fence: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	FenceValues[FrameIndex] = 1;
	FenceEvent = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);

	if (!FenceEvent) {
		Debug::Log("[RenderDX] Failed to create fence event: 0x%08X\n", ::GetLastError());
		return false;
	}

	Debug::Log("[RenderDX] Fence objects created successfully.\n");
	return true;
}

bool DXRenderer::CreateFixedSurfaceGpuResources() {
	HRESULT hr = S_OK;

	const UINT sourceRowBytes = RenderWidth * sizeof(std::uint16_t);
	SurfaceUploadRowPitch = (sourceRowBytes + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1); // Align up
	SurfaceUploadBufferSize = static_cast<UINT64>(SurfaceUploadRowPitch) * static_cast<UINT64>(RenderHeight);

	D3D12_HEAP_PROPERTIES defaultHeap {};
	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultHeap.CreationNodeMask = 1;
	defaultHeap.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC textureDesc {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Alignment = 0;
	textureDesc.Width = RenderWidth;
	textureDesc.Height = RenderHeight;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_B5G6R5_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	SurfaceTextureState = D3D12_RESOURCE_STATE_COPY_DEST;

	if (FAILED(hr = Device->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		SurfaceTextureState,
		nullptr,
		IID_PPV_ARGS(&SurfaceTexture)
	))) {
		Debug::Log("[RenderDX] Failed to create surface texture resource: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	D3D12_HEAP_PROPERTIES uploadHeap {};
	uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeap.CreationNodeMask = 1;
	uploadHeap.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC uploadDesc {};
	uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadDesc.Alignment = 0;
	uploadDesc.Width = SurfaceUploadBufferSize;
	uploadDesc.Height = 1;
	uploadDesc.DepthOrArraySize = 1;
	uploadDesc.MipLevels = 1;
	uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadDesc.SampleDesc.Count = 1;
	uploadDesc.SampleDesc.Quality = 0;
	uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	for (UINT i = 0; i < kFrameCount; ++i) {
		if (FAILED(hr = Device->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&uploadDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&SurfaceUploadBuffers[i])
		))) {
			Debug::Log("[RenderDX] Failed to create surface upload buffer %u: 0x%08X\n", i, static_cast<unsigned int>(hr));
			return false;
		}

		D3D12_RANGE readRange {};
		readRange.Begin = 0;
		readRange.End = 0;

		if (FAILED(SurfaceUploadBuffers[i]->Map(0, &readRange, reinterpret_cast<void**>(&SurfaceUploadMapped[i])))) {
			Debug::Log("[RenderDX] Failed to map surface upload buffer %u: 0x%08X\n", i, static_cast<unsigned int>(hr));
			return false;
		}
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc {};
	srvDesc.Format = DXGI_FORMAT_B5G6R5_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	Device->CreateShaderResourceView(
		SurfaceTexture.Get(),
		&srvDesc,
		SrvHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return true;
}

void DXRenderer::UpdateViewportAndScissor() {
	if (ScaleRender) {
		RenderViewportX = 0.0f;
		RenderViewportY = 0.0f;
		RenderViewportWidth = static_cast<float>(WindowWidth);
		RenderViewportHeight = static_cast<float>(WindowHeight);

		if (DXRenderOptions::Config().PreserveAspectRatio && RenderWidth > 0 && RenderHeight > 0 && WindowWidth > 0 && WindowHeight > 0) {
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
	HRESULT hr = S_OK;

	if (!CommandQueue || !Fence || !FenceEvent) {
		return true; // If we don't have the necessary objects, we can't wait, but also can't report an error. 
	}

	const UINT64 currentFenceValue = FenceValues[FrameIndex];
	if (FAILED(hr = CommandQueue->Signal(Fence.Get(), currentFenceValue))) {
		Debug::Log("[RenderDX] Failed to signal command queue for GPU synchronization: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	if (FAILED(hr = Fence->SetEventOnCompletion(currentFenceValue, FenceEvent))) {
		Debug::Log("[RenderDX] Failed to set event on fence completion for GPU synchronization: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	DWORD waitResult = ::WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);
	if (waitResult != WAIT_OBJECT_0) {
		Debug::Log("[RenderDX] Wait for GPU synchronization event failed with error code: 0x%08X\n", ::GetLastError());
		return false;
	}

	++FenceValues[FrameIndex];
	return true;
}

Microsoft::WRL::ComPtr<ID3DBlob> DXRenderer::CompileShader(std::string_view source, std::string_view entryPoint, std::string_view target) {
	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	UINT compileFlags = 0;
	HRESULT hr = FP_D3DCompile(source.data(), source.length(), nullptr, nullptr, nullptr, entryPoint.data(), target.data(), compileFlags, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr)) {
		if (errorBlob)
			Debug::Log("[RenderDX] Shader compilation error: %s\n", static_cast<const char*>(errorBlob->GetBufferPointer()));
		else
			Debug::Log("[RenderDX] Unknown shader compilation error.\n");
	}

	return shaderBlob;
}

bool DXRenderer::PopulateCommandListForCPUSurface(const void* pixels, int source_pitch) {
	HRESULT hr = S_OK;

	if (FAILED(hr = CommandAllocators[FrameIndex]->Reset())) {
		Debug::Log("[RenderDX] Failed to reset command allocator for populating command list: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	if (FAILED(hr = CommandList->Reset(CommandAllocators[FrameIndex].Get(), PipelineState.Get()))) {
		Debug::Log("[RenderDX] Failed to reset command list for populating commands: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	UploadSurfaceToGpu(pixels, source_pitch);

	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER backBufferToRenderTarget {};
	backBufferToRenderTarget.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBufferToRenderTarget.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBufferToRenderTarget.Transition.pResource = RenderTargets[FrameIndex].Get();
	backBufferToRenderTarget.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	backBufferToRenderTarget.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	backBufferToRenderTarget.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	CommandList->ResourceBarrier(1, &backBufferToRenderTarget);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += static_cast<SIZE_T>(FrameIndex) * static_cast<SIZE_T>(RtvDescriptorSize);
	CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	CommandList->SetDescriptorHeaps(1, SrvHeap.GetAddressOf());
	CommandList->SetGraphicsRootSignature(RootSignature.Get());
	CommandList->SetGraphicsRootDescriptorTable(0, SrvHeap->GetGPUDescriptorHandleForHeapStart());
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList->DrawInstanced(3, 1, 0, 0);

	D3D12_RESOURCE_BARRIER backBufferToPresent {};
	backBufferToPresent.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBufferToPresent.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBufferToPresent.Transition.pResource = RenderTargets[FrameIndex].Get();
	backBufferToPresent.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	backBufferToPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBufferToPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	CommandList->ResourceBarrier(1, &backBufferToPresent);

	if (FAILED(hr = CommandList->Close())) {
		Debug::Log("[RenderDX] Failed to close command list after populating commands: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	return true;
}

void DXRenderer::UploadSurfaceToGpu(const void* pixels, int source_pitch) {
	auto dstBase = SurfaceUploadMapped[FrameIndex];
	const auto* srcBase = static_cast<const std::uint8_t*>(pixels);
	const UINT sourceRowBytes = RenderWidth * static_cast<UINT>(sizeof(std::uint16_t));
	for (int y = 0; y < RenderHeight; ++y) {
		auto* dstRow = dstBase + static_cast<size_t>(y) * SurfaceUploadRowPitch;
		const auto* srcRow = srcBase + static_cast<size_t>(y) * source_pitch;
		std::memcpy(dstRow, srcRow, sourceRowBytes);
	}

	TransitionSurfaceTexture(SurfaceTextureState, D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12_TEXTURE_COPY_LOCATION dstLocation {};
	dstLocation.pResource = SurfaceTexture.Get();
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION srcLocation {};
	srcLocation.pResource = SurfaceUploadBuffers[FrameIndex].Get();
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcLocation.PlacedFootprint.Offset = 0;
	srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_B5G6R5_UNORM;
	srcLocation.PlacedFootprint.Footprint.Width = RenderWidth;
	srcLocation.PlacedFootprint.Footprint.Height = RenderHeight;
	srcLocation.PlacedFootprint.Footprint.Depth = 1;
	srcLocation.PlacedFootprint.Footprint.RowPitch = SurfaceUploadRowPitch;

	CommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

	TransitionSurfaceTexture(SurfaceTextureState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void DXRenderer::TransitionSurfaceTexture(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	if (before == after) {
		return;
	}

	D3D12_RESOURCE_BARRIER barrier {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = SurfaceTexture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;

	CommandList->ResourceBarrier(1, &barrier);

	SurfaceTextureState = after;
}

bool DXRenderer::MoveToNextFrame() {
	HRESULT hr = S_OK;

	const UINT64 currentFenceValue = FenceValues[FrameIndex];
	if (FAILED(hr = CommandQueue->Signal(Fence.Get(), currentFenceValue))) {
		Debug::Log("[RenderDX] Failed to signal command queue for moving to next frame: 0x%08X\n", static_cast<unsigned int>(hr));
		return false;
	}

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
	if (Fence->GetCompletedValue() < FenceValues[FrameIndex]) {
		if (FAILED(hr = Fence->SetEventOnCompletion(FenceValues[FrameIndex], FenceEvent))) {
			Debug::Log("[RenderDX] Failed to set event on fence completion for moving to next frame: 0x%08X\n", static_cast<unsigned int>(hr));
			return false;
		}
		DWORD waitResult = ::WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);
		if (waitResult != WAIT_OBJECT_0) {
			Debug::Log("[RenderDX] Wait for fence event failed while moving to next frame with error code: 0x%08X\n", ::GetLastError());
			return false;
		}
	}

	FenceValues[FrameIndex] = currentFenceValue + 1;
	return true;
}
