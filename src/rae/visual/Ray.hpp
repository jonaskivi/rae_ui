#pragma once

#include <glm/glm.hpp>

namespace rae
{

using vec3 = glm::vec3;

class Ray
{
public:
	Ray(){}
	Ray(const vec3& set_origin, const vec3& set_direction)
	{
		a = set_origin;
		b = set_direction;
	}

	vec3 origin() const { return a; }
	vec3 direction() const { return b; }
	void setDirection(vec3 set_direction) { b = set_direction; }
	vec3 point_at_parameter(float t) const { return a + t * b; }

	vec3 a;
	vec3 b;
};

}
