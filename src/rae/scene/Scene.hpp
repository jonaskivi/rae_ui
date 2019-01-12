#pragma once

#include "rae/core/Types.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/entity/Hierarchy.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/asset/AssetLinkSystem.hpp"
#include "rae/editor/EditorSystem.hpp"

namespace rae
{

class Time;
class AssetSystem;
class Input;
struct InputEvent;

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
	void onFrameEnd();

	void handleInput(const InputState& inputState, const Array<InputEvent>& events);

	void createTestWorld(AssetSystem& assetSystem);
	void createTestWorld2(AssetSystem& assetSystem);

	Id createAddObjectButton(AssetSystem& assetSystem);
	Id createRandomBunnyEntity(AssetSystem& assetSystem);
	Id createRandomCubeEntity(AssetSystem& assetSystem);
	Id createCube(AssetSystem& assetSystem, const vec3& position, const vec3& halfExtents, const Color& color);
	Id createSphere(AssetSystem& assetSystem, const vec3& position, float radius, const Color& color);
	Id createBunny(AssetSystem& assetSystem, const vec3& position, const Color& color);

	void selectNextEntity();

	const EntitySystem&		entitySystem()		const { return m_entitySystem; }
	const CameraSystem&		cameraSystem()		const { return m_cameraSystem; }
	const SelectionSystem&	selectionSystem()	const { return m_selectionSystem; }
	const TransformSystem&	transformSystem()	const { return m_transformSystem; }
	const AssetLinkSystem&	assetLinkSystem()	const { return m_assetLinkSystem; }
	const EditorSystem&		editorSystem()		const { return m_editorSystem; }

	EntitySystem&		entitySystem()		{ return m_entitySystem; }
	CameraSystem&		cameraSystem()		{ return m_cameraSystem; }
	SelectionSystem&	selectionSystem()	{ return m_selectionSystem; }
	TransformSystem&	transformSystem()	{ return m_transformSystem; }
	AssetLinkSystem&	assetLinkSystem()	{ return m_assetLinkSystem; }
	EditorSystem&		editorSystem()		{ return m_editorSystem; }

	bool checkIfNeedsToBeActiveScene()
	{
		bool result = m_needsToBeActiveScene;
		m_needsToBeActiveScene = false;
		return result;
	}

private:
	void setIsActive(bool value) { m_isActive = value; }

	bool m_isActive = false;
	bool m_needsToBeActiveScene = false; // Some kind of messaging system would be better than this bool.
	String m_name;

	EntitySystem		m_entitySystem;
	TransformSystem		m_transformSystem;
	CameraSystem		m_cameraSystem;
	SelectionSystem		m_selectionSystem;
	AssetLinkSystem		m_assetLinkSystem;
	EditorSystem		m_editorSystem;
};

}
