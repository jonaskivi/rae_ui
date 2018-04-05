#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>

#include "loguru/loguru.hpp"

#include "rae/visual/Shader.hpp"
#include "rae/visual/Material.hpp"

using namespace rae;

Shader::Shader(String vertexFilePath, String fragmentFilePath) :
	m_vertexFilePath(vertexFilePath),
	m_fragmentFilePath(fragmentFilePath)
{
}

Shader::~Shader()
{
	if (m_programId != 0)
	{
		glDeleteProgram(m_programId);
	}
}

GLuint Shader::load(String vertexFilePath, String fragmentFilePath)
{
	// Create the shaders
	GLuint vertexShaderId	= glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	String vertexShaderCode;
	std::ifstream vertexShaderStream(vertexFilePath, std::ios::in);

	if (vertexShaderStream.is_open())
	{
		String line = "";
		while (getline(vertexShaderStream, line))
		{
			vertexShaderCode += "\n" + line;
		}
		vertexShaderStream.close();
	}
	else
	{
		LOG_F(ERROR, "Can't open shader: %s", vertexFilePath.c_str());
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	String fragmentShaderCode;
	std::ifstream fragmentShaderStream(fragmentFilePath, std::ios::in);

	if (fragmentShaderStream.is_open())
	{
		String line = "";
		while (getline(fragmentShaderStream, line))
		{
			fragmentShaderCode += "\n" + line;
		}
		fragmentShaderStream.close();
	}
	else
	{
		LOG_F(ERROR, "Can't open shader: %s", fragmentFilePath.c_str());
		getchar();
		return 0;
	}

	GLint result = GL_FALSE;
	int infoLogLength;

	// Compile Vertex Shader
	LOG_F(INFO, "Compiling vertex shader: %s", vertexFilePath.c_str());

	const char* vertexSourcePointer = vertexShaderCode.c_str();
	glShaderSource(vertexShaderId, 1, &vertexSourcePointer , NULL);
	glCompileShader(vertexShaderId);

	// Check Vertex Shader
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 1)
	{
		LOG_F(ERROR, "Error in shader: %s infoLogLength: %i", vertexFilePath.c_str(), infoLogLength);
		std::vector<char> vertexShaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(vertexShaderId, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
		LOG_F(ERROR, "%s", &vertexShaderErrorMessage[0]);
		getchar();
		return 0;
	}

	// Compile Fragment Shader
	LOG_F(INFO, "Compiling fragment shader: %s", fragmentFilePath.c_str());
	const char* fragmentSourcePointer = fragmentShaderCode.c_str();
	glShaderSource(fragmentShaderId, 1, &fragmentSourcePointer , NULL);
	glCompileShader(fragmentShaderId);

	// Check Fragment Shader
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 1)
	{
		LOG_F(ERROR, "Error in shader: %s", fragmentFilePath.c_str());
		Array<char> fragmentShaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(fragmentShaderId, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
		LOG_F(ERROR, "%s", &fragmentShaderErrorMessage[0]);
		getchar();
		return 0;
	}

	// Link the program
	GLuint programId = glCreateProgram();
	LOG_F(INFO, "Created shader program: %i", (int)programId);
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);

	// Check the program
	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 1)
	{
		LOG_F(ERROR, "Error in shader: %s or %s", vertexFilePath.c_str(), fragmentFilePath.c_str());
		Array<char> programErrorMessage(infoLogLength+1);
		glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
		LOG_F(ERROR, "%s", &programErrorMessage[0]);
		getchar();
		return 0;
	}

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	m_programId = programId;

	prepareUniforms();

	return programId;
}

void Shader::use()
{
	glUseProgram(m_programId);
}

ModelViewMatrixShader::ModelViewMatrixShader(String vertexFilePath, String fragmentFilePath) :
	Shader(vertexFilePath, fragmentFilePath)
{
}

void ModelViewMatrixShader::prepareUniforms()
{
	m_modelViewMatrixUni = glGetUniformLocation(m_programId, "modelViewProjectionMatrix");
}

void ModelViewMatrixShader::pushModelViewMatrix(const mat4& matrix)
{
	glUniformMatrix4fv(m_modelViewMatrixUni, 1, GL_FALSE, &matrix[0][0]);
}

BasicShader::BasicShader() :
	ModelViewMatrixShader("./data/shaders/basic.vert", "./data/shaders/basic.frag")
{
}

void BasicShader::prepareUniforms()
{
	ModelViewMatrixShader::prepareUniforms();
	m_viewMatrixUni			= glGetUniformLocation(m_programId, "viewMatrix");
	m_modelMatrixUni		= glGetUniformLocation(m_programId, "modelMatrix");
	m_lightPositionUni		= glGetUniformLocation(m_programId, "lightPosition_worldspace");
	m_tempBlendColorUni		= glGetUniformLocation(m_programId, "tempBlendColor");
	m_textureUni			= glGetUniformLocation(m_programId, "textureSampler");
}

void BasicShader::pushViewMatrix(const mat4& matrix)
{
	glUniformMatrix4fv(m_viewMatrixUni, 1, GL_FALSE, &matrix[0][0]);
}

void BasicShader::pushModelMatrix(const mat4& matrix)
{
	glUniformMatrix4fv(m_modelMatrixUni, 1, GL_FALSE, &matrix[0][0]);
}

void BasicShader::pushLightPosition(const vec3& position)
{
	glUniform3f(m_lightPositionUni, position.x, position.y, position.z);
}

void BasicShader::pushTempBlendColor(const Color& color)
{
	glUniform3f(m_tempBlendColorUni, color.x, color.y, color.z);
}

void BasicShader::pushTexture(const Material& material)
{
	GLuint textureId = material.textureId();

	if (textureId == 0)
	{
		//LOG_F(ERROR, "BasicShader::pushTexture: material has textureId 0.");
	}

	// Bind texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	// Set textureSampler to use Texture Unit 0
	glUniform1i(m_textureUni, 0);
}

PickingShader::PickingShader() :
	ModelViewMatrixShader("./data/shaders/picking.vert", "./data/shaders/picking.frag")
{
}

void PickingShader::prepareUniforms()
{
	ModelViewMatrixShader::prepareUniforms();
	m_entityUni = glGetUniformLocation(m_programId, "entityID");
}

void PickingShader::pushEntityId(Id id)
{
	glUniform1i(m_entityUni, id);
}

SingleColorShader::SingleColorShader() :
	ModelViewMatrixShader("./data/shaders/single_color.vert", "./data/shaders/single_color.frag")
{
}

void SingleColorShader::prepareUniforms()
{
	ModelViewMatrixShader::prepareUniforms();
	m_colorUni = glGetUniformLocation(m_programId, "lineColor");
}

void SingleColorShader::pushColor(const Color& color)
{
	glUniform3f(m_colorUni, color.x, color.y, color.z);
}
