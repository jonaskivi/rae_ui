#pragma once

#include <GL/glew.h>
#include "rae/core/Types.hpp"

namespace rae
{

class Material;

class Shader
{
public:
	Shader(){}
	Shader(String vertexFilePath, String fragmentFilePath);
	virtual ~Shader();

	GLuint load() { return load(m_vertexFilePath, m_fragmentFilePath); }

	virtual void prepareUniforms() {}

	void use();

	// RAE_TODO get rid of this detail, and encapsulate gl calls to functions:
	GLuint getProgramId() { return m_programId; }

protected:
	GLuint load(String vertexFilePath, String fragmentFilePath);

	GLuint m_programId = 0u;
	String m_vertexFilePath;
	String m_fragmentFilePath;
};

class ModelViewMatrixShader : public Shader
{
public:
	ModelViewMatrixShader(){}
	ModelViewMatrixShader(String vertexFilePath, String fragmentFilePath);
	virtual void prepareUniforms() override;

	void pushModelViewMatrix(const mat4& matrix);

protected:
	GLuint m_modelViewMatrixUni;
};

class BasicShader : public ModelViewMatrixShader
{
public:
	BasicShader();
	virtual void prepareUniforms() override;

	void pushViewMatrix(const mat4& matrix);
	void pushModelMatrix(const mat4& matrix);
	void pushLightPosition(const vec3& position);
	void pushTexture(const Material& material);

private:
	GLuint m_viewMatrixUni;
	GLuint m_modelMatrixUni;
	GLuint m_lightPositionUni;
	GLuint m_textureUni;
};

class PickingShader : public ModelViewMatrixShader
{
public:
	PickingShader();
	virtual void prepareUniforms() override;

	void pushEntityId(Id id);

private:
	GLuint m_entityUni;
};

class SingleColorShader : public ModelViewMatrixShader
{
public:
	SingleColorShader();
	virtual void prepareUniforms() override;

	void pushColor(const Color& color);

private:
	GLuint m_colorUni;
};

class OutlineShader : public ModelViewMatrixShader
{
public:
	OutlineShader();
	virtual void prepareUniforms() override;

	void pushColor(const Color& color);

private:
	GLuint m_colorUni;
};

}
