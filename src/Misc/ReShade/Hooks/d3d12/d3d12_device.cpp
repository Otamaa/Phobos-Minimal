/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */
#include <Utilities/Debug.h>
#include "d3d12_device.hpp"
#include "d3d12_device_downlevel.hpp"
#include "d3d12_command_list.hpp"
#include "d3d12_command_queue.hpp"
#include "d3d12_resource.hpp"
#include "d3d12_impl_type_convert.hpp"

#include "../../Utils/com_utils.hpp"

#include "../../Hooks/hook_manager.hpp"


#include <cwchar> // std::wcslen
#include <algorithm> // std::find_if
#include <Lib/utfcpp/utf8/unchecked.h>

using reshade::d3d12::to_handle;

extern std::shared_mutex g_d3d12_adapter_mutex;

D3D12Device::D3D12Device(ID3D12Device *original) :
	device_impl(original)
{
	assert(_orig != nullptr);

	// Add proxy object to the private data of the device, so that it can be retrieved again when only the original device is available
	D3D12Device *const device_proxy = this;
	_orig->SetPrivateData(__uuidof(D3D12Device), sizeof(device_proxy), &device_proxy);
}
D3D12Device::~D3D12Device()
{
	// Remove pointer to this proxy object from the private data of the device (in case the device unexpectedly survives)
	_orig->SetPrivateData(__uuidof(D3D12Device), 0, nullptr);
}

bool D3D12Device::check_and_upgrade_interface(REFIID riid)
{
	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) || // This is the IID_IUnknown identity object
		riid == __uuidof(ID3D12Object))
		return true;

	static constexpr IID iid_lookup[] = {
		__uuidof(ID3D12Device),   // {189819F1-1DB6-4B57-BE54-1821339B85F7}
		__uuidof(ID3D12Device1),  // {77ACCE80-638E-4E65-8895-C1F23386863E}
		__uuidof(ID3D12Device2),  // {30BAA41E-B15B-475C-A0BB-1AF5C5B64328}
		__uuidof(ID3D12Device3),  // {81DADC15-2BAD-4392-93C5-101345C4AA98}
		__uuidof(ID3D12Device4),  // {E865DF17-A9EE-46F9-A463-3098315AA2E5}
		__uuidof(ID3D12Device5),  // {8B4F173B-2FEA-4B80-8F58-4307191AB95D}
		__uuidof(ID3D12Device6),  // {C70B221B-40E4-4A17-89AF-025A0727A6DC}
		__uuidof(ID3D12Device7),  // {5C014B53-68A1-4B9B-8BD1-DD6046B9358B}
		__uuidof(ID3D12Device8),  // {9218E6BB-F944-4F7E-A75C-B1B2C7B701F3}
		__uuidof(ID3D12Device9),  // {4C80E962-F032-4F60-BC9E-EBC2CFA1D83C}
		__uuidof(ID3D12Device10), // {517F8718-AA66-49F9-B02B-A7AB89C06031}
		__uuidof(ID3D12Device11), // {5405C344-D457-444E-B4DD-2366E45AEE39}
		__uuidof(ID3D12Device12), // {5AF5C532-4C91-4CD0-B541-15A405395FC5}
		__uuidof(ID3D12Device13), // {14EECFFC-4DF8-40F7-A118-5C816F45695E}
		__uuidof(ID3D12Device14), // {5F6E592D-D895-44C2-8E4A-88AD4926D323}
		__uuidof(ID3D12Device15), // {76CFF76F-1E9b-4450-8CDC-34F1AF788E5B}
	};

	for (unsigned short version = 0; version < std::size(iid_lookup); ++version)
	{
		if (riid != iid_lookup[version])
			continue;

		if (version > _interface_version)
		{
			IUnknown *new_interface = nullptr;
			if (FAILED(_orig->QueryInterface(riid, reinterpret_cast<void **>(&new_interface))))
				return false;
			_orig->Release();
			_orig = static_cast<ID3D12Device *>(new_interface);
			_interface_version = version;
		}

		return true;
	}

	return false;
}

HRESULT STDMETHODCALLTYPE D3D12Device::QueryInterface(REFIID riid, void **ppvObj)
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

	// Special case for d3d12on7
	if (riid == __uuidof(ID3D12DeviceDownlevel)) // {74EAEE3F-2F4B-476D-82BA-2B85CB49E310}
	{
		if (_downlevel == nullptr)
		{
			// Not a 'com_ptr' since D3D12DeviceDownlevel will take ownership
			ID3D12DeviceDownlevel *downlevel = nullptr;
			if (SUCCEEDED(_orig->QueryInterface(&downlevel)))
				_downlevel = new D3D12DeviceDownlevel(this, downlevel);
		}

		if (_downlevel != nullptr)
			return _downlevel->QueryInterface(riid, ppvObj);
		return E_NOINTERFACE;
	}

	// Unimplemented interfaces:
	//   ID3D12DebugDevice  {3FEBD6DD-4973-4787-8194-E45F9E28923E}
	//   ID3D12DebugDevice1 {A9B71770-D099-4A65-A698-3DEE10020F88}
	//   ID3D12DebugDevice2 {60ECCBC1-378D-4DF1-894C-F8AC5CE4D7DD}

	return _orig->QueryInterface(riid, ppvObj);
}
ULONG   STDMETHODCALLTYPE D3D12Device::AddRef()
{
	const std::unique_lock<std::shared_mutex> lock(g_d3d12_adapter_mutex);

	_orig->AddRef();
	return InterlockedIncrement(&_ref);
}
ULONG   STDMETHODCALLTYPE D3D12Device::Release()
{
	const std::unique_lock<std::shared_mutex> lock(g_d3d12_adapter_mutex);

	const ULONG ref = InterlockedDecrement(&_ref);
	if (ref != 0)
	{
		_orig->Release();
		return ref;
	}

	if (_downlevel != nullptr)
	{
		// Release the reference that was added when the downlevel interface was first queried in 'QueryInterface' above
		_downlevel->_orig->Release();
		delete _downlevel;
	}

	const auto orig = _orig;
	const auto interface_version = _interface_version;

	this->~D3D12Device();

	const ULONG ref_orig = orig->Release();
	if (ref_orig != 0) // Verify internal reference count
		Debug::Log("Reference count for ID3D12Device%hu object %p (%p) is inconsistent (%lu).", interface_version, this, orig, ref_orig);
	else
		operator delete(this, sizeof(D3D12Device));
	return 0;
}

