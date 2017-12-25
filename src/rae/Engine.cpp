#include "Engine.hpp"

#include <glm/glm.hpp>

#include "rae/core/ISystem.hpp"
#include "Entity.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"
#include "Material.hpp"
#include "ComponentType.hpp"
#include "Random.hpp"

namespace rae
{

Engine::Engine(GLFWwindow* set_window)
: m_window(set_window),
m_input(m_screenSystem),
m_cameraSystem(m_input),
m_rayTracer(m_cameraSystem),
m_uiSystem(m_input, m_screenSystem, m_objectFactory, m_transformSystem, m_renderSystem), 
m_renderSystem(m_objectFactory, m_window, m_input, m_cameraSystem,
			   m_transformSystem, m_uiSystem, m_rayTracer)
{
	m_currentTime = glfwGetTime();
	m_previousTime = m_currentTime;

	// TODO: now we only have 1 system... :)
	//m_inputSystem = new InputSystem(window, &m_objectFactory);
	//m_systems.push_back(g_input);

	addSystem(m_input);
	addSystem(m_transformSystem);
	addSystem(m_cameraSystem);
	addSystem(m_uiSystem);
	addSystem(m_rayTracer);
	addSystem(m_renderSystem);

	// Load model
	Mesh& mesh = m_objectFactory.createMesh();
	mesh.loadModel("./data/models/bunny.obj");
	m_modelID = mesh.id();

	m_meshID     = m_renderSystem.createBox().id();
	m_materialID = m_renderSystem.createMaterial(0, glm::vec4(0.2f, 0.5f, 0.7f, 0.0f)).id();
	m_bunnyMaterialID = m_renderSystem.createMaterial(1, glm::vec4(0.7f, 0.3f, 0.1f, 0.0f)).id();
	m_buttonMaterialID = m_renderSystem.createAnimatingMaterial(2, glm::vec4(0.0f, 0.0f, 0.1f, 0.0f)).id();
	
	createEmptyEntity(); // hack at index 0

	createTestWorld2();

	/*
	for(unsigned i = 0; i < 50; ++i)
	{
		Entity& entity = createEmptyEntity();
		//entity.addComponent( (int)ComponentType::TRANSFORM, m_objectFactory.createTransform(float(i) - 25.0f, float(i % 10) - 5.0f, 10.0f + (i % 10)).id() );
		entity.addComponent( (int)ComponentType::TRANSFORM, m_objectFactory.createTransform(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f)).id() );
		if(i % 2 == 1)
			entity.addComponent( (int)ComponentType::MATERIAL, m_materialID );
		entity.addComponent( (int)ComponentType::MESH, m_meshID );
	}
	*/

	using std::placeholders::_1;
	m_input.connectMouseButtonPressEventHandler(std::bind(&Engine::onMouseEvent, this, _1));
	m_input.connectKeyEventHandler(std::bind(&Engine::onKeyEvent, this, _1));
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

			////JONDE REMOVE m_input.clearFrame();
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

	reactToInput(m_input);

	bool changed = false;

	//std::cout << "FRAME START.\n";

	for (auto system : m_systems)
	{
		if (system->isEnabled())
		{
			bool systemChanged = system->update(m_currentTime, deltaTime, m_objectFactory.entities());
			changed = systemChanged ? true : changed;
			//std::cout << system->name() << " update: " << systemChanged << "\n";
		}
	}

	//std::cout << "FRAME END.\n";

	for (auto system : m_systems)
	{
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

Entity& Engine::createAddObjectButton()
{
	Entity& entity = createEmptyEntity();
	m_transformSystem.addTransform(entity.id(), Transform(vec3(0.0f, 0.0f, 5.0f)));
	m_transformSystem.setPosition(entity.id(), vec3(0.0f, 0.0f, 0.0f));

	//JONDE REMOVE Transform& transform = m_objectFactory.createTransform(0.0f, 0.0f, 5.0f);
	//JONDE REMOVE transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
	//JONDE REMOVE entity.addComponent( (int)ComponentType::TRANSFORM, transform.id() );
	entity.addComponent( (int)ComponentType::MATERIAL, m_buttonMaterialID );
	entity.addComponent( (int)ComponentType::MESH, m_meshID );
	return entity;
}

Entity& Engine::createRandomBunnyEntity()
{
	Entity& entity = createEmptyEntity();
	m_transformSystem.addTransform(entity.id(), Transform(vec3(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f))));
	//JONDE REMOVE entity.addComponent( (int)ComponentType::TRANSFORM, m_objectFactory.createTransform(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f)).id() );
	entity.addComponent( (int)ComponentType::MATERIAL, m_bunnyMaterialID );
	entity.addComponent( (int)ComponentType::MESH, m_modelID );
	
	return entity;
}

Entity& Engine::createRandomCubeEntity()
{
	Entity& entity = createEmptyEntity();
	m_transformSystem.addTransform(entity.id(), Transform(vec3(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f))));
	//JONDE REMOVE entity.addComponent( (int)ComponentType::TRANSFORM, m_objectFactory.createTransform(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f)).id() );
	entity.addComponent( (int)ComponentType::MATERIAL, m_materialID );
	entity.addComponent( (int)ComponentType::MESH, m_meshID );
	return entity;
}

Entity& Engine::createCube(glm::vec3 position, glm::vec4 color)
{
	Entity& entity = createEmptyEntity();
	// The desired API:
	m_transformSystem.addTransform(entity.id(), Transform(position));
	//m_geometrySystem.setMesh(entity, m_meshID);
	//m_materialSystem.setMaterial(entity, color);

	// The old API:
	//JONDE REMOVE entity.addComponent( (int)ComponentType::TRANSFORM, m_objectFactory.createTransform(position).id() );
	entity.addComponent( (int)ComponentType::MATERIAL, m_renderSystem.createMaterial(0, color).id() );
	entity.addComponent( (int)ComponentType::MESH, m_meshID );
	return entity;
}

Entity& Engine::createBunny(glm::vec3 position, glm::vec4 color)
{
	Entity& entity = createEmptyEntity();
	m_transformSystem.addTransform(entity.id(), Transform(position));
	//JONDE REMOVE entity.addComponent( (int)ComponentType::TRANSFORM, m_objectFactory.createTransform(position).id() );
	entity.addComponent( (int)ComponentType::MATERIAL, m_bunnyMaterialID );
	entity.addComponent( (int)ComponentType::MESH, m_modelID );
	return entity;
}

Entity& Engine::createEmptyEntity()
{
	return m_objectFactory.createEmptyEntity();
}

void Engine::createTestWorld()
{
	createAddObjectButton(); // at index 1
}

void Engine::createTestWorld2()
{
	//createAddObjectButton(); // at index 1

	auto cube0 = createCube(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec4(0.8f, 0.3f, 0.3f, 0.0f));
	
	auto cube1 = createCube(glm::vec3(1.0f, 0.0f, -1.0f), glm::vec4(0.8f, 0.6f, 0.2f, 0.0f));
	auto cube2 = createCube(glm::vec3(-0.5f, 0.65f, -1.0f), glm::vec4(0.8f, 0.4f, 0.8f, 0.0f));
	auto cube3 = createCube(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec4(0.8f, 0.5f, 0.3f, 0.0f));
	auto cube4 = createCube(glm::vec3(-3.15f, 0.1f, -5.0f), glm::vec4(0.05f, 0.2f, 0.8f, 0.0f));

	auto bunny1 = createBunny(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.05f, 0.2f, 0.8f, 0.0f));

