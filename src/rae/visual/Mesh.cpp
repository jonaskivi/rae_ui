#include "rae/visual/Mesh.hpp"

#include <fstream>

#include "GL/glew.h"
#include "loguru/loguru.hpp"

#include <glm/gtx/vector_angle.hpp>

#include "rae/core/Utils.hpp"
#include "rae/visual/Material.hpp"


using namespace rae;

Mesh::Mesh()
{
	//m_material = new Lambertian(vec3(0.1f, 0.2f, 0.7f));
	m_material = new Metal(vec3(0.1f, 0.2f, 0.7f), 0.3f);
}

Mesh::~Mesh()
{
	LOG_F(INFO, "Mesh destructor.");
	freeVBOs();
	delete m_material;
}

Mesh::Mesh(Mesh&& other)
{
	other.freeVBOs();

	m_vertices = std::move(other.m_vertices);
	m_uvs = std::move(other.m_uvs);
	m_normals = std::move(other.m_normals);
	m_indices = std::move(other.m_indices);
	m_aabb = std::move(other.m_aabb);
	m_material = other.m_material;

	other.m_material = nullptr;

	createVBOs();
}

Mesh& Mesh::operator=(Mesh&& other)
{
	if (this != &other)
	{
		freeVBOs();
		other.freeVBOs();

		m_vertices = std::move(other.m_vertices);
		m_uvs = std::move(other.m_uvs);
		m_normals = std::move(other.m_normals);
		m_indices = std::move(other.m_indices);
		m_aabb = std::move(other.m_aabb);
		m_material = other.m_material;

		other.m_material = nullptr;

		createVBOs();
	}
	return *this;
}

void Mesh::createVBOs(GLenum usage)
{
	//LOG_F(INFO, "Mesh::createVBOs.");

	if (m_vertices.size() <= 0 ||
		m_indices.size() <= 0 ||
		m_uvs.size() <= 0 ||
		m_normals.size() <= 0)
	{
		LOG_F(ERROR, "Mesh::createVBOs: Can't create VBOs because vertices (or something else) is empty.");
		return;
	}

	if (m_vertexBufferId == 0)
		glGenBuffers(1, &m_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], usage);

	if (m_uvBufferId == 0)
		glGenBuffers(1, &m_uvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], usage);

	if (m_normalBufferId == 0)
		glGenBuffers(1, &m_normalBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], usage);

	if (!m_normalsForOutline.empty())
	{
		if (m_outlineNormalBufferId == 0)
			glGenBuffers(1, &m_outlineNormalBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_outlineNormalBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_normalsForOutline.size() * sizeof(glm::vec3), &m_normalsForOutline[0], usage);
	}

	if (m_indexBufferId == 0)
		glGenBuffers(1, &m_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLushort), &m_indices[0], usage);

	if (m_vertexBufferId == 0 &&
		m_uvBufferId == 0 &&
		m_normalBufferId == 0 &&
		m_indexBufferId == 0)
	{
		LOG_F(ERROR, "Mesh::createVBOs FAILED m_vertexBufferId: %i "
			"m_uvBufferId: %i m_normalBufferId: %i m_indexBufferId: %i",
			m_vertexBufferId,
			m_uvBufferId,
			m_normalBufferId,
			m_indexBufferId
			);
		return;
	}
}

void Mesh::freeVBOs()
{
	if (m_vertexBufferId == 0 &&
		m_uvBufferId == 0 &&
		m_normalBufferId == 0 &&
		m_indexBufferId == 0)
	{
		LOG_F(ERROR, "Mesh::freeVBOs, but no resources created.");
		return;
	}

	//LOG_F(INFO, "Mesh::freeVBOs.");
	glDeleteBuffers(1, &m_vertexBufferId);
	glDeleteBuffers(1, &m_uvBufferId);
	glDeleteBuffers(1, &m_normalBufferId);
	glDeleteBuffers(1, &m_indexBufferId);

	if (m_outlineNormalBufferId != 0)
	{
		glDeleteBuffers(1, &m_outlineNormalBufferId);
		m_outlineNormalBufferId = 0;
	}

	m_vertexBufferId	= 0;
	m_uvBufferId		= 0;
	m_normalBufferId	= 0;
	m_indexBufferId		= 0;
}

