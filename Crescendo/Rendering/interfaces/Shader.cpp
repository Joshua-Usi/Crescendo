#include "Shader.h"

#include "platform/OpenGL/OpenGLShader.h"
#include "Renderer.h"

namespace Crescendo::Rendering
{
	Shader* Shader::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: CS_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLShader();
		default: throw "Invalid Graphics API";
		}
	}
	Shader* Shader::Create(const char* vertexSource, const char* fragmentSource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: CS_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLShader(vertexSource, fragmentSource);
		default: throw "Invalid Graphics API";
		}
	}
}