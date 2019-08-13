#pragma once

#include "rae/core/Types.hpp"
#include "rae/scene/Scene.hpp"

namespace rae
{

class Time;
class AssetSystem;

class SceneSystem : public ISystem
{
public:
	SceneSystem(
		const Time& time,
		Input& input);

	virtual UpdateStatus update() override;
	virtual void onFrameEnd() override;

	Scene& createScene(const String& name = "Untitled");
	void activateScene(int index);

	bool hasActiveScene() const
	{
		if (m_activeSceneIdx >= 0 && m_activeSceneIdx < (int)m_scenes.size())
			return true;
		return false;
	}

	const Scene& activeScene() const { return *m_scenes[m_activeSceneIdx]; }
	Scene& modifyActiveScene() { return *m_scenes[m_activeSceneIdx]; }

	bool hasScene(int index) const
	{
		if (index >= 0 && index < (int)m_scenes.size())
			return true;
		return false;
	}

	const Scene& defaultScene() const { return scene(0); }
	Scene& defaultScene() { return scene(0); }

	const Scene& scene(int index) const { return *m_scenes[index]; }
	Scene& scene(int index) { return *m_scenes[index]; }

	// Returns -1 if the scene was not found.
	int getSceneIndex(const Scene& findScene)
	{
		int i = 0;
		for (auto&& scene : m_scenes)
		{
			if (&findScene == scene.get())
			{
				return i;
			}
			i++;
		}
		// RAE_TODO temp
		assert(0);
		return -1;
	}

private:
	void deactivateAllScenes();

	const Time&			m_time;
	Input&				m_input;

	Array<UniquePtr<Scene>>	m_scenes;
	int						m_activeSceneIdx = 0;
};

}
