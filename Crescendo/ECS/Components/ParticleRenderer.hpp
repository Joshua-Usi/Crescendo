#pragma once
#include "common.hpp"
#include "Component.hpp"
#include "Rendering/ResourceHandles.hpp"

CS_NAMESPACE_BEGIN
{
	struct ParticleRenderer : Component
	{
		Vulkan::TextureHandle texture;

		ParticleRenderer(Vulkan::TextureHandle texture)
			: texture(texture)
		{}
	};
}