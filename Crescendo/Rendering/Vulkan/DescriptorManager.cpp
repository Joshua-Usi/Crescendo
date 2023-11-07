#include "DescriptorManager.hpp"
#include "Core/common.hpp"
#include "Types/Create.hpp"

namespace Crescendo::Vulkan
{
	DescriptorManager::DescriptorManager(VkDevice device, uint32_t maxDescriptorsPerPool) : device(device), maxDescriptorsPerPool(maxDescriptorsPerPool)
	{
		CS_ASSERT(maxDescriptorsPerPool > 0, "Max Descriptors per pool cannot be O, must be at least 1");
	}
	DescriptorManager::~DescriptorManager()
	{
		this->Destroy();
	}
	DescriptorManager::DescriptorManager(DescriptorManager&& other) noexcept : device(other.device), pools(std::move(other.pools)), maxDescriptorsPerPool(other.maxDescriptorsPerPool)
	{
		other.pools.clear();
	}
	DescriptorManager& DescriptorManager::operator=(DescriptorManager&& other) noexcept
	{
		if (this != &other)
		{
			this->device = other.device;
			this->maxDescriptorsPerPool = other.maxDescriptorsPerPool;
			this->pools = std::move(other.pools); other.pools.clear();
		}
		return *this;
	}
	void DescriptorManager::Destroy()
	{
		for (auto& pool : this->pools) vkDestroyDescriptorPool(this->device, pool.pool, nullptr);
		this->pools.clear();
	}
	DescriptorManager::Pool* DescriptorManager::FindCompatibleAndOpenPool(VkDescriptorType poolType)
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
	DescriptorManager::Pool* DescriptorManager::AllocatePool(VkDescriptorType poolType)
	{
		Pool pool(nullptr, poolType, 0);

		const std::vector<VkDescriptorPoolSize> sizes = { { poolType, this->maxDescriptorsPerPool } };
		VkDescriptorPoolCreateInfo poolInfo = Create::DescriptorPoolCreateInfo(0, this->maxDescriptorsPerPool, sizes);
		vkCreateDescriptorPool(this->device, &poolInfo, nullptr, &pool.pool);

		this->pools.push_back(pool);
		return &this->pools.back();
	}
	DescriptorManager::Pool* DescriptorManager::GetPool(VkDescriptorType type)
	{
		Pool* pool = this->FindCompatibleAndOpenPool(type);
		return pool ? pool : this->AllocatePool(type);
	}
	VkDescriptorSet DescriptorManager::AllocateSet(VkDescriptorType type, VkDescriptorSetLayout layout)
	{
		Pool* pool = this->GetPool(type);

		VkDescriptorSetAllocateInfo allocInfo = Create::DescriptorSetAllocateInfo(pool->pool, { layout });
		VkDescriptorSet set;
		vkAllocateDescriptorSets(this->device, &allocInfo, &set);
		pool->descriptorsUsed++;
		return set;
	}
	std::vector<VkDescriptorSet> DescriptorManager::AllocateSets(VkDescriptorType type, VkDescriptorSetLayout layout, uint32_t count)
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
}