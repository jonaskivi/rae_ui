#include "rae/visual/Ray.hpp"
#include "rae/visual/Plane.hpp"

#include <glm/gtx/norm.hpp> // For distance2

namespace rae
{

bool rayPlaneIntersection(const Ray& ray, const Plane& plane, vec3& outContactPoint)
{
	float denom = glm::dot(plane.normal(), ray.direction());
	if (fabs(denom) > 0.0001f)
	{
		float length = glm::dot(plane.origin() - ray.origin(), plane.normal()) / denom;
		if (length >= 0)
		{
			outContactPoint = ray.origin() + (ray.direction() * length);
			return true;
		}
	}
	return false;
}

void closestPointLineLine(
	const vec3& v0,
	const vec3& v10,
	const vec3& v2,
	const vec3& v32,
	float& d, // out distance on first line.
	float& d2) // out distance on second line.
{
	// Make sure they're not close to zero.
	assert(!Math::isZero(v10));
	assert(!Math::isZero(v32));

	vec3 v02 = v0 - v2;
	float d0232 = glm::dot(v02, v32);
	float d3210 = glm::dot(v32, v10);
	float d3232 = glm::dot(v32, v32);
	// Don't call with a zero direction vector.
	assert(d3232 != 0.f);
	float d0210 = glm::dot(v02, v10);
	float d1010 = glm::dot(v10, v10);
	float denom = d1010 * d3232 - d3210 * d3210;
	if (denom != 0.f)
	{
		d = (d0232 * d3210 - d0210 * d3232) / denom;
	}
	else
	{
		d = 0.f;
	}
	d2 = (d0232 + d * d3210) / d3232;
}

vec3 Ray::closestPoint(const vec3& targetPoint, float& d) const
{
	d = std::max(0.0f, glm::dot(targetPoint - m_origin, m_direction));
	return getPointAt(d);
}

vec3 Ray::closestPoint(const LineSegment& other, float& d, float& d2) const
{
	closestPointLineLine(m_origin, m_direction, other.start(), other.end() - other.start(), d, d2);

	if (d < 0.f)
	{
		d = 0.f;
		if (d2 >= 0.f && d2 <= 1.f)
		{
			other.closestPoint(m_origin, d2);
			return m_origin;
		}

		vec3 p;
		float t2;

		if (d2 < 0.f)
		{
			p = other.start();
			t2 = 0.f;
		}
		else // u2 > 1.f
		{
			p = other.end();
			t2 = 1.f;
		}

		vec3 closestPointTemp = closestPoint(p, d);
		vec3 closestPoint2 = other.closestPoint(m_origin, d2);
		if (glm::distance2(closestPointTemp, p) <= glm::distance2(closestPoint2, m_origin))
		{
			d2 = t2;
			return closestPointTemp;
		}
		else
		{
			d = 0.f;
			return m_origin;
		}
	}
	else if (d2 < 0.f)
	{
		d2 = 0.f;
		return closestPoint(other.start(), d);
	}
	else if (d2 > 1.f)
	{
		d2 = 1.f;
		return closestPoint(other.end(), d);
	}
	else
		return getPointAt(d);
}

}
