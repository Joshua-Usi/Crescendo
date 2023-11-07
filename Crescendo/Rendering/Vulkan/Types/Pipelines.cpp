#include "Pipelines.hpp"
#include "../Device.hpp"

namespace Crescendo::Vulkan
{
	Pipelines::Pipelines(Device& device, std::vector<VkPipeline>& pipelines, std::vector<VkDescriptorSetLayout>& dataLayouts, const std::vector<Set>& setMetadata, const std::vector<cs_std::graphics::Attribute>& vertexAttributes, const PipelineVariants& variants, VkPipelineLayout layout)
		: device(&device), pipelines(std::move(pipelines)), dataLayouts(std::move(dataLayouts)), setMetadata(setMetadata), descriptorSets(this->setMetadata.size()),
		vertexAttributes(vertexAttributes), variants(variants), layout(layout) {}
	Pipelines::~Pipelines()
	{
		for (auto& layout : this->dataLayouts) vkDestroyDescriptorSetLayout(*this->device, layout, nullptr);
		for (auto& pipeline : this->pipelines) vkDestroyPipeline(*this->device, pipeline, nullptr);
		vkDestroyPipelineLayout(*this->device, this->layout, nullptr);
	}
	Pipelines::Pipelines(Pipelines&& other) noexcept
		: device(other.device), pipelines(std::move(other.pipelines)), dataLayouts(std::move(other.dataLayouts)), setMetadata(other.setMetadata), descriptorSets(std::move(other.descriptorSets)), vertexAttributes(other.vertexAttributes), variants(other.variants), layout(other.layout)
	{
		other.pipelines.clear();
		other.dataLayouts.clear();
		other.setMetadata.clear();
		other.vertexAttributes.clear();
		other.descriptorSets.clear();
		other.layout = nullptr;
	}
	Pipelines& Pipelines::operator=(Pipelines&& other) noexcept
	{
		if (this != &other)
		{
			device = other.device;
			pipelines = std::move(other.pipelines); other.pipelines.clear();
			dataLayouts = std::move(other.dataLayouts); other.dataLayouts.clear();
			setMetadata = other.setMetadata; other.setMetadata.clear();
			vertexAttributes = other.vertexAttributes; other.vertexAttributes.clear();
			descriptorSets = std::move(other.descriptorSets); other.descriptorSets.clear();
			variants = other.variants;
			layout = other.layout; other.layout = nullptr;
		}
		return *this;
	}
	uint32_t Pipelines::CreateDescriptorSet(uint32_t set)
	{
		return this->CreateDescriptorSets(set, 1);
	}
	uint32_t Pipelines::CreateDescriptorSets(uint32_t set, uint32_t count)
	{
		uint32_t begin = this->descriptorSets[set].size();
		for (uint32_t i = 0; i < count; i++)
		{
			// Create buffer resources
			// Each set uses 1 buffer, the bindings are offset in this buffer
			VkDescriptorSet descriptorSet = this->device->AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, this->dataLayouts[set]);
			Buffer buffer = this->device->CreateBuffer(this->setMetadata[set].SetSize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

			// Write bindings to set
			for (const auto& binding : this->setMetadata[set].bindings)
			{
				VkDescriptorBufferInfo bufferInfo = Create::DescriptorBufferInfo(buffer, 0, this->setMetadata[set].SetSize());
				VkWriteDescriptorSet descriptorWrite = Create::WriteDescriptorSet(descriptorSet, binding.binding, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, nullptr, &bufferInfo, nullptr);
				this->device->WriteDescriptorSet(descriptorWrite);
			}
			// Emplace new descriptor set
			this->descriptorSets[set].emplace_back(buffer, descriptorSet);
		}
		// Return index of new descriptor set
		return begin;
	}
	void Pipelines::UpdateDescriptorData(uint32_t idx, uint32_t set, uint32_t binding, const void* data, size_t size)
	{
		const uint32_t offset = setMetadata[set].BindingOffset(binding);
		memcpy(static_cast<char*>(descriptorSets[set][idx].buffer.mPtr) + offset, data, size);
	}
}