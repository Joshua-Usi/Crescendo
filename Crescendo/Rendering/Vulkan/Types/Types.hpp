#pragma once

#include "Volk/volk.h"

#include "Buffer.hpp"
#include "Image.hpp"

#include <vector>

namespace Crescendo::Vulkan
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
			Crescendo::Vulkan::Buffer buffer;
			uint32_t elements;
			cs_std::graphics::Attribute attribute;
		};
		Crescendo::Vulkan::Buffer indexBuffer;
		uint32_t indexCount;
		std::vector<Attribute> vertexAttributes;
	};

	struct Texture
	{
		Crescendo::Vulkan::Image image;
		VkDescriptorSet set;
	};
}