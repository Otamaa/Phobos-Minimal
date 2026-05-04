/*
 * Copyright (C) 2014 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */
#include <Utilities/Debug.h>

#include "vulkan_hooks.hpp"
#include "vulkan_impl_device.hpp"
#include "vulkan_impl_command_list.hpp"
#include "vulkan_impl_type_convert.hpp"

#include "../../Utils/lockfree_linear_map.hpp"

#include <cstring> // std::memcpy, std::memset
#include <algorithm> // std::copy_n, std::max, std::min, std::swap

extern lockfree_linear_map<void *, reshade::vulkan::device_impl *, 8> g_vulkan_devices;


VkResult VKAPI_CALL vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(BeginCommandBuffer, device_impl);
	const VkResult result = trampoline(commandBuffer, pBeginInfo);

	return result;
}
VkResult VKAPI_CALL vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(EndCommandBuffer, device_impl);
	const VkResult result = trampoline(commandBuffer);

	return result;
}

void VKAPI_CALL vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBindPipeline, device_impl);
	trampoline(commandBuffer, pipelineBindPoint, pipeline);
}

void VKAPI_CALL vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport *pViewports)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdSetViewport, device_impl);
	trampoline(commandBuffer, firstViewport, viewportCount, pViewports);
}
void VKAPI_CALL vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdSetScissor, device_impl);
	trampoline(commandBuffer, firstScissor, scissorCount, pScissors);

}

void VKAPI_CALL vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdSetDepthBias, device_impl);
	trampoline(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);

}
void VKAPI_CALL vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdSetBlendConstants, device_impl);
	trampoline(commandBuffer, blendConstants);

}
void VKAPI_CALL vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdSetStencilCompareMask, device_impl);
	trampoline(commandBuffer, faceMask, compareMask);

}
void VKAPI_CALL vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdSetStencilWriteMask, device_impl);
	trampoline(commandBuffer, faceMask, writeMask);

}
void VKAPI_CALL vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdSetStencilReference, device_impl);
	trampoline(commandBuffer, faceMask, reference);
}

void VKAPI_CALL vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBindDescriptorSets, device_impl);
	trampoline(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void VKAPI_CALL vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBindIndexBuffer, device_impl);
	trampoline(commandBuffer, buffer, offset, indexType);

}
void VKAPI_CALL vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBindVertexBuffers, device_impl);
	trampoline(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

void VKAPI_CALL vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDraw, device_impl);
	trampoline(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
void VKAPI_CALL vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawIndexed, device_impl);
	trampoline(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void VKAPI_CALL vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawIndirect, device_impl);
	trampoline(commandBuffer, buffer, offset, drawCount, stride);
}
void VKAPI_CALL vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawIndexedIndirect, device_impl);
	trampoline(commandBuffer, buffer, offset, drawCount, stride);
}
void VKAPI_CALL vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDispatch, device_impl);
	trampoline(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
void VKAPI_CALL vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));


	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDispatchIndirect, device_impl);
	trampoline(commandBuffer, buffer, offset);
}

void VKAPI_CALL vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy *pRegions)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyBuffer, device_impl);


	trampoline(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}
void VKAPI_CALL vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyImage, device_impl);
	trampoline(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
void VKAPI_CALL vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBlitImage, device_impl);

	trampoline(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}
void VKAPI_CALL vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyBufferToImage, device_impl);

	trampoline(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}
void VKAPI_CALL vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyImageToBuffer, device_impl);

	trampoline(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

void VKAPI_CALL vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdUpdateBuffer, device_impl);

	trampoline(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void VKAPI_CALL vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdClearColorImage, device_impl);
	trampoline(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}
void VKAPI_CALL vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdClearDepthStencilImage, device_impl);
	trampoline(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
void VKAPI_CALL vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdClearAttachments, device_impl);
	trampoline(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void VKAPI_CALL vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdResolveImage, device_impl);
	trampoline(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void VKAPI_CALL vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdPipelineBarrier, device_impl);
	trampoline(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void VKAPI_CALL vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBeginQuery, device_impl);
	trampoline(commandBuffer, queryPool, query, flags);
}
void VKAPI_CALL vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdEndQuery, device_impl);
	trampoline(commandBuffer, queryPool, query);
}
void VKAPI_CALL vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));


	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdWriteTimestamp, device_impl);
	trampoline(commandBuffer, pipelineStage, queryPool, query);
}

void VKAPI_CALL vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));


	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyQueryPoolResults, device_impl);
	trampoline(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

void VKAPI_CALL vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdPushConstants, device_impl);
	trampoline(commandBuffer, layout, stageFlags, offset, size, pValues);

}

