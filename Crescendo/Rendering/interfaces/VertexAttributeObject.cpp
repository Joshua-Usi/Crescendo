#include "VertexAttributeObject.h"

#include "platform/OpenGL/OpenGLVertexAttributeObject.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	VertexAttributeObject* VertexAttributeObject::Create()
	{
		switch (Renderer::GetAPI())
		{
		case GraphicsAPI::None:
			CS_ASSERT(false, "GraphicsAPI::None is currently not supported!");
			return nullptr;
		case GraphicsAPI::OpenGL:
			return new OpenGLVertexAttributeObject();
			break;
		default:
			throw "Invalid Graphics API";
		}
	}
}