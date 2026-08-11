/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#include "d3d9_device.hpp"
#include "d3d9_swapchain.hpp"
#include "d3d9_impl_type_convert.hpp"

#include "../dll_log.hpp" // Include late to get 'hr_to_string' helper function
#include "../hook_manager.hpp"

#include <psapi.h>
#include <intrin.h>

// These are defined in d3d9.h, but are used as function names below
#undef IDirect3D9_CreateDevice
#undef IDirect3D9Ex_CreateDeviceEx

static bool is_within_ddraw(const void* address)
{
	const HMODULE ddraw = GetModuleHandleA("DDRAW.DLL");
	if (ddraw == nullptr)
		return false;

	MODULEINFO info = {};
	if (!GetModuleInformation(GetCurrentProcess(), ddraw, &info, sizeof(info)))
		return false;

	const auto addr = static_cast<const BYTE*>(address);
	const auto base = static_cast<const BYTE*>(info.lpBaseOfDll);

	return addr > base && addr < base + info.SizeOfImage;
}


static std::string format_to_string(D3DFORMAT format)
{
	switch (format)
	{
	case D3DFMT_UNKNOWN:
		return "D3DFMT_UNKNOWN";
	case D3DFMT_A8R8G8B8:
		return "D3DFMT_A8R8G8B8";
	case D3DFMT_X8R8G8B8:
		return "D3DFMT_X8R8G8B8";
	case D3DFMT_R5G6B5:
		return "D3DFMT_R5G6B5";
	case D3DFMT_X1R5G5B5:
		return "D3DFMT_X1R5G5B5";
	case D3DFMT_A2R10G10B10:
		return "D3DFMT_A2R10G10B10";
	default:
		char temp_string[11];
		return std::string(temp_string, std::snprintf(temp_string, std::size(temp_string), "%lu", static_cast<DWORD>(format)));
	}
}

void dump_and_modify_present_parameters(D3DPRESENT_PARAMETERS &pp, [[maybe_unused]] IDirect3D9 *d3d, [[maybe_unused]] UINT adapter_index, [[maybe_unused]] HWND focus_window)
{
	reshade::log::message(reshade::log::level::info, "Dumping presentation parameters:");
	reshade::log::message(reshade::log::level::info, "  +-----------------------------------------+-----------------------------------------+");
	reshade::log::message(reshade::log::level::info, "  | Parameter                               | Value                                   |");
	reshade::log::message(reshade::log::level::info, "  +-----------------------------------------+-----------------------------------------+");
	reshade::log::message(reshade::log::level::info, "  | BackBufferWidth                         |"                                " %-39u |", pp.BackBufferWidth);
	reshade::log::message(reshade::log::level::info, "  | BackBufferHeight                        |"                                " %-39u |", pp.BackBufferHeight);
	reshade::log::message(reshade::log::level::info, "  | BackBufferFormat                        |"                                " %-39s |", format_to_string(pp.BackBufferFormat).c_str());
	reshade::log::message(reshade::log::level::info, "  | BackBufferCount                         |"                                " %-39u |", pp.BackBufferCount);
	reshade::log::message(reshade::log::level::info, "  | MultiSampleType                         |"                               " %-39lu |", static_cast<DWORD>(pp.MultiSampleType));
	reshade::log::message(reshade::log::level::info, "  | MultiSampleQuality                      |"                               " %-39lu |", pp.MultiSampleQuality);
	reshade::log::message(reshade::log::level::info, "  | SwapEffect                              |"                               " %-39lu |", static_cast<DWORD>(pp.SwapEffect));
	reshade::log::message(reshade::log::level::info, "  | DeviceWindow                            |"                                " %-39p |", pp.hDeviceWindow);
	reshade::log::message(reshade::log::level::info, "  | Windowed                                |"                                " %-39s |", pp.Windowed != FALSE ? "TRUE" : "FALSE");
	reshade::log::message(reshade::log::level::info, "  | EnableAutoDepthStencil                  |"                                " %-39s |", pp.EnableAutoDepthStencil ? "TRUE" : "FALSE");
	reshade::log::message(reshade::log::level::info, "  | AutoDepthStencilFormat                  |"                               " %-39lu |", static_cast<DWORD>(pp.AutoDepthStencilFormat));
	reshade::log::message(reshade::log::level::info, "  | Flags                                   |"                              " %-#39lx |", pp.Flags);
	reshade::log::message(reshade::log::level::info, "  | FullScreen_RefreshRateInHz              |"                                " %-39u |", pp.FullScreen_RefreshRateInHz);
	reshade::log::message(reshade::log::level::info, "  | PresentationInterval                    |"                               " %-#39x |", pp.PresentationInterval);
	reshade::log::message(reshade::log::level::info, "  +-----------------------------------------+-----------------------------------------+");

}
void dump_and_modify_present_parameters(D3DPRESENT_PARAMETERS &pp, D3DDISPLAYMODEEX &fullscreen_desc, IDirect3D9 *d3d, UINT adapter_index, HWND focus_window)
{
	dump_and_modify_present_parameters(pp, d3d, adapter_index, focus_window);

	assert(fullscreen_desc.Size == sizeof(D3DDISPLAYMODEEX));

	// Update fullscreen display mode in case it was not provided by the application
	if (!pp.Windowed && fullscreen_desc.RefreshRate == 0)
	{
		fullscreen_desc.Width = pp.BackBufferWidth;
		fullscreen_desc.Height = pp.BackBufferHeight;
		fullscreen_desc.RefreshRate = pp.FullScreen_RefreshRateInHz;
		fullscreen_desc.Format = pp.BackBufferFormat;
	}
}