void VKAPI_CALL vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));


	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBeginRenderPass, device_impl);
	trampoline(commandBuffer, pRenderPassBegin, contents);
}
void VKAPI_CALL vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));


	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdNextSubpass, device_impl);
	trampoline(commandBuffer, contents);
}
void VKAPI_CALL vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdEndRenderPass, device_impl);
	trampoline(commandBuffer);
}

void VKAPI_CALL vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdExecuteCommands, device_impl);
	trampoline(commandBuffer, commandBufferCount, pCommandBuffers);
}

void VKAPI_CALL vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawIndirectCount, device_impl);
	trampoline(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
void VKAPI_CALL vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawIndexedIndirectCount, device_impl);
	trampoline(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void VKAPI_CALL vkCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassBeginInfo *pSubpassBeginInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBeginRenderPass2, device_impl);
	trampoline(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
void VKAPI_CALL vkCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdNextSubpass2, device_impl);
	trampoline(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
void VKAPI_CALL vkCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdEndRenderPass2, device_impl);
	trampoline(commandBuffer, pSubpassEndInfo);
}

void VKAPI_CALL vkCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdPipelineBarrier2, device_impl);
	trampoline(commandBuffer, pDependencyInfo);

}

void VKAPI_CALL vkCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdWriteTimestamp2, device_impl);
	trampoline(commandBuffer, stage, queryPool, query);
}

void VKAPI_CALL vkCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyBuffer2, device_impl);

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::copy_buffer_region>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

		for (uint32_t i = 0; i < pCopyBufferInfo->regionCount; ++i)
		{
			const VkBufferCopy2 &region = pCopyBufferInfo->pRegions[i];

			if (reshade::invoke_addon_event<reshade::addon_event::copy_buffer_region>(
					cmd_impl,
					reshade::api::resource { (uint64_t)pCopyBufferInfo->srcBuffer }, region.srcOffset,
					reshade::api::resource { (uint64_t)pCopyBufferInfo->dstBuffer }, region.dstOffset, region.size))
				continue;

			// Handle each region separately, so that they can be individually skipped
			VkCopyBufferInfo2 region_info = *pCopyBufferInfo;
			region_info.regionCount = 1;
			region_info.pRegions = &region;

			trampoline(commandBuffer, &region_info);
		}
	}
	else
#endif
	trampoline(commandBuffer, pCopyBufferInfo);
}
void VKAPI_CALL vkCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyImage2, device_impl);

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::copy_texture_region>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

		const bool src_is_3d = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_IMAGE>(pCopyImageInfo->srcImage)->create_info.imageType == VK_IMAGE_TYPE_3D;
		const bool dst_is_3d = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_IMAGE>(pCopyImageInfo->dstImage)->create_info.imageType == VK_IMAGE_TYPE_3D;

		for (uint32_t i = 0; i < pCopyImageInfo->regionCount; ++i)
		{
			const VkImageCopy2 &region = pCopyImageInfo->pRegions[i];

			const reshade::api::subresource_box src_box = {
				static_cast<uint32_t>(region.srcOffset.x),
				static_cast<uint32_t>(region.srcOffset.y),
				static_cast<uint32_t>(region.srcOffset.z),
				static_cast<uint32_t>(region.srcOffset.x) + region.extent.width,
				static_cast<uint32_t>(region.srcOffset.y) + region.extent.height,
				static_cast<uint32_t>(region.srcOffset.z) + (src_is_3d ? region.extent.depth : region.srcSubresource.layerCount)
			};
			const reshade::api::subresource_box dst_box = {
				static_cast<uint32_t>(region.dstOffset.x),
				static_cast<uint32_t>(region.dstOffset.y),
				static_cast<uint32_t>(region.dstOffset.z),
				static_cast<uint32_t>(region.dstOffset.x) + region.extent.width,
				static_cast<uint32_t>(region.dstOffset.y) + region.extent.height,
				static_cast<uint32_t>(region.dstOffset.z) + (dst_is_3d ? region.extent.depth : region.dstSubresource.layerCount)
			};

			if (reshade::invoke_addon_event<reshade::addon_event::copy_texture_region>(
					cmd_impl,
					reshade::api::resource { (uint64_t)pCopyImageInfo->srcImage }, calc_subresource_index(device_impl, pCopyImageInfo->srcImage, region.srcSubresource), &src_box,
					reshade::api::resource { (uint64_t)pCopyImageInfo->dstImage }, calc_subresource_index(device_impl, pCopyImageInfo->dstImage, region.dstSubresource), &dst_box,
					reshade::api::filter_mode::min_mag_mip_point))
				continue;

			// Handle each region separately, so that they can be individually skipped
			VkCopyImageInfo2 region_info = *pCopyImageInfo;
			region_info.regionCount = 1;
			region_info.pRegions = &region;

			trampoline(commandBuffer, &region_info);
		}
	}
	else
