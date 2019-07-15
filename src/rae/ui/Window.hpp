#pragma once

#include "rae/core/version.hpp"
#include "rae/ui/Input.hpp"

#include <GL/glew.h>
#ifdef version_glfw
	#include <GLFW/glfw3.h>
#endif

#include "rae/core/Types.hpp"

struct NVGcontext;

namespace rae
{

class Window
{
public:
	Window(const String& name, int width, int height, bool isFullscreen);

	Window(GLFWwindow* windowHandle) :
		m_windowHandle(windowHandle)
	{
	}

	~Window();

	void create(const String& name, int width, int height, bool isFullscreen);
	void destroy();

#ifdef version_glfw
	GLFWwindow* windowHandle() const { return m_windowHandle; }
#endif

	NVGcontext* nanoVG() const { return m_nanoVG; }

	void update();
	void activateContext();
	void swapBuffers();

	String name() { return m_name; }

	int width()			const { return m_width; }
	int height()		const { return m_height; }
	int pixelWidth()	const { return m_pixelWidth; }
	int pixelHeight()	const { return m_pixelHeight; }

	float screenPixelRatio() const { return m_screenPixelRatio; }

	float xPixelsToNormalizedWindow(float pixels) const { return pixels / pixelWidth(); }
	float yPixelsToNormalizedWindow(float pixels) const { return pixels / pixelHeight(); }

	void setUISceneIndex(int uiSceneIndex) { m_uiSceneIndex = uiSceneIndex; }
	int uiSceneIndex() { return m_uiSceneIndex; }

	void osEventResizeWindow(int width, int height);
	void osEventResizeWindowPixels(int width, int height);

	void osMouseEvent(EventType eventType);
	void osMouseEvent(EventType eventType, int button, float xP, float yP);

	const Array<InputEvent>& events() { return m_events; }
	void clearEvents() { m_events.clear(); }

	bool isOpen() const { return m_isOpen; }
	void toggleFullscreen();

private:

#ifdef version_glfw
	GLFWwindow* m_windowHandle = nullptr;
#endif

	NVGcontext* m_nanoVG = nullptr;

	String m_name;
	int m_width = 0;
	int m_height = 0;
	int m_pixelWidth = 0;
	int m_pixelHeight = 0;
	// Window pixel ratio for hi-dpi screens.
	float m_screenPixelRatio = 1.0f;

	//float m_pixelsPerMM = 0.0f;

	int m_uiSceneIndex = -1;

	bool m_isOpen = true;

	bool m_isFullscreen = false;
	// Save windowed position and size when going to fullscreen:
	int m_positionXBackup = 0;
	int m_positionYBackup = 0;
	int m_widthBackup = 0;
	int m_heightBackup = 0;

	// Events to be handled.
	Array<InputEvent> m_events;
};

}

