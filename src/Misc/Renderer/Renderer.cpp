#include "Renderer.h"

#include <Utilities/Debug.h>

#include <CommCtrl.h>

#include <Unsorted.h>
#include <ColorStruct.h>
#include <Phobos.Lua.h>

#include "Functions.h"

#include <algorithm>
#include <fstream>
#include <filesystem>

struct DXConfig
{
	static bool WindowedBorder;
	static bool StartFullscreen;
	static bool PreserveAspectRatio;
	static bool EnableVSync;
};

bool DXConfig::WindowedBorder { true };
bool DXConfig::StartFullscreen { false };
bool DXConfig::PreserveAspectRatio { false };
bool DXConfig::EnableVSync { false };

DXRenderer& DXRenderer::Instance() {
	static DXRenderer instance;
	return instance;
}

static LONG_PTR GetConfiguredWindowedStyle(LONG_PTR style, bool visible) {
	if (DXConfig::WindowedBorder)
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
	if (DXConfig::WindowedBorder)
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

	if (!Phobos::AppIconPath.empty()) {
		Debug::LogInfo("Applying AppIcon from \"{}\"", Phobos::AppIconPath.c_str());
		wc.hIcon = (HICON)::LoadImageA(instance, Phobos::AppIconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	} else {
		wc.hIcon = ::LoadIconA(instance, MAKEINTRESOURCEA(93));
	}

	wc.hCursor = ::LoadCursorA(nullptr, IDC_ARROW);
	wc.lpszClassName = LuaData::MainWindowStr.c_str();
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

	if (!Game::hWnd()) {
		Debug::Log("[RenderDX] Failed to create main window\n");
		return false;
	}

	// Disable clipping because we draw Win32 child windows as part of the main window
	style = GetWindowLongPtrA(Game::hWnd(), GWL_STYLE);
	style &= ~(WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	SetWindowLongPtrA(Game::hWnd(), GWL_STYLE, style);

	Hwnd = Game::hWnd();
	WindowWidth = window_width;
	WindowHeight = window_height;

	if (DXConfig::StartFullscreen)
		ToggleFullscreen();

	::ShowWindow(Game::hWnd(), cmd_show);
	::UpdateWindow(Game::hWnd());

	Game::IMEContext = ::ImmGetContext(Game::hWnd());
	::ImmAssociateContext(Game::hWnd(), nullptr);

	::RegisterHotKey(Game::hWnd(), 1, MOD_ALT | MOD_CONTROL | MOD_SHIFT, 'M');

	// Gain focus for the game window to ensure it receives input
	::SetForegroundWindow(Game::hWnd());
	Game::IsFocused = true;

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
		SurfaceTexture.reset();
	}

	for (UINT i = 0; i < kFrameCount; ++i) {
		if (SurfaceUploadBuffers[i] && SurfaceUploadMapped[i]) {
			SurfaceUploadBuffers[i]->Unmap(0, nullptr);
			SurfaceUploadMapped[i] = nullptr;
		}
	}

	for (UINT i = 0; i < kFrameCount; ++i) {
		if (SurfaceUploadBuffers[i]) {
			SurfaceUploadBuffers[i].reset();
		}
	}

	if (Fence) {
		Fence.reset();
	}
	if (FenceEvent) {
		FenceEvent.reset();
	}

	if (CommandList) {
		CommandList.reset();
	}
	for (UINT i = 0; i < kFrameCount; ++i) {
		if (CommandAllocators[i]) {
			CommandAllocators[i].reset();
		}
	}

	if (PipelineState) {
		PipelineState.reset();
	}
	if (RootSignature) {
		RootSignature.reset();
	}

	if (SrvHeap) {
		SrvHeap.reset();
	}

	for (UINT i = 0; i < kFrameCount; ++i) {
		if (RenderTargets[i]) {
			RenderTargets[i].reset();
		}
	}

	if (RtvHeap) {
		RtvHeap.reset();
	}
	RtvDescriptorSize = 0;

	if (SwapChain) {
		BOOL fullscreenState = FALSE;
		wil::com_ptr_nothrow<IDXGIOutput> pTarget;
		if (SUCCEEDED(SwapChain->GetFullscreenState(&fullscreenState, &pTarget)) && fullscreenState)
			SwapChain->SetFullscreenState(FALSE, nullptr);

		SwapChain.reset();
	}
	FrameIndex = 0;

	if (CommandQueue) {
		CommandQueue.reset();
	}

	if (Device) {
		Device.reset();
	}
	if (Factory) {
		Factory.reset();
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
		target.reset();
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

		if (!DXConfig::WindowedBorder) {
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

	ID3D12CommandList* list[] = { CommandList.get() };
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
	UINT syncInterval = DXConfig::EnableVSync ? 1 : 0;

	if (FAILED(SwapChain->Present(syncInterval, 0))) {
		Debug::Log("[RenderDX] Failed to present swap chain.\n");
		return false;
	}

	return MoveToNextFrame();
}

void DXRenderer::MoveWindow(int x, int y, int width, int height) {
	if (Windowed && !DXConfig::WindowedBorder) {
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

template<typename T>
bool LoadProc(HMODULE module, const char* name, T& out, const char* debugName) {
	out = reinterpret_cast<T>(::GetProcAddress(module, name));

	if (!out) {
		Debug::Log("[RenderDX] Missing export: %s\n", debugName);
		return false;
	}
	return true;
}

static HMODULE LoadD3DCompilerDLL()
{
	static const wchar_t* candidates[] = {
		L"d3dcompiler_47.dll",
		L"d3dcompiler_46.dll",
		L"d3dcompiler_43.dll"
	};

	for (auto dll : candidates) {
		HMODULE mod = ::LoadLibraryW(dll);
		if (mod) {
			Debug::Log("[RenderDX] Loaded shader compiler: %ls\n", dll);
			return mod;
		}
	}

	Debug::Log("[RenderDX] WARNING: No d3dcompiler DLL found.\n");
	return nullptr;
}

bool DXRenderer::LoadImports()
{
	// =========================
	// D3D12 CORE
	// =========================
	D3D12Lib.reset(::LoadLibraryW(L"d3d12.dll"));
	if (!D3D12Lib)
	{
		Debug::Log("[RenderDX] Failed to load d3d12.dll.\n");
		return false;
	}

#if DXRENDER_DEBUG
	LoadProc(D3D12Lib.get(),
		"D3D12GetDebugInterface",
		FP_D3D12GetDebugInterface,
		"D3D12GetDebugInterface");
#endif

	if (!LoadProc(D3D12Lib.get(),
		"D3D12CreateDevice",
		FP_D3D12CreateDevice,
		"D3D12CreateDevice"))
	{
		return false;
	}

	if (!LoadProc(D3D12Lib.get(),
		"D3D12SerializeRootSignature",
		FP_D3D12SerializeRootSignature,
		"D3D12SerializeRootSignature"))
	{
		return false;
	}

	// =========================
	// DXGI
	// =========================
	DXGILib.reset(::LoadLibraryW(L"dxgi.dll"));
	if (!DXGILib)
	{
		Debug::Log("[RenderDX] Failed to load dxgi.dll.\n");
		return false;
	}

	if (!LoadProc(DXGILib.get(),
		"CreateDXGIFactory2",
		FP_CreateDXGIFactory2,
		"CreateDXGIFactory2"))
	{
		return false;
	}

	// =========================
	// D3D COMPILER (OPTIONAL)
	// =========================
	D3DCompilerLib.reset(LoadD3DCompilerDLL());

	if (D3DCompilerLib)
	{
		bool ok = true;

		ok &= LoadProc(D3DCompilerLib.get(),
			"D3DCompile",
			FP_D3DCompile,
			"D3DCompile");

		ok &= LoadProc(D3DCompilerLib.get(),
			"D3DCreateBlob",
			FP_D3DCreateBlob,
			"D3DCreateBlob");

		if (!ok)
		{
			Debug::Log("[RenderDX] Shader compiler partially failed (disabled).\n");

			FP_D3DCompile = nullptr;
			FP_D3DCreateBlob = nullptr;
		}
	}
	else
	{
		// not fatal anymore
		FP_D3DCompile = nullptr;
		FP_D3DCreateBlob = nullptr;
	}

	return true;
}

void DXRenderer::UnloadImports() {
	if (D3DCompilerLib) {
		D3DCompilerLib.reset();
		FP_D3DCompile = nullptr;
		FP_D3DCreateBlob = nullptr;
	}
	if (DXGILib) {
		DXGILib.reset();
		FP_CreateDXGIFactory2 = nullptr;
	}
	if (D3D12Lib) {
		D3D12Lib.reset();
#if DXRENDER_DEBUG
		FP_D3D12GetDebugInterface = nullptr;
#endif
		FP_D3D12CreateDevice = nullptr;
		FP_D3D12SerializeRootSignature = nullptr;
	}
}

UINT64 AdapterScore::ScoreAdapter(const DXGI_ADAPTER_DESC1& d)
{
	UINT64 score = 0;

	// Must not be software
	if (d.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		return 0;

	// Prefer dedicated GPU memory
	score += d.DedicatedVideoMemory / (1024 * 1024); // MB

	// Huge boost for real discrete GPUs
	if (d.DedicatedVideoMemory > 4ull * 1024 * 1024 * 1024)
		score += 100000;

	// Slight boost for known discrete class
	if (d.DedicatedVideoMemory > 0)
		score += 50000;

	return score;
}

bool DXRenderer::CreateDevice()
{
	UINT factoryFlags = 0;

#if DXRENDER_DEBUG
	wil::com_ptr_nothrow<ID3D12Debug> debugController;
	if (SUCCEEDED(FP_D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	if (FAILED(FP_CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&Factory))))
		return false;

	D3D_FEATURE_LEVEL selectedLevel = D3D_FEATURE_LEVEL_12_0;

	wil::com_ptr_nothrow<IDXGIAdapter1> bestAdapter {};
	UINT64 bestScore = 0;

	// -------------------------------------------------
	// 1. Adapter selection (score-based, safe for all GPUs)
	// -------------------------------------------------
	for (UINT i = 0;; i++)
	{
		wil::com_ptr_nothrow<IDXGIAdapter1> adapter {};

		if (Factory->EnumAdapterByGpuPreference(
			i,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&adapter)) == DXGI_ERROR_NOT_FOUND)
		{
			break;
		}

		DXGI_ADAPTER_DESC1 desc {};
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		UINT64 score = AdapterScore::ScoreAdapter(desc);

		Debug::Log("[RenderDX] Adapter %d: %ls | USED VRAM=%llu MB\n",
			i,
			desc.Description,
			desc.DedicatedVideoMemory / (1024ull * 1024ull));

		if (score > bestScore)
		{
			bestScore = score;
			bestAdapter = adapter;
		}
	}

	if (!bestAdapter)
	{
		Debug::Log("[RenderDX] No suitable GPU found.\n");
		return false;
	}

	// -------------------------------------------------
	// 2. Create D3D12 device
	// -------------------------------------------------
	if (!CreateDeviceInternal(bestAdapter.get(), selectedLevel))
	{
		Debug::Log("[RenderDX] Failed to create D3D12 device.\n");
		return false;
	}
	UINT nodeCount = Device->GetNodeCount();
	Debug::Log("[RenderDX] D3D12 Node Count = %u\n", nodeCount);

	// -------------------------------------------------
	// 3. Query FINAL adapter info (IMPORTANT FIX)
	// -------------------------------------------------
	DXGI_ADAPTER_DESC1 desc {};
	bestAdapter->GetDesc1(&desc);

	Caps.VendorID = desc.VendorId;
	Caps.IsUMA = (desc.DedicatedVideoMemory == 0);

	// fallback (still useful)
	Caps.VRAM = desc.DedicatedVideoMemory / (1024ull * 1024ull);

	// REAL memory query
	wil::com_ptr_nothrow<IDXGIAdapter3> adapter3 {};
	if (SUCCEEDED(bestAdapter->QueryInterface(IID_PPV_ARGS(&adapter3))))
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO info {};
		if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(
			0,
			DXGI_MEMORY_SEGMENT_GROUP_LOCAL,
			&info)))
		{
			Caps.VRAM_MB = info.Budget / (1024ull * 1024ull);
			Caps.VRAM_GB = static_cast<double>(Caps.VRAM_MB) / 1024.0;
		}
	}
	else
	{
		// fallback to DXGI desc
		Caps.VRAM_MB = Caps.VRAM / (1024ull * 1024ull);
		Caps.VRAM_GB = static_cast<double>(Caps.VRAM_MB) / 1024.0;
	}

	Debug::Log(
		"[RenderDX] Adapter: %ls | Vendor: 0x%x | UMA: %s | VRAM: %llu / %llu MB (%.2f GB)\n",
		desc.Description,
		Caps.VendorID,
		Caps.IsUMA ? "Yes" : "No",
		Caps.VRAM,
		Caps.VRAM_MB,
		Caps.VRAM_GB
	);

	// -------------------------------------------------
	// 4. Feature level store
	// -------------------------------------------------
	Caps.FeatureLevel = selectedLevel;

	// -------------------------------------------------
	// 5. Query GPU capabilities (DXIL / RT / Mesh etc.)
	// -------------------------------------------------
	QueryCapabilities();

	Debug::Log("[RenderDX] Device created. FeatureLevel = 0x%x\n", selectedLevel);

	return true;
}

bool DXRenderer::CreateDeviceInternal(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL& outLevel)
{
	const D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};

	for (auto level : levels)
	{
		if (SUCCEEDED(FP_D3D12CreateDevice(
			adapter,
			level,
			IID_PPV_ARGS(&Device))))
		{
			outLevel = level;
			return true;
		}
	}

	return false;
}

void DXRenderer::QueryCapabilities()
{
	if (!Device)
		return;

	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS opts {
			.TiledResourcesTier = D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED,
			.ResourceBindingTier = D3D12_RESOURCE_BINDING_TIER_1,
			.ConservativeRasterizationTier = D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED
		};

		if (SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &opts, sizeof(opts))))
		{
			Caps.ResourceBindingTier = opts.ResourceBindingTier;
			Caps.ConservativeRasterTier = opts.ConservativeRasterizationTier;
			Caps.TiledResourcesTier = opts.TiledResourcesTier;
		}
		else
		{
			Debug::Log("[RenderDX] D3D12_OPTIONS query FAILED\n");
		}
	}

	{
		D3D12_FEATURE_DATA_SHADER_CACHE sc { .SupportFlags = D3D12_SHADER_CACHE_SUPPORT_NONE };
		if (SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE, &sc, sizeof(sc))))
		{
			Caps.HasShaderCache = sc.SupportFlags != D3D12_SHADER_CACHE_SUPPORT_NONE;
		}
	}

	std::string sm_str { "Unknown" };

	{
		D3D12_FEATURE_DATA_SHADER_MODEL sm { .HighestShaderModel = D3D_SHADER_MODEL_6_9 };

		COMPILETIMEEVAL auto ToString = [](D3D_SHADER_MODEL model)
			{
				switch (model)
				{
				case D3D_SHADER_MODEL_5_1: return "5.1";
				case D3D_SHADER_MODEL_6_0: return "6.0";
				case D3D_SHADER_MODEL_6_1: return "6.1";
				case D3D_SHADER_MODEL_6_2: return "6.2";
				case D3D_SHADER_MODEL_6_3: return "6.3";
				case D3D_SHADER_MODEL_6_4: return "6.4";
				case D3D_SHADER_MODEL_6_5: return "6.5";
				case D3D_SHADER_MODEL_6_6: return "6.6";
				case D3D_SHADER_MODEL_6_7: return "6.7";
				case D3D_SHADER_MODEL_6_8: return "6.8";
				case D3D_SHADER_MODEL_6_9: return "6.9";
				default: return "Unknown";
				}
			};

		HRESULT hr = Device->CheckFeatureSupport(
			D3D12_FEATURE_SHADER_MODEL,
			&sm,
			sizeof(sm)
		);

		if (SUCCEEDED(hr))
		{
			Caps.HasDXIL = sm.HighestShaderModel >= D3D_SHADER_MODEL_6_0;
			sm_str = ToString(sm.HighestShaderModel);
		}
		else
		{
			Debug::Log("[RenderDX] ShaderModel query FAILED: 0x%x\n", hr);
			Caps.HasDXIL = false;
		}
	}

	D3D12_RAYTRACING_TIER RT_Tier = D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
	std::string rt_tier_str = "NOT_SUPPORTED";

	// ---- Raytracing / Mesh / modern features ----
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 opt5 {};

		Debug::Log("[RenderDX] Querying D3D12_FEATURE_D3D12_OPTIONS5 (size=%zu)...\n", sizeof(opt5));

		HRESULT hr = Device->CheckFeatureSupport(
			D3D12_FEATURE_D3D12_OPTIONS5,
			&opt5,
			sizeof(opt5));

		Debug::Log("[RenderDX] CheckFeatureSupport result: 0x%x (%s)\n",
			hr, SUCCEEDED(hr) ? "SUCCESS" : "FAILED");

		if (SUCCEEDED(hr))
		{
			// Log ALL fields from OPTIONS5
			Debug::Log("[RenderDX] OPTIONS5 results:\n");
			Debug::Log("  - SRVOnlyTiledResourceTier3: %d\n", opt5.SRVOnlyTiledResourceTier3);
			Debug::Log("  - RenderPassesTier: %d\n", (int)opt5.RenderPassesTier);
			Debug::Log("  - RaytracingTier: %d\n", (int)opt5.RaytracingTier);

			RT_Tier = opt5.RaytracingTier;
			Caps.HasRenderPasses = opt5.RenderPassesTier != D3D12_RENDER_PASS_TIER_0;
			Caps.HasRaytracing = opt5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;

			// Convert tier to string for better logging
			switch (opt5.RaytracingTier)
			{
			case D3D12_RAYTRACING_TIER_NOT_SUPPORTED: rt_tier_str = "NOT_SUPPORTED"; break;
			case D3D12_RAYTRACING_TIER_1_0: rt_tier_str = "1.0"; break;
			case D3D12_RAYTRACING_TIER_1_1: rt_tier_str = "1.1"; break;
			default: rt_tier_str = std::format("UNKNOWN({})", (int)opt5.RaytracingTier); break;
			}
		}
		else
		{
			Debug::Log("[RenderDX] D3D12_OPTIONS5 (Raytracing) query FAILED: 0x%x\n", hr);
			Caps.HasRaytracing = false;
		}
	}

	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS6 opt6 {
			.VariableShadingRateTier = D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED
		};

		if (SUCCEEDED(Device->CheckFeatureSupport(
			D3D12_FEATURE_D3D12_OPTIONS6,
			&opt6,
			sizeof(opt6))))
		{
			Caps.HasVRS =
				opt6.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
		}
	}

	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 opt7 {
			.MeshShaderTier = D3D12_MESH_SHADER_TIER_NOT_SUPPORTED,
			.SamplerFeedbackTier = D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED
		};
		if (SUCCEEDED(Device->CheckFeatureSupport(
			D3D12_FEATURE_D3D12_OPTIONS7,
			&opt7,
			sizeof(opt7))))
		{
			Caps.HasMeshShader =
				opt7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;

			Caps.HasSamplerFeedback =
				opt7.SamplerFeedbackTier != D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED;
		}
	}

	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS12 opt12 {};
		if (SUCCEEDED(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &opt12, sizeof(opt12))))
		{
			Debug::Log("[RenderDX] OPTIONS12 - EnhancedBarriersSupported: %d\n", opt12.EnhancedBarriersSupported);
		}
	}

	Debug::LogInfo("[RenderDX] === FINAL CAPS ===");
	Debug::LogInfo("[RenderDX] DXIL: {}", Caps.HasDXIL);
	Debug::LogInfo("[RenderDX] Raytracing: {} (Tier: {})", Caps.HasRaytracing, rt_tier_str);
	Debug::LogInfo("[RenderDX] ShaderModel: {}", sm_str);
	Debug::LogInfo("[RenderDX] VRS: {}", Caps.HasVRS);
	Debug::LogInfo("[RenderDX] MeshShader: {}", Caps.HasMeshShader);
	Debug::LogInfo("[RenderDX] SamplerFeedback: {}", Caps.HasSamplerFeedback);
	Debug::LogInfo("[RenderDX] ShaderCache: {}", Caps.HasShaderCache);
	Debug::LogInfo("[RenderDX] RenderPasses: {}", Caps.HasRenderPasses);

	Debug::LogInfo("[RenderDX] ResourceBindingTier: {}", (int)Caps.ResourceBindingTier);
	Debug::LogInfo("[RenderDX] ConservativeRasterTier: {}", (int)Caps.ConservativeRasterTier);
	Debug::LogInfo("[RenderDX] TiledResourcesTier: {}", (int)Caps.TiledResourcesTier);

}
bool DXRenderer::CreateCommandQueue() {
	D3D12_COMMAND_QUEUE_DESC queueDesc {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	if (FAILED(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)))) {
		Debug::Log("[RenderDX] Failed to create command queue.\n");
		return false;
	}

	Debug::Log("[RenderDX] Command queue created successfully.\n");
	return true;
}