#endif
	trampoline(commandBuffer, pCopyImageInfo);
}
void VKAPI_CALL vkCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyBufferToImage2, device_impl);

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::copy_buffer_to_texture>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

		const bool dst_is_3d = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_IMAGE>(pCopyBufferToImageInfo->dstImage)->create_info.imageType == VK_IMAGE_TYPE_3D;

		for (uint32_t i = 0; i < pCopyBufferToImageInfo->regionCount; ++i)
		{
			const VkBufferImageCopy2 &region = pCopyBufferToImageInfo->pRegions[i];

			const reshade::api::subresource_box dst_box = {
				static_cast<uint32_t>(region.imageOffset.x),
				static_cast<uint32_t>(region.imageOffset.y),
				static_cast<uint32_t>(region.imageOffset.z),
				static_cast<uint32_t>(region.imageOffset.x + region.imageExtent.width),
				static_cast<uint32_t>(region.imageOffset.y + region.imageExtent.height),
				static_cast<uint32_t>(region.imageOffset.z) + (dst_is_3d ? region.imageExtent.depth : region.imageSubresource.layerCount)
			};

			if (reshade::invoke_addon_event<reshade::addon_event::copy_buffer_to_texture>(
					cmd_impl,
					reshade::api::resource { (uint64_t)pCopyBufferToImageInfo->srcBuffer }, region.bufferOffset, region.bufferRowLength, region.bufferImageHeight,
					reshade::api::resource { (uint64_t)pCopyBufferToImageInfo->dstImage }, calc_subresource_index(device_impl, pCopyBufferToImageInfo->dstImage, region.imageSubresource), &dst_box))
				continue;

			VkCopyBufferToImageInfo2 region_info = *pCopyBufferToImageInfo;
			region_info.regionCount = 1;
			region_info.pRegions = &region;

			trampoline(commandBuffer, &region_info);
		}
	}
	else
#endif
	trampoline(commandBuffer, pCopyBufferToImageInfo);
}
void VKAPI_CALL vkCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyImageToBuffer2, device_impl);

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::copy_texture_to_buffer>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

		const bool src_is_3d = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_IMAGE>(pCopyImageToBufferInfo->srcImage)->create_info.imageType == VK_IMAGE_TYPE_3D;

		for (uint32_t i = 0; i < pCopyImageToBufferInfo->regionCount; ++i)
		{
			const VkBufferImageCopy2 &region = pCopyImageToBufferInfo->pRegions[i];

			const reshade::api::subresource_box src_box = {
				static_cast<uint32_t>(region.imageOffset.x),
				static_cast<uint32_t>(region.imageOffset.y),
				static_cast<uint32_t>(region.imageOffset.z),
				static_cast<uint32_t>(region.imageOffset.x + region.imageExtent.width),
				static_cast<uint32_t>(region.imageOffset.y + region.imageExtent.height),
				static_cast<uint32_t>(region.imageOffset.z) + (src_is_3d ? region.imageExtent.depth : region.imageSubresource.layerCount)
			};

			if (reshade::invoke_addon_event<reshade::addon_event::copy_texture_to_buffer>(
					cmd_impl,
					reshade::api::resource { (uint64_t)pCopyImageToBufferInfo->srcImage }, calc_subresource_index(device_impl, pCopyImageToBufferInfo->srcImage, region.imageSubresource), &src_box,
					reshade::api::resource { (uint64_t)pCopyImageToBufferInfo->dstBuffer }, region.bufferOffset, region.bufferRowLength, region.bufferImageHeight))
				continue;

			VkCopyImageToBufferInfo2 region_info = *pCopyImageToBufferInfo;
			region_info.regionCount = 1;
			region_info.pRegions = &region;

			trampoline(commandBuffer, &region_info);
		}
	}
	else
