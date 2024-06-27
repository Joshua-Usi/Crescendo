#include "BindlessDescriptorManager.hpp"
#include "Volk/volk.h"
#include "Create.hpp"
#include <array>

CS_NAMESPACE_BEGIN::Vulkan
{
	constexpr uint32_t UNIFORM_BINDING = 0, BUFFER_BINDING = 1, IMAGE_BINDING = 2;
	BindlessDescriptorManager::BindlessDescriptorManager() : device(nullptr), layout(nullptr), pool(nullptr), set(nullptr), spec({ 0, 0 }) {}
	BindlessDescriptorManager::BindlessDescriptorManager(VkDevice device, const BindlessDescriptorManagerSpecification& spec) : device(device), spec(spec)
	{
		constexpr std::array<VkDescriptorType, 3> types{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		// For simplicity, we treat buffers and uniforms as the same thing, so the true maximum is 2x the maxBuffers
		const std::array<uint32_t, 3> setsSizes = { { spec.maxBuffers, spec.maxBuffers, spec.maxImages } };
		std::array<VkDescriptorSetLayoutBinding, 3> bindings = {};	
		std::array<VkDescriptorBindingFlags, 3> flags = {};
		std::array<VkDescriptorPoolSize, 3> sizes = {};
		for (uint8_t i = 0; i < types.size(); i++)
		{
			bindings[i] = Create::DescriptorSetLayoutBinding(i, types[i], setsSizes[i], VK_SHADER_STAGE_ALL);
			flags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			sizes[i] = { types[i], setsSizes[i] };
		}
		const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = Create::DescriptorSetLayoutBindingFlagsCreateInfo(flags);
		const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = Create::DescriptorSetLayoutCreateInfo(&bindingFlags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, bindings);
		vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &layout);
		// Create the pool
		const VkDescriptorPoolCreateInfo poolInfo = Create::DescriptorPoolCreateInfo(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, 1, sizes);
		vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
		// Create the global set
		const VkDescriptorSetAllocateInfo allocateInfo = Create::DescriptorSetAllocateInfo(pool, &layout);
		vkAllocateDescriptorSets(device, &allocateInfo, &set);

		for (uint32_t i = 0; i < spec.maxBuffers; i++)
		{
			openBuffers.push(static_cast<BufferHandle>(spec.maxBuffers - i - 1));
			areBuffersActive.push_back(false);
		}
		for (uint32_t i = 0; i < spec.maxImages; i++)
		{
			openImages.push(static_cast<ImageHandle>(spec.maxImages - i - 1));
			areImagesActive.push_back(false);
		}
	}
	BindlessDescriptorManager::~BindlessDescriptorManager()
	{
		if (device == nullptr) return;
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
		vkDestroyDescriptorPool(device, pool, nullptr);
	}
	BindlessDescriptorManager::BindlessDescriptorManager(BindlessDescriptorManager&& other) noexcept
		: device(other.device), pool(other.pool), layout(other.layout), set(other.set), spec(other.spec),
		areBuffersActive(std::move(other.areBuffersActive)),
		areImagesActive(std::move(other.areImagesActive)),
		openBuffers(std::move(other.openBuffers)),
		openImages(std::move(other.openImages))
	{
		device = nullptr;
	}
	BindlessDescriptorManager& BindlessDescriptorManager::operator=(BindlessDescriptorManager&& other) noexcept
	{
		if (this != &other)
		{
			device = other.device; other.device = nullptr;
			pool = other.pool;
			layout = other.layout;
			set = other.set;
			spec = other.spec;
			areBuffersActive = std::move(other.areBuffersActive);
			areImagesActive = std::move(other.areImagesActive);
			openBuffers = std::move(other.openBuffers);
			openImages = std::move(other.openImages);
		}
		return *this;
	}
	BindlessDescriptorManager::BufferHandle BindlessDescriptorManager::StoreBuffer(VkBuffer buffer)
	{
		CS_ASSERT(openBuffers.size() > 0, "Buffer handles have been exhausted! Increase the maximum number of storage buffers in the spec");
		BufferHandle handle = openBuffers.top();
		areBuffersActive[static_cast<uint32_t>(handle)] = true;
		openBuffers.pop();

		const VkDescriptorBufferInfo bufferInfo = Create::DescriptorBufferInfo(buffer, 0, VK_WHOLE_SIZE);
		const VkWriteDescriptorSet write = Create::WriteDescriptorSet(set, BUFFER_BINDING, static_cast<uint32_t>(handle), 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &bufferInfo);
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

		return static_cast<BufferHandle>(handle);
	}
	BindlessDescriptorManager::ImageHandle BindlessDescriptorManager::StoreImage(VkImageView view, VkSampler sampler)
	{
		CS_ASSERT(openImages.size() > 0, "Image handles have been exhausted! Increase the maximum number of sampled images in the spec");
		ImageHandle handle = openImages.top();
		areImagesActive[static_cast<uint32_t>(handle)] = true;
		openImages.pop();

		const VkDescriptorImageInfo imageInfo = Create::DescriptorImageInfo(sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		const VkWriteDescriptorSet write = Create::WriteDescriptorSet(set, IMAGE_BINDING, static_cast<uint32_t>(handle), 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo);
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

		return static_cast<ImageHandle>(handle);
	}
	void BindlessDescriptorManager::FreeBuffer(BufferHandle handle)
	{
		CS_ASSERT_WARNING(handle != BindlessDescriptorManager::BufferHandle::Invalid, "Provided buffer handle is not a valid handle");
		CS_ASSERT_WARNING(areBuffersActive[static_cast<uint32_t>(handle)], "Provided buffer handle is not active");
		if (handle != BindlessDescriptorManager::BufferHandle::Invalid && areBuffersActive[static_cast<uint32_t>(handle)])
		{
			openBuffers.push(handle);
			areBuffersActive[static_cast<uint32_t>(handle)] = false;
		}
	}
	void BindlessDescriptorManager::FreeImage(ImageHandle handle)
	{
		CS_ASSERT_WARNING(handle != BindlessDescriptorManager::ImageHandle::Invalid, "Provided image handle is not a valid handle");
		CS_ASSERT_WARNING(areImagesActive[static_cast<uint32_t>(handle)], "Provided image handle is not active");
		if (handle != BindlessDescriptorManager::ImageHandle::Invalid && areImagesActive[static_cast<uint32_t>(handle)])
		{
			openImages.push(handle);
			areImagesActive[static_cast<uint32_t>(handle)] = false;
		}
	}
	VkDescriptorSetLayout BindlessDescriptorManager::GetLayout() const { return layout; }
	VkDescriptorSet BindlessDescriptorManager::GetSet() const { return set; }
	uint32_t BindlessDescriptorManager::GetMaxBuffersCount() const { return spec.maxBuffers; }
	uint32_t BindlessDescriptorManager::GetMaxImagesCount() const { return spec.maxImages; }
	uint32_t BindlessDescriptorManager::GetUsedBuffersCount() const { return spec.maxBuffers - static_cast<uint32_t>(openBuffers.size()); }
	uint32_t BindlessDescriptorManager::GetUsedImagesCount() const { return spec.maxImages - static_cast<uint32_t>(openImages.size()); }
}