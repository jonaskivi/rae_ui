#pragma once

#include <array>

#include "rae/core/ISystem.hpp"
#include "rae/entity/Table.hpp"

#include "rae/visual/Mesh.hpp"
#include "rae/visual/Material.hpp"

namespace rae
{

class Camera;
class CameraSystem;
class RenderSystem;
class AssetSystem;
class SelectionSystem;

enum class Axis
{
	X,
	Y,
	Z,
	Count
};

class IGizmo
{
public:
	void setPosition(const vec3& position) { m_position = position; }

protected:
	vec3 m_position;

	std::array<Transform, (int)Axis::Count> m_axisTransforms;
	std::array<bool, (int)Axis::Count> m_axisHovers = { false, false, false };
	std::array<bool, (int)Axis::Count> m_axisActives = { false, false, false };
};

class TranslateGizmo : public IGizmo
{
public:
	TranslateGizmo();

	void hover() {}
	void render3D(const Camera& camera, RenderSystem& renderSystem, AssetSystem& assetSystem);

	void setGizmoMaterialId(Id id);

protected:
	float m_gizmoSizeMultiplier = 0.1f;
	Mesh m_lineMesh;
	Id m_materialId;
};

class TransformTool
{
public:
	UpdateStatus update();
	UpdateStatus hover();
	void render3D(const Camera& camera, RenderSystem& renderSystem, AssetSystem& assetSystem);

	void setGizmoMaterialId(Id id);

	void onSelectionChanged(SelectionSystem& selectionSystem);

protected:
	TranslateGizmo m_translateGizmo;
};

class EditorSystem : public ISystem
{
public:
	EditorSystem(CameraSystem& cameraSystem, RenderSystem& renderSystem, AssetSystem& assetSystem,
		SelectionSystem& selectionSystem);

	UpdateStatus update() override;
	void render3D() override;
protected:
	CameraSystem&		m_cameraSystem;
	RenderSystem&		m_renderSystem;
	AssetSystem&		m_assetSystem;
	SelectionSystem&	m_selectionSystem;

	TransformTool		m_transformTool;
};

}
