#pragma once

#include <stdint.h> // uint8_t etc.
#include <mutex>
#include <thread>
#include <atomic>

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

class Time;
class CameraSystem;
class Camera;
class Material;

class RayTracer : public ISystem
{
public:
	RayTracer(const Time& time, CameraSystem& cameraSystem);
	~RayTracer();

	String name() override { return "RayTracer"; }

	void showScene(int number);
	void clearScene();

	void createSceneOne(HitableList& world, bool loadBunny = false);
	void createSceneFromBook(HitableList& list);

	UpdateStatus update() override;

	void updateRenderThread();

	void renderAllAtOnce();
	void renderSamples();
	void updateImageBuffer();
	void renderNanoVG(NVGcontext* vg,  float x, float y, float w, float h);
	void setNanoVG(NVGcontext* nanoVG);

	void autoFocus();

	vec3 rayTrace(const Ray& ray, int depth = 0);
	vec3 sky(const Ray& ray);

	void requestClear(); // Ask for buffer and rendering state to be cleared on start of next update.
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
	void writeToPng(String filename);

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
	ImageBuffer* m_buffer = nullptr;
	std::mutex m_bufferMutex;
	std::atomic<bool> m_frameReady;

	bool m_requestClear;

	int m_allAtOnceSamplesLimit = 2000;
	int m_samplesLimit = 0;
	int m_bouncesLimit = 50;
	
	int m_currentSample = 0;
	double m_totalRayTracingTime = -1.0;

	// for renderAllAtOnce:
	double m_startTime = -1.0;

	const Time& m_time;
	CameraSystem& m_cameraSystem;
	HitableList m_world;
	BvhNode m_tree;

	NVGcontext* m_nanoVG = nullptr;
	NVGpaint m_imgPaint;

	bool m_renderThreadActive = true;
	std::thread m_renderThread;
};

} // end namespace rae
