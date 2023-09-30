#pragma once

#include "Core/common.hpp"
#include "vulkan/vulkan.h"
#include "Create.hpp"

#include <vector>

namespace Crescendo::internal
{
	/// <summary>
	/// Automatically allocates descriptor pools for required sets
	/// </summary>
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
		/// <summary>
		/// Finds the next compatible pool, if one doesn't exist, returns nullptr
		/// </summary>
		/// <param name="poolType">Type of the pool</param>
		/// <returns>Pointer to the pool</returns>
		inline Pool* FindCompatibleAndOpenPool(VkDescriptorType poolType)
		{
			for (auto& pool : this->pools)
			{
				if (pool.poolType == poolType && pool.descriptorsUsed < this->maxDescriptorsPerPool)
				{
					return &pool;
				}
			}
			// If we didn't find a valid pool, return nullptr
			return nullptr;
		}
		/// <summary>
		/// Allocate a new pool of a specific type, adds it to the pool list and returns a reference to it
		/// </summary>
		/// <param name="poolType">New pools types</param>
		/// <returns>Reference to the allocated pool</returns>
		inline Pool& AllocatePool(VkDescriptorType poolType)
		{
			Pool pool(nullptr, poolType, 0);

			const std::vector<VkDescriptorPoolSize> sizes = { { poolType, this->maxDescriptorsPerPool } };
			VkDescriptorPoolCreateInfo poolInfo = Create::DescriptorPoolCreateInfo(0, this->maxDescriptorsPerPool, sizes);
			vkCreateDescriptorPool(this->device, &poolInfo, nullptr, &pool.pool);

			this->pools.push_back(pool);
			return this->pools.back();
		}
		/// <summary>
		/// Finds a compatible pool, if one doesn't exist, creates a new one, return the pool
		/// </summary>
		/// <param name="type">Pool type</param>
		/// <returns>Pointer to the new pool</returns>
		inline Pool* GetPool(VkDescriptorType type)
		{
			Pool* pool = this->FindCompatibleAndOpenPool(type);
			if (pool == nullptr) pool = &this->AllocatePool(type);

			return pool;
		}
	public:
		DescriptorManager() = default;
		inline DescriptorManager(VkDevice device) : pools({}), device(device), maxDescriptorsPerPool(0) {}
		~DescriptorManager() = default;

		/// <summary>
		/// Initialise the pool manager, required to be called at only once. Set the max number of descriptors here
		/// </summary>
		/// <param name="maxDescriptorsPerPool">Maximum number of descriptors allowed in the pool, cannot be changed</param>
		/// <returns>Reference to self to allow for iniline initialisation</returns>
		inline DescriptorManager& Initialise(uint32_t maxDescriptorsPerPool)
		{
			CS_ASSERT(maxDescriptorsPerPool > 0, "Max Descriptors per pool cannot be O, must be at least 1");
			CS_ASSERT(this->maxDescriptorsPerPool == 0, "Cannot initialise the descriptor manager twice");
			this->maxDescriptorsPerPool = maxDescriptorsPerPool;

			return *this;
		}
		/// <summary>
		/// Destroys the pool manager, required to be called at least once before the program exits
		/// </summary>
		inline void Destroy()
		{
			for (auto& pool : this->pools) vkDestroyDescriptorPool(this->device, pool.pool, nullptr);
		}
	public:
		/// <summary>
		/// Create one set with the given layout
		/// </summary>
		/// <param name="type">Descriptor type</param>
		/// <param name="layout">Descriptor set layout</param>
		/// <returns></returns>
		inline VkDescriptorSet AllocateSet(VkDescriptorType type, VkDescriptorSetLayout layout)
		{
			Pool* pool = this->GetPool(type);

			VkDescriptorSetAllocateInfo allocInfo = Create::DescriptorSetAllocateInfo(pool->pool, { layout });
			VkDescriptorSet set;
			vkAllocateDescriptorSets(this->device, &allocInfo, &set);
			pool->descriptorsUsed++;
			return set;
		}
		/// <summary>
		/// Create n sets with the given layout, more optimised path than calling AllocateSet n times
		/// </summary>
		/// <param name="type">Descriptor type</param>
		/// <param name="layout">Descriptor set layout</param>
		/// <param name="count">Number of sets to create</param>
		/// <returns></returns>
		inline std::vector<VkDescriptorSet> AllocateSets(VkDescriptorType type, VkDescriptorSetLayout layout, uint32_t count)
		{
			uint32_t setsLeft = count;
			std::vector<VkDescriptorSet> sets(count);
			const std::vector<VkDescriptorSetLayout> layouts(count, layout);

			while (setsLeft > 0)
			{
				Pool* pool = this->GetPool(type);

				uint32_t setsToAllocate = std::min(setsLeft, this->maxDescriptorsPerPool - pool->descriptorsUsed);
				setsLeft -= setsToAllocate;

				VkDescriptorSetAllocateInfo allocInfo = Create::DescriptorSetAllocateInfo(pool->pool, layouts);
				vkAllocateDescriptorSets(this->device, &allocInfo, sets.data() + (count - setsLeft));
				pool->descriptorsUsed += setsToAllocate;
			}
			return sets;
		}
	};
}