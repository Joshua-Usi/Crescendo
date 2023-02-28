#include "VertexArray.h"

#include "platform/OpenGL/OpenGLVertexArray.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: CS_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLVertexArray();
		default: throw "Invalid Graphics API";
		}
	}
}