// Möller-Trumbore ray triangle intersection
// t is outDistance on the ray
// u and v are coordinates on the triangle plane?
bool Mesh::rayTriangleIntersection(const vec3& rayStart, const vec3& rayDirection,
	const vec3& v1, const vec3& v2, const vec3& v3,
	float& t, float& u, float& v/*, bool& frontFacing*/) const
{
	vec3 e2 = v3 - v1;            // second edge
	vec3 e1 = v2 - v1;            // first edge
	vec3 r = glm::cross(rayDirection, e2); // (rayDirection X e2)
	vec3 s = rayStart - v1;       // translated ray origin
	float a = glm::dot(e1, r);    // a = (d X e2) * e1
	float f = 1.0f / a;
	vec3 q = glm::cross(s, e1);
	u = glm::dot(s, r);
	//frontFacing = true;

	const float epsilon = 0.000001f;

	if (a > epsilon)
	{
		// Front facing triangle...
		if (u < 0.0f || u > a)
		{
			return false;
		}

		float v = glm::dot(rayDirection, q);

		if (v < 0.0f || u + v > a)
		{
			return false;
		}
	}/* We are not interested in backfacing triangles currently
	else if (a < -epsilon)
	{
		// Back facing triangle...
		frontFacing = false;
		if (u > 0.0f || u < a)
		{
			return false;
		}

		float v = glm::dot(rayDirection, q);

		if (v > 0.0f || u + v < a)
		{
			return false;
		}
	}
	*/
	else
	{
		// Ray is parallel to plane of the triangle (or backfacing as we've disabled that branch)
		//frontFacing = false;
		return false;
	}

	t = f * glm::dot(e2, q);
	u = u * f;
	v = v * f;

	return true;
}

bool Mesh::hit(const vec3& position, const Ray& ray, float t_min, float t_max, HitRecord& record) const
{
	Ray rayInLocalPosition = ray;
	rayInLocalPosition.moveOrigin(-position);

	if (m_aabb.hit(rayInLocalPosition, t_min, t_max) == false)
		return false;

	vec3 v0, v1, v2;
	float u, v;
	float hitDistance;

	bool isHit = false;

	for (int i = 0; i < triangleCount(); ++i)
	{
		getTriangle(i, v0, v1, v2);

		if (rayTriangleIntersection(
			rayInLocalPosition.origin(),
			rayInLocalPosition.direction(), v0, v1, v2, hitDistance, u, v)
				&& hitDistance < t_max
				&& hitDistance > t_min)
		{
			isHit = true;
			record.t = hitDistance;
			record.point = ray.pointAtParameter(record.t);
			record.normal = getFaceNormal(i); // currently just face normals
			record.material = m_material;
		}
	}

	return isHit;
}

void Mesh::getTriangle(int idx, vec3& out0, vec3& out1, vec3& out2) const
{
	if (idx >= triangleCount())
	{
		assert(0);
		return;
	}

	idx = idx * 3;
	out0 = m_vertices[m_indices[idx]];
	out1 = m_vertices[m_indices[idx+1]];
	out2 = m_vertices[m_indices[idx+2]];
}

vec3 Mesh::getFaceNormal(int idx) const
{
	if (idx >= triangleCount())
	{
		assert(0);
		return vec3(0.0f, 1.0f, 0.0f);
	}

	idx = idx * 3;
	vec3 normal = m_normals[m_indices[idx]];
	normal += m_normals[m_indices[idx+1]];
	normal += m_normals[m_indices[idx+2]];
	return glm::normalize(normal);
}

void Mesh::computeAabb()
{
	m_aabb.clear();
	for(int i = 0; i < (int)m_vertices.size(); ++i)
	{
		m_aabb.grow(m_vertices[i]);
	}
}

