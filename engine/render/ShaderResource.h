#pragma once
#include "core/math/mat4.h"
#include "GL/glew.h"
#include <fstream>
#include <string>
#include <sstream>

class ShaderResource
{
public:
	unsigned int program;

	ShaderResource()
	{
		program = 0;
	}

	~ShaderResource() noexcept {
		Destroy();
	}
	
	void LoadShaders(const char* vertexFile, const char* fragmentFile)
	{
		std::string vCode;
		std::string fCode;
		try
		{
			std::ifstream vFile (vertexFile);
			std::ifstream fFile (fragmentFile);

			if (!vFile.is_open())
				std::cout << "[ERROR] VERTEX SHADER FILE NOT FOUND\n";
			if (!fFile.is_open())
				std::cout << "[ERROR] FRAGMENT SHADER FILE NOT FOUND\n";

			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vFile.rdbuf();
			fShaderStream << fFile.rdbuf();

			vFile.close();
			fFile.close();

			vCode = vShaderStream.str();
			fCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
		}

		const char* vShaderCode = vCode.c_str();
		const char* fShaderCode = fCode.c_str();

		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT__COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void UseProgram() const
	{
		glUseProgram(program);
	}

	void SetMatrix(const char* variableName, mat4 m) const
	{
		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, variableName), 1, GL_FALSE, (float*)&m);
	}

	void SetVec4(const char* variableName, vec4 v) const
	{
		glUseProgram(program);
		glUniform4fv(glGetUniformLocation(program, variableName), 1, (float*)&v);
	}

	void SetVec3(const char* variableName, vec3 v) const
	{
		glUseProgram(program);
		glUniform3fv(glGetUniformLocation(program, variableName), 1, (float*)&v);
	}

	void SetFloat(const char* variableName, float f) const
	{
		glUseProgram(program);
		glUniform1f(glGetUniformLocation(program, variableName), f);
	}

	void SetInt(const char* variableName, int i) const
	{
		glUseProgram(program);
		glUniform1i(glGetUniformLocation(program, variableName), i);
	}

	void Destroy()
	{
		glDeleteProgram(program);
		program = 0;
	}

	ShaderResource(const ShaderResource&) = delete;

	ShaderResource& operator=(const ShaderResource&) = delete;

	ShaderResource(ShaderResource&& other) noexcept {
		program = other.program;
		other.program = 0;
	}

	ShaderResource& operator=(ShaderResource&& other) noexcept {
		Destroy();
		program = other.program;
		other.program = 0;
		return *this;
	};

};