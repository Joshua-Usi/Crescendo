#include "OpenGLVertexAttributeObject.h"

#include "glad/glad.h"

namespace Crescendo::Rendering
{
	OpenGLVertexAttributeObject::OpenGLVertexAttributeObject()
	{
		glGenVertexArrays(1, &this->vertexAttributeID);
	}
	OpenGLVertexAttributeObject::~OpenGLVertexAttributeObject()
	{
		glDeleteVertexArrays(1, &this->vertexAttributeID);
	}
	void OpenGLVertexAttributeObject::SetAttributePointer(uint32_t index, int32_t components, uint64_t size)
	{
		glVertexAttribPointer(index, components, GL_FLOAT, GL_FALSE, components * size, (void*)0);
	}
	void OpenGLVertexAttributeObject::EnableAttribute(uint32_t index)
	{
		glEnableVertexAttribArray(index);
	}
	void OpenGLVertexAttributeObject::DisableAttribute(uint32_t index)
	{
		glDisableVertexAttribArray(index);
	}
	void OpenGLVertexAttributeObject::Bind()
	{
		glBindVertexArray(this->vertexAttributeID);
	}
	void OpenGLVertexAttributeObject::Unbind()
	{
		glBindVertexArray(0);
	}
}