#pragma once

#include <stdint.h> // uint8_t etc.
#include <mutex>
#include <thread>

#include "nanovg.h"

#include "rae/core/Types.hpp"
#include "rae/core/ISystem.hpp"

#include "rae/visual/Ray.hpp"
#include "rae_ray/HitRecord.hpp"
#include "rae_ray/Hitable.hpp"
#include "rae_ray/HitableList.hpp"
#include "rae_ray/BvhNode.hpp"

#include "rae/image/ImageBuffer.hpp"

namespace rae
{

class CameraSystem;
class Camera;
class Material;

class RayTracer : public ISystem
{
public:
	RayTracer(CameraSystem& cameraSystem);
	~RayTracer();

	String name() override { return "RayTracer"; }

	void showScene(int number);
	void clearScene();

	void createSceneOne(HitableList& world, bool loadBunny = false);
	void createSceneFromBook(HitableList& list);

	bool update(double time, double delta_time) override;
	void destroyEntities(const Array<Id>& entities) override;
	void defragmentTables() override {}
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

	void plusBounces(int delta = 1);
	void minusBounces(int delta = 1);

	void onCameraChanged(const Camera& camera);

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

	CameraSystem& m_cameraSystem;
	HitableList m_world;
	BvhNode m_tree;

	NVGcontext* m_vg = nullptr;
	NVGpaint m_imgPaint;
};

} // end namespace rae
