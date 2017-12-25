#include "RenderSystem.hpp"

#include <iostream>
	using namespace std;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using glm::vec3;
using glm::quat;
using glm::mat4;

#include "nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "core/Utils.hpp"
#include "ui/Input.hpp"

#include "ObjectFactory.hpp"
#include "Transform.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Entity.hpp"
#include "Shader.hpp"
#include "ComponentType.hpp"

#include "CameraSystem.hpp"

namespace rae
{

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

RenderSystem::RenderSystem(ObjectFactory& objectFactory,
	GLFWwindow* setWindow,
	Input& input,
	CameraSystem& cameraSystem,
	TransformSystem& transformSystem,
	UISystem& uiSystem,
	RayTracer& rayTracer)
: m_objectFactory(objectFactory),
m_window(setWindow),
m_input(input),
m_nroFrames(0),
m_fpsTimer(0.0),
m_fpsString("fps:"),
//m_pickedString("Nothing picked"),
vg(nullptr),
m_cameraSystem(cameraSystem),
m_transformSystem(transformSystem),
m_uiSystem(uiSystem),
m_rayTracer(rayTracer)
{
	debugTransform = new Transform(vec3(0,0,0));
	debugTransform2 = new Transform(vec3(0,0,0));

	initNanoVG();

	init();
}

RenderSystem::~RenderSystem()
{
	glDeleteProgram(shaderID);
	glDeleteProgram(pickingShaderID);
}

void RenderSystem::initNanoVG()
{
	vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	if (vg == nullptr)
	{
		cout << "Could not init nanovg.\n";
		getchar();
		exit(0);
		assert(0);
	}

	glfwGetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
	glfwGetFramebufferSize(m_window, &m_windowPixelWidth, &m_windowPixelHeight);
	// Calculate pixel ratio for hi-dpi screens.
	m_screenPixelRatio = (float)m_windowPixelWidth / (float)m_windowWidth;

	if( loadFonts(vg) == -1 )
	{
		cout << "Could not load fonts\n";
		getchar();
		exit(0);
		assert(0);
	}

	m_rayTracer.setNanovgContext(vg);

	//m_backgroundImage.load(vg, "/Users/joonaz/Documents/jonas/opencv-3.2.0/samples/data/basketball1.png");
	//m_backgroundImage.load(vg, "/Users/joonaz/Dropbox/taustakuvat/apple_galaxy.jpg");
}

void RenderSystem::init()
{
	// Background color
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 
	glEnable(GL_CULL_FACE);

	// Init basic shader

	shaderID = loadShaders( "./data/shaders/basic.vert", "./data/shaders/basic.frag" );
	if(shaderID == 0)
		exit(0);

	modelViewMatrixUni = glGetUniformLocation(shaderID, "modelViewProjectionMatrix");
	viewMatrixUni = glGetUniformLocation(shaderID, "viewMatrix");
	modelMatrixUni = glGetUniformLocation(shaderID, "modelMatrix");

	glUseProgram(shaderID);
	lightPositionUni = glGetUniformLocation(shaderID, "lightPosition_worldspace");

	textureUni  = glGetUniformLocation(shaderID, "textureSampler");

	// Init picking shader

	pickingShaderID = loadShaders( "./data/shaders/picking.vert", "./data/shaders/picking.frag" );
	if(pickingShaderID == 0)
		exit(0);
	pickingModelViewMatrixUni = glGetUniformLocation(pickingShaderID, "modelViewProjectionMatrix");
	entityUni = glGetUniformLocation(pickingShaderID, "entityID");
}

Mesh& RenderSystem::createBox()
{
	Mesh& mesh = m_objectFactory.createMesh();

	mesh.generateBox();
	mesh.createVBOs();
	return mesh;
}

Material& RenderSystem::createMaterial(int type, const glm::vec4& color)
{
	Material& material = m_objectFactory.createMaterial(type, color);
	material.generateFBO(vg);
	return material;
}

Material& RenderSystem::createAnimatingMaterial(int type, const glm::vec4& color)
{
	Material& material = m_objectFactory.createMaterial(type, color);
	material.animate(true);
	material.generateFBO(vg);
	return material;
}

void RenderSystem::checkErrors(const char *file, int line)
{
	GLenum err (glGetError());

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

		#ifdef RAE_DEBUG
			cerr << "OpenGL error: " << error << " - Received error in " << file << ":" << line << "\n";
		#endif
		err = glGetError();
	}
}

