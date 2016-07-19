#pragma once

#include <stdint.h> // uint8_t etc.
#include <vector>

#include "nanovg.h"

#include <glm/glm.hpp>
using glm::vec3;

#include "Ray.hpp"
#include "HitRecord.hpp"

namespace Rae
{

class Camera;
class Material;

class Hitable
{
public:
	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const = 0;
};

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

// HitableList.hpp
class HitableList : public Hitable
{
public:
	HitableList(){}
	HitableList(int size)
	{
		list.reserve(size);
	}
	~HitableList()
	{
		clear();
	}

	void clear()
	{
		for(size_t i = 0; i < list.size(); ++i)
		{
			delete list[i];
		}
		list.clear();
	}

	virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& record) const;

	void add(Hitable* hitable)
	{
		list.push_back(hitable);
	}

	std::vector<Hitable*> list;
};

struct ImageBuffer
{
	ImageBuffer(int setWidth, int setHeight);

	void createImage(NVGcontext* vg);
	void update8BitImageBuffer(NVGcontext* vg);
	void clear();

	int channels = 4; // needs to be 4 for rgba with nanovg create image func
	
	int width;
	int height;

	std::vector<vec3> color_data;
	std::vector<uint8_t> data;

	int imageId;
};

class RayTracer
{
public:
	RayTracer(Camera& setCamera);

	void showScene(int number);
	void clearScene();

	void createSceneOne(HitableList& world, Camera& camera);
	void createSceneFromBook(HitableList& list, Camera& camera);

	void update(double time, double delta_time);
	void renderAllAtOnce(double time);
	void renderSamples(double time, double delta_time);
	void updateImageBuffer();
	void renderNanoVG(NVGcontext* vg,  float x, float y, float w, float h);
	void setNanovgContext(NVGcontext* set_vg);

	void autoFocus();

	vec3 rayTrace(const Ray& ray, Hitable& world, int depth);
	vec3 sky(const Ray& ray);

	void clear();
	void toggleBufferQuality();
	bool isFastMode() { return m_isFastMode; }
	void toggleFastMode() { m_isFastMode = !m_isFastMode; }
	float rayMaxLength();

	HitRecord debugHitRecord;

	void toggleInfoText() { m_isInfoText = !m_isInfoText; }
	bool isInfoText() { return m_isInfoText; }

	void toggleVisualizeFocusDistance() { m_isVisualizeFocusDistance = !m_isVisualizeFocusDistance; }

protected:

	bool m_isInfoText = true;
	bool m_isFastMode = false;
	bool m_isVisualizeFocusDistance = true;

	double switchTime = 5.0f; // time to switch to big buffer rendering in seconds
	ImageBuffer smallBuffer = ImageBuffer(300, 150);
	//ImageBuffer bigBuffer = ImageBuffer(1000, 500);
	ImageBuffer bigBuffer = ImageBuffer(1920, 1080);
	ImageBuffer* buffer;

	int samplesLimit = 2000;
	
	int currentSample = 0;
	double totalRayTracingTime = -1.0;

	// for renderAllAtOnce:
	double startTime = -1.0;

	Camera& m_camera;
	HitableList m_world;

	NVGcontext* vg = nullptr;
	NVGpaint imgPaint;
};

} // end namespace Rae
