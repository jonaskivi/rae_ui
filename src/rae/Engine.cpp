#include "rae/Engine.hpp"

#include <glm/glm.hpp>

#include "loguru/loguru.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/scene/Transform.hpp"
#include "rae/visual/Material.hpp"
#include "rae/core/Random.hpp"
#include "rae/ui/Window.hpp"

using namespace rae;

Engine* g_engine = nullptr;

Engine::Engine() :
	Engine("Rae Application", -1, -1)
{
	g_engine = this;
}

Engine::Engine(
	const String& applicationName,
	int mainWindowWidth,
	int mainWindowHeight,
	bool isFullscreen) :
		m_screenSystem(),
		m_input(m_screenSystem),
		m_windowSystem(m_input, applicationName, mainWindowWidth, mainWindowHeight, isFullscreen),
		m_debugSystem(),
		m_assetSystem(m_time, m_windowSystem.mainWindow().nanoVG()),
		m_sceneSystem(m_time, m_input/*, m_assetSystem*/),
		m_uiSystem(m_windowSystem, m_time, m_input, m_screenSystem, m_assetSystem, m_debugSystem),
		m_rayTracer(m_time, m_windowSystem, m_assetSystem, m_sceneSystem),
		m_renderSystem(m_time, m_input, m_screenSystem,
			m_windowSystem, m_assetSystem, m_uiSystem, m_sceneSystem,
			m_rayTracer)
{
	g_engine = this;
	m_time.initTime(glfwGetTime());
}

Engine::~Engine()
{
	g_engine = nullptr;
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

void Engine::addBaseSystems()
{
	m_systems.push_back(&m_windowSystem);
	m_systems.push_back(&m_input);
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
		//RAE_TEMP: Commented out because of some performance issues on a Macbook Pro.
		// Somehow the glfwWaitEvents slows things down so much that we miss the vsync.
		// Possibly at least text rendering is really slow.
		/*
		//glfwPollEvents(); // Don't use this here, it's for games. Use it in the inner loop if something is updating.
		// It will take up too much CPU all the time, even when nothing is happening.
		//RAE_TEMP: glfwWaitEvents(); // Use this instead. It will sleep when no events are being received.

		while (m_running == true && update() == UpdateStatus::Changed)
		{
			glfwPollEvents();

			// RAE_TODO: Hmm. How does time previous thing here react to window size update()?
			// It probably doesn't work properly with that.
			m_time.setPreviousTime();
		}
		*/

		//RAE_TEMP:
		glfwPollEvents();
		update();
		m_time.setPreviousTime();
	}
}

