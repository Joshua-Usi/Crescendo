#include "glad/glad.h"
#include "OpenGLBuffers.h"

namespace Crescendo::Rendering
{
	Crescendo::Rendering::OpenGLVertexBuffer::OpenGLVertexBuffer(float* pointer, uint32_t count)
	{
		glGenBuffers(1, &this->bufferID);
		this->Bind();
		this->SetData(pointer, count);
	}
	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &this->bufferID);
	}
	void OpenGLVertexBuffer::SetData(float* pointer, uint32_t count)
	{
		this->count = count;
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count, pointer, GL_STATIC_DRAW);
	}
	void OpenGLVertexBuffer::Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, this->bufferID);
	}
	void OpenGLVertexBuffer::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	uint32_t OpenGLVertexBuffer::GetCount()
	{
		return this->count;
	}
	BufferLayout& OpenGLVertexBuffer::GetLayout()
	{
		return layout;
	}
	void OpenGLVertexBuffer::SetLayout(const BufferLayout& layout)
	{
		this->layout = layout;
	}
	/* ------------------------------------------------------------------- */
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* pointer, uint32_t count)
	{
		glGenBuffers(1, &this->bufferID);
		this->Bind();
		this->SetData(pointer, count);
	}
	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &this->bufferID);
	}
	void OpenGLIndexBuffer::SetData(uint32_t* pointer, uint32_t count)
	{
		this->count = count;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * count, pointer, GL_STATIC_DRAW);
	}
	void OpenGLIndexBuffer::Bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->bufferID);
	}
	void OpenGLIndexBuffer::Unbind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	uint32_t OpenGLIndexBuffer::GetCount()
	{
		return this->count;
	}
}