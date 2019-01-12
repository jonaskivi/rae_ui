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

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const override;
	virtual Box getAabb(float t0, float t1) const override;

	virtual void iterate(std::function<void(const Box&)> process) const override;

protected:

	Hitable* m_left = nullptr;
	Hitable* m_right = nullptr;

	Box m_aabb;
};

}