void Mesh::generateCube()
{
	//LOG_F("Generating Mesh.");

	float boxSize = 0.5f; // Actually half of the box size

	m_vertices =
	{
		glm::vec3(-boxSize,  boxSize,  boxSize), // 0
		glm::vec3( boxSize,  boxSize,  boxSize), // 1
		glm::vec3( boxSize,  boxSize, -boxSize), // 2
		glm::vec3(-boxSize,  boxSize, -boxSize), // 3

		glm::vec3(-boxSize, -boxSize,  boxSize), // 4
		glm::vec3(-boxSize, -boxSize, -boxSize), // 5
		glm::vec3( boxSize, -boxSize, -boxSize), // 6
		glm::vec3( boxSize, -boxSize,  boxSize), // 7

		glm::vec3(-boxSize, -boxSize, -boxSize), // 8
		glm::vec3(-boxSize,  boxSize, -boxSize), // 9
		glm::vec3( boxSize,  boxSize, -boxSize), // 10
		glm::vec3( boxSize, -boxSize, -boxSize), // 11

		glm::vec3( boxSize, -boxSize,  boxSize), // 12
		glm::vec3( boxSize,  boxSize,  boxSize), // 13
		glm::vec3(-boxSize,  boxSize,  boxSize), // 14
		glm::vec3(-boxSize, -boxSize,  boxSize), // 15

		glm::vec3(-boxSize, -boxSize,  boxSize), // 16
		glm::vec3(-boxSize,  boxSize,  boxSize), // 17
		glm::vec3(-boxSize,  boxSize, -boxSize), // 18
		glm::vec3(-boxSize, -boxSize, -boxSize), // 19

		glm::vec3( boxSize, -boxSize, -boxSize), // 20
		glm::vec3( boxSize,  boxSize, -boxSize), // 21
		glm::vec3( boxSize,  boxSize,  boxSize), // 22
		glm::vec3( boxSize, -boxSize,  boxSize)  // 23
	};

	m_uvs =
	{
		glm::vec2(1.0f,  0.0f), // 1
		glm::vec2(1.0f,  1.0f), // 2
		glm::vec2(0.0f,  1.0f), // 3
		glm::vec2(0.0f,  0.0f), // 0

		glm::vec2(1.0f,  0.0f), // 1
		glm::vec2(1.0f,  1.0f), // 2
		glm::vec2(0.0f,  1.0f), // 3
		glm::vec2(0.0f,  0.0f), // 0

		glm::vec2(1.0f,  0.0f), // 1
		glm::vec2(1.0f,  1.0f), // 2
		glm::vec2(0.0f,  1.0f), // 3
		glm::vec2(0.0f,  0.0f), // 0

		glm::vec2(1.0f,  0.0f), // 1
		glm::vec2(1.0f,  1.0f), // 2
		glm::vec2(0.0f,  1.0f), // 3
		glm::vec2(0.0f,  0.0f), // 0

		glm::vec2(1.0f,  0.0f), // 1
		glm::vec2(1.0f,  1.0f), // 2
		glm::vec2(0.0f,  1.0f), // 3
		glm::vec2(0.0f,  0.0f), // 0

		glm::vec2(1.0f,  0.0f), // 1
		glm::vec2(1.0f,  1.0f), // 2
		glm::vec2(0.0f,  1.0f), // 3
		glm::vec2(0.0f,  0.0f)  // 0
	};

	// *--------*
	// |\      /|
	// | *----* |
	// | |    | |
	// | *----* |
	// |/      \|
	// *--------*

	// 0--------1
	//  \      /
	//   3----2
	//
	//   5----6
	//  /      \
	// 4--------7

	// 14------13
	// |        |
	// | 9---10 |
	// | |    | |
	// | 8---11 |
	// |        |
	// 15------12

	// 17       22
	// |\      /|
	// | 18   21|
	// | |    | |
	// | 19   20|
	// |/      \|
	// 16      23

	m_normals =
	{
		glm::vec3( 0.0f,  1.0f,  0.0f), // 0
		glm::vec3( 0.0f,  1.0f,  0.0f), // 1
		glm::vec3( 0.0f,  1.0f,  0.0f), // 2
		glm::vec3( 0.0f,  1.0f,  0.0f), // 3

		glm::vec3( 0.0f, -1.0f,  0.0f), // 0
		glm::vec3( 0.0f, -1.0f,  0.0f), // 1
		glm::vec3( 0.0f, -1.0f,  0.0f), // 2
		glm::vec3( 0.0f, -1.0f,  0.0f), // 3

		glm::vec3( 0.0f,  0.0f, -1.0f), // 0
		glm::vec3( 0.0f,  0.0f, -1.0f), // 1
		glm::vec3( 0.0f,  0.0f, -1.0f), // 2
		glm::vec3( 0.0f,  0.0f, -1.0f), // 3

		glm::vec3( 0.0f,  0.0f,  1.0f), // 0
		glm::vec3( 0.0f,  0.0f,  1.0f), // 1
		glm::vec3( 0.0f,  0.0f,  1.0f), // 2
		glm::vec3( 0.0f,  0.0f,  1.0f), // 3

		glm::vec3(-1.0f,  0.0f,  0.0f), // 0
		glm::vec3(-1.0f,  0.0f,  0.0f), // 1
		glm::vec3(-1.0f,  0.0f,  0.0f), // 2
		glm::vec3(-1.0f,  0.0f,  0.0f), // 3

		glm::vec3( 1.0f,  0.0f,  0.0f), // 0
		glm::vec3( 1.0f,  0.0f,  0.0f), // 1
		glm::vec3( 1.0f,  0.0f,  0.0f), // 2
		glm::vec3( 1.0f,  0.0f,  0.0f)  // 3
	};



	m_indices =
	{
		//roof
		0, 1, 2,
		0, 2, 3,

		//floor
		4, 5, 6,
		4, 6, 7,

		//back
		8, 9, 10,
		8, 10, 11,

		//front
		12, 13, 14,
		12, 14, 15,

		//left
		16, 17, 18,
		16, 18, 19,

		//right
		20, 21, 22,
		20, 22, 23
	};

	computeAabb();
	computeFaceNormals();
	computeOutlineNormals();

	//LOG_F("size of: m_vertices: %i size of m_indices: %i", (int)m_vertices.size(), (int)m_indices.size());
}

