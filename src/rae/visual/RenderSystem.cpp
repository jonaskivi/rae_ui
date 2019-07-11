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

	if (m_basicShader.load() == 0)
	{
		exit(0);
	}

	m_basicShader.use();

	if (m_pickingShader.load() == 0)
	{
		exit(0);
	}

	if (m_singleColorShader.load() == 0)
	{
		exit(0);
	}

	if (m_outlineShader.load() == 0)
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

void RenderSystem::render3D(const Scene& scene, const Window& window, RenderSystem& renderSystem)
{
	if (!m_sceneSystem.hasActiveScene())
		return;

	//LOG_F(INFO, "Render.");

	if ((m_renderMode == RenderMode::MixedRayTraceRasterize ||
		m_renderMode == RenderMode::RayTrace) &&
		scene.isActive())
	{
		renderRayTracerOutput(window);
	}

	if (m_renderMode == RenderMode::MixedRayTraceRasterize ||
		m_renderMode == RenderMode::Rasterize)
	{
		m_basicShader.use();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		bool isWireframeMode = false;
		if (!isWireframeMode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		renderMeshes(scene);
		// Optimally would like to also render outline for raytraced output, but currently it isn't possible.
		// Maybe we could just raytrace it?
		renderOutline(scene);

		if (m_renderNormals)
		{
			renderNormals(scene);
		}
	}

	scene.editorSystem().render3D(scene, window, renderSystem);
}

void RenderSystem::endFrame3D()
{
	if (m_rayTracer.isEnabled())
	{
		m_rayTracer.updateDebugTexts();
	}
}

void RenderSystem::renderMeshes(const Scene& scene)
{
	//debug
	Mesh* debugMesh = nullptr;
	Material* debugMaterial = nullptr;

	const Camera& camera = scene.cameraSystem().currentCamera();
	auto& transformSystem = scene.transformSystem();
	auto& selectionSystem = scene.selectionSystem();
	auto& assetLinkSystem = scene.assetLinkSystem();

	glDisable(GL_STENCIL_TEST);

	query<MeshLink>(assetLinkSystem.meshLinks(), [&](Id id, const MeshLink& meshLink)
	{
		bool selected = selectionSystem.isPartOfSelection(id);
		bool hovered = selectionSystem.isHovered(id);

		if (not selected && not hovered)
		{
			Material* material = nullptr;

			if (m_assetSystem.isMaterial(id))
				material = &m_assetSystem.getMaterial(id);
			else if (assetLinkSystem.m_materialLinks.check(id))
				material = &m_assetSystem.getMaterial(assetLinkSystem.materialLinks().get(id));

			if (transformSystem.hasTransform(id) &&
				material)
			{
				const Mesh& mesh = m_assetSystem.getMesh(assetLinkSystem.meshLinks().get(id));
				const Transform& transform = transformSystem.getTransform(id);

				//debugMaterial = &material;
				//debugMesh = &mesh;

				#ifdef RAE_DEBUG
					LOG_F(INFO, "Going to render Mesh. id: %i", id);
					LOG_F(INFO, "MeshLink is: %i", assetLinkSystem.meshLinks().get(id));
				#endif

				renderMesh(camera, transform, *material, mesh);
			}
		}
	});

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // All fragments should update the stencil buffer.
	glStencilMask(0xFF); // Enable writing to the stencil buffer.

	query<Selected>(selectionSystem.selectedByParent(), [&](Id id)
	{
		Material* material = nullptr;

		if (m_assetSystem.isMaterial(id))
			material = &m_assetSystem.getMaterial(id);
		else if (assetLinkSystem.m_materialLinks.check(id))
			material = &m_assetSystem.getMaterial(assetLinkSystem.materialLinks().get(id));

		if (transformSystem.hasTransform(id) &&
			material)
		{
			const Mesh& mesh = m_assetSystem.getMesh(assetLinkSystem.meshLinks().get(id));
			const Transform& transform = transformSystem.getTransform(id);

			renderMesh(camera, transform, *material, mesh);
		}
	});

	// RAE_TODO This is pretty stupid. We need a separate query for hovers. We should try to combine hover and selected
	// queries so we can do this in one step.
	query<Hover>(selectionSystem.hovers(), [&](Id id)
	{
		bool selected = selectionSystem.isPartOfSelection(id);
		if (not selected)
		{
			Material* material = nullptr;

			if (m_assetSystem.isMaterial(id))
				material = &m_assetSystem.getMaterial(id);
			else if (assetLinkSystem.m_materialLinks.check(id))
				material = &m_assetSystem.getMaterial(assetLinkSystem.materialLinks().get(id));

			if (transformSystem.hasTransform(id) &&
				material)
			{
				const Mesh& mesh = m_assetSystem.getMesh(assetLinkSystem.meshLinks().get(id));
				const Transform& transform = transformSystem.getTransform(id);

				renderMesh(camera, transform, *material, mesh);
			}
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

void RenderSystem::renderOutline(const Scene& scene)
{
	const Camera& camera = scene.cameraSystem().currentCamera();
	auto& transformSystem = scene.transformSystem();
	auto& selectionSystem = scene.selectionSystem();
	auto& assetLinkSystem = scene.assetLinkSystem();

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00); // Disable writing to the stencil buffer.
	glDisable(GL_DEPTH_TEST);

	const auto hoverColor = Utils::createColor8bit(255, 165, 0);
	const auto activeColor = Utils::createColor8bit(0, 255, 165);

	query<MeshLink>(assetLinkSystem.meshLinks(), [&](Id id, const MeshLink& meshLink)
	{
		bool selected = selectionSystem.isPartOfSelection(id);
		bool hovered = selectionSystem.isHovered(id);
		if ((selected || hovered) && transformSystem.hasTransform(id))
		{
			const Mesh& mesh = m_assetSystem.getMesh(assetLinkSystem.meshLinks().get(id));
			Transform transform = transformSystem.getTransform(id);
			//transform.scale = transform.scale * 1.2f;

			renderMeshOutline(camera, transform, hovered ? hoverColor : activeColor, mesh);
		}
	});

	glStencilMask(0xFF);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
}

void RenderSystem::renderNormals(const Scene& scene)
{
	const Camera& camera = scene.cameraSystem().currentCamera();
	auto& transformSystem = scene.transformSystem();
	auto& selectionSystem = scene.selectionSystem();
	auto& assetLinkSystem = scene.assetLinkSystem();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	const auto normalColor = Utils::createColor8bit(255, 0, 255);

	query<MeshLink>(assetLinkSystem.meshLinks(), [&](Id id, const MeshLink& meshLink)
	{
		bool selected = selectionSystem.isPartOfSelection(id);
		if (selected && transformSystem.hasTransform(id))
		{
			const Mesh& mesh = m_assetSystem.getMesh(assetLinkSystem.meshLinks().get(id));
			Transform transform = transformSystem.getTransform(id);
			//transform.scale = transform.scale * 1.2f;

			renderMeshNormals(camera, transform, normalColor, mesh);
		}
	});

	glEnable(GL_DEPTH_TEST);
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
	const Material& material,
	const Mesh& mesh)
{
	glFrontFace(mesh.glWindingOrder());

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

	m_basicShader.pushTexture(material);

	mesh.render(m_basicShader.getProgramId());
}

void RenderSystem::renderMeshSingleColor(
	const Camera& camera,
	const Transform& transform,
	const Color& color,
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

void RenderSystem::renderMeshOutline(
	const Camera& camera,
	const Transform& transform,
	const Color& color,
	const Mesh& mesh)
{
	m_outlineShader.use();

	const float t_outlineMultiplier = 0.005f;
	float screenSizeFactor = camera.screenSizeFactor(transform.position)
		* t_outlineMultiplier
		* (1.0f / transform.scale.x); // Compensate for the effect of transform scale.

	mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
	mat4 rotationMatrix = glm::toMat4(transform.rotation);
	mat4 scaleMatrix = glm::scale(mat4(1.0f), transform.scale);
	mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;

	m_outlineShader.pushModelViewMatrix(combinedMatrix);
	m_outlineShader.pushColor(color);
	m_outlineShader.pushScreenSizeFactor(screenSizeFactor);

	mesh.renderForOutline(m_outlineShader.getProgramId());
}

void RenderSystem::renderMeshNormals(
	const Camera& camera,
	const Transform& transform,
	const Color& color,
	const Mesh& mesh)
{
	/* RAE_TODO some more efficient way to render the normals.
	m_singleColorShader.use();

	mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
	mat4 rotationMatrix = glm::toMat4(transform.rotation);
	mat4 scaleMatrix = glm::scale(mat4(1.0f), transform.scale);
	mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;

	m_singleColorShader.pushModelViewMatrix(combinedMatrix);
	m_singleColorShader.pushColor(color);
	*/

	const Array<vec3>& vertices = mesh.vertices();
	const Array<vec3>& normals = mesh.normals();

	// Assert if vertices and normals count doesn't match?

	const float VisualizeNormalsLength = 0.1f;

	for (int i = 0; i < (int)vertices.size() && i < (int)normals.size(); ++i)
	{
		vec3 transformedPos = transform.position + vertices[i];
		vec3 normalTip = transformedPos + (normals[i] * VisualizeNormalsLength);

		// This is not the most optimal way of drawing the normals.
		g_debugSystem->drawLine({ transformedPos, normalTip }, color);

		/* // Something like this could also work, but we should store the Mesh.
		Mesh lineMesh;

		lineMesh.generateLinesFromVertices({ vertices[i], normalTip });
		lineMesh.createVBOs(GL_DYNAMIC_DRAW);
		lineMesh.renderLines(m_singleColorShader.getProgramId());
		*/
	}
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
	m_rayTracer.requestClear();
}
