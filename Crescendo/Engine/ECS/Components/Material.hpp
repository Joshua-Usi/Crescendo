#pragma once

#include "common.hpp"
#include "Rendering/Vulkan2/ResourceManager.hpp"

CS_NAMESPACE_BEGIN
{
	struct Material : public Component
	{
		uint32_t pipelineID;
		Vulkan::TextureHandle diffuseHandle, normalHandle;
		bool isTransparent, isDoubleSided, isShadowCasting;

		Material() : pipelineID(0), diffuseHandle(), normalHandle(), isTransparent(false), isDoubleSided(false), isShadowCasting(true) {}
		Material(uint32_t pipelineID, Vulkan::TextureHandle diffuseHandle, Vulkan::TextureHandle normalHandle, bool isTransparent, bool isDoubleSided, bool isShadowCasting) : pipelineID(pipelineID), diffuseHandle(diffuseHandle), normalHandle(normalHandle), isTransparent(isTransparent), isDoubleSided(isDoubleSided), isShadowCasting(isShadowCasting) {}
	};
}