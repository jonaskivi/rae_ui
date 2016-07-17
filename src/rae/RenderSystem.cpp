#include "RenderSystem.hpp"

#include <iostream>
	using namespace std;

#include <glm/gtc/matrix_transform.hpp>

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

namespace Rae
{

int loadFonts(NVGcontext* vg)
{
	int font;
	font = nvgCreateFont(vg, "sans", "./data/fonts/Roboto-Regular.ttf");
	if (font == -1) {
		printf("Could not add font regular. File missing: ./data/fonts/Roboto-Regular.ttf\n");
		return -1;
	}
	font = nvgCreateFont(vg, "sans-bold", "./data/fonts/Roboto-Bold.ttf");
	if (font == -1) {
		printf("Could not add font bold. File missing: ./data/fonts/Roboto-Bold.ttf\n");
		return -1;
	}
	return 0;
}

RenderSystem::RenderSystem(ObjectFactory* set_factory, GLFWwindow* set_window, Input& input)
: m_objectFactory(set_factory),
m_window(set_window),
m_input(input),
m_nroFrames(0),
m_fpsTimer(0.0),
m_fpsString("fps:"),
//m_pickedString("Nothing picked"),
vg(nullptr),
camera(/*fieldOfView*/Math::toRadians(20.0f), /*aspect*/16.0f / 9.0f, /*aperture*/0.1f, /*focusDistance*/10.0f),
imageRenderer(&camera)
{
	debugTransform = new Transform(1,0,0,0);

	initNanoVG();

	init();

	using std::placeholders::_1;
	m_input.registerMouseButtonPressCallback(std::bind(&RenderSystem::onMouseEvent, this, _1));
	m_input.registerMouseButtonReleaseCallback(std::bind(&RenderSystem::onMouseEvent, this, _1));
	m_input.registerMouseMotionCallback(std::bind(&RenderSystem::onMouseEvent, this, _1));
	m_input.registerScrollCallback(std::bind(&RenderSystem::onMouseEvent, this, _1));
	m_input.registerKeyEventCallback(std::bind(&RenderSystem::onKeyEvent, this, _1));
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

	imageRenderer.setNanovgContext(vg);
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
	Mesh& mesh = m_objectFactory->createMesh();

	mesh.generateBox();
	mesh.createVBOs();
	return mesh;
}

Material& RenderSystem::createAnimatingMaterial(int type, const glm::vec4& color)
{
	Material& material = m_objectFactory->createMaterial(type, color);

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

void RenderSystem::update(double time, double delta_time, std::vector<Entity>& entities)
{
	#ifdef RAE_DEBUG
		cout<<"RenderSystem::update().\n";
	#endif

	checkErrors(__FILE__, __LINE__);

	m_nroFrames++;
	m_fpsTimer += delta_time;

	if( m_fpsTimer >= 5.0 )
	{
		m_fpsString = std::string("fps: ") + std::to_string(m_nroFrames / 5.0)
			+ " / " + std::to_string(5000.0f / m_nroFrames) + " ms";
		m_nroFrames = 0;
		m_fpsTimer = 0.0;
	}

	for (auto& material : m_objectFactory->materials())
	{
		material.update(vg, time);
	}

	updateCamera(time, delta_time);

	imageRenderer.update(time, delta_time);

	render(time, delta_time, entities);

	render2d(time, delta_time);
	
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

	for(auto& entity : entities)
	{
		Transform* transform = nullptr;
		Material*  material  = nullptr;
		Mesh*      mesh      = nullptr;
		
		for(auto& componentIndex : entity.components())
		{
			switch( (ComponentType)componentIndex.type )
			{
				default:
					//cout << "ERROR: Strange type: " << componentIndex.type << "\n";
				break;
				case ComponentType::TRANSFORM:
					if(transform == nullptr)
						transform = m_objectFactory->getTransform(componentIndex.id);
					else cout << "ERROR: Found another transform component. id: " << componentIndex.id << "\n";
				break;
				case ComponentType::MATERIAL:
					if(material == nullptr)
					{
						material = m_objectFactory->getMaterial(componentIndex.id);
						debugMaterial = material;
					}
					else cout << "ERROR: Found another material component. id: " << componentIndex.id << "\n";
				break;
				case ComponentType::MESH:
					if(mesh == nullptr)
					{
						mesh = m_objectFactory->getMesh(componentIndex.id);
						debugMesh = mesh;
					}
					else cout << "ERROR: Found another mesh component. id: " << componentIndex.id << "\n";
				break;
			}
		}

		if( transform && mesh )
		{
			#ifdef RAE_DEBUG
				cout << "Going to render Mesh. id: " << mesh->id() << "\n";
			#endif

			// Update animation... TODO move this elsewhere.
			transform->update(time, delta_time);

			renderMesh(transform, material, mesh);
		}
		//else cout << "No mesh and no transform.\n";
	}

	if (debugTransform && debugMesh && debugMaterial)
	{
		debugTransform->update(time, delta_time);
		renderMesh(debugTransform, nullptr, debugMesh);
	}
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
	for(auto& entity : entities)
	{
		Transform* transform = nullptr;
		Mesh*      mesh      = nullptr;
		
		for(auto& componentIndex : entity.components())
		{
			switch( (ComponentType)componentIndex.type )
			{
				default:
					//cout << "ERROR: Strange type: " << componentIndex.type << "\n";
				break;
				case ComponentType::TRANSFORM:
					if(transform == nullptr)
						transform = m_objectFactory->getTransform(componentIndex.id);
					else cout << "ERROR: Found another transform component. id: " << componentIndex.id << "\n";
				break;
				case ComponentType::MATERIAL:
				break;
				case ComponentType::MESH:
					if(mesh == nullptr)
						mesh = m_objectFactory->getMesh(componentIndex.id);
					else cout << "ERROR: Found another mesh component. id: " << componentIndex.id << "\n";
				break;
			}
		}

		if( transform && mesh )
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

void RenderSystem::renderMesh(Transform* transform, Material* material, Mesh* mesh)
{
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glm::mat4& modelMatrix = transform->modelMatrix();
	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.m_projectionMatrix * camera.m_viewMatrix * modelMatrix;

	glUniformMatrix4fv(modelViewMatrixUni, 1, GL_FALSE, &combinedMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixUni, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixUni, 1, GL_FALSE, &camera.m_viewMatrix[0][0]);

	glm::vec3 lightPos = glm::vec3(2.0f, 0.0f, 0.0f);
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

void RenderSystem::renderMeshPicking(Transform* transform, Mesh* mesh, int entity_id)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glm::mat4& modelMatrix = transform->modelMatrix();
	// The model-view-projection matrix
	glm::mat4 combinedMatrix = camera.m_projectionMatrix * camera.m_viewMatrix * modelMatrix;

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
		imageRenderer.renderNanoVG(vg, 0, 0, m_windowWidth, m_windowHeight);
	nvgEndFrame(vg);
}

void RenderSystem::render2d(double time, double delta_time)
{
	//nanovg

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (imageRenderer.isInfoText())
	{
		nvgBeginFrame(vg, m_windowWidth, m_windowHeight, m_screenPixelRatio);
			nvgFontFace(vg, "sans");

			float vertPos = 10.0f;

			nvgFontSize(vg, 18.0f);
			nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			nvgFillColor(vg, nvgRGBA(128, 128, 128, 192));
			nvgText(vg, 10.0f, vertPos, m_fpsString.c_str(), nullptr); vertPos += 20.0f;

			nvgText(vg, 10.0f, vertPos, "Esc to quit, R reset, F autofocus, VB focus distance,"
				" NM aperture, G debug view, T text, U fastmode", nullptr); vertPos += 20.0f;
			nvgText(vg, 10.0f, vertPos, "Movement: Second mouse button, WASDQE, Arrows", nullptr); vertPos += 20.0f;
			nvgText(vg, 10.0f, vertPos, "Y toggle resolution", nullptr); vertPos += 20.0f;

			std::string entity_count_str = "Entities: " + std::to_string(m_objectFactory->entityCount());
			nvgText(vg, 10.0f, vertPos, entity_count_str.c_str(), nullptr); vertPos += 20.0f;

			std::string transform_count_str = "Transforms: " + std::to_string(m_objectFactory->transformCount());
			nvgText(vg, 10.0f, vertPos, transform_count_str.c_str(), nullptr); vertPos += 20.0f;

			std::string mesh_count_str = "Meshes: " + std::to_string(m_objectFactory->meshCount());
			nvgText(vg, 10.0f, vertPos, mesh_count_str.c_str(), nullptr); vertPos += 20.0f;

			std::string material_count_str = "Materials: " + std::to_string(m_objectFactory->materialCount());
			nvgText(vg, 10.0f, vertPos, material_count_str.c_str(), nullptr); vertPos += 20.0f;

			//nvgText(vg, 10.0f, vertPos, m_pickedString.c_str(), nullptr);

		nvgEndFrame(vg);
	}
}

void RenderSystem::clearImageRenderer()
{
	imageRenderer.clear();
}

void RenderSystem::osEventResizeWindow(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;
	m_screenPixelRatio = (float)m_windowPixelWidth / (float)m_windowWidth;
}

void RenderSystem::osEventResizeWindowPixels(int width, int height)
{
	m_windowPixelWidth = width;
	m_windowPixelHeight = height;
	m_screenPixelRatio = (float)m_windowPixelWidth / (float)m_windowWidth;
}

void RenderSystem::onMouseEvent(const Input& input)
{
	if (input.eventType == EventType::MOUSE_MOTION)
	{
		if (input.mouse.button(MouseButton::SECOND))
		{
			//cout << "RenderSystem mouse motion. x: " << input->mouse.xRel
			//	<< " y: " << input->mouse.yRel << endl;

			const float rotateSpeedMul = 5.0f;

			camera.rotateYaw(input.mouse.xRel * -1.0f * rotateSpeedMul);
			camera.rotatePitch(input.mouse.yRel * -1.0f * rotateSpeedMul);
		}
	}
	else if (input.eventType == EventType::MOUSE_BUTTON_PRESS)
	{
		//cout << "RenderSystem mouse press. x: " << input->mouse.x
		//	<< " y: " << input->mouse.y << endl;
	}
	else if (input.eventType == EventType::MOUSE_BUTTON_RELEASE)
	{
		if (input.mouse.eventButton == MouseButton::FIRST)
		{
			//cout << "RenderSystem mouse release. x: " << input->mouse.x
			//	<< " y: " << input->mouse.y << endl;
		}
	}

	if (input.eventType == EventType::SCROLL)
	{
		const float scrollSpeedMul = -0.1f;
		camera.plusFieldOfView(input.mouse.scrollY * scrollSpeedMul);
	}
}

void RenderSystem::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KEY_PRESS)
	{
		switch (input.key.value)
		{
			case KeySym::R: clearImageRenderer(); break;
			case KeySym::G: toggleGlRenderer(); break; // more like debug view currently
			case KeySym::T: imageRenderer.toggleInfoText(); break;
			case KeySym::Y: imageRenderer.toggleBufferQuality(); break;
			case KeySym::U: imageRenderer.toggleFastMode(); break;
			case KeySym::_1: imageRenderer.showScene(1); break;
			case KeySym::_2: imageRenderer.showScene(2); break;
			default:
			break;
		}
	}
}

void RenderSystem::updateCamera(double time, double delta_time)
{
	camera.setAspectRatio( float(m_windowPixelWidth) / float(m_windowPixelHeight) );

	if (m_input.getKeyState(KeySym::Control_L))
		camera.setCameraSpeedDown(true);
	else camera.setCameraSpeedDown(false);

	if (m_input.getKeyState(KeySym::Shift_L))
		camera.setCameraSpeedUp(true);
	else camera.setCameraSpeedUp(false);

	// Rotation with arrow keys
	if (m_input.getKeyState(KeySym::Left))
		camera.rotateYaw(float(delta_time), +1);
	else if (m_input.getKeyState(KeySym::Right))
		camera.rotateYaw(float(delta_time), -1);

	if (m_input.getKeyState(KeySym::Up))
		camera.rotatePitch(float(delta_time), +1);
	else if (m_input.getKeyState(KeySym::Down))
		camera.rotatePitch(float(delta_time), -1);

	// Camera movement
	if (m_input.getKeyState(KeySym::W)) { camera.moveForward(float(delta_time)); }
	if (m_input.getKeyState(KeySym::S)) { camera.moveBackward(float(delta_time)); }
	if (m_input.getKeyState(KeySym::D)) { camera.moveRight(float(delta_time)); }
	if (m_input.getKeyState(KeySym::A)) { camera.moveLeft(float(delta_time)); }
	if (m_input.getKeyState(KeySym::E)) { camera.moveUp(float(delta_time)); }
	if (m_input.getKeyState(KeySym::Q)) { camera.moveDown(float(delta_time)); }

	if (m_input.getKeyState(KeySym::N)) { camera.minusAperture(); }
	if (m_input.getKeyState(KeySym::M)) { camera.plusAperture(); }

	if (glfwGetKey( m_window, GLFW_KEY_V ) == GLFW_PRESS) { camera.minusFocusDistance(); }
	if (glfwGetKey( m_window, GLFW_KEY_B ) == GLFW_PRESS) { camera.plusFocusDistance(); }

	if (glfwGetKey( m_window, GLFW_KEY_F ) == GLFW_PRESS)
	{
		imageRenderer.autoFocus();
		debugTransform->setTarget(imageRenderer.debugHitRecord.point, 0.5f);
	}

	if (camera.update())
	{
		imageRenderer.clear();
	}

	//cout<<"camerapos: x: "<<m_cameraPosition.x << " y: " << m_cameraPosition.y << " z: " << m_cameraPosition.z
	//	<< " yaw: " << m_yawAngle << " pitch: " << m_pitchAngle << "\n";
}

} //end namespace Rae

