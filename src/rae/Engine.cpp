#include "rae/Engine.hpp"

#include <glm/glm.hpp>

#include "rae/core/Log.hpp"
#include "rae/core/ISystem.hpp"
#include "rae/visual/Mesh.hpp"
#include "rae/visual/Transform.hpp"
#include "rae/visual/Material.hpp"
#include "rae/core/Random.hpp"

using namespace rae;

Engine::Engine(GLFWwindow* set_window) :
	m_window(set_window),
	m_input(m_screenSystem),
	m_cameraSystem(m_entitySystem, m_transformSystem, m_input),
	m_rayTracer(m_cameraSystem),
	m_uiSystem(m_input, m_screenSystem, m_entitySystem, m_transformSystem, m_renderSystem), 
	m_renderSystem(m_entitySystem, m_window, m_input, m_screenSystem, m_cameraSystem,
		m_transformSystem, m_uiSystem, m_rayTracer)
{
	m_currentTime = glfwGetTime();
	m_previousTime = m_currentTime;

	addSystem(m_input);
	addSystem(m_transformSystem);
	addSystem(m_cameraSystem);
	addSystem(m_uiSystem);
	addSystem(m_rayTracer);
	addSystem(m_renderSystem);

	// RAE_TODO need to get rid of OpenGL picking hack button at id 0.
	Id emptyEntityId = m_entitySystem.createEntity(); // hack at index 0
	rae_log("Create empty hack entity at 0: ", emptyEntityId);

	// Load model
	Id meshID = m_renderSystem.createMesh("./data/models/bunny.obj");
	m_modelID = meshID;

	m_meshID     = m_renderSystem.createBox();
	m_materialID = m_renderSystem.createMaterial(Colour(0.2f, 0.5f, 0.7f, 0.0f));
	m_bunnyMaterialID = m_renderSystem.createMaterial(Colour(0.7f, 0.3f, 0.1f, 0.0f));
	m_buttonMaterialID = m_renderSystem.createAnimatingMaterial(Colour(0.0f, 0.0f, 0.1f, 0.0f));

	createTestWorld2();

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

void Engine::addSystem(ISystem& ownSystem)
{
	m_systems.push_back(&ownSystem);
}

void Engine::run()
{
	do {
		//glfwPollEvents(); // Don't use this here, it's for games. Use it in the inner loop if something is updating.
		// It will take up too much CPU all the time, even when nothing is happening.
		glfwWaitEvents(); //use this instead. It will sleep when no events are being received.

		while (m_running == true && update() == true)
		{
			// Swap buffers
			glfwSwapBuffers(m_window);

			glfwPollEvents();

			if (glfwWindowShouldClose(m_window) != 0)
				m_running = false;

			m_previousTime = m_currentTime;
		}

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS
		   && glfwWindowShouldClose(m_window) == 0);
}

bool Engine::update()
{
	// Measure speed
	m_currentTime = glfwGetTime();
	double deltaTime = m_currentTime - m_previousTime;

	if (!m_destroyEntities.empty())
	{
		for (auto system : m_systems)
		{
			system->destroyEntities(m_destroyEntities);
		}
		m_entitySystem.destroyEntities(m_destroyEntities);
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

	bool changed = false;

	//rae_log("FRAME START.");

	for (auto system : m_systems)
	{
		if (system->isEnabled())
		{
			bool systemChanged = system->update(m_currentTime, deltaTime);
			changed = systemChanged ? true : changed;
			//rae_log(system->name(), " update: ", systemChanged);
		}
	}

	//rae_log("FRAME END.");

	for (auto system : m_systems)
	{
		// A potential issue where isEnabled is changed to false earlier in the update,
		// and then onFrameEnd doesn't get called for the system.
		if (system->isEnabled())
		{
			system->onFrameEnd();
		}
	}

	return changed;
}

void Engine::askForFrameUpdate()
{
	//glfwPostEmptyEvent(); //TODO need to update to GLFW 3.1
}

Id Engine::createAddObjectButton()
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createAddObjectButton id: ", id);
	m_transformSystem.addTransform(id, Transform(vec3(0.0f, 0.0f, 5.0f)));
	m_transformSystem.setPosition(id, vec3(0.0f, 0.0f, 0.0f));

	m_renderSystem.addMaterialLink(id, m_buttonMaterialID);
	m_renderSystem.addMeshLink(id, m_meshID);

	return id;
}

Id Engine::createRandomBunnyEntity()
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createRandomBunnyEntity id: ", id);
	m_transformSystem.addTransform(id, Transform(vec3(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f))));

	m_renderSystem.addMaterialLink(id, m_bunnyMaterialID);
	m_renderSystem.addMeshLink(id, m_modelID);

	return id;
}

Id Engine::createRandomCubeEntity()
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createRandomCubeEntity id: ", id);
	m_transformSystem.addTransform(id, Transform(vec3(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f))));

	m_renderSystem.addMaterialLink(id, m_materialID);
	m_renderSystem.addMeshLink(id, m_meshID);

	return id;
}

Id Engine::createCube(const vec3& position, const Colour& color)
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createCube id: ", id);
	// The desired API:
	m_transformSystem.addTransform(id, Transform(position));
	//m_geometrySystem.setMesh(entity, m_meshID);
	//m_materialSystem.setMaterial(entity, color);

	m_renderSystem.addMaterial(id, Material(color));
	m_renderSystem.addMeshLink(id, m_meshID);

	return id;
}

