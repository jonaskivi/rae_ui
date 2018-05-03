#pragma once

#include "loguru/loguru.hpp"
#include "rae/core/Types.hpp"

namespace rae
{

class ScreenInfo
{
public:

	ScreenInfo();
	ScreenInfo(
		int screenNumber,
		int screenWidthP,
		int screenHeightP,
		int visibleAreaWidthP = -1,
		int visibleAreaHeightP = -1);
	~ScreenInfo(){}

	void printInfo()
	{
		LOG_F(INFO, "Screen number: %i width: %i height: %i aspect ratio: %f",
			screenNumber(), screenWidthP(), screenHeightP(), screenAspect());
	}

	// -1 is UNDEFINED, 0 is the first screen
	int screenNumber() { return m_screenNumber; }
	void setScreenNumber(int set) { m_screenNumber = set; }

	void setScreenSizeP(int set_widthP, int set_heightP);

	int screenWidthP() { return m_screenWidthP; }
	void screenWidthP(int set);

	int screenHeightP() { return m_screenHeightP; }
	void screenHeightP(int set);

	// Screen width and height in height coordinates.
	float screenWidth() { return 1.0f * screenAspect(); }
	// screenHeight in height coordinates should always be 1.0f.
	float screenHeight() { return 1.0f; }

	// helpers for half screenWidth and height:

	float screenHalfWidth() { return 0.5f * screenAspect(); }
	// screenHalfHeight in height coordinates should always be 0.5f.
	float screenHalfHeight() { return 0.5f; }

	void calculateHalfScreens()
	{
		screenHalfWidthP( float(screenWidthP() / 2) );
		screenHalfHeightP( float(screenHeightP() / 2) );
	}
	float screenHalfWidthP() { return m_screenHalfWidthP; }
	void screenHalfWidthP(float set) { m_screenHalfWidthP = set; }

	float screenHalfHeightP() { return m_screenHalfHeightP; }
	void screenHalfHeightP(float set) { m_screenHalfHeightP = set; }

	void calculateScreenAspect()
	{
		screenAspect( (float)(m_screenWidthP)/(float)(m_screenHeightP) );
	}
	float screenAspect() { return m_screenAspect; }
	void screenAspect(float set) { m_screenAspect = set; }

	// On Mac OS X these are the visibleFrame, which is the size of the screen that is not
	// occupied by the top main menu, the dock 
	// (or if it is hidden the activation area of the dock) etc.
	// So, if you would create a window with these sizes, it would not overlap the dock or menu.

	// With these, we'll use -1 as UNDEFINED
	int visibleAreaWidthP() { return m_visibleAreaWidthP; }
	void visibleAreaWidthP(int set) { m_visibleAreaWidthP = set; }

	int visibleAreaHeightP() { return m_visibleAreaHeightP; }
	void visibleAreaHeightP(int set) { m_visibleAreaHeightP = set; }

	//

	float pixel() { return m_pixelsToHeight; }
	float pixelsToHeight() { return m_pixelsToHeight; }

	float heightToPixels() { return m_heightToPixels; }

	// Ok. We should make these methods as well:
	float pixelsToHeight(float inPixels) { return inPixels * m_pixelsToHeight; }
	float heightToPixels(float inHeight) { return inHeight * m_heightToPixels; }

	// Convert a percentage (e.g. -0.5 - 0.5f which is the whole screen) to "height units" (in the width direction)
	// e.g. 0.5 is 50% and it will be half of 1.78 when the aspect ratio is 1:1.78 (16:9)
	// so the result will be 0.89, which is half of the screen width in "height units".
	float percentToWidth(float inPercent) { return inPercent * screenAspect(); }

	// This returns and accepts Height coordinates
	float roundToPixels( float set )
	{
		float tempPixel = pixel();
		return ((float)((int)( set / tempPixel ))) * tempPixel;
	}

	// Always try to use PPCM (pixels per cm) or PPMM instead of DPI, even though DPI is more commonly used.
	// Use SI everywhere.
	float ppmm() { return m_pixelsPerMM; } // A shorthand
	float pixelsPerMM() { return m_pixelsPerMM; }

	float ppcm() { return m_pixelsPerMM * 10.0f; } // A shorthand
	float pixelsPerCM() { return m_pixelsPerMM * 10.0f; }
	// Calculate the PPCM and DPI using the physical size, if physical size is set (> 0.0f)
	void calculatePixelsPerMM();

	// The default dpi (dots per inch) is 96.0f.
	// The user should change it according to screen size.
	// A smaller dpi will result in smaller widgets and
	// smaller text.
	void setDpi(float set);
	float dpi() { return m_dpi; }

	float setDpiMul(float set) { return m_dpiMul * set; }
	float dpiMul() { return m_dpiMul; }

	void dpiToDefault() { setDpi(96.0f); }

	// A setting for RoundedRectangles:
	// e.g. in HORIZONTAL this is
	// the width of leftRect and rightRect.
	// This must be correct for the given dpi
	// because otherwise the rounded curve will
	// get clipped too soon.
	// Setting the dpi will change this too.
	// And it will get adjusted to be in the range 0.25f - 0.48f.
	float curveSideSize() { return m_curveSideSize; }

	void setName(const String& name) { m_name = name; }
	void setPhysicalSize(float widthMM, float heightMM)
	{
		m_widthMM = widthMM;
		m_heightMM = heightMM;
	}

	float widthMM() { return m_widthMM; }
	float heightMM() { return m_heightMM; }

protected:
	int m_screenNumber = -1;

	int m_screenWidthP = 1280;
	int m_screenHeightP = 800;

	float m_screenHalfWidthP;
	float m_screenHalfHeightP;

	float m_screenAspect;

	int m_visibleAreaWidthP = -1;
	int m_visibleAreaHeightP = -1;

	float m_pixelsToHeight = 0.00125f; // 1.0f / screenHeightP;
	float m_heightToPixels = 800.0f; // the same as m_screenHeightP...

	float m_pixelsPerMM;
	float m_dpi;
	float m_dpiMul;

	float m_curveSideSize = 0.3f;

	String m_name = "Unnamed monitor";
	float m_widthMM = -1.0f;
	float m_heightMM = -1.0f;
};

} // namespace rae
