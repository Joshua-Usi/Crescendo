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
		virtual ~OpenGLShaderProgram();

		virtual void AttachShader(ShaderType type, const char* shaderSource) override;
		virtual void Link() override;
		virtual void Bind() override;
		virtual void Unbind() override;
	};
}