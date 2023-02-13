#pragma once

#include "core/core.h"
#include "interfaces/BufferObject.h"

namespace Crescendo::Rendering
{
	class OpenGLBufferObject : public BufferObject
	{
	private:
		uint32_t bufferID;
		BufferType type;
	public:
		OpenGLBufferObject(BufferType bufferType);
		virtual ~OpenGLBufferObject();

		virtual void SetData(void* pointer, uint64_t size) override;
		virtual void Bind() override;
		virtual void Unbind() override;
	};
}