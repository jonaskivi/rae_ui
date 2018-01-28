#include "rae/editor/EditorSystem.hpp"

#include "rae/visual/CameraSystem.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"

using namespace rae;

const float AxisThickness = 0.05f;

vec3 axisVector(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return vec3(1.0f, 0.0f, 0.0f);
		case Axis::Y:
			return vec3(0.0f, 1.0f, 0.0f);
		case Axis::Z:
			return vec3(0.0f, 0.0f, 1.0f);
		default:
			assert(0);
			break;
	}
	return vec3(0.0f, 0.0f, 0.0f);
}

Color axisColor(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return Color(1.0f, 0.0f, 0.0f, 1.0f);
		case Axis::Y:
			return Color(0.0f, 1.0f, 0.0f, 1.0f);
		case Axis::Z:
			return Color(0.0f, 0.0f, 1.0f, 1.0f);
		default:
			assert(0);
			break;
	}
	return Color(1.0f, 1.0f, 1.0f, 1.0f);
}

qua axisRotation(Axis axis)
{
	switch(axis)
	{
		case Axis::X:
			return qua();
		case Axis::Y:
			return qua(vec3(0.0f, 0.0f, Math::toRadians(90.0f)));
		case Axis::Z:
			return qua(vec3(0.0f, Math::toRadians(90.0f), 0.0f));
		default:
			assert(0);
			break;
	}
	return qua();
}

TranslateGizmo::TranslateGizmo()
{
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		m_axisTransforms[i].position = vec3(0.0f, 0.0f, 0.0f);
		m_axisTransforms[i].rotation = axisRotation(Axis(i));
		m_axisTransforms[i].scale = vec3(1.0f, AxisThickness, AxisThickness);
	}

	m_lineMesh.generateBox();
	m_lineMesh.createVBOs();
}

void TranslateGizmo::setGizmoMaterialId(Id id)
{
	m_materialId = id;
}

void TranslateGizmo::render3D(const Camera& camera, RenderSystem& renderSystem, AssetSystem& assetSystem)
{
	auto& material = assetSystem.getMaterial(m_materialId);

	vec4 transformedPosition = camera.getProjectionAndViewMatrix() * vec4(m_position, 1.0f);

	float gizmoCameraFactor = transformedPosition.w * m_gizmoSizeMultiplier;

	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		Transform transform = m_axisTransforms[i];

		transform.scale = transform.scale * gizmoCameraFactor;
		transform.position = m_position + (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor);
		renderSystem.renderMesh(camera,
			transform,
			axisColor(Axis(i)), material, m_lineMesh, false);
	}
}

UpdateStatus TransformTool::hover()
{
	m_translateGizmo.hover();
	return UpdateStatus::NotChanged;
}

UpdateStatus TransformTool::update()
{
	UpdateStatus hoverStatus = hover();

	return UpdateStatus::NotChanged;
}

void TransformTool::render3D(const Camera& camera, RenderSystem& renderSystem, AssetSystem& assetSystem)
{
	m_translateGizmo.render3D(camera, renderSystem, assetSystem);
}

void TransformTool::setGizmoMaterialId(Id id)
{
	m_translateGizmo.setGizmoMaterialId(id);
}

void TransformTool::onSelectionChanged(SelectionSystem& selectionSystem)
{
	m_translateGizmo.setPosition(selectionSystem.selectionPosition());
}

EditorSystem::EditorSystem(CameraSystem& cameraSystem, RenderSystem& renderSystem, AssetSystem& assetSystem,
	SelectionSystem& selectionSystem) :
		m_cameraSystem(cameraSystem),
		m_renderSystem(renderSystem),
		m_assetSystem(assetSystem),
		m_selectionSystem(selectionSystem)
{
	Id gizmoMaterialID = m_assetSystem.createMaterial(Color(1.0f, 1.0f, 1.0f, 1.0f));
	m_transformTool.setGizmoMaterialId(gizmoMaterialID);

	using std::placeholders::_1;
	m_selectionSystem.onSelectionChanged.connect(std::bind(&TransformTool::onSelectionChanged, &m_transformTool, _1));
}

UpdateStatus EditorSystem::update()
{
	UpdateStatus transformToolStatus = m_transformTool.update();
	return transformToolStatus;
}

void EditorSystem::render3D()
{
	m_transformTool.render3D(m_cameraSystem.getCurrentCamera(), m_renderSystem, m_assetSystem);
}
