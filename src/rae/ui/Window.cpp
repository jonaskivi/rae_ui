#include "rae/ui/Window.hpp"

#include "loguru/loguru.hpp"

#include "nanovg.h"

#ifdef _WIN32
#define NANOVG_GL3_IMPLEMENTATION
#elif __linux__
#define NANOVG_GL3_IMPLEMENTATION
#else
#define NANOVG_GL2_IMPLEMENTATION
#endif

#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/ui/WindowSystem.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/Engine.hpp"

using namespace rae;

extern WindowSystem* g_windowSystem;
extern Engine* g_engine;

int loadFonts(NVGcontext* vg)
{
	int font;
	font = nvgCreateFont(vg, "sans", "./data/fonts/Roboto-Regular.ttf");
	if (font == -1)
	{
		LOG_F(ERROR, "Could not add font regular. File missing: ./data/fonts/Roboto-Regular.ttf");
		return -1;
	}

	font = nvgCreateFont(vg, "sans-bold", "./data/fonts/Roboto-Bold.ttf");
	if (font == -1)
	{
		LOG_F(ERROR, "Could not add font bold. File missing: ./data/fonts/Roboto-Bold.ttf\n");
		return -1;
	}

	font = nvgCreateFont(vg, "logo", "./data/fonts/coolvetica_pupu.ttf");
	if (font == -1)
	{
		LOG_F(ERROR, "Could not add font Avenir. File missing: ./data/fonts/coolvetica_pupu.ttf\n");
		return -1;
	}

	return 0;
}

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height)
{
	if (g_windowSystem == nullptr)
		return;
	g_windowSystem->osEventResizeWindow(window, width, height);
	if (g_engine != nullptr)
		g_engine->update();
}

void glfwWindowPixelSizeCallback(GLFWwindow* window, int width, int height)
{
	if (g_windowSystem == nullptr)
		return;
	g_windowSystem->osEventResizeWindowPixels(window, width, height);
}

void glfwCursorEnterCallback(GLFWwindow* window, int entered)
{
	if (g_windowSystem == nullptr)
		return;
	if (entered)
		g_windowSystem->osEventCursorEnter(window);
	else g_windowSystem->osEventCursorLeave(window);
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

Window::Window(const String& name, int width, int height, bool isFullscreen)
{
	create(name, width, height, isFullscreen);
}

void Window::create(const String& name, int width, int height, bool isFullscreen)
{
	if (m_windowHandle)
	{
		destroy();
	}

	m_isFullscreen = isFullscreen;
	m_name = name;
	m_width = width;
	m_height = height;

	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

	if (m_width == -1 || m_height == -1)
	{
		if (m_width == -1)
			m_width = mode->width - 100;

		if (m_height == -1)
			m_height = mode->height - 100;
	}

	if (m_isFullscreen)
	{
		m_widthBackup = m_width;
		m_heightBackup = m_height;

		m_windowHandle = glfwCreateWindow(
			mode->width, mode->height, name.c_str(), primaryMonitor, nullptr);
	}
	else
	{
		m_windowHandle = glfwCreateWindow(
			m_width, m_height, name.c_str(), nullptr, nullptr);
	}

	if (m_windowHandle == nullptr)
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
		glfwTerminate();
		return;
	}

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	glfwSwapInterval(0);

	glfwSetInputMode(m_windowHandle, GLFW_STICKY_KEYS, GL_TRUE);

	// This could be used to attach our own pointer to the window. This way
	// we could possibly do away with the global g_windowSystem variable, and use
	// this window class pointer directly when receiving callbacks.
	//glfwSetWindowUserPointer(m_windowHandle, this);
	// And in the callback:
	//void* ptr = glfwGetWindowUserPointer(windowHandle);
	//if (Window* window = static_cast<Window*>(ptr))
	//	window->resize(x, y);

	glfwSetWindowSizeCallback     (m_windowHandle, glfwWindowSizeCallback);
	glfwSetFramebufferSizeCallback(m_windowHandle, glfwWindowPixelSizeCallback); // Support hi-dpi displays
	glfwSetCursorEnterCallback    (m_windowHandle, glfwCursorEnterCallback);
	glfwSetMouseButtonCallback    (m_windowHandle, glfwOnMouseButton);
	glfwSetCursorPosCallback      (m_windowHandle, glfwOnMouseMotion);
	glfwSetKeyCallback            (m_windowHandle, glfwKeyCallback);
	glfwSetScrollCallback         (m_windowHandle, glfwScrollCallback);

	// Initialize pixel sizes
	glfwGetWindowSize(m_windowHandle, &m_width, &m_height);
	glfwGetFramebufferSize(m_windowHandle, &m_pixelWidth, &m_pixelHeight);
	m_screenPixelRatio = (float)m_pixelWidth / (float)m_width;

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
		glfwTerminate();
	}

	if (loadFonts(m_nanoVG) == -1)
	{
		LOG_F(ERROR, "Could not load fonts");
		getchar();
		glfwTerminate();
	}
}

Window::~Window()
{
	destroy();
}

void Window::destroy()
{
	if (m_windowHandle)
	{
		// Need to activate the right context before destroying it too.
		// Otherwise we might end up destroying the wrong nanoVG context and
		// nothing gets drawn on our second window.
		activateContext();
	}

	if (m_nanoVG)
	{
		#ifdef NANOVG_GL3_IMPLEMENTATION
			nvgDeleteGL3(m_nanoVG);
		#else
			nvgDeleteGL2(m_nanoVG);
		#endif
	}

	if (m_windowHandle)
	{
		glfwDestroyWindow(m_windowHandle);
	}
}

void Window::update()
{
	if (glfwWindowShouldClose(m_windowHandle) != 0)
	{
		m_isOpen = false;
	}
}

void Window::activateContext()
{
	glfwMakeContextCurrent(m_windowHandle);
}

void Window::swapBuffers()
{
	glfwSwapBuffers(m_windowHandle);
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

void Window::osMouseEvent(EventType eventType)
{
	m_events.emplace_back(eventType);
}

void Window::osMouseEvent(EventType eventType, int button, float xP, float yP)
{
	m_events.emplace_back(eventType, Input::intToMouseButton(button), xP, yP);
}

void Window::toggleFullscreen()
{
	m_isFullscreen = !m_isFullscreen;

	if (m_isFullscreen)
	{
		// GLFW 3.1:
		{
			glfwGetWindowPos(m_windowHandle, &m_positionXBackup, &m_positionXBackup);

			create(m_name, m_width, m_height, m_isFullscreen);
		}

		/* RAE_TODO needs GLFW 3.2:
		{
			glfwGetWindowPos(m_windowHandle, &m_positionXBackup, &m_positionXBackup);
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

			m_widthBackup = m_width;
			m_heightBackup = m_height;

			glfwSetWindowMonitor(
				m_windowHandle,
				glfwGetPrimaryMonitor(),
				0,
				0,
				mode->width,
				mode->height,
				0);
		}
		*/
	}
	else
	{
		// GLFW 3.1:
		{
			m_width = m_widthBackup;
			m_height = m_heightBackup;

			create(m_name, m_width, m_height, m_isFullscreen);
		}

		/* RAE_TODO needs GLFW 3.2:
		{
			glfwSetWindowMonitor(m_windowHandle,
				nullptr,
				m_positionXBackup,
				m_positionYBackup,
				m_widthBackup,
				m_heightBackup,
				0);
		}
		*/
	}
}
