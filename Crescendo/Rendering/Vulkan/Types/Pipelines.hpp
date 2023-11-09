#pragma once

#include "Volk/volk.h"

#include <cstdint>
#include <vector>

#include "cs_std/graphics/model.hpp"

#include "Buffer.hpp"
#include "Types.hpp"
#include "PipelineVariants.hpp"

namespace Crescendo::Vulkan
{
	class Device;

	// Stores a set of pipelines
	struct Pipelines
	{
	public:
		// Minimal descriptor set metadata
		struct Set
		{
			struct Binding { uint32_t binding, size; };
			std::vector<Binding> bindings;
			uint32_t set;
			// Get the size of the set in bytes
			uint32_t SetSize() const
			{
				uint32_t size = 0;
				for (const auto& binding : bindings) size += binding.size;
				return size;
			}
			uint32_t BindingOffset(uint32_t binding) const
			{
				uint32_t offset = 0;
				for (uint32_t i = 0; i < binding; i++) offset += bindings[i].size;
				return offset;
			}
		};
		struct DescriptorSet
		{
			Crescendo::Vulkan::Buffer buffer;
			VkDescriptorSet set;

			DescriptorSet() = default;
			DescriptorSet(Crescendo::Vulkan::Buffer& buffer, VkDescriptorSet set) : buffer(std::move(buffer)), set(set) {}
			~DescriptorSet() = default;
			// No copy
			DescriptorSet(const DescriptorSet&) = delete;
			DescriptorSet& operator=(const DescriptorSet&) = delete;
			// Move
			DescriptorSet(DescriptorSet&& other) noexcept : buffer(std::move(other.buffer)), set(other.set) { other.set = nullptr; }
			DescriptorSet& operator=(DescriptorSet&& other) noexcept { if (this != &other) { buffer = std::move(other.buffer); set = other.set; other.set = nullptr; } return *this; }
		};
	public:
		std::vector<VkPipeline> pipelines;
		std::vector<VkDescriptorSetLayout> dataLayouts;
		// Metadata requires to build each of the sets in this pipeline
		std::vector<Set> setMetadata;
		std::vector<std::vector<DescriptorSet>> descriptorSets;
		std::vector<cs_std::graphics::Attribute> vertexAttributes;
		PipelineVariants variants;
		Device* device; // Pointer to device, makes it easier to handle descriptor sets
		VkPipelineLayout layout;
	public:
		Pipelines() = default;
		Pipelines(Device& device,
			std::vector<VkPipeline>& pipelines, std::vector<VkDescriptorSetLayout>& dataLayouts,
			const std::vector<Set>& setMetadata, const std::vector<cs_std::graphics::Attribute>& vertexAttributes,
			const PipelineVariants& variants, VkPipelineLayout layout);
		~Pipelines();
		// No copy
		Pipelines(const Pipelines&) = delete;
		Pipelines& operator=(const Pipelines&) = delete;
		// Move
		Pipelines(Pipelines&& other) noexcept;
		Pipelines& operator=(Pipelines&& other) noexcept;
	public:
		uint32_t CreateDescriptorSet(uint32_t set);
		// Create multiple descriptor sets, Returns the index of the first set
		uint32_t CreateDescriptorSets(uint32_t set, uint32_t count);
		void UpdateDescriptorData(uint32_t idx, uint32_t set, uint32_t binding, const void* data, size_t size);
		template<typename T> void UpdateDescriptorData(uint32_t idx, uint32_t set, uint32_t binding, const T& data) { UpdateDescriptorData(idx, set, binding, &data, sizeof(T)); }
		std::vector<VkBuffer> GetMatchingBuffers(const Crescendo::Vulkan::Mesh& mesh) const;
	public:
		operator VkPipelineLayout() const { return layout; }
		VkPipeline operator [](uint32_t index) const { return pipelines[index]; }
	};
}