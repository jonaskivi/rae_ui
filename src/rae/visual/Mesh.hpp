#pragma once

#include <vector>
#include <string>
using namespace std;

#include <glm/glm.hpp>
using glm::vec3;

//ASSIMP
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"
//end // ASSIMP

#include "rae_ray/Hitable.hpp"
#include "rae/visual/Box.hpp"

namespace rae
{

class Material;

class Mesh : public Hitable
{
public:
	int id() { return m_id; }
protected:
	void id(int set) { m_id = set; }
	int m_id;

public:
	Mesh(){}
	Mesh(int set_id);
	~Mesh();
	
	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const;
	virtual Box getAabb(float t0, float t1) const { return m_aabb; }

	void generateBox();

	//ASSIMP
	bool loadModel(const string& filepath);
	void loadNode(const aiScene* scene, const aiNode* node);
	//end // ASSIMP

	void createVBOs();
	void render(unsigned set_shader_program_id) const;
	int triangleCount() const { return int(indices.size()) / 3; }
	void computeAabb();

protected:

	bool rayTriangleIntersection(const vec3& rayStart, const vec3& rayDirection,
		const vec3& v1, const vec3& v2, const vec3& v3,
		float& t, float& u, float& v/*, bool& frontFacing*/) const;
	void getTriangle(int idx, vec3& out0, vec3& out1, vec3& out2) const;
	vec3 getFaceNormal(int idx) const;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned short> indices;

	unsigned vertexBufferID;
	unsigned uvBufferID;
	unsigned normalBufferID;
	unsigned indexBufferID;

	Box m_aabb;
	Material* material; // TODO make better, don't use pointer. Use component ID.
};

} // end namespace rae
