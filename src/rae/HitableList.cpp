#include "HitableList.hpp"
#include "rae/visual/Ray.hpp"
#include "HitRecord.hpp"
#include "rae/visual/Box.hpp"

using namespace rae;

bool HitableList::hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const
{
	HitRecord tempRecord;
	bool hitAnything = false;
	float closestSoFar = t_max;
	for (size_t i = 0; i < m_list.size(); ++i)
	{
		if (m_list[i]->hit(ray, t_min, closestSoFar, tempRecord))
		{
			hitAnything = true;
			closestSoFar = tempRecord.t;
			record = tempRecord;
		}
	}
	return hitAnything;
}

Box HitableList::getAabb(float t0, float t1) const
{
	return Box();
}