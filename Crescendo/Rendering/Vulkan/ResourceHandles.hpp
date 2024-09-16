#pragma once
#include "common.hpp"
#include "cs_std/slot_map.hpp"
#include "raii/Buffer.hpp"
#include "raii/Image.hpp"
#include "raii/Sampler.hpp"
#include "cs_std/graphics/model.hpp"

#define CS_RESOURCE_MANAGER_CREATE_HANDLE(type, name)\
class name\
{\
private:\
	union k\
	{\
		/* If set to 4,293,967,295, the key will be invalid */\
		uint32_t index;\
		cs_std::slotmap_key<type> key; \
	} key;\
public:\
	name() { key.index = 0xFFFFFFFF; }\
	name(cs_std::slotmap_key<type> k) { key.key = k; }\
	operator cs_std::slotmap_key<type>() const { return key.key; }\
	uint32_t GetIndex() const { return cs_std::slotmap_key<type>::toIndex(key.key); }\
	bool IsValid() const { return key.index != 0xFFFFFFFF; }\
}

CS_NAMESPACE_BEGIN::Vulkan
{
	struct Buffer
	{
		Vk::Buffer buffer;
		Buffer() = default;
		Buffer(Vk::Buffer&& b) : buffer(std::move(b)) {}
	};

	struct Texture
	{
		Vk::Image image;
		VkSampler sampler;
		Texture() = default;
		Texture(Vk::Image&& i, VkSampler sampler) : image(std::move(i)), sampler(sampler) {}
	};

	struct Mesh
	{
		struct Attribute { Vk::Buffer buffer; uint32_t elements; cs_std::graphics::Attribute attribute; };
		std::vector<Attribute> vertexAttributes;
		Vk::Buffer indexBuffer;
		uint32_t indexCount;
		VkIndexType indexType;
		Mesh(std::vector<Attribute>&& vertexAttributes = {}, Vk::Buffer&& indexBuffer = {}, uint32_t indexCount = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32) : vertexAttributes(std::move(vertexAttributes)), indexBuffer(std::move(indexBuffer)), indexCount(indexCount), indexType(indexType) {}
		Vk::Buffer* GetAttributeBuffer(cs_std::graphics::Attribute attribute)
		{
			for (auto& vertexAttribute : vertexAttributes)
			{
				if (vertexAttribute.attribute == attribute) return &vertexAttribute.buffer;
			}
			return nullptr;
		}
	};

	CS_RESOURCE_MANAGER_CREATE_HANDLE(Buffer, BufferHandle);
	CS_RESOURCE_MANAGER_CREATE_HANDLE(Texture, TextureHandle);
	CS_RESOURCE_MANAGER_CREATE_HANDLE(Mesh, MeshHandle);
}