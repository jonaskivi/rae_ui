#include "RenderSystem.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/core/Log.hpp"
#include "rae/core/Utils.hpp"
#include "rae/core/Time.hpp"
#include "rae/ui/Input.hpp"
#include "rae/ui/DebugSystem.hpp"

#include "rae/visual/Transform.hpp"
#include "rae/visual/Material.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Shader.hpp"

#include "rae/entity/EntitySystem.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"

using namespace rae;

int loadFonts(NVGcontext* vg)
{
	int font;
	font = nvgCreateFont(vg, "sans", "./data/fonts/Roboto-Regular.ttf");
	if (font == -1)
	{
		printf("Could not add font regular. File missing: ./data/fonts/Roboto-Regular.ttf\n");
		return -1;
	}

	font = nvgCreateFont(vg, "sans-bold", "./data/fonts/Roboto-Bold.ttf");
	if (font == -1)
	{
		printf("Could not add font bold. File missing: ./data/fonts/Roboto-Bold.ttf\n");
		return -1;
	}

	font = nvgCreateFont(vg, "logo", "./data/fonts/coolvetica_pupu.ttf");
	if (font == -1)
	{
		printf("Could not add font Avenir. File missing: ./data/fonts/coolvetica_pupu.ttf\n");
		return -1;
	}

	return 0;
}

RenderSystem::RenderSystem(
	const Time& time,
	EntitySystem& entitySystem,
	GLFWwindow* setWindow,
	Input& input,
	ScreenSystem& screenSystem,
	TransformSystem& transformSystem,
	CameraSystem& cameraSystem,
	AssetSystem& assetSystem,
	SelectionSystem& selectionSystem,
	RayTracer& rayTracer) :
		m_time(time),
		m_entitySystem(entitySystem),
		m_window(setWindow),
		m_input(input),
		m_screenSystem(screenSystem),
		m_transformSystem(transformSystem),
		m_cameraSystem(cameraSystem),
		m_assetSystem(assetSystem),
		m_selectionSystem(selectionSystem),
		m_rayTracer(rayTracer)
{
	addTable(m_meshLinks);
	addTable(m_materialLinks);

	debugTransform = new Transform(vec3(0,0,0));
	debugTransform2 = new Transform(vec3(0,0,0));

	initNanoVG();

	init();
}

RenderSystem::~RenderSystem()
{
}

void RenderSystem::initNanoVG()
{
	m_nanoVG = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	if (m_nanoVG == nullptr)
	{
		rae_log("Could not init nanovg.");
		getchar();
		exit(0);
		assert(0);
	}

	int windowWidth;
	int windowHeight;
	int windowPixelWidth;
	int windowPixelHeight;

	glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
	glfwGetFramebufferSize(m_window, &windowPixelWidth, &windowPixelHeight);

	m_screenSystem.osEventResizeWindow(windowWidth, windowHeight);
	m_screenSystem.osEventResizeWindowPixels(windowPixelWidth, windowPixelHeight);

	if (loadFonts(m_nanoVG) == -1)
	{
		rae_log("Could not load fonts");
		getchar();
		exit(0);
		assert(0);
	}

	m_rayTracer.setNanoVG(m_nanoVG);
	m_assetSystem.setNanoVG(m_nanoVG);

	//m_backgroundImage.load(m_nanoVG, "/Users/joonaz/Documents/jonas/opencv-3.2.0/samples/data/basketball1.png");
	//m_backgroundImage.load(m_nanoVG, "/Users/joonaz/Dropbox/taustakuvat/apple_galaxy.jpg");
}

void RenderSystem::init()
{
	// Background color
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 
	glEnable(GL_CULL_FACE);

	// Init basic shader
	if (m_basicShader.load("./data/shaders/basic.vert", "./data/shaders/basic.frag") == 0)
	{
		exit(0);
	}

	m_basicShader.use();

	// Init picking shader
	if (m_pickingShader.load("./data/shaders/picking.vert", "./data/shaders/picking.frag") == 0)
	{
		exit(0);
	}
}

Id RenderSystem::createBox()
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createBox entity: ", id);
	Mesh mesh;
	m_assetSystem.addMesh(id, std::move(mesh));

	// Got into nasty crashes when I first created the VBOs and then moved the mesh to the table.
	// Apparently you can't do that. Must first move mesh into table, and only create VBOs at the final memory pointers.
	Mesh& mesh2 = m_assetSystem.getMesh(id);
	mesh2.generateBox();
	mesh2.createVBOs();
	return id;
}

