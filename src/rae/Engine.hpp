#pragma once

#include "core/version.hpp"
#include <GL/glew.h>
#ifdef version_glfw
	#include <GLFW/glfw3.h>
#endif

#include "rae/core/Types.hpp"
#include "rae/core/Time.hpp"
#include "rae/scene/SceneSystem.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/ui/WindowSystem.hpp"
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
	Engine();
	Engine(const String& applicationName, int mainWindowWidth = -1, int mainWindowHeight = -1);

	// Restarts the engine if it was stopped with quit().
	void start();
	// Quit the current main loop. Other main loops might follow.
	void quit();
	// Run the main loop.
	void run();
	UpdateStatus update();

	void askForFrameUpdate();

	void destroyEntity(Id id);
	void defragmentTablesAsync();

	void addSystem(ISystem& system);
	// Adds windowSystem and input, possibly others.
	void addBaseSystems();
	void addRenderer3D(ISystem& system);
	void addRenderer2D(ISystem& system);

	Input& input() { return m_input; }
	ScreenSystem& screenSystem() { return m_screenSystem; }
	WindowSystem& windowSystem() { return m_windowSystem; }
	DebugSystem& debugSystem() { return m_debugSystem; }
	AssetSystem& assetSystem() { return m_assetSystem; }
	SceneSystem& sceneSystem() { return m_sceneSystem; }
	UISystem& uiSystem() { return m_uiSystem; }
	RayTracer& rayTracer() { return m_rayTracer; }
	RenderSystem& renderSystem() { return m_renderSystem; }
	EditorSystem& editorSystem() { return m_editorSystem; }

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

	bool m_running = true;

	Time m_time;
	ScreenSystem m_screenSystem;
	Input m_input;

	Array<ISystem*> m_systems;
	Array<ISystem*> m_renderers3D;
	Array<ISystem*> m_renderers2D;

	WindowSystem		m_windowSystem;
	DebugSystem			m_debugSystem; // Depends on UISystem.WindowSystem to be initialized first because of shaders.
	AssetSystem			m_assetSystem;
	SceneSystem			m_sceneSystem;
	UISystem			m_uiSystem;
	RayTracer			m_rayTracer;
	RenderSystem		m_renderSystem;
	EditorSystem		m_editorSystem;

	Array<Id>			m_destroyEntities;
	bool				m_defragmentTables = false;
};

}
