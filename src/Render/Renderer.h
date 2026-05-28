#pragma once

#include <Windows.h>

#include <dxgi1_6.h>
#include <d3d11.h>

#include <wrl/client.h>

#define DXRENDER_DEBUG 0

class DXRenderer {
public:
	static DXRenderer& Instance();

	bool CreateMainWindow(HINSTANCE instance, int cmdShow, int width, int height, WNDPROC proc);
	void DestroyMainWindow();

	bool IsRendererReady();
	bool CreateRenderer(int width, int height, int bitsPerPixel);
	void DestroyRenderer();
	bool ResizeWindow(int width, int height);

	void ToggleFullscreen();

	bool UploadSurfaceToTexture(void* pSurfaceData, int sourcePitch);
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

	bool CreateFactory(UINT dxgiFactoryFlags);
	bool CreateDevice();
	bool CreateSwapChain();
	bool CreateFlipSwapChain(IDXGIFactory2* pFactory2, DXGI_SWAP_EFFECT swapEffect);
	bool CreateLegacySwapChain();
	bool CreateRenderTargetViews();
	bool CreateSurfacePipeline();
	bool CreateFixedSurfaceGpuResources();

	void UpdateViewportAndScissor();

	bool WaitForGpu();
	bool RenderSurface();
	bool UploadSurfaceToGpu(const void* pPixels, int sourcePitch);

	HMODULE D3D11Lib { nullptr }; // Loaded d3d11.dll handle.
	decltype(&D3D11CreateDevice) D3D11CreateDeviceProc { nullptr }; // D3D11CreateDevice entry point.

	HMODULE DXGILib { nullptr }; // Loaded dxgi.dll handle.
	decltype(&CreateDXGIFactory2) CreateDXGIFactory2Proc { nullptr }; // CreateDXGIFactory2 entry point.
	decltype(&CreateDXGIFactory1) CreateDXGIFactory1Proc { nullptr }; // CreateDXGIFactory1 entry point.
	decltype(&CreateDXGIFactory) CreateDXGIFactoryProc { nullptr }; // CreateDXGIFactory entry point.

	HWND Hwnd { nullptr }; // Main game window handle.
	int WindowWidth { 0 }; // Current window width.
	int WindowHeight { 0 }; // Current window height.
	float RenderViewportX { 0.0f }; // Current render viewport left in client coordinates.
	float RenderViewportY { 0.0f }; // Current render viewport top in client coordinates.
	float RenderViewportWidth { 0.0f }; // Current render viewport width in client coordinates.
	float RenderViewportHeight { 0.0f }; // Current render viewport height in client coordinates.
	RECT WindowedRect {}; // Saved window rectangle before borderless fullscreen.
	LONG_PTR WindowedStyle { 0 }; // Saved window style before borderless fullscreen.
	LONG_PTR WindowedExStyle { 0 }; // Saved extended window style before borderless fullscreen.
	int RenderWidth { 0 }; // Source render surface width.
	int RenderHeight { 0 }; // Source render surface height.
	UINT RenderPitch { 0 }; // Source render pitch.
	bool ScaleRender { true }; // Whether the source surface is scaled to the window.
	bool Windowed { true }; // Whether the window is in windowed mode.
	bool HasWindowedState { false }; // Whether windowed placement has been saved.

	Microsoft::WRL::ComPtr<IDXGIFactory> Factory; // Base DXGI factory used for adapter and swap-chain creation.
	Microsoft::WRL::ComPtr<ID3D11Device> Device; // D3D11 device.
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext; // Immediate D3D11 context.
	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain; // Window swap chain.
	UINT SwapChainBufferCount { 0 }; // Current swap-chain back-buffer count.

	static constexpr UINT FrameCount = 2;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView; // Back-buffer render target.

	D3D11_VIEWPORT Viewport {}; // Current render viewport.
	D3D11_RECT ScissorRect {}; // Current render scissor rectangle.

	Microsoft::WRL::ComPtr<ID3D11Texture2D> SurfaceTexture; // Dynamic RGB565 source texture.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SurfaceShaderResourceView; // Source texture SRV.

	Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader; // Fullscreen triangle vertex shader.
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader; // Surface sampling pixel shader.
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState; // Point sampler for pixel-perfect scaling.
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerState; // Rasterizer state with scissor enabled.
	Microsoft::WRL::ComPtr<ID3D11BlendState> BlendState; // Opaque blend state.
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState; // Disabled depth/stencil state.
};
