#pragma once

#include "rae/core/version.hpp"
#include <GL/glew.h>
#ifdef version_glfw
	#include <GLFW/glfw3.h>
#endif

#include "rae/core/Types.hpp"
#include "rae/ui/Window.hpp"
#include "rae/core/ISystem.hpp"

struct NVGcontext;

namespace rae
{

class Input;
class Window;

class WindowSystem : public ISystem
{
public:
	WindowSystem(
		Input& input,
		const String& mainWindowName,
		int mainWindowWidth = -1,
		int mainWindowHeight = -1,
		bool isFullscreen = false);
	WindowSystem(GLFWwindow* windowHandle, Input& input);
	~WindowSystem();

	virtual UpdateStatus update() override;
	virtual void onFrameEnd() override;

	// TODO multiwindow support
	void osEventResizeWindow(GLFWwindow* windowHandle, int width, int height);
	void osEventResizeWindowPixels(GLFWwindow* windowHandle, int width, int height);

	void osEventCursorEnter(GLFWwindow* windowHandle);
	void osEventCursorLeave(GLFWwindow* windowHandle);

	void osMouseButtonPress(GLFWwindow* windowHandle, int button, float x, float y);
	void osMouseButtonRelease(GLFWwindow* windowHandle, int button, float x, float y);
	void osMouseMotion(GLFWwindow* windowHandle, float x, float y);
	void osScrollEvent(GLFWwindow* windowHandle, float scrollX, float scrollY);
	void osKeyEvent(GLFWwindow* windowHandle, int key, int scancode, int action, int mods);

	const Window& mainWindow() const { return *m_windows[0]; }
	Window& modifyMainWindow() { return *m_windows[0]; }

	const Window& window(int index) const { return *m_windows[index]; }
	Window& modifyWindow(int index) { return *m_windows[index]; }
	Window* windowPtr(GLFWwindow* windowHandle);
	int windowCount() const { return (int)m_windows.size(); }

	Window& createWindow(const String& title, int width, int height, bool isFullscreen);

private:

	void initOnce();

	Input& m_input;

	Array<UniquePtr<Window>>		m_windows;
};

}