void Mesh::generateSphere(float radius, int rings, int sectors)
{
	auto pushIndices = [this](int sectors, int r, int s)
	{
		int curRow = r * sectors;
		int nextRow = (r+1) * sectors;
		int nextS = (s+1) % sectors;

		m_indices.push_back(curRow + s);
		m_indices.push_back(nextRow + s);
		m_indices.push_back(nextRow + nextS);

		m_indices.push_back(curRow + s);
		m_indices.push_back(nextRow + nextS);
		m_indices.push_back(curRow + nextS);
	};

	const float R = 1.0f/(float)(rings-1);
	const float S = 1.0f/(float)(sectors-1);

	for (int r = 0; r < rings; ++r)
	{
		for (int s = 0; s < sectors; ++s)
		{
			float y = sinf(float(-M_PI_2) + float(M_PI) * r * R );
			float x = cosf(2 * float(M_PI) * s * S) * sinf(float(M_PI) * r * R );
			float z = sinf(2 * float(M_PI) * s * S) * sinf(float(M_PI) * r * R );

			m_uvs.push_back(vec2(s*S, r*R));
			m_vertices.push_back(vec3(x,y,z) * radius);
			if (r < rings-1)
			{
				pushIndices(sectors, r, s);
			}
		}
	}

	computeFaceNormals();
	computeOutlineNormals();
	computeAabb();
}

void Mesh::generateCone(int steps)
{
	m_vertices.emplace_back(vec3(0.0f, 0.0f, 0.0f));
	m_vertices.emplace_back(vec3(0.0f, 1.0f, 0.0f));
	m_uvs.emplace_back(vec2(0.5f, 0.5f));
	m_uvs.emplace_back(vec2(0.5f, 0.5f));

	float stepAngle = Math::Tau / float(steps);
	for (float angle = 0.0f; angle < Math::Tau; angle += stepAngle)
	{
		m_vertices.emplace_back(vec3(
			sinf(angle),
			0.0f,
			cosf(angle)));

		m_uvs.emplace_back(vec2(
			(sinf(angle) + 1.0f) * 0.5f,
			(cosf(angle) + 1.0f) * 0.5f));
	}

	// Circle
	for (int i = 2; i < (int)m_vertices.size()-1; ++i)
	{
		m_indices.emplace_back(0);
		m_indices.emplace_back(i);
		m_indices.emplace_back(i+1);
	}

	m_indices.emplace_back(0);
	m_indices.emplace_back(m_vertices.size()-1);
	m_indices.emplace_back(2);

	// Cone "circle"
	for (int i = 2; i < (int)m_vertices.size()-1; ++i)
	{
		m_indices.emplace_back(1);
		m_indices.emplace_back(i);
		m_indices.emplace_back(i+1);
	}

	m_indices.emplace_back(1);
	m_indices.emplace_back(m_vertices.size()-1);
	m_indices.emplace_back(2);

	computeFaceNormals();
	computeOutlineNormals();
	computeAabb();
}

void Mesh::computeOutlineNormals()
{
	m_normalsForOutline.clear();

	m_normalsForOutline = computeSmoothNormals();
}

/* RAE_TODO draft:
void Mesh::removeDuplicateVertices()
{
	struct UniqueVec
	{
		UniqueVec(const vec3 value, int index) :
			value(value)
		{
			indices.emplace_back(index);
		}

		vec3 value;
		Array<int> indices;
	};

	Array<UniqueVertex> uniqueVertices;

	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		bool isUnique = true;
		for (int j = 0; j < (int)uniqueVertices.size(); ++j)
		{
			auto&& uniqueVec = uniqueVertices[j];
			if (Utils::isEqualVec(m_vertices[i], uniqueVec.value))
			{
				uniqueVec.indices.emplace_back(i);
				isUnique = false;
				break;
			}
		}

		if (isUnique)
		{
			uniqueVertices.emplace_back(m_vertices[i], i);
		}
	}
}
*/