bool RenderSystem::update(double time, double delta_time, std::vector<Entity>& entities)
{
	#ifdef RAE_DEBUG
		cout<<"RenderSystem::update().\n";
	#endif

	checkErrors(__FILE__, __LINE__);

	m_nroFrames++;
	m_fpsTimer += delta_time;

	if (m_fpsTimer >= 5.0)
	{
		m_fpsString = std::string("fps: ") + std::to_string(m_nroFrames / 5.0)
			+ " / " + std::to_string(5000.0f / m_nroFrames) + " ms";
		m_nroFrames = 0;
		m_fpsTimer = 0.0;
	}

	// TODO move to material system
	for (auto& material : m_objectFactory.materials())
	{
		material.update(vg, time);
	}

	//JONDE TEMP:
	m_backgroundImage.update(vg);

	render(time, delta_time, entities);

	m_uiSystem.render(time, delta_time, entities, vg, m_windowWidth, m_windowHeight, m_screenPixelRatio);
	//JONDE TEMP RAYTRACER render2d(time, delta_time);

	return false; // for now
}

void RenderSystem::render(double time, double delta_time, std::vector<Entity>& entities)
{
	glViewport(0, 0, m_windowPixelWidth, m_windowPixelHeight);

	// Clear the screen
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	render2dBackground(time, delta_time);

	if (m_glRendererOn == false)
		return;

	glUseProgram(shaderID);

	//debug
	Mesh* debugMesh = nullptr;
	Material* debugMaterial = nullptr;

	for (auto& entity : entities)
	{
		const Transform& transform = m_transformSystem.getTransform(entity.id());
		Material*  material  = nullptr;
		Mesh*      mesh      = nullptr;
		
		for (auto& componentIndex : entity.components())
		{
			switch( (ComponentType)componentIndex.type )
			{
				default:
					//cout << "ERROR: Strange type: " << componentIndex.type << "\n";
				break;
				/*JONDE REMOVE case ComponentType::TRANSFORM:
					if(transform == nullptr)
						transform = m_objectFactory.getTransform(componentIndex.id);
					else cout << "ERROR: Found another transform component. id: " << componentIndex.id << "\n";
				break;
				*/
				case ComponentType::MATERIAL:
					if(material == nullptr)
					{
						material = m_objectFactory.getMaterial(componentIndex.id);
						debugMaterial = material;
					}
					else cout << "ERROR: Found another material component. id: " << componentIndex.id << "\n";
				break;
				case ComponentType::MESH:
					if(mesh == nullptr)
					{
						mesh = m_objectFactory.getMesh(componentIndex.id);
						debugMesh = mesh;
					}
					else cout << "ERROR: Found another mesh component. id: " << componentIndex.id << "\n";
				break;
			}
		}

		if (mesh)
		{
			#ifdef RAE_DEBUG
				cout << "Going to render Mesh. id: " << mesh->id() << "\n";
			#endif

			renderMesh(transform, material, mesh);
		}
		//else cout << "No mesh and no transform.\n";
	}

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

void RenderSystem::renderPicking(std::vector<Entity>& entities)
{
	glViewport(0, 0, m_windowPixelWidth, m_windowPixelHeight);
	//glViewport(0, 0, m_windowWidth, m_windowHeight);
	
	// Clear the screen
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(pickingShaderID);

	int entity_id = 0;
	for (auto& entity : entities)
	{
		const Transform& transform = m_transformSystem.getTransform(entity.id());
		Mesh*      mesh      = nullptr;
		
		for (auto& componentIndex : entity.components())
		{
			switch( (ComponentType)componentIndex.type )
			{
				default:
					//cout << "ERROR: Strange type: " << componentIndex.type << "\n";
				break;
				/*JONDE REMOVE case ComponentType::TRANSFORM:
					if(transform == nullptr)
						transform = m_objectFactory.getTransform(componentIndex.id);
					else cout << "ERROR: Found another transform component. id: " << componentIndex.id << "\n";
				break;
				*/
				case ComponentType::MATERIAL:
				break;
				case ComponentType::MESH:
					if(mesh == nullptr)
						mesh = m_objectFactory.getMesh(componentIndex.id);
					else cout << "ERROR: Found another mesh component. id: " << componentIndex.id << "\n";
				break;
			}
		}

		if (mesh)
		{
			#ifdef RAE_DEBUG
				cout << "Going to render Mesh. id: " << mesh->id() << "\n";
			#endif

			renderMeshPicking(transform, mesh, entity_id);
		}
		//else cout << "No mesh and no transform.\n";

		++entity_id;
	}
}

void RenderSystem::renderMesh(const Transform& transform, Material* material, Mesh* mesh)
{
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	//JONDE REMOVE glm::mat4& modelMatrix = transform->modelMatrix();

	mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
	mat4 rotationMatrix = glm::toMat4(transform.rotation);
	mat4 modelMatrix = translationMatrix * rotationMatrix;// * scaleMatrix;

	const Camera& camera = m_cameraSystem.getCurrentCamera();
	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;

	glUniformMatrix4fv(modelViewMatrixUni, 1, GL_FALSE, &combinedMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixUni, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixUni, 1, GL_FALSE, &camera.viewMatrix()[0][0]);

	glm::vec3 lightPos = glm::vec3(5.0f, 4.0f, 5.0f);
	glUniform3f(lightPositionUni, lightPos.x, lightPos.y, lightPos.z);

	// Bind texture in Texture Unit 0
	if( material != nullptr )
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, material->textureID());
		// Set textureSampler to use Texture Unit 0
		glUniform1i(textureUni, 0);
	}
	else glBindTexture(GL_TEXTURE_2D, 0);
	
	mesh->render(shaderID);
}

