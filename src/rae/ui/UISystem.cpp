#include "rae/ui/UISystem.hpp"

#include <memory>

#include "nanovg.h"
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

#include "loguru/loguru.hpp"
#include "rae/core/Utils.hpp"
#include "rae/core/Time.hpp"
#include "rae/core/ScreenSystem.hpp"
#include "rae/ui/Input.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/visual/Box.hpp"

using namespace rae;

rae::UISystem* rae::g_ui = nullptr;

UISystem::UISystem(
	WindowSystem& windowSystem,
	const Time& time,
	Input& input,
	ScreenSystem& screenSystem,
	AssetSystem& assetSystem,
	DebugSystem& debugSystem
	) :
		ISystem("UISystem"),
		m_time(time),
		m_input(input),
		m_windowSystem(windowSystem),
		m_screenSystem(screenSystem),
		m_assetSystem(assetSystem),
		m_debugSystem(debugSystem)
{
	// RAE_TODO reuse default theme across ui scenes: createDefaultTheme();

	createUIScene("Default");

	g_ui = this;
}

UISystem::~UISystem()
{
	g_ui = nullptr;
}

UIScene& UISystem::createUIScene(const String& name)
{
	LOG_F(INFO, "Creating UIScene: %s", name.c_str());
	m_uiScenes.emplace_back(std::make_unique<UIScene>(
		name, m_time, m_input, m_screenSystem, m_debugSystem, m_assetSystem));
	return *m_uiScenes.back();
}

void UISystem::connectWindowToScene(Window& window, UIScene& uiScene)
{
	window.setUISceneIndex(getSceneIndex(uiScene));
	uiScene.connectToWindow(window);
}

UpdateStatus UISystem::update()
{
	UpdateStatus status = UpdateStatus::NotChanged;
	for (auto&& uiScene : m_uiScenes)
	{
		auto sceneStatus = uiScene->update();
		if (sceneStatus != UpdateStatus::NotChanged)
		{
			status = sceneStatus;
		}
	}

	return status;
}

void UISystem::onFrameEnd()
{
	for (auto&& uiScene : m_uiScenes)
	{
		uiScene->onFrameEnd();
	}
}

void UISystem::render2D(UIScene& uiScene, NVGcontext* nanoVG)
{
	uiScene.render2D(nanoVG, m_assetSystem);
}