template <typename T>
static void init_device_proxy(T *&device, D3DDEVTYPE device_type, HWND device_window, bool use_software_rendering, bool is_within_ddraw)
{
	// Enable software vertex processing if the application requested a software device
	if (use_software_rendering)
		device->SetSoftwareVertexProcessing(TRUE);

	if (device_type == D3DDEVTYPE_NULLREF)
	{
		reshade::log::message(reshade::log::level::warning, "Skipping device because the device type is 'D3DDEVTYPE_NULLREF'.");
		return;
	}

	// Some video applications create a non-displaying device targeting the desktop window
	if (GetDesktopWindow() == device_window)
	{
		reshade::log::message(reshade::log::level::warning, "Skipping device because the focus window is the desktop window.");
		return;
	}

	IDirect3DSwapChain9 *swapchain = nullptr;
	device->GetSwapChain(0, &swapchain);
	assert(swapchain != nullptr); // There should always be an implicit swap chain

	const auto device_proxy = new Direct3DDevice9(device, use_software_rendering);
	device_proxy->_implicit_swapchain = new Direct3DSwapChain9(device_proxy, swapchain);

	// Overwrite returned device with proxy device
	device = device_proxy;

	// Check if this device was created via D3D9on12 and hook it too if so
	//init_device_proxy_for_d3d9on12(device_proxy);

	// Upgrade to extended interface if available to prevent compatibility issues with some games
	com_ptr<IDirect3DDevice9Ex> deviceex;
	device_proxy->QueryInterface(IID_PPV_ARGS(&deviceex));

#if RESHADE_VERBOSE_LOG
	reshade::log::message(
		reshade::log::level::debug,
		"Returning IDirect3DDevice9%s object %p (%p).",
		device_proxy->_extended_interface ? "Ex" : "", device_proxy, device_proxy->_orig);
#endif
}

// Needs to be set before entering the D3D9 runtime, to avoid hooking internal D3D device creation (e.g. when PIX is attached)
thread_local bool g_in_d3d9_runtime = false;

// Also needs to be set during D3D9 device creation, to avoid hooking internal D3D11 devices created on Windows 10
thread_local bool g_in_dxgi_runtime;

