#pragma once

#include "rae/core/Types.hpp"
#include "rae/entity/Table.hpp"

#include "rae/core/ISystem.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/ui/WindowSystem.hpp"
#include "rae/ui/UIScene.hpp"

namespace rae
{

class Time;
class Input;
class ScreenSystem;
class AssetSystem;
class DebugSystem;

class UISystem : public ISystem
{
public:
	UISystem(
		WindowSystem& windowSystem,
		const Time& time,
		Input& input,
		ScreenSystem& screenSystem,
		AssetSystem& assetSystem,
		DebugSystem& debugSystem);

	~UISystem();

	UIScene& createUIScene(const String& name = "Untitled");
	void connectWindowToScene(Window& window, UIScene& uiScene);

	UpdateStatus update() override;
	virtual void render2D(UIScene& uiScene, NVGcontext* nanoVG) override;

	bool hasScene(int index) const
	{
		if (index >= 0 && index < (int)m_uiScenes.size())
			return true;
		return false;
	}

	const UIScene& defaultScene() const { return scene(0); }
	UIScene& defaultScene() { return scene(0); }

	const UIScene& scene(int index) const { return m_uiScenes[index]; }
	UIScene& scene(int index) { return m_uiScenes[index]; }

	// Returns -1 if the scene was not found.
	int getSceneIndex(const UIScene& findScene)
	{
		int i = 0;
		for (auto&& scene : m_uiScenes)
		{
			if (&findScene == &scene)
			{
				return i;
			}
			i++;
		}
		//RAE_TODO temp
		assert(0);
		return -1;
	}

private:

	//RAE_TODO void createDefaultTheme();

	const Time&			m_time;
	Input&				m_input;
	ScreenSystem&		m_screenSystem;
	WindowSystem&		m_windowSystem;
	AssetSystem&		m_assetSystem;
	DebugSystem&		m_debugSystem;

	//RAE_TODO RENAME TO m_scenes
	Array<UIScene>		m_uiScenes;
};

extern UISystem* g_ui;

}