HRESULT STDMETHODCALLTYPE D3D12Device::GetPrivateData(REFGUID guid, UINT *pDataSize, void *pData)
{
	return _orig->GetPrivateData(guid, pDataSize, pData);
}
HRESULT STDMETHODCALLTYPE D3D12Device::SetPrivateData(REFGUID guid, UINT DataSize, const void *pData)
{
	return _orig->SetPrivateData(guid, DataSize, pData);
}
HRESULT STDMETHODCALLTYPE D3D12Device::SetPrivateDataInterface(REFGUID guid, const IUnknown *pData)
{
	return _orig->SetPrivateDataInterface(guid, pData);
}
HRESULT STDMETHODCALLTYPE D3D12Device::SetName(LPCWSTR Name)
{
	return _orig->SetName(Name);
}

UINT    STDMETHODCALLTYPE D3D12Device::GetNodeCount()
{
	return _orig->GetNodeCount();
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC *pDesc, REFIID riid, void **ppCommandQueue)
{
	Debug::Log(
		
		"Redirecting ID3D12Device::CreateCommandQueue(this = %p, pDesc = %p, riid = %s, ppCommandQueue = %p) ...",
		this, pDesc, Debug::iid_to_string(riid).c_str(), ppCommandQueue);

	if (pDesc == nullptr)
		return E_INVALIDARG;

	Debug::Log("> Dumping command queue description:");
	Debug::Log("  +-----------------------------------------+-----------------------------------------+");
	Debug::Log("  | Parameter                               | Value                                   |");
	Debug::Log("  +-----------------------------------------+-----------------------------------------+");
	Debug::Log("  | Type                                    | %-39d |", static_cast<int>(pDesc->Type));
	Debug::Log("  | Priority                                | %-39d |", pDesc->Priority);
	Debug::Log("  | Flags                                   | %-39x |", static_cast<unsigned int>(pDesc->Flags));
	Debug::Log("  | NodeMask                                | %-39x |", pDesc->NodeMask);
	Debug::Log("  +-----------------------------------------+-----------------------------------------+");

	const HRESULT hr = _orig->CreateCommandQueue(pDesc, riid, ppCommandQueue);
	if (SUCCEEDED(hr))
	{
		assert(ppCommandQueue != nullptr);

		const auto command_queue_proxy = new D3D12CommandQueue(this, static_cast<ID3D12CommandQueue *>(*ppCommandQueue));

		// Upgrade to the actual interface version requested here
		if (command_queue_proxy->check_and_upgrade_interface(riid))
		{
			*ppCommandQueue = command_queue_proxy;
		}
		else // Do not hook object if we do not support the requested interface
		{
			delete command_queue_proxy; // Delete instead of release to keep reference count untouched
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void **ppCommandAllocator)
{
	return _orig->CreateCommandAllocator(type, riid, ppCommandAllocator);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC *pDesc, REFIID riid, void **ppPipelineState)
{
	if (pDesc == nullptr)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	hr = _orig->CreateGraphicsPipelineState(pDesc, riid, ppPipelineState);

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC *pDesc, REFIID riid, void **ppPipelineState)
{
	if (pDesc == nullptr)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	hr = _orig->CreateComputePipelineState(pDesc, riid, ppPipelineState);


	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator *pCommandAllocator, ID3D12PipelineState *pInitialState, REFIID riid, void **ppCommandList)
{
	const HRESULT hr = _orig->CreateCommandList(nodeMask, type, pCommandAllocator, pInitialState, riid, ppCommandList);
	if (SUCCEEDED(hr))
	{
		assert(ppCommandList != nullptr);

		// Ignore video command lists ('ID3D12VideoProcessCommandList', 'ID3D12VideoProcessCommandList', ...) in case they are created with the base 'ID3D12CommandList' interface
		if (type >= D3D12_COMMAND_LIST_TYPE_DIRECT && type <= D3D12_COMMAND_LIST_TYPE_COPY)
		{
			const auto command_list_proxy = new D3D12GraphicsCommandList(this, static_cast<ID3D12GraphicsCommandList *>(*ppCommandList));

			// Upgrade to the actual interface version requested here (and only hook graphics command lists)
			if (command_list_proxy->check_and_upgrade_interface(riid))
			{
				*ppCommandList = command_list_proxy;
			}
			else // Do not hook object if we do not support the requested interface
			{
				Debug::Log("Unknown interface %s in ID3D12Device::CreateCommandList.", Debug::iid_to_string(riid).c_str());

				delete command_list_proxy; // Delete instead of release to keep reference count untouched
			}
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CheckFeatureSupport(D3D12_FEATURE Feature, void *pFeatureSupportData, UINT FeatureSupportDataSize)
{
	return _orig->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC *pDescriptorHeapDesc, REFIID riid, void **ppvHeap)
{

	const HRESULT hr = _orig->CreateDescriptorHeap(pDescriptorHeapDesc, riid, ppvHeap);
	if (SUCCEEDED(hr))
	{
		assert(pDescriptorHeapDesc != nullptr);
	}

	return hr;
}
UINT    STDMETHODCALLTYPE D3D12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType)
{
	return _orig->GetDescriptorHandleIncrementSize(DescriptorHeapType);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateRootSignature(UINT nodeMask, const void *pBlobWithRootSignature, SIZE_T blobLengthInBytes, REFIID riid, void **ppvRootSignature)
{
	HRESULT hr = S_OK;
	hr = _orig->CreateRootSignature(nodeMask, pBlobWithRootSignature, blobLengthInBytes, riid, ppvRootSignature);


	return hr;
}
void    STDMETHODCALLTYPE D3D12Device::CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	_orig->CreateConstantBufferView(pDesc, DestDescriptor);
}
void    STDMETHODCALLTYPE D3D12Device::CreateShaderResourceView(ID3D12Resource *pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	_orig->CreateShaderResourceView(pResource, pDesc, DestDescriptor);
}
void    STDMETHODCALLTYPE D3D12Device::CreateUnorderedAccessView(ID3D12Resource *pResource, ID3D12Resource *pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	_orig->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, DestDescriptor);
}
void    STDMETHODCALLTYPE D3D12Device::CreateRenderTargetView(ID3D12Resource *pResource, const D3D12_RENDER_TARGET_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	_orig->CreateRenderTargetView(pResource, pDesc, DestDescriptor);
}
void    STDMETHODCALLTYPE D3D12Device::CreateDepthStencilView(ID3D12Resource *pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	_orig->CreateDepthStencilView(pResource, pDesc, DestDescriptor);
}
void    STDMETHODCALLTYPE D3D12Device::CreateSampler(const D3D12_SAMPLER_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	_orig->CreateSampler(pDesc, DestDescriptor);
}
void    STDMETHODCALLTYPE D3D12Device::CopyDescriptors(UINT NumDestDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE *pDestDescriptorRangeStarts, const UINT *pDestDescriptorRangeSizes, UINT NumSrcDescriptorRanges, const D3D12_CPU_DESCRIPTOR_HANDLE *pSrcDescriptorRangeStarts, const UINT *pSrcDescriptorRangeSizes, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
	_orig->CopyDescriptors(NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);
}
void    STDMETHODCALLTYPE D3D12Device::CopyDescriptorsSimple(UINT NumDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
	_orig->CopyDescriptorsSimple(NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);
}
D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE D3D12Device::GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC *pResourceDescs)
{
	return _orig->GetResourceAllocationInfo(visibleMask, numResourceDescs, pResourceDescs);
}
D3D12_HEAP_PROPERTIES STDMETHODCALLTYPE D3D12Device::GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE heapType)
{
	return _orig->GetCustomHeapProperties(nodeMask, heapType);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommittedResource(const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue, REFIID riid, void **ppvResource)
{
	if (pHeapProperties == nullptr || pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return _orig->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riid, ppvResource);

	const HRESULT hr = _orig->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateHeap(const D3D12_HEAP_DESC *pDesc, REFIID riid, void **ppvHeap)
{
	return _orig->CreateHeap(pDesc, riid, ppvHeap);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreatePlacedResource(ID3D12Heap *pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE *pOptimizedClearValue, REFIID riid, void **ppvResource)
{
	if (pHeap == nullptr || pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return _orig->CreatePlacedResource(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);

	const HRESULT hr = _orig->CreatePlacedResource(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);

		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateReservedResource(const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE *pOptimizedClearValue, REFIID riid, void **ppvResource)
{
	if (pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return _orig->CreateReservedResource(pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);

	const HRESULT hr = _orig->CreateReservedResource(pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateSharedHandle(ID3D12DeviceChild *pObject, const SECURITY_ATTRIBUTES *pAttributes, DWORD Access, LPCWSTR Name, HANDLE *pHandle)
{
	return _orig->CreateSharedHandle(pObject, pAttributes, Access, Name, pHandle);
}
HRESULT STDMETHODCALLTYPE D3D12Device::OpenSharedHandle(HANDLE NTHandle, REFIID riid, void **ppvObj)
{
	const HRESULT hr = _orig->OpenSharedHandle(NTHandle, riid, ppvObj);
	if (SUCCEEDED(hr) && ppvObj != nullptr)
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvObj);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::OpenSharedHandleByName(LPCWSTR Name, DWORD Access, HANDLE *pNTHandle)
{
	return _orig->OpenSharedHandleByName(Name, Access, pNTHandle);
}
HRESULT STDMETHODCALLTYPE D3D12Device::MakeResident(UINT NumObjects, ID3D12Pageable *const *ppObjects)
{
	return _orig->MakeResident(NumObjects, ppObjects);
}
HRESULT STDMETHODCALLTYPE D3D12Device::Evict(UINT NumObjects, ID3D12Pageable *const *ppObjects)
{
	return _orig->Evict(NumObjects, ppObjects);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateFence(UINT64 InitialValue, D3D12_FENCE_FLAGS Flags, REFIID riid, void **ppFence)
{
	return _orig->CreateFence(InitialValue, Flags, riid, ppFence);
}
HRESULT STDMETHODCALLTYPE D3D12Device::GetDeviceRemovedReason()
{
	return _orig->GetDeviceRemovedReason();
}
void    STDMETHODCALLTYPE D3D12Device::GetCopyableFootprints(const D3D12_RESOURCE_DESC *pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pLayouts, UINT *pNumRows, UINT64 *pRowSizeInBytes, UINT64 *pTotalBytes)
{
	_orig->GetCopyableFootprints(pResourceDesc, FirstSubresource, NumSubresources, BaseOffset, pLayouts, pNumRows, pRowSizeInBytes, pTotalBytes);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateQueryHeap(const D3D12_QUERY_HEAP_DESC *pDesc, REFIID riid, void **ppvHeap)
{
	const HRESULT hr = _orig->CreateQueryHeap(pDesc, riid, ppvHeap);


	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::SetStablePowerState(BOOL Enable)
{
	return _orig->SetStablePowerState(Enable);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandSignature(const D3D12_COMMAND_SIGNATURE_DESC *pDesc, ID3D12RootSignature *pRootSignature, REFIID riid, void **ppvCommandSignature)
{
	return _orig->CreateCommandSignature(pDesc, pRootSignature, riid, ppvCommandSignature);
}
void    STDMETHODCALLTYPE D3D12Device::GetResourceTiling(ID3D12Resource *pTiledResource, UINT *pNumTilesForEntireResource, D3D12_PACKED_MIP_INFO *pPackedMipDesc, D3D12_TILE_SHAPE *pStandardTileShapeForNonPackedMips, UINT *pNumSubresourceTilings, UINT FirstSubresourceTilingToGet, D3D12_SUBRESOURCE_TILING *pSubresourceTilingsForNonPackedMips)
{
	_orig->GetResourceTiling(pTiledResource, pNumTilesForEntireResource, pPackedMipDesc, pStandardTileShapeForNonPackedMips, pNumSubresourceTilings, FirstSubresourceTilingToGet, pSubresourceTilingsForNonPackedMips);
}
LUID    STDMETHODCALLTYPE D3D12Device::GetAdapterLuid()
{
	return _orig->GetAdapterLuid();
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreatePipelineLibrary(const void *pLibraryBlob, SIZE_T BlobLength, REFIID riid, void **ppPipelineLibrary)
{
	assert(_interface_version >= 1);

	const HRESULT hr = static_cast<ID3D12Device1 *>(_orig)->CreatePipelineLibrary(pLibraryBlob, BlobLength, riid, ppPipelineLibrary);
	if (SUCCEEDED(hr) && ppPipelineLibrary != nullptr)
	{
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::SetEventOnMultipleFenceCompletion(ID3D12Fence *const *ppFences, const UINT64 *pFenceValues, UINT NumFences, D3D12_MULTIPLE_FENCE_WAIT_FLAGS Flags, HANDLE hEvent)
{
	assert(_interface_version >= 1);

	return static_cast<ID3D12Device1 *>(_orig)->SetEventOnMultipleFenceCompletion(ppFences, pFenceValues, NumFences, Flags, hEvent);
}
HRESULT STDMETHODCALLTYPE D3D12Device::SetResidencyPriority(UINT NumObjects, ID3D12Pageable *const *ppObjects, const D3D12_RESIDENCY_PRIORITY *pPriorities)
{
	assert(_interface_version >= 1);


	return static_cast<ID3D12Device1 *>(_orig)->SetResidencyPriority(NumObjects, ppObjects, pPriorities);
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreatePipelineState(const D3D12_PIPELINE_STATE_STREAM_DESC *pDesc, REFIID riid, void **ppPipelineState)
{
	assert(_interface_version >= 2);

	if (pDesc == nullptr)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
		hr = static_cast<ID3D12Device2 *>(_orig)->CreatePipelineState(pDesc, riid, ppPipelineState);

	return hr;
}

HRESULT STDMETHODCALLTYPE D3D12Device::OpenExistingHeapFromAddress(const void *pAddress, REFIID riid, void **ppvHeap)
{
	assert(_interface_version >= 3);

	return static_cast<ID3D12Device3 *>(_orig)->OpenExistingHeapFromAddress(pAddress, riid, ppvHeap);
}
HRESULT STDMETHODCALLTYPE D3D12Device::OpenExistingHeapFromFileMapping(HANDLE hFileMapping, REFIID riid, void **ppvHeap)
{
	assert(_interface_version >= 3);

	return static_cast<ID3D12Device3 *>(_orig)->OpenExistingHeapFromFileMapping(hFileMapping, riid, ppvHeap);
}
HRESULT STDMETHODCALLTYPE D3D12Device::EnqueueMakeResident(D3D12_RESIDENCY_FLAGS Flags, UINT NumObjects, ID3D12Pageable *const *ppObjects, ID3D12Fence *pFenceToSignal, UINT64 FenceValueToSignal)
{
	assert(_interface_version >= 3);

	return static_cast<ID3D12Device3 *>(_orig)->EnqueueMakeResident(Flags, NumObjects, ppObjects, pFenceToSignal, FenceValueToSignal);
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandList1(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_LIST_FLAGS flags, REFIID riid, void **ppCommandList)
{
	assert(_interface_version >= 4);

	const HRESULT hr = static_cast<ID3D12Device4 *>(_orig)->CreateCommandList1(nodeMask, type, flags, riid, ppCommandList);
	if (SUCCEEDED(hr))
	{
		assert(ppCommandList != nullptr);

		// Ignore video command lists ('ID3D12VideoProcessCommandList', 'ID3D12VideoProcessCommandList', ...) in case they are created with the base 'ID3D12CommandList' interface
		if (type >= D3D12_COMMAND_LIST_TYPE_DIRECT && type <= D3D12_COMMAND_LIST_TYPE_COPY)
		{
			const auto command_list_proxy = new D3D12GraphicsCommandList(this, static_cast<ID3D12GraphicsCommandList *>(*ppCommandList));

			// Upgrade to the actual interface version requested here (and only hook graphics command lists)
			if (command_list_proxy->check_and_upgrade_interface(riid))
			{
				*ppCommandList = command_list_proxy;
	}
			else // Do not hook object if we do not support the requested interface or this is a compute command list
			{
				Debug::Log("Unknown interface %s in ID3D12Device4::CreateCommandList1.", Debug::iid_to_string(riid).c_str());

				delete command_list_proxy; // Delete instead of release to keep reference count untouched
			}
		}
	}

	return hr;

}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateProtectedResourceSession(const D3D12_PROTECTED_RESOURCE_SESSION_DESC *pDesc, REFIID riid, void **ppSession)
{
	assert(_interface_version >= 4);

	return static_cast<ID3D12Device4 *>(_orig)->CreateProtectedResourceSession(pDesc, riid, ppSession);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommittedResource1(const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue, ID3D12ProtectedResourceSession *pProtectedSession, REFIID riid, void **ppvResource)
{
	assert(_interface_version >= 4);

	if (pHeapProperties == nullptr || pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return static_cast<ID3D12Device4 *>(_orig)->CreateCommittedResource1(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, pProtectedSession, riid, ppvResource);

	const HRESULT hr = static_cast<ID3D12Device4 *>(_orig)->CreateCommittedResource1(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, pProtectedSession, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateHeap1(const D3D12_HEAP_DESC *pDesc, ID3D12ProtectedResourceSession *pProtectedSession, REFIID riid, void **ppvHeap)
{
	assert(_interface_version >= 4);

	return static_cast<ID3D12Device4 *>(_orig)->CreateHeap1(pDesc, pProtectedSession, riid, ppvHeap);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateReservedResource1(const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE *pOptimizedClearValue, ID3D12ProtectedResourceSession *pProtectedSession, REFIID riid, void **ppvResource)
{
	assert(_interface_version >= 4);

	if (pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return static_cast<ID3D12Device4 *>(_orig)->CreateReservedResource1(pDesc, InitialState, pOptimizedClearValue, pProtectedSession, riid, ppvResource);

	const HRESULT hr = static_cast<ID3D12Device4 *>(_orig)->CreateReservedResource1(pDesc, InitialState, pOptimizedClearValue, pProtectedSession, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);


		}
	}


	return hr;
}
D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE D3D12Device::GetResourceAllocationInfo1(UINT VisibleMask, UINT NumResourceDescs, const D3D12_RESOURCE_DESC *pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1)
{
	assert(_interface_version >= 4);

	return static_cast<ID3D12Device4 *>(_orig)->GetResourceAllocationInfo1(VisibleMask, NumResourceDescs, pResourceDescs, pResourceAllocationInfo1);
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateLifetimeTracker(ID3D12LifetimeOwner *pOwner, REFIID riid, void **ppvTracker)
{
	assert(_interface_version >= 5);

	return static_cast<ID3D12Device5 *>(_orig)->CreateLifetimeTracker(pOwner, riid, ppvTracker);
}
void    STDMETHODCALLTYPE D3D12Device::RemoveDevice()
{
	assert(_interface_version >= 5);

	static_cast<ID3D12Device5 *>(_orig)->RemoveDevice();
}
HRESULT STDMETHODCALLTYPE D3D12Device::EnumerateMetaCommands(UINT *pNumMetaCommands, D3D12_META_COMMAND_DESC *pDescs)
{
	assert(_interface_version >= 5);

	return static_cast<ID3D12Device5 *>(_orig)->EnumerateMetaCommands(pNumMetaCommands, pDescs);
}
HRESULT STDMETHODCALLTYPE D3D12Device::EnumerateMetaCommandParameters(REFGUID CommandId, D3D12_META_COMMAND_PARAMETER_STAGE Stage, UINT *pTotalStructureSizeInBytes, UINT *pParameterCount, D3D12_META_COMMAND_PARAMETER_DESC *pParameterDescs)
{
	assert(_interface_version >= 5);

	return static_cast<ID3D12Device5 *>(_orig)->EnumerateMetaCommandParameters(CommandId, Stage, pTotalStructureSizeInBytes, pParameterCount, pParameterDescs);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateMetaCommand(REFGUID CommandId, UINT NodeMask, const void *pCreationParametersData, SIZE_T CreationParametersDataSizeInBytes, REFIID riid, void **ppMetaCommand)
{
	assert(_interface_version >= 5);

	return static_cast<ID3D12Device5 *>(_orig)->CreateMetaCommand(CommandId, NodeMask, pCreationParametersData, CreationParametersDataSizeInBytes, riid, ppMetaCommand);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateStateObject(const D3D12_STATE_OBJECT_DESC *pDesc, REFIID riid, void **ppStateObject)
{
	assert(_interface_version >= 5);

	if (pDesc == nullptr)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	hr = static_cast<ID3D12Device5 *>(_orig)->CreateStateObject(pDesc, riid, ppStateObject);

	return hr;
}
void    STDMETHODCALLTYPE D3D12Device::GetRaytracingAccelerationStructurePrebuildInfo(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS *pDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO *pInfo)
{
	// assert(_interface_version >= 5); // Cyberpunk 2077 incorrectly calls this on a 'ID3D12Device3' object

	static_cast<ID3D12Device5 *>(_orig)->GetRaytracingAccelerationStructurePrebuildInfo(pDesc, pInfo);
}
D3D12_DRIVER_MATCHING_IDENTIFIER_STATUS STDMETHODCALLTYPE D3D12Device::CheckDriverMatchingIdentifier(D3D12_SERIALIZED_DATA_TYPE SerializedDataType, const D3D12_SERIALIZED_DATA_DRIVER_MATCHING_IDENTIFIER *pIdentifierToCheck)
{
	assert(_interface_version >= 5);

	return static_cast<ID3D12Device5 *>(_orig)->CheckDriverMatchingIdentifier(SerializedDataType, pIdentifierToCheck);
}

HRESULT STDMETHODCALLTYPE D3D12Device::SetBackgroundProcessingMode(D3D12_BACKGROUND_PROCESSING_MODE Mode, D3D12_MEASUREMENTS_ACTION MeasurementsAction, HANDLE hEventToSignalUponCompletion, BOOL *pbFurtherMeasurementsDesired)
{
	assert(_interface_version >= 6);

	return static_cast<ID3D12Device6*>(_orig)->SetBackgroundProcessingMode(Mode, MeasurementsAction, hEventToSignalUponCompletion, pbFurtherMeasurementsDesired);
}

HRESULT STDMETHODCALLTYPE D3D12Device::AddToStateObject(const D3D12_STATE_OBJECT_DESC *pAddition, ID3D12StateObject *pStateObjectToGrowFrom, REFIID riid, void **ppNewStateObject)
{
	assert(_interface_version >= 7);

	if (pAddition == nullptr)
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	hr = static_cast<ID3D12Device7 *>(_orig)->AddToStateObject(pAddition, pStateObjectToGrowFrom, riid, ppNewStateObject);

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateProtectedResourceSession1(const D3D12_PROTECTED_RESOURCE_SESSION_DESC1 *pDesc, REFIID riid, void **ppSession)
{
	assert(_interface_version >= 7);

	return static_cast<ID3D12Device7 *>(_orig)->CreateProtectedResourceSession1(pDesc, riid, ppSession);
}

D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE D3D12Device::GetResourceAllocationInfo2(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC1 *pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1)
{
	assert(_interface_version >= 8);

	return static_cast<ID3D12Device8 *>(_orig)->GetResourceAllocationInfo2(visibleMask, numResourceDescs, pResourceDescs, pResourceAllocationInfo1);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommittedResource2(const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC1 *pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue, ID3D12ProtectedResourceSession *pProtectedSession, REFIID riid, void **ppvResource)
{
	assert(_interface_version >= 8);

	if (pHeapProperties == nullptr || pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return static_cast<ID3D12Device8 *>(_orig)->CreateCommittedResource2(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, pProtectedSession, riid, ppvResource);

	const HRESULT hr = static_cast<ID3D12Device8 *>(_orig)->CreateCommittedResource2(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, pProtectedSession, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreatePlacedResource1(ID3D12Heap *pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC1 *pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE *pOptimizedClearValue, REFIID riid, void **ppvResource)
{
	assert(_interface_version >= 8);

	if (pHeap == nullptr || pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return static_cast<ID3D12Device8 *>(_orig)->CreatePlacedResource1(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);

	const HRESULT hr = static_cast<ID3D12Device8 *>(_orig)->CreatePlacedResource1(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);

		}
	}

	return hr;
}
void    STDMETHODCALLTYPE D3D12Device::CreateSamplerFeedbackUnorderedAccessView(ID3D12Resource *pTargetedResource, ID3D12Resource *pFeedbackResource, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{

	assert(_interface_version >= 8);

	static_cast<ID3D12Device8 *>(_orig)->CreateSamplerFeedbackUnorderedAccessView(pTargetedResource, pFeedbackResource, DestDescriptor);
}
void    STDMETHODCALLTYPE D3D12Device::GetCopyableFootprints1(const D3D12_RESOURCE_DESC1 *pResourceDesc, UINT FirstSubresource, UINT NumSubresources, UINT64 BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pLayouts, UINT *pNumRows, UINT64 *pRowSizeInBytes, UINT64 *pTotalBytes)
{
	assert(_interface_version >= 8);

	static_cast<ID3D12Device8 *>(_orig)->GetCopyableFootprints1(pResourceDesc, FirstSubresource, NumSubresources, BaseOffset, pLayouts, pNumRows, pRowSizeInBytes, pTotalBytes);
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateShaderCacheSession(const D3D12_SHADER_CACHE_SESSION_DESC *pDesc, REFIID riid, void **ppvSession)
{
	assert(_interface_version >= 9);

	return static_cast<ID3D12Device9 *>(_orig)->CreateShaderCacheSession(pDesc, riid, ppvSession);
}
HRESULT STDMETHODCALLTYPE D3D12Device::ShaderCacheControl(D3D12_SHADER_CACHE_KIND_FLAGS Kinds, D3D12_SHADER_CACHE_CONTROL_FLAGS Control)
{
	assert(_interface_version >= 9);

	return static_cast<ID3D12Device9 *>(_orig)->ShaderCacheControl(Kinds, Control);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandQueue1(const D3D12_COMMAND_QUEUE_DESC *pDesc, REFIID CreatorID, REFIID riid, void **ppCommandQueue)
{
	assert(_interface_version >= 9);

	Debug::Log(
		"Redirecting ID3D12Device9::CreateCommandQueue1(this = %p, pDesc = %p, CreatorID = %s, riid = %s, ppCommandQueue = %p) ...",
		this, pDesc, Debug::iid_to_string(CreatorID).c_str(), Debug::iid_to_string(riid).c_str(), ppCommandQueue);

	if (pDesc == nullptr)
		return E_INVALIDARG;

	Debug::Log("> Dumping command queue description:");
	Debug::Log("  +-----------------------------------------+-----------------------------------------+");
	Debug::Log("  | Parameter                               | Value                                   |");
	Debug::Log("  +-----------------------------------------+-----------------------------------------+");
	Debug::Log("  | Type                                    | %-39d |", static_cast<int>(pDesc->Type));
	Debug::Log("  | Priority                                | %-39d |", pDesc->Priority);
	Debug::Log("  | Flags                                   | %-39x |", static_cast<unsigned int>(pDesc->Flags));
	Debug::Log("  | NodeMask                                | %-39x |", pDesc->NodeMask);
	Debug::Log("  +-----------------------------------------+-----------------------------------------+");

	const HRESULT hr = static_cast<ID3D12Device9 *>(_orig)->CreateCommandQueue1(pDesc, CreatorID, riid, ppCommandQueue);
	if (SUCCEEDED(hr))
	{
		assert(ppCommandQueue != nullptr);

		const auto command_queue_proxy = new D3D12CommandQueue(this, static_cast<ID3D12CommandQueue *>(*ppCommandQueue));

		// Upgrade to the actual interface version requested here
		if (command_queue_proxy->check_and_upgrade_interface(riid))
		{
			*ppCommandQueue = command_queue_proxy;
		}
		else // Do not hook object if we do not support the requested interface
		{
			delete command_queue_proxy; // Delete instead of release to keep reference count untouched
		}
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommittedResource3(const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC1 *pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE *pOptimizedClearValue, ID3D12ProtectedResourceSession *pProtectedSession, UINT32 NumCastableFormats, const DXGI_FORMAT *pCastableFormats, REFIID riid, void **ppvResource)
{
	assert(_interface_version >= 10);

	if (pHeapProperties == nullptr || pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return static_cast<ID3D12Device10 *>(_orig)->CreateCommittedResource3(pHeapProperties, HeapFlags, pDesc, InitialLayout, pOptimizedClearValue, pProtectedSession, NumCastableFormats, pCastableFormats, riid, ppvResource);

	const HRESULT hr = static_cast<ID3D12Device10 *>(_orig)->CreateCommittedResource3(pHeapProperties, HeapFlags, pDesc, InitialLayout, pOptimizedClearValue, pProtectedSession, NumCastableFormats, pCastableFormats, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreatePlacedResource2(ID3D12Heap *pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC1 *pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE *pOptimizedClearValue, UINT32 NumCastableFormats, const DXGI_FORMAT *pCastableFormats, REFIID riid, void **ppvResource)
{
	assert(_interface_version >= 10);

	if (pHeap == nullptr || pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return static_cast<ID3D12Device10 *>(_orig)->CreatePlacedResource2(pHeap, HeapOffset, pDesc, InitialLayout, pOptimizedClearValue, NumCastableFormats, pCastableFormats, riid, ppvResource);

	const HRESULT hr = static_cast<ID3D12Device10 *>(_orig)->CreatePlacedResource2(pHeap, HeapOffset, pDesc, InitialLayout, pOptimizedClearValue, NumCastableFormats, pCastableFormats, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateReservedResource2(const D3D12_RESOURCE_DESC *pDesc, D3D12_BARRIER_LAYOUT InitialLayout, const D3D12_CLEAR_VALUE *pOptimizedClearValue, ID3D12ProtectedResourceSession *pProtectedSession, UINT32 NumCastableFormats, const DXGI_FORMAT *pCastableFormats, REFIID riid, void **ppvResource)
{
	assert(_interface_version >= 10);

	if (pDesc == nullptr)
		return E_INVALIDARG;
	if (ppvResource == nullptr) // This can happen when application only wants to validate input parameters
		return static_cast<ID3D12Device10 *>(_orig)->CreateReservedResource2(pDesc, InitialLayout, pOptimizedClearValue, pProtectedSession, NumCastableFormats, pCastableFormats, riid, ppvResource);

	const HRESULT hr = static_cast<ID3D12Device10 *>(_orig)->CreateReservedResource2(pDesc, InitialLayout, pOptimizedClearValue, pProtectedSession, NumCastableFormats, pCastableFormats, riid, ppvResource);
	if (SUCCEEDED(hr))
	{
		if (riid == __uuidof(ID3D12Resource) ||
			riid == __uuidof(ID3D12Resource1) ||
			riid == __uuidof(ID3D12Resource2))
		{
			const auto resource = static_cast<ID3D12Resource *>(*ppvResource);

			reshade::hooks::install("ID3D12Resource::GetDevice", reshade::hooks::vtable_from_instance(resource), 7, &ID3D12Resource_GetDevice);
		}
	}

	return hr;
}

void    STDMETHODCALLTYPE D3D12Device::CreateSampler2(const D3D12_SAMPLER_DESC2 *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	assert(_interface_version >= 11);


	static_cast<ID3D12Device11 *>(_orig)->CreateSampler2(pDesc, DestDescriptor);
}

D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE D3D12Device::GetResourceAllocationInfo3(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC1 *pResourceDescs, const UINT32 *pNumCastableFormats, const DXGI_FORMAT *const *ppCastableFormats, D3D12_RESOURCE_ALLOCATION_INFO1 *pResourceAllocationInfo1)
{
	assert(_interface_version >= 12);

	return static_cast<ID3D12Device12 *>(_orig)->GetResourceAllocationInfo3(visibleMask, numResourceDescs, pResourceDescs, pNumCastableFormats, ppCastableFormats, pResourceAllocationInfo1);
}

HRESULT STDMETHODCALLTYPE D3D12Device::OpenExistingHeapFromAddress1(const void *pAddress, SIZE_T size, REFIID riid, void **ppvHeap)
{
	assert(_interface_version >= 13);

	return static_cast<ID3D12Device13 *>(_orig)->OpenExistingHeapFromAddress1(pAddress, size, riid, ppvHeap);
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateRootSignatureFromSubobjectInLibrary(UINT nodeMask, const void *pLibraryBlob, SIZE_T blobLengthInBytes, LPCWSTR subobjectName, REFIID riid, void **ppvRootSignature)
{
	assert(_interface_version >= 14);

	return static_cast<ID3D12Device14 *>(_orig)->CreateRootSignatureFromSubobjectInLibrary(nodeMask, pLibraryBlob, blobLengthInBytes, subobjectName, riid, ppvRootSignature);
}

HRESULT STDMETHODCALLTYPE D3D12Device::RegisterTrimNotificationCallback(D3D12_REGISTER_TRIM_NOTIFICATION *pData)
{
	assert(_interface_version >= 15);

	return static_cast<ID3D12Device15 *>(_orig)->RegisterTrimNotificationCallback(pData);
}
HRESULT STDMETHODCALLTYPE D3D12Device::UnregisterTrimNotificationCallback(DWORD CallbackCookie)
{
	assert(_interface_version >= 15);

	return static_cast<ID3D12Device15 *>(_orig)->UnregisterTrimNotificationCallback(CallbackCookie);
}
HRESULT STDMETHODCALLTYPE D3D12Device::TryCreateShaderResourceView(ID3D12Resource *pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	assert(_interface_version >= 15);
	const HRESULT hr = static_cast<ID3D12Device15 *>(_orig)->TryCreateShaderResourceView(pResource, pDesc, DestDescriptor);


	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::TryCreateUnorderedAccessView(ID3D12Resource *pResource, ID3D12Resource *pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	assert(_interface_version >= 15);

	const HRESULT hr = static_cast<ID3D12Device15 *>(_orig)->TryCreateUnorderedAccessView(pResource, pCounterResource, pDesc, DestDescriptor);
	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::TryCreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	assert(_interface_version >= 15);
	const HRESULT hr = static_cast<ID3D12Device15 *>(_orig)->TryCreateConstantBufferView(pDesc, DestDescriptor);

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::TryCreateSampler2(const D3D12_SAMPLER_DESC2 *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	assert(_interface_version >= 15);

	const HRESULT hr = static_cast<ID3D12Device15 *>(_orig)->TryCreateSampler2(pDesc, DestDescriptor);

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::TryCreateRenderTargetView(ID3D12Resource *pResource, const D3D12_RENDER_TARGET_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	assert(_interface_version >= 15);

	const HRESULT hr = static_cast<ID3D12Device15 *>(_orig)->TryCreateRenderTargetView(pResource, pDesc, DestDescriptor);

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::TryCreateDepthStencilView(ID3D12Resource *pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC *pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	assert(_interface_version >= 15);

	const HRESULT hr = static_cast<ID3D12Device15 *>(_orig)->TryCreateDepthStencilView(pResource, pDesc, DestDescriptor);

	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::TryCreateSamplerFeedbackUnorderedAccessView(ID3D12Resource *pTargetedResource, ID3D12Resource *pFeedbackResource, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	assert(_interface_version >= 15);

	return static_cast<ID3D12Device15 *>(_orig)->TryCreateSamplerFeedbackUnorderedAccessView(pTargetedResource, pFeedbackResource, DestDescriptor);
}
HRESULT STDMETHODCALLTYPE D3D12Device::CreateQueryHeap1(const D3D12_QUERY_HEAP_DESC *pDesc, D3D12_QUERY_HEAP_FLAGS Flags, REFIID riid, void **ppvHeap)
{
	assert(_interface_version >= 15);

	const HRESULT hr = static_cast<ID3D12Device15 *>(_orig)->CreateQueryHeap1(pDesc, Flags, riid, ppvHeap);
	return hr;
}
HRESULT STDMETHODCALLTYPE D3D12Device::ResolveQueryData(ID3D12QueryHeap *pQueryHeap, D3D12_QUERY_TYPE Type, UINT StartIndex, UINT NumQueries, void *pResolved)
{
	assert(_interface_version >= 15);
	return static_cast<ID3D12Device15 *>(_orig)->ResolveQueryData(pQueryHeap, Type, StartIndex, NumQueries, pResolved);
}