#pragma once

#include "rae/core/Types.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/entity/Hierarchy.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"

namespace rae
{

class Time;
class AssetSystem;
class UISystem;

using MeshLink = Id;
using MaterialLink = Id;

class AssetLinkSystem : public ISystem
{
friend class RenderSystem;

public:
	AssetLinkSystem();

	void addMeshLink(Id id, Id linkId);
	void addMaterialLink(Id id, Id linkId);

	const Table<MeshLink>&		meshLinks()		const { return m_meshLinks; }
	const Table<MaterialLink>&	materialLinks() const { return m_materialLinks; }

private:
	Table<MeshLink>			m_meshLinks;
	Table<MaterialLink>		m_materialLinks;
};

class Scene
{
friend class SceneSystem;

public:
	Scene(
		const String& name,
		const Time& time,
		Input& input);

	String name() const { return m_name; }
	bool isActive() const { return m_isActive; }

	UpdateStatus update();

	void createTestWorld(AssetSystem& assetSystem);
	void createTestWorld2(AssetSystem& assetSystem);

	Id createAddObjectButton(AssetSystem& assetSystem);
	Id createRandomBunnyEntity(AssetSystem& assetSystem);
	Id createRandomCubeEntity(AssetSystem& assetSystem);
	Id createCube(AssetSystem& assetSystem, const vec3& position, const Color& color);
	Id createBunny(AssetSystem& assetSystem, const vec3& position, const Color& color);

	const EntitySystem&		entitySystem()		const { return m_entitySystem; }
	const CameraSystem&		cameraSystem()		const { return m_cameraSystem; }
	const SelectionSystem&	selectionSystem()	const { return m_selectionSystem; }
	const TransformSystem&	transformSystem()	const { return m_transformSystem; }
	const AssetLinkSystem&	assetLinkSystem()	const { return m_assetLinkSystem; }

	EntitySystem&		entitySystem()		{ return m_entitySystem; }
	CameraSystem&		cameraSystem()		{ return m_cameraSystem; }
	SelectionSystem&	selectionSystem()	{ return m_selectionSystem; }
	TransformSystem&	transformSystem()	{ return m_transformSystem; }
	AssetLinkSystem&	assetLinkSystem()	{ return m_assetLinkSystem; }

private:
	void setIsActive(bool value) { m_isActive = value; }

	bool m_isActive = false;
	String m_name;

	EntitySystem		m_entitySystem;
	TransformSystem		m_transformSystem;
	CameraSystem		m_cameraSystem;
	SelectionSystem		m_selectionSystem;
	AssetLinkSystem		m_assetLinkSystem;
};

class SceneSystem : public ISystem
{
public:
	SceneSystem(
		const Time& time,
		Input& input);

	String name() const override { return "SceneSystem"; }
	virtual UpdateStatus update() override;

	Scene& createScene(const String& name = "Untitled");
	void activateScene(int index);

	bool hasActiveScene() const
	{
		if (m_activeSceneIdx >= 0 && m_activeSceneIdx < (int)m_scenes.size())
			return true;
		return false;
	}

	const Scene& activeScene() const { return m_scenes[m_activeSceneIdx]; }
	Scene& activeScene() { return m_scenes[m_activeSceneIdx]; }

	bool hasScene(int index) const
	{
		if (index >= 0 && index < (int)m_scenes.size())
			return true;
		return false;
	}

	const Scene& defaultScene() const { return scene(0); }
	Scene& defaultScene() { return scene(0); }

	const Scene& scene(int index) const { return m_scenes[index]; }
	Scene& scene(int index) { return m_scenes[index]; }

	// Returns -1 if the scene was not found.
	int getSceneIndex(const Scene& findScene)
	{
		int i = 0;
		for (auto&& scene : m_scenes)
		{
			if (&findScene == &scene)
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

	Array<Scene>		m_scenes;
	int					m_activeSceneIdx = 0;
};

}
