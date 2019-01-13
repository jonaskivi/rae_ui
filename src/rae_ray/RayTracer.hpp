#pragma once

#include <stdint.h> // uint8_t etc.
#include <mutex>
#include <thread>
#include <atomic>

#include "nanovg.h"

#include "rae/core/Types.hpp"
#include "rae/core/ISystem.hpp"

#include "rae/scene/SceneSystem.hpp"

#include "rae/visual/Ray.hpp"
#include "rae_ray/HitRecord.hpp"
#include "rae_ray/Hitable.hpp"
#include "rae_ray/HitableList.hpp"
#include "rae_ray/BvhNode.hpp"

#include "rae/image/ImageBuffer.hpp"

namespace rae
{

class Time;
class AssetSystem;
class WindowSystem;
class SceneSystem;
class Camera;
class Material;

using VolumeParent = Id;
using VolumeChildren = Array<Id>;

#define circleArea(r) (PI*r*r)

class VolumeHierarchySystem
{
public:

	bool hasParent(Id id) const;
	Id getParent(Id id) const;
	bool hasChildren(Id id) const;
	void addChild(Id parent, Id child);

	const Table<VolumeParent>& volumeParents() const { return m_parents; }
	const Table<VolumeChildren>& volumeChildren() const { return m_childrens; }

private:
	Table<VolumeParent>		m_parents;
	Table<VolumeChildren>	m_childrens;
};

class RayTracer : public ISystem
{
public:
	RayTracer(
		const Time& time,
		WindowSystem& windowSystem,
		AssetSystem& assetSystem,
		SceneSystem& sceneSystem);
	~RayTracer();

	void showScene(int number);
	void clearScene();

	void updateScene(const Scene& scene);
	void createSceneOne(HitableList& world, bool loadBunny = false);
	void createSceneFromBook(HitableList& list);

	UpdateStatus update() override;
	void updateDebugTexts();

	void updateRenderThread();

	void renderAllAtOnce();
	void renderSamples();
	void updateImageBuffer();
	void renderNanoVG(NVGcontext* vg,  float x, float y, float w, float h);
	void render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) const;

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

	ImageBuffer<float>& imageBuffer() { return *m_buffer; }
	ImageBuffer<uint8_t>& uintBuffer() { return *m_uintBuffer; }
	void writeToPng(String filename);

	void plusBounces(int delta = 1);
	void minusBounces(int delta = 1);

protected:

	bool m_isInfoText = true;
	bool m_isFastMode = false;
	bool m_isVisualizeFocusDistance = true;

	double m_switchTime = 5.0f; // time to switch to big buffer rendering in seconds

	std::mutex				m_bufferMutex;
	std::atomic<bool>		m_frameReady;

	ImageBuffer<float>		m_smallBuffer;
	ImageBuffer<float>		m_bigBuffer;
	ImageBuffer<float>*		m_buffer = nullptr;

	ImageBuffer<uint8_t>	m_smallUintBuffer;
	ImageBuffer<uint8_t>	m_bigUintBuffer;
	ImageBuffer<uint8_t>*	m_uintBuffer = nullptr;

	bool m_requestClear;

	int m_allAtOnceSamplesLimit = 2000;
	int m_samplesLimit = 0;
	int m_bouncesLimit = 50;

	int m_currentSample = 0;
	double m_totalRayTracingTime = -1.0;

	// for renderAllAtOnce:
	double m_startTime = -1.0;

	const Time& m_time;
	WindowSystem&	m_windowSystem;
	AssetSystem&	m_assetSystem;
	SceneSystem&	m_sceneSystem;

	Scene*			m_scene;
	HitableList		m_world;
	BvhNode			m_tree;

	NVGpaint m_imgPaint;

	bool m_renderThreadActive = true;
	std::thread m_renderThread;
};

} // end namespace rae
