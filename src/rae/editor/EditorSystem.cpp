#include "rae/editor/EditorSystem.hpp"

#include "rae/core/Log.hpp"
#include "rae/visual/CameraSystem.hpp"
#include "rae/visual/RenderSystem.hpp"
#include "rae/asset/AssetSystem.hpp"
#include "rae/editor/SelectionSystem.hpp"
#include "rae/ui/Input.hpp"

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

bool TranslateGizmo::hover(Input& input, const Camera& camera)
{
	const float MinHoverDistance = 0.0f;
	const float MaxHoverDistance = 8000.0f;

	Ray mouseRay = camera.getExactRay(input.mouse.normalizedWindowX(), input.mouse.normalizedWindowY());

	vec4 transformedPosition = camera.getProjectionAndViewMatrix() * vec4(m_position, 1.0f);
	float gizmoCameraFactor = transformedPosition.w * m_gizmoSizeMultiplier;

	// Clear hovers
	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		m_axisHovers[i] = false;
	}

	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		Transform transform = m_axisTransforms[i];
		transform.scale = transform.scale * gizmoCameraFactor;
		transform.position = m_position + (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor);

		Box axisBox = m_lineMesh.getAabb();
		axisBox.transform(transform);
		bool isHit = axisBox.hit(mouseRay, MinHoverDistance, MaxHoverDistance);
		m_axisHovers[i] = isHit;

		// RAE_TODO: Now we just return on first hit, but we would actually want to sort them by distance.
		if (isHit)
		{
			return true;
		}
	}
	return false;
}

void TranslateGizmo::render3D(const Camera& camera, RenderSystem& renderSystem, AssetSystem& assetSystem)
{
	auto& material = assetSystem.getMaterial(m_materialId);

	vec4 transformedPosition = camera.getProjectionAndViewMatrix() * vec4(m_position, 1.0f);
	float gizmoCameraFactor = transformedPosition.w * m_gizmoSizeMultiplier;

	const auto hoverColor = Utils::createColor8bit(255, 165, 0);
	const auto activeColor = Utils::createColor8bit(255, 215, 0);

	for (int i = 0; i < (int)Axis::Count; ++i)
	{
		Transform transform = m_axisTransforms[i];

		Color color = m_axisActives[i] ? activeColor : (m_axisHovers[i] ? hoverColor : axisColor(Axis(i)));

		transform.scale = transform.scale * gizmoCameraFactor;
		transform.position = m_position + (axisVector(Axis(i)) * 0.5f * gizmoCameraFactor);
		renderSystem.renderMesh(camera,
			transform,
			color,
			material, m_lineMesh, false);
	}
}

bool TransformTool::hover(Input& input, const Camera& camera)
{
	return m_translateGizmo.hover(input, camera);
}

HandleStatus TransformTool::handleInput(Input& input, const Camera& camera)
{
	bool isActive = m_translateGizmo.isActive();
	if (isActive)
	{
		if (input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonRelease)
		{
			m_translateGizmo.deactivate();
		}
		return HandleStatus::Handled;
	}
	else
	{
		bool isHover = hover(input, camera);
		if (isHover)
		{
			if (input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonPress)
			{
				m_translateGizmo.activateHovered();
			}
			return HandleStatus::Handled;
		}
	}
	return HandleStatus::NotHandled;
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
	SelectionSystem& selectionSystem, Input& input) :
		m_cameraSystem(cameraSystem),
		m_renderSystem(renderSystem),
		m_assetSystem(assetSystem),
		m_selectionSystem(selectionSystem),
		m_input(input)
{
	Id gizmoMaterialID = m_assetSystem.createMaterial(Color(1.0f, 1.0f, 1.0f, 1.0f));
	m_transformTool.setGizmoMaterialId(gizmoMaterialID);

	using std::placeholders::_1;
	m_selectionSystem.onSelectionChanged.connect(std::bind(&TransformTool::onSelectionChanged, &m_transformTool, _1));
}

UpdateStatus EditorSystem::update()
{
	HandleStatus transformToolStatus = m_transformTool.handleInput(m_input, m_cameraSystem.getCurrentCamera());

	if (transformToolStatus == HandleStatus::NotHandled)
	{
		if (m_input.mouse.buttonEvent(MouseButton::First) == EventType::MouseButtonPress)
		{
			Id hoveredId = m_selectionSystem.hovered();
			// RAE_TODO: Needs to have a higher level function in Input where we can ask for modifier states for Control.
			if (m_input.getKeyState(KeySym::Control_L) ||
				m_input.getKeyState(KeySym::Control_R) ||
				m_input.getKeyState(KeySym::Super_L) ||
				m_input.getKeyState(KeySym::Super_R))
			{
				if (hoveredId != InvalidId)
					m_selectionSystem.toggleSelected(hoveredId);
			}
			else if (hoveredId == InvalidId)
			{
				m_selectionSystem.clearSelection();
			}
			else
			{
				m_selectionSystem.setSelection({ hoveredId });
			}
		}
	}

	return UpdateStatus::NotChanged; // for now.
}

void EditorSystem::render3D()
{
	m_transformTool.render3D(m_cameraSystem.getCurrentCamera(), m_renderSystem, m_assetSystem);
}
