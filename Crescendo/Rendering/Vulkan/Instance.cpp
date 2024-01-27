#include "Instance.hpp"

#include "GLFW/glfw3.h"
#include "Instance.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	bool Instance::isVolkInitialised = false;

	Instance::Instance(bool useValidationLayers, const std::string& appName, const std::string& engineName, const std::vector<void*>& windows)
	{
		constexpr uint32_t CS_VK_MAJOR = 1, CS_VK_MINOR = 3, CS_VK_PATCH = 0;

		if (!isVolkInitialised)
		{
			const VkResult result = volkInitialize();
			CS_ASSERT(result == VK_SUCCESS, "Failed to initialise volk!");
			isVolkInitialised = result == VK_SUCCESS;
		}

		// Create instance and debug messenger
		const vkb::Result<vkb::Instance> instanceResult = vkb::InstanceBuilder(vkGetInstanceProcAddr)
			.set_app_name(appName.c_str()).set_engine_name(engineName.c_str())
			.request_validation_layers(useValidationLayers).require_api_version(CS_VK_MAJOR, CS_VK_MINOR, CS_VK_PATCH)
			.use_default_debug_messenger().build();
		if (!instanceResult) cs_std::console::fatal("Failed to create Vulkan instance!", instanceResult.error().message());
		
		this->vkbInstance = instanceResult.value();
		volkLoadInstance(*this);

		for (const auto& window : windows)
		{
			this->surfaces.emplace_back(*this, window);
		}
	}
	Instance::~Instance()
	{
		if (this->vkbInstance.instance == nullptr) return;
		this->surfaces.clear();
		vkb::destroy_instance(this->vkbInstance);
	}
	Instance::Instance(Instance&& other) noexcept : vkbInstance(other.vkbInstance), surfaces(std::move(other.surfaces))
	{
		other.vkbInstance.instance = nullptr;
		other.surfaces.clear();
	}
	Instance& Instance::operator=(Instance&& other) noexcept
	{
		if (this == &other) return *this;
		if (this->vkbInstance.instance != nullptr) this->~Instance();

		this->vkbInstance = other.vkbInstance; other.vkbInstance.instance = nullptr;
		this->surfaces = std::move(other.surfaces); other.surfaces.clear();
		return *this;
	}
}