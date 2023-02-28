
#include "platform/OpenGL/OpenGLGraphicsContext.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	GraphicsContext* GraphicsContext::Create(void* windowHandle)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: CS_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLGraphicsContext((GLFWwindow*)windowHandle);
		default: throw "Invalid Graphics API";
		}
	}
}