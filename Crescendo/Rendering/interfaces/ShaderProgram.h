#pragma once

#include "core/core.h"

#include "glm/glm.hpp"

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

		/// <summary>
		/// Attaches a shader to the program via shader source text
		/// </summary>
		/// <param name="type">the type of shader to attach</param>
		/// <param name="shaderSource">pointer to the source of the shader data</param>
		virtual void AttachShader(ShaderType type, const char* shaderSource) = 0;
		/// <summary>
		/// Once all shaders are attached, link must be called at least once to compile the shader
		/// </summary>
		virtual void Link() = 0;
		/// <summary>
		/// Bind the shader to the current context for changing
		/// </summary>
		virtual void Bind() = 0;
		/// <summary>
		/// Unbind the current shader to prevent unintended changes
		/// </summary>
		virtual void Unbind() = 0;

		virtual void SetBool(const char* name, bool value) = 0;

		virtual void SetInt(const char* name, int32_t value) = 0;
		virtual void SetInt2(const char* name, const glm::ivec2& value) = 0;
		virtual void SetInt3(const char* name, const glm::ivec3& value) = 0;
		virtual void SetInt4(const char* name, const glm::ivec4& value) = 0;

		virtual void SetFloat(const char* name, float value) = 0;
		virtual void SetFloat2(const char* name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const char* name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const char* name, const glm::vec4& value) = 0;

		virtual void SetMat3(const char* name, const glm::mat3& value) = 0;
		virtual void SetMat4(const char* name, const glm::mat4& value) = 0;

		/// <summary>
		/// Creates a RenderAPI specific ShaderProgram
		/// </summary>
		/// <returns>A polymorphic ShaderProgram</returns>
		static ShaderProgram* Create();
		/// <summary>
		/// Creates a RenderAPI specific ShaderProgram taking vertex and fragment shaders and automatically compiling and linking
		/// </summary>
		/// <param name="vertexSource">Source text of vertex shader</param>
		/// <param name="fragmentSource">Source text of fragment shader</param>
		/// <returns></returns>
		static ShaderProgram* Create(const char* vertexSource, const char* fragmentSource);
	};
}