#endif
	trampoline(commandBuffer, pCopyImageToBufferInfo);
}
void VKAPI_CALL vkCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBlitImage2, device_impl);

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::copy_texture_region>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

		const bool src_is_3d = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_IMAGE>(pBlitImageInfo->srcImage)->create_info.imageType == VK_IMAGE_TYPE_3D;
		const bool dst_is_3d = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_IMAGE>(pBlitImageInfo->dstImage)->create_info.imageType == VK_IMAGE_TYPE_3D;

		for (uint32_t i = 0; i < pBlitImageInfo->regionCount; ++i)
		{
			const VkImageBlit2 &region = pBlitImageInfo->pRegions[i];

			const reshade::api::subresource_box src_box = {
				static_cast<uint32_t>(region.srcOffsets[0].x),
				static_cast<uint32_t>(region.srcOffsets[0].y),
				static_cast<uint32_t>(region.srcOffsets[0].z),
				static_cast<uint32_t>(region.srcOffsets[1].x),
				static_cast<uint32_t>(region.srcOffsets[1].y),
				src_is_3d ? static_cast<uint32_t>(region.srcOffsets[1].z) : static_cast<uint32_t>(region.srcOffsets[0].z) + region.srcSubresource.layerCount
			};
			const reshade::api::subresource_box dst_box = {
				static_cast<uint32_t>(region.dstOffsets[0].x),
				static_cast<uint32_t>(region.dstOffsets[0].y),
				static_cast<uint32_t>(region.dstOffsets[0].z),
				static_cast<uint32_t>(region.dstOffsets[1].x),
				static_cast<uint32_t>(region.dstOffsets[1].y),
				dst_is_3d ? static_cast<uint32_t>(region.dstOffsets[1].z) : static_cast<uint32_t>(region.dstOffsets[0].z) + region.dstSubresource.layerCount
			};

			if (reshade::invoke_addon_event<reshade::addon_event::copy_texture_region>(
					cmd_impl,
					reshade::api::resource { (uint64_t)pBlitImageInfo->srcImage }, calc_subresource_index(device_impl, pBlitImageInfo->srcImage, region.srcSubresource), &src_box,
					reshade::api::resource { (uint64_t)pBlitImageInfo->dstImage }, calc_subresource_index(device_impl, pBlitImageInfo->dstImage, region.dstSubresource), &dst_box,
					pBlitImageInfo->filter == VK_FILTER_NEAREST ? reshade::api::filter_mode::min_mag_mip_point : reshade::api::filter_mode::min_mag_mip_linear))
				continue;

			VkBlitImageInfo2 region_info = *pBlitImageInfo;
			region_info.regionCount = 1;
			region_info.pRegions = &region;

			trampoline(commandBuffer, &region_info);
		}
	}
	else
#endif
	trampoline(commandBuffer, pBlitImageInfo);
}
void VKAPI_CALL vkCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdResolveImage2, device_impl);

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::resolve_texture_region>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

		const bool src_is_3d = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_IMAGE>(pResolveImageInfo->srcImage)->create_info.imageType == VK_IMAGE_TYPE_3D;

		for (uint32_t i = 0; i < pResolveImageInfo->regionCount; ++i)
		{
			const VkImageResolve2 &region = pResolveImageInfo->pRegions[i];

			const reshade::api::subresource_box src_box = {
				static_cast<uint32_t>(region.srcOffset.x),
				static_cast<uint32_t>(region.srcOffset.y),
				static_cast<uint32_t>(region.srcOffset.z),
				static_cast<uint32_t>(region.srcOffset.x + region.extent.width),
				static_cast<uint32_t>(region.srcOffset.y + region.extent.height),
				static_cast<uint32_t>(region.srcOffset.z) + (src_is_3d ? region.extent.depth : region.srcSubresource.layerCount)
			};

			if (reshade::invoke_addon_event<reshade::addon_event::resolve_texture_region>(
					cmd_impl,
					reshade::api::resource { (uint64_t)pResolveImageInfo->srcImage },
					calc_subresource_index(device_impl, pResolveImageInfo->srcImage, region.srcSubresource),
					&src_box,
					reshade::api::resource { (uint64_t)pResolveImageInfo->dstImage },
					calc_subresource_index(device_impl, pResolveImageInfo->dstImage, region.dstSubresource),
					static_cast<uint32_t>(region.dstOffset.x),
					static_cast<uint32_t>(region.dstOffset.y),
					static_cast<uint32_t>(region.dstOffset.z),
					reshade::api::format::unknown))
				continue;

			VkResolveImageInfo2 region_info = *pResolveImageInfo;
			region_info.regionCount = 1;
			region_info.pRegions = &region;

			trampoline(commandBuffer, &region_info);
		}
	}
	else
#endif
	trampoline(commandBuffer, pResolveImageInfo);
}

