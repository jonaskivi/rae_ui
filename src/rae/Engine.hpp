#ifndef RAE_ENGINE_HPP
#define RAE_ENGINE_HPP

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ObjectFactory.hpp"
#include "core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "TransformSystem.hpp"
#include "CameraSystem.hpp"
#include "rae/ui/UISystem.hpp"
#include "RenderSystem.hpp"
#include "RayTracer.hpp"

namespace rae
{

class ISystem;
class RenderSystem;
class Entity;

class Engine
{
public:
	Engine(GLFWwindow* set_window); // TODO move the GLFWwindow into its own Window class.

	UISystem& getUISystem() { return m_uiSystem; }

	void run();
	bool update();

	void askForFrameUpdate();

	Entity& createAddObjectButton();
	Entity& createRandomBunnyEntity();
	Entity& createRandomCubeEntity();
	Entity& createCube(glm::vec3 position, glm::vec4 color);
	Entity& createBunny(glm::vec3 position, glm::vec4 color);
	Entity& createEmptyEntity();

	void createTestWorld();
	void createTestWorld2();

	void addSystem(ISystem& ownSystem);
	RenderSystem& getRenderSystem() { return m_renderSystem; }
	Input& getInput() { return m_input; }

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
	
	ObjectFactory m_objectFactory;

	std::vector<ISystem*> m_systems;

	TransformSystem		m_transformSystem;
	CameraSystem		m_cameraSystem;
	RayTracer			m_rayTracer;
	UISystem			m_uiSystem;
	RenderSystem		m_renderSystem;

	int m_meshID; // These should go someplace else...
	int m_modelID;
	int m_materialID;
	int m_bunnyMaterialID;
	int m_buttonMaterialID;
};

}

#endif