bool DXRenderer::CreateSwapChain() {
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

	wil::com_ptr_nothrow<IDXGISwapChain1> swapChain1;
	if (FAILED(Factory->CreateSwapChainForHwnd(CommandQueue.get(), Hwnd, &desc, nullptr, nullptr, &swapChain1))) {
		Debug::Log("[RenderDX] Failed to create swap chain.\n");
		return false;
	}

	if (FAILED(Factory->MakeWindowAssociation(Hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN))) {
		Debug::Log("[RenderDX] Failed to set window association.\n");
		return false;
	}

	if (FAILED(swapChain1->QueryInterface(IID_PPV_ARGS(&SwapChain)))) {
		Debug::Log("[RenderDX] Failed to query IDXGISwapChain3 interface.\n");
		return false;
	}

	Debug::Log("[RenderDX] Swap chain created successfully.\n");
	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
	return true;
}

bool DXRenderer::CreateRtvHeap() {
	D3D12_DESCRIPTOR_HEAP_DESC desc {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = kFrameCount;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	if (FAILED(Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&RtvHeap)))) {
		Debug::Log("[RenderDX] Failed to create RTV descriptor heap.\n");
		return false;
	}

	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	Debug::Log("[RenderDX] RTV descriptor heap created successfully.\n");
	return true;
}

