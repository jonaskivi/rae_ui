#include "rae/scene/Scene.hpp"

#include "rae/core/Time.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/core/Random.hpp"

using namespace rae;

Scene::Scene(
	const String& name,
	const Time& time,
	Input& input) :
		m_name(name),
		m_entitySystem("SceneSystem"),
		m_transformSystem(),
		m_selectionSystem(m_transformSystem),
		m_cameraSystem(time, m_entitySystem, m_transformSystem, input)
{
}

UpdateStatus Scene::update()
{
	auto transformSystemStatus = m_transformSystem.update();
	auto cameraSystemStatus = m_cameraSystem.update();
	auto selectionSystemStatus = m_selectionSystem.update();

	// Return update status
	if (transformSystemStatus != UpdateStatus::NotChanged)
		return transformSystemStatus;
	else if (cameraSystemStatus != UpdateStatus::NotChanged)
		return cameraSystemStatus;
	else if (selectionSystemStatus != UpdateStatus::NotChanged)
		return selectionSystemStatus;

	return UpdateStatus::NotChanged;
}

void Scene::createTestWorld(AssetSystem& assetSystem)
{
	LOG_F(INFO, "Creating test world to scene: %s", m_name.c_str());

	createAddObjectButton(assetSystem);

	auto bunny1 = createBunny(assetSystem, glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.2f, 0.8f, 0.0f));
	auto cube1 = createCube(assetSystem, glm::vec3(3.0f, 2.0f, 1.0f), glm::vec4(0.8f, 0.6f, 0.2f, 0.0f));
}

void Scene::createTestWorld2(AssetSystem& assetSystem)
{
	LOG_F(INFO, "Creating test world 2 to scene: %s", m_name.c_str());

	//createAddObjectButton(); // at index 1

	auto cube0 = createCube(assetSystem, glm::vec3(0.0f, 0.0f, -10.0f), glm::vec4(0.8f, 0.3f, 0.3f, 0.0f));

	auto cube1 = createCube(assetSystem, glm::vec3(1.0f, 0.0f, 10.0f), glm::vec4(0.8f, 0.6f, 0.2f, 0.0f));
	auto cube2 = createCube(assetSystem, glm::vec3(-0.5f, 0.65f, -1.0f), glm::vec4(0.8f, 0.4f, 0.8f, 0.0f));
	auto cube3 = createCube(assetSystem, glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec4(0.8f, 0.5f, 0.3f, 0.0f));
	auto cube4 = createCube(assetSystem, glm::vec3(-3.15f, 0.1f, -5.0f), glm::vec4(0.05f, 0.2f, 0.8f, 0.0f));

	auto bunny1 = createBunny(assetSystem, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.05f, 0.2f, 0.8f, 0.0f));

	m_transformSystem.addChild(bunny1, cube1);
	m_transformSystem.addChild(bunny1, cube2);
	m_transformSystem.addChild(bunny1, cube3);
}

Id Scene::createAddObjectButton(AssetSystem& assetSystem)
{
	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createAddObjectButton id: %i", id);
	m_transformSystem.addTransform(id, Transform(vec3(0.0f, 0.0f, 5.0f)));
	m_transformSystem.setPosition(id, vec3(0.0f, 0.0f, 0.0f));

	m_assetLinkSystem.addMaterialLink(id, assetSystem.getAnimatingMaterialId());
	m_assetLinkSystem.addMeshLink(id, assetSystem.getCubeMeshId());

	return id;
}

Id Scene::createRandomBunnyEntity(AssetSystem& assetSystem)
{
	Id id = m_entitySystem.createEntity();
	LOG_F(INFO, "createRandomBunnyEntity id: %i", id);
	m_transformSystem.addTransform(id, Transform(vec3(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f))));

	m_assetLinkSystem.addMaterialLink(id, assetSystem.getBunnyMaterialId());
	m_assetLinkSystem.addMeshLink(id, assetSystem.getBunnyMeshId());

	return id;
}

Id Scene::createRandomCubeEntity(AssetSystem& assetSystem)
{
	Id id = m_entitySystem.createEntity();
	LOG_F(INFO, "createRandomCubeEntity id: %i", id);
	m_transformSystem.addTransform(id, Transform(vec3(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f))));

	m_assetLinkSystem.addMaterialLink(id, assetSystem.getTestMaterialId());
	m_assetLinkSystem.addMeshLink(id, assetSystem.getCubeMeshId());

	return id;
}

Id Scene::createCube(AssetSystem& assetSystem, const vec3& position, const Color& color)
{
	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createCube id: %i", id);
	// The desired API:
	m_transformSystem.addTransform(id, Transform(position));
	//m_geometrySystem.setMesh(entity, m_meshID);
	//m_materialSystem.setMaterial(entity, color);

	//assert(0);
	//////////m_assetSystem.addMaterial(id, Material(color));
	m_assetLinkSystem.addMaterialLink(id, assetSystem.getTestMaterialId());
	m_assetLinkSystem.addMeshLink(id, assetSystem.getCubeMeshId());

	return id;
}

Id Scene::createBunny(AssetSystem& assetSystem, const vec3& position, const Color& color)
{
	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createBunny id: %i", id);
	m_transformSystem.addTransform(id, Transform(position));

	m_assetLinkSystem.addMaterialLink(id, assetSystem.getBunnyMaterialId());
	m_assetLinkSystem.addMeshLink(id, assetSystem.getBunnyMeshId());

	return id;
}