HRESULT STDMETHODCALLTYPE IDirect3D9_CreateDevice(IDirect3D9 *pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface)
{
	auto trampoline = reshade::hooks::call(IDirect3D9_CreateDevice, reshade::hooks::vtable_from_instance(pD3D) + 16);

	// Pass on unmodified in case this called from within the runtime, to avoid hooking internal devices
	if (g_in_d3d9_runtime)
		return trampoline(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	reshade::log::message(
		reshade::log::level::info,
		"Redirecting IDirect3D9::CreateDevice(this = %p, Adapter = %u, DeviceType = %d, hFocusWindow = %p, BehaviorFlags = %#x, pPresentationParameters = %p, ppReturnedDeviceInterface = %p) ...",
		pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if (pPresentationParameters == nullptr)
		return D3DERR_INVALIDCALL;

	if ((BehaviorFlags & D3DCREATE_ADAPTERGROUP_DEVICE) != 0)
	{
		reshade::log::message(reshade::log::level::warning, "Adapter group devices are unsupported.");
		return D3DERR_NOTAVAILABLE;
	}

	D3DPRESENT_PARAMETERS pp = *pPresentationParameters;
	dump_and_modify_present_parameters(pp, pD3D, Adapter, hFocusWindow);

	const bool use_software_rendering = (BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0;
	if (use_software_rendering)
	{
		reshade::log::message(reshade::log::level::info, "> Replacing 'D3DCREATE_SOFTWARE_VERTEXPROCESSING' flag with 'D3DCREATE_MIXED_VERTEXPROCESSING' to allow for hardware rendering.");

		BehaviorFlags = (BehaviorFlags & ~D3DCREATE_SOFTWARE_VERTEXPROCESSING) | D3DCREATE_MIXED_VERTEXPROCESSING;
	}

	assert(!g_in_dxgi_runtime);
	g_in_d3d9_runtime = g_in_dxgi_runtime = true;
	const HRESULT hr = trampoline(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, &pp, ppReturnedDeviceInterface);
	g_in_d3d9_runtime = g_in_dxgi_runtime = false;

	// Update output values (see https://docs.microsoft.com/windows/win32/api/d3d9/nf-d3d9-idirect3d9-createdevice)
	pPresentationParameters->BackBufferWidth = pp.BackBufferWidth;
	pPresentationParameters->BackBufferHeight = pp.BackBufferHeight;
	pPresentationParameters->BackBufferFormat = pp.BackBufferFormat;
	pPresentationParameters->BackBufferCount = pp.BackBufferCount;

	const bool called_from_ddraw = is_within_ddraw(_ReturnAddress());

	if (SUCCEEDED(hr))
	{
		init_device_proxy(*ppReturnedDeviceInterface, DeviceType, (pp.hDeviceWindow != nullptr) ? pp.hDeviceWindow : hFocusWindow, use_software_rendering, called_from_ddraw);
	}
	else
	{
		reshade::log::message(reshade::log::level::warning, "IDirect3D9::CreateDevice failed with error code %s.", reshade::log::hr_to_string(hr).c_str());
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE IDirect3D9Ex_CreateDeviceEx(IDirect3D9Ex *pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode, IDirect3DDevice9Ex **ppReturnedDeviceInterface)
{
	const auto trampoline = reshade::hooks::call(IDirect3D9Ex_CreateDeviceEx, reshade::hooks::vtable_from_instance(pD3D) + 20);

	if (g_in_d3d9_runtime)
		return trampoline(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);

	reshade::log::message(
		reshade::log::level::info,
		"Redirecting IDirect3D9Ex::CreateDeviceEx(this = %p, Adapter = %u, DeviceType = %d, hFocusWindow = %p, BehaviorFlags = %#x, pPresentationParameters = %p, pFullscreenDisplayMode = %p, ppReturnedDeviceInterface = %p) ...",
		pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, pFullscreenDisplayMode, ppReturnedDeviceInterface);

	if (pPresentationParameters == nullptr)
		return D3DERR_INVALIDCALL;

	if ((BehaviorFlags & D3DCREATE_ADAPTERGROUP_DEVICE) != 0)
	{
		reshade::log::message(reshade::log::level::warning, "Adapter group devices are unsupported.");
		return D3DERR_NOTAVAILABLE;
	}

	D3DDISPLAYMODEEX fullscreen_mode = { sizeof(fullscreen_mode) };
	if (pFullscreenDisplayMode != nullptr)
		fullscreen_mode = *pFullscreenDisplayMode;
	D3DPRESENT_PARAMETERS pp = *pPresentationParameters;
	dump_and_modify_present_parameters(pp, fullscreen_mode, pD3D, Adapter, hFocusWindow);

	const bool use_software_rendering = (BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0;
	if (use_software_rendering)
	{
		reshade::log::message(reshade::log::level::info, "> Replacing 'D3DCREATE_SOFTWARE_VERTEXPROCESSING' flag with 'D3DCREATE_MIXED_VERTEXPROCESSING' to allow for hardware rendering.");

		BehaviorFlags = (BehaviorFlags & ~D3DCREATE_SOFTWARE_VERTEXPROCESSING) | D3DCREATE_MIXED_VERTEXPROCESSING;
	}

	assert(!g_in_dxgi_runtime);
	g_in_d3d9_runtime = g_in_dxgi_runtime = true;
	const HRESULT hr = trampoline(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, &pp, pp.Windowed ? nullptr : &fullscreen_mode, ppReturnedDeviceInterface);
	g_in_d3d9_runtime = g_in_dxgi_runtime = false;

	// Update output values (see https://docs.microsoft.com/windows/win32/api/d3d9/nf-d3d9-idirect3d9ex-createdeviceex)
	pPresentationParameters->BackBufferWidth = pp.BackBufferWidth;
	pPresentationParameters->BackBufferHeight = pp.BackBufferHeight;
	pPresentationParameters->BackBufferFormat = pp.BackBufferFormat;
	pPresentationParameters->BackBufferCount = pp.BackBufferCount;

	const bool called_from_ddraw = is_within_ddraw(_ReturnAddress());

	if (SUCCEEDED(hr))
	{
		init_device_proxy(*ppReturnedDeviceInterface, DeviceType, (pp.hDeviceWindow != nullptr) ? pp.hDeviceWindow : hFocusWindow, use_software_rendering, called_from_ddraw);
	}
	else
	{
		reshade::log::message(reshade::log::level::warning, "IDirect3D9Ex::CreateDeviceEx failed with error code %s.", reshade::log::hr_to_string(hr).c_str());
	}

	return hr;
}

extern "C" IDirect3D9 *WINAPI Direct3DCreate9(UINT SDKVersion)
{
	const auto trampoline = reshade::hooks::call(Direct3DCreate9);

	if (g_in_d3d9_runtime)
		return trampoline(SDKVersion);

	reshade::log::message(reshade::log::level::info, "Redirecting Direct3DCreate9(SDKVersion = %#x) ...", SDKVersion);

	assert(!g_in_dxgi_runtime);
	g_in_d3d9_runtime = g_in_dxgi_runtime = true;
	IDirect3D9 *const res = trampoline(SDKVersion);
	g_in_d3d9_runtime = g_in_dxgi_runtime = false;
	if (res == nullptr)
	{
		reshade::log::message(reshade::log::level::warning, "Direct3DCreate9 failed.");
		return nullptr;
	}

	reshade::hooks::install("IDirect3D9::CreateDevice", reshade::hooks::vtable_from_instance(res), 16, &IDirect3D9_CreateDevice);

#if RESHADE_VERBOSE_LOG
	reshade::log::message(reshade::log::level::debug, "Returning IDirect3D9 object %p.", res);
#endif
	return res;
}

extern "C"     HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex **ppD3D)
{
	const auto trampoline = reshade::hooks::call(Direct3DCreate9Ex);

	if (g_in_d3d9_runtime)
		return trampoline(SDKVersion, ppD3D);

	reshade::log::message(reshade::log::level::info, "Redirecting Direct3DCreate9Ex(SDKVersion = %#x, ppD3D = %p) ...", SDKVersion, ppD3D);

	assert(!g_in_dxgi_runtime);
	g_in_d3d9_runtime = g_in_dxgi_runtime = true;
	const HRESULT hr = trampoline(SDKVersion, ppD3D);
	g_in_d3d9_runtime = g_in_dxgi_runtime = false;
	if (FAILED(hr))
	{
		reshade::log::message(reshade::log::level::warning, "Direct3DCreate9Ex failed with error code %s.", reshade::log::hr_to_string(hr).c_str());
		return hr;
	}

	assert(ppD3D != nullptr);

	reshade::hooks::install("IDirect3D9Ex::CreateDevice", reshade::hooks::vtable_from_instance(*ppD3D), 16, &IDirect3D9_CreateDevice);
	reshade::hooks::install("IDirect3D9Ex::CreateDeviceEx", reshade::hooks::vtable_from_instance(*ppD3D), 20, &IDirect3D9Ex_CreateDeviceEx);

#if RESHADE_VERBOSE_LOG
	reshade::log::message(reshade::log::level::debug, "Returning IDirect3D9Ex object %p.", *ppD3D);
#endif
	return hr;
}