Array<vec3> Mesh::computeSmoothNormals()
{
	Array<vec3> normals;
	normals.reserve((int)m_vertices.size());
	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		normals.emplace_back(0.0f, 1.0f, 0.0f);
	}

	// Find duplicate vecs from the faces. Get angle weigths for each normal, and add them together.

	struct UniqueVec
	{
		UniqueVec(const vec3& value, int vertexIndex, const vec3& normal, float angleWeight) :
			value(value)
		{
			vertexIndices.emplace_back(vertexIndex);
			vertexNormals.emplace_back(normal);
			angleWeights.emplace_back(angleWeight);
		}

		vec3 value;
		Array<int> vertexIndices;
		Array<vec3> vertexNormals;
		Array<float> angleWeights;
		vec3 finalNormal;
	};

	Array<UniqueVec> uniqueVertices;

	auto addToPossiblyUniqueVectors = [&uniqueVertices](
		const vec3& value,
		int vertexIndex,
		const vec3& normal,
		float angleWeight)
	{
		bool isUnique = true;
		for (int j = 0; j < (int)uniqueVertices.size(); ++j)
		{
			auto&& uniqueVec = uniqueVertices[j];
			if (Utils::isEqualVec(value, uniqueVec.value))
			{
				uniqueVec.vertexIndices.emplace_back(vertexIndex);
				uniqueVec.vertexNormals.emplace_back(normal);
				uniqueVec.angleWeights.emplace_back(angleWeight);

				isUnique = false;
				break;
			}
		}

		if (isUnique)
		{
			uniqueVertices.emplace_back(value, vertexIndex, normal, angleWeight);
		}
	};

	vec3 a;
	vec3 b;
	vec3 c;

	vec3 v1;
	vec3 v2;

	vec3 u;
	vec3 v;
	vec3 result;

	for (int i = 0; i < (int)m_indices.size(); i = i + 3)
	{
		a = m_vertices[m_indices[i]];
		b = m_vertices[m_indices[i+1]];
		c = m_vertices[m_indices[i+2]];

		v1[0] = a[0] - b[0];
		v1[1] = a[1] - b[1];
		v1[2] = a[2] - b[2];

		v2[0] = b[0] - c[0];
		v2[1] = b[1] - c[1];
		v2[2] = b[2] - c[2];

		// Cross product:
		result[0] = v1[1] * v2[2] - v1[2] * v2[1];
		result[1] = v1[2] * v2[0] - v1[0] * v2[2];
		result[2] = v1[0] * v2[1] - v1[1] * v2[0];

		// Normalizing here will ignore the surface area of the triangle.
		//result = glm::normalize(result);

		m_normals[m_indices[i]] = result;
		m_normals[m_indices[i+1]] = result;
		m_normals[m_indices[i+2]] = result;

		float angleA = glm::angle(glm::normalize(b - a), glm::normalize(c - a));
		float angleB = glm::angle(glm::normalize(c - b), glm::normalize(a - b));
		float angleC = glm::angle(glm::normalize(a - c), glm::normalize(b - c));

		addToPossiblyUniqueVectors(a, m_indices[i], result, angleA);
		addToPossiblyUniqueVectors(b, m_indices[i+1], result, angleB);
		addToPossiblyUniqueVectors(c, m_indices[i+2], result, angleC);
	}

	// Process
	for (int j = 0; j < (int)uniqueVertices.size(); ++j)
	{
		auto&& uniqueVec = uniqueVertices[j];
		uniqueVec.finalNormal = vec3(0,0,0);

		//RAE_TODO assert
		if ((int)uniqueVec.vertexNormals.size() != (int)uniqueVec.angleWeights.size())
		{
			LOG_F(INFO, "Error: computeSmoothNormals. uniqueVec. vertexNormals should be the same size as angleWeights.");
			continue; // Should assert here.
		}

		for (int k = 0; k < (int)uniqueVec.vertexNormals.size(); ++k)
		{
			uniqueVec.finalNormal += uniqueVec.vertexNormals[k] * uniqueVec.angleWeights[k];
		}

		uniqueVec.finalNormal = glm::normalize(uniqueVec.finalNormal);

		// Copy to normals
		for (int k = 0; k < (int)uniqueVec.vertexIndices.size(); ++k)
		{
			normals[uniqueVec.vertexIndices[k]] = uniqueVec.finalNormal;
		}
	}

	return normals;
}

