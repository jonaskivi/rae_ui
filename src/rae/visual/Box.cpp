#include "rae/visual/Box.hpp"

#include <algorithm>

#include "rae/core/Utils.hpp"
#include "rae/visual/Ray.hpp"

using namespace rae;

void Box::init(const Box& left, const Box& right)
{
	clear();
	grow(left);
	grow(right);
}

void Box::grow(const Box& set)
{
	grow(set.min());
	grow(set.max());
}

void Box::grow(vec3 set)
{
	m_min.x = std::min(m_min.x, set.x);
	m_min.y = std::min(m_min.y, set.y);
	m_min.z = std::min(m_min.z, set.z);

	m_max.x = std::max(m_max.x, set.x);
	m_max.y = std::max(m_max.y, set.y);
	m_max.z = std::max(m_max.z, set.z);
}

void Box::transform(const Transform& tr)
{
	if (!valid())
		return;

	m_min *= tr.scale;
	m_max *= tr.scale;

	// Rotate the corners of this AABB and then grow another AABB
	// with those points.
	auto growRotatedBox = [this](const qua& rotation) -> Box
	{
		Box result;
		for (int i = 0; i < 8; ++i)
		{
			result.grow(rotation * corner(i));
		}
		return result;
	};

	*this = growRotatedBox(tr.rotation);
	translate(tr.position);
}

void Box::translate(const vec3& offset)
{
	m_min += offset;
	m_max += offset;
}

void Box::translatePivot(const Pivot& pivot)
{
	if (!valid())
		return;

	auto halfDimensions = dimensions() * 0.5f;

	m_min -= (pivot * halfDimensions);
	m_max -= (pivot * halfDimensions);
}

bool Box::hit(const Ray& ray, float minDistance, float maxDistance) const
{
	for (int a = 0; a < 3; ++a)
	{
		float invD = 1.0f / ray.direction()[a];
		float t0 = (min()[a] - ray.origin()[a]) * invD;
		float t1 = (max()[a] - ray.origin()[a]) * invD;
		if (invD < 0.0f)
			std::swap(t0, t1);
		minDistance = t0 > minDistance ? t0 : minDistance;
		maxDistance = t1 < maxDistance ? t1 : maxDistance;
		if (maxDistance <= minDistance)
			return false;
	}
	return true;
}

bool Box::hit(vec2 position) const
{
	if (position.x <= m_max.x &&
		position.x >= m_min.x &&
		position.y <= m_max.y &&
		position.y >= m_min.y)
		return true;
	return false;
}

String Box::toString() const
{
	return "min: " + Utils::toString(m_min)
		+ " max: " + Utils::toString(m_max)
		+ "\nwidth: " + Utils::toString(width())
		+ " height: " + Utils::toString(height());
}
