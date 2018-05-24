#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "nanovg.h"

#ifdef _WIN32
#define NANOVG_GL3_IMPLEMENTATION
#else
#define NANOVG_GL2_IMPLEMENTATION
#endif

#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/ui/WindowSystem.hpp"
#include "rae/ui/Input.hpp"

#include "loguru/loguru.hpp"

using namespace rae;

WindowSystem* g_windowSystem = nullptr;

int loadFonts(NVGcontext* vg)
{
	int font;
	font = nvgCreateFont(vg, "sans", "./data/fonts/Roboto-Regular.ttf");
	if (font == -1)
	{
		printf("Could not add font regular. File missing: ./data/fonts/Roboto-Regular.ttf\n");
		return -1;
	}

	font = nvgCreateFont(vg, "sans-bold", "./data/fonts/Roboto-Bold.ttf");
	if (font == -1)
	{
		printf("Could not add font bold. File missing: ./data/fonts/Roboto-Bold.ttf\n");
		return -1;
	}

	font = nvgCreateFont(vg, "logo", "./data/fonts/coolvetica_pupu.ttf");
	if (font == -1)
	{
		printf("Could not add font Avenir. File missing: ./data/fonts/coolvetica_pupu.ttf\n");
		return -1;
	}

	return 0;
}

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height)
{
	if (g_windowSystem == nullptr)
		return;
	g_windowSystem->osEventResizeWindow(window, width, height);
}

void glfwWindowPixelSizeCallback(GLFWwindow* window, int width, int height)
{
	if (g_windowSystem == nullptr)
		return;
	g_windowSystem->osEventResizeWindowPixels(window, width, height);
}

void glfwOnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	if (g_windowSystem == nullptr)
		return;

	double mx, my;
	glfwGetCursorPos(window, &mx, &my);

	//LOG_F(INFO, "glfwOnMouseButtonPress. x: %f y: %f", mx, my);
	if (action == GLFW_PRESS)
	{
		g_windowSystem->osMouseButtonPress(window, button, (float)mx, (float)my);
	}
	else if (action == GLFW_RELEASE)
	{
		g_windowSystem->osMouseButtonRelease(window, button, (float)mx, (float)my);
	}
}

void glfwOnMouseMotion(GLFWwindow* window, double set_x, double set_y)
{
	if( g_windowSystem == nullptr )
		return;

	g_windowSystem->osMouseMotion(window, (float)set_x, (float)set_y);
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if( g_windowSystem == nullptr )
		return;

	g_windowSystem->osKeyEvent(window, key, scancode, action, mods);
}

void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if( g_windowSystem == nullptr )
		return;

	g_windowSystem->osScrollEvent(window, (float)xoffset, (float)yoffset);
}

//----------------------------------------------------------------------------------------------------------------------

Window::Window(const String& name, int width, int height) :
	m_name(name),
	m_width(width),
	m_height(height)
{
	// Create window
	if (m_width == -1 || m_height == -1)
	{
		GLFWmonitor* screen     = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(screen);

		if (m_width == -1)
			m_width = mode->width - 100;

		if (m_height == -1)
			m_height = mode->height - 100;
	}

	m_window = glfwCreateWindow(m_width, m_height, name.c_str(), nullptr, nullptr);


	if (m_window == nullptr)
	{
		LOG_F(ERROR, "Failed to open GLFW window.");
		getchar();
		glfwTerminate();
		return;
	}

	activateContext();

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		LOG_F(ERROR, "Failed to initialize GLEW.");
		getchar();
		exit(0);
		return;
	}

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	glfwSwapInterval(0);

	glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetWindowSizeCallback     (m_window, glfwWindowSizeCallback);
	glfwSetFramebufferSizeCallback(m_window, glfwWindowPixelSizeCallback); // Support hi-dpi displays
	glfwSetMouseButtonCallback    (m_window, glfwOnMouseButton);
	glfwSetCursorPosCallback      (m_window, glfwOnMouseMotion);
	glfwSetKeyCallback            (m_window, glfwKeyCallback);
	glfwSetScrollCallback         (m_window, glfwScrollCallback);

	glfwGetWindowSize(m_window, &m_width, &m_height);
	glfwGetFramebufferSize(m_window, &m_pixelWidth, &m_pixelHeight);

	// Init NanoVG
	#ifdef NANOVG_GL3_IMPLEMENTATION
		m_nanoVG = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	#else
		m_nanoVG = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	#endif

	if (m_nanoVG == nullptr)
	{
		LOG_F(ERROR, "Could not init nanovg");
		getchar();
		exit(0);
	}

	if (loadFonts(m_nanoVG) == -1)
	{
		LOG_F(ERROR, "Could not load fonts");
		getchar();
		exit(0);
	}
}

Window::~Window()
{
	if (m_nanoVG)
	{
		#ifdef NANOVG_GL3_IMPLEMENTATION
			nvgDeleteGL3(m_nanoVG);
		#else
			nvgDeleteGL2(m_nanoVG);
		#endif
	}

	if (m_window)
	{
		glfwDestroyWindow(m_window);
	}
}

Window::Window(Window&& other)
{
	m_window = other.m_window;
	other.m_window = nullptr;

	m_nanoVG = other.m_nanoVG;
	other.m_nanoVG = nullptr;

	m_width = other.m_width;
	m_height = other.m_height;
	m_pixelWidth = other.m_pixelWidth;
	m_pixelHeight = other.m_pixelHeight;
	m_screenPixelRatio = other.m_screenPixelRatio;

	m_uiSceneIndex = other.m_uiSceneIndex;
}

void Window::activateContext()
{
	glfwMakeContextCurrent(m_window);
}

void Window::swapBuffers()
{
	glfwSwapBuffers(m_window);
}

void Window::osEventResizeWindow(int width, int height)
{
	m_width = width;
	m_height = height;
	m_screenPixelRatio = (float)m_pixelWidth / (float)m_width;
}

void Window::osEventResizeWindowPixels(int width, int height)
{
	m_pixelWidth = width;
	m_pixelHeight = height;
	m_screenPixelRatio = (float)m_pixelWidth / (float)m_width;
}

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
