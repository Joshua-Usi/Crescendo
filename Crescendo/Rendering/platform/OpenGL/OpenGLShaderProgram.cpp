#pragma once

#include "core/core.h"
#include "OpenGLShaderProgram.h"
#include "console/console.h"

#include "glad/glad.h"

namespace Crescendo::Rendering
{
	uint32_t OpenGLShaderProgram::GetUniformLocation(const char* name)
	{
		// Check if it exists in the uniform cache
		if (this->uniformCache.find(name) != this->uniformCache.end())
		{
			// Return if it does
			return this->uniformCache[0];
		}
		// Otherwise take the long route and get the uniform location directly from the GPU
		return glGetUniformLocation(this->shaderProgramID, name);
	}
	OpenGLShaderProgram::OpenGLShaderProgram()
	{
		this->shaderProgramID = glCreateProgram();
	}
	OpenGLShaderProgram::OpenGLShaderProgram(const char* vertexSource, const char* fragmentSource)
	{
		this->shaderProgramID = glCreateProgram();
		this->AttachShader(ShaderType::Vertex, vertexSource);
		this->AttachShader(ShaderType::Fragment, fragmentSource);
		this->Link();
	}
	OpenGLShaderProgram::~OpenGLShaderProgram()
	{
		glDeleteProgram(this->shaderProgramID);
		for (uint64_t i = 0; i < this->shaderIDs.size(); i++)
		{
			glDeleteShader(this->shaderIDs[i]);
		}
	}
	void OpenGLShaderProgram::AttachShader(ShaderType type, const char* shaderSource)
	{
		uint32_t shaderID;

		switch (type)
		{
		case ShaderType::Vertex:
			shaderID = glCreateShader(GL_VERTEX_SHADER);
			break;
		case ShaderType::Fragment:
			shaderID = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		default:
			CS_ASSERT(false, "Invalid ShaderType");
		}

		glShaderSource(shaderID, 1, &shaderSource, 0);

		glCompileShader(shaderID);

		int32_t isSuccessfullyCompiled;
		char infoLog[512];
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isSuccessfullyCompiled);
		if (!isSuccessfullyCompiled)
		{
			glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
			Engine::Console::EngineError("Shader Compilation Failed: {}", infoLog);
		}

		glAttachShader(this->shaderProgramID, shaderID);

		this->shaderIDs.push_back(shaderID);
	}
	void OpenGLShaderProgram::Link()
	{
		glLinkProgram(this->shaderProgramID);

		int32_t isSuccessfullyCompiled;
		char infoLog[512];
		glGetProgramiv(this->shaderProgramID, GL_LINK_STATUS, &isSuccessfullyCompiled);
		if (!isSuccessfullyCompiled)
		{
			glGetProgramInfoLog(this->shaderProgramID, 512, NULL, infoLog);
			Engine::Console::EngineError("Shader Program Linking Failed: {}", infoLog);
			
			glDeleteProgram(this->shaderProgramID);
			for (uint64_t i = 0; i < this->shaderIDs.size(); i++)
			{
				glDeleteShader(this->shaderIDs[i]);
			}
			this->shaderIDs.clear();
			return;
		}

		for (uint64_t i = 0; i < this->shaderIDs.size(); i++)
		{
			glDetachShader(this->shaderProgramID, this->shaderIDs[i]);
		}
		this->shaderIDs.clear();
	}
	void OpenGLShaderProgram::Bind()
	{
		glUseProgram(this->shaderProgramID);
	}
	void OpenGLShaderProgram::Unbind()
	{
		glUseProgram(0);
	}
	void OpenGLShaderProgram::SetBool(const char* name, bool value)
	{
		glUniform1i(this->GetUniformLocation(name), (int)value);
	}
	void OpenGLShaderProgram::SetInt(const char* name, int32_t value)
	{
		glUniform1i(this->GetUniformLocation(name), value);
	}
	void OpenGLShaderProgram::SetInt2(const char* name, glm::ivec2& value)
	{
		glUniform2iv(this->GetUniformLocation(name), 1, &value[0]);
	}
	void OpenGLShaderProgram::SetInt3(const char* name, glm::ivec3& value)
	{
		glUniform3iv(this->GetUniformLocation(name), 1, &value[0]);
	}
	void OpenGLShaderProgram::SetInt4(const char* name, glm::ivec4& value)
	{
		glUniform4iv(this->GetUniformLocation(name), 1, &value[0]);
	}
	void OpenGLShaderProgram::SetFloat(const char* name, float value)
	{
		glUniform1f(this->GetUniformLocation(name), value);
	}
	void OpenGLShaderProgram::SetFloat2(const char* name, glm::vec2& value)
	{
		glUniform2fv(this->GetUniformLocation(name), 1, &value[0]);
	}
	void OpenGLShaderProgram::SetFloat3(const char* name, glm::vec3& value)
	{
		glUniform3fv(this->GetUniformLocation(name), 1, &value[0]);
	}
	void OpenGLShaderProgram::SetFloat4(const char* name, glm::vec4& value)
	{
		glUniform4fv(this->GetUniformLocation(name), 1, &value[0]);
	}
	void OpenGLShaderProgram::SetMat3(const char* name, glm::mat3& value)
	{
		glUniformMatrix3fv(this->GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
	}
	void OpenGLShaderProgram::SetMat4(const char* name, glm::mat4& value)
	{
		glUniformMatrix4fv(this->GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
	}
}