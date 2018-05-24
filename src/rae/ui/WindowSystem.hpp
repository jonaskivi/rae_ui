#pragma once

#include "rae/core/version.hpp"
#include "rae/core/Types.hpp"

#ifdef version_glfw
	#include <GLFW/glfw3.h>
#endif

struct NVGcontext;

namespace rae
{

class Input;

class Window
{
public:
	Window(const String& name, int width, int height);

	Window(GLFWwindow* window) :
		m_window(window)
	{
	}

	~Window();

	Window(Window&& other);

#ifdef version_glfw
	GLFWwindow* windowHandle() const { return m_window; }
#endif

	NVGcontext* nanoVG() const { return m_nanoVG; }

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

	void osEventResizeWindow(int width, int height);
	void osEventResizeWindowPixels(int width, int height);

	void setUISceneIndex(int uiSceneIndex) { m_uiSceneIndex = uiSceneIndex; }
	int uiSceneIndex() { return m_uiSceneIndex; }

private:

#ifdef version_glfw
	GLFWwindow* m_window;
#endif

	NVGcontext* m_nanoVG;

	String m_name;
	int m_width = 0;
	int m_height = 0;
	int m_pixelWidth = 0;
	int m_pixelHeight = 0;
	// Window pixel ratio for hi-dpi screens.
	float m_screenPixelRatio = 1.0f;

	//float m_pixelsPerMM = 0.0f;

	int m_uiSceneIndex = -1;
};

class WindowSystem
{
friend class UISystem;

public:
	WindowSystem(Input& input, const String& mainWindowName, int mainWindowWidth = -1, int mainWindowHeight = -1);
	WindowSystem(GLFWwindow* windowHandle, Input& input);
	~WindowSystem();

	// TODO multiwindow support
	void osEventResizeWindow(GLFWwindow* windowHandle, int width, int height);
	void osEventResizeWindowPixels(GLFWwindow* windowHandle, int width, int height);

	void osMouseButtonPress(GLFWwindow* windowHandle, int button, float x, float y);
	void osMouseButtonRelease(GLFWwindow* windowHandle, int button, float x, float y);
	void osMouseMotion(GLFWwindow* windowHandle, float x, float y);
	void osScrollEvent(GLFWwindow* windowHandle, float scrollX, float scrollY);
	void osKeyEvent(GLFWwindow* windowHandle, int key, int scancode, int action, int mods);

	const Window& mainWindow() const { return m_windows[0]; }
	Window& mainWindow() { return m_windows[0]; }

	const Window& window(int index) const { return m_windows[index]; }
	Window& window(int index) { return m_windows[index]; }
	Window* window(GLFWwindow* windowHandle);
	int windowCount() const { return (int)m_windows.size(); }

	Window& createWindow(const String& title, int width, int height);

private:

	void initOnce();

	Input& m_input;

	// Possibly we could use Table here too? But can we use it without an EntitySystem?
	Array<Window>		m_windows;
};

}
