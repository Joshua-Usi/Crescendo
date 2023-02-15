#include "interfaces/BufferObject.h"

#include "platform/OpenGL/OpenGLBufferObject.h"
#include "OpenGLBufferObject.h"

#include "glad/glad.h"

namespace Crescendo::Rendering
{
	OpenGLBufferObject::OpenGLBufferObject(BufferType bufferType)
	{
		count = 0;
		type = bufferType;
		glGenBuffers(1, &this->bufferID);
	}
	OpenGLBufferObject::~OpenGLBufferObject()
	{
		glDeleteBuffers(1, &this->bufferID);
	}
	void OpenGLBufferObject::SetData(void* pointer, uint32_t count)
	{
		this->count = count;
		switch (type)
		{
		case BufferType::Vertex:
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count, pointer, GL_STATIC_DRAW);
			break;
		case BufferType::Index:
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * count, pointer, GL_STATIC_DRAW);
			break;
		}
	}
	void OpenGLBufferObject::Bind()
	{
		switch (type)
		{
		case BufferType::Vertex:
			glBindBuffer(GL_ARRAY_BUFFER, this->bufferID);
			break;
		case BufferType::Index:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->bufferID);
			break;
		}
	}
	void OpenGLBufferObject::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	uint32_t OpenGLBufferObject::GetCount()
	{
		return this->count;
	}
}