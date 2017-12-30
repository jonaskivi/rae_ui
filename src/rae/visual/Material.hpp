#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "rae/core/Types.hpp"
#include "rae/visual/Ray.hpp"
#include "rae_ray/HitRecord.hpp"

struct NVGcontext;
struct NVGLUframebuffer;

namespace rae
{

class Material
{
public:
	Material() {}
	Material(vec3 albedo) :
		albedo(albedo),
		m_color(albedo.r, albedo.g, albedo.b, 1.0f)
	{
	}

	Material(Colour albedo) :
		m_color(albedo),
		albedo(albedo.r, albedo.g, albedo.b)
	{
	}

	virtual bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;
	virtual vec3 emitted(const vec3& p) const { return vec3(0.0f, 0.0f, 0.0f); }
	
	bool metal(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;

	//JONDE REALLY NOW: Combine albedo with m_color...
	vec3 albedo;

	void generateFBO(NVGcontext* vg);
	void update(NVGcontext* vg, double time);

	GLuint textureID() const;

	// JONDE UMM, DECIDE THE SPELLING ALREADY:
	void setColor(Colour set);
	const Colour& color() { return m_color; }

	void animate(bool set) { m_animate = set; }

protected:
	NVGLUframebuffer* m_framebufferObject = nullptr;
	int m_width = 512;
	int m_height = 512;

	// JONDE DECIDE SPELLING, REALLY:
	Colour m_color;

	bool m_initialized = false;
	bool m_animate = false;
	//JONDE REMOVE?
	int m_type = 1; // TODO enum. Currently 0 and 1 supported!
};

class Lambertian : public Material
{
public:
	Lambertian(vec3 set_albedo)
		: Material(set_albedo)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const override;
};

class Metal : public Material
{
public:
	Metal(vec3 set_albedo, float set_roughness)
		: Material(set_albedo),
		roughness(set_roughness)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const override;

	float roughness = 0.0f;
};

class Dielectric : public Material
{
public:
	Dielectric(vec3 set_albedo, float set_refractive_index)
		: Material(set_albedo),
		refractive_index(set_refractive_index)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const override;

	float refractive_index = 0.0f;
};

class Light : public Material
{
public:
	Light(vec3 setAlbedo)
		: Material(setAlbedo)
	{
	}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const override { return false; }
	vec3 emitted(const vec3& p) const override { return albedo; }
};

}
