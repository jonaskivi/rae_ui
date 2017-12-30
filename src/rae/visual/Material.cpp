#include <iostream>
using namespace std;
#include <math.h>
#include <assert.h>

#include "rae/core/Random.hpp"

#include "rae/visual/Material.hpp" // includes glew.h which is needed by nanovg headers.

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

using namespace rae;

// TODO move to RayTracerUtils.hpp
vec3 randomInUnitSphere()
{
	vec3 p;
	do
	{
		p = 2.0f * vec3(getRandom(), getRandom(), getRandom()) - vec3(1,1,1);
	} while (glm::dot(p,p) >= 1.0f);
	return p;
}

bool Material::scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const
{
	return false;
}

bool Lambertian::scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const
{
	vec3 target = record.point + record.normal + randomInUnitSphere();
	scattered = Ray(record.point, target - record.point);
	attenuation = albedo;
	return true;
}

vec3 reflect(const vec3& v, const vec3& normal)
{
	return v - 2.0f * glm::dot(v, normal) * normal;
}

bool Metal::scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const
{
	vec3 reflected = reflect( glm::normalize(r_in.direction()), record.normal );
	scattered = Ray(record.point, reflected + roughness * randomInUnitSphere());
	attenuation = albedo;
	return (glm::dot(scattered.direction(), record.normal) > 0);
}

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted)
{
	vec3 uv = glm::normalize(v);
	float dt = glm::dot(uv, n);
	float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);
	if (discriminant > 0)
	{
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	return false;
}

float schlick(float cosine, float refractive_index)
{
	float r0 = (1.0f - refractive_index) / (1.0f + refractive_index);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
}

bool Dielectric::scatter(const Ray& r_in, const HitRecord& record, vec3& attenuation, Ray& scattered) const
{
	vec3 outward_normal;
	vec3 reflected = reflect(r_in.direction(), record.normal);
	float ni_over_nt;
	attenuation = vec3(1,1,1);
	vec3 refracted;
	float reflect_probability;
	float cosine;
	if (glm::dot(r_in.direction(), record.normal) > 0)
	{
		outward_normal = -record.normal;
		ni_over_nt = refractive_index;
		cosine = refractive_index * glm::dot(r_in.direction(), record.normal) / r_in.direction().length();
	}
	else
	{
		outward_normal = record.normal;
		ni_over_nt = 1.0f / refractive_index;
		cosine = -glm::dot(r_in.direction(), record.normal) / r_in.direction().length();
	}

	if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
	{
		reflect_probability = schlick(cosine, refractive_index);
	}
	else
	{
		reflect_probability = 1.0f;
	}

	if (drand48() < reflect_probability)
	{
		scattered = Ray(record.point, reflected); // REFLECT vs
	}
	else
	{
		scattered = Ray(record.point, refracted); // REFRACT !!
	}
	return true;
}

void Material::generateFBO(NVGcontext* vg)
{
	m_framebufferObject = nvgluCreateFramebuffer(vg, m_width, m_height, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
	if (m_framebufferObject == nullptr)
	{
		cout << "Could not create FBO.\n";
		assert(0);
	}
}

void Material::update(NVGcontext* vg, double time)
{
	if (m_framebufferObject == nullptr)
		return;

	if (m_initialized == true && m_animate == false)
		return;

	float circle_size = float((cos(time) + 1.0) * 128.0);

	nvgluBindFramebuffer(m_framebufferObject);
	glViewport(0, 0, m_width, m_height);

	// Any alpha other than zero will fail for some FBO reason
	glClearColor(m_color.r, m_color.g, m_color.b, 0.0f);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgBeginFrame(vg, m_width, m_height, /*pixelRatio*/1.0f);

		nvgBeginPath(vg);

		if (m_animate)
			nvgCircle(vg, float(m_width) * 0.5f, float(m_height) * 0.5f, circle_size);
		
		if(m_type == 2)
			nvgFillColor(vg, nvgRGBA(220, 45, 0, 200));
		else if(m_type == 1)
			nvgFillColor(vg, nvgRGBA(0, 220, 45, 200));
		else nvgFillColor(vg, nvgRGBA(10, 145, 200, 200)); 

		nvgFill(vg);

		if(m_type == 2)
		{
			nvgFontFace(vg, "sans");

			nvgFontSize(vg, 80.0f);
			nvgTextAlign(vg, NVG_ALIGN_CENTER);
			nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
			nvgText(vg, float(m_width) * 0.5f, (float(m_height) * 0.5f) + 20.0f, "Add Object", nullptr);
		}

	nvgEndFrame(vg);
	nvgluBindFramebuffer(NULL);

	m_initialized = true;
}

GLuint Material::textureID() const
{
	if( m_framebufferObject == nullptr )
		return 0;
	return m_framebufferObject->texture;
}

void Material::setColor(Colour set)
{
	m_color = set;
}
