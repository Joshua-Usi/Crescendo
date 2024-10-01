#pragma once
#include "common.hpp"
#include "cs_std/slot_map.hpp"
#include "Vulkan/raii/Buffer.hpp"
#include "Vulkan/raii/Framebuffer.hpp"
#include "Vulkan/raii/Image.hpp"
#include "Vulkan/raii/Pipeline.hpp"
#include "Vulkan/raii/RenderPass.hpp"
#include "cs_std/graphics/model.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	template<typename T>
	class BaseHandle
	{
	private:
		union k
		{
			/* If set to 4,293,967,295, the key will be invalid */
			uint32_t index;
			cs_std::slotmap_key<T> key;
		} key;
	public:
		BaseHandle() { key.index = 0xFFFFFFFF; }
		BaseHandle(cs_std::slotmap_key<T> k) { key.key = k; }
		operator cs_std::slotmap_key<T>() const { return key.key; }
		uint32_t GetIndex() const { return cs_std::slotmap_key<T>::toIndex(key.key); }
		bool IsValid() const { return key.index != 0xFFFFFFFF; }
	};

	template<typename Tag>
	struct TypedBufferHandle : BaseHandle<Vulkan::Vk::Buffer>
	{
		using BaseHandle::BaseHandle;
	};

	struct SSBOBufferTag {};
	struct GPUBufferTag {};

	using SSBOBufferHandle = TypedBufferHandle<SSBOBufferTag>;
	using GPUBufferHandle = TypedBufferHandle<GPUBufferTag>;
	using TextureHandle = BaseHandle<Vulkan::Vk::Image>;
	using FramebufferHandle = BaseHandle<Vulkan::Vk::Framebuffer>;
	using RenderPassHandle = BaseHandle<Vulkan::Vk::RenderPass>;
	using PipelineHandle = BaseHandle<Vulkan::Vk::Pipeline>;

	struct Mesh
	{
		struct Attribute { GPUBufferHandle buffer; cs_std::graphics::Attribute attribute; };
		std::vector<Attribute> vertexAttributes;
		GPUBufferHandle indexBuffer;
		uint32_t indexCount;
		VkIndexType indexType;
		Mesh() = default;
		Mesh(const std::vector<Attribute>& vertexAttributes, GPUBufferHandle, uint32_t indexCount, VkIndexType indexType = VK_INDEX_TYPE_UINT32)
			: vertexAttributes(vertexAttributes), indexBuffer(indexBuffer), indexCount(indexCount), indexType(indexType) {}
		GPUBufferHandle GetAttributeBufferHandle(cs_std::graphics::Attribute attribute)
		{
			for (auto& vertexAttribute : vertexAttributes)
				if (vertexAttribute.attribute == attribute)
					return vertexAttribute.buffer;
			return GPUBufferHandle();
		}
	};

}