bool DXRenderer::CreateRenderTargetViews() {
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < kFrameCount; ++i) {
		if (FAILED(SwapChain->GetBuffer(i, IID_PPV_ARGS(&RenderTargets[i])))) {
			Debug::Log("[RenderDX] Failed to get back buffer %u.\n", i);
			return false;
		}
		Device->CreateRenderTargetView(RenderTargets[i].get(), nullptr, rtvHandle);
		rtvHandle.ptr += RtvDescriptorSize;
	}

	Debug::Log("[RenderDX] Render target views created successfully.\n");
	return true;
}

bool DXRenderer::CreateSrvHeap() {
	D3D12_DESCRIPTOR_HEAP_DESC desc {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1; // For surface texture SRV
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;

	if (FAILED(Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&SrvHeap)))) {
		Debug::Log("[RenderDX] Failed to create SRV descriptor heap.\n");
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

	wil::com_ptr_nothrow<ID3DBlob> rootSigBlob;
	wil::com_ptr_nothrow<ID3DBlob> errorBlob;

	HRESULT hr = FP_D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rootSigBlob.put(),
		errorBlob.put()
	);

	if (FAILED(hr)) {
		if (errorBlob)
			Debug::Log("[RenderDX] Root signature serialization error: %s\n", static_cast<const char*>(errorBlob->GetBufferPointer()));
		else
			Debug::Log("[RenderDX] Unknown root signature serialization error.\n");
		return false;
	}

	if (FAILED(Device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)))) {
		Debug::Log("[RenderDX] Failed to create root signature.\n");
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

		// 全屏三角形。
		// 这里不保持宽高比，完整 Surface 会被拉伸填满整个 viewport。
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
	psoDesc.pRootSignature = RootSignature.get();
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

	if (FAILED(Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PipelineState)))) {
		Debug::Log("[RenderDX] Failed to create pipeline state.\n");
		return false;
	}

	Debug::Log("[RenderDX] Pipeline state created successfully.\n");
	return true;
}

