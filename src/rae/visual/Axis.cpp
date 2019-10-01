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

Plane axisPlane(Axis axis, vec3 origin)
{
	return Plane(origin, axisVector(axis));
}

Plane computeMostPerpendicularAxisPlane(Axis axis, const vec3& gizmoOrigin, const vec3& rayDirection)
{
	Plane plane1;
	Plane plane2;

	// Check which other axis plane is the most perpendicular to the given ray.
	// For X axis consider Y and X axes.
	switch(axis)
	{
		case Axis::X:
			plane1 = axisPlane(Axis::Y, gizmoOrigin);
			plane2 = axisPlane(Axis::Z, gizmoOrigin);
			break;
		case Axis::Y:
			plane1 = axisPlane(Axis::X, gizmoOrigin);
			plane2 = axisPlane(Axis::Z, gizmoOrigin);
			break;
		case Axis::Z:
			plane1 = axisPlane(Axis::X, gizmoOrigin);
			plane2 = axisPlane(Axis::Y, gizmoOrigin);
			break;
		default:
			assert(0);
			break;
	}

	float perpendicular1 = glm::dot(rayDirection, plane1.normal());
	float perpendicular2 = glm::dot(rayDirection, plane2.normal());

	if (fabs(perpendicular1) >= fabs(perpendicular2))
	{
		return plane1;
	}
	else
	{
		return plane2;
	}
}

}
