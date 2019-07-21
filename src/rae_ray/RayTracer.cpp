#include "RayTracer.hpp"

#include <thread>

#include "rae/core/Utils.hpp"
#include "rae/core/Random.hpp"
#include "rae/core/Time.hpp"

#include "rae/ui/DebugSystem.hpp"
#include "rae/ui/WindowSystem.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/scene/SceneSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/visual/Material.hpp"
#include "rae_ray/Sphere.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/image/ImageBuffer.hpp"

using namespace rae;

bool VolumeHierarchySystem::hasParent(Id id) const
{
	if (!m_parents.check(id))
		return false;
	return (m_parents.getF(id) != InvalidId);
}

Id VolumeHierarchySystem::getParent(Id id) const
{
	return m_parents.get(id);
}

bool VolumeHierarchySystem::hasChildren(Id id) const
{
	if (!m_childrens.check(id))
		return false;
	return (m_childrens.get(id).size() > 0);
}

void VolumeHierarchySystem::addChild(Id parent, Id child)
{
	if (!hasChildren(parent))
	{
		m_childrens.assign(parent, { child });
	}
	else
	{
		auto& childrenArray = m_childrens.modifyF(parent);
		// Check for duplicates
		for (auto&& id : childrenArray)
		{
			if (id == child)
			{
				LOG_F(ERROR, "Same child added multiple times to a parent.");
				assert(0);
				return;
			}
		}
		childrenArray.emplace_back(child);
	}

	if (!hasParent(child))
		m_parents.assign(child, Parent(parent));

	//m_childrenChanged.assign(parent, Changed());
	//m_parentChanged.assign(child, Changed());
}

RayTracer::RayTracer(
	const Time& time,
	WindowSystem& windowSystem,
	AssetSystem& assetSystem,
	SceneSystem& sceneSystem) :
		ISystem("RayTracer"),
		m_world(4),
		m_time(time),
		m_windowSystem(windowSystem),
		m_assetSystem(assetSystem),
		m_sceneSystem(sceneSystem),
		m_renderThread(&RayTracer::updateRenderThread, this)
{
	setIsEnabled(false);

	m_smallBuffer.init(300, 150);
	m_bigBuffer.init(1920, 1080);
	m_buffer = &m_smallBuffer;

	m_smallUintBuffer.init(300, 150);
	m_bigUintBuffer.init(1920, 1080);
	m_uintBuffer = &m_smallUintBuffer;

	m_smallUintBuffer.createImage(m_windowSystem.mainWindow().nanoVG());
	m_bigUintBuffer.createImage(m_windowSystem.mainWindow().nanoVG());

	//createSceneOne(m_world);
	//createSceneFromBook(m_world);
}

RayTracer::~RayTracer()
{
	m_renderThreadActive = false;
	m_renderThread.join();
}

void RayTracer::updateScene(const Scene& scene)
{
	/* RAE_TODO possibly remove this, because we don't want BVH to be done like this:
	const Camera& camera = scene.cameraSystem().currentCamera();
	const auto& transformSystem = scene.transformSystem();
	const auto& selectionSystem = scene.selectionSystem();
	const auto& assetLinkSystem = scene.assetLinkSystem();

	query<MeshLink>(assetLinkSystem.meshLinks(), [&](Id id, const MeshLink& meshLink)
	{
		Material* material = nullptr;
		const Mesh& mesh = m_assetSystem.getMesh(assetLinkSystem.meshLinks().get(id));

		if (m_assetSystem.isMaterial(id))
			material = &m_assetSystem.getMaterial(id);
		else if (assetLinkSystem.materialLinks().check(id))
			material = &m_assetSystem.getMaterial(assetLinkSystem.materialLinks().get(id));

		if (transformSystem.hasTransform(id) &&
			material)
		{
			const Transform& transform = transformSystem.getTransform(id);

			//#ifdef RAE_DEBUG
				LOG_F(INFO, "Adding Mesh to raytracer. id: %i", id);
				//LOG_F(INFO, "MeshLink is: %i", assetLinkSystem.m_meshLinks.get(id));
			//#endif

			m_world.add(
				new OldSphere(transform.position, 0.5f,
				new Lambertian(material->color3()))
				);
		}
	});

	m_tree.init(m_world.list(), 0, 0);
	*/
}

