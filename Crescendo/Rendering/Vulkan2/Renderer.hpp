#pragma once

#include "common.hpp"
#include "Instance.hpp"

CS_NAMESPACE_BEGIN
{
	struct RendererSpecification
	{
		bool enableValidationLayers;
		std::string appName;
		std::string engineName;
		void* window;
		uint32_t descriptorSetsPerPool;
		uint32_t framesInFlight;
		uint32_t anisotropicSamples;
		uint32_t multisamples;
		float renderScale;
	};

	class Renderer
	{
	private:
		Vulkan::Instance instance;
	public:
		
	};
}