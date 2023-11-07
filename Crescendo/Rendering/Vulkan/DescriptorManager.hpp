#pragma once

#include "volk/volk.h"

#include <vector>

namespace Crescendo::Vulkan
{
	// Automatically creates and allocates pools of descriptor sets
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
		Pool* FindCompatibleAndOpenPool(VkDescriptorType poolType);
		/// <summary>
		/// Allocate a new pool of a specific type, adds it to the pool list and returns a reference to it
		/// </summary>
		/// <param name="poolType">New pools types</param>
		/// <returns>Reference to the allocated pool</returns>
		Pool* AllocatePool(VkDescriptorType poolType);
		/// <summary>
		/// Finds a compatible pool, if one doesn't exist, creates a new one, returns the pool
		/// </summary>
		/// <param name="type">Pool type</param>
		/// <returns>Pointer to the new pool</returns>
		Pool* GetPool(VkDescriptorType type);
	public:
		DescriptorManager() = default;
		DescriptorManager(VkDevice device, uint32_t maxDescriptorsPerPool);
		~DescriptorManager();
		// No copy
		DescriptorManager(const DescriptorManager&) = delete;
		DescriptorManager &operator=(const DescriptorManager&) = delete;
		// Move is allowed
		DescriptorManager(DescriptorManager&& other) noexcept;
		DescriptorManager &operator=(DescriptorManager&& other) noexcept;

		void Destroy();

		/// <summary>
		/// Create one set with the given layout
		/// </summary>
		/// <param name="type">Descriptor type</param>
		/// <param name="layout">Descriptor set layout</param>
		/// <returns></returns>
		VkDescriptorSet AllocateSet(VkDescriptorType type, VkDescriptorSetLayout layout);
		/// <summary>
		/// Create n sets with the given layout, more optimised path than calling AllocateSet n times
		/// </summary>
		/// <param name="type">Descriptor type</param>
		/// <param name="layout">Descriptor set layout</param>
		/// <param name="count">Number of sets to create</param>
		/// <returns></returns>
		std::vector<VkDescriptorSet> AllocateSets(VkDescriptorType type, VkDescriptorSetLayout layout, uint32_t count);
	};
}