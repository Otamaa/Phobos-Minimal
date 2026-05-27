#pragma once

#include <Windows.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#include <Lib/wil/com.h>

#include <array>
#include <string_view>
#include <d3dcommon.h>

#define DXRENDER_DEBUG 0

struct AdapterScore
{
	wil::com_ptr_nothrow<IDXGIAdapter1> adapter {};
	DXGI_ADAPTER_DESC1 desc {};
	UINT64 score {};

	static UINT64 ScoreAdapter(const DXGI_ADAPTER_DESC1& d);
};

struct DXCaps
{
	D3D_FEATURE_LEVEL FeatureLevel { D3D_FEATURE_LEVEL_12_0 };

	bool HasRaytracing {};
	bool HasVRS {};
	bool HasMeshShader {};
	bool HasSamplerFeedback {};
	bool HasDXIL {};

	D3D12_RESOURCE_BINDING_TIER ResourceBindingTier {};
	D3D12_CONSERVATIVE_RASTERIZATION_TIER ConservativeRasterTier {};
	D3D12_TILED_RESOURCES_TIER TiledResourcesTier {};

	bool HasShaderCache {};
	bool HasRenderPasses {};

	UINT VendorID {};
	UINT64 VRAM {};
	UINT64 VRAM_MB {};
	double VRAM_GB {};
	bool IsUMA {};
};

class DXRenderer {
public:
	static DXRenderer& Instance();

	bool CreateDevice();

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

	bool CreateDeviceInternal(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL& outLevel);
	void QueryCapabilities();

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
	wil::com_ptr_nothrow<ID3DBlob> CompileShader(std::string_view source, std::string_view entryPoint, std::string_view target);
	wil::com_ptr_nothrow<ID3DBlob> CompileTheShader(std::string_view source, std::string_view entryPoint, std::string_view target);
	bool SaveCompiledShader(const char* filename, ID3DBlob* blob);
	wil::com_ptr_nothrow<ID3DBlob> LoadCompiledShader(const char* filename);
	bool PopulateCommandListForCPUSurface(const void* pixels, int source_pitch);
	void UploadSurfaceToGpu(const void* pixels, int source_pitch);
	void TransitionSurfaceTexture(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	bool MoveToNextFrame();

	wil::unique_hmodule D3D12Lib { };
#if DXRENDER_DEBUG
	decltype(&D3D12GetDebugInterface) FP_D3D12GetDebugInterface { nullptr };
#endif
	decltype(&D3D12CreateDevice) FP_D3D12CreateDevice { nullptr };
	decltype(&D3D12SerializeRootSignature) FP_D3D12SerializeRootSignature { nullptr };

	wil::unique_hmodule DXGILib { };
	decltype(&CreateDXGIFactory2) FP_CreateDXGIFactory2 { nullptr };

	wil::unique_hmodule D3DCompilerLib { };
	decltype(&D3DCompile) FP_D3DCompile { nullptr };
	decltype(&D3DCreateBlob) FP_D3DCreateBlob { nullptr };

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

	DXCaps Caps {};

	wil::com_ptr_nothrow<IDXGIFactory6> Factory { };
	wil::com_ptr_nothrow<ID3D12Device> Device { };
	wil::com_ptr_nothrow<ID3D12CommandQueue> CommandQueue { };
	wil::com_ptr_nothrow<IDXGISwapChain3> SwapChain { };
	wil::com_ptr_nothrow<ID3D12DescriptorHeap> RtvHeap { };

	static constexpr UINT kFrameCount = 2;
	std::array<wil::com_ptr_nothrow<ID3D12Resource>, kFrameCount> RenderTargets {};

	std::array<wil::com_ptr_nothrow<ID3D12CommandAllocator>, kFrameCount> CommandAllocators {};
	wil::com_ptr_nothrow<ID3D12GraphicsCommandList> CommandList { };

	wil::com_ptr_nothrow<ID3D12Fence> Fence { };
	std::array<UINT64, kFrameCount> FenceValues {};
	wil::unique_handle FenceEvent { };

	D3D12_VIEWPORT Viewport {};
	D3D12_RECT ScissorRect {};

	wil::com_ptr_nothrow<ID3D12Resource> SurfaceTexture { };
	D3D12_RESOURCE_STATES SurfaceTextureState { D3D12_RESOURCE_STATE_COPY_DEST };

	std::array<wil::com_ptr_nothrow<ID3D12Resource>, kFrameCount> SurfaceUploadBuffers {};
	std::array<std::uint8_t*, kFrameCount> SurfaceUploadMapped {};

	UINT SurfaceUploadRowPitch { 0 };
	UINT64 SurfaceUploadBufferSize { 0 };

	wil::com_ptr_nothrow<ID3D12DescriptorHeap> SrvHeap { };
	wil::com_ptr_nothrow<ID3D12RootSignature> RootSignature { };
	wil::com_ptr_nothrow<ID3D12PipelineState> PipelineState { };
};
