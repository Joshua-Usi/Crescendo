#pragma once

#include "core/core.h"
#include "interfaces/ShaderProgram.h"

#include <vector>

namespace Crescendo::Rendering
{
	class OpenGLShaderProgram : public ShaderProgram
	{
	private:
		uint32_t shaderProgramID;
		std::vector<uint32_t> shaderIDs;
	public:
		OpenGLShaderProgram();
		/// <summary>
		/// Option to create shaders directly without having to call other methods
		/// </summary>
		/// <param name="vertexSource">Vertex shader source</param>
		/// <param name="fragmentSource">Fragment shader source</param>
		OpenGLShaderProgram(const char* vertexSource, const char* fragmentSource);
		virtual ~OpenGLShaderProgram();

		virtual void AttachShader(ShaderType type, const char* shaderSource) override;
		virtual void Link() override;
		virtual void Bind() override;
		virtual void Unbind() override;
	};
}