#pragma once

#include "core/core.h"
#include "interfaces/ShaderProgram.h"

#include <vector>
#include <unordered_map>

namespace Crescendo::Rendering
{
	class OpenGLShaderProgram : public ShaderProgram
	{
	private:
		uint32_t shaderProgramID;
		std::vector<uint32_t> shaderIDs;
		std::unordered_map<const char*, uint32_t> uniformCache;

		uint32_t GetUniformLocation(const char* name);
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

		virtual void SetBool(const char* name, bool value) override;

		virtual void SetInt(const char* name, int32_t value) override;
		virtual void SetInt2(const char* name, const glm::ivec2& value) override;
		virtual void SetInt3(const char* name, const glm::ivec3& value) override;
		virtual void SetInt4(const char* name, const glm::ivec4& value) override;

		virtual void SetFloat(const char* name, float value) override;
		virtual void SetFloat2(const char* name, const glm::vec2& value) override;
		virtual void SetFloat3(const char* name, const glm::vec3& value) override;
		virtual void SetFloat4(const char* name, const glm::vec4& value) override;

		virtual void SetMat3(const char* name, const glm::mat3& value) override;
		virtual void SetMat4(const char* name, const glm::mat4& value) override;
	};
}