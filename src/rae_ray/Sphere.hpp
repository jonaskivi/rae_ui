#pragma once

#include <glm/glm.hpp>

#include "Hitable.hpp"

namespace rae
{

using vec3 = glm::vec3;

class Ray;
struct HitRecord;
class Material;
class Aabb;

class OldSphere : public Hitable
{
public:
	OldSphere(){}
	OldSphere(vec3 setCenter, float setRadius, Material* setMaterial)
		: center(setCenter),
		radius(setRadius),
		material(setMaterial)
	{}

	~OldSphere();

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const;
	virtual Box getAabb(float t0, float t1) const;

	vec3 center;
	float radius;
	Material* material;
};

}
