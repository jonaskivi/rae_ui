#pragma once

#include <glm/glm.hpp>
using glm::vec3;

#include "Hitable.hpp"

namespace Rae
{

class Ray;
class HitRecord;
class Material;

class Sphere : public Hitable
{
public:
	Sphere(){}
	Sphere(vec3 setCenter, float setRadius, Material* setMaterial)
		: center(setCenter),
		radius(setRadius),
		material(setMaterial)
	{}

	~Sphere();

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const;

	vec3 center;
	float radius;
	Material* material;
};

}
