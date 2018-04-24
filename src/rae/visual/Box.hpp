#pragma once

#include "rae/core/Types.hpp"
#include "rae/scene/Transform.hpp"

namespace rae
{

class Ray;
struct Transform;

class Box
{
public:
	Box() :
		m_min(FLT_MAX, FLT_MAX, FLT_MAX),
		m_max(FLT_MIN, FLT_MIN, FLT_MIN)
	{
	}

	Box(const vec3& setMin, const vec3& setMax) :
		m_min(setMin),
		m_max(setMax)
	{
	}

	void clear()
	{
		m_min = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		m_max = vec3(FLT_MIN, FLT_MIN, FLT_MIN);
	}

	bool valid()
	{
		if (m_min.x <= m_max.x &&
			m_min.y <= m_max.y &&
			m_min.z <= m_max.z)
		{
			return true;
		}
		return false;
	}

	void init(const Box& left, const Box& right);
	void grow(const Box& set);
	void grow(vec3 set);

	vec3 dimensions() const
	{
		return m_max - m_min;
	}

	void transform(const Transform& tr);

	const vec3& min() const { return m_min; }
	const vec3& max() const { return m_max; }

	float left() { return m_min.x; }
	float right() { return m_max.x; }
	float up() { return m_max.y; }
	float down() { return m_min.y; }

	// 3D hit test
	bool hit(const Ray& ray, float minDistance, float maxDistance) const;
	// 2D hit test
	bool hit(vec2 position) const;

protected:
	vec3 m_min;
	vec3 m_max;
};

}