void RayTracer::createSceneOne(HitableList& world, bool loadBunny)
{
	/*
	Camera& camera = m_sceneSystem.activescene().cameraSystem().currentCamera();
	camera.setFieldOfViewDeg(44.6f);
	camera.setPosition(vec3(-0.16f, 2.9664f, 14.8691f));
	camera.setYaw(Math::toRadians(178.560333f));
	camera.setPitch(Math::toRadians(-10.8084f));
	camera.setAperture(0.07f);
	camera.setFocusDistance(14.763986f);
	 */

	// A big light
	world.add(
		new OldSphere(vec3(0.0f, 6.0f, -1.0f), 2.0f,
		new Light(vec3(4.0f, 4.0f, 4.0f)))
		);

	// A small light
	world.add(
		new OldSphere(vec3(3.85, 2.3, -0.15f), 0.2f,
		new Light(vec3(16.0f, 16.0f, 16.0f)))
		);

	// A ball
	world.add(
		new OldSphere(vec3(0, 0.3, -2), 0.5f,
		new Lambertian(vec3(0.8f, 0.3f, 0.3f)))
		);
	// The planet
	world.add(
		new OldSphere(vec3(0, -100.5f, -1), 100.0f,
		new Lambertian(vec3(0.0f, 0.7f, 0.8f)))
		);

	// Metal balls
	world.add(
		new OldSphere(vec3(1, 0, 0), 0.5f,
		new Metal(vec3(0.8f, 0.6f, 0.2f), /*roughness*/0.0f))
		);
	world.add(
		new OldSphere(vec3(-1.5f, 0.65f, 0.5), 0.4f,
		new Metal(vec3(0.8f, 0.4f, 0.8f), /*roughness*/0.3f))
		);
	// Dielectric, glass ball
	world.add(
		new OldSphere(vec3(-1, 0, 1), 0.5f,
		new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f))
		);
	world.add(
		new OldSphere(vec3(-3.15f, 0.1f, -5), 0.6f,
		new Lambertian(vec3(0.05f, 0.2f, 0.8f)))
		);

	///////////////////

	auto bunny = new Mesh();
	if (loadBunny)
		bunny->loadModel("./data/models/bunny.obj");
	else bunny->generateCube();

	world.add(bunny);

	m_tree.init(world.list(), 0, 0);
}

void RayTracer::createSceneFromBook(HitableList& list)
{
	/*
	Camera& camera = m_sceneSystem.activeScene().cameraSystem().currentCamera();

	camera.setPosition(vec3(16.857f, 2.0f, 6.474f));
	camera.setYaw(Math::toRadians(247.8f));
	camera.setPitch(Math::toRadians(-4.762f));
	camera.setAperture(0.1f);
	camera.setFocusDistance(17.29f);
	*/

	list.add( new OldSphere(vec3(0,-1000,0), 1000, new Lambertian(vec3(0.5, 0.5, 0.5))) );

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			float choose_mat = getRandom();
			vec3 center(a + 0.9f * getRandom(), 0.2f, b + 0.9f * getRandom());
			if ((center-vec3(4,0.2,0)).length() > 0.9f)
			{
				if (choose_mat < 0.8f)
				{
					// diffuse
					list.add( new OldSphere(center, 0.2f, new Lambertian(vec3( getRandom()*getRandom(), getRandom()*getRandom(), getRandom()*getRandom()))));
				}
				else if (choose_mat < 0.95f)
				{
					// metal
					list.add( new OldSphere(center, 0.2f,
							new Metal(vec3(0.5f*(1.0f + getRandom()), 0.5f*(1.0f + getRandom()), 0.5f*(1.0f + getRandom())), /*roughness*/ 0.5f*getRandom())));
				}
				else
				{
					// glass
					list.add( new OldSphere(center, 0.2f, new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f)) );
				}
			}
		}
	}

	list.add( new OldSphere(vec3(0, 1, 0), 1.0, new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f)) );
	list.add( new OldSphere(vec3(-4, 1, 0), 1.0, new Lambertian(vec3(0.0, 0.2, 0.9))) );
	list.add( new OldSphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0)) );

	m_tree.init(list.list(), 0, 0);
}