void VKAPI_CALL vkCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	const auto cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	assert(!cmd_impl->_is_in_render_pass);
	assert(pRenderingInfo != nullptr);
	assert(pRenderingInfo->colorAttachmentCount <= 8);

	// Update current attachments on the command list
	for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount && i < 8; ++i)
		cmd_impl->current_color_attachments[i] = pRenderingInfo->pColorAttachments[i].imageView;

	if (pRenderingInfo->pDepthAttachment != nullptr)
		cmd_impl->current_depth_stencil_attachment = pRenderingInfo->pDepthAttachment->imageView;
	else if (pRenderingInfo->pStencilAttachment != nullptr)
		cmd_impl->current_depth_stencil_attachment = pRenderingInfo->pStencilAttachment->imageView;

	cmd_impl->_is_in_render_pass = 3;

	temp_mem<reshade::api::render_pass_render_target_desc, 8> rts(pRenderingInfo->colorAttachmentCount);
	for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i)
		rts[i] = reshade::vulkan::convert_render_pass_render_target_desc(pRenderingInfo->pColorAttachments + i);

	reshade::api::render_pass_depth_stencil_desc ds;
	if (pRenderingInfo->pDepthAttachment != nullptr || pRenderingInfo->pStencilAttachment != nullptr)
		ds = reshade::vulkan::convert_render_pass_depth_stencil_desc(pRenderingInfo->pDepthAttachment, pRenderingInfo->pStencilAttachment);

	if (reshade::invoke_addon_event<reshade::addon_event::begin_render_pass>(
			cmd_impl,
			pRenderingInfo->colorAttachmentCount, rts.p,
			pRenderingInfo->pDepthAttachment != nullptr || pRenderingInfo->pStencilAttachment != nullptr ? &ds : nullptr,
			reshade::vulkan::convert_render_pass_flags(pRenderingInfo->flags)))
		return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBeginRendering, device_impl);
	trampoline(commandBuffer, pRenderingInfo);
}
void VKAPI_CALL vkCmdEndRendering(VkCommandBuffer commandBuffer)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	const auto cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	assert(cmd_impl->_is_in_render_pass == 3 || cmd_impl->_is_in_render_pass & 0x80);

	const bool skip_end_render_pass = reshade::invoke_addon_event<reshade::addon_event::end_render_pass>(cmd_impl);

	std::memset(cmd_impl->current_color_attachments, 0, sizeof(cmd_impl->current_color_attachments));
	cmd_impl->current_depth_stencil_attachment = VK_NULL_HANDLE;

	if (skip_end_render_pass)
	{
		assert(!cmd_impl->_is_in_render_pass);
		return;
	}
	if (cmd_impl->_is_in_render_pass & 0x80)
	{
		// Render pass was overriden by an add-on, so need to end it the same way
		cmd_impl->end_render_pass();
		return;
	}
	else
	{
		cmd_impl->_is_in_render_pass = 0;
	}
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdEndRendering, device_impl);
	trampoline(commandBuffer);
}

void VKAPI_CALL vkCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const VkDeviceSize *pStrides)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBindVertexBuffers2, device_impl);
	trampoline(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);

#if RESHADE_ADDON >= 2
	if (!reshade::has_addon_event<reshade::addon_event::bind_vertex_buffers>())
		return;

	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	temp_mem<uint32_t> strides_32(bindingCount);
	for (uint32_t i = 0; i < bindingCount; ++i)
	{
		assert(pStrides[i] <= std::numeric_limits<uint32_t>::max());
		strides_32[i] = static_cast<uint32_t>(pStrides[i]);
	}

	reshade::invoke_addon_event<reshade::addon_event::bind_vertex_buffers>(
		cmd_impl, firstBinding, bindingCount, reinterpret_cast<const reshade::api::resource *>(pBuffers), pOffsets, strides_32.p);
#endif
}

void VKAPI_CALL vkCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdPushDescriptorSet, device_impl);
	trampoline(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);

