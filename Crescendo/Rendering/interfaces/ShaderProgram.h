#pragma once

#include "core/core.h"

#include "GraphicsAPI.h"

namespace Crescendo::Rendering
{
	enum class ShaderType
	{
		Vertex,
		Fragment,
	};
	class ShaderProgram
	{
	public:
		virtual ~ShaderProgram() {}

		virtual void AttachShader(ShaderType type, const char* shaderSource) = 0;
		virtual void Link() = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		static ShaderProgram* Create();
	};
}