#include "RenderSystem.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "loguru/loguru.hpp"
#include "rae/core/Utils.hpp"
#include "rae/core/Time.hpp"
#include "rae/ui/Input.hpp"
#include "rae/ui/DebugSystem.hpp"

#include "rae/scene/Transform.hpp"
#include "rae/visual/Material.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Shader.hpp"

#include "rae/image/ImageBuffer.hpp"

#include "rae/entity/EntitySystem.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/WindowSystem.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/ui/WindowSystem.hpp"
#include "rae/ui/UISystem.hpp"
#include "rae/scene/SceneSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"

using namespace rae;

RenderSystem::RenderSystem(
	const Time& time,
	Input& input,
	ScreenSystem& screenSystem,
	WindowSystem& windowSystem,
	AssetSystem& assetSystem,
	UISystem& uiSystem,
	SceneSystem& sceneSystem,
	RayTracer& rayTracer) :
		ISystem("RenderSystem"),
		m_time(time),
		m_input(input),
		m_screenSystem(screenSystem),
		m_windowSystem(windowSystem),
		m_assetSystem(assetSystem),
		m_uiSystem(uiSystem),
		m_sceneSystem(sceneSystem),
		m_rayTracer(rayTracer)
{
	LOG_F(INFO, "Init %s", name().c_str());

	debugTransform = new Transform(vec3(0,0,0));
	debugTransform2 = new Transform(vec3(0,0,0));

	init();
}

RenderSystem::~RenderSystem()
{
}

// RAE_TODO pass nanoVG context to raytracer

void RenderSystem::init()
{
	// Background color
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// Init basic shader
	if (m_basicShader.load() == 0)
	{
		exit(0);
	}

	m_basicShader.use();

	// Init picking shader
	if (m_pickingShader.load() == 0)
	{
		exit(0);
	}

	if (m_singleColorShader.load() == 0)
	{
		exit(0);
	}
}

void RenderSystem::checkErrors(const char *file, int line)
{
	GLenum err(glGetError());

	while(err != GL_NO_ERROR)
	{
		std::string error;

		switch(err)
		{
			case GL_INVALID_OPERATION:              error = "GL_INVALID_OPERATION";              break;
			case GL_INVALID_ENUM:                   error = "GL_INVALID_ENUM";                   break;
			case GL_INVALID_VALUE:                  error = "GL_INVALID_VALUE";                  break;
			case GL_OUT_OF_MEMORY:                  error = "GL_OUT_OF_MEMORY";                  break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "GL_INVALID_FRAMEBUFFER_OPERATION";  break;
			default:                                error = "Unknown error: " + std::to_string((int)err); break;
		}

		LOG_F(ERROR, "OpenGL error: %s - Received error in %s : %i", error.c_str(), file, line);

		err = glGetError();
	}
}

UpdateStatus RenderSystem::update()
{
	#ifdef RAE_DEBUG
		LOG_F(INFO, "RenderSystem::update().");
	#endif

	return UpdateStatus::NotChanged; // for now
}

