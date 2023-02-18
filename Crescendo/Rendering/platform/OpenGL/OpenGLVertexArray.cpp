#include "OpenGLVertexArray.h"

#include "glad/glad.h"

namespace Crescendo::Rendering
{
	GLenum ShaderDataTypeToOpenGLBaseType(Rendering::ShaderDataType type)
	{
		switch (type) {
			case Rendering::ShaderDataType::Bool: return GL_BOOL;

			case Rendering::ShaderDataType::Int: return GL_INT;
			case Rendering::ShaderDataType::Int2: return GL_INT;
			case Rendering::ShaderDataType::Int3: return GL_INT;
			case Rendering::ShaderDataType::Int4: return GL_INT;

			case Rendering::ShaderDataType::Float: return GL_FLOAT;
			case Rendering::ShaderDataType::Float2: return GL_FLOAT;
			case Rendering::ShaderDataType::Float3: return GL_FLOAT;
			case Rendering::ShaderDataType::Float4: return GL_FLOAT;

			case Rendering::ShaderDataType::Double: return GL_DOUBLE;
			case Rendering::ShaderDataType::Double2: return GL_DOUBLE;
			case Rendering::ShaderDataType::Double3: return GL_DOUBLE;
			case Rendering::ShaderDataType::Double4: return GL_DOUBLE;

			case Rendering::ShaderDataType::Mat3: return GL_FLOAT;
			case Rendering::ShaderDataType::Mat4: return GL_FLOAT;
		}

		CS_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}
	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &this->vertexArrayID);
	}
	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &this->vertexArrayID);
	}
	void OpenGLVertexArray::Bind()
	{
		glBindVertexArray(this->vertexArrayID);
	}
	void OpenGLVertexArray::Unbind()
	{
		glBindVertexArray(0);
	}
	void OpenGLVertexArray::AddVertexBuffer(std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		this->Bind();
		vertexBuffer->Bind();

		CS_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "VertexBuffer has no layout!");

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.type),
				element.normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.offset);
			index++;
		}
		this->vertexBuffers.push_back(vertexBuffer);
	}
	void OpenGLVertexArray::SetIndexBuffer(std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		this->Bind();
		indexBuffer->Bind();
		this->indexBuffer = indexBuffer;
	}
	const std::vector<std::shared_ptr<VertexBuffer>>& OpenGLVertexArray::GetVertexBuffers()
	{
		return this->vertexBuffers;
	}
	const std::shared_ptr<IndexBuffer>& OpenGLVertexArray::GetIndexBuffer()
	{
		return this->indexBuffer;
	}
}