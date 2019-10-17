#pragma once

#include "rae/core/Types.hpp"

namespace rae
{

class Plane;
class LineSegment;

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
	void setDirection(const vec3& direction) { m_direction = direction; }
	void setOrigin(const vec3& origin) { m_origin = origin; }
	void moveOrigin(const vec3& delta) { m_origin += delta; }
	// Return the point on the ray at the distance t from origin towards direction.
	vec3 getPointAt(float t) const { return m_origin + t * m_direction; }

	vec3 closestPoint(const vec3& targetPoint, float& d) const;
	vec3 closestPoint(const LineSegment& other, float& d, float& d2) const;

protected:
	vec3 m_origin;
	vec3 m_direction;
};

bool rayPlaneIntersection(const Ray& ray, const Plane& plane, vec3& outContactPoint);

// From mathgeolib:
// The first line is specified by two points start0 and end0. The second line is specified by
// two points start1 and end1.
// The implementation of this function follows http://paulbourke.net/geometry/lineline3d/
// Should probably be in Line class like in MathGeoLib.
void closestPointLineLine(
	const vec3& v0,
	const vec3& v10,
	const vec3& v2,
	const vec3& v32,
	float& d, // out distance on first line.
	float& d2 // out distance on second line.
);

}