UpdateStatus Engine::update()
{
	if (!m_sceneSystem.hasActiveScene())
		return UpdateStatus::NotChanged;

	Scene& activeScene = m_sceneSystem.modifyActiveScene();
	auto& entitySystem = activeScene.entitySystem();

	// Measure speed
	m_time.setTime(glfwGetTime());

	if (!m_destroyEntities.empty())
	{
		//for (auto system : m_systems)
		//{
		//	system->destroyEntities(m_destroyEntities);
		//}
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

	if (m_windowSystem.windowCount() == 0)
	{
		quit();
		return UpdateStatus::Changed;
	}

	// Update window sizes to scenes
	{
		for (int i = 0; i < m_windowSystem.windowCount(); ++i)
		{
			const Window& window = m_windowSystem.window(i);

			int uiSceneIndex = window.uiSceneIndex();
			if (m_uiSystem.hasScene(uiSceneIndex))
			{
				UIScene& uiScene = m_uiSystem.scene(uiSceneIndex);
				uiScene.updateWindowSize(window);
			}
		}
	}

	reactToInput(m_input);

	// HandleInput func
	{
		int grabbedSceneIndex = -1;
		for (int i = 0; i < m_uiSystem.sceneCount(); ++i)
		{
			if (m_uiSystem.scene(i).inputState().isGrabbed())
			{
				grabbedSceneIndex = i;
				break;
			}
		}

		for (int i = 0; i < m_windowSystem.windowCount(); ++i)
		{
			auto&& window = m_windowSystem.modifyWindow(i);

			int uiSceneIndex = window.uiSceneIndex();
			if (m_uiSystem.hasScene(uiSceneIndex))
			{
				UIScene& uiScene = m_uiSystem.scene(uiSceneIndex);

				// Handle UI scene input if nothing is grabbed, or if this UI scene is grabbed.
				if (grabbedSceneIndex == -1 || grabbedSceneIndex == uiSceneIndex)
				{
					uiScene.handleInput(window.events());

					int eventsForSceneIndex = uiScene.eventsForSceneIndex();

					if (m_sceneSystem.hasScene(eventsForSceneIndex))
					{
						Scene& scene = m_sceneSystem.scene(eventsForSceneIndex);
						scene.handleInput(uiScene.inputState(), window.events());

						if (scene.checkIfNeedsToBeActiveScene())
						{
							m_sceneSystem.activateScene(eventsForSceneIndex);
							uiScene.activateViewportForSceneIndex(eventsForSceneIndex);
						}
					}
				}

				window.clearEvents();
			}
		}
	}

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
		else
		{
			system->updateWhenDisabled();
		}
	}

	// Render func
	{
		for (int i = 0; i < m_windowSystem.windowCount(); ++i)
		{
			auto&& window = m_windowSystem.modifyWindow(i);
			//LOG_F(INFO, "Window: %s sceneIndex: %i", window.name().c_str(), window.sceneIndex());

			window.activateContext();

			int uiSceneIndex = window.uiSceneIndex();
			//LOG_F(INFO, "window: %i uiSceneIndex: %i", i, uiSceneIndex);
			if (m_uiSystem.hasScene(uiSceneIndex))
			{
				// RAE_TODO possibly const
				UIScene& uiScene = m_uiSystem.scene(uiSceneIndex);

				m_renderSystem.beginFrame3D();

				const auto& viewports = uiScene.viewports();

				// We can only have 1 maximized 3D viewport at a time.
				Id maximizedViewport = InvalidId;
				query<Viewport>(viewports, [&](Id id, const Viewport& viewport)
				{
					const Maximizer& maximizer = uiScene.getMaximizer(id);
					if (maximizedViewport == InvalidId && maximizer.isMaximized())
					{
						maximizedViewport = id;

						// RAE_TODO: make some querys breakable by returning a bool value if we should continue or not.
						// return false;
					}
					// RAE_TODO: continue:
					//return true;
				});

				query<Viewport>(viewports, [&](Id id, const Viewport& viewport)
				{
					// Nothing maximized or this is the maximized viewport, so render this viewport.
					if (maximizedViewport == InvalidId || maximizedViewport == id)
					{
						Rectangle viewportRectangle = uiScene.getViewportPixelRectangle(id);

						m_renderSystem.setViewport(viewportRectangle, window);

						if (m_sceneSystem.hasScene(viewport.sceneIndex))
						{
							// Prepare
							{
								Scene& scene = m_sceneSystem.scene(viewport.sceneIndex);

								for (auto system : m_renderers3D)
								{
									if (system->isEnabled())
									{
										system->prepareRender3D(scene);
									}
								}
							}

							// Render
							{
								const Scene& scene = m_sceneSystem.scene(viewport.sceneIndex);

								for (auto system : m_renderers3D)
								{
									if (system->isEnabled())
									{
										system->render3D(scene, window, m_renderSystem);
									}
								}
							}
						}
					}
				});

				m_renderSystem.endFrame3D();

				m_renderSystem.beginFrame2D(window);

				for (auto system : m_renderers2D)
				{
					if (system->isEnabled())
					{
						system->render2D(uiScene, window.nanoVG());
					}
				}

				m_renderSystem.endFrame2D(window);
			}

			window.swapBuffers();

			//LOG_F(INFO, "FRAME END.");
		}
	}

	for (auto system : m_systems)
	{
		// A potential issue where isEnabled is changed to false earlier in the update,
		// and then onFrameEnd doesn't get called for the system. So we run onFrameEnd even if the system
		// is disabled.
		//if (system->isEnabled())
		{
			system->onFrameEnd();
		}
	}

	return engineUpdateStatus;
}

void Engine::askForFrameUpdate()
{
	glfwPostEmptyEvent();
}

void Engine::osEventResizeWindow(int width, int height)
{
	if (!m_sceneSystem.hasActiveScene())
		return;
	Scene& scene = m_sceneSystem.modifyActiveScene();
	auto& cameraSystem = scene.modifyCameraSystem();
	cameraSystem.setAspectRatio(float(width) / float(height));
}

void Engine::osEventResizeWindowPixels(int width, int height)
{
	if (!m_sceneSystem.hasActiveScene())
		return;
	Scene& scene = m_sceneSystem.modifyActiveScene();
	auto& cameraSystem = scene.modifyCameraSystem();
	cameraSystem.setAspectRatio(float(width) / float(height));
}

void Engine::onMouseEvent(const Input& input)
{
	if (!m_sceneSystem.hasActiveScene())
		return;
/*RAE_TODO RENDERPICKING WINDOW...

	Scene& scene = m_sceneSystem.activeScene();
	auto& selectionSystem = scene.selectionSystem();

	if (input.eventType == EventType::MouseButtonPress)
	{
		if (input.mouse.eventButton == MouseButton::First)
		{
			const auto& window = m_windowSystem.window();

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
	*/
}

void Engine::onKeyEvent(const Input& input)
{
	if (input.eventType == EventType::KeyPress)
	{
		/*
		switch (input.key.value)
		{
			default:
			break;
		}
		*/
	}
}

void Engine::reactToInput(const Input& input)
{
}