void RayTracer::showScene(int number)
{
	if (number == 1)
	{
		clearScene();
		createSceneOne(m_world, false);
	}

	if (number == 2)
	{
		clearScene();
		createSceneOne(m_world, true);
	}

	if (number == 3)
	{
		clearScene();
		createSceneFromBook(m_world);
	}
}

void RayTracer::clearScene()
{
	m_world.clear();
	m_sceneSystem.activeScene().cameraSystem().setNeedsUpdate();
	clear();
}

void RayTracer::requestClear()
{
	m_requestClear = true;
}

void RayTracer::clear()
{
	std::lock_guard<std::mutex> lock(m_bufferMutex);
	m_frameReady = false;
	m_buffer->clear();
	m_currentSample = 0;
	m_totalRayTracingTime = -1.0;
	m_startTime = -1.0f;
}

std::string toString(const HitRecord& record)
{
	return "t: " + std::to_string(record.t) + ", "
		+ "point: (" + std::to_string(record.point.x) + ", "
		+ std::to_string(record.point.y) + ", "
		+ std::to_string(record.point.z) + ")";
}

void RayTracer::autoFocus()
{
	auto& scene = m_sceneSystem.activeScene();
	auto& transformSystem = scene.transformSystem();
	auto& assetLinkSystem = scene.assetLinkSystem();
	Camera& camera = scene.cameraSystem().modifyCurrentCamera();

	if (scene.selectionSystem().isSelection())
	{
		auto selectedIds = scene.selectionSystem().selectedIds();
		const Transform& transform = transformSystem.getWorldTransform(selectedIds.front());

		// Animating the focus is kind of silly for a raytracer.
		// Might be more interesting when it is actually realtime and uses GPU
		camera.animateFocusPosition(transform.position, camera.focusSpeed());
	}
	else
	{
		// Get a ray to middle of the screen and focus there
		Ray ray = camera.getExactRay(0.5f, 0.5f);
		HitRecord finalRecord;

		auto sphereHitFunc = [](
			const vec3& position,
			float radius,
			Material* material,
			const Ray& ray,
			float t_min,
			float t_max,
			HitRecord& record) -> bool
		{
			vec3 oc = ray.origin() - position;
			float a = glm::dot(ray.direction(), ray.direction());
			float b = glm::dot(oc, ray.direction());
			float c = glm::dot(oc, oc) - radius * radius;
			float discriminant = b * b - a * c;
			if (discriminant > 0)
			{
				float temp = (-b - sqrt(discriminant)) / a;
				if (temp < t_max && temp > t_min)
				{
					record.t = temp;
					record.point = ray.pointAtParameter(record.t);
					record.normal = (record.point - position) / radius;
					record.material = material;
					return true;
				}
			}
			return false;
		};

		bool hit = false;

		float closestSoFar = rayMaxLength();

		query<Box>(transformSystem.boxes(), [&](Id id, const Box& box)
		{
			HitRecord record;

			const Transform& transform = transformSystem.getWorldTransform(id);

			Material* defaultMaterial = nullptr;

			// This is pretty random. So we check if it's a Sphere or a Mesh... and do hit testing on those... yeah. Need to refactor.
			if ((transformSystem.hasSphere(id)
				&& sphereHitFunc(transform.position, box.radius() * transform.scale.x, defaultMaterial, ray, 0.001f, closestSoFar, record))
				||
				(!transformSystem.hasSphere(id) &&
				assetLinkSystem.hasMeshLink(id) &&
				m_assetSystem.getMesh(assetLinkSystem.getMeshLink(id)).hit(transform.position, ray, 0.001f, closestSoFar, record)))
			{
				closestSoFar = record.t;
				finalRecord = record;

				hit = true;
			}


			//mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
			//mat4 rotationMatrix = glm::toMat4(transform.rotation);
			//mat4 scaleMatrix = glm::scale(mat4(1.0f), transform.scale);
			//mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

			// The model-view-projection matrix
			//glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;
		});

		if (hit)
		{
			debugHitRecord = finalRecord;
			camera.animateFocusPosition(finalRecord.point, camera.focusSpeed());
		}
	}
}

