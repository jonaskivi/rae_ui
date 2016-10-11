#pragma once

namespace Rae
{

class Ray;
class HitRecord;

class Hitable
{
public:
	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const = 0;
};

}
