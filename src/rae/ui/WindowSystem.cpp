#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <memory>
#include "rae/core/make_unique.hpp"

#include "rae/ui/WindowSystem.hpp"
#include "rae/ui/Input.hpp"

#include "loguru/loguru.hpp"

using namespace rae;

WindowSystem* g_windowSystem = nullptr;

//----------------------------------------------------------------------------------------------------------------------

void WindowSystem::osEventResizeWindow(GLFWwindow* windowHandle, int width, int height)
{
	windowPtr(windowHandle)->osEventResizeWindow(width, height);
}

void WindowSystem::osEventResizeWindowPixels(GLFWwindow* windowHandle, int width, int height)
{
	windowPtr(windowHandle)->osEventResizeWindowPixels(width, height);
}

void WindowSystem::osEventCursorEnter(GLFWwindow* windowHandle)
{
	auto* window2 = windowPtr(windowHandle);
	window2->osMouseEvent(EventType::MouseEnter);
}

void WindowSystem::osEventCursorLeave(GLFWwindow* windowHandle)
{
	auto* window2 = windowPtr(windowHandle);
	window2->osMouseEvent(EventType::MouseLeave);
}

void WindowSystem::osMouseButtonPress(GLFWwindow* windowHandle, int button, float xP, float yP)
{
	auto* window2 = windowPtr(windowHandle);

	window2->osMouseEvent(
		EventType::MouseButtonPress,
		button,
		xP,
		yP);

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
	auto* window2 = windowPtr(windowHandle);

	window2->osMouseEvent(
		EventType::MouseButtonRelease,
		button,
		xP,
		yP);

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
	auto* window2 = windowPtr(windowHandle);

	window2->osMouseEvent(
		EventType::MouseMotion,
		(int)MouseButton::Undefined,
		xP,
		yP);

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
	auto* window2 = windowPtr(windowHandle);
	m_input.osScrollEvent(*window2, scrollX, scrollY);
}

void WindowSystem::osKeyEvent(GLFWwindow* windowHandle, int key, int scancode, int action, int mods)
{
	auto* window2 = windowPtr(windowHandle);

	// glfw mods are not handled at the moment
	EventType eventType = EventType::Undefined;
	if (action == GLFW_PRESS)
		eventType = EventType::KeyPress;
	else if (action == GLFW_RELEASE)
		eventType = EventType::KeyRelease;

	m_input.osKeyEvent(*window2, eventType, key, (int32_t)scancode);
}

WindowSystem::WindowSystem(
	Input& input,
	const String& mainWindowName,
	int mainWindowWidth,
	int mainWindowHeight,
	bool isFullscreen) :
		ISystem("WindowSystem"),
		m_input(input)
{
	g_windowSystem = this;

	initOnce();

	createWindow(mainWindowName, mainWindowWidth, mainWindowHeight, isFullscreen);
}

WindowSystem::WindowSystem(GLFWwindow* windowHandle, Input& input) :
	m_input(input)
{
	g_windowSystem = this;

	initOnce();

	m_windows.emplace_back(std::make_unique<Window>(windowHandle));
}

WindowSystem::~WindowSystem()
{
	g_windowSystem = nullptr;
}

UpdateStatus WindowSystem::update()
{
	for (auto&& window : m_windows)
	{
		window->update();
	}

	return UpdateStatus::NotChanged;
}

void WindowSystem::onFrameEnd()
{
	ISystem::onFrameEnd();

	int i = 0;
	Array<int> shouldClose;
	for (auto&& window : m_windows)
	{
		if (window->isOpen() == false)
		{
			shouldClose.emplace_back(i);
		}
		++i;
	}

	// Currently UIScene is not automatically destroyed when the window closes. You could still create
	// another window and start showing an existing UIScene in that window...
	// But it will get destroyed when the engine quits.
	for (int idx : shouldClose)
	{
		m_windows.erase(m_windows.begin() + idx);
	}
}

Window* WindowSystem::windowPtr(GLFWwindow* windowHandle)
{
	for (auto&& window : m_windows)
	{
		if (window->windowHandle() == windowHandle)
			return window.get();
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

Window& WindowSystem::createWindow(const String& title, int width, int height, bool isFullscreen)
{
	LOG_F(INFO, "Creating window: %s", title.c_str());

	m_windows.emplace_back(std::make_unique<Window>(title, width, height, isFullscreen));
	return *m_windows.back();
}
