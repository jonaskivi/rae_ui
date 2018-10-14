#include "rae/scene/SceneSystem.hpp"

#include "rae/core/Time.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/ui/DebugSystem.hpp"

using namespace rae;

SceneSystem::SceneSystem(
	const Time& time,
	Input& input) :
		ISystem("SceneSystem"),
		m_time(time),
		m_input(input)
{
	LOG_F(INFO, "Init %s", name().c_str());

	m_scenes.reserve(4);

	// We must always have a scene
	createScene("Default");
	activateScene(0);
}

Scene& SceneSystem::createScene(const String& name)
{
	LOG_F(INFO, "Creating Scene: %s", name.c_str());
	m_scenes.emplace_back(name, m_time, m_input);
	return m_scenes.back();
}

void SceneSystem::activateScene(int index)
{
	if (index < 0 || index >= (int)m_scenes.size())
	{
		LOG_F(ERROR, "Can't activate scene. Invalid index: %i. There are only %i scenes.", index, (int)m_scenes.size());
		return;
	}

	deactivateAllScenes();
	m_activeSceneIdx = index;
	activeScene().setIsActive(true);
	LOG_F(INFO, "Active Scene set to %i %s", m_activeSceneIdx, activeScene().m_entitySystem.owner().c_str());
}

void SceneSystem::deactivateAllScenes()
{
	for (auto&& scene : m_scenes)
	{
		scene.setIsActive(false);
	}
}

UpdateStatus SceneSystem::update()
{
	if (!hasActiveScene())
		return UpdateStatus::NotChanged;

	return activeScene().update();

	// Update all scenes version:
	//UpdateStatus status;
	//for (int i = 0; i < (int)m_scenes.size(); ++i)
	//{
	//	status = m_scenes[i].update();
	//}
	//return status;
}
