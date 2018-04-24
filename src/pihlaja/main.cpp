#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#ifdef __APPLE__
#	define GLFW_INCLUDE_GLCOREARB
#endif
#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include "nanovg.h"

#ifdef _WIN32
#define NANOVG_GL3_IMPLEMENTATION
#else
#define NANOVG_GL2_IMPLEMENTATION
#endif

#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/Engine.hpp"
#include "test/Test2DCoordinates.hpp"
#include "pihlaja/Pihlaja.hpp"

#define LOGURU_IMPLEMENTATION 1
#include "loguru/loguru.hpp"

#define CATCH_CONFIG_RUNNER
#include "rae/core/catch.hpp"
#include <exception>

//----------------------------------------

Engine* g_engine = nullptr;

void windowSizeCallback(GLFWwindow* window, int width, int height)
{
	if( g_engine == nullptr )
		return;
	g_engine->osEventResizeWindow(width, height);
}

void windowPixelSizeCallback(GLFWwindow* window, int width, int height)
{
	if( g_engine == nullptr )
		return;
	g_engine->osEventResizeWindowPixels(width, height);
}

void glfwOnMouseButton(GLFWwindow* set_window, int set_button, int set_action, int set_mods)
{
	if( g_engine == nullptr )
		return;

	double mx, my;
	glfwGetCursorPos(set_window, &mx, &my);

	//LOG_F(INFO, "glfwOnMouseButtonPress. x: %f y: %f", mx, my);
	if(set_action == GLFW_PRESS)
	{
		g_engine->osMouseButtonPress(set_button, (float)mx, (float)my);
	}
	else if(set_action == GLFW_RELEASE)
	{
		g_engine->osMouseButtonRelease(set_button, (float)mx, (float)my);
	}
}

void glfwOnMouseMotion(GLFWwindow* set_window, double set_x, double set_y)
{
	if( g_engine == nullptr )
		return;

	g_engine->osMouseMotion((float)set_x, (float)set_y);
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if( g_engine == nullptr )
		return;

	g_engine->osKeyEvent(key, scancode, action, mods);
}

void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if( g_engine == nullptr )
		return;

	g_engine->osScrollEvent((float)xoffset, (float)yoffset);	
}

int main(int argc, char** argv)
{
	loguru::init(argc, argv);

	try
	{
		// Run all unit tests.
		int result = Catch::Session().run(argc, argv);
		if (result != 0)
		{
			LOG_F(ERROR, "Unit test(s) failed. Result: %i", result);
			LOG_F(INFO, "Press Return to Quit.");
			std::cin.get(); // Keep console window open.
			return -1;
		}
	}
	catch (const std::exception& ex)
	{
		auto pMessage = ex.what();
		if (pMessage)
		{
			LOG_F(ERROR, "An unhandled exception was thrown:\n%s", pMessage);
		}
		LOG_F(INFO, "Press Return to Quit.");
		std::cin.get(); // Keep console window open.
		return -1;
	}

	// Initialise GLFW
	if( !glfwInit() )
	{
		LOG_F(ERROR, "Failed to initialize GLFW.");
		return -1;
	}

	// Enable MSAA (multisample anti-aliasing)
	glfwWindowHint(GLFW_SAMPLES, 4);

	#ifndef _WIN32 // don't require this on win32, and works with more cards
	// Set OpenGL version to 2.1
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	#endif
	
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	// Create window

	GLFWmonitor* screen     = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(screen);

	window = glfwCreateWindow(mode->width - 200, mode->height - 200, "Pihlaja", nullptr, nullptr);

	if (window == nullptr)
	{
		LOG_F(ERROR, "Failed to open GLFW window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		LOG_F(ERROR, "Failed to initialize GLEW.");
		return -1;
	}
	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	glfwSwapInterval(0);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetWindowSizeCallback     (window, windowSizeCallback);
	glfwSetFramebufferSizeCallback(window, windowPixelSizeCallback); // Support hi-dpi displays
	glfwSetMouseButtonCallback    (window, glfwOnMouseButton);
	glfwSetCursorPosCallback      (window, glfwOnMouseMotion);
	glfwSetKeyCallback            (window, glfwKeyCallback);
	glfwSetScrollCallback         (window, glfwScrollCallback);

	NVGcontext* nanoVG;

	#ifdef _WIN32
		nanoVG = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	#else
		nanoVG = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	#endif

	if (nanoVG == nullptr)
	{
		LOG_F(ERROR, "Could not init nanovg.");
		getchar();
		exit(0);
		assert(0);
	}

	{
		Test2DCoordinates test2DCoordinates(window, nanoVG);
		g_engine = test2DCoordinates.getEngine();
		test2DCoordinates.run();
		g_engine = nullptr;
	}

	{
		Pihlaja pihlaja(window, nanoVG);
		g_engine = pihlaja.getEngine();
		pihlaja.run();
		g_engine = nullptr;
	}

	glfwTerminate();

	return 0;
}

