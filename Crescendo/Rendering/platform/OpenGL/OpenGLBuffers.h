#pragma once

#include "core/core.h"
#include "interfaces/Buffers.h"

namespace Crescendo::Rendering
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	private:
		uint32_t bufferID;
		uint32_t count;
		BufferLayout layout;
	public:
		OpenGLVertexBuffer(float* pointer, uint32_t count);
		virtual ~OpenGLVertexBuffer() override;

		virtual void SetData(float* pointer, uint32_t count) override;
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual uint32_t GetCount() override;
		virtual BufferLayout& GetLayout() override;
		virtual void SetLayout(const BufferLayout& layout) override;
	};
	class OpenGLIndexBuffer : public IndexBuffer
	{
	private:
		uint32_t bufferID;
		uint32_t count;
	public:
		OpenGLIndexBuffer(uint32_t* pointer, uint32_t count);
		virtual ~OpenGLIndexBuffer() override;

		virtual void SetData(uint32_t* pointer, uint32_t count) override;
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual uint32_t GetCount() override;
	};
}