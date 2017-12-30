
#include "GL/glew.h"
#include "Mesh.hpp"

#include <iostream>
#include <fstream>

#include "rae/visual/Material.hpp"

using namespace rae;

Mesh::Mesh()
{
	//m_material = new Lambertian(vec3(0.1f, 0.2f, 0.7f));
	m_material = new Metal(vec3(0.1f, 0.2f, 0.7f), 0.3f);
}

Mesh::~Mesh()
{
	std::cout << "Mesh destructor.\n";
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

void Mesh::createVBOs()
{
	std::cout << "Mesh::createVBOs.\n";

	glGenBuffers(1, &m_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_uvBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_normalBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLushort), &m_indices[0], GL_STATIC_DRAW);
}

void Mesh::freeVBOs()
{
	if (m_vertexBufferId == 0 &&
		m_uvBufferId == 0 &&
		m_normalBufferId == 0 &&
		m_indexBufferId == 0)
	{
		std::cout << "Mesh::freeVBOs, but no resources created.\n";
		return;
	}

	std::cout << "Mesh::freeVBOs.\n";
	glDeleteBuffers(1, &m_vertexBufferId);
	glDeleteBuffers(1, &m_uvBufferId);
	glDeleteBuffers(1, &m_normalBufferId);
	glDeleteBuffers(1, &m_indexBufferId);

	m_vertexBufferId	= 0;
	m_uvBufferId		= 0;
	m_normalBufferId	= 0;
	m_indexBufferId	= 0;
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

bool Mesh::hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const
{
	if (m_aabb.hit(ray, t_min, t_max) == false)
		return false;

	vec3 v0, v1, v2;
	float u, v;
	float hitDistance;

	bool isHit = false;

	for (int i = 0; i < triangleCount(); ++i)
	{
		getTriangle(i, v0, v1, v2);

		if (rayTriangleIntersection(ray.origin(), ray.direction(), v0, v1, v2, hitDistance, u, v)
			&& hitDistance < t_max
			&& hitDistance > t_min)
		{
			isHit = true;
			record.t = hitDistance;
			record.point = ray.point_at_parameter(record.t);
			record.normal = getFaceNormal(i); // currently just face m_normals
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

//version without initializer lists (vs2012):
void Mesh::generateBox()
{
	//std::cout<<"Generating Mesh.\n";

	float boxSize = 0.5f; // Actually half of the box size

	m_vertices.push_back( glm::vec3(-boxSize,  boxSize,  boxSize) ); // 0
	m_vertices.push_back( glm::vec3( boxSize,  boxSize,  boxSize) ); // 1
	m_vertices.push_back( glm::vec3( boxSize,  boxSize, -boxSize) ); // 2
	m_vertices.push_back( glm::vec3(-boxSize,  boxSize, -boxSize) ); // 3

	m_vertices.push_back( glm::vec3(-boxSize, -boxSize,  boxSize) ); // 4
	m_vertices.push_back( glm::vec3(-boxSize, -boxSize, -boxSize) ); // 5
	m_vertices.push_back( glm::vec3( boxSize, -boxSize, -boxSize) ); // 6
	m_vertices.push_back( glm::vec3( boxSize, -boxSize,  boxSize) ); // 7

	m_vertices.push_back( glm::vec3(-boxSize, -boxSize, -boxSize) ); // 8
	m_vertices.push_back( glm::vec3(-boxSize,  boxSize, -boxSize) ); // 9
	m_vertices.push_back( glm::vec3( boxSize,  boxSize, -boxSize) ); // 10
	m_vertices.push_back( glm::vec3( boxSize, -boxSize, -boxSize) ); // 11
		
	m_vertices.push_back( glm::vec3( boxSize, -boxSize,  boxSize) ); // 12
	m_vertices.push_back( glm::vec3( boxSize,  boxSize,  boxSize) ); // 13
	m_vertices.push_back( glm::vec3(-boxSize,  boxSize,  boxSize) ); // 14
	m_vertices.push_back( glm::vec3(-boxSize, -boxSize,  boxSize) ); // 15

	m_vertices.push_back( glm::vec3(-boxSize, -boxSize,  boxSize) ); // 16
	m_vertices.push_back( glm::vec3(-boxSize,  boxSize,  boxSize) ); // 17
	m_vertices.push_back( glm::vec3(-boxSize,  boxSize, -boxSize) ); // 18
	m_vertices.push_back( glm::vec3(-boxSize, -boxSize, -boxSize) ); // 19
		
	m_vertices.push_back( glm::vec3( boxSize, -boxSize, -boxSize) ); // 20
	m_vertices.push_back( glm::vec3( boxSize,  boxSize, -boxSize) ); // 21
	m_vertices.push_back( glm::vec3( boxSize,  boxSize,  boxSize) ); // 22
	m_vertices.push_back( glm::vec3( boxSize, -boxSize,  boxSize) );// 23

	m_uvs.push_back( glm::vec2( 1.0f,  0.0f) ); // 1
	m_uvs.push_back( glm::vec2( 1.0f,  1.0f) ); // 2
	m_uvs.push_back( glm::vec2( 0.0f,  1.0f) ); // 3
	m_uvs.push_back( glm::vec2( 0.0f,  0.0f) ); // 0
		
	m_uvs.push_back( glm::vec2( 1.0f,  0.0f) ); // 1
	m_uvs.push_back( glm::vec2( 1.0f,  1.0f) ); // 2
	m_uvs.push_back( glm::vec2( 0.0f,  1.0f) ); // 3
	m_uvs.push_back( glm::vec2( 0.0f,  0.0f) ); // 0

	m_uvs.push_back( glm::vec2( 1.0f,  0.0f) ); // 1
	m_uvs.push_back( glm::vec2( 1.0f,  1.0f) ); // 2
	m_uvs.push_back( glm::vec2( 0.0f,  1.0f) ); // 3
	m_uvs.push_back( glm::vec2( 0.0f,  0.0f) ); // 0

	m_uvs.push_back( glm::vec2( 1.0f,  0.0f) ); // 1
	m_uvs.push_back( glm::vec2( 1.0f,  1.0f) ); // 2
	m_uvs.push_back( glm::vec2( 0.0f,  1.0f) ); // 3
	m_uvs.push_back( glm::vec2( 0.0f,  0.0f) ); // 0

	m_uvs.push_back( glm::vec2( 1.0f,  0.0f) ); // 1
	m_uvs.push_back( glm::vec2( 1.0f,  1.0f) ); // 2
	m_uvs.push_back( glm::vec2( 0.0f,  1.0f) ); // 3
	m_uvs.push_back( glm::vec2( 0.0f,  0.0f) ); // 0

	m_uvs.push_back( glm::vec2( 1.0f,  0.0f) ); // 1
	m_uvs.push_back( glm::vec2( 1.0f,  1.0f) ); // 2
	m_uvs.push_back( glm::vec2( 0.0f,  1.0f) ); // 3
	m_uvs.push_back( glm::vec2( 0.0f,  0.0f) ); // 0

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

	m_normals.push_back( glm::vec3( 0.0f,  1.0f,  0.0f) ); // 0
	m_normals.push_back( glm::vec3( 0.0f,  1.0f,  0.0f) ); // 1
	m_normals.push_back( glm::vec3( 0.0f,  1.0f,  0.0f) ); // 2
	m_normals.push_back( glm::vec3( 0.0f,  1.0f,  0.0f) ); // 3
		
	m_normals.push_back( glm::vec3( 0.0f, -1.0f,  0.0f) ); // 0
	m_normals.push_back( glm::vec3( 0.0f, -1.0f,  0.0f) ); // 1
	m_normals.push_back( glm::vec3( 0.0f, -1.0f,  0.0f) ); // 2
	m_normals.push_back( glm::vec3( 0.0f, -1.0f,  0.0f) ); // 3

	m_normals.push_back( glm::vec3( 0.0f,  0.0f, -1.0f) ); // 0
	m_normals.push_back( glm::vec3( 0.0f,  0.0f, -1.0f) ); // 1
	m_normals.push_back( glm::vec3( 0.0f,  0.0f, -1.0f) ); // 2
	m_normals.push_back( glm::vec3( 0.0f,  0.0f, -1.0f) ); // 3

	m_normals.push_back( glm::vec3( 0.0f,  0.0f,  1.0f) ); // 0
	m_normals.push_back( glm::vec3( 0.0f,  0.0f,  1.0f) ); // 1
	m_normals.push_back( glm::vec3( 0.0f,  0.0f,  1.0f) ); // 2
	m_normals.push_back( glm::vec3( 0.0f,  0.0f,  1.0f) ); // 3

	m_normals.push_back( glm::vec3(-1.0f,  0.0f,  0.0f) ); // 0
	m_normals.push_back( glm::vec3(-1.0f,  0.0f,  0.0f) ); // 1
	m_normals.push_back( glm::vec3(-1.0f,  0.0f,  0.0f) ); // 2
	m_normals.push_back( glm::vec3(-1.0f,  0.0f,  0.0f) ); // 3

	m_normals.push_back( glm::vec3( 1.0f,  0.0f,  0.0f) ); // 0
	m_normals.push_back( glm::vec3( 1.0f,  0.0f,  0.0f) ); // 1
	m_normals.push_back( glm::vec3( 1.0f,  0.0f,  0.0f) ); // 2
	m_normals.push_back( glm::vec3( 1.0f,  0.0f,  0.0f) );// 3
	
	//roof
	m_indices.push_back( 0);
	m_indices.push_back( 1);
	m_indices.push_back( 2);
	m_indices.push_back( 0);
	m_indices.push_back( 2);
	m_indices.push_back( 3);
	
	//floor
	m_indices.push_back( 4);
	m_indices.push_back( 5);
	m_indices.push_back( 6); 
	m_indices.push_back( 4);
	m_indices.push_back( 6);
	m_indices.push_back( 7); 

	//back
	m_indices.push_back( 8);
	m_indices.push_back( 9);
	m_indices.push_back(10);
	m_indices.push_back( 8);
	m_indices.push_back(10);
	m_indices.push_back(11);

	//front
	m_indices.push_back(12);
	m_indices.push_back(13);
	m_indices.push_back(14);
	m_indices.push_back(12);
	m_indices.push_back(14);
	m_indices.push_back(15);

	//left
	m_indices.push_back(16);
	m_indices.push_back(17);
	m_indices.push_back(18);
	m_indices.push_back(16);
	m_indices.push_back(18);
	m_indices.push_back(19);

	//right
	m_indices.push_back(20);
	m_indices.push_back(21);
	m_indices.push_back(22);
	m_indices.push_back(20);
	m_indices.push_back(22);
	m_indices.push_back(23);

	computeAabb();

	//std::cout << "size of: m_vertices: " << m_vertices.size() << " size of m_indices: " << m_indices.size() << "\n";
}

void Mesh::computeAabb()
{
	m_aabb.clear();
	for(int i = 0; i < (int)m_vertices.size(); ++i)
	{
		m_aabb.grow(m_vertices[i]);
	}
}

/*
// C++11 version. TODO fix m_UVs in this version to be the same as above

void Mesh::generateBox()
{
	std::cout<<"Generating Mesh.\n";

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
		glm::vec3( boxSize, -boxSize,  boxSize) // 23
	};

	m_uvs =
	{
		glm::vec2( 0.0f,  0.0f), // 0
		glm::vec2( 1.0f,  0.0f), // 1
		glm::vec2( 1.0f,  1.0f), // 2
		glm::vec2( 0.0f,  1.0f), // 3
		
		glm::vec2( 0.0f,  0.0f), // 0
		glm::vec2( 1.0f,  0.0f), // 1
		glm::vec2( 1.0f,  1.0f), // 2
		glm::vec2( 0.0f,  1.0f), // 3

		glm::vec2( 0.0f,  0.0f), // 0
		glm::vec2( 1.0f,  0.0f), // 1
		glm::vec2( 1.0f,  1.0f), // 2
		glm::vec2( 0.0f,  1.0f), // 3

		glm::vec2( 0.0f,  0.0f), // 0
		glm::vec2( 1.0f,  0.0f), // 1
		glm::vec2( 1.0f,  1.0f), // 2
		glm::vec2( 0.0f,  1.0f), // 3

		glm::vec2( 0.0f,  0.0f), // 0
		glm::vec2( 1.0f,  0.0f), // 1
		glm::vec2( 1.0f,  1.0f), // 2
		glm::vec2( 0.0f,  1.0f), // 3

		glm::vec2( 0.0f,  0.0f), // 0
		glm::vec2( 1.0f,  0.0f), // 1
		glm::vec2( 1.0f,  1.0f), // 2
		glm::vec2( 0.0f,  1.0f) // 3
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
		
		glm::vec3( 0.0f,  -1.0f,  0.0f), // 0
		glm::vec3( 0.0f,  -1.0f,  0.0f), // 1
		glm::vec3( 0.0f,  -1.0f,  0.0f), // 2
		glm::vec3( 0.0f,  -1.0f,  0.0f), // 3

		glm::vec3( 0.0f,  0.0f,  -1.0f), // 0
		glm::vec3( 0.0f,  0.0f,  -1.0f), // 1
		glm::vec3( 0.0f,  0.0f,  -1.0f), // 2
		glm::vec3( 0.0f,  0.0f,  -1.0f), // 3

		glm::vec3( 0.0f,  0.0f,  1.0f), // 0
		glm::vec3( 0.0f,  0.0f,  1.0f), // 1
		glm::vec3( 0.0f,  0.0f,  1.0f), // 2
		glm::vec3( 0.0f,  0.0f,  1.0f), // 3

		glm::vec3( -1.0f,  0.0f,  0.0f), // 0
		glm::vec3( -1.0f,  0.0f,  0.0f), // 1
		glm::vec3( -1.0f,  0.0f,  0.0f), // 2
		glm::vec3( -1.0f,  0.0f,  0.0f), // 3

		glm::vec3( 1.0f,  0.0f,  0.0f), // 0
		glm::vec3( 1.0f,  0.0f,  0.0f), // 1
		glm::vec3( 1.0f,  0.0f,  0.0f), // 2
		glm::vec3( 1.0f,  0.0f,  0.0f) // 3
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

	std::cout<<"size of: m_vertices: "<<m_vertices.size()<<" size of m_indices: "<<m_indices.size()<<"\n";	
}
*/

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
		std::cout << "Couldn't open file: " << filepath << "\n";
		//logInfo( importer.GetErrorString());
		std::cout << importer.GetErrorString() << "\n";
		return false;
	}

	scene = importer.ReadFile( filepath, /*aiProcessPreset_TargetRealtime_Quality*/0);

	if( !scene )
	{
		//logInfo( importer.GetErrorString());
		std::cout << importer.GetErrorString() << "\n";
		return false;
	}

	loadNode(scene, scene->mRootNode);

	// Aabb already computed inside loadNode because we need it for UV computation
	//computeAabb();
	createVBOs();

	std::cout << "Succesfully imported scene " << filepath << "\n";
	return true;
}

void Mesh::loadNode(const aiScene* scene, const aiNode* node)
{
	std::cout << "Node mesh count: " << node->mNumMeshes << "\n";

	if (node->mNumMeshes > 0)
	{

		for (uint i = 0; i < node->mNumMeshes; ++i)
		{
			std::cout << "Node: " << i << "\n";

			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			uint f;
			for (f = 0; f < mesh->mNumFaces; ++f)
			{
			
			}

			std::cout << "Faces: " << f << "\n";

			for (uint n = 0; n < node->mNumChildren; ++n)
			{
				loadNode(scene, node->mChildren[n]);
			}

		}
	}
	else
	{
		const aiMesh* mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

		std::cout << "m_vertices: " << mesh->mNumVertices << "\n";
		std::cout << "faces: " << mesh->mNumFaces << "\n";
		
		if (mesh->HasTextureCoords(0) == false)
		{
			std::cout << "no texture coordinates in mesh.\n";
		}

		m_aabb.clear();

		// Fill m_vertices positions
		m_vertices.reserve(mesh->mNumVertices);
		for (uint i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D pos = mesh->mVertices[i];
			m_vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
			m_aabb.grow(m_vertices[i]);
		}

		// Fill m_vertices texture coordinates
		m_uvs.reserve(mesh->mNumVertices);
		for (uint i = 0; i < mesh->mNumVertices; i++)
		{
			//REMOVE cout <<"uv: " << i <<"\n";
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
				m_normals.push_back(glm::vec3(n.x, n.y, n.z));
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
