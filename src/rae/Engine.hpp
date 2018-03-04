#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "rae/core/Types.hpp"
#include "rae/core/Time.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "rae/visual/TransformSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/ui/UISystem.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae/editor/EditorSystem.hpp"
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
	UpdateStatus update();

	void askForFrameUpdate();

	Id createAddObjectButton();
	Id createRandomBunnyEntity();
	Id createRandomCubeEntity();
	Id createCube(const vec3& position, const Color& color);
	Id createBunny(const vec3& position, const Color& color);

	void createTestWorld();
	void createTestWorld2();

	void destroyEntity(Id id);
	void defragmentTablesAsync();

	void addSystem(ISystem& system);
	void addRenderer3D(ISystem& system);
	void addRenderer2D(ISystem& system);
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

	bool m_running = true;

	Time m_time;
	ScreenSystem m_screenSystem;
	Input m_input;
	EntitySystem m_entitySystem;

	Array<ISystem*> m_systems;
	Array<ISystem*> m_renderers3D;
	Array<ISystem*> m_renderers2D;

	TransformSystem		m_transformSystem;
	CameraSystem		m_cameraSystem;
	AssetSystem			m_assetSystem;
	SelectionSystem		m_selectionSystem;
	RayTracer			m_rayTracer;
	UISystem			m_uiSystem;
	DebugSystem			m_debugSystem;
	RenderSystem		m_renderSystem;
	EditorSystem		m_editorSystem;

	Array<Id>			m_destroyEntities;
	bool				m_defragmentTables = false;

	int m_meshID; // These should go someplace else...
	int m_modelID;
	int m_materialID;
	int m_bunnyMaterialID;
	int m_buttonMaterialID;
};

}
