#include <iostream>
#include <string>
using namespace std;

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
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "rae/Engine.hpp"
#include "pihlaja/Pihlaja.hpp"

#include "rae/core/Log.hpp"

#define CATCH_CONFIG_RUNNER
#include "rae/core/catch.hpp"
#include <iostream>
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

	//rae_log("glfwOnMouseButtonPress. x: ", mx, " y: ", my);
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

int main(int argCount, char** ppArgs)
{
	try
	{
		// Run all unit tests.
		auto result = Catch::Session().run(argCount, ppArgs);
		if (result != 0)
		{
			std::cout << "Unit test(s) failed. Result: " << result << "\n";
			std::cout << "Press Return to Quit.\n";
			std::cin.get(); // Keep console window open.
			return -1;
		}
	}
	catch (const std::exception& ex)
	{
		auto pMessage = ex.what();
		if (pMessage)
		{
			std::cout << "An unhandled exception was thrown:\n" << pMessage << "\n";
		}
		std::cout << "Press Return to Quit.\n";
		std::cin.get(); // Keep console window open.
		return -1;
	}

	// Initialise GLFW
	if( !glfwInit() )
	{
		rae_log_error("Failed to initialize GLFW");
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
	
	if( window == nullptr )
	{
		rae_log_error("Failed to open GLFW window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		rae_log_error("Failed to initialize GLEW");
		return -1;
	}
	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	glfwSwapInterval(0);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	Pihlaja pihlaja(window);

	// Set global access point just for GLFW callbacks.
	g_engine = pihlaja.getEngine();
	glfwSetWindowSizeCallback     (window, windowSizeCallback);
	glfwSetFramebufferSizeCallback(window, windowPixelSizeCallback); // Support hi-dpi displays
	glfwSetMouseButtonCallback    (window, glfwOnMouseButton);
	glfwSetCursorPosCallback      (window, glfwOnMouseMotion);
	glfwSetKeyCallback            (window, glfwKeyCallback);
	glfwSetScrollCallback         (window, glfwScrollCallback);

	pihlaja.run();

	g_engine = nullptr;

	glfwTerminate();

	return 0;
}

