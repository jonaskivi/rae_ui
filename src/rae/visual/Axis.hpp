#pragma once

#include "rae/core/Types.hpp"

namespace rae
{

class Plane;

enum class Axis
{
	X,
	Y,
	Z,
	Count
};

vec3 axisVector(Axis axis);
Color axisColor(Axis axis);
qua axisRotation(Axis axis);
Plane axisPlane(Axis axis, vec3 origin);
Plane computeMostPerpendicularAxisPlane(Axis axis, const vec3& gizmoOrigin, const vec3& rayDirection);

}
