#pragma once

#include "rae/core/Types.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/entity/Hierarchy.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/asset/AssetLinkSystem.hpp"

namespace rae
{

class Time;
class AssetSystem;

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

}
