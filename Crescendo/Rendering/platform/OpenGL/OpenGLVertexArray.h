#pragma once

#include "core/core.h"
#include "interfaces/VertexArray.h"

namespace Crescendo::Rendering
{
	class OpenGLVertexArray : public VertexArray
	{
	private:
		uint32_t vertexArrayID;
		std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
		std::shared_ptr<IndexBuffer> indexBuffer;
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray() override;
		
		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(std::shared_ptr<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() override;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() override;
	};
}