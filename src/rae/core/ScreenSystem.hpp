#pragma once

#include "rae/core/version.hpp"
#include "rae/core/Types.hpp"
#include "rae/core/ScreenInfo.hpp"

namespace rae
{

class ScreenSystem
{
public:

	ScreenSystem();

	float heightToPixels(float heightCoords) { return heightCoords * screenHeightP() * 1.0f; };//RAE_TODO * m_windows[0].screenPixelRatio(); }
	// TODO: micro-optimize to multiplications:
	float pixelsToHeight(float pixels) { return pixels / screenHeightP(); }; //RAE_TODO / m_windows[0].screenPixelRatio(); }

	float mmToPixels(float mm)       { return mm * pixelsPerMM(); };
	vec2  mmToPixels(const vec2& mm) { return mm * pixelsPerMM(); };
	vec3  mmToPixels(const vec3& mm) { return mm * pixelsPerMM(); };

	float pixelsToMM(float pixels) { return pixels / pixelsPerMM(); };
	vec2 pixelsToMM(const vec2& pixels) { return pixels / pixelsPerMM(); };
	vec3 pixelsToMM(const vec3& pixels) { return pixels / pixelsPerMM(); };

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
};

}