	Hierarchy& hierarchy1 = m_objectFactory.createHierarchy();
	cube1.addComponent( (int)ComponentType::HIERARCHY, hierarchy1.id() );
	hierarchy1.addChild(cube2.id());

	Hierarchy& hierarchy2 = m_objectFactory.createHierarchy();
	cube2.addComponent( (int)ComponentType::HIERARCHY, hierarchy2.id() );
	hierarchy2.setParent(cube1.id());
	hierarchy2.addChild(cube3.id());

	Hierarchy& hierarchy3 = m_objectFactory.createHierarchy();
	cube3.addComponent( (int)ComponentType::HIERARCHY, hierarchy3.id() );
	hierarchy3.setParent(cube2.id());
}

void Engine::osEventResizeWindow(int width, int height)
{
	m_renderSystem.osEventResizeWindow(width, height);
}

void Engine::osEventResizeWindowPixels(int width, int height)
{
	m_renderSystem.osEventResizeWindowPixels(width, height);
}

void Engine::osMouseButtonPress(int set_button, float set_xP, float set_yP)
{
	// Have to scale input on retina screens:
	set_xP = set_xP * m_renderSystem.screenPixelRatio();
	set_yP = set_yP * m_renderSystem.screenPixelRatio();

	m_input.osMouseEvent(
		EventType::MouseButtonPress,
		set_button,
		set_xP - (m_renderSystem.windowPixelWidth()*0.5f),
		set_yP - (m_renderSystem.windowPixelHeight()*0.5f),
		/*set_amount*/0.0f );
}