vec3 RayTracer::rayTrace(const Ray& ray, int depth)
{
	/* RAE_TODO possibly remove:
	if (m_tree.hit(ray, 0.001f, rayMaxLength(), record))
	{
		// Visualize focus distance with a line
		if (m_isVisualizeFocusDistance)
		{
			float hitDistance = glm::length(record.point - camera.position());
			if (Utils::isEqual(camera.focusDistance(), hitDistance, 0.01f) == true)
			{
				return vec3(0,1,1); // cyan line
			}
		}

		// Normal raytracing
		if (isFastMode() == false)
		{
			Ray scattered;
			vec3 attenuation;
			vec3 emitted = record.material->emitted(record.point);

			if (depth < m_bouncesLimit && record.material->scatter(ray, record, attenuation, scattered))
			{
				return emitted + attenuation * rayTrace(scattered, depth + 1);
			}
			else
			{
				return emitted;
			}
		}
		else // FastMode returns just the material color
		{
			return record.material->color3();
		}
	}
	*/

	//scene.transformSystem().processHierarchy();

	const auto& scene = m_sceneSystem.activeScene();

	const Camera& camera = scene.cameraSystem().currentCamera();
	HitRecord finalRecord;

	const auto& transformSystem = scene.transformSystem();
	const auto& assetLinkSystem = scene.assetLinkSystem();

	Lambertian lambertianMaterial1(vec3(0.0f, 0.3f, 1.0f));
	Lambertian lambertianMaterial2(vec3(1.0f, 1.0f, 1.0f));
	Lambertian lambertianMaterial3(vec3(0.9f, 0.4f, 0.2f));
	Lambertian lambertianMaterial4(vec3(0.8f, 0.9f, 0.1f));
	Dielectric glassMaterial(vec3(0.8f, 0.5f, 0.3f), 1.5f);

	Array<Material*> tempMaterials =
	{
		&lambertianMaterial1,
		&lambertianMaterial2,
		&lambertianMaterial3,
		&lambertianMaterial4,
		&glassMaterial
	};

	auto sphereHitFunc = [](
		const vec3& position,
		float radius,
		Material* material,
		const Ray& ray,
		float t_min,
		float t_max,
		HitRecord& record) -> bool
	{
		vec3 oc = ray.origin() - position;
		float a = glm::dot(ray.direction(), ray.direction());
		float b = glm::dot(oc, ray.direction());
		float c = glm::dot(oc, oc) - radius * radius;
		float discriminant = b * b - a * c;
		if (discriminant > 0)
		{
			float temp = (-b - sqrt(discriminant)) / a;
			if (temp < t_max && temp > t_min)
			{
				record.t = temp;
				record.point = ray.pointAtParameter(record.t);
				record.normal = (record.point - position) / radius;
				record.material = material;
				return true;
			}
		}
		return false;
	};

	vec3 resultColor = vec3(0,0,0);
	bool hit = false;
	bool needsScatter = false;
	bool hitLine = false;
	Ray scattered;

	float closestSoFar = rayMaxLength();

	query<Box>(transformSystem.boxes(), [&](Id id, const Box& box)
	{
		HitRecord record;

		const Transform& transform = transformSystem.getWorldTransform(id);

		//LOG_F(INFO, "raytrace box: %i", (int)id);

		Material* defaultMaterial = tempMaterials[int(id) % int(tempMaterials.size())];

		// This is pretty random. So we check if it's a Sphere or a Mesh... and do hit testing on those... yeah. Need to refactor.
		if ((transformSystem.hasSphere(id)
			&& sphereHitFunc(transform.position, box.radius() * transform.scale.x, defaultMaterial, ray, 0.001f, closestSoFar, record))
			||
			(!transformSystem.hasSphere(id) &&
			assetLinkSystem.hasMeshLink(id) &&
			m_assetSystem.getMesh(assetLinkSystem.getMeshLink(id)).hit(transform.position, ray, 0.001f, closestSoFar, record)))
		{
			bool hitLineLocal = false;

			closestSoFar = record.t;
			finalRecord = record;

			hit = true;

			//LOG_F(INFO, "raytrace box: %i HIT", (int)id);

			// Visualize focus distance with a line
			if (m_isVisualizeFocusDistance)
			{
				float hitDistance = glm::length(record.point - camera.position());
				if (Utils::isEqual(camera.focusDistance(), hitDistance, 0.01f) == true)
				{
					hitLine = true;
					hitLineLocal = true;
					resultColor = vec3(0,1,1); // cyan line
				}
			}

			if (!hitLineLocal)
			{
				// Normal raytracing
				if (isFastMode() == false)
				{
					vec3 attenuation;
					vec3 emitted = record.material->emitted(record.point);

					hitLine = false;

					if (depth < m_bouncesLimit && record.material->scatter(ray, record, attenuation, scattered))
					{
						needsScatter = true;
						resultColor = emitted + attenuation;
					}
					else
					{
						resultColor = emitted;
					}
				}
				else // FastMode returns just the material color
				{
					resultColor = record.material->color3();
				}
			}
		}


		//mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
		//mat4 rotationMatrix = glm::toMat4(transform.rotation);
		//mat4 scaleMatrix = glm::scale(mat4(1.0f), transform.scale);
		//mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

		// The model-view-projection matrix
		//glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;
	});

	if (hit)
	{
		if (needsScatter && !hitLine)
		{
			// RAE_TODO get rid of this recursion:
			resultColor *= rayTrace(scattered, depth + 1);
		}

		return resultColor;
	}

	return sky(ray);
}

