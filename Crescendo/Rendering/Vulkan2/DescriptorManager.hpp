#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"
#include <vector>

CS_NAMESPACE_BEGIN::Vulkan
{
	class DescriptorManager
	{
	private:
		struct Pool
		{
			VkDescriptorPool pool;
			// Pools only contain one type of descriptor
			VkDescriptorType poolType;
			uint32_t descriptorsUsed;
		};
		std::vector<Pool> pools;
		VkDevice device;
		// This value is designed NOT to change once it is set
		uint32_t maxDescriptorsPerPool;
	private:
		Pool* FindCompatibleAndOpenPool(VkDescriptorType poolType);
		Pool* AllocatePool(VkDescriptorType poolType);
		Pool* GetPool(VkDescriptorType type);
	public:
		DescriptorManager();
		DescriptorManager(VkDevice device, uint32_t maxDescriptorsPerPool);
		~DescriptorManager();
		DescriptorManager(const DescriptorManager&) = delete;
		DescriptorManager& operator=(const DescriptorManager&) = delete;
		DescriptorManager(DescriptorManager&& other) noexcept;
		DescriptorManager& operator=(DescriptorManager&& other) noexcept;
		VkDescriptorSet AllocateSet(VkDescriptorType type, VkDescriptorSetLayout layout);
		std::vector<VkDescriptorSet> AllocateSets(VkDescriptorType type, VkDescriptorSetLayout layout, uint32_t count);
	};
}