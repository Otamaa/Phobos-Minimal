/*
 * Copyright (C) 2022 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "d3d12_device.hpp"
#include "d3d12_resource.hpp"
#include "../../Utils/com_utils.hpp"

#include "../../Hooks/hook_manager.hpp"

extern std::shared_mutex g_d3d12_adapter_mutex;

// Monster Hunter Rise calls 'ID3D12Device::CopyDescriptorsSimple' on a device queried from a resource
// This crashes if that device pointer is not pointing to the proxy device, due to our modified descriptor handles, so need to make sure that is the case
HRESULT STDMETHODCALLTYPE ID3D12Resource_GetDevice(ID3D12Resource *pResource, REFIID riid, void **ppvDevice)
{
	const HRESULT hr = reshade::hooks::call(ID3D12Resource_GetDevice, reshade::hooks::vtable_from_instance(pResource) + 7)(pResource, riid, ppvDevice);
	if (FAILED(hr))
		return hr;

	const auto device = static_cast<ID3D12Device *>(*ppvDevice);
	assert(device != nullptr);

	const std::unique_lock<std::shared_mutex> lock(g_d3d12_adapter_mutex);

	const auto device_proxy = get_private_pointer_d3dx<D3D12Device>(device);
	if (device_proxy != nullptr && device_proxy->_orig == device)
	{
		InterlockedIncrement(&device_proxy->_ref);
		*ppvDevice = device_proxy;
	}

	return hr;
}