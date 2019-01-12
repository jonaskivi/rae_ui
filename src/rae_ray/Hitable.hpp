#pragma once

#include "rae/visual/Box.hpp"

namespace rae
{

class Ray;
struct HitRecord;
class Box;

class Hitable
{
public:

	Hitable(){}
	virtual ~Hitable(){}

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const = 0;
	virtual Box getAabb(float t0, float t1) const = 0;

	virtual void iterate(std::function<void(const Box&)> process) const
	{
		process(getAabb(0,0));
	}
};

}
