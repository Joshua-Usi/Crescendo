#include "core/core.h"

#include "BufferObject.h"

#include "platform/OpenGL/OpenGLBufferObject.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	BufferObject* BufferObject::Create(BufferType bufferType)
	{
		switch (Renderer::GetAPI())
		{
		case GraphicsAPI::None:
			CS_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		case GraphicsAPI::OpenGL:
			return new OpenGLBufferObject(bufferType);
		default:
			throw "Invalid Graphics API";
		}
	}
}