#include "rae/visual/Axis.hpp"

#include "rae/core/Math.hpp"
#include "rae/visual/Plane.hpp"

namespace rae
{

vec3 axisVector(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return vec3(1.0f, 0.0f, 0.0f);
		case Axis::Y:
			return vec3(0.0f, 1.0f, 0.0f);
		case Axis::Z:
			return vec3(0.0f, 0.0f, 1.0f);
		default:
			assert(0);
			break;
	}
	return vec3(0.0f, 0.0f, 0.0f);
}

Color axisColor(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return Color(1.0f, 0.0f, 0.0f, 1.0f);
		case Axis::Y:
			return Color(0.0f, 1.0f, 0.0f, 1.0f);
		case Axis::Z:
			return Color(0.0f, 0.0f, 1.0f, 1.0f);
		default:
			assert(0);
			break;
	}
	return Color(1.0f, 1.0f, 1.0f, 1.0f);
}

qua axisRotation(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return qua();
		case Axis::Y:
			return qua(vec3(0.0f, 0.0f, Math::toRadians(90.0f)));
		case Axis::Z:
			return qua(vec3(0.0f, -Math::toRadians(90.0f), 0.0f));
		default:
			assert(0);
			break;
	}
	return qua();
}

}