#if RESHADE_ADDON >= 2
	if (!reshade::has_addon_event<reshade::addon_event::push_descriptors>())
		return;

	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	uint32_t max_descriptors = 0;
	for (uint32_t i = 0; i < descriptorWriteCount; ++i)
		max_descriptors = std::max(max_descriptors, pDescriptorWrites[i].descriptorCount);
	temp_mem<uint64_t> descriptors(max_descriptors * 2);

	const auto shader_stages = reshade::vulkan::convert_shader_stages(pipelineBindPoint);

	for (uint32_t i = 0, j = 0; i < descriptorWriteCount; ++i, j = 0)
	{
		const VkWriteDescriptorSet &write = pDescriptorWrites[i];

		reshade::api::descriptor_table_update update;
		update.table = { 0 };
		update.binding = write.dstBinding;
		update.array_offset = write.dstArrayElement;
		update.count = write.descriptorCount;
		update.type = reshade::vulkan::convert_descriptor_type(write.descriptorType);
		update.descriptors = descriptors.p;

		switch (write.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			for (uint32_t k = 0; k < write.descriptorCount; ++k, ++j)
				descriptors[j] = (uint64_t)write.pImageInfo[k].sampler;
			break;
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			for (uint32_t k = 0; k < write.descriptorCount; ++k, j += 2)
				descriptors[j + 0] = (uint64_t)write.pImageInfo[k].sampler,
				descriptors[j + 1] = (uint64_t)write.pImageInfo[k].imageView;
			break;
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			for (uint32_t k = 0; k < write.descriptorCount; ++k, ++j)
				descriptors[j] = (uint64_t)write.pImageInfo[k].imageView;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			for (uint32_t k = 0; k < write.descriptorCount; ++k, ++j)
				descriptors[j] = (uint64_t)write.pTexelBufferView[k];
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			static_assert(sizeof(reshade::api::buffer_range) == sizeof(VkDescriptorBufferInfo));
			update.descriptors = write.pBufferInfo;
			break;
#if VK_KHR_acceleration_structure
		case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
			if (const auto write_acceleration_structure =
					find_in_structure_chain<VkWriteDescriptorSetAccelerationStructureKHR>(
						write.pNext, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR))
			{
				assert(update.count == write_acceleration_structure->accelerationStructureCount);
				update.descriptors = write_acceleration_structure->pAccelerationStructures;
				break;
			}
			[[fallthrough]];
#endif
		default:
			update.count = 0;
			update.descriptors = nullptr;
			break;
		}

		reshade::invoke_addon_event<reshade::addon_event::push_descriptors>(
			cmd_impl,
			shader_stages,
			reshade::api::pipeline_layout { (uint64_t)layout },
			set,
			update);
	}
#endif
}
void VKAPI_CALL vkCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void *pData)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdPushDescriptorSetWithTemplate, device_impl);
	trampoline(commandBuffer, descriptorUpdateTemplate, layout, set, pData);

#if RESHADE_ADDON >= 2
	if (!reshade::has_addon_event<reshade::addon_event::push_descriptors>())
		return;

	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	const auto template_data = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE>(descriptorUpdateTemplate);

	uint32_t max_descriptors = 0;
	for (const VkDescriptorUpdateTemplateEntry &entry : template_data->entries)
		max_descriptors = std::max(max_descriptors, entry.descriptorCount);
	temp_mem<uint64_t> descriptors(max_descriptors * 2);

	const auto shader_stages = reshade::vulkan::convert_shader_stages(template_data->bind_point);

	for (uint32_t i = 0, j = 0; i < static_cast<uint32_t>(template_data->entries.size()); ++i, j = 0)
	{
		const VkDescriptorUpdateTemplateEntry &entry = template_data->entries[i];

		reshade::api::descriptor_table_update update;
		update.table = { 0 };
		update.binding = entry.dstBinding;
		update.array_offset = entry.dstArrayElement;
		update.count = entry.descriptorCount;
		update.type = reshade::vulkan::convert_descriptor_type(entry.descriptorType);
		update.descriptors = descriptors.p;

		const void *const base = static_cast<const uint8_t *>(pData) + entry.offset;

		switch (entry.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			for (uint32_t k = 0; k < entry.descriptorCount; ++k, ++j)
				descriptors[j] = (uint64_t)static_cast<const VkDescriptorImageInfo *>(base)[k].sampler;
			break;
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			for (uint32_t k = 0; k < entry.descriptorCount; ++k, j += 2)
				descriptors[j + 0] = (uint64_t)static_cast<const VkDescriptorImageInfo *>(base)[k].sampler,
				descriptors[j + 1] = (uint64_t)static_cast<const VkDescriptorImageInfo *>(base)[k].imageView;
			break;
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			for (uint32_t k = 0; k < entry.descriptorCount; ++k, ++j)
				descriptors[j] = (uint64_t)static_cast<const VkDescriptorImageInfo *>(base)[k].imageView;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			for (uint32_t k = 0; k < entry.descriptorCount; ++k, ++j)
				descriptors[j] = (uint64_t)static_cast<const VkBufferView *>(base)[k];
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			update.descriptors = static_cast<const VkDescriptorBufferInfo *>(base);
			break;
#if VK_KHR_acceleration_structure
		case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
			update.descriptors = static_cast<const VkAccelerationStructureKHR *>(base);
			break;
#endif
		default:
			update.count = 0;
			update.descriptors = nullptr;
			break;
		}

		reshade::invoke_addon_event<reshade::addon_event::push_descriptors>(
			cmd_impl,
			shader_stages,
			reshade::api::pipeline_layout { (uint64_t)layout },
			set,
			update);
	}
