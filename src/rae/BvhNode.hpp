#pragma once

#include <vector>

#include <glm/glm.hpp>
using glm::vec3;

#include "Hitable.hpp"
#include "Aabb.hpp"

namespace Rae
{

class Ray;
class HitRecord;
class Aabb;

class BvhNode : public Hitable
{
public:
	BvhNode(){}
	BvhNode(std::vector<Hitable*>& hitables, float time0, float time1);

	void init(std::vector<Hitable*>& hitables, float time0, float time1);

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const;
	virtual Aabb getAabb(float t0, float t1) const;

protected:

	Hitable* m_left;
	Hitable* m_right;

	Aabb m_aabb;
};

}
