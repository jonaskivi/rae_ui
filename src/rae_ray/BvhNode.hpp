#pragma once

#include "rae/core/Types.hpp"

#include "Hitable.hpp"
#include "rae/visual/Box.hpp"

namespace rae
{

class Ray;
struct HitRecord;
class Box;

class BvhNode : public Hitable
{
public:
	BvhNode(){}
	BvhNode(Array<Hitable*>& hitables, float time0, float time1);

	void init(Array<Hitable*>& hitables, float time0, float time1);

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const;
	virtual Box getAabb(float t0, float t1) const;

protected:

	Hitable* m_left;
	Hitable* m_right;

	Box m_aabb;
};

}
