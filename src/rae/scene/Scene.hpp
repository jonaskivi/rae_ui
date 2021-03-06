#pragma once

#include "rae/core/Types.hpp"
#include "rae/entity/EntitySystem.hpp"
#include "rae/entity/Hierarchy.hpp"
#include "rae/scene/TransformSystem.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/visual/ShapeRenderer.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/asset/AssetLinkSystem.hpp"
#include "rae/editor/EditorSystem.hpp"

namespace rae
{

class Time;
class AssetSystem;
class Input;
struct InputEvent;

// Generic scene data storage, when it is difficult to come up with where to put the table.
class SceneDataSystem : public ISystem
{
public:
	SceneDataSystem()
	{
		addTable(m_names);
	}

	void setName(Id id, const String& name) { m_names.assign(id, name); }
	const String& getName(Id id) const { return m_names.get(id); }
private:
	Table<String>		m_names;
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
	void onFrameEnd();

	void handleInput(const InputState& inputState, const Array<InputEvent>& events);

	Id createAddObjectButton(AssetSystem& assetSystem);
	Id createRandomBunnyEntity(AssetSystem& assetSystem);
	Id createRandomCubeEntity(AssetSystem& assetSystem);
	Id createCube(
		AssetSystem& assetSystem,
		const String& name,
		const vec3& position,
		const vec3& halfExtents,
		Id materialId);
	Id createSphere(AssetSystem& assetSystem, const String& name, const vec3& position, float radius, Id materialId);
	Id createBunny(AssetSystem& assetSystem, const String& name, const vec3& position, Id materialId);

	void selectNextEntity();

	const EntitySystem&		entitySystem()		const { return m_entitySystem; }
	const CameraSystem&		cameraSystem()		const { return m_cameraSystem; }
	const SelectionSystem&	selectionSystem()	const { return m_selectionSystem; }
	const TransformSystem&	transformSystem()	const { return m_transformSystem; }
	const AssetLinkSystem&	assetLinkSystem()	const { return m_assetLinkSystem; }
	const EditorSystem&		editorSystem()		const { return m_editorSystem; }
	const ShapeRenderer&	shapeRenderer()		const { return m_shapeRenderer; }
	const SceneDataSystem&	sceneDataSystem()	const { return m_sceneDataSystem; }

	EntitySystem&		modifyEntitySystem()	{ return m_entitySystem; }
	CameraSystem&		modifyCameraSystem()	{ return m_cameraSystem; }
	SelectionSystem&	modifySelectionSystem()	{ return m_selectionSystem; }
	TransformSystem&	modifyTransformSystem()	{ return m_transformSystem; }
	AssetLinkSystem&	modifyAssetLinkSystem()	{ return m_assetLinkSystem; }
	EditorSystem&		modifyEditorSystem()	{ return m_editorSystem; }
	ShapeRenderer&		modifyShapeRenderer()	{ return m_shapeRenderer; }
	SceneDataSystem&	modifySceneDataSystem()	{ return m_sceneDataSystem; }

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
	SceneDataSystem		m_sceneDataSystem;
	CameraSystem		m_cameraSystem;
	SelectionSystem		m_selectionSystem;
	AssetLinkSystem		m_assetLinkSystem;
	ShapeRenderer		m_shapeRenderer;
	EditorSystem		m_editorSystem;
};

}