vec3 RayTracer::sky(const Ray& ray)
{
	vec3 unitDirection = glm::normalize(ray.direction());
	float t = 0.5f * (unitDirection.y + 1.0f);
	return (1.0f - t) * vec3(0.3f, 0.4f, 1.0f) + t * vec3(0.7f, 0.8f, 1.0f);
	//return (1.0f - t) * vec3(0.0f, 0.0f, 0.0f) + t * vec3(0.05f, 0.05f, 0.05f);
}

//#define RENDER_ALL_AT_ONCE

UpdateStatus RayTracer::update()
{
	if (m_sceneSystem.activeScene().transformSystem().hasAnyTransformChanged() ||
		m_sceneSystem.activeScene().cameraSystem().hasCameraUpdated() ||
		m_sceneSystem.activeScene().selectionSystem().hasSelectionChanged())
	{
		autoFocus();
		requestClear();
	}

	m_tree.iterate([](const Box& box)
	{
		g_debugSystem->drawLineBox(box, Colors::blue);
	});

	if (!m_isEnabled)
		return UpdateStatus::Disabled;

	/*
	Old time based switch buffers system:

	if (switchTime > 0.0f)
	{
		if (m_buffer != &m_bigBuffer
			&&
			(total_ray_tracing_time >= switchTime
			||
			current_sample >= samples_limit))
		{
			m_buffer = &m_bigBuffer;
			m_buffer->clear();
			current_sample = 0;
		}
	}
	*/

	if (m_startTime == -1.0f)
		m_startTime = m_time.time();

	if (m_totalRayTracingTime == -1.0f)
		m_totalRayTracingTime = m_time.time();

	#ifdef RENDER_ALL_AT_ONCE
		renderAllAtOnce();
		if (m_currentSample <= m_allAtOnceSamplesLimit) // do once more than render
		{
			updateImageBuffer();
		}
	#else

		if (m_frameReady == true)
		{
			updateImageBuffer();
			m_frameReady = false;
		}
		else if (m_currentSample == 0)
		{
			// A silly hack to copy the buffer always when sample count is 0. This keeps the update fluid
			// when moving the camera.
			updateImageBuffer();
		}
	#endif

	m_totalRayTracingTime = m_time.time() - m_startTime;

	return UpdateStatus::Changed;
}

