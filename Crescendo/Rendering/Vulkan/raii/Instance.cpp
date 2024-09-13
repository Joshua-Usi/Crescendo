#include "Instance.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	bool isVolkInitialised = false;

	Instance::Instance() : vkbInstance() { vkbInstance.instance = nullptr; }
	Instance::Instance(const InstanceSpecification& spec)
	{
		if (!isVolkInitialised)
		{
			const VkResult result = volkInitialize();
			isVolkInitialised = result == VK_SUCCESS;
			if (!isVolkInitialised) cs_std::console::fatal("Failed to initialise volk!");
		}
		const vkb::Result<vkb::Instance> instanceResult = vkb::InstanceBuilder(vkGetInstanceProcAddr)
			.set_app_name(spec.appName.c_str()).set_engine_name(spec.engineName.c_str())
			.request_validation_layers(spec.useValidationLayers).require_api_version(spec.version.major, spec.version.minor, spec.version.patch)
			.use_default_debug_messenger().build();
		if (!instanceResult) cs_std::console::fatal("Failed to create Vulkan instance! ", instanceResult.error().message());
		this->vkbInstance = instanceResult.value();
		volkLoadInstance(*this);
	}
	Instance::~Instance()
	{
		if (this->vkbInstance.instance == nullptr) return;
		vkb::destroy_instance(this->vkbInstance);
	}
	Instance::Instance(Instance&& other) noexcept : vkbInstance(other.vkbInstance)
	{
		other.vkbInstance.instance = nullptr;
	}
	Instance& Instance::operator=(Instance&& other) noexcept
	{
		if (this == &other) return *this;
		this->vkbInstance = other.vkbInstance; other.vkbInstance.instance = nullptr;
		return *this;
	}
	Instance::operator VkInstance() const { return vkbInstance; }
	Instance::operator const vkb::Instance& () const { return vkbInstance; }
	VkInstance Instance::GetInstance() const { return vkbInstance; }
}