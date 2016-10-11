#pragma once

namespace Rae
{

class Ray;
class HitRecord;
class Aabb;

class Hitable
{
public:

	Hitable(){}
	virtual ~Hitable(){}

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const = 0;
	virtual Aabb getAabb(float t0, float t1) const = 0;
};

}
