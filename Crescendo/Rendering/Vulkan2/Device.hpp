#pragma once

#include "common.hpp"
#include "RAII.hpp"
#include "Allocator.hpp"
#include "BindlessDescriptorManager.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Device
	{
	private:
		Vk::Device device;
		Allocator allocator;
		BindlessDescriptorManager descriptorManager;
	public:
		struct DeviceSpecification
		{
			Vk::Device::DeviceCreateInfo deviceCreateInfo;
			BindlessDescriptorManager::BindlessDescriptorManagerSpecification descriptorManagerSpec;
		};
	public:
		Device();
		Device(const Vk::Instance& instance, const Vk::PhysicalDevice& physicalDevice, const DeviceSpecification& spec);
		~Device() = default;
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;
	public:
		Vk::Device::Queue GetUniversalQueue() const;
		Vk::Device::Queue GetTransferQueue() const;
		Vk::Device::Queue GetComputeQueue() const;
	public:
		// General creation functions
		//Vk::Pipeline CreatePipeline(const PipelineBuilderInfo& info);
		//Vk::PipelineLayout CreatePipelineLayout(const std::vector<Vk::DescriptorSetLayout>& descriptorSetLayouts);
		//Vk::RenderPass CreateRenderPass(const Vk::SurfaceFormat& surfaceFormat, VkImageLayout finalLayout);
		//Vk::Framebuffer CreateFramebuffer(const Vk::RenderPass& renderPass, const Vk::Surface& surface, const Vk::ImageView& imageView);
		//Vk::Semaphore CreateSemaphore();
		//Vk::Fence CreateFence();
		Vk::Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		Vk::Image CreateImage(const VkImageCreateInfo& createInfo, VmaMemoryUsage memoryUsage);
		Vk::Sampler CreateSampler(const VkSamplerCreateInfo& createInfo);
		//Vk::ShaderModule CreateShaderModule(const std::vector<char>& code);
		//Vk::ShaderReflection CreateShaderReflection(const std::vector<char>& code);
	public:
		void WaitIdle();
		operator const Vk::Device& () const;
		operator VkDevice() const;
		BindlessDescriptorManager& GetBindlessDescriptorManager();
	};
}