#endif
}

#if VK_EXT_transform_feedback
void VKAPI_CALL vkCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBindTransformFeedbackBuffersEXT, device_impl);
	trampoline(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);

#if RESHADE_ADDON >= 2
	if (!reshade::has_addon_event<reshade::addon_event::bind_stream_output_buffers>())
		return;

	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	reshade::invoke_addon_event<reshade::addon_event::bind_stream_output_buffers>(
		cmd_impl, firstBinding, bindingCount, reinterpret_cast<const reshade::api::resource *>(pBuffers), pOffsets, pSizes, nullptr, nullptr);
#endif
}

void VKAPI_CALL vkCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::begin_query>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);
		const auto pool_data = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_QUERY_POOL>(queryPool);

		if (reshade::invoke_addon_event<reshade::addon_event::begin_query>(cmd_impl, reshade::api::query_heap { (uint64_t)queryPool }, reshade::vulkan::convert_query_type(pool_data->type, index), query))
			return;
	}
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBeginQueryIndexedEXT, device_impl);
	trampoline(commandBuffer, queryPool, query, flags, index);
}
void VKAPI_CALL vkCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::end_query>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);
		const auto pool_data = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_QUERY_POOL>(queryPool);

		if (reshade::invoke_addon_event<reshade::addon_event::end_query>(cmd_impl, reshade::api::query_heap { (uint64_t)queryPool }, reshade::vulkan::convert_query_type(pool_data->type, index), query))
			return;
	}
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdEndQueryIndexedEXT, device_impl);
	trampoline(commandBuffer, queryPool, query, index);
}
#endif

#if VK_EXT_multi_draw
void VKAPI_CALL vkCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	for (uint32_t i = 0; i < drawCount; ++i)
		if (reshade::invoke_addon_event<reshade::addon_event::draw>(cmd_impl, pVertexInfo[i].vertexCount, instanceCount, pVertexInfo[i].firstVertex, firstInstance))
			return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawMultiEXT, device_impl);
	trampoline(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}
void VKAPI_CALL vkCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	for (uint32_t i = 0; i < drawCount; ++i)
		if (reshade::invoke_addon_event<reshade::addon_event::draw_indexed>(cmd_impl, pIndexInfo[i].indexCount, instanceCount, pIndexInfo[i].firstIndex, pVertexOffset != nullptr ? *pVertexOffset : pIndexInfo[i].vertexOffset, firstInstance))
			return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawMultiIndexedEXT, device_impl);
	trampoline(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}
#endif

#if VK_KHR_acceleration_structure
void VKAPI_CALL vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos)
{
	assert(pInfos != nullptr && ppBuildRangeInfos != nullptr);

	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));
	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBuildAccelerationStructuresKHR, device_impl);

#if RESHADE_ADDON >= 2
	if (reshade::has_addon_event<reshade::addon_event::build_acceleration_structure>())
	{
		reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

		for (uint32_t i = 0; i < infoCount; ++i)
		{
			const VkAccelerationStructureBuildGeometryInfoKHR &info = pInfos[i];
			const VkAccelerationStructureBuildRangeInfoKHR *const range_infos = ppBuildRangeInfos[i];

			std::vector<reshade::api::acceleration_structure_build_input> build_inputs;
			build_inputs.reserve(info.geometryCount);
			for (uint32_t k = 0; k < info.geometryCount; ++k)
				build_inputs.push_back(reshade::vulkan::convert_acceleration_structure_build_input(info.ppGeometries != nullptr ? *info.ppGeometries[k] : info.pGeometries[k], range_infos[k]));

			if (reshade::invoke_addon_event<reshade::addon_event::build_acceleration_structure>(
					cmd_impl,
					reshade::vulkan::convert_acceleration_structure_type(info.type),
					reshade::vulkan::convert_acceleration_structure_build_flags(info.flags),
					static_cast<uint32_t>(build_inputs.size()),
					build_inputs.data(),
					reshade::api::resource {},
					info.scratchData.deviceAddress,
					reshade::api::resource_view { (uint64_t)info.srcAccelerationStructure },
					reshade::api::resource_view { (uint64_t)info.dstAccelerationStructure },
					static_cast<reshade::api::acceleration_structure_build_mode>(info.mode)))
				continue;

			// Handle each build info separately, so that they can be individually skipped
			trampoline(commandBuffer, 1, &info, &range_infos);
		}
	}
	else
#endif
	trampoline(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}