Id RenderSystem::createSphere()
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createSphere entity: ", id);
	Mesh mesh;
	m_assetSystem.addMesh(id, std::move(mesh));

	Mesh& mesh2 = m_assetSystem.getMesh(id);
	mesh2.generateSphere();
	mesh2.createVBOs();
	return id;
}

void RenderSystem::addMeshLink(Id id, Id linkId)
{
	m_meshLinks.assign(id, std::move(linkId));
}

void RenderSystem::addMaterialLink(Id id, Id linkId)
{
	m_materialLinks.assign(id, std::move(linkId));
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

		rae_log_error("OpenGL error: ", error, " - Received error in ", file, ":", line);

		err = glGetError();
	}
}

UpdateStatus RenderSystem::update()
{
	#ifdef RAE_DEBUG
		cout<<"RenderSystem::update().\n";
	#endif

	// RAE_TODO TEMP:
	m_backgroundImage.update(m_nanoVG);

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

	const auto& window = m_screenSystem.window();

	glViewport(0, 0, window.pixelWidth(), window.pixelHeight());

	// Clear the screen
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderSystem::render3D()
{
	render2dBackground();

	if (m_glRendererOn == false)
		return;

	m_basicShader.use();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	const Camera& camera = m_cameraSystem.getCurrentCamera();
	const Color white(1.0f, 1.0f, 1.0f, 1.0f);

	//debug
	Mesh* debugMesh = nullptr;
	Material* debugMaterial = nullptr;

	query<MeshLink>(m_meshLinks, [&](Id id, MeshLink& meshLink)
	{
		Material* material = nullptr;
		Mesh& mesh = m_assetSystem.getMesh(m_meshLinks.get(id));

		if (m_assetSystem.isMaterial(id))
			material = &m_assetSystem.getMaterial(id);
		else if (m_materialLinks.check(id))
			material = &m_assetSystem.getMaterial(m_materialLinks.get(id));

		if (m_transformSystem.hasTransform(id) &&
			material)
		{
			const Transform& transform = m_transformSystem.getTransform(id);

			//debugMaterial = &material;
			//debugMesh = &mesh;

			#ifdef RAE_DEBUG
				cout << "Going to render Mesh. id: " << id << "\n";
				cout << "MeshLink is: " << m_meshLinks.get(id) << "\n";
			#endif

			renderMesh(camera, transform, white, *material, mesh, m_selectionSystem.isSelected(id));
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
}

void RenderSystem::endFrame3D()
{
	{
		g_debugSystem->showDebugText("Esc to quit, R reset, F autofocus, H visualize focus, VB focus distance,"
			" NM aperture, KL bounces, G debug view, Tab UI, U fastmode");
		g_debugSystem->showDebugText("Movement: Second mouse button, WASDQE, Arrows");
		g_debugSystem->showDebugText("Y toggle resolution");
		g_debugSystem->showDebugText("Entities: " + std::to_string(m_entitySystem.entityCount()));
		g_debugSystem->showDebugText("Transforms: " + std::to_string(m_transformSystem.transformCount()));
		g_debugSystem->showDebugText("Meshes: " + std::to_string(m_assetSystem.meshCount()));
		g_debugSystem->showDebugText("Materials: " + std::to_string(m_assetSystem.materialCount()));
	}
}

void RenderSystem::renderPicking()
{
	const auto& window = m_screenSystem.window();

	glViewport(0, 0, window.pixelWidth(), window.pixelHeight());

	// Clear the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_pickingShader.use();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	const Camera& camera = m_cameraSystem.getCurrentCamera();

	query<MeshLink>(m_meshLinks, [&](Id id, MeshLink& meshLink)
	{
		Mesh& mesh = m_assetSystem.getMesh(m_meshLinks.get(id));

		if (m_transformSystem.hasTransform(id))
		{
			const Transform& transform = m_transformSystem.getTransform(id);

			#ifdef RAE_DEBUG
				rae_log("Going to render Mesh. id: ", id);
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

void RenderSystem::renderMeshPicking(
	const Camera& camera,
	const Transform& transform,
	const Mesh& mesh,
	Id id)
{
	mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
	mat4 rotationMatrix = glm::toMat4(transform.rotation);
	mat4 modelMatrix = translationMatrix * rotationMatrix;// * scaleMatrix;

	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;

	m_pickingShader.pushModelViewMatrix(combinedMatrix);
	m_pickingShader.pushEntityId(id);

	glBindTexture(GL_TEXTURE_2D, 0); // No texture

	mesh.render(m_pickingShader.getProgramId());
}

void RenderSystem::render2dBackground()
{
	// Nanovg

	const auto& window = m_screenSystem.window();

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(m_nanoVG, window.width(), window.height(), window.screenPixelRatio());

	// RAE_TODO RAYTRACER:
	//m_rayTracer.renderNanoVG(m_nanoVG, 0.0f, 0.0f, (float)window.width(), (float)window.height());
	//renderImageBuffer(m_nanoVG, m_backgroundImage, 0.0f, 0.0f, (float)window.width(), (float)window.height());

	Box rayWindow(
		vec3(0.0f, 0.0f, 0.0f),
		vec3(float(window.width()), float(window.height()), 0.0f));
	m_rayTracer.renderNanoVG(m_nanoVG,
		rayWindow.min().x, rayWindow.min().y,
		rayWindow.dimensions().x, rayWindow.dimensions().y);
	/*
	// Video window:
	Box imageWindow(
		vec3(float(window.width()) * 0.5f, float(window.height()) * 0.5f, 0.0f),
		vec3((float(window.width()) * 0.5f) * 2.0f, (float(window.height()) * 0.5f) * 2.0f, 0.0f));
	renderImageBuffer(m_nanoVG, m_backgroundImage,
		imageWindow.min().x, imageWindow.min().y,
		imageWindow.dimensions().x, imageWindow.dimensions().y);
	*/
	nvgEndFrame(m_nanoVG);
}

void RenderSystem::renderImageBuffer(NVGcontext* vg, ImageBuffer& readBuffer,
	float x, float y, float w, float h)
{
	nvgSave(vg);

	NVGpaint imgPaint = nvgImagePattern(vg, x, y, w, h, 0.0f, readBuffer.imageId(), 1.0f);
	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillPaint(vg, imgPaint);
	nvgFill(vg);

	nvgRestore(vg);
}

void RenderSystem::beginFrame2D()
{
	const auto& window = m_screenSystem.window();

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(m_nanoVG, window.width(), window.height(), window.screenPixelRatio());
}

void RenderSystem::render2D(NVGcontext* nanoVG)
{
	/*if (m_rayTracer.isInfoText())
	{
		nvgFontFace(m_nanoVG, "sans");

		float vertPos = 10.0f;

		nvgFontSize(m_nanoVG, 18.0f);
		nvgTextAlign(m_nanoVG, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(m_nanoVG, nvgRGBA(128, 128, 128, 192));
		nvgText(m_nanoVG, 10.0f, vertPos, m_fpsString.c_str(), nullptr); vertPos += 20.0f;

		nvgText(m_nanoVG, 10.0f, vertPos, "Esc to quit, R reset, F autofocus, H visualize focus, VB focus distance,"
			" NM aperture, KL bounces, G debug view, T text, U fastmode", nullptr); vertPos += 20.0f;
		nvgText(m_nanoVG, 10.0f, vertPos, "Movement: Second mouse button, WASDQE, Arrows", nullptr); vertPos += 20.0f;
		nvgText(m_nanoVG, 10.0f, vertPos, "Y toggle resolution", nullptr); vertPos += 20.0f;

		std::string entity_count_str = "Entities: " + std::to_string(m_entitySystem.entityCount());
		nvgText(m_nanoVG, 10.0f, vertPos, entity_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string transform_count_str = "Transforms: " + std::to_string(m_transformSystem.transformCount());
		nvgText(m_nanoVG, 10.0f, vertPos, transform_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string mesh_count_str = "Meshes: " + std::to_string(m_assetSystem.meshCount());
		nvgText(m_nanoVG, 10.0f, vertPos, mesh_count_str.c_str(), nullptr); vertPos += 20.0f;

		std::string material_count_str = "Materials: " + std::to_string(m_assetSystem.materialCount());
		nvgText(m_nanoVG, 10.0f, vertPos, material_count_str.c_str(), nullptr); vertPos += 20.0f;

		//nvgText(m_nanoVG, 10.0f, vertPos, m_pickedString.c_str(), nullptr);
	}
	*/
}

void RenderSystem::endFrame2D()
{
	nvgEndFrame(m_nanoVG);
}

void RenderSystem::clearImageRenderer()
{
	m_rayTracer.clear();
}

void RenderSystem::osEventResizeWindow(int width, int height)
{
	m_screenSystem.osEventResizeWindow(width, height);
	m_cameraSystem.setAspectRatio(float(width) / float(height));
}

void RenderSystem::osEventResizeWindowPixels(int width, int height)
{
	m_screenSystem.osEventResizeWindowPixels(width, height);
	m_cameraSystem.setAspectRatio(float(width) / float(height));
}
