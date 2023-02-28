#include "core/core.h"

#include "Buffers.h"

#include "platform/OpenGL/OpenGLBuffers.h"
#include "Renderer.h"

#include <vector>

namespace Crescendo::Rendering
{
	VertexBuffer* VertexBuffer::Create(float* pointer, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: CS_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLVertexBuffer(pointer, count);
		default: throw "Invalid Graphics API";
		}
	}
	IndexBuffer* IndexBuffer::Create(uint32_t* pointer, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: CS_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLIndexBuffer(pointer, count);
		default: throw "Invalid Graphics API";
		}
	}
	void BufferLayout::SetOffsetsAndStride()
	{
		uint32_t offset = 0;
		this->stride = 0;
		for (auto& element : this->elements)
		{
			element.offset = offset;
			offset += element.size;
			this->stride += element.size;
		}
	}
}