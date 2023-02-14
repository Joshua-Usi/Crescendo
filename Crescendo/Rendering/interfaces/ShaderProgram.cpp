#include "ShaderProgram.h"

#include "platform/OpenGL/OpenGLShaderProgram.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	ShaderProgram* ShaderProgram::Create()
	{
		switch (Renderer::GetAPI())
		{
		case GraphicsAPI::None: CS_ASSERT(false, "GraphicsAPI::None is currently not supported!"); return nullptr;
		case GraphicsAPI::OpenGL: return new OpenGLShaderProgram();
		default: throw "Invalid Graphics API";
		}
	}
}