void Mesh::computeFaceNormals()
{
	// TEMP just enough normals to get by:
	m_normals.clear();

	for (int i = 0; i < (int)m_vertices.size(); ++i)
	{
		m_normals.emplace_back(0.0f, 1.0f, 0.0f);
	}

	vec3 a;
	vec3 b;
	vec3 c;

	vec3 v1;
	vec3 v2;

	vec3 u;
	vec3 v;
	vec3 result;

	for (int i = 0; i < (int)m_indices.size(); i = i + 3)
	{
		/*a = m_vertices[m_indices[i]];
		b = m_vertices[m_indices[i+1]];
		c = m_vertices[m_indices[i+2]];

		u = b - a;
		v = c - a;

		result.x = (u.y * v.z) - (u.z * v.y);
		result.y = (u.z * v.x) - (u.x * v.z);
		result.z =	(	u.x * v.y) - (u.y * v.x);

		result = glm::normalize(result);
		m_normals[i] = result;
	}*/

		a = m_vertices[m_indices[i]];
		b = m_vertices[m_indices[i+1]];
		c = m_vertices[m_indices[i+2]];

		v1[0] = a[0] - b[0];
		v1[1] = a[1] - b[1];
		v1[2] = a[2] - b[2];

		v2[0] = b[0] - c[0];
		v2[1] = b[1] - c[1];
		v2[2] = b[2] - c[2];

		// Cross product:
		result[0] = v1[1] * v2[2] - v1[2] * v2[1];
		result[1] = v1[2] * v2[0] - v1[0] * v2[2];
		result[2] = v1[0] * v2[1] - v1[1] * v2[0];

		result = glm::normalize(result);

		m_normals[m_indices[i]] = result;
		m_normals[m_indices[i+1]] = result;
		m_normals[m_indices[i+2]] = result;
	}


		/* Copy face normals into vertex normals:
		if ((f1[0]*3)+2 < m_vertexNormals.length)
		{
			m_vertexNormals[(f1[0]*3)+0] = result[0];
			m_vertexNormals[(f1[0]*3)+1] = result[1];
			m_vertexNormals[(f1[0]*3)+2] = result[2];
		}

		if ((f1[1]*3)+2 < m_vertexNormals.length)
		{
			m_vertexNormals[(f1[1]*3)+0] = result[0];
			m_vertexNormals[(f1[1]*3)+1] = result[1];
			m_vertexNormals[(f1[1]*3)+2] = result[2];
		}

		if ((f1[2]*3)+2 < m_vertexNormals.length)
		{
			m_vertexNormals[(f1[2]*3)+0] = result[0];
			m_vertexNormals[(f1[2]*3)+1] = result[1];
			m_vertexNormals[(f1[2]*3)+2] = result[2];
		}
		*/
}

void Mesh::generateLinesFromVertices(const Array<vec3>& vertices)
{
	m_vertices.clear();
	m_uvs.clear();
	m_normals.clear();
	m_indices.clear();

	//int i = 0;
	//int index = 0;
	for (auto&& vertex : vertices)
	{
		m_vertices.emplace_back(vertex);
		m_uvs.emplace_back(vec2(0.0f, 0.0f));
		m_normals.emplace_back(vec3(0.0f, 1.0f, 0.0f));

		/*m_indices.emplace_back(index);
		if (i % 2 == 0)
		{
			index++;
		}
		++i;
		*/
	}

	int index = 0;
	for (int i = 0; i < (((int)m_vertices.size())-1)*2; ++i)
	{
		m_indices.emplace_back(index);
		if (i % 2 == 0)
		{
			index++;
		}
	}
}

