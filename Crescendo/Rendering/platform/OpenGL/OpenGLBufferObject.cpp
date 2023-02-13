#include "interfaces/BufferObject.h"

#include "platform/OpenGL/OpenGLBufferObject.h"
#include "OpenGLBufferObject.h"

#include "glad/glad.h"

namespace Crescendo::Rendering
{
	OpenGLBufferObject::OpenGLBufferObject(BufferType bufferType)
	{
		type = bufferType;
		glGenBuffers(1, &this->bufferID);
	}
	OpenGLBufferObject::~OpenGLBufferObject()
	{
		glDeleteBuffers(1, &this->bufferID);
	}
	void OpenGLBufferObject::SetData(void* pointer, uint64_t size)
	{
		switch (type)
		{
		case BufferType::ArrayBuffer:
			glBufferData(GL_ARRAY_BUFFER, size, pointer, GL_STATIC_DRAW);
			break;
		case BufferType::ElementArrayBuffer:
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, pointer, GL_STATIC_DRAW);
			break;
		}
	}
	void OpenGLBufferObject::Bind()
	{
		switch (type)
		{
		case BufferType::ArrayBuffer:
			glBindBuffer(GL_ARRAY_BUFFER, this->bufferID);
			break;
		case BufferType::ElementArrayBuffer:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->bufferID);
			break;
		}
	}
	void OpenGLBufferObject::Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}