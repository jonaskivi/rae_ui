#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

//ASSIMP
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"
//end // ASSIMP

#include "rae/core/Types.hpp"

#include "rae_ray/Hitable.hpp"
#include "rae/visual/Box.hpp"

namespace rae
{

class Material;

enum class WindingOrder
{
	CounterClockwise,
	Clockwise
	// Not supported yet: TwoSided
};

class Mesh : public Hitable
{
public:
	Mesh();
	~Mesh();

	Mesh(Mesh&& other);
	Mesh& operator=(Mesh&& other);

	void createVBOs(GLenum usage = GL_STATIC_DRAW);
	void freeVBOs();

	// A silly fix for the need for a position from the instance:
	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const override { return false; };
	virtual bool hit(const vec3& position, const Ray& ray, float t_min, float t_max, HitRecord& record) const;
	virtual Box getAabb(float t0 = 0.0f, float t1 = 0.0f) const { return m_aabb; }

	void generateCube();
	void generateSphere(float radius = 0.5f, int rings = 32, int sectors = 32);
	void generateCone(int steps = 12);

	void generateLinesFromVertices(const Array<vec3>& vertices);

	//ASSIMP
	bool loadModel(const String& filepath);
	void loadNode(const aiScene* scene, const aiNode* node);
	//end // ASSIMP

	void render(uint shaderProgramId) const;
	void renderForOutline(uint shaderProgramId) const;
	void renderLines(uint shaderProgramId) const;
	int triangleCount() const { return int(m_indices.size()) / 3; }
	void computeAabb();
	void computeFaceNormals();
	Array<vec3> computeSmoothNormals();
	void computeOutlineNormals();

	const Array<vec3>& vertices() const { return m_vertices; }
	const Array<vec3>& normals() const { return m_normals; }

	WindingOrder windingOrder() const { return m_windingOrder; }
	GLenum glWindingOrder() const { return m_windingOrder == WindingOrder::CounterClockwise ? GL_CCW : GL_CW; }
	void setWindingOrder(WindingOrder value) { m_windingOrder = value; }

protected:

	bool rayTriangleIntersection(const vec3& rayStart, const vec3& rayDirection,
		const vec3& v1, const vec3& v2, const vec3& v3,
		float& t, float& u, float& v/*, bool& frontFacing*/) const;
	void getTriangle(int idx, vec3& out0, vec3& out1, vec3& out2) const;
	vec3 getFaceNormal(int idx) const;

	Array<vec3> m_vertices;
	Array<vec2> m_uvs;
	Array<vec3> m_normals;
	// Some models have flat normals, and those don't work so well with outlines. These are smooth normals just for
	// outline rendering. We might not have them, so the outline renderer will check that and use the regular normals
	// in that case.
	Array<vec3> m_normalsForOutline;
	Array<GLushort> m_indices;

	GLuint m_vertexBufferId        = 0;
	GLuint m_uvBufferId            = 0;
	GLuint m_normalBufferId        = 0;
	GLuint m_outlineNormalBufferId = 0;
	GLuint m_indexBufferId         = 0;

	WindingOrder m_windingOrder = WindingOrder::CounterClockwise;

	Box m_aabb;
	Material* m_material; // RAE_TODO make better, don't use pointer. Use component ID.
};

} // end namespace rae