//ASSIMP
bool Mesh::loadModel(const String& filepath)
{
	Assimp::Importer importer;

	//Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;
	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
	// Create a logger instance for Console Output
	Assimp::DefaultLogger::create("",severity, aiDefaultLogStream_STDOUT);
	// And another for file output
	Assimp::DefaultLogger::create("assimp_log.txt",severity, aiDefaultLogStream_FILE);
	Assimp::DefaultLogger::get()->info("this is my info-call");

	const aiScene* scene = nullptr;

	//check if file exists
	std::ifstream fin(filepath.c_str());
	if(!fin.fail())
	{
		fin.close();
	}
	else
	{
		LOG_F(ERROR, "Couldn't open file: %s", filepath.c_str());
		LOG_F(ERROR, importer.GetErrorString());
		return false;
	}

	scene = importer.ReadFile( filepath, /*aiProcessPreset_TargetRealtime_Quality*/0);

	if( !scene )
	{
		LOG_F(ERROR, importer.GetErrorString());
		return false;
	}

	loadNode(scene, scene->mRootNode);

	// Aabb already computed inside loadNode because we need it for UV computation
	//computeAabb();
	createVBOs();

	LOG_F(INFO, "Succesfully imported scene %s", filepath.c_str());
	return true;
}

void Mesh::loadNode(const aiScene* scene, const aiNode* node)
{
	//LOG_F(INFO, "Node mesh count: %i", node->mNumMeshes);

	if (node->mNumMeshes > 0)
	{

		for (uint i = 0; i < node->mNumMeshes; ++i)
		{
			//LOG_F(INFO, "Node: %i", i);

			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			uint f;
			for (f = 0; f < mesh->mNumFaces; ++f)
			{

			}

			//LOG_F(INFO, "Faces: %i", f);

			for (uint n = 0; n < node->mNumChildren; ++n)
			{
				loadNode(scene, node->mChildren[n]);
			}

		}
	}
	else
	{
		const aiMesh* mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

		//LOG_F(INFO, "m_vertices: %i", mesh->mNumVertices);
		//LOG_F(INFO, "faces: %i", mesh->mNumFaces);

		if (mesh->HasTextureCoords(0) == false)
		{
			//LOG_F(INFO, "No texture coordinates in mesh.");
		}

		m_aabb.clear();

		// Fill m_vertices positions
		m_vertices.reserve(mesh->mNumVertices);
		for (uint i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D pos = mesh->mVertices[i];

			// Test swapping Y and Z:
			//m_vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
			m_vertices.push_back(glm::vec3(pos.x, pos.z, pos.y));


			m_aabb.grow(m_vertices[i]);
		}

		// Fill m_vertices texture coordinates
		m_uvs.reserve(mesh->mNumVertices);
		for (uint i = 0; i < mesh->mNumVertices; i++)
		{
			if(mesh->HasTextureCoords(0))
			{
				aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
				m_uvs.push_back(glm::vec2(UVW.x, UVW.y));
			}
			else
			{
				// really TEMP m_uvs
				//m_uvs.push_back(glm::vec2(float(i) / float(mesh->mNumVertices), float(i) / float(mesh->mNumVertices)));

				m_uvs.push_back(glm::vec2((m_vertices[i].x - m_aabb.min().x) / m_aabb.dimensions().x, (m_vertices[i].y - m_aabb.min().y) / m_aabb.dimensions().y));
			}
		}

		// Fill m_vertices m_normals
		m_normals.reserve(mesh->mNumVertices);
		for (uint i = 0; i<mesh->mNumVertices; i++)
		{
			if(mesh->HasNormals())
			{
				aiVector3D n = mesh->mNormals[i];

				// Test swapping Y and Z
				//m_normals.push_back(glm::vec3(n.x, n.y, n.z));
				m_normals.push_back(glm::vec3(n.x, n.z, n.y));
			}
			else
			{
				// really TEMP m_normals
				m_normals.push_back(glm::normalize( glm::vec3(float(i) / float(mesh->mNumVertices), float(i) / float(mesh->mNumVertices), 1.0f) ));
			}
		}

		// Fill face m_indices
		m_indices.reserve(3*mesh->mNumFaces);
		for (uint i = 0; i<mesh->mNumFaces; i++)
		{
			// Assume the model has only triangles.
			m_indices.push_back(mesh->mFaces[i].mIndices[0]);
			m_indices.push_back(mesh->mFaces[i].mIndices[1]);
			m_indices.push_back(mesh->mFaces[i].mIndices[2]);
		}
	}
}
//end // ASSIMP

