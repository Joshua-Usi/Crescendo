#include "ShaderProgram.h"

#include "platform/OpenGL/OpenGLShaderProgram.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	ShaderProgram* ShaderProgram::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: CS_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLShaderProgram();
		default: throw "Invalid Graphics API";
		}
	}
	ShaderProgram* ShaderProgram::Create(const char* vertexSource, const char* fragmentSource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: CS_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLShaderProgram(vertexSource, fragmentSource);
		default: throw "Invalid Graphics API";
		}
	}
}