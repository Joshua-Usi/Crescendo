#pragma once
#include "common.hpp"
#include "Rendering/ResourceHandles.hpp"

CS_NAMESPACE_BEGIN
{
	struct Material : public Component
	{
		Vulkan::PipelineHandle pipeline;
		Vulkan::TextureHandle diffuseHandle, normalHandle;
		bool isTransparent, isDoubleSided, isShadowCasting;

		Material() : isTransparent(false), isDoubleSided(false), isShadowCasting(true) {}
		Material(Vulkan::PipelineHandle pipeline, Vulkan::TextureHandle diffuseHandle, Vulkan::TextureHandle normalHandle, bool isTransparent, bool isDoubleSided, bool isShadowCasting) : pipeline(pipeline), diffuseHandle(diffuseHandle), normalHandle(normalHandle), isTransparent(isTransparent), isDoubleSided(isDoubleSided), isShadowCasting(isShadowCasting) {}
	};
}