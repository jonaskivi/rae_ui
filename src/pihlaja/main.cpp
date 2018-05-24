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

#include "nanovg.h"
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

int main(int argc, char* argv[])
{
	loguru::init(argc, argv);

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--help" || arg == "-h")
		{
			std::cout << "Usage: " << argv[0] << " <OPTIONS>\n";
			std::cout << "\t--help -h : This help screen.\n";
			std::cout << "\t--version : Print out the version of the application and some of the libraries.\n";
			return 0;
		}
		else if (arg == "--version")
		{
			std::cout << "Pihlaja version " << __DATE__ << "\n";
			std::cout << "GLFW version "
				<< GLFW_VERSION_MAJOR << "."
				<< GLFW_VERSION_MINOR << "."
				<< GLFW_VERSION_REVISION << "\n";
			return 0;
		}
	}

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

	{
		Test2DCoordinates test2DCoordinates;
		g_engine = test2DCoordinates.getEngine();
		test2DCoordinates.run();
		g_engine = nullptr;
	}

	{
		Pihlaja pihlaja;
		g_engine = pihlaja.getEngine();
		pihlaja.run();
		g_engine = nullptr;
	}

	glfwTerminate();

	return 0;
}