bool DXRenderer::CreateCommandObjects() {
	for (UINT i = 0; i < kFrameCount; ++i) {
		if (FAILED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[i])))) {
			Debug::Log("[RenderDX] Failed to create command allocator %u.\n", i);
			return false;
		}
	}

	if (FAILED(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocators[FrameIndex].get(), PipelineState.get(), IID_PPV_ARGS(&CommandList)))) {
		Debug::Log("[RenderDX] Failed to create command list.\n");
		return false;
	}

	Debug::Log("[RenderDX] Command objects created successfully.\n");
	CommandList->Close(); // Command list needs to be closed before reset in the render loop.
	return true;
}

bool DXRenderer::CreateFenceObjects() {
	FenceValues.fill(0);

	if (FAILED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)))) {
		Debug::Log("[RenderDX] Failed to create fence.\n");
		return false;
	}

	FenceValues[FrameIndex] = 1;
	FenceEvent.reset(::CreateEventW(nullptr, FALSE, FALSE, nullptr));

	if (!FenceEvent) {
		Debug::Log("[RenderDX] Failed to create fence event.\n");
		return false;
	}

	Debug::Log("[RenderDX] Fence objects created successfully.\n");
	return true;
}

bool DXRenderer::CreateFixedSurfaceGpuResources() {
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

	if (FAILED(Device->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		SurfaceTextureState,
		nullptr,
		IID_PPV_ARGS(&SurfaceTexture)
	))) {
		Debug::Log("[RenderDX] Failed to create surface texture resource.\n");
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
		if (FAILED(Device->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&uploadDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&SurfaceUploadBuffers[i])
		))) {
			Debug::Log("[RenderDX] Failed to create surface upload buffer %u.\n", i);
			return false;
		}

		D3D12_RANGE readRange {};
		readRange.Begin = 0;
		readRange.End = 0;

		if (FAILED(SurfaceUploadBuffers[i]->Map(0, &readRange, reinterpret_cast<void**>(&SurfaceUploadMapped[i])))) {
			Debug::Log("[RenderDX] Failed to map surface upload buffer %u.\n", i);
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
		SurfaceTexture.get(),
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

		if (DXConfig::PreserveAspectRatio && RenderWidth > 0 && RenderHeight > 0 && WindowWidth > 0 && WindowHeight > 0) {
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
	if (!CommandQueue || !Fence || !FenceEvent) {
		return true; // If we don't have the necessary objects, we can't wait, but also can't report an error. 
	}

	const UINT64 currentFenceValue = FenceValues[FrameIndex];
	if (FAILED(CommandQueue->Signal(Fence.get(), currentFenceValue))) {
		Debug::Log("[RenderDX] Failed to signal command queue for GPU synchronization.\n");
		return false;
	}

	if (FAILED(Fence->SetEventOnCompletion(currentFenceValue, FenceEvent.get()))) {
		Debug::Log("[RenderDX] Failed to set event on fence completion for GPU synchronization.\n");
		return false;
	}

	DWORD waitResult = ::WaitForSingleObjectEx(FenceEvent.get(), INFINITE, FALSE);
	if (waitResult != WAIT_OBJECT_0) {
		Debug::Log("[RenderDX] Wait for GPU synchronization event failed with error code: %lu\n", GetLastError());
		return false;
	}

	++FenceValues[FrameIndex];
	return true;
}

wil::com_ptr_nothrow<ID3DBlob> DXRenderer::LoadCompiledShader(const char* filename)
{
	if (!std::filesystem::exists(filename))
	{
		return nullptr;
	}

	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		return nullptr;
	}

	const std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	wil::com_ptr_nothrow<ID3DBlob> blob;

	if (FAILED(FP_D3DCreateBlob(static_cast<SIZE_T>(size), blob.put())))
	{
		return nullptr;
	}

	if (!file.read(
		static_cast<char*>(blob->GetBufferPointer()),
		size))
	{
		return nullptr;
	}

	Debug::Log("[RenderDX] Shader cache loaded: %s\n", filename);

	return blob;
}

