#pragma once

#include <Windows.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#include <wrl/client.h>

#include <array>
#include <string_view>
#include <d3dcommon.h>

#include <Helpers/CompileTime.h>

#ifdef DEBUG
#define DXRENDER_DEBUG 0
#else
#define DXRENDER_DEBUG 1
#endif

class DXRenderer {
public:
	static DXRenderer& Instance();

	bool CreateMainWindow(HINSTANCE instance, int cmd_show, int width, int height, WNDPROC proc);
	void DestroyMainWindow();

	bool IsRendererReady();
	bool CreateRenderer(int width, int height, int bits_per_pixel);
	void DestroyRenderer();
	bool ResizeWindow(int width, int height);

	void ToggleFullscreen();

	bool UploadSurfaceToTexture(void* surface_data, int source_pitch);
	void SetRenderScale(bool scale);
	bool Present();

	void MoveWindow(int x, int y, int width, int height);
	bool IsWindowed() const;

	int GetWindowWidth() const;
	int GetWindowHeight() const;
	float GetViewportX() const;
	float GetViewportY() const;
	float GetViewportWidth() const;
	float GetViewportHeight() const;

private:
	DXRenderer();
	~DXRenderer();

	bool LoadImports();
	void UnloadImports();

	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateSwapChain();
	bool CreateRtvHeap();
	bool CreateRenderTargetViews();
	bool CreateSrvHeap();
	bool CreateSurfacePipeline();
	bool CreateCommandObjects();
	bool CreateFenceObjects();
	bool CreateFixedSurfaceGpuResources();

	void UpdateViewportAndScissor();

	bool WaitForGpu();
	Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(std::string_view source, std::string_view entryPoint, std::string_view target);
	bool PopulateCommandListForCPUSurface(const void* pixels, int source_pitch);
	void UploadSurfaceToGpu(const void* pixels, int source_pitch);
	void TransitionSurfaceTexture(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	bool MoveToNextFrame();

	HMODULE D3D12Lib { nullptr };
#if DXRENDER_DEBUG
	decltype(&D3D12GetDebugInterface) FP_D3D12GetDebugInterface { nullptr };
#endif
	decltype(&D3D12CreateDevice) FP_D3D12CreateDevice { nullptr };
	decltype(&D3D12SerializeRootSignature) FP_D3D12SerializeRootSignature { nullptr };

	HMODULE DXGILib;
	decltype(&CreateDXGIFactory2) FP_CreateDXGIFactory2 { nullptr };

	HMODULE D3DCompilerLib { nullptr };
	decltype(&D3DCompile) FP_D3DCompile { nullptr };

	HWND Hwnd { nullptr };
	int WindowWidth { 0 };
	int WindowHeight { 0 };
	float RenderViewportX { 0.0f }; // Current render viewport left in client coordinates.
	float RenderViewportY { 0.0f }; // Current render viewport top in client coordinates.
	float RenderViewportWidth { 0.0f }; // Current render viewport width in client coordinates.
	float RenderViewportHeight { 0.0f }; // Current render viewport height in client coordinates.
	RECT WindowedRect {}; // Saved window rectangle before borderless fullscreen.
	LONG_PTR WindowedStyle { 0 }; // Saved window style before borderless fullscreen.
	LONG_PTR WindowedExStyle { 0 }; // Saved extended window style before borderless fullscreen.
	int RenderWidth { 0 };
	int RenderHeight { 0 };
	UINT RenderPitch { 0 };
	bool ScaleRender { true };
	bool Windowed { true };
	bool HasWindowedState { false }; // Whether windowed placement has been saved.

	UINT FrameIndex { 0 };
	UINT RtvDescriptorSize { 0 };

	Microsoft::WRL::ComPtr<IDXGIFactory6> Factory;
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RtvHeap;

	static constexpr UINT kFrameCount = 2;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kFrameCount> RenderTargets {};

	std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, kFrameCount> CommandAllocators {};
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;

	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	std::array<UINT64, kFrameCount> FenceValues {};
	HANDLE FenceEvent;

	D3D12_VIEWPORT Viewport {};
	D3D12_RECT ScissorRect {};

	Microsoft::WRL::ComPtr<ID3D12Resource> SurfaceTexture;
	D3D12_RESOURCE_STATES SurfaceTextureState { D3D12_RESOURCE_STATE_COPY_DEST };

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kFrameCount> SurfaceUploadBuffers {};
	std::array<std::uint8_t*, kFrameCount> SurfaceUploadMapped {};

	UINT SurfaceUploadRowPitch { 0 };
	UINT64 SurfaceUploadBufferSize { 0 };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SrvHeap;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;
};
