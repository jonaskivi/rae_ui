#ifndef RAE_MATERIAL_HPP
#define RAE_MATERIAL_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Ray.hpp"
#include "HitRecord.hpp"

struct NVGcontext;
struct NVGLUframebuffer;

namespace Rae
{

class Material
{
public:
	int id() { return m_id; }
protected:
	void id(int set) { m_id = set; }
	int m_id;

public:
	Material(){}
	Material(vec3 set_albedo)
	: albedo(set_albedo)
	{
	}

	virtual bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;
	
	bool metal(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;

	vec3 albedo;

	// ----------------------- Legacy:

	Material(int set_id, int set_type, const glm::vec4& set_color); // That type thing is really strange...

	void generateFBO(NVGcontext* vg);
	void update(NVGcontext* vg, double time);

	GLuint textureID();

	void setColor(glm::vec4 set);
	const glm::vec4& color() { return m_color; }

	void animate(bool set) { m_animate = set; }

protected:
	NVGLUframebuffer* m_framebufferObject;
	int m_width;
	int m_height;

	glm::vec4 m_color;

	bool m_initialized = false;
	bool m_animate = false;
	int m_type; // TODO enum. Currently 0 and 1 supported!
};

class Lambertian : public Material
{
public:
	Lambertian(vec3 set_albedo)
		: Material(set_albedo)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;
};

class Metal : public Material
{
public:
	Metal(vec3 set_albedo, float set_roughness)
		: Material(set_albedo),
		roughness(set_roughness)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;

	float roughness = 0.0f;
};

class Dielectric : public Material
{
public:
	Dielectric(vec3 set_albedo, float set_refractive_index)
		: Material(set_albedo),
		refractive_index(set_refractive_index)
	{}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;

	float refractive_index = 0.0f;
};

}

#endif