void RenderSystem::renderMeshPicking(const Transform& transform, Mesh* mesh, int entity_id)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	mat4 translationMatrix = glm::translate(mat4(1.0f), transform.position);
	mat4 rotationMatrix = glm::toMat4(transform.rotation);
	mat4 modelMatrix = translationMatrix * rotationMatrix;// * scaleMatrix;

	const Camera& camera = m_cameraSystem.getCurrentCamera();
	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.getProjectionAndViewMatrix() * modelMatrix;

	glUniformMatrix4fv(pickingModelViewMatrixUni, 1, GL_FALSE, &combinedMatrix[0][0]);
	glUniform1i(entityUni, entity_id);
	
	glBindTexture(GL_TEXTURE_2D, 0); // No texture
	
	mesh->render(pickingShaderID);
}

void RenderSystem::render2dBackground(double time, double delta_time)
{
	//nanovg

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(vg, m_windowWidth, m_windowHeight, m_screenPixelRatio);
	
	//JONDE RAYTRACER:
	//m_rayTracer.renderNanoVG(vg, 0.0f, 0.0f, (float)m_windowWidth, (float)m_windowHeight);
	//renderImageBuffer(vg, m_backgroundImage, 0.0f, 0.0f, (float)m_windowWidth, (float)m_windowHeight);

	Box rayWindow(
		vec3(0.0f, 0.0f, 0.0f),
		vec3(float(m_windowWidth) * 0.5f, float(m_windowHeight) * 0.5f, 0.0f));
	m_rayTracer.renderNanoVG(vg,
		rayWindow.min().x, rayWindow.min().y,
		rayWindow.dimensions().x, rayWindow.dimensions().y);
	Box imageWindow(
		vec3(float(m_windowWidth) * 0.5f, float(m_windowHeight) * 0.5f, 0.0f),
		vec3((float(m_windowWidth) * 0.5f) * 2.0f, (float(m_windowHeight) * 0.5f) * 2.0f, 0.0f));
	renderImageBuffer(vg, m_backgroundImage,
		imageWindow.min().x, imageWindow.min().y,
		imageWindow.dimensions().x, imageWindow.dimensions().y);
		
	nvgEndFrame(vg);
}