void VKAPI_CALL vkCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides, const uint32_t *const *ppMaxPrimitiveCounts)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdBuildAccelerationStructuresIndirectKHR, device_impl);
	trampoline(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}

void VKAPI_CALL vkCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo)
{
	assert(pInfo != nullptr);

	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON >= 2
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	if (reshade::invoke_addon_event<reshade::addon_event::copy_acceleration_structure>(
			cmd_impl,
			reshade::api::resource_view { (uint64_t)pInfo->src },
			reshade::api::resource_view { (uint64_t)pInfo->dst },
			reshade::vulkan::convert_acceleration_structure_copy_mode(pInfo->mode)))
		return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdCopyAccelerationStructureKHR, device_impl);
	trampoline(commandBuffer, pInfo);
}
void VKAPI_CALL vkCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON >= 2
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	if (reshade::invoke_addon_event<reshade::addon_event::query_acceleration_structures>(
			cmd_impl,
			accelerationStructureCount,
			reinterpret_cast<const reshade::api::resource_view *>(pAccelerationStructures),
			reshade::api::query_heap { (uint64_t)queryPool },
			reshade::vulkan::convert_query_type(queryType),
			firstQuery))
		return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdWriteAccelerationStructuresPropertiesKHR, device_impl);
	trampoline(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}
#endif

#if VK_KHR_ray_tracing_pipeline
void VKAPI_CALL vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth)
{
	assert(pRaygenShaderBindingTable != nullptr && pMissShaderBindingTable != nullptr && pHitShaderBindingTable != nullptr && pCallableShaderBindingTable != nullptr);

	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	if (reshade::invoke_addon_event<reshade::addon_event::dispatch_rays>(
			cmd_impl,
			reshade::api::resource {},
			pRaygenShaderBindingTable->deviceAddress,
			pRaygenShaderBindingTable->size,
			reshade::api::resource {},
			pMissShaderBindingTable->deviceAddress,
			pMissShaderBindingTable->size,
			pMissShaderBindingTable->stride,
			reshade::api::resource {},
			pHitShaderBindingTable->deviceAddress,
			pHitShaderBindingTable->size,
			pHitShaderBindingTable->stride,
			reshade::api::resource {},
			pCallableShaderBindingTable->deviceAddress,
			pCallableShaderBindingTable->size,
			pCallableShaderBindingTable->stride,
			width, height, depth))
		return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdTraceRaysKHR, device_impl);
	trampoline(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
void VKAPI_CALL vkCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdTraceRaysIndirectKHR, device_impl);
	trampoline(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}
#endif
#if VK_KHR_ray_tracing_maintenance1
void VKAPI_CALL vkCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	if (reshade::invoke_addon_event<reshade::addon_event::draw_or_dispatch_indirect>(cmd_impl, reshade::api::indirect_command::dispatch_rays, reshade::api::resource {}, indirectDeviceAddress, 1, 0))
		return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdTraceRaysIndirect2KHR, device_impl);
	trampoline(commandBuffer, indirectDeviceAddress);
}
#endif

#if VK_EXT_mesh_shader
void VKAPI_CALL vkCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	if (reshade::invoke_addon_event<reshade::addon_event::dispatch_mesh>(cmd_impl, groupCountX, groupCountY, groupCountZ))
		return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawMeshTasksEXT, device_impl);
	trampoline(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
void VKAPI_CALL vkCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	if (reshade::invoke_addon_event<reshade::addon_event::draw_or_dispatch_indirect>(cmd_impl, reshade::api::indirect_command::dispatch_mesh, reshade::api::resource { (uint64_t)buffer }, offset, drawCount, stride))
		return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawMeshTasksIndirectEXT, device_impl);
	trampoline(commandBuffer, buffer, offset, drawCount, stride);
}
void VKAPI_CALL vkCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
{
	reshade::vulkan::device_impl *const device_impl = g_vulkan_devices.at(dispatch_key_from_handle(commandBuffer));

#if RESHADE_ADDON
	reshade::vulkan::command_list_impl *const cmd_impl = device_impl->get_private_data_for_object<VK_OBJECT_TYPE_COMMAND_BUFFER>(commandBuffer);

	if (reshade::invoke_addon_event<reshade::addon_event::draw_or_dispatch_indirect>(cmd_impl, reshade::api::indirect_command::dispatch_mesh, reshade::api::resource { (uint64_t)buffer }, offset, maxDrawCount, stride))
		return;
#endif

	RESHADE_VULKAN_GET_DEVICE_DISPATCH_PTR(CmdDrawMeshTasksIndirectCountEXT, device_impl);
	trampoline(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
#endif
