#pragma once

#include "common.hpp"
#include "VkBootstrap/VkBootstrap.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{	
	class Instance
	{
	private:
		vkb::Instance vkbInstance;
	public:
		struct InstanceSpecification
		{
			bool useValidationLayers;
			struct Version { uint32_t major, minor, patch; } version;
			std::string appName;
			std::string engineName;
		};
	public:
		Instance();
		Instance(const InstanceSpecification& spec);
		~Instance();
		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
		Instance(Instance&& other) noexcept;
		Instance& operator=(Instance&& other) noexcept;
	public:
		operator VkInstance() const;
		operator const vkb::Instance& () const;
		VkInstance GetInstance() const;
	};
}