#include "Instance.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	Instance::Instance(const InstanceSpecification& spec)
	{
		constexpr uint32_t VULKAN_MAJOR = 1, VULKAN_MINOR = 3, VULKAN_PATCH = 0;

		this->instance = Vk::Instance({
			.useValidationLayers = spec.useValidationLayers,
			.version = { VULKAN_MAJOR, VULKAN_MINOR, VULKAN_PATCH },
			.appName = spec.appName,
			.engineName = spec.engineName
		});

		for (const auto& window : spec.windows)
		{
			this->surfaces.emplace_back(this->instance, window);
		}
	}
	Instance::Instance(Instance&& other) noexcept : instance(std::move(other.instance)), surfaces(std::move(other.surfaces))
	{
		other.surfaces.clear();
	}
	Instance& Instance::operator=(Instance&& other) noexcept
	{
		if (this == &other) return *this;
		this->instance = std::move(other.instance);
		this->surfaces = std::move(other.surfaces);
		other.surfaces.clear();
		return *this;
	}
	Surface& Instance::GetSurface(size_t index)
	{
		return this->surfaces[index];
	}
	void Instance::AddSurface(void* window, std::function<void()> swapchainRecreationCallback)
	{
		this->surfaces.emplace_back(this->instance, window, swapchainRecreationCallback);
	}
	void Instance::RemoveSurface(size_t index)
	{
		this->surfaces.erase(this->surfaces.begin() + index);
	}
}