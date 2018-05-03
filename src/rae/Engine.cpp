#include "rae/Engine.hpp"

#include <glm/glm.hpp>

#include "loguru/loguru.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/scene/Transform.hpp"
#include "rae/visual/Material.hpp"
#include "rae/core/Random.hpp"

using namespace rae;

//RAE_TODO: Split this into Engine & ViewportSystem classes:
// Engine just handles all the systems and their updates.
// ViewportSystem handles 3D picking etc... ?

Engine::Engine(GLFWwindow* glfwWindow, NVGcontext* nanoVG) :
	m_screenSystem(glfwWindow),
	m_input(m_screenSystem),
	m_debugSystem(),
	m_assetSystem(m_time),
	m_sceneSystem(m_time, m_input/*, m_assetSystem*/),
	m_uiSystem(m_time, m_input, m_screenSystem, m_assetSystem, m_debugSystem),
	m_rayTracer(m_time, m_assetSystem, m_sceneSystem),
	m_renderSystem(nanoVG, m_time, glfwWindow, m_input, m_screenSystem,
		m_assetSystem, m_uiSystem, m_sceneSystem,
		m_rayTracer),
	m_editorSystem(m_sceneSystem, m_renderSystem, m_assetSystem, m_input/*, m_uiSystem*/)
{
	m_time.initTime(glfwGetTime());

	using std::placeholders::_1;
	m_input.connectMouseButtonPressEventHandler(std::bind(&Engine::onMouseEvent, this, _1));
	m_input.connectKeyEventHandler(std::bind(&Engine::onKeyEvent, this, _1));
}

void Engine::destroyEntity(Id id)
{
	m_destroyEntities.emplace_back(id);
}

void Engine::defragmentTablesAsync()
{
	m_defragmentTables = true;
}

void Engine::addSystem(ISystem& system)
{
	m_systems.push_back(&system);
}

void Engine::addRenderer3D(ISystem& system)
{
	m_renderers3D.push_back(&system);
}

void Engine::addRenderer2D(ISystem& system)
{
	m_renderers2D.push_back(&system);
}

void Engine::start()
{
	m_running = true;
}

void Engine::quit()
{
	m_running = false;
}

void Engine::run()
{
	while (m_running)
	{
		//glfwPollEvents(); // Don't use this here, it's for games. Use it in the inner loop if something is updating.
		// It will take up too much CPU all the time, even when nothing is happening.
		glfwWaitEvents(); //use this instead. It will sleep when no events are being received.

		GLFWwindow* window = m_screenSystem.window(0).windowHandle();

		while (m_running == true && update() == UpdateStatus::Changed)
		{
			// Swap buffers
			glfwSwapBuffers(window);

			glfwPollEvents();

			if (glfwWindowShouldClose(window) != 0)
			{
				m_running = false;
			}

			m_time.setPreviousTime();
		}
	}
}

UpdateStatus Engine::update()
{
	if (!m_sceneSystem.hasActiveScene())
		return UpdateStatus::NotChanged;

	Scene& scene = m_sceneSystem.activeScene();
	auto& entitySystem = scene.entitySystem();

	// Measure speed
	m_time.setTime(glfwGetTime());

	if (!m_destroyEntities.empty())
	{
		/*for (auto system : m_systems)
		{
			system->destroyEntities(m_destroyEntities);
		}*/
		m_sceneSystem.destroyEntities(m_destroyEntities);
		m_destroyEntities.clear();
	}

	if (m_defragmentTables)
	{
		for (auto system : m_systems)
		{
			system->defragmentTables();
		}
		m_defragmentTables = false;
	}

	reactToInput(m_input);

	UpdateStatus engineUpdateStatus = UpdateStatus::NotChanged;

	//LOG_F(INFO, "FRAME START.");

	for (auto system : m_systems)
	{
		if (system->isEnabled())
		{
			UpdateStatus updateStatus = system->update();
			engineUpdateStatus = (updateStatus == UpdateStatus::Changed) ? UpdateStatus::Changed : engineUpdateStatus;
			//LOG_F(INFO, "%s update: %s", system->name(), bool(updateStatus == UpdateStatus::Changed) ? "true" : "false");
		}
	}

	m_renderSystem.beginFrame3D();

	for (int i = 0; i < m_uiSystem.viewportCount(); ++i)
	{
		Rectangle viewport = m_uiSystem.getViewportPixelRectangle(i);

		m_renderSystem.setViewport(viewport);

		if (m_sceneSystem.hasScene(i))
		{
			const Scene& scene = m_sceneSystem.getScene(i);

			for (auto system : m_renderers3D)
			{
				if (system->isEnabled())
				{
					system->render3D(scene);
				}
			}
		}
	}

	m_renderSystem.endFrame3D();

	m_renderSystem.beginFrame2D();
	for (auto system : m_renderers2D)
	{
		if (system->isEnabled())
		{
			system->render2D(m_renderSystem.nanoVG());
		}
	}
	m_renderSystem.endFrame2D();

	//LOG_F(INFO, "FRAME END.");

	for (auto system : m_systems)
	{
		// A potential issue where isEnabled is changed to false earlier in the update,
		// and then onFrameEnd doesn't get called for the system.
		if (system->isEnabled())
		{
			system->onFrameEnd();
		}
	}

	return engineUpdateStatus;
}

