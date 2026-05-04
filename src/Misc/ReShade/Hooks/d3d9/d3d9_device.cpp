/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */
#include <Utilities/Debug.h>

#include "d3d9_device.hpp"
#include "d3d9_swapchain.hpp"
#include "d3d9on12_device.hpp"
#include "d3d9_impl_type_convert.hpp"

#include "../../Utils/com_utils.hpp"
#include "../hook_manager.hpp"

using reshade::d3d9::to_handle;

extern thread_local bool g_in_d3d9_runtime;
extern thread_local bool g_in_dxgi_runtime;

extern void dump_and_modify_present_parameters(D3DPRESENT_PARAMETERS &pp, IDirect3D9 *d3d, UINT adapter_index, HWND focus_window);
extern void dump_and_modify_present_parameters(D3DPRESENT_PARAMETERS &pp, D3DDISPLAYMODEEX &fullscreen_desc, IDirect3D9 *d3d, UINT adapter_index, HWND focus_window);

const reshade::api::subresource_box *convert_rect_to_box(const RECT *rect, reshade::api::subresource_box &box)
{
	if (rect == nullptr)
		return nullptr;

	assert(rect->left >= 0 && rect->top >= 0 && rect->right >= 0 && rect->bottom >= 0);

	box.left = static_cast<uint32_t>(rect->left);
	box.top = static_cast<uint32_t>(rect->top);
	box.front = 0;
	box.right = static_cast<uint32_t>(rect->right);
	box.bottom = static_cast<uint32_t>(rect->bottom);
	box.back = 1;

	return &box;
}
const reshade::api::subresource_box *convert_rect_to_box(const POINT *point, LONG width, LONG height, reshade::api::subresource_box &box)
{
	if (point == nullptr)
		return nullptr;

	assert(point->x >= 0 && point->y >= 0 && width >= 0 && height >= 0);

	box.left = static_cast<uint32_t>(point->x);
	box.top = static_cast<uint32_t>(point->y);
	box.front = 0;
	box.right = static_cast<uint32_t>(point->x + width);
	box.bottom = static_cast<uint32_t>(point->y + height);
	box.back = 1;

	return &box;
}

Direct3DDevice9::Direct3DDevice9(IDirect3DDevice9   *original, bool use_software_rendering) :
	device_impl(original),
	_use_software_rendering(use_software_rendering)
{
	assert(_orig != nullptr);

	device_impl::on_init();

	// Delay initialization of auto depth-stencil until after implicit swap chain proxy was created (see 'Direct3DSwapChain9::Direct3DSwapChain9')
}
Direct3DDevice9::Direct3DDevice9(IDirect3DDevice9Ex *original, bool use_software_rendering) :
	Direct3DDevice9(static_cast<IDirect3DDevice9 *>(original), use_software_rendering)
{
	_extended_interface = true;
}
Direct3DDevice9::~Direct3DDevice9()
{
	on_reset();
}

