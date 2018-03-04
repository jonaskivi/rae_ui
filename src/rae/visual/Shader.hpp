#pragma once

#include <GL/glew.h>
#include "rae/core/Types.hpp"

namespace rae
{

class Material;

class Shader
{
public:

virtual ~Shader();

GLuint load(String vertexFilePath, String fragmentFilePath);

virtual void prepareUniforms() {}

void use();

// RAE_TODO get rid of this detail, and encapsulate gl calls to functions:
GLuint getProgramId() { return m_programId; }

protected:
GLuint m_programId = 0u;
};

class ModelViewMatrixShader : public Shader
{
public:
	virtual void prepareUniforms() override;

	void pushModelViewMatrix(const mat4& matrix);

protected:
	GLuint m_modelViewMatrixUni;
};

class BasicShader : public ModelViewMatrixShader
{
public:
	virtual void prepareUniforms() override;

	void pushViewMatrix(const mat4& matrix);
	void pushModelMatrix(const mat4& matrix);
	void pushLightPosition(const vec3& position);
	void pushTempBlendColor(const Color& color);
	void pushTexture(const Material& material);

private:
	GLuint m_viewMatrixUni;
	GLuint m_modelMatrixUni;
	GLuint m_lightPositionUni;
	GLuint m_tempBlendColorUni;
	GLuint m_textureUni;
};

class PickingShader : public ModelViewMatrixShader
{
public:
	virtual void prepareUniforms() override;

	void pushEntityId(Id id);

private:
	GLuint m_entityUni;
};

}
