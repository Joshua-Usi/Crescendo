#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"
#include <stack>

CS_NAMESPACE_BEGIN::Vulkan
{
	class BindlessDescriptorManager
	{
	public:
		enum class BufferHandle : uint32_t { Invalid = std::numeric_limits<uint32_t>::max() };
		enum class ImageHandle : uint32_t { Invalid = std::numeric_limits<uint32_t>::max() };
		struct BindlessDescriptorManagerSpecification { uint32_t maxBuffers, maxImages; };
	private:
		VkDevice device;
		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;
		VkDescriptorSet set;
		BindlessDescriptorManagerSpecification spec;
		std::vector<bool> areBuffersActive, areImagesActive; // These are used to check if a handle is valid
		std::stack<BufferHandle, std::vector<BufferHandle>> openBuffers;
		std::stack<ImageHandle, std::vector<ImageHandle>> openImages;
	public:
		BindlessDescriptorManager();
		// Number of sets is the absolute upperbound, cannot be changed without switching to another descriptor array or a full restart
		BindlessDescriptorManager(VkDevice device, const BindlessDescriptorManagerSpecification& spec);
		~BindlessDescriptorManager();
		BindlessDescriptorManager(const BindlessDescriptorManager&) = delete;
		BindlessDescriptorManager& operator=(const BindlessDescriptorManager&) = delete;
		BindlessDescriptorManager(BindlessDescriptorManager&& other) noexcept;
		BindlessDescriptorManager& operator=(BindlessDescriptorManager&& other) noexcept;
	public:
		BufferHandle StoreBuffer(VkBuffer buffer);
		ImageHandle StoreImage(VkImageView view, VkSampler sampler);
		void FreeBuffer(BufferHandle handle);
		void FreeImage(ImageHandle handle);
		VkDescriptorSetLayout GetLayout() const;
		VkDescriptorSet GetSet() const;
		uint32_t GetMaxBuffersCount() const;
		uint32_t GetMaxImagesCount() const;
		uint32_t GetUsedBuffersCount() const;
		uint32_t GetUsedImagesCount() const;
	};
}