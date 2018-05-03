#pragma once

#include "rae/core/Types.hpp"

namespace rae
{

struct Transform
{
public:
	Transform(){}

	Transform(const vec3& position);
	String toString() const;

	vec3	position		= vec3(0.0f, 0.0f, 0.0f);
	qua		rotation;
	vec3	scale			= vec3(1.0f, 1.0f, 1.0f);
};

}
