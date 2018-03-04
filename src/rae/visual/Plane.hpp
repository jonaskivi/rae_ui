#pragma once

#include <glm/glm.hpp>

namespace rae
{

using vec3 = glm::vec3;

class Plane
{
public:
	Plane() {}
	Plane(const vec3& origin, const vec3& normal)
	{
		m_origin = origin;
		m_normal = normal;
	}

	vec3 origin() const { return m_origin; }
	vec3 normal() const { return m_normal; }
	void setNormal(const vec3& normal) { m_normal = normal; }

protected:
	vec3 m_origin;
	vec3 m_normal;
};

}
