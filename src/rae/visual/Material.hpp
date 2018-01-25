#pragma once

#include <glm/glm.hpp>

#include "rae/core/Types.hpp"
#include "rae/visual/Ray.hpp"
#include "rae_ray/HitRecord.hpp"

#include "rae/image/ImageBuffer.hpp"

struct NVGcontext;

namespace rae
{

class Material
{
public:
	Material() {}
	Material(Color3 albedo) :
		m_color(albedo.r, albedo.g, albedo.b, 1.0f)
	{
	}

	Material(Color albedo) :
		m_color(albedo)
	{
	}

	virtual bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;
	virtual vec3 emitted(const vec3& p) const { return vec3(0.0f, 0.0f, 0.0f); }
	
	bool metal(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;

	void generateFBO(NVGcontext* vg);
	void update(NVGcontext* vg, double time);

	GLuint textureId() const;

	void setColor(Color set);
	const Color& color() { return m_color; }
	Color3 color3() { return Color3(m_color); }

	void animate(bool set) { m_animate = set; }

protected:
	FrameBufferImage m_frameBufferImage;

	Color m_color;

	bool m_initialized = false;
	bool m_animate = false;
	// RAE_TODO REMOVE?
	int m_type = 1; // TODO enum. Currently 0 and 1 supported!
};

class Lambertian : public Material
{
public:
	Lambertian(Color3 albedo) :
		Material(albedo)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const override;
};

class Metal : public Material
{
public:
	Metal(Color3 albedo, float roughness) :
		Material(albedo),
		roughness(roughness)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const override;

	float roughness = 0.0f;
};

class Dielectric : public Material
{
public:
	Dielectric(Color3 albedo, float refractiveIndex) :
		Material(albedo),
		refractiveIndex(refractiveIndex)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const override;

	float refractiveIndex = 0.0f;
};

class Light : public Material
{
public:
	Light(Color3 albedo) :
		Material(albedo)
	{
	}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const override { return false; }
	Color3 emitted(const vec3& p) const override { return Color3(m_color); }
};

}
