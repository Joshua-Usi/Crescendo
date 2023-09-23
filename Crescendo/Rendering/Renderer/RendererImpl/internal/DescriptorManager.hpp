#pragma once

#include "Core/common.hpp"
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
			VkDescriptorType poolType;
			uint32_t descriptorsUsed;
		};
		std::vector<Pool> pools;
		VkDevice device;
		// This value is designed NOT to change once it is set
		uint32_t maxDescriptorsPerPool;
	private:
		inline Pool* FindCompatibleAndOpenPool(VkDescriptorType poolType)
		{
			for (auto& pool : this->pools)
			{
				if (pool.poolType == poolType && pool.descriptorsUsed < this->maxDescriptorsPerPool - 1)
				{
					return &pool;
				}
			}
			// If we didn't find a valid pool, return nullptr
			return nullptr;
		}
		inline Pool& AllocatePool(VkDescriptorType poolType)
		{
			Pool pool(nullptr, poolType, 0);

			const std::vector<VkDescriptorPoolSize> sizes = { { poolType, this->maxDescriptorsPerPool } };
			VkDescriptorPoolCreateInfo poolInfo = Create::DescriptorPoolCreateInfo(0, this->maxDescriptorsPerPool, sizes);
			vkCreateDescriptorPool(this->device, &poolInfo, nullptr, &pool.pool);

			this->pools.push_back(pool);
			return this->pools.back();
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
		inline VkDescriptorSet AllocateSet(VkDescriptorType type, VkDescriptorSetLayout layout)
		{
			Pool* pool = this->FindCompatibleAndOpenPool(type);
			if (pool == nullptr) pool = &this->AllocatePool(type);

			VkDescriptorSetAllocateInfo allocInfo = Create::DescriptorSetAllocateInfo(pool->pool, { layout });
			VkDescriptorSet set;
			vkAllocateDescriptorSets(this->device, &allocInfo, &set);
			pool->descriptorsUsed++;
			return set;
		}
	};
}