void RenderSystem::renderImageBuffer(NVGcontext* vg, ImageBuffer& readBuffer,
	float x, float y, float w, float h)
{
	nvgSave(vg);

	NVGpaint imgPaint = nvgImagePattern(vg, x, y, w, h, 0.0f, readBuffer.imageId, 1.0f);
	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillPaint(vg, imgPaint);
	nvgFill(vg);

	nvgRestore(vg);
}

void RenderSystem::render2d(double time, double delta_time)
{
	//nanovg

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (m_rayTracer.isInfoText())
	{
		nvgBeginFrame(vg, m_windowWidth, m_windowHeight, m_screenPixelRatio);
			nvgFontFace(vg, "sans");

			float vertPos = 10.0f;

			nvgFontSize(vg, 18.0f);
			nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			nvgFillColor(vg, nvgRGBA(128, 128, 128, 192));
			nvgText(vg, 10.0f, vertPos, m_fpsString.c_str(), nullptr); vertPos += 20.0f;

			nvgText(vg, 10.0f, vertPos, "Esc to quit, R reset, F autofocus, H visualize focus, VB focus distance,"
				" NM aperture, KL bounces, G debug view, T text, U fastmode", nullptr); vertPos += 20.0f;
			nvgText(vg, 10.0f, vertPos, "Movement: Second mouse button, WASDQE, Arrows", nullptr); vertPos += 20.0f;
			nvgText(vg, 10.0f, vertPos, "Y toggle resolution", nullptr); vertPos += 20.0f;

			std::string entity_count_str = "Entities: " + std::to_string(m_objectFactory.entityCount());
			nvgText(vg, 10.0f, vertPos, entity_count_str.c_str(), nullptr); vertPos += 20.0f;

			std::string transform_count_str = "Transforms: " + std::to_string(m_transformSystem.transformCount());
			nvgText(vg, 10.0f, vertPos, transform_count_str.c_str(), nullptr); vertPos += 20.0f;

			std::string mesh_count_str = "Meshes: " + std::to_string(m_objectFactory.meshCount());
			nvgText(vg, 10.0f, vertPos, mesh_count_str.c_str(), nullptr); vertPos += 20.0f;

			std::string material_count_str = "Materials: " + std::to_string(m_objectFactory.materialCount());
			nvgText(vg, 10.0f, vertPos, material_count_str.c_str(), nullptr); vertPos += 20.0f;

			//nvgText(vg, 10.0f, vertPos, m_pickedString.c_str(), nullptr);

		nvgEndFrame(vg);
	}
}

void RenderSystem::clearImageRenderer()
{
	m_rayTracer.clear();
}

void RenderSystem::osEventResizeWindow(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;
	m_screenPixelRatio = (float)m_windowPixelWidth / (float)m_windowWidth;
	m_cameraSystem.setAspectRatio(float(width) / float(height));
}

void RenderSystem::osEventResizeWindowPixels(int width, int height)
{
	m_windowPixelWidth = width;
	m_windowPixelHeight = height;
	m_screenPixelRatio = (float)m_windowPixelWidth / (float)m_windowWidth;
	m_cameraSystem.setAspectRatio(float(width) / float(height));
}

} //end namespace rae