void RenderSystem::beginFrame3D()
{
	checkErrors(__FILE__, __LINE__);

	// Frame timing
	{
		m_nroFrames++;
		m_fpsTimer += m_time.deltaTime();

		if (m_fpsTimer >= 5.0)
		{
			m_fpsString = std::string("fps: ") + std::to_string(m_nroFrames / 5.0)
				+ " / " + std::to_string(5000.0f / m_nroFrames) + " ms";
			m_nroFrames = 0;
			m_fpsTimer = 0.0;
		}

		g_debugSystem->showDebugText(m_fpsString);
	}

	// Clear the screen
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderSystem::setViewport(const Rectangle& viewport, const Window& window)
{
	//FULLSCREEN: const auto& window = m_windowSystem.window();
	// glViewport(0, 0, window.pixelWidth(), window.pixelHeight());

	if (viewport.width == 0.0f)
	{
		glViewport(0, 0, window.pixelWidth(), window.pixelHeight());
	}
	else
	{
		glViewport(viewport.x, window.pixelHeight() - viewport.y - viewport.height, viewport.width, viewport.height);
	}
}

void RenderSystem::render3D(const Scene& scene, const Window& window)
{
	if (!m_sceneSystem.hasActiveScene())
		return;

	//LOG_F(INFO, "Render.");

	const Camera& camera = scene.cameraSystem().currentCamera();
	auto& transformSystem = scene.transformSystem();
	auto& selectionSystem = scene.selectionSystem();
	auto& assetLinkSystem = scene.assetLinkSystem();

	if ((m_renderMode == RenderMode::MixedRayTraceRasterize ||
		m_renderMode == RenderMode::RayTrace) &&
		scene.isActive())
	{
		renderRayTracerOutput(window);
	}

	if (m_renderMode != RenderMode::MixedRayTraceRasterize &&
		m_renderMode != RenderMode::Rasterize)
	{
		return;
	}

	m_basicShader.use();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	bool isWireframeMode = false;
	if (!isWireframeMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	const Color white(1.0f, 1.0f, 1.0f, 1.0f);

	//debug
	Mesh* debugMesh = nullptr;
	Material* debugMaterial = nullptr;

	query<MeshLink>(assetLinkSystem.meshLinks(), [&](Id id, const MeshLink& meshLink)
	{
		Material* material = nullptr;
		const Mesh& mesh = m_assetSystem.getMesh(assetLinkSystem.meshLinks().get(id));

		if (m_assetSystem.isMaterial(id))
			material = &m_assetSystem.getMaterial(id);
		else if (assetLinkSystem.m_materialLinks.check(id))
			material = &m_assetSystem.getMaterial(assetLinkSystem.materialLinks().get(id));

		if (transformSystem.hasTransform(id) &&
			material)
		{
			const Transform& transform = transformSystem.getTransform(id);

			//debugMaterial = &material;
			//debugMesh = &mesh;

			#ifdef RAE_DEBUG
				LOG_F(INFO, "Going to render Mesh. id: %i", id);
				LOG_F(INFO, "MeshLink is: %i", assetLinkSystem.meshLinks().get(id));
			#endif

			renderMesh(camera, transform, white, *material, mesh, selectionSystem.isSelected(id));
		}
	});

	/*
	if (debugTransform && debugMesh && debugMaterial)
	{
		debugTransform->update(time, delta_time);
		debugTransform2->update(time, delta_time);
		renderMesh(debugTransform, nullptr, debugMesh);
		renderMesh(debugTransform2, nullptr, debugMesh);
	}
	*/

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderSystem::endFrame3D()
{
	if (m_rayTracer.isEnabled())
	{
		m_rayTracer.updateDebugTexts();
	}
}

void RenderSystem::renderPicking(const Window& window)
{
	if (!m_sceneSystem.hasActiveScene())
		return;

	Scene& scene = m_sceneSystem.activeScene();
	auto& cameraSystem = scene.cameraSystem();
	auto& transformSystem = scene.transformSystem();
	auto& assetLinkSystem = scene.assetLinkSystem();

	const Camera& camera = cameraSystem.currentCamera();

	//FULLSCREEN: const auto& window = m_windowSystem.window();
	// glViewport(0, 0, window.pixelWidth(), window.pixelHeight());

	//TODO sceneIndex:
	Rectangle viewport = m_uiSystem.defaultScene().getViewportPixelRectangle(0);

	setViewport(viewport, window);

	// Clear the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_pickingShader.use();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	query<MeshLink>(assetLinkSystem.m_meshLinks, [&](Id id, MeshLink& meshLink)
	{
		Mesh& mesh = m_assetSystem.getMesh(assetLinkSystem.m_meshLinks.get(id));

		if (transformSystem.hasTransform(id))
		{
			const Transform& transform = transformSystem.getTransform(id);

			#ifdef RAE_DEBUG
				LOG_F(INFO, "Going to render Mesh. id: %i", id);
			#endif

			renderMeshPicking(camera, transform, mesh, id);
		}
	});
}

void RenderSystem::renderMesh(
	const Camera& camera,
	const Transform& transform,
	const Color& color,
	const Material& material,
	const Mesh& mesh,
	bool isSelected)
{
	mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
	mat4 rotationMatrix = glm::toMat4(transform.rotation);
	mat4 scaleMatrix = glm::scale(mat4(1.0f), transform.scale);
	mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;

	m_basicShader.pushModelViewMatrix(combinedMatrix);
	m_basicShader.pushModelMatrix(modelMatrix);
	m_basicShader.pushViewMatrix(camera.viewMatrix());

	glm::vec3 lightPos = glm::vec3(5.0f, 4.0f, 5.0f);
	m_basicShader.pushLightPosition(lightPos);

	if (isSelected)
	{
		m_basicShader.pushTempBlendColor(Color(0.0f, 2.0f, 2.0f, 1.0f));
	}
	else
	{
		m_basicShader.pushTempBlendColor(color);
	}

	m_basicShader.pushTexture(material);

	mesh.render(m_basicShader.getProgramId());
}

void RenderSystem::renderMeshSingleColor(
	const Camera& camera,
	const Transform& transform,
	const Color& color,
	const Material& material,
	const Mesh& mesh)
{
	m_singleColorShader.use();

	mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
	mat4 rotationMatrix = glm::toMat4(transform.rotation);
	mat4 scaleMatrix = glm::scale(mat4(1.0f), transform.scale);
	mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;

	m_singleColorShader.pushModelViewMatrix(combinedMatrix);
	m_singleColorShader.pushColor(color);

	mesh.render(m_singleColorShader.getProgramId());
}

void RenderSystem::renderMeshPicking(
	const Camera& camera,
	const Transform& transform,
	const Mesh& mesh,
	Id id)
{
	mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
	mat4 rotationMatrix = glm::toMat4(transform.rotation);
	mat4 scaleMatrix = glm::scale(mat4(1.0f), transform.scale);
	mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;

	m_pickingShader.pushModelViewMatrix(combinedMatrix);
	m_pickingShader.pushEntityId(id);

	glBindTexture(GL_TEXTURE_2D, 0); // No texture

	mesh.render(m_pickingShader.getProgramId());
}

void RenderSystem::render2dBackground(const Window& window)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgBeginFrame(window.nanoVG(), window.width(), window.height(), window.screenPixelRatio());
	renderImageNano(window.nanoVG(), m_backgroundImage.imageId(), 0.0f, 0.0f, (float)window.width(), (float)window.height());
	nvgEndFrame(window.nanoVG());
}

void RenderSystem::renderRayTracerOutput(const Window& window)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgBeginFrame(window.nanoVG(), window.width(), window.height(), window.screenPixelRatio());
	m_rayTracer.renderNanoVG(window.nanoVG(), 0.0f, 0.0f, (float)window.width(), (float)window.height());
	nvgEndFrame(window.nanoVG());
}

void RenderSystem::beginFrame2D(const Window& window)
{
	glViewport(0, 0, window.pixelWidth(), window.pixelHeight());
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(window.nanoVG(), window.width(), window.height(), window.screenPixelRatio());
}

void RenderSystem::render2D(UIScene& uiScene, NVGcontext* nanoVG)
{
}

void RenderSystem::endFrame2D(const Window& window)
{
	nvgEndFrame(window.nanoVG());
}

void RenderSystem::clearImageRenderer()
{
	m_rayTracer.clear();
}
