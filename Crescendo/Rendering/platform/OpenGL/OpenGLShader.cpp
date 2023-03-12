#pragma once

#include "core/core.h"
#include "OpenGLShader.h"
#include "console/console.h"

#include "glad/glad.h"

namespace Crescendo::Rendering
{
	OpenGLShader::OpenGLShader()
	{
		this->shaderID = glCreateProgram();
	}
	OpenGLShader::OpenGLShader(const char* vertexSource, const char* fragmentSource)
	{
		this->shaderID = glCreateProgram();
		this->AttachShader(Shader::Type::Vertex, vertexSource);
		this->AttachShader(Shader::Type::Fragment, fragmentSource);
		this->Link();
	}
	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(this->shaderID);
		for (uint64_t i = 0; i < this->shaderIDs.size(); i++)
		{
			glDeleteShader(this->shaderIDs[i]);
		}
	}
	void OpenGLShader::AttachShader(Shader::Type type, const char* shaderSource)
	{
		uint32_t shaderID;

		switch (type)
		{
		case Shader::Type::Vertex:
			shaderID = glCreateShader(GL_VERTEX_SHADER);
			break;
		case Shader::Type::Fragment:
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

		glAttachShader(this->shaderID, shaderID);

		this->shaderIDs.push_back(shaderID);
	}
	void OpenGLShader::Link()
	{
		glLinkProgram(this->shaderID);

		int32_t isSuccessfullyCompiled;
		char infoLog[512];
		glGetProgramiv(this->shaderID, GL_LINK_STATUS, &isSuccessfullyCompiled);
		if (!isSuccessfullyCompiled)
		{
			glGetProgramInfoLog(this->shaderID, 512, NULL, infoLog);
			Engine::Console::EngineError("Shader Program Linking Failed: {}", infoLog);
			
			glDeleteProgram(this->shaderID);
			for (uint64_t i = 0; i < this->shaderIDs.size(); i++)
			{
				glDeleteShader(this->shaderIDs[i]);
			}
			this->shaderIDs.clear();
			return;
		}

		for (uint64_t i = 0; i < this->shaderIDs.size(); i++)
		{
			glDetachShader(this->shaderID, this->shaderIDs[i]);
		}
		this->shaderIDs.clear();
	}
	void OpenGLShader::Bind()
	{
		glUseProgram(this->shaderID);
	}
	void OpenGLShader::Unbind()
	{
		glUseProgram(0);
	}
	void OpenGLShader::SetBool(const char* name, bool value)
	{
		glUniform1i(glGetUniformLocation(this->shaderID, name), (int)value);
	}
	void OpenGLShader::SetInt(const char* name, int32_t value)
	{
		glUniform1i(glGetUniformLocation(this->shaderID, name), value);
	}
	void OpenGLShader::SetInt2(const char* name, const glm::ivec2& value)
	{
		glUniform2iv(glGetUniformLocation(this->shaderID, name), 1, &value[0]);
	}
	void OpenGLShader::SetInt3(const char* name, const glm::ivec3& value)
	{
		glUniform3iv(glGetUniformLocation(this->shaderID, name), 1, &value[0]);
	}
	void OpenGLShader::SetInt4(const char* name, const glm::ivec4& value)
	{
		glUniform4iv(glGetUniformLocation(this->shaderID, name), 1, &value[0]);
	}
	void OpenGLShader::SetFloat(const char* name, float value)
	{
		glUniform1f(glGetUniformLocation(this->shaderID, name), value);
	}
	void OpenGLShader::SetFloat2(const char* name, const glm::vec2& value)
	{
		glUniform2fv(glGetUniformLocation(this->shaderID, name), 1, &value[0]);
	}
	void OpenGLShader::SetFloat3(const char* name, const glm::vec3& value)
	{
		glUniform3fv(glGetUniformLocation(this->shaderID, name), 1, &value[0]);
	}
	void OpenGLShader::SetFloat4(const char* name, const glm::vec4& value)
	{
		glUniform4fv(glGetUniformLocation(this->shaderID, name), 1, &value[0]);
	}
	void OpenGLShader::SetMat3(const char* name, const glm::mat3& value)
	{
		glUniformMatrix3fv(glGetUniformLocation(this->shaderID, name), 1, GL_FALSE, &value[0][0]);
	}
	void OpenGLShader::SetMat4(const char* name, const glm::mat4& value)
	{
		glUniformMatrix4fv(glGetUniformLocation(this->shaderID, name), 1, GL_FALSE, &value[0][0]);
	}
}