Id Engine::createBunny(const vec3& position, const Colour& color)
{
	Id id = m_entitySystem.createEntity();
	//rae_log("createBunny id: ", id);
	m_transformSystem.addTransform(id, Transform(position));

	m_renderSystem.addMaterialLink(id, m_bunnyMaterialID);
	m_renderSystem.addMeshLink(id, m_modelID);

	return id;
}

void Engine::createTestWorld()
{
	createAddObjectButton(); // at index 1
}

void Engine::createTestWorld2()
{
	//rae_log("createTestWorld2);

	//createAddObjectButton(); // at index 1

	auto cube0 = createCube(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(0.8f, 0.3f, 0.3f, 0.0f));
	
	auto cube1 = createCube(glm::vec3(1.0f, 0.0f, -1.0f), glm::vec4(0.8f, 0.6f, 0.2f, 0.0f));
	auto cube2 = createCube(glm::vec3(-0.5f, 0.65f, -1.0f), glm::vec4(0.8f, 0.4f, 0.8f, 0.0f));
	auto cube3 = createCube(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec4(0.8f, 0.5f, 0.3f, 0.0f));
	auto cube4 = createCube(glm::vec3(-3.15f, 0.1f, -5.0f), glm::vec4(0.05f, 0.2f, 0.8f, 0.0f));

	auto bunny1 = createBunny(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.05f, 0.2f, 0.8f, 0.0f));

	/*Hierarchy& hierarchy1 = m_entitySystem.createHierarchy();
	cube1.addComponent( (int)ComponentType::HIERARCHY, hierarchy1.id() );
	hierarchy1.addChild(cube2.id());

	Hierarchy& hierarchy2 = m_entitySystem.createHierarchy();
	cube2.addComponent( (int)ComponentType::HIERARCHY, hierarchy2.id() );
	hierarchy2.setParent(cube1.id());
	hierarchy2.addChild(cube3.id());

	Hierarchy& hierarchy3 = m_entitySystem.createHierarchy();
	cube3.addComponent( (int)ComponentType::HIERARCHY, hierarchy3.id() );
	hierarchy3.setParent(cube2.id());
	*/
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

	//rae_log("osMouseButtonPress after screenPixelRatio: ", window.screenPixelRatio(),
	//	" x: ", set_xP, " y: ", set_yP);

	float setAmount = 0.0f;
	m_input.osMouseEvent(
		EventType::MouseButtonPress,
		set_button,
		set_xP - (window.pixelWidth() * 0.5f),
		set_yP - (window.pixelHeight() * 0.5f),
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
		set_xP - (window.pixelWidth() * 0.5f),
		set_yP - (window.pixelHeight() * 0.5f),
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
		set_xP - (window.pixelWidth() * 0.5f),
		set_yP - (window.pixelHeight() * 0.5f),
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
	if (input.eventType == EventType::MouseButtonPress)
	{
		if (input.mouse.eventButton == MouseButton::First)
		{
			const auto& window = m_screenSystem.window();

			//cout << "mouse press: x: "<< input.mouse.x << " y: " << input.mouse.y << endl;
			//cout << "mouse press: xP: "<< (int)m_screenSystem.heightToPixels(input.mouse.x) + (m_renderSystem.windowPixelWidth() / 2)
			//	<< " yP: " << m_renderSystem.windowPixelHeight() - (int)m_screenSystem.heightToPixels(input.mouse.y) - (m_renderSystem.windowPixelHeight() / 2) << endl;
	
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
				// do nothing, it's the background.
			}
			else if (pickedID == 13)
			{
				createRandomCubeEntity();
				createRandomBunnyEntity();
			}
			else
			{
				rae_log("Picked entity: ", pickedID);
				destroyEntity(pickedID);
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
			case KeySym::Escape: m_running = false; break;
			case KeySym::R: m_renderSystem.clearImageRenderer(); break;
			case KeySym::G:
				m_renderSystem.toggleGlRenderer(); // more like debug view currently
				m_rayTracer.toggleIsEnabled();
				break;
			case KeySym::Tab: m_rayTracer.toggleInfoText(); break;
			case KeySym::Y: m_rayTracer.toggleBufferQuality(); break;
			case KeySym::U: m_rayTracer.toggleFastMode(); break;
			case KeySym::H: m_rayTracer.toggleVisualizeFocusDistance(); break;
			case KeySym::_1: m_rayTracer.showScene(1); break;
			case KeySym::_2: m_rayTracer.showScene(2); break;
			case KeySym::_3: m_rayTracer.showScene(3); break;
			default:
			break;
		}
	}
}

void Engine::reactToInput(const Input& input)
{
	if (input.getKeyState(KeySym::I))
	{
		createRandomCubeEntity();
		createRandomBunnyEntity();
	}

	if (input.getKeyState(KeySym::O))
	{
		rae_log("Destroy biggestId: ", m_entitySystem.biggestId());
		destroyEntity((Id)getRandomInt(20, m_entitySystem.biggestId()));
	}

	if (input.getKeyState(KeySym::P))
	{
		defragmentTablesAsync();
	}

	// TODO use KeySym::Page_Up
	if (input.getKeyState(KeySym::K)) { m_rayTracer.minusBounces(); }
	if (input.getKeyState(KeySym::L)) { m_rayTracer.plusBounces(); }
}
