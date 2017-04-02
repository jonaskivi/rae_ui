#pragma once

namespace rae
{

class Material;

// Hitable.hpp
struct HitRecord
{
	float t;
	vec3 point;
	vec3 normal;
	Material* material = nullptr;
};

}