void RayTracer::updateRenderThread()
{
	while (m_renderThreadActive)
	{
		/*
		// Crashes for some reason, but ideally we'd want this to be in this thread too.
		if (m_buffer && m_requestToggleBuffer)
		{
			toggleBufferQuality();
			m_requestToggleBuffer = false;
		}
		*/

		if (m_buffer && m_requestClear)
		{
			clear();
			m_requestClear = false;
		}

		if (m_buffer && m_frameReady == false)
		{
			std::lock_guard<std::mutex> lock(m_bufferMutex);
			renderSamples();
		}
	}
}

void RayTracer::updateDebugTexts()
{
	const Camera& camera = m_sceneSystem.activeScene().cameraSystem().currentCamera();

	g_debugSystem->showDebugText("Samples: " + std::to_string(m_currentSample));

	if (m_samplesLimit > 0)
	{
		g_debugSystem->showDebugText("/" + std::to_string(m_samplesLimit));
	}

	g_debugSystem->showDebugText("Time: " + std::to_string(m_totalRayTracingTime) + " s");

	g_debugSystem->showDebugText("Position: "
		+ std::to_string(camera.position().x) + ", "
		+ std::to_string(camera.position().y) + ", "
		+ std::to_string(camera.position().z));

	g_debugSystem->showDebugText("Yaw: "
		+ std::to_string(Math::toDegrees(camera.yaw())) + "°"
		+ " Pitch: "
		+ std::to_string(Math::toDegrees(camera.pitch())) + "°");

	g_debugSystem->showDebugText("Field of View: " + std::to_string(Math::toDegrees(camera.fieldOfView())) + "°");
	g_debugSystem->showDebugText("Focus distance: " + std::to_string(camera.focusDistance()));
	g_debugSystem->showDebugText(camera.isContinuousAutoFocus() ? "Autofocus ON" : "Autofocus OFF");
	g_debugSystem->showDebugText("Aperture: " + std::to_string(camera.aperture()));
	g_debugSystem->showDebugText("Bounces: " + std::to_string(m_bouncesLimit));

	g_debugSystem->showDebugText("Debug hit pos: "
		+ std::to_string(debugHitRecord.point.x) + ", "
		+ std::to_string(debugHitRecord.point.y) + ", "
		+ std::to_string(debugHitRecord.point.z));

	vec3 focusPos = camera.getFocusPosition();
	g_debugSystem->showDebugText("Debug focus pos: "
		+ std::to_string(focusPos.x) + ", "
		+ std::to_string(focusPos.y) + ", "
		+ std::to_string(focusPos.z));
}

void RayTracer::requestToggleBufferQuality()
{
	m_requestToggleBuffer = true;
}

void RayTracer::toggleBufferQuality()
{
	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);
		if (m_buffer == &m_smallBuffer)
		{
			m_buffer = &m_bigBuffer;
		}
		else
		{
			m_buffer = &m_smallBuffer;
		}

		if (m_uintBuffer == &m_smallUintBuffer)
		{
			m_uintBuffer = &m_bigUintBuffer;
		}
		else
		{
			m_uintBuffer = &m_smallUintBuffer;
		}

		requestClear();
	}
}

float RayTracer::rayMaxLength()
{
	if (isFastMode() == false)
		return FLT_MAX;
	else return 5.0f;
}

void RayTracer::plusBounces(int delta)
{
	m_bouncesLimit += delta;
	m_bouncesLimit = std::max(0, m_bouncesLimit);
	m_bouncesLimit = std::min(5000, m_bouncesLimit);
}

