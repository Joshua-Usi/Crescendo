#pragma once

#include "vulkan/vulkan.h"

#include "Create.hpp"

#include <vector>

namespace Crescendo::internal
{
	class DescriptorManager
	{
	private:
		struct Pool
		{
			VkDescriptorPool pool;
			uint32_t descriptorsUsed;
			VkDescriptorType poolType;
		};
		std::vector<Pool> pools;
		VkDevice device;
		// This value is designed NOT to change once it is set
		uint32_t maxDescriptorsPerPool;
	private:
		inline VkDescriptorPool FindCompatibleAndOpenPool(VkDescriptorType poolType)
		{
			for (const auto& pool : this->pools)
			{
				if (pool.poolType == poolType && pool.descriptorsUsed < this->maxDescriptorsPerPool)
				{
					return pool.pool;
				}
			}
			// If we didn't find a valid pool, return nullptr
			return nullptr;
		}
		inline VkDescriptorPool AllocatePool(VkDescriptorType poolType)
		{
			Pool pool = {};

			const std::vector<VkDescriptorPoolSize> sizes = { { poolType, this->maxDescriptorsPerPool } };
			VkDescriptorPoolCreateInfo poolInfo = Create::DescriptorPoolCreateInfo(0, this->maxDescriptorsPerPool, sizes);
			vkCreateDescriptorPool(this->device, &poolInfo, nullptr, &pool.pool);
			
			this->pools.push_back(pool);

			return pool.pool;
		}
	public:
		DescriptorManager() = default;
		inline DescriptorManager(VkDevice device) : pools({}), device(device), maxDescriptorsPerPool(0) {}
		~DescriptorManager() = default;

		inline DescriptorManager& Initialise(uint32_t maxDescriptorsPerPool)
		{
			CS_ASSERT(maxDescriptorsPerPool > 0, "Max Descriptors per pool cannot be O, must be at least 1");
			this->maxDescriptorsPerPool = maxDescriptorsPerPool;
			return *this;
		}
		inline void Destroy()
		{
			for (auto& pool : this->pools) vkDestroyDescriptorPool(this->device, pool.pool, nullptr);
		}
	public:
		inline VkDescriptorSet AllocateSet(VkDescriptorType type, const std::vector<VkDescriptorSetLayout>& layouts)
		{
			VkDescriptorPool pool = this->FindCompatibleAndOpenPool(type);
			if (pool == nullptr) pool = this->AllocatePool(type);

			VkDescriptorSetAllocateInfo allocInfo = Create::DescriptorSetAllocateInfo(pool, layouts);
			VkDescriptorSet set;
			vkAllocateDescriptorSets(this->device, &allocInfo, &set);
			return set;
		}
	};
}