#include "GraphicsContext.h"

#include "platform/OpenGL/OpenGLGraphicsContext.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	GraphicsContext* GraphicsContext::Create(void* windowHandle)
	{
		switch (Renderer::GetAPI())
		{
		case GraphicsAPI::None:
			CS_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		case GraphicsAPI::OpenGL:
			return new OpenGLGraphicsContext((GLFWwindow*)windowHandle);
			break;
		default:
			throw "Invalid Graphics API";
		}
	}
}