#include "rae/core/ScreenSystem.hpp"
#include "rae/core/version.hpp"

#ifdef version_glfw
	#include <GLFW/glfw3.h>
#endif

using namespace rae;

ScreenSystem::ScreenSystem()
{
	updateScreenInfo();
}

#ifdef version_cocoa

void ScreenSystem::updateScreenInfo()
{
	#ifdef DebugRae
		RaeLog << "ScreenSystem::updateScreenInfo() START.\n";
	#endif

	//If we happen to have some screens there already, we'll clear them first.
	screens.clear();

	NSRect screenRect;
	NSRect visibleRect;
	NSArray* screenArray = [NSScreen screens];
	uint screenCount = [screenArray count];
	
	#ifdef DebugScreenInfo
		RaeLog << "ScreenSystem::updateScreenInfo() found " << screenCount << " screens.\n";
	#endif
	
	for( uint i = 0; i < screenCount; i++ )
	{
		NSScreen* screen = [screenArray objectAtIndex: i];
		screenRect = [screen frame];
		NSScreen* visibleframe = [screenArray objectAtIndex: i];
		visibleRect = [screen visibleFrame];

		screens.emplace_back(
			i, screenRect.size.width, screenRect.size.height,
			visibleRect.size.width, visibleRect.size.height);
		
		#ifdef DebugScreenInfo
			RaeLog << "Screen [" << i << "]: " << screenRect.size.width << "x"
				<< screenRect.size.height << " : visibleArea: " << visibleRect.size.width
				<< "x" << visibleRect.size.height << "\n";
		#endif
		
	}
		
	#ifdef DebugRae
		RaeLog << "ScreenSystem::updateScreenInfo() END.\n";
	#endif
}

#endif // version_cocoa

#ifdef version_glfw

void ScreenSystem::updateScreenInfo()
{
	#ifdef DebugRae
		RaeLog << "ScreenSystem::updateScreenInfo() START.\n";
	#endif

	//If we happen to have some screens there already, we'll clear them first.
	screens.clear();

	int screenCount;
	GLFWmonitor** monitors = glfwGetMonitors(&screenCount);

	#ifdef DebugScreenInfo
		RaeLog << "ScreenSystem::updateScreenInfo() found " << screenCount << " screens.\n";
	#endif
	
	for (int i = 0; i < screenCount; ++i)
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[i]);

		// GLFW won't tell us the size of the visible area, which doesn't include the menubars etc.
		screens.emplace_back(i, videoMode->width, videoMode->height, videoMode->width, videoMode->height);
		
		#ifdef DebugScreenInfo
			RaeLog << "Screen [" << i << "]: " << videoMode->width << "x"
				<< videoMode->height << " : no visibleArea info available with GLFW.\n";
		#endif
		
	}
		
	#ifdef DebugRae
		RaeLog << "ScreenSystem::updateScreenInfo() END.\n";
	#endif
}

#endif // version_glfw

int ScreenSystem::numberOfScreens()
{
	return int(screens.size());
}

int ScreenSystem::screenWidthP(int set_screen)
{
	return screens[set_screen].screenWidthP();
}

int ScreenSystem::screenHeightP(int set_screen)
{
	return screens[set_screen].screenHeightP();
}

float ScreenSystem::screenWidth(int set_screen)
{
	return screens[set_screen].screenWidth();
}

float ScreenSystem::screenHeight(int set_screen)
{
	return screens[set_screen].screenHeight();
}

float ScreenSystem::screenHalfWidthP(int set_screen)
{
	return screens[set_screen].screenHalfWidthP();
}

float ScreenSystem::screenHalfHeightP(int set_screen)
{
	return screens[set_screen].screenHalfHeightP();
}
float ScreenSystem::screenHalfWidth(int set_screen)
{
	return screens[set_screen].screenHalfWidth();
}
float ScreenSystem::screenHalfHeight(int set_screen)
{
	return screens[set_screen].screenHalfHeight();
}

float ScreenSystem::screenAspect(int set_screen)
{
	return screens[set_screen].screenAspect();
}

float ScreenSystem::pixel(int set_screen)
{
	return screens[set_screen].pixel();
}

float ScreenSystem::pixelsToHeight(int set_screen)
{
	return screens[set_screen].pixelsToHeight();
}

float ScreenSystem::pixelsToHeight(float in_pixels, int set_screen)
{
	//return in_pixels * m_pixelsToHeight;
	return screens[set_screen].pixelsToHeight(in_pixels);
}

float ScreenSystem::heightToPixels(float in_height, int set_screen)
{
	//return in_height * m_heightToPixels;
	return screens[set_screen].heightToPixels(in_height);
}

float ScreenSystem::heightToPixels(int set_screen)
{
	return screens[set_screen].heightToPixels();
}

float ScreenSystem::roundToPixels( float set, int set_screen)
{
	//float temppixel = pixel();
	//return ((float)((int)( set / temppixel ))) * temppixel;
	return screens[set_screen].roundToPixels(set);
}

// The default dpi (dots per inch) is 96.0f.
// The user should change it according to screen size.
// A smaller dpi will result in smaller widgets and smaller text.
void ScreenSystem::dpi(float set, int set_screen)
{
	screens[set_screen].dpi(set);
}
float ScreenSystem::dpi(int set_screen)
{
	return screens[set_screen].dpi();
}
float ScreenSystem::dpiMul(float set, int set_screen)
{
	return screens[set_screen].dpiMul(set);
}
float ScreenSystem::dpiMul(int set_screen)
{
	return screens[set_screen].dpiMul();
}
void ScreenSystem::dpiToDefault(int set_screen)
{
	screens[set_screen].dpiToDefault();
}
float ScreenSystem::curveSideSize(int set_screen)
{
	return screens[set_screen].curveSideSize();
}

// Convert a percentage (e.g. -0.5 - 0.5f which is the whole screen) to "height units" (in the width direction)
// e.g. 0.5 is 50% and it will be half of 1.78 when the aspect ratio is 1:1.78 (16:9)
// so the result will be 0.89, which is half of the screen width in "height units".
float ScreenSystem::percentToWidth(float in_percent, int set_screen)
{
	return screens[set_screen].percentToWidth(in_percent);
}