void Engine::osMouseButtonRelease(int set_button, float set_xP, float set_yP)
{
	// Have to scale input on retina screens:
	set_xP = set_xP * m_renderSystem.screenPixelRatio();
	set_yP = set_yP * m_renderSystem.screenPixelRatio();

	m_input.osMouseEvent(
		EventType::MouseButtonRelease,
		set_button,
		set_xP - (m_renderSystem.windowPixelWidth()*0.5f),
		set_yP - (m_renderSystem.windowPixelHeight()*0.5f),
		/*set_amount*/0.0f );
}

void Engine::osMouseMotion(float set_xP, float set_yP)
{
	// Have to scale input on retina screens:
	set_xP = set_xP * m_renderSystem.screenPixelRatio();
	set_yP = set_yP * m_renderSystem.screenPixelRatio();

	m_input.osMouseEvent(
		EventType::MouseMotion,
		(int)MouseButton::Undefined,
		set_xP - (m_renderSystem.windowPixelWidth()*0.5f),
		set_yP - (m_renderSystem.windowPixelHeight()*0.5f),
		/*set_amount*/0.0f );
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
			//cout << "mouse press: x: "<< input.mouse.x << " y: " << input.mouse.y << endl;
			//cout << "mouse press: xP: "<< (int)m_screenSystem.heightToPixels(input.mouse.x) + (m_renderSystem.windowPixelWidth() / 2)
			//	<< " yP: " << m_renderSystem.windowPixelHeight() - (int)m_screenSystem.heightToPixels(input.mouse.y) - (m_renderSystem.windowPixelHeight() / 2) << endl;
	
			unsigned char res[4];

			m_renderSystem.renderPicking( m_objectFactory.entities() );

			//glGetIntegerv(GL_VIEWPORT, viewport);
			glReadPixels(
				(int)m_screenSystem.heightToPixels(input.mouse.x) + (m_renderSystem.windowPixelWidth() / 2),
				m_renderSystem.windowPixelHeight() - (int)m_screenSystem.heightToPixels(input.mouse.y) - (m_renderSystem.windowPixelHeight() / 2),
				1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);

			// Decode entity ID from red and green channels!
			int pickedID = res[0] + (res[1] * 256);

			//m_renderSystem.m_pickedString = std::to_string(pickedID) + " is " + std::to_string(res[0]) + " and " + std::to_string(res[1]);

			if( pickedID == 0)
			{
				// do nothing, it's the background.
			}
			else if( pickedID == 1)
			{
				createRandomCubeEntity(); // Hmm, we have no proper event handling yet, so entity 1 is the Add Object button!
				createRandomBunnyEntity();
			}
			else
			{
				m_objectFactory.destroyEntity( pickedID );
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
			case KeySym::P: m_objectFactory.measure(); break; // JONDE TEMP measure
			case KeySym::R: m_renderSystem.clearImageRenderer(); break;
			case KeySym::G:
				m_renderSystem.toggleGlRenderer(); // more like debug view currently
				m_rayTracer.toggleIsEnabled();
				break;
			case KeySym::T: m_rayTracer.toggleInfoText(); break;
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
		m_objectFactory.destroyEntity(getRandomInt(2, m_objectFactory.entityCount() )); // Hmm. Magic number 2 is the first index with created box entities.
	}

	// TODO use KeySym::Page_Up
	if (input.getKeyState(KeySym::K)) { m_rayTracer.minusBounces(); }
	if (input.getKeyState(KeySym::L)) { m_rayTracer.plusBounces(); }
}

} // end namespace rae

