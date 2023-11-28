#pragma once

#include "common.hpp"

#include "Volk/volk.h"

#include "Buffer.hpp"
#include "Image.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	struct SSBO
	{
		Buffer buffer;
		VkDescriptorSet set;
	};

	struct Mesh
	{
		struct Attribute
		{
			Buffer buffer;
			uint32_t elements;
			cs_std::graphics::Attribute attribute;
		};
		Buffer indexBuffer;
		uint32_t indexCount;
		std::vector<Attribute> vertexAttributes;
	};

	struct Texture
	{
		Image image;
		VkDescriptorSet set;
	};
}