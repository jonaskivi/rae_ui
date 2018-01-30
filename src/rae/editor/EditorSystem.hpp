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
class Input;

enum class Axis
{
	X,
	Y,
	Z,
	Count
};

enum class HandleStatus
{
	NotHandled,
	Handled
};

class IGizmo
{
public:
	void setPosition(const vec3& position) { m_position = position; }

protected:
	vec3 m_position;

	std::array<Transform,	(int)Axis::Count>	m_axisTransforms;
	std::array<bool,		(int)Axis::Count>	m_axisHovers = { false, false, false };
	std::array<bool,		(int)Axis::Count>	m_axisActives = { false, false, false };
};

class TranslateGizmo : public IGizmo
{
public:
	TranslateGizmo();

	bool isHovered() const
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			if (m_axisHovers[i])
				return true;
		}
		return false;
	}

	bool isActive() const
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			if (m_axisActives[i])
				return true;
		}
		return false;
	}

	void activateHovered()
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_axisActives[i] = m_axisHovers[i];
		}
	}

	void deactivate()
	{
		for (int i = 0; i < (int)Axis::Count; ++i)
		{
			m_axisActives[i] = false;
		}
	}

	bool hover(Input& input, const Camera& camera);
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
	HandleStatus handleInput(Input& input, const Camera& camera);
	bool hover(Input& input, const Camera& camera);
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
		SelectionSystem& selectionSystem, Input& input);

	UpdateStatus update() override;
	void render3D() override;
protected:
	CameraSystem&		m_cameraSystem;
	RenderSystem&		m_renderSystem;
	AssetSystem&		m_assetSystem;
	SelectionSystem&	m_selectionSystem;
	Input&				m_input;

	TransformTool		m_transformTool;
};

}
