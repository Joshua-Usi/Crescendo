#pragma once

#include "common.hpp"
#include "RAII.hpp"
#include "Surface.hpp"
#include <functional>

CS_NAMESPACE_BEGIN::Vulkan
{
	struct InstanceSpecification
	{
		bool useValidationLayers;
		std::string appName, engineName;
	};

	class Instance
	{
	private:
		Vk::Instance instance;
		std::vector<Surface> surfaces;
	public:
		Instance() = default;
		Instance(const InstanceSpecification& spec);
		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
		Instance(Instance&& other) noexcept;
		Instance& operator=(Instance&& other) noexcept;
		Surface& GetSurface(size_t index);
		void CreateSurface(void* window, const Surface::SurfaceSpecification& spec);
		void RemoveSurface(size_t index);
		size_t GetSurfaceCount() const;
	};
}