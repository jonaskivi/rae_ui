#pragma once

#include <iostream>

#include <stdint.h> // uint8_t etc.
#include <vector>
#include <mutex>
#include <thread>

#include "nanovg.h"

#include <glm/glm.hpp>
using glm::vec3;

#include "Ray.hpp"
#include "HitRecord.hpp"
#include "Hitable.hpp"
#include "HitableList.hpp"
#include "BvhNode.hpp"

namespace Rae
{

class Camera;
class Material;

struct ImageBuffer
{
	ImageBuffer();
	ImageBuffer(int setWidth, int setHeight);

	void init(int setWidth, int setHeight);
	void init();

	void createImage(NVGcontext* vg);
	void update8BitImageBuffer(NVGcontext* vg);
	void clear();

	int channels = 4; // needs to be 4 for rgba with nanovg create image func
	
	int width;
	int height;

	std::vector<vec3> colorData;
	std::vector<uint8_t> data;

	int imageId;
};

class RayTracer
{
public:
	RayTracer(Camera& setCamera);
	~RayTracer();

	void showScene(int number);
	void clearScene();

	void createSceneOne(HitableList& world, Camera& camera);
	void createSceneFromBook(HitableList& list, Camera& camera);

	void update(double time, double deltaTime);
	void renderAllAtOnce(double time);
	void renderSamples(double time, double deltaTime);
	void updateImageBuffer();
	void renderNanoVG(NVGcontext* vg,  float x, float y, float w, float h);
	void setNanovgContext(NVGcontext* setVg);

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

	ImageBuffer& imageBuffer() { return *m_buffer; }

protected:

	bool m_isInfoText = true;
	bool m_isFastMode = false;
	bool m_isVisualizeFocusDistance = true;

	double m_switchTime = 5.0f; // time to switch to big buffer rendering in seconds
	ImageBuffer m_smallBuffer;
	ImageBuffer m_bigBuffer;
	ImageBuffer* m_buffer;

	int m_samplesLimit = 2000;
	int m_bouncesLimit = 50;
	
	int m_currentSample = 0;
	double m_totalRayTracingTime = -1.0;

	// for renderAllAtOnce:
	double m_startTime = -1.0;

	Camera& m_camera;
	HitableList m_world;
	BvhNode m_tree;

	NVGcontext* m_vg = nullptr;
	NVGpaint m_imgPaint;
};

} // end namespace Rae
