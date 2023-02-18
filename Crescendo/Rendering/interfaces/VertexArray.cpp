#include "VertexArray.h"

#include "platform/OpenGL/OpenGLVertexArray.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case GraphicsAPI::None: CS_ASSERT(false, "GraphicsAPI::None is currently not supported!"); return nullptr;
		case GraphicsAPI::OpenGL: return new OpenGLVertexArray();
		default: throw "Invalid Graphics API";
		}
	}
}