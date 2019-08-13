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

	float mmToPixels(float mm)       const { return mm * pixelsPerMM(); };
	vec2  mmToPixels(const vec2& mm) const { return mm * pixelsPerMM(); };
	vec3  mmToPixels(const vec3& mm) const { return mm * pixelsPerMM(); };

	float pixelsToMM(float pixels) const { return pixels / pixelsPerMM(); };
	vec2 pixelsToMM(const vec2& pixels) const { return pixels / pixelsPerMM(); };
	vec3 pixelsToMM(const vec3& pixels) const { return pixels / pixelsPerMM(); };

	float heightToAltPixels(float heightCoords) const { return heightCoords * screenHeightP(); }
	// TODO: micro-optimize to multiplications:
	float altPixelsToHeight(float pixels) const { return pixels / screenHeightP(); }

	void updateScreenInfo();
	int numberOfScreens() const;

	float ppmm(int screenIndex = 0) const; // shorthand
	float pixelsPerMM(int screenIndex = 0) const;

	int screenWidthP(int screenIndex = 0) const;
	int screenHeightP(int screenIndex = 0) const;
	// Screen width and height in height coordinates.
	float screenWidth(int screenIndex = 0) const;
	// screenHeight in height coordinates should always be 1.0f.
	float screenHeight(int screenIndex = 0) const;

	float screenHalfWidthP(int screenIndex = 0) const;
	float screenHalfHeightP(int screenIndex = 0) const;
	float screenHalfWidth(int screenIndex = 0) const;
	float screenHalfHeight(int screenIndex = 0) const;

	float screenAspect(int screenIndex = 0) const;

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

	float pixel(int screenIndex = 0) const;
	float pixelsToHeightOLD(int screenIndex = 0) const;

	float heightToPixelsOLD(int screenIndex = 0) const;

	float pixelsToHeightOLD(float pixels, int screenIndex = 0) const;
	float heightToPixelsOLD(float heightCoords, int screenIndex = 0) const;

	float percentToWidth(float percent, int screenIndex = 0) const;

	// This returns and accepts Height coordinates
	float roundToPixels( float heightCoords, int screenIndex = 0 ) const;

	// The default dpi (dots per inch) is 96.0f.
	// The user should change it according to screen size.
	// A smaller dpi will result in smaller widgets and smaller text.
	void setDpi(float value, int screenIndex = 0);
	float dpi(int screenIndex = 0) const;
	void setDpiMul(float value, int screenIndex = 0);
	float dpiMul(int screenIndex = 0) const;
	void dpiToDefault(int screenIndex = 0);
	float curveSideSize(int screenIndex = 0) const;

protected:
	Array<ScreenInfo>	screens;
};

}
