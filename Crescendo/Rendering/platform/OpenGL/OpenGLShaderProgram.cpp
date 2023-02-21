#pragma once

#include "core/core.h"
#include "OpenGLShaderProgram.h"
#include "console/console.h"

#include "glad/glad.h"

namespace Crescendo::Rendering
{
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
}