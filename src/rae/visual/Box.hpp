#pragma once

#include "rae/core/Types.hpp"
#include "rae/scene/Transform.hpp"

#include <algorithm>

namespace rae
{

class Ray;
struct Transform;

// An Axis-aligned box (AABB)
class Box
{
public:
	Box() :
		m_min(FLT_MAX, FLT_MAX, FLT_MAX),
		m_max(-FLT_MAX, -FLT_MAX, -FLT_MAX)
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
		m_max = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
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

	vec3 halfDimensions() const
	{
		return dimensions() * 0.5f;
	}

	float radius() const
	{
		vec3 hd = halfDimensions();
		return std::max(hd.x, std::max(hd.y, hd.z));
	}

	float width() const { return right() - left(); }
	float height() const { return up() - down(); }

	void transform(const Transform& tr);
	void translate(const Pivot& pivot);

	const vec3& min() const { return m_min; }
	const vec3& max() const { return m_max; }

	float left() const { return m_min.x; }
	float right() const { return m_max.x; }
	float up() const { return m_max.y; }
	float down() const { return m_min.y; }

	// Corners of the AABB box in a right handed Z-up coordinate system.
	//    Z
	//            1,1,1
	//    6---------7
	//    |\        |\
	//    | \       | \
	//    |  4---------5
	//  Y |  |      |  |
	//    2 -|- - - 3  |
	//     \ |       \ |
	//      \|        \|
	//       0---------1   X
	//   -1,-1,-1

	// Return corner point
	vec3 corner(int i) const
	{
		switch(i)
		{
			default:
			case 0: return m_min;
			case 1: return vec3(m_max.x, m_min.y, m_min.z);
			case 2: return vec3(m_min.x, m_max.y, m_min.z);
			case 3: return vec3(m_max.x, m_max.y, m_min.z);
			case 4: return vec3(m_min.x, m_min.y, m_max.z);
			case 5: return vec3(m_max.x, m_min.y, m_max.z);
			case 6: return vec3(m_min.x, m_max.y, m_max.z);
			case 7: return m_max;
		}
	}

	// 3D hit test
	bool hit(const Ray& ray, float minDistance, float maxDistance) const;
	// 2D hit test
	bool hit(vec2 position) const;

	String toString() const;

protected:
	vec3 m_min;
	vec3 m_max;
};

}