wil::com_ptr_nothrow<ID3DBlob> DXRenderer::CompileTheShader(std::string_view source, std::string_view entryPoint, std::string_view target)
{
	wil::com_ptr_nothrow<ID3DBlob> shaderBlob;
	wil::com_ptr_nothrow<ID3DBlob> errorBlob;

	UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;  // Add optimization

	HRESULT hr = FP_D3DCompile(
		source.data(), source.length(),
		nullptr, nullptr, nullptr,
		entryPoint.data(), target.data(),
		compileFlags, 0,
		&shaderBlob, &errorBlob
	);

	if (FAILED(hr))
	{
		if (errorBlob)
			Debug::Log("[RenderDX] Shader compilation error: %s\n", static_cast<const char*>(errorBlob->GetBufferPointer()));
		else
			Debug::Log("[RenderDX] Unknown shader compilation error.\n");
	}

	return shaderBlob;
}

bool DXRenderer::SaveCompiledShader(const char* filename, ID3DBlob* blob)
{
	if (!blob || !filename)
	{
		return false;
	}

	// Create directory if needed
	std::error_code ec;

	std::filesystem::create_directories(
		std::filesystem::path(filename).parent_path(),
		ec
	);

	if (ec)
	{
		Debug::Log(
			"[RenderDX] Failed creating shader cache directory: %s (%s)\n",
			filename,
			ec.message().c_str()
		);
		return false;
	}

	std::ofstream file(filename, std::ios::binary);

	if (!file.is_open())
	{
		Debug::Log(
			"[RenderDX] Failed opening shader cache file: %s\n",
			filename
		);
		return false;
	}

	file.write(
		static_cast<const char*>(blob->GetBufferPointer()),
		static_cast<std::streamsize>(blob->GetBufferSize())
	);

	// Check if write failed
	if (!file.good())
	{
		Debug::Log(
			"[RenderDX] Failed writing shader cache: %s\n",
			filename
		);
		return false;
	}

	Debug::Log(
		"[RenderDX] Saved compiled shader: %s\n",
		filename
	);

	return true;
}

