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

}