bool Direct3DDevice9::check_and_upgrade_interface(REFIID riid)
{
	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) ||
		riid == __uuidof(IDirect3DDevice9))   // {D0223B96-BF7A-43fd-92BD-A43B0D82B9EB}
		return true;
	if (riid != __uuidof(IDirect3DDevice9Ex)) // {B18B10CE-2649-405a-870F-95F777D4313A}
		return false;

	if (!_extended_interface)
	{
		IDirect3DDevice9Ex *new_interface = nullptr;
		if (FAILED(_orig->QueryInterface(IID_PPV_ARGS(&new_interface))))
			return false;
		_orig->Release();
		_orig = new_interface;
		_extended_interface = true;
	}

	return true;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice9::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
		return E_POINTER;

	if (check_and_upgrade_interface(riid))
	{
		AddRef();
		*ppvObj = this;
		return S_OK;
	}

	// Interface ID to query the original object from a proxy object
	if (riid == IID_UnwrappedObject)
	{
		_orig->AddRef();
		*ppvObj = _orig;
		return S_OK;
	}

	if (riid == __uuidof(IDirect3DDevice9On12))
	{
		if (_d3d9on12_device != nullptr)
			return _d3d9on12_device->QueryInterface(riid, ppvObj);
	}

	// Unimplemented interfaces:
	//   IDirect3DDevice9Video {26DC4561-A1EE-4ae7-96DA-118A36C0EC95}

	return _orig->QueryInterface(riid, ppvObj);
}
ULONG   STDMETHODCALLTYPE Direct3DDevice9::AddRef()
{
	_orig->AddRef();
	return InterlockedIncrement(&_ref);
}
ULONG   STDMETHODCALLTYPE Direct3DDevice9::Release()
{
	const ULONG ref = InterlockedDecrement(&_ref);
	if (ref != 0)
	{
		_orig->Release();
		return ref;
	}

	const auto orig = _orig;
	const bool extended_interface = _extended_interface;

	// Borderlands 2 is not counting references correctly and will release the device before 'IDirect3DDevice9::Reset' calls, so try and detect this and prevent deletion
	if (_resource_ref > 5)
	{
		Debug::Log("Reference count for IDirect3DDevice9%s object %p (%p) is inconsistent! Leaking resources ...", extended_interface ? "Ex" : "", this, orig);
		_ref = 1;
		// Always return zero in case a game is trying to release all references in a while loop, which would otherwise run indefinitely
		return 0;
	}

	if (_d3d9on12_device != nullptr)
	{
		// Release the reference that was added when the D3D9on12 device was first queried in 'init_device_proxy_for_d3d9on12' (see d3d9on12.cpp)
		_d3d9on12_device->_orig->Release();
		delete _d3d9on12_device;
	}

	// Release remaining references to this device
	_implicit_swapchain->Release();

	assert(_additional_swapchains.empty());

	// Only call destructor and do not yet free memory before calling final 'Release' below
	// Some resources may still be alive here (e.g. because of a state block from the Steam overlay, which is released on device destruction), which will then call the resource destruction callbacks during the final 'Release' and still access this memory
	this->~Direct3DDevice9();

	const ULONG ref_orig = orig->Release();
	if (ref_orig != 0) // Verify internal reference count
		Debug::Log("Reference count for IDirect3DDevice9%s object %p (%p) is inconsistent (%lu).", extended_interface ? "Ex" : "", this, orig, ref_orig);
	else
		operator delete(this, sizeof(Direct3DDevice9));
	return 0;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice9::TestCooperativeLevel()
{
	return _orig->TestCooperativeLevel();
}
UINT    STDMETHODCALLTYPE Direct3DDevice9::GetAvailableTextureMem()
{
	return _orig->GetAvailableTextureMem();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::EvictManagedResources()
{
	return _orig->EvictManagedResources();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetDirect3D(IDirect3D9 **ppD3D9)
{
	return _orig->GetDirect3D(ppD3D9);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetDeviceCaps(D3DCAPS9 *pCaps)
{
	return _orig->GetDeviceCaps(pCaps);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE *pMode)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return D3DERR_INVALIDCALL;
	}

	return _implicit_swapchain->GetDisplayMode(pMode);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return _orig->GetCreationParameters(pParameters);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 *pCursorBitmap)
{
	return _orig->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}
void    STDMETHODCALLTYPE Direct3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
	return _orig->SetCursorPosition(X, Y, Flags);
}
BOOL    STDMETHODCALLTYPE Direct3DDevice9::ShowCursor(BOOL bShow)
{
	return _orig->ShowCursor(bShow);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	Debug::Log(
		
		"Redirecting IDirect3DDevice9::CreateAdditionalSwapChain(this = %p, pPresentationParameters = %p, ppSwapChain = %p) ...",
		this, pPresentationParameters, ppSwapChain);

	if (pPresentationParameters == nullptr)
		return D3DERR_INVALIDCALL;

	D3DPRESENT_PARAMETERS pp = *pPresentationParameters;
	dump_and_modify_present_parameters(pp, _d3d.get(), _cp.AdapterOrdinal, _cp.hFocusWindow);

	const HRESULT hr = _orig->CreateAdditionalSwapChain(&pp, ppSwapChain);

	// Update output values (see https://docs.microsoft.com/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-createadditionalswapchain)
	pPresentationParameters->BackBufferWidth = pp.BackBufferWidth;
	pPresentationParameters->BackBufferHeight = pp.BackBufferHeight;
	pPresentationParameters->BackBufferFormat = pp.BackBufferFormat;
	pPresentationParameters->BackBufferCount = pp.BackBufferCount;

	if (SUCCEEDED(hr))
	{
		AddRef(); // Add reference which is released when the swap chain is destroyed (see 'Direct3DSwapChain9::Release')

		const auto swapchain_proxy = new Direct3DSwapChain9(this, *ppSwapChain);
		_additional_swapchains.push_back(swapchain_proxy);
		*ppSwapChain = swapchain_proxy;
	}
	else
	{
		Debug::Log("IDirect3DDevice9::CreateAdditionalSwapChain failed with error code %s.", Debug::hr_to_string(hr).c_str());
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9 **ppSwapChain)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return D3DERR_INVALIDCALL;
	}

	if (ppSwapChain == nullptr)
		return D3DERR_INVALIDCALL;

	_implicit_swapchain->AddRef();
	*ppSwapChain = _implicit_swapchain;

	return D3D_OK;
}
UINT    STDMETHODCALLTYPE Direct3DDevice9::GetNumberOfSwapChains()
{
	return 1; // Multi-head swap chains are not supported
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	Debug::Log(
		
		"Redirecting IDirect3DDevice9::Reset(this = %p, pPresentationParameters = %p) ...",
		this, pPresentationParameters);

	if (pPresentationParameters == nullptr)
		return D3DERR_INVALIDCALL;

	D3DPRESENT_PARAMETERS pp = *pPresentationParameters;
	dump_and_modify_present_parameters(pp, _d3d.get(), _cp.AdapterOrdinal, _cp.hFocusWindow);

	// Release all resources before performing reset
	_implicit_swapchain->on_reset(true);
	on_reset();

	assert(!g_in_d3d9_runtime && !g_in_dxgi_runtime);
	g_in_d3d9_runtime = g_in_dxgi_runtime = true;
	const HRESULT hr = _orig->Reset(&pp);
	g_in_d3d9_runtime = g_in_dxgi_runtime = false;

	// Update output values (see https://docs.microsoft.com/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-reset)
	pPresentationParameters->BackBufferWidth = pp.BackBufferWidth;
	pPresentationParameters->BackBufferHeight = pp.BackBufferHeight;
	pPresentationParameters->BackBufferFormat = pp.BackBufferFormat;
	pPresentationParameters->BackBufferCount = pp.BackBufferCount;

	if (SUCCEEDED(hr))
	{
		on_init();
		_implicit_swapchain->on_init(true);
	}
	else
	{
		Debug::Log("IDirect3DDevice9::Reset failed with error code %s!", Debug::hr_to_string(hr).c_str());

		// Initialize device implementation even when reset failed, so that 'init_command_list' and 'init_command_queue' events are still called
		on_init();
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	_implicit_swapchain->on_present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, 0);

	const HRESULT hr = _orig->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	_implicit_swapchain->on_finish_present(hr);

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return D3DERR_INVALIDCALL;
	}

	return _implicit_swapchain->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS *pRasterStatus)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return D3DERR_INVALIDCALL;
	}

	return _implicit_swapchain->GetRasterStatus(pRasterStatus);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
	return _orig->SetDialogBoxMode(bEnableDialogs);
}
void    STDMETHODCALLTYPE Direct3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP *pRamp)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return;
	}

	return _orig->SetGammaRamp(0, Flags, pRamp);
}
void    STDMETHODCALLTYPE Direct3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP *pRamp)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return;
	}

	return _orig->GetGammaRamp(0, pRamp);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9 **ppTexture, HANDLE *pSharedHandle)
{
	return _orig->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9 **ppVolumeTexture, HANDLE *pSharedHandle)
{
	const HRESULT hr = _orig->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9 **ppCubeTexture, HANDLE *pSharedHandle)
{
	const HRESULT hr = _orig->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9 **ppVertexBuffer, HANDLE *pSharedHandle)
{
	// Need to allow buffer for use in software vertex processing, since application uses software and not hardware processing, but device was created with both
	if (_use_software_rendering)
		Usage |= D3DUSAGE_SOFTWAREPROCESSING;

	const HRESULT hr = _orig->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9 **ppIndexBuffer, HANDLE *pSharedHandle)
{
	if (_use_software_rendering)
		Usage |= D3DUSAGE_SOFTWAREPROCESSING;

	const HRESULT hr = _orig->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle)
{
	const HRESULT hr = _orig->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle)
{
	const HRESULT hr = _orig->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::UpdateSurface(IDirect3DSurface9 *pSrcSurface, const RECT *pSrcRect, IDirect3DSurface9 *pDstSurface, const POINT *pDstPoint)
{
	return _orig->UpdateSurface(pSrcSurface, pSrcRect, pDstSurface, pDstPoint);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::UpdateTexture(IDirect3DBaseTexture9 *pSrcTexture, IDirect3DBaseTexture9 *pDstTexture)
{
	return _orig->UpdateTexture(pSrcTexture, pDstTexture);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetRenderTargetData(IDirect3DSurface9 *pSrcSurface, IDirect3DSurface9 *pDstSurface)
{
	return _orig->GetRenderTargetData(pSrcSurface, pDstSurface);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9 *pDestSurface)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return D3DERR_INVALIDCALL;
	}

	return _implicit_swapchain->GetFrontBufferData(pDestSurface);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::StretchRect(IDirect3DSurface9 *pSrcSurface, const RECT *pSrcRect, IDirect3DSurface9 *pDstSurface, const RECT *pDstRect, D3DTEXTUREFILTERTYPE Filter)
{
	return _orig->StretchRect(pSrcSurface, pSrcRect, pDstSurface, pDstRect, Filter);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::ColorFill(IDirect3DSurface9 *pSurface, const RECT *pRect, D3DCOLOR Color)
{
	return _orig->ColorFill(pSurface, pRect, Color);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle)
{
	const HRESULT hr = _orig->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget)
{
	const HRESULT hr = _orig->SetRenderTarget(RenderTargetIndex, pRenderTarget);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 **ppRenderTarget)
{
	return _orig->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetDepthStencilSurface(IDirect3DSurface9 *pNewZStencil)
{
	const HRESULT hr = _orig->SetDepthStencilSurface(pNewZStencil);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
	const HRESULT hr = _orig->GetDepthStencilSurface(ppZStencilSurface);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::BeginScene()
{
	return _orig->BeginScene();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::EndScene()
{	return _orig->EndScene();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::Clear(DWORD Count, const D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return _orig->Clear(Count, pRects, Flags, Color, Z, Stencil);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix)
{
	return _orig->SetTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return _orig->GetTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix)
{
	return _orig->MultiplyTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetViewport(const D3DVIEWPORT9 *pViewport)
{
	const HRESULT hr = _orig->SetViewport(pViewport);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetViewport(D3DVIEWPORT9 *pViewport)
{
	return _orig->GetViewport(pViewport);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetMaterial(const D3DMATERIAL9 *pMaterial)
{
	return _orig->SetMaterial(pMaterial);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetMaterial(D3DMATERIAL9 *pMaterial)
{
	return _orig->GetMaterial(pMaterial);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetLight(DWORD Index, const D3DLIGHT9 *pLight)
{
	return _orig->SetLight(Index, pLight);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetLight(DWORD Index, D3DLIGHT9 *pLight)
{
	return _orig->GetLight(Index, pLight);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::LightEnable(DWORD Index, BOOL Enable)
{
	return _orig->LightEnable(Index, Enable);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return _orig->GetLightEnable(Index, pEnable);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetClipPlane(DWORD Index, const float *pPlane)
{
	return _orig->SetClipPlane(Index, pPlane);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetClipPlane(DWORD Index, float *pPlane)
{
	return _orig->GetClipPlane(Index, pPlane);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	const HRESULT hr = _orig->SetRenderState(State, Value);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	return _orig->GetRenderState(State, pValue);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9 **ppSB)
{
	return _orig->CreateStateBlock(Type, ppSB);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::BeginStateBlock()
{
	return _orig->BeginStateBlock();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::EndStateBlock(IDirect3DStateBlock9 **ppSB)
{
	return _orig->EndStateBlock(ppSB);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetClipStatus(const D3DCLIPSTATUS9 *pClipStatus)
{
	return _orig->SetClipStatus(pClipStatus);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus)
{
	return _orig->GetClipStatus(pClipStatus);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9 **ppTexture)
{
	return _orig->GetTexture(Stage, ppTexture);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
	const HRESULT hr = _orig->SetTexture(Stage, pTexture);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	return _orig->GetTextureStageState(Stage, Type, pValue);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return _orig->SetTextureStageState(Stage, Type, Value);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD *pValue)
{
	return _orig->GetSamplerState(Sampler, Type, pValue);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return _orig->SetSamplerState(Sampler, Type, Value);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::ValidateDevice(DWORD *pNumPasses)
{
	return _orig->ValidateDevice(pNumPasses);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY *pEntries)
{
	return _orig->SetPaletteEntries(PaletteNumber, pEntries);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	return _orig->GetPaletteEntries(PaletteNumber, pEntries);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return _orig->SetCurrentTexturePalette(PaletteNumber);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	return _orig->GetCurrentTexturePalette(PaletteNumber);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetScissorRect(const RECT *pRect)
{
	const HRESULT hr = _orig->SetScissorRect(pRect);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetScissorRect(RECT *pRect)
{
	return _orig->GetScissorRect(pRect);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return _orig->SetSoftwareVertexProcessing(bSoftware);
}
BOOL    STDMETHODCALLTYPE Direct3DDevice9::GetSoftwareVertexProcessing()
{
	return _orig->GetSoftwareVertexProcessing();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetNPatchMode(float nSegments)
{
	return _orig->SetNPatchMode(nSegments);
}
float   STDMETHODCALLTYPE Direct3DDevice9::GetNPatchMode()
{
	return _orig->GetNPatchMode();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return _orig->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
{
	return _orig->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	const HRESULT hr = _orig->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void *pIndexData, D3DFORMAT IndexDataFormat, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	const HRESULT hr = _orig->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9 *pDestBuffer, IDirect3DVertexDeclaration9 *pVertexDecl, DWORD Flags)
{
	return _orig->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateVertexDeclaration(const D3DVERTEXELEMENT9 *pVertexElements, IDirect3DVertexDeclaration9 **ppDecl)
{
	const HRESULT hr = _orig->CreateVertexDeclaration(pVertexElements, ppDecl);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl)
{
	const HRESULT hr = _orig->SetVertexDeclaration(pDecl);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9 **ppDecl)
{
	return _orig->GetVertexDeclaration(ppDecl);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetFVF(DWORD FVF)
{
	const HRESULT hr = _orig->SetFVF(FVF);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetFVF(DWORD *pFVF)
{
	return _orig->GetFVF(pFVF);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateVertexShader(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader)
{
	const HRESULT hr = _orig->CreateVertexShader(pFunction, ppShader);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetVertexShader(IDirect3DVertexShader9 *pShader)
{
	const HRESULT hr = _orig->SetVertexShader(pShader);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetVertexShader(IDirect3DVertexShader9 **ppShader)
{
	return _orig->GetVertexShader(ppShader);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetVertexShaderConstantF(UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	const HRESULT hr = _orig->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float *pConstantData, UINT Vector4fCount)
{
	return _orig->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetVertexShaderConstantI(UINT StartRegister, const int *pConstantData, UINT Vector4iCount)
{
	const HRESULT hr = _orig->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int *pConstantData, UINT Vector4iCount)
{
	return _orig->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetVertexShaderConstantB(UINT StartRegister, const BOOL *pConstantData, UINT BoolCount)
{
	const HRESULT hr = _orig->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL *pConstantData, UINT BoolCount)
{
	return _orig->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride)
{
	const HRESULT hr = _orig->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 **ppStreamData, UINT *OffsetInBytes, UINT *pStride)
{
	return _orig->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{
	return _orig->SetStreamSourceFreq(StreamNumber, Divider);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT *Divider)
{
	return _orig->GetStreamSourceFreq(StreamNumber, Divider);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetIndices(IDirect3DIndexBuffer9 *pIndexData)
{
	const HRESULT hr = _orig->SetIndices(pIndexData);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetIndices(IDirect3DIndexBuffer9 **ppIndexData)
{
	return _orig->GetIndices(ppIndexData);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader)
{
	const HRESULT hr = _orig->CreatePixelShader(pFunction, ppShader);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetPixelShader(IDirect3DPixelShader9 *pShader)
{
	const HRESULT hr = _orig->SetPixelShader(pShader);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetPixelShader(IDirect3DPixelShader9 **ppShader)
{
	return _orig->GetPixelShader(ppShader);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetPixelShaderConstantF(UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	const HRESULT hr = _orig->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float *pConstantData, UINT Vector4fCount)
{
	return _orig->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetPixelShaderConstantI(UINT StartRegister, const int *pConstantData, UINT Vector4iCount)
{
	const HRESULT hr = _orig->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int *pConstantData, UINT Vector4iCount)
{
	return _orig->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetPixelShaderConstantB(UINT StartRegister, const BOOL *pConstantData, UINT BoolCount)
{
	const HRESULT hr = _orig->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL *pConstantData, UINT BoolCount)
{
	return _orig->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::DrawRectPatch(UINT Handle, const float *pNumSegs, const D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return _orig->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::DrawTriPatch(UINT Handle, const float *pNumSegs, const D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return _orig->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::DeletePatch(UINT Handle)
{
	return _orig->DeletePatch(Handle);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9 **ppQuery)
{
	return _orig->CreateQuery(Type, ppQuery);
}

HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetConvolutionMonoKernel(UINT width, UINT height, float *rows, float *columns)
{
	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->SetConvolutionMonoKernel(width, height, rows, columns);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::ComposeRects(IDirect3DSurface9 *pSrc, IDirect3DSurface9 *pDst, IDirect3DVertexBuffer9 *pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9 *pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset)
{
	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::PresentEx(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion, DWORD dwFlags)
{
	assert(_extended_interface);

	_implicit_swapchain->on_present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

	const HRESULT hr = static_cast<IDirect3DDevice9Ex *>(_orig)->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);

	_implicit_swapchain->on_finish_present(hr);

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetGPUThreadPriority(INT *pPriority)
{
	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->GetGPUThreadPriority(pPriority);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetGPUThreadPriority(INT Priority)
{
	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->SetGPUThreadPriority(Priority);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::WaitForVBlank(UINT iSwapChain)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return D3DERR_INVALIDCALL;
	}

	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->WaitForVBlank(0);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CheckResourceResidency(IDirect3DResource9 **pResourceArray, UINT32 NumResources)
{
	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->CheckResourceResidency(pResourceArray, NumResources);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::SetMaximumFrameLatency(UINT MaxLatency)
{
	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->SetMaximumFrameLatency(MaxLatency);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetMaximumFrameLatency(UINT *pMaxLatency)
{
	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->GetMaximumFrameLatency(pMaxLatency);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CheckDeviceState(HWND hDestinationWindow)
{
	assert(_extended_interface);
	return static_cast<IDirect3DDevice9Ex *>(_orig)->CheckDeviceState(hDestinationWindow);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle, DWORD Usage)
{
	assert(_extended_interface);

	const HRESULT hr = static_cast<IDirect3DDevice9Ex *>(_orig)->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle, DWORD Usage)
{
	assert(_extended_interface);

	const HRESULT hr = static_cast<IDirect3DDevice9Ex *>(_orig)->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle, DWORD Usage)
{
	assert(_extended_interface);

	const HRESULT hr = static_cast<IDirect3DDevice9Ex *>(_orig)->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::ResetEx(D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode)
{
	assert(_extended_interface);

	Debug::Log("Redirecting IDirect3DDevice9Ex::ResetEx(this = %p, pPresentationParameters = %p, pFullscreenDisplayMode = %p) ...",
		this, pPresentationParameters, pFullscreenDisplayMode);

	if (pPresentationParameters == nullptr)
		return D3DERR_INVALIDCALL;

	D3DDISPLAYMODEEX fullscreen_mode = { sizeof(fullscreen_mode) };
	if (pFullscreenDisplayMode != nullptr)
		fullscreen_mode = *pFullscreenDisplayMode;
	D3DPRESENT_PARAMETERS pp = *pPresentationParameters;
	dump_and_modify_present_parameters(pp, fullscreen_mode, _d3d.get(), _cp.AdapterOrdinal, _cp.hFocusWindow);

	// Release all resources before performing reset
	_implicit_swapchain->on_reset(true);
	on_reset();

	assert(!g_in_d3d9_runtime && !g_in_dxgi_runtime);
	g_in_d3d9_runtime = g_in_dxgi_runtime = true;
	const HRESULT hr = static_cast<IDirect3DDevice9Ex *>(_orig)->ResetEx(&pp, pp.Windowed ? nullptr : &fullscreen_mode);
	g_in_d3d9_runtime = g_in_dxgi_runtime = false;

	// Update output values (see https://docs.microsoft.com/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9ex-resetex)
	pPresentationParameters->BackBufferWidth = pp.BackBufferWidth;
	pPresentationParameters->BackBufferHeight = pp.BackBufferHeight;
	pPresentationParameters->BackBufferFormat = pp.BackBufferFormat;
	pPresentationParameters->BackBufferCount = pp.BackBufferCount;

	if (SUCCEEDED(hr))
	{
		on_init();
		_implicit_swapchain->on_init(true);
	}
	else
	{
		Debug::Log("IDirect3DDevice9Ex::ResetEx failed with error code %s!", Debug::hr_to_string(hr).c_str());

		// Initialize device implementation even when reset failed, so that 'init_command_list' and 'init_command_queue' events are still called
		on_init();
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice9::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX *pMode, D3DDISPLAYROTATION *pRotation)
{
	if (iSwapChain != 0)
	{
		Debug::Log("Access to multi-head swap chain at index %u is unsupported.", iSwapChain);
		return D3DERR_INVALIDCALL;
	}

	assert(_extended_interface);
	assert(_implicit_swapchain->_extended_interface);
	return static_cast<IDirect3DSwapChain9Ex *>(_implicit_swapchain)->GetDisplayModeEx(pMode, pRotation);
}