wil::com_ptr_nothrow<ID3DBlob> DXRenderer::CompileShader(std::string_view source, std::string_view entryPoint, std::string_view target) {
	std::string cacheFile = fmt::format("shader_cache/{}_{}.cso", entryPoint, target);

	if (auto cached = LoadCompiledShader(cacheFile.c_str())) {
		Debug::Log("[RenderDX] Loaded cached shader: %s\n", cacheFile.c_str());
		return cached;
	}

	wil::com_ptr_nothrow<ID3DBlob> compiled = DXRenderer::CompileTheShader(source, entryPoint, target);
	if (compiled) {
		SaveCompiledShader(cacheFile.c_str(), compiled.get());
	}
	return compiled;
}

bool DXRenderer::PopulateCommandListForCPUSurface(const void* pixels, int source_pitch) {
	if (FAILED(CommandAllocators[FrameIndex]->Reset())) {
		Debug::Log("[RenderDX] Failed to reset command allocator for populating command list.\n");
		return false;
	}

	if (FAILED(CommandList->Reset(CommandAllocators[FrameIndex].get(), PipelineState.get()))) {
		Debug::Log("[RenderDX] Failed to reset command list for populating commands.\n");
		return false;
	}

	UploadSurfaceToGpu(pixels, source_pitch);

	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	D3D12_RESOURCE_BARRIER backBufferToRenderTarget {};
	backBufferToRenderTarget.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBufferToRenderTarget.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBufferToRenderTarget.Transition.pResource = RenderTargets[FrameIndex].get();
	backBufferToRenderTarget.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	backBufferToRenderTarget.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	backBufferToRenderTarget.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	CommandList->ResourceBarrier(1, &backBufferToRenderTarget);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += static_cast<SIZE_T>(FrameIndex) * static_cast<SIZE_T>(RtvDescriptorSize);
	CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	CommandList->SetDescriptorHeaps(1, SrvHeap.addressof());
	CommandList->SetGraphicsRootSignature(RootSignature.get());
	CommandList->SetGraphicsRootDescriptorTable(0, SrvHeap->GetGPUDescriptorHandleForHeapStart());
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList->DrawInstanced(3, 1, 0, 0);

	D3D12_RESOURCE_BARRIER backBufferToPresent {};
	backBufferToPresent.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backBufferToPresent.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backBufferToPresent.Transition.pResource = RenderTargets[FrameIndex].get();
	backBufferToPresent.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	backBufferToPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backBufferToPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	CommandList->ResourceBarrier(1, &backBufferToPresent);

	if (FAILED(CommandList->Close())) {
		Debug::Log("[RenderDX] Failed to close command list after populating commands.\n");
		return false;
	}

	return true;
}

