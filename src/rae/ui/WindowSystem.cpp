#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "rae/ui/WindowSystem.hpp"
#include "rae/ui/Input.hpp"

#include "loguru/loguru.hpp"

using namespace rae;

WindowSystem* g_windowSystem = nullptr;

//----------------------------------------------------------------------------------------------------------------------

void WindowSystem::osEventResizeWindow(GLFWwindow* windowHandle, int width, int height)
{
	window(windowHandle)->osEventResizeWindow(width, height);
}

void WindowSystem::osEventResizeWindowPixels(GLFWwindow* windowHandle, int width, int height)
{
	window(windowHandle)->osEventResizeWindowPixels(width, height);
}

void WindowSystem::osMouseButtonPress(GLFWwindow* windowHandle, int button, float xP, float yP)
{
	auto* window2 = window(windowHandle);
	// Have to scale input on retina screens:
	xP = xP * window2->screenPixelRatio();
	yP = yP * window2->screenPixelRatio();

	//LOG_F(INFO, "osMouseButtonPress after screenPixelRatio: %f x: %f y: %f", window.screenPixelRatio(),
	//	xP, yP);

	float setAmount = 0.0f;
	m_input.osMouseEvent(
		*window2,
		EventType::MouseButtonPress,
		button,
		xP,
		yP,
		setAmount);
}

void WindowSystem::osMouseButtonRelease(GLFWwindow* windowHandle, int button, float xP, float yP)
{
	auto* window2 = window(windowHandle);
	// Have to scale input on retina screens:
	xP = xP * window2->screenPixelRatio();
	yP = yP * window2->screenPixelRatio();

	float setAmount = 0.0f;
	m_input.osMouseEvent(
		*window2,
		EventType::MouseButtonRelease,
		button,
		xP,
		yP,
		setAmount);
}

void WindowSystem::osMouseMotion(GLFWwindow* windowHandle, float xP, float yP)
{
	auto* window2 = window(windowHandle);
	// Have to scale input on retina screens:
	xP = xP * window2->screenPixelRatio();
	yP = yP * window2->screenPixelRatio();

	float setAmount = 0.0f;
	m_input.osMouseEvent(
		*window2,
		EventType::MouseMotion,
		(int)MouseButton::Undefined,
		xP,
		yP,
		setAmount);
}

void WindowSystem::osScrollEvent(GLFWwindow* windowHandle, float scrollX, float scrollY)
{
	auto* window2 = window(windowHandle);
	m_input.osScrollEvent(*window2, scrollX, scrollY);
}

void WindowSystem::osKeyEvent(GLFWwindow* windowHandle, int key, int scancode, int action, int mods)
{
	auto* window2 = window(windowHandle);

	// glfw mods are not handled at the moment
	EventType eventType = EventType::Undefined;
	if (action == GLFW_PRESS)
		eventType = EventType::KeyPress;
	else if (action == GLFW_RELEASE)
		eventType = EventType::KeyRelease;

	m_input.osKeyEvent(*window2, eventType, key, (int32_t)scancode);
}

WindowSystem::WindowSystem(Input& input, const String& mainWindowName, int mainWindowWidth, int mainWindowHeight) :
	m_input(input)
{
	g_windowSystem = this;

	initOnce();

	createWindow(mainWindowName, mainWindowWidth, mainWindowHeight);
}

WindowSystem::WindowSystem(GLFWwindow* windowHandle, Input& input) :
	m_input(input)
{
	g_windowSystem = this;

	initOnce();

	m_windows.emplace_back(windowHandle);
}

WindowSystem::~WindowSystem()
{
	g_windowSystem = nullptr;
}

UpdateStatus WindowSystem::update()
{
	for (auto&& window : m_windows)
	{
		window.update();
	}

	return UpdateStatus::NotChanged;
}

Window* WindowSystem::window(GLFWwindow* windowHandle)
{
	for (auto&& window : m_windows)
	{
		if (window.windowHandle() == windowHandle)
			return &window;
	}
	return nullptr;
}

void WindowSystem::initOnce()
{
	static bool once = false;
	if (once)
		return;

	// Enable MSAA (multisample anti-aliasing)
	glfwWindowHint(GLFW_SAMPLES, 4);

	#ifndef _WIN32 // don't require this on win32, and works with more cards
	// Set OpenGL version to 2.1
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	#endif

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	once = true;
}

Window& WindowSystem::createWindow(const String& title, int width, int height)
{
	LOG_F(INFO, "Creating window: %s", title.c_str());

	//auto* window = new Window(title, width, height);
	//m_windows.push_back(window);
	//return m_windows[0]->windowHandle();

	m_windows.emplace_back(title, width, height);
	return m_windows.back();
}