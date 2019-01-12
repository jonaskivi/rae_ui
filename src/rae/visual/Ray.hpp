#pragma once

#include <glm/glm.hpp>

namespace rae
{

using vec3 = glm::vec3;

class Ray
{
public:
	Ray() {}
	Ray(const vec3& origin, const vec3& direction)
	{
		m_origin = origin;
		m_direction = direction;
	}

	const vec3& origin() const { return m_origin; }
	const vec3& direction() const { return m_direction; }
	void setDirection(vec3 direction) { m_direction = direction; }
	void setOrigin(vec3 origin) { m_origin = origin; }
	void moveOrigin(vec3 delta) { m_origin += delta; }
	vec3 pointAtParameter(float t) const { return m_origin + t * m_direction; }

protected:
	vec3 m_origin;
	vec3 m_direction;
};

}
