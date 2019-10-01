#pragma once

#include "rae/core/Types.hpp"
#include "rae/core/Math.hpp"

// For circle:
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// For LineSegment
#include "rae/visual/Ray.hpp"
#include <glm/gtx/norm.hpp>

namespace rae
{

class Plane
{
public:
	Plane() {}
	Plane(const vec3& origin, const vec3& normal) :
		m_origin(origin),
		m_normal(normal)
	{
	}

	vec3 origin() const { return m_origin; }
	vec3 normal() const { return m_normal; }
	void setNormal(const vec3& normal) { m_normal = normal; }

	// https://stackoverflow.com/a/41897378
	vec3 project(const vec3& point) const
	{
		vec3 projected = point - (glm::dot(m_normal, (point - m_origin)) * m_normal);
		return projected;
	}

protected:
	vec3 m_origin;
	vec3 m_normal;
};

// A circle or a disc
class Circle : public Plane
{
public:
	Circle() {}
	Circle(const vec3& origin, const vec3& normal, float radius) :
		Plane(origin, normal),
		m_radius(radius)
	{
	}

	// Returns a point on the circle from the given angle (in radians).
	vec3 getPointOnEdge(float angleRadians) const
	{
		vec3 direction = vec3(0.0f, -sinf(angleRadians), cosf(angleRadians));
		vec3 position = m_origin + (glm::toMat3(Math::rotationOnto(vec3(1,0,0), m_normal))
			* (m_radius * direction));
		return position;
	}

	// From mathgeolib. Except the normalize part is different.
	vec3 closestPointToEdge(const vec3& point) const
	{
		vec3 pointOnPlane = Plane::project(point);
		vec3 diff = pointOnPlane - m_origin;
		if (Math::isZero(diff))
		{
			// On the center of the circle, so all points are equally close.
			return getPointOnEdge(0.0f);
		}
		return m_origin + (glm::normalize(diff) * m_radius);
	}

protected:
	float m_radius = 1.0f;
};

// From mathgeolib:
class LineSegment
{
public:
	LineSegment() {}
	LineSegment(const vec3& start, const vec3& end) :
		m_start(start),
		m_end(end)
	{
	}

	vec3 getPointAt(float d) const
	{
		return (1.0f - d) * m_start + d * m_end;
	}

	vec3 closestPoint(const vec3& point, float& d) const
	{
		vec3 dir = m_end - m_start;
		d = Math::clamp(glm::dot(point - m_start, dir) / glm::length2(dir), 0.0f, 1.0f);
		return m_start + d * dir;
	}

	vec3 closestPoint(const Ray& other, float& d, float& d2) const
	{
		other.closestPoint(*this, d2, d);
		return getPointAt(d);
	}

	vec3 centerPoint() const
	{
		return (m_start + m_end) * 0.5f;
	}

	float length() const
	{
		return glm::distance(m_start, m_end);
	}

	float distance(const Ray& other) const
	{
		float d; // Distance on this.
		float d2; // Distance on other.
		closestPoint(other, d, d2);
		return glm::distance(getPointAt(d), other.getPointAt(d2));
	}

	vec3 start() const { return m_start; }
	vec3 end() const { return m_end; }

protected:

	vec3 m_start;
	vec3 m_end;
};

class Capsule : public LineSegment
{
public:
	Capsule() {}
	Capsule(const vec3& start, const vec3& end, float radius) :
		LineSegment(start, end),
		m_radius(radius)
	{
	}

	bool intersects(const Ray& ray) const
	{
		return LineSegment::distance(ray) <= m_radius;
	}

	vec3 center() const
	{
		return LineSegment::centerPoint();
	}

	float lineLength() const
	{
		return LineSegment::length();
	}

	float diameter() const
	{
		return 2.0f * m_radius;
	}

	float radius() const { return m_radius; }

	vec3 upDirection() const
	{
		vec3 d = m_end - m_start;
		d = glm::normalize(d); // Will always result in a normalized vector, even if start == end.
		return d;
	}

protected:
	float m_radius = 1.0f;
};

}
