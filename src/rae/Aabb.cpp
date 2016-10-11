#include "Aabb.hpp"
#include "Ray.hpp"

using namespace Rae;

void Aabb::grow(vec3 set)
{
	if(m_min.x > set.x)
		m_min.x = set.x;
	if(m_min.y > set.y)
		m_min.y = set.y;
	if(m_min.z > set.z)
		m_min.z = set.z;

	if(m_max.x < set.x)
		m_max.x = set.x;
	if(m_max.y < set.y)
		m_max.y = set.y;
	if(m_max.z < set.z)
		m_max.z = set.z;
}

bool Aabb::hit(const Ray& ray, float minDistance, float maxDistance) const
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