void Mesh::render(uint shaderProgramId) const
{
	// Get a handle for our buffers
	GLuint vertexPositionId		= glGetAttribLocation(shaderProgramId, "inPosition");
	GLuint vertexUvId			= glGetAttribLocation(shaderProgramId, "inUV");
	GLuint vertexNormalId		= glGetAttribLocation(shaderProgramId, "inNormal");

	// m_vertices
	glEnableVertexAttribArray(vertexPositionId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glVertexAttribPointer(
		vertexPositionId,	// The attribute we want to configure
		3,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride
		(void*)0			// array buffer offset
	);

	// m_UVs
	glEnableVertexAttribArray(vertexUvId);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvBufferId);
	glVertexAttribPointer(
		vertexUvId,			// The attribute we want to configure
		2,					// size : U+V => 2
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride
		(void*)0			// array buffer offset
	);

	// m_normals
	glEnableVertexAttribArray(vertexNormalId);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferId);
	glVertexAttribPointer(
		vertexNormalId,		// The attribute we want to configure
		3,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride
		(void*)0			// array buffer offset
	);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);

	glDrawElements(
		GL_TRIANGLES,
		(GLsizei)m_indices.size(),
		GL_UNSIGNED_SHORT,
		(void*)0
	);

	glDisableVertexAttribArray(vertexPositionId);
	glDisableVertexAttribArray(vertexUvId);
	glDisableVertexAttribArray(vertexNormalId);
}

void Mesh::renderForOutline(uint shaderProgramId) const
{
	// Get a handle for our buffers
	GLuint vertexPositionId		= glGetAttribLocation(shaderProgramId, "inPosition");
	GLuint vertexUvId			= glGetAttribLocation(shaderProgramId, "inUV");
	GLuint vertexNormalId		= glGetAttribLocation(shaderProgramId, "inNormal");

	// m_vertices
	glEnableVertexAttribArray(vertexPositionId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glVertexAttribPointer(
		vertexPositionId,	// The attribute we want to configure
		3,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride
		(void*)0			// array buffer offset
	);

	// m_UVs
	glEnableVertexAttribArray(vertexUvId);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvBufferId);
	glVertexAttribPointer(
		vertexUvId,			// The attribute we want to configure
		2,					// size : U+V => 2
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride
		(void*)0			// array buffer offset
	);

	// m_normals
	if (!m_normalsForOutline.empty())
	{
		glEnableVertexAttribArray(vertexNormalId);
		glBindBuffer(GL_ARRAY_BUFFER, m_outlineNormalBufferId);
		glVertexAttribPointer(
			vertexNormalId,		// The attribute we want to configure
			3,					// size
			GL_FLOAT,			// type
			GL_FALSE,			// normalized?
			0,					// stride
			(void*)0			// array buffer offset
		);
	}
	else
	{
		glEnableVertexAttribArray(vertexNormalId);
		glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferId);
		glVertexAttribPointer(
			vertexNormalId,		// The attribute we want to configure
			3,					// size
			GL_FLOAT,			// type
			GL_FALSE,			// normalized?
			0,					// stride
			(void*)0			// array buffer offset
		);
	}

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);

	glDrawElements(
		GL_TRIANGLES,
		(GLsizei)m_indices.size(),
		GL_UNSIGNED_SHORT,
		(void*)0
	);

	glDisableVertexAttribArray(vertexPositionId);
	glDisableVertexAttribArray(vertexUvId);
	glDisableVertexAttribArray(vertexNormalId);
}

void Mesh::renderLines(uint shaderProgramId) const
{
	// Get a handle for our buffers
	GLuint vertexPositionId		= glGetAttribLocation(shaderProgramId, "inPosition");
	GLuint vertexUvId			= glGetAttribLocation(shaderProgramId, "inUV");
	GLuint vertexNormalId		= glGetAttribLocation(shaderProgramId, "inNormal");

	// m_vertices
	glEnableVertexAttribArray(vertexPositionId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glVertexAttribPointer(
		vertexPositionId,	// The attribute we want to configure
		3,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride
		(void*)0			// array buffer offset
	);

	// m_UVs
	glEnableVertexAttribArray(vertexUvId);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvBufferId);
	glVertexAttribPointer(
		vertexUvId,			// The attribute we want to configure
		2,					// size : U+V => 2
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride
		(void*)0			// array buffer offset
	);

	// m_normals
	glEnableVertexAttribArray(vertexNormalId);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferId);
	glVertexAttribPointer(
		vertexNormalId,		// The attribute we want to configure
		3,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride
		(void*)0			// array buffer offset
	);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);

	glDrawElements(
		GL_LINES,
		(GLsizei)m_indices.size(),
		GL_UNSIGNED_SHORT,
		(void*)0
	);

	glDisableVertexAttribArray(vertexPositionId);
	glDisableVertexAttribArray(vertexUvId);
	glDisableVertexAttribArray(vertexNormalId);
}