void RayTracer::minusBounces(int delta)
{
	m_bouncesLimit -= delta;
	m_bouncesLimit = std::max(0, m_bouncesLimit);
	m_bouncesLimit = std::min(5000, m_bouncesLimit);
}

void RayTracer::renderAllAtOnce()
{
	// timings for 100 samples at 500x250:
	// 14.791584 s
	// 14.715808 s
	// 14.710577 s

	if (m_currentSample < m_allAtOnceSamplesLimit)
	{
		const Camera& camera = m_sceneSystem.activeScene().cameraSystem().currentCamera();

		// Parallel was about 3.6 times faster here. From 48 seconds to 13 seconds with a very low resolution and sample count.
		parallel_for(0, m_buffer->height(), [&](int y)
		{
			for (int x = 0; x < m_buffer->width(); ++x)
			{
				vec3 color;

				for (int sample = 0; sample < m_allAtOnceSamplesLimit; sample++)
				{
					float u = float(x + drand48()) / float(m_buffer->width());
					float v = float(y + drand48()) / float(m_buffer->height());

					Ray ray = camera.getRay(u, v);
					color += rayTrace(ray);
				}

				color /= float(m_allAtOnceSamplesLimit);

				m_buffer->setPixelColor3(x, y, color);
			}
		});

		m_currentSample = m_allAtOnceSamplesLimit;
	}
	else if (m_currentSample == m_allAtOnceSamplesLimit)
	{
		updateImageBuffer();
		m_currentSample++;
	}
}

void RayTracer::renderSamples()
{
	// timings for 100 samples at 500x250:
	// 15.426324 s
	// 15.402015 s
	// 15.347182 s

	if (m_samplesLimit == 0 || m_currentSample < m_samplesLimit)
	{
		// Take a copy of the camera so that it doesn't wobble.
		Camera camera = m_sceneSystem.activeScene().cameraSystem().currentCamera();

		// Single threaded
		//for (int j = 0; j < m_buffer->height; ++j)
		// Parallel, about twice the performance
		parallel_for(0, m_buffer->height(), [&](int y)
		{
			for (int x = 0; x < m_buffer->width(); ++x)
			{
				float u = float(x + drand48()) / float(m_buffer->width());
				float v = float(y + drand48()) / float(m_buffer->height());

				Ray ray = camera.getRay(u, v);
				vec3 color = rayTrace(ray);

				//http://stackoverflow.com/questions/22999487/update-the-average-of-a-continuous-sequence-of-numbers-in-constant-time
				// add to average
				m_buffer->setPixelColor3(x, y,
					(float(m_currentSample) * m_buffer->getPixelColor3(x, y) + color) / float(m_currentSample + 1));
			}
		});

		m_currentSample++;
		m_frameReady = true;
	}
}

void RayTracer::writeToPng(String filename)
{
	std::lock_guard<std::mutex> lock(m_bufferMutex);
	m_uintBuffer->writeToPng(filename);
}

void RayTracer::updateImageBuffer()
{
	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);
		copy8BitImageBuffer(*m_buffer, *m_uintBuffer);
	}

	m_uintBuffer->updateToNanoVG(m_windowSystem.mainWindow().nanoVG());
}

void RayTracer::renderNanoVG(NVGcontext* vg, float x, float y, float w, float h)
{
	if (!m_isEnabled)
		return;

	ImageBuffer<uint8_t>& readBuffer = uintBuffer();

	nvgSave(vg);

	//override the given parameters and reuse w and h...
	/*
	x = -g_rae->screenHalfWidthP();
	y = -g_rae->screenHalfHeightP();
	w = g_rae->screenWidthP();
	h = g_rae->screenHeightP();
	*/

	m_imgPaint = nvgImagePattern(vg, x, y, w, h, 0.0f, readBuffer.imageId(), 1.0f);
	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillPaint(vg, m_imgPaint);
	nvgFill(vg);

	nvgRestore(vg);
}

void RayTracer::render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem) const
{
}
