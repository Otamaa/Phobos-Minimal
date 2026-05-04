/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#include "vulkan_hooks.hpp"
#include "vulkan_impl_device.hpp"
#include "../../Utils/lockfree_linear_map.hpp"
#include <cstring> // std::strcmp

extern lockfree_linear_map<void *, vulkan_instance, 16> g_vulkan_instances;
extern lockfree_linear_map<void *, reshade::vulkan::device_impl *, 8> g_vulkan_devices;

#define RESHADE_VULKAN_HOOK_PROC(name) \
	if (0 == std::strcmp(name_without_prefix, #name)) \
		return reinterpret_cast<PFN_vkVoidFunction>(vk##name)
#define RESHADE_VULKAN_HOOK_PROC_OPTIONAL(name, suffix) \
	if (0 == std::strcmp(name_without_prefix, #name #suffix) && g_vulkan_devices.at(dispatch_key_from_handle(device))->_dispatch_table.name != nullptr) \
		return reinterpret_cast<PFN_vkVoidFunction>(vk##name);

PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char *pName)
{
	if (pName == nullptr || pName[0] != 'v' || pName[1] != 'k')
		return nullptr;
	const char *name_without_prefix = pName + 2;

	// The Vulkan loader gets the 'vkDestroyDevice' function from the device dispatch table
	RESHADE_VULKAN_HOOK_PROC(DestroyDevice);

#if VK_KHR_swapchain
	RESHADE_VULKAN_HOOK_PROC(CreateSwapchainKHR);
	RESHADE_VULKAN_HOOK_PROC(DestroySwapchainKHR);
	RESHADE_VULKAN_HOOK_PROC(QueuePresentKHR);
#endif

	// Need to self-intercept as well, since some layers rely on this (e.g. Steam overlay)
	// See https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md#layer-conventions-and-rules
	RESHADE_VULKAN_HOOK_PROC(GetDeviceProcAddr);

	if (device == VK_NULL_HANDLE)
		return nullptr;

	const auto trampoline = g_vulkan_devices.at(dispatch_key_from_handle(device))->_dispatch_table.GetDeviceProcAddr;

	return trampoline(device, pName);
}

PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *pName)
{
	if (pName == nullptr || pName[0] != 'v' || pName[1] != 'k')
		return nullptr;
	const char *name_without_prefix = pName + 2;

#if VK_VERSION_1_0
	RESHADE_VULKAN_HOOK_PROC(CreateInstance);
	RESHADE_VULKAN_HOOK_PROC(DestroyInstance);
	RESHADE_VULKAN_HOOK_PROC(CreateDevice);
	RESHADE_VULKAN_HOOK_PROC(DestroyDevice);
#endif

#if VK_VERSION_1_3
	RESHADE_VULKAN_HOOK_PROC(GetPhysicalDeviceToolProperties);
#endif

#if VK_KHR_win32_surface
	RESHADE_VULKAN_HOOK_PROC(CreateWin32SurfaceKHR);
#endif

#if VK_KHR_surface
	RESHADE_VULKAN_HOOK_PROC(DestroySurfaceKHR);
#endif

#if VK_EXT_tooling_info
	RESHADE_VULKAN_HOOK_PROC(GetPhysicalDeviceToolPropertiesEXT);
#endif

	// Self-intercept here as well to stay consistent with 'vkGetDeviceProcAddr' implementation
	RESHADE_VULKAN_HOOK_PROC(GetInstanceProcAddr);

	if (instance == VK_NULL_HANDLE)
		return nullptr;

	const auto trampoline = g_vulkan_instances.at(dispatch_key_from_handle(instance)).dispatch_table.GetInstanceProcAddr;
	return trampoline(instance, pName);
}

enum VkNegotiateLayerStructType
{
	LAYER_NEGOTIATE_UNINTIALIZED = 0,
	LAYER_NEGOTIATE_INTERFACE_STRUCT = 1,
};

struct VkNegotiateLayerInterface
{
	VkNegotiateLayerStructType sType;
	void *pNext;
	uint32_t loaderLayerInterfaceVersion;
	PFN_vkGetInstanceProcAddr pfnGetInstanceProcAddr;
	PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr;
	PFN_vkGetInstanceProcAddr pfnGetPhysicalDeviceProcAddr;
};

extern "C" VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct)
{
	if (pVersionStruct == nullptr ||
		pVersionStruct->sType != LAYER_NEGOTIATE_INTERFACE_STRUCT)
		return VK_ERROR_INITIALIZATION_FAILED;

	pVersionStruct->loaderLayerInterfaceVersion = 2; // Version 2 added 'vkNegotiateLoaderLayerInterfaceVersion'
	pVersionStruct->pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
	pVersionStruct->pfnGetDeviceProcAddr = vkGetDeviceProcAddr;
	pVersionStruct->pfnGetPhysicalDeviceProcAddr = nullptr;

	return VK_SUCCESS;
}
