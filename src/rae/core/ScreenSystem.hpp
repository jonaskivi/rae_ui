#pragma once

#include <GLFW/glfw3.h>

#include "rae/core/version.hpp"
#include "rae/core/Types.hpp"
#include "rae/core/ScreenInfo.hpp"

namespace rae
{

class Window
{
public:
	Window(GLFWwindow* window) :
		m_window(window)
	{
	}

#ifdef version_glfw
	GLFWwindow* windowHandle() const { return m_window; }
#endif

	int   width()       const { return m_width;       }
	int   height()      const { return m_height;      }
	int   pixelWidth()  const { return m_pixelWidth;  }
	int   pixelHeight() const { return m_pixelHeight; }
	float screenPixelRatio()  const { return m_screenPixelRatio;  }

	void osEventResizeWindow(int width, int height);
	void osEventResizeWindowPixels(int width, int height);

private:

#ifdef version_glfw
	GLFWwindow* m_window;
#endif

	int m_width = 0;
	int m_height = 0;
	int m_pixelWidth = 0;
	int m_pixelHeight = 0;
	// Window pixel ratio for hi-dpi screens.
	float m_screenPixelRatio  = 1.0f;

	//float m_pixelsPerMM = 0.0f;
};

// RAE_TODO rename to WindowSystem
class ScreenSystem
{
public:

	ScreenSystem(GLFWwindow* window);

	// TODO multiwindow support
	void osEventResizeWindow(int width, int height);
	void osEventResizeWindowPixels(int width, int height);

	const Window& window(int index = 0) { return m_windows[index]; }

	float xPixelsToNormalizedWindow(float pixels) { return pixels / m_windows[0].pixelWidth(); }
	float yPixelsToNormalizedWindow(float pixels) { return pixels / m_windows[0].pixelHeight(); }

	float heightToPixels(float heightCoords) { return heightCoords * screenHeightP() * m_windows[0].screenPixelRatio(); }
	// TODO: micro-optimize to multiplications:
	float pixelsToHeight(float pixels) { return pixels / screenHeightP() / m_windows[0].screenPixelRatio(); }

	float mmToPixels(float mm) { return mm * pixelsPerMM(); };
	float pixelsToMM(float pixels) { return pixels / pixelsPerMM(); };

	float heightToAltPixels(float heightCoords) { return heightCoords * screenHeightP(); }
	// TODO: micro-optimize to multiplications:
	float altPixelsToHeight(float pixels) { return pixels / screenHeightP(); }

	void updateScreenInfo();
	int numberOfScreens();

	float ppmm(int screenIndex = 0); // shorthand
	float pixelsPerMM(int screenIndex = 0);

	int screenWidthP(int set_screen = 0);
	int screenHeightP(int set_screen = 0);
	// Screen width and height in height coordinates.
	float screenWidth(int set_screen = 0);
	// screenHeight in height coordinates should always be 1.0f.
	float screenHeight(int set_screen = 0);
	
	float screenHalfWidthP(int set_screen = 0);
	float screenHalfHeightP(int set_screen = 0);
	float screenHalfWidth(int set_screen = 0);
	float screenHalfHeight(int set_screen = 0);

	float screenAspect(int set_screen = 0);
	
	// to convert Height coordinates to pixels:
	// inHCoord * pixelsToHeight()
	// to convert pixels to Height coordinates:
	// inPixels * heightToPixels()
	// or inPixels * screenHeightP()
		
	// The height of one pixel in Height coordinates.
	// to convert pixels to Height coordinates:
	// inPixels * pixel()
	// to convert Height coordinates to pixels (slowly) (it's better to use heightToPixels *):
	// inHCoord / pixel()
	// float pixel() { return 1.0f/m_screenHeightP; }
	
	float pixel(int set_screen = 0);
	float pixelsToHeightOLD(int set_screen = 0);
	
	float heightToPixelsOLD(int set_screen = 0);
	
	float pixelsToHeightOLD(float in_pixels, int set_screen = 0);
	float heightToPixelsOLD(float in_height, int set_screen = 0);
	
	float percentToWidth(float in_percent, int set_screen = 0);

	// This returns and accepts Height coordinates
	float roundToPixels( float set, int set_screen = 0 );
	
	// The default dpi (dots per inch) is 96.0f.
	// The user should change it according to screen size.
	// A smaller dpi will result in smaller widgets and smaller text.
	void dpi(float set, int set_screen = 0);
	float dpi(int set_screen = 0);
	float dpiMul(float set, int set_screen = 0);
	float dpiMul(int set_screen = 0);
	void dpiToDefault(int set_screen = 0);
	float curveSideSize(int set_screen = 0);

protected:
	Array<ScreenInfo>	screens;
	Array<Window>		m_windows;
};
	
}
