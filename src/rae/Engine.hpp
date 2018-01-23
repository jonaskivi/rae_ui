#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "rae/core/Types.hpp"

#include "rae/entity/EntitySystem.hpp"
#include "core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "rae/visual/TransformSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/ui/UISystem.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae_ray/RayTracer.hpp"

namespace rae
{

class ISystem;
class RenderSystem;
struct Entity;

class Engine
{
public:
	Engine(GLFWwindow* set_window); // TODO move the GLFWwindow into its own Window class.

	UISystem& getUISystem() { return m_uiSystem; }

	void run();
	bool update();

	void askForFrameUpdate();

	Id createAddObjectButton();
	Id createRandomBunnyEntity();
	Id createRandomCubeEntity();
	Id createCube(const vec3& position, const Colour& color);
	Id createBunny(const vec3& position, const Colour& color);

	void createTestWorld();
	void createTestWorld2();

	void destroyEntity(Id id);
	void defragmentTablesAsync();

	void addSystem(ISystem& ownSystem);
	RenderSystem& getRenderSystem() { return m_renderSystem; }
	Input& getInput() { return m_input; }

	RayTracer& getRayTracerSystem() { return m_rayTracer; }

	void osEventResizeWindow(int width, int height);
	void osEventResizeWindowPixels(int width, int height);

	void osMouseButtonPress(int set_button, float x, float y);
	void osMouseButtonRelease(int set_button, float x, float y);
	void osMouseMotion(float x, float y);
	void osScrollEvent(float scrollX, float scrollY);
	void osKeyEvent(int key, int scancode, int action, int mods);

	void onMouseEvent(const Input& input);
	void onKeyEvent(const Input& input);
	void reactToInput(const Input& input);

protected:

	GLFWwindow* m_window;

	double m_previousTime;
	double m_currentTime;
	bool m_running = true;

	ScreenSystem m_screenSystem;
	Input m_input;
	
	EntitySystem m_entitySystem;

	Array<ISystem*> m_systems;

	TransformSystem		m_transformSystem;
	CameraSystem		m_cameraSystem;
	SelectionSystem		m_selectionSystem;
	RayTracer			m_rayTracer;
	UISystem			m_uiSystem;
	RenderSystem		m_renderSystem;

	Array<Id>			m_destroyEntities;
	bool				m_defragmentTables = false;

	int m_meshID; // These should go someplace else...
	int m_modelID;
	int m_materialID;
	int m_bunnyMaterialID;
	int m_buttonMaterialID;
};

}
