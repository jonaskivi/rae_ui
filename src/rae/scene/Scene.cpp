#include "rae/scene/Scene.hpp"

#include "rae/core/Time.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/ui/DebugSystem.hpp"
#include "rae/core/Random.hpp"
#include "rae/ui/Input.hpp"

using namespace rae;

Scene::Scene(
	const String& name,
	const Time& time,
	Input& input) :
		m_name(name),
		m_entitySystem("SceneSystem"),
		m_transformSystem(),
		m_selectionSystem(m_transformSystem),
		m_cameraSystem(time, m_entitySystem, m_transformSystem, input),
		m_editorSystem(m_selectionSystem, input)
{
}

UpdateStatus Scene::update()
{
	auto transformSystemStatus = m_transformSystem.update();
	auto cameraSystemStatus = m_cameraSystem.update();
	auto selectionSystemStatus = m_selectionSystem.update();
	auto editorSystemStatus = m_editorSystem.update(*this);

	// Return update status
	if (transformSystemStatus != UpdateStatus::NotChanged)
		return transformSystemStatus;
	else if (cameraSystemStatus != UpdateStatus::NotChanged)
		return cameraSystemStatus;
	else if (selectionSystemStatus != UpdateStatus::NotChanged)
		return selectionSystemStatus;
	else if (editorSystemStatus != UpdateStatus::NotChanged)
		return editorSystemStatus;

	return UpdateStatus::NotChanged;
}

void Scene::handleInput(const InputState& inputState, const Array<InputEvent>& events)
{
	if (inputState.mouse.anyButtonDown())
	{
		m_needsToBeActiveScene = true;
	}

	m_editorSystem.handleInput(inputState, events, *this);
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

	auto cube1 = createCube(assetSystem, glm::vec3(3.0f, 0.0f, 2.0f), glm::vec4(0.8f, 0.6f, 0.2f, 0.0f));
	auto cube2 = createCube(assetSystem, glm::vec3(-1.5f, 1.0f, -1.0f), glm::vec4(0.8f, 0.4f, 0.8f, 0.0f));
	auto cube3 = createCube(assetSystem, glm::vec3(-3.0f, 1.0f, -2.0f), glm::vec4(0.8f, 0.5f, 0.3f, 0.0f));
	auto cube4 = createCube(assetSystem, glm::vec3(-5.15f, 3.0f, -4.0f), glm::vec4(0.05f, 0.2f, 0.8f, 0.0f));

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
	vec3 halfExtents = vec3(0.5f, 0.5f, 0.5f);

	Id id = m_entitySystem.createEntity();
	LOG_F(INFO, "createRandomBunnyEntity id: %i", id);
	m_transformSystem.addTransform(id, Transform(vec3(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f))));
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	m_transformSystem.addPivot(id, Pivots::Center);

	m_assetLinkSystem.addMaterialLink(id, assetSystem.getBunnyMaterialId());
	m_assetLinkSystem.addMeshLink(id, assetSystem.getBunnyMeshId());

	return id;
}

Id Scene::createRandomCubeEntity(AssetSystem& assetSystem)
{
	vec3 halfExtents = vec3(0.5f, 0.5f, 0.5f);

	Id id = m_entitySystem.createEntity();
	LOG_F(INFO, "createRandomCubeEntity id: %i", id);
	m_transformSystem.addTransform(id, Transform(vec3(getRandom(-10.0f, 10.0f), getRandom(-10.0f, 10.0f), getRandom(4.0f, 50.0f))));
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	m_transformSystem.addPivot(id, Pivots::Center);

	m_assetLinkSystem.addMaterialLink(id, assetSystem.getTestMaterialId());
	m_assetLinkSystem.addMeshLink(id, assetSystem.getCubeMeshId());

	return id;
}

Id Scene::createCube(AssetSystem& assetSystem, const vec3& position, const Color& color)
{
	vec3 halfExtents = vec3(0.5f, 0.5f, 0.5f);

	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createCube id: %i", id);
	// The desired API:
	m_transformSystem.addTransform(id, Transform(position));
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	m_transformSystem.addPivot(id, Pivots::Center);
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
	vec3 halfExtents = vec3(0.5f, 0.5f, 0.5f);

	Id id = m_entitySystem.createEntity();
	//LOG_F(INFO, "createBunny id: %i", id);
	m_transformSystem.addTransform(id, Transform(position));
	m_transformSystem.addBox(id, Box(-(halfExtents), halfExtents));
	m_transformSystem.addPivot(id, Pivots::Center);

	m_assetLinkSystem.addMaterialLink(id, assetSystem.getBunnyMaterialId());
	m_assetLinkSystem.addMeshLink(id, assetSystem.getBunnyMeshId());

	return id;
}

void Scene::selectNextEntity()
{
	if (m_entitySystem.entityCount() <= 0)
		return;

	Id firstSelected = InvalidId;
	if (m_selectionSystem.isSelection())
	{
		Array<Id> selectedIds = m_selectionSystem.selectedIds();
		if (not selectedIds.empty())
		{
			firstSelected = selectedIds.front();
			m_selectionSystem.clearSelection();
		}
	}

	Id id = firstSelected;
	bool running = true;
	bool reachedEnd = false;
	while (running)
	{
		id = id + 1;
		if (m_entitySystem.isAlive(id))
		{
			m_selectionSystem.setSelected(id, true);
			running = false;
			break;
		}

		if (id >= m_entitySystem.biggestId())
		{
			id = InvalidId;
			if (reachedEnd)
			{
				// Second time of reaching end. Maybe this can't happen. I guess we should run into firstSelected first.
				running = false;
				break;
			}
			reachedEnd = true;
		}
	}
}