void Engine::askForFrameUpdate()
{
	//glfwPostEmptyEvent(); //TODO need to update to GLFW 3.1
}

void Engine::osEventResizeWindow(int width, int height)
{
	// TODO could pass it straight to screenSystem and cameraSystem.
	m_renderSystem.osEventResizeWindow(width, height);
}

void Engine::osEventResizeWindowPixels(int width, int height)
{
	// TODO could pass it straight to screenSystem and cameraSystem.
	m_renderSystem.osEventResizeWindowPixels(width, height);
}

void Engine::osMouseButtonPress(int set_button, float set_xP, float set_yP)
{
	const auto& window = m_screenSystem.window();
	// Have to scale input on retina screens:
	set_xP = set_xP * window.screenPixelRatio();
	set_yP = set_yP * window.screenPixelRatio();

	//LOG_F(INFO, "osMouseButtonPress after screenPixelRatio: %f x: %f y: %f", window.screenPixelRatio(),
	//	set_xP, set_yP);

	float setAmount = 0.0f;
	m_input.osMouseEvent(
		EventType::MouseButtonPress,
		set_button,
		set_xP,
		set_yP,
		setAmount);
}

void Engine::osMouseButtonRelease(int set_button, float set_xP, float set_yP)
{
	const auto& window = m_screenSystem.window();
	// Have to scale input on retina screens:
	set_xP = set_xP * window.screenPixelRatio();
	set_yP = set_yP * window.screenPixelRatio();

	float setAmount = 0.0f;
	m_input.osMouseEvent(
		EventType::MouseButtonRelease,
		set_button,
		set_xP,
		set_yP,
		setAmount);
}

void Engine::osMouseMotion(float set_xP, float set_yP)
{
	const auto& window = m_screenSystem.window();
	// Have to scale input on retina screens:
	set_xP = set_xP * window.screenPixelRatio();
	set_yP = set_yP * window.screenPixelRatio();

	float setAmount = 0.0f;
	m_input.osMouseEvent(
		EventType::MouseMotion,
		(int)MouseButton::Undefined,
		set_xP,
		set_yP,
		setAmount);
}

void Engine::osScrollEvent(float scrollX, float scrollY)
{
	m_input.osScrollEvent(scrollX, scrollY);
}

void Engine::osKeyEvent(int key, int scancode, int action, int mods)
{
	// glfw mods are not handled at the moment
	EventType eventType = EventType::Undefined;
	if (action == GLFW_PRESS)
		eventType = EventType::KeyPress;
	else if (action == GLFW_RELEASE)
		eventType = EventType::KeyRelease;

	m_input.osKeyEvent(eventType, key, (int32_t)scancode);
}

void Engine::onMouseEvent(const Input& input)
{
	if (!m_sceneSystem.hasActiveScene())
		return;

	Scene& scene = m_sceneSystem.activeScene();
	auto& selectionSystem = scene.selectionSystem();

	if (input.eventType == EventType::MouseButtonPress)
	{
		if (input.mouse.eventButton == MouseButton::First)
		{
			const auto& window = m_screenSystem.window();

			//LOG_F(INFO, "mouse press: x: %f y: %f", input.mouse.x, input.mouse.y);
			//LOG_F(INFO, "mouse press: xP: %f yP: %f", (int)m_screenSystem.heightToPixels(input.mouse.x) + (m_renderSystem.windowPixelWidth() / 2),
			//	m_renderSystem.windowPixelHeight() - (int)m_screenSystem.heightToPixels(input.mouse.y) - (m_renderSystem.windowPixelHeight() / 2));

			unsigned char res[4];

			m_renderSystem.renderPicking();

			//glGetIntegerv(GL_VIEWPORT, viewport);
			glReadPixels(
				(int)m_screenSystem.heightToPixels(input.mouse.x) + (window.pixelWidth() / 2),
				window.pixelHeight() - (int)m_screenSystem.heightToPixels(input.mouse.y) - (window.pixelHeight() / 2),
				1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);

			// Decode entity ID from red and green channels!
			int pickedID = res[0] + (res[1] * 256);

			//m_renderSystem.m_pickedString = std::to_string(pickedID) + " is " + std::to_string(res[0]) + " and " + std::to_string(res[1]);

			if (pickedID == 0)
			{
				// Hit the background.
				selectionSystem.clearPixelClicked();
			}
			else if (pickedID == 13)
			{
				scene.createRandomCubeEntity(m_assetSystem);
				scene.createRandomBunnyEntity(m_assetSystem);
			}
			else
			{
				//LOG_F(INFO, "Pixel clicked entity: %i", pickedID);
				selectionSystem.setPixelClicked({ pickedID });
			}
		}
	}
}

void Engine::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KeyPress)
	{
		switch (input.key.value)
		{
			default:
			break;
		}
	}
}

void Engine::reactToInput(const Input& input)
{
}
