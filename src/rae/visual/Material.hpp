#pragma once

#include <glm/glm.hpp>

#include "rae/core/Types.hpp"
#include "rae/visual/Ray.hpp"
#include "rae_ray/HitRecord.hpp"

#include "rae/image/ImageBuffer.hpp"

struct NVGcontext;

namespace rae
{

enum class MaterialType
{
	Lambertian,
	Metal,
	Dielectric,
	Light
};

class Material
{
public:
	Material() {}
	Material(
		const String& name,
		const Color3& albedo,
		MaterialType materialType = MaterialType::Lambertian,
		float roughness = 0.0f,
		float refractiveIndex = 0.0f) :
			m_name(name),
			m_color(albedo.r, albedo.g, albedo.b, 1.0f),
			m_materialType(materialType),
			m_roughness(roughness),
			m_refractiveIndex(refractiveIndex)
	{
	}

	Material(
		const String& name,
		const Color& albedo,
		MaterialType materialType = MaterialType::Lambertian,
		float roughness = 0.0f,
		float refractiveIndex = 0.0f) :
			m_name(name),
			m_color(albedo),
			m_materialType(materialType),
			m_roughness(roughness),
			m_refractiveIndex(refractiveIndex)
	{
	}

	bool scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const;
	vec3 emitted(const vec3& p) const;

	void generateFBO(NVGcontext* vg);
	void update(NVGcontext* vg, double time);

	GLuint textureId() const;

	void setColor(Color set);
	const Color& color() const { return m_color; }
	Color3 color3() const { return Color3(m_color); }

	void animate(bool set) { m_animate = set; }

	const String& name() const { return m_name; }
	MaterialType materialType() const { return m_materialType; }

protected:
	FrameBufferImage m_frameBufferImage;

	String m_name = "Material";
	Color m_color;
	MaterialType m_materialType = MaterialType::Lambertian;

	float m_roughness = 0.0f; // for Metal
	float m_refractiveIndex = 0.0f; // for Dielectric

	bool m_initialized = false;
	bool m_animate = false;
	// RAE_TODO REMOVE?
	int m_type = 1; // TODO enum. Currently 0 and 1 supported!
};

}
