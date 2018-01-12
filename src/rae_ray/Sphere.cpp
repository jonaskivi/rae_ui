#include "Sphere.hpp"
#include "rae/visual/Ray.hpp"
#include "HitRecord.hpp"
#include "rae/visual/Box.hpp"
#include "rae/visual/Material.hpp"

using namespace rae;

Sphere::~Sphere()
{
	delete material; // Hmm. Currently no shared materials... TODO memory management.
}

bool Sphere::hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const
{
	vec3 oc = ray.origin() - center;
	float a = glm::dot(ray.direction(), ray.direction());
	float b = glm::dot(oc, ray.direction());
	float c = glm::dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0)
	{
		float temp = (-b - sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min)
		{
			record.t = temp;
			record.point = ray.point_at_parameter(record.t);
			record.normal = (record.point - center) / radius;
			record.material = material;
			return true;
		}
	}
	return false;
}

Box Sphere::getAabb(float t0, float t1) const
{
	vec3 cornerVec = vec3(radius, radius, radius);
	return Box(center - cornerVec, center + cornerVec);
}