void DXRenderer::UploadSurfaceToGpu(const void* pixels, int source_pitch) {
	auto dstBase = SurfaceUploadMapped[FrameIndex];
	const auto* srcBase = static_cast<const std::uint8_t*>(pixels);
	const UINT sourceRowBytes = RenderWidth * static_cast<UINT>(sizeof(std::uint16_t));
	if (static_cast<UINT>(source_pitch) == SurfaceUploadRowPitch) {
		std::memcpy(dstBase, srcBase, sourceRowBytes * RenderHeight);
	} else{
		for (int y = 0; y < RenderHeight; ++y) {
			auto* dstRow = dstBase + static_cast<size_t>(y) * SurfaceUploadRowPitch;
			const auto* srcRow = srcBase + static_cast<size_t>(y) * source_pitch;
			std::memcpy(dstRow, srcRow, sourceRowBytes);
		}
	}

	TransitionSurfaceTexture(SurfaceTextureState, D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12_TEXTURE_COPY_LOCATION dstLocation {};
	dstLocation.pResource = SurfaceTexture.get();
	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstLocation.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION srcLocation {};
	srcLocation.pResource = SurfaceUploadBuffers[FrameIndex].get();
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
	barrier.Transition.pResource = SurfaceTexture.get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;

	CommandList->ResourceBarrier(1, &barrier);

	SurfaceTextureState = after;
}

bool DXRenderer::MoveToNextFrame() {
	const UINT currentFenceValue = FenceValues[FrameIndex];
	if (FAILED(CommandQueue->Signal(Fence.get(), currentFenceValue))) {
		Debug::Log("[RenderDX] Failed to signal command queue for moving to next frame.\n");
		return false;
	}

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
	if (Fence->GetCompletedValue() < FenceValues[FrameIndex]) {
		if (FAILED(Fence->SetEventOnCompletion(FenceValues[FrameIndex], FenceEvent.get()))) {
			Debug::Log("[RenderDX] Failed to set event on fence completion for moving to next frame.\n");
			return false;
		}
		DWORD waitResult = ::WaitForSingleObjectEx(FenceEvent.get(), INFINITE, FALSE);
		if (waitResult != WAIT_OBJECT_0) {
			Debug::Log("[RenderDX] Wait for fence event failed while moving to next frame with error code: %lu\n", GetLastError());
			return false;
		}
	}

	FenceValues[FrameIndex] = currentFenceValue + 1